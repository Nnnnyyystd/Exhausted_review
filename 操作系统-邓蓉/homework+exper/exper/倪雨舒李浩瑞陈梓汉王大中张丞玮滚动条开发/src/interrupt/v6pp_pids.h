#ifndef V6PP_PIDS_H_
#define V6PP_PIDS_H_

/* 必须与内核 Utility.h 的 v6pp_pids 完全一致 */
struct v6pp_pids {
    int pid;
    int ppid;
};

#endif
