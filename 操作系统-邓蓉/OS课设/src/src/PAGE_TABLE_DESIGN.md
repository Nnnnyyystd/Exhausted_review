# Page Table Design Notes

This document explains how the page-table-related redesign in `src` was completed, and why the current result shows that the paging design is working in the intended direction.

## 1. Goal

Before this change, the system still behaved closer to a "contiguous process image" model. Even though the hardware used paging, the kernel-side management logic was still centered on:

- contiguous physical memory regions
- a more global user page table view
- image growth and relocation at coarse granularity

The goal of this redesign was to move the system toward a clearer page-based model:

- physical memory is allocated and freed in page frames
- each process owns its own page directory
- each process owns its own private user page table
- `fork`, `exec`, `brk`, stack growth, and `exit` are handled page by page
- the design leaves room for shared pages and copy-on-write behavior

## 2. Physical Memory Is Now Managed by Page Frame

### 2.1 Bitmap-based allocation

Relevant files:

- [MapNode.h](/d:/UNIX%20V6++V1/oos/src/include/MapNode.h)
- [Allocator.h](/d:/UNIX%20V6++V1/oos/src/include/Allocator.h)
- [Allocator.cpp](/d:/UNIX%20V6++V1/oos/src/mm/Allocator.cpp)

The new low-level mechanism is based on `BitMap` and `BitMapAllocator`.

`BitMap` stores:

- the physical start address of a pool
- the number of bitmap rows
- the bitmap array itself

`BitMapAllocator` provides:

- `Alloc(BitMap &bitmap, unsigned long size)`
- `Free(BitMap &bitmap, unsigned long size, unsigned long addrIdx)`

The key design change is:

1. user physical memory is no longer treated only as one large contiguous chunk
2. allocation scans for free 4 KB page frames
3. allocation marks pages in the bitmap, and free clears them again

This is the first step of memory discretization.

### 2.2 `PageManager` now works at page level

Relevant files:

- [PageManager.h](/d:/UNIX%20V6++V1/oos/src/include/PageManager.h)
- [PageManager.cpp](/d:/UNIX%20V6++V1/oos/src/mm/PageManager.cpp)

`PageManager` now has two important members:

```cpp
BitMap bitmap;
int Page[MemoryDescriptor::USER_SPACE_SIZE / PAGE_SIZE];
```

Their roles are:

- `bitmap`: records whether a physical page frame is free or used
- `Page[]`: page reference count, used for sharing and COW-related logic

The behavior of `AllocMemory()` and `FreeMemory()` is now page-oriented:

- `AllocMemory()` calls `BitMapAllocator::Alloc()`
- `FreeMemory()` first decrements the page reference count
- a page frame is actually returned to the bitmap allocator only when the reference count reaches zero

This shows that the allocation unit has changed from "region" to "page frame".

## 3. Each Process Now Owns Page Table State

### 3.1 Each process owns a page directory

Relevant files:

- [Process.h](/d:/UNIX%20V6++V1/oos/src/include/Process.h)
- [Process.cpp](/d:/UNIX%20V6++V1/oos/src/proc/Process.cpp)

`Process` now contains:

```cpp
PageDirectory *pPageDirectory;
unsigned long GetPageDirectoryPhyAddr();
```

This means the page directory is now part of per-process state, instead of relying on only one global user mapping structure.

`GetPageDirectoryPhyAddr()` converts the kernel linear address of the page directory into the physical address required by `CR3`.

### 3.2 Each process owns a private user page table

Relevant files:

- [MemoryDescriptor.h](/d:/UNIX%20V6++V1/oos/src/include/MemoryDescriptor.h)
- [MemoryDescriptor.cpp](/d:/UNIX%20V6++V1/oos/src/proc/MemoryDescriptor.cpp)

The important change is that `MemoryDescriptor::Initialize()` no longer allocates two user page tables at once. It allocates only one private user page table:

```cpp
this->m_UserPageTableArray =
    (PageTable *)(kernelPageManager.AllocMemory(sizeof(PageTable))
    + Machine::KERNEL_SPACE_START_ADDRESS);
```

So the current structure is:

- user page table 0: shared/common table
- user page table 1: private table for the current process

Also, `MapToPageTable()` no longer copies relative mappings into one global runtime table. Instead, it simply refreshes the current process page directory:

```cpp
FlushPageDirectory(u.u_procp->GetPageDirectoryPhyAddr());
```

That means the page table content is already prepared inside the process state; the CPU only needs to switch to that page directory.

## 4. How the Page Directory Is Built

Relevant files:

- [ProcessManager.h](/d:/UNIX%20V6++V1/oos/src/include/ProcessManager.h)
- [ProcessManager.cpp](/d:/UNIX%20V6++V1/oos/src/proc/ProcessManager.cpp)

Two important helper functions were added:

- `AllocPageDirectory()`
- `InitProcPageDirectory(Process *proc, PageTable *privateUsrPageTable)`

The page directory layout is:

1. entry 0 points to the shared user page table 0
2. the kernel high-memory entry points to the kernel page table
3. entry 1 points to the process-private user page table 1

So every process gets:

- a shared low user-space mapping entry
- a shared kernel-space mapping entry
- a private user-space page-table entry

This is the intended "shared common mappings + private process mappings" structure.

## 5. Why Context Switch Also Switches Address Space

Relevant files:

- [Assembly.h](/d:/UNIX%20V6++V1/oos/src/include/Assembly.h)
- [ProcessManager.h](/d:/UNIX%20V6++V1/oos/src/include/ProcessManager.h)
- [ProcessManager.cpp](/d:/UNIX%20V6++V1/oos/src/proc/ProcessManager.cpp)

`FlushPageDirectory` was changed into a parameterized macro:

```cpp
#define FlushPageDirectory(pgTable) \
    __asm__ __volatile__(" movl %0, %%cr3" : : "r"(pgTable));
```

So `CR3` is no longer always loaded with a fixed `0x200000`. It is now loaded dynamically with the current process page directory physical address.

`SwtchUStruct(p)` was also changed so that it:

1. updates the kernel page-table entry that maps the PPDA area
2. loads `p->GetPageDirectoryPhyAddr()` into `CR3`

As a result, process scheduling now switches:

- saved registers
- kernel stack context
- page-directory-based address space

## 6. How `fork()` Reflects the New Design

Relevant file:

- [ProcessManager.cpp](/d:/UNIX%20V6++V1/oos/src/proc/ProcessManager.cpp#L100)

There are three major changes in the `fork()` path.

### 6.1 The child gets its own page directory and page table

Inside `NewProc()`:

- a new private user page table is allocated for the child
- a new page directory is allocated for the child
- `InitProcPageDirectory()` fills the child page directory

This means parent and child no longer rely on the same page-table object.

### 6.2 Writable parent mappings are converted to read-only

`ModifyPageTable()` scans the parent page table and:

- finds present writable entries
- changes `m_ReadWriter` from writable to read-only
- increments the reference count for the underlying physical frame

This is the expected preparation step for copy-on-write style behavior.

### 6.3 Parent and child keep the same virtual addresses

That is exactly what `pageDemo` shows: both processes print the same virtual addresses for the same variables, but writes do not corrupt each other.

## 7. Why `exec`, `brk`, Stack Growth, and `exit` Also Prove the Design

### 7.1 `exec`

Relevant files:

- [ProcessManager.cpp](/d:/UNIX%20V6++V1/oos/src/proc/ProcessManager.cpp#L492)
- [PEParser.cpp](/d:/UNIX%20V6++V1/oos/src/pe/PEParser.cpp)

`Exec()` no longer depends on "grow one large process image and relocate it". Instead it:

- allocates text pages one by one
- allocates data pages one by one
- allocates stack pages one by one
- writes those mappings directly into the private user page table
- refreshes the current process mapping through `MapToPageTable()`

Also, the `Text` structure changed from one base address into a page-frame array:

```cpp
unsigned long x_caddr[10];
```

That is direct evidence that the text segment is no longer modeled as one contiguous physical range only.

### 7.2 `brk` and stack growth

Relevant files:

- [Process.cpp](/d:/UNIX%20V6++V1/oos/src/proc/Process.cpp#L325)
- [Process.cpp](/d:/UNIX%20V6++V1/oos/src/proc/Process.cpp#L368)

`SStack()` and `SBreak()` now work by updating page-table entries page by page:

- stack growth allocates one more page and writes one more PTE
- heap expansion allocates page frames and fills PTEs
- heap shrink frees page frames and clears PTEs

This shows that runtime address-space growth is now page-based.

### 7.3 `exit`

Relevant file:

- [Process.cpp](/d:/UNIX%20V6++V1/oos/src/proc/Process.cpp#L135)

`Exit()` also became page-based:

- it walks data-segment-related PTEs
- it walks stack-related PTEs
- it frees page frames one by one
- it clears PTEs
- it finally releases the page table itself

So process teardown now reclaims pages, not just one contiguous memory image.

## 8. Kernel Helpers Had To Change Too

Relevant files:

- [Utility.cpp](/d:/UNIX%20V6++V1/oos/src/kernel/Utility.cpp)
- [Machine.cpp](/d:/UNIX%20V6++V1/oos/src/machine/Machine.cpp)
- [main.cpp](/d:/UNIX%20V6++V1/oos/src/kernel/main.cpp)

### 8.1 `Utility::CopySeg`

`CopySeg()` now:

1. borrows two entries from the kernel page table
2. maps the source page and destination page into kernel virtual space
3. refreshes the current process page directory
4. copies page contents through kernel virtual addresses
5. restores the borrowed entries and refreshes again

So even page copying now explicitly depends on page-table manipulation.

### 8.2 `Machine`

`Machine` gained:

- `InitKernelPageTable()`
- `InitZeroUserPageTable()`

Also, `EnablePageProtection()` was adjusted so that the page-directory physical address is written explicitly into `CR3`. This confirms that the low-level paging control flow was redesigned around dynamic page-directory switching.

## 9. How `pageDemo` Validates the Result

Relevant files:

- [pageDemo.c](/d:/UNIX%20V6++V1/oos/src/program/pageDemo.c)
- [Makefile](/d:/UNIX%20V6++V1/oos/src/program/Makefile#L58)
- [Makefile](/d:/UNIX%20V6++V1/oos/src/program/Makefile#L207)

This demo program validates the design in two ways.

### 9.1 It prints one process address layout

It prints:

- `main1`
- the global variable `g_data`
- a stack variable
- multiple `malloc()` results
- the current heap end from `sbrk(0)`

That demonstrates that different user-space regions live in different logical pages.

### 9.2 It compares parent and child after `fork()`

The program then:

- forks
- prints `g_data` and `heap_b` addresses and values in the child
- modifies those values in the child
- waits in the parent
- prints the same addresses and values again in the parent

If the output shows:

- parent and child see the same virtual addresses
- the parent still preserves the old values after the child writes

then it proves:

1. parent and child share the same virtual address layout
2. they are isolated by different page-table state and physical frames
3. the current paging design is working correctly

## 10. Conclusion

This redesign is not just "using another allocator". It changed the whole address-space organization into:

- physical memory managed by page frame
- page-frame reference counting
- one page directory per process
- one private user page table per process
- `fork`, `exec`, `brk`, stack growth, and `exit` all driven by page-table updates
- dynamic `CR3` switching during process switch

From both the code structure and the `pageDemo` output, the system now supports the key property we wanted:

"same virtual address, but isolated physical-page-backed contents across processes"

That is why the current page table design can be considered correct in structure and effective in behavior.
