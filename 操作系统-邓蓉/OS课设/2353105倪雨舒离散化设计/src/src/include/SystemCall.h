#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

/* 
 * SystemCallTableEntry�ṹ��ϵͳ
 * ���ô���������ڱ��ı��
 * 
 * ��Ӧ��UnixV6�����е�sysent�ṹ
 * struct sysent		@line 2667
 * {
 *	int count;
 *	int (*call)();
 * }
 */
/*ϵͳ������ڱ�����Ķ���*/
struct SystemCallTableEntry
{
	unsigned int	count;			//ϵͳ���õĲ�������
			 int	(*call)();		//��Ӧϵͳ���ô���������ָ��
};

/* 
 * UNIX V6��ʹ�ñ����trapָ����ĵ�6bit��Ϊindex������ڱ���������
 * trapָ���ܹ���Բ�ͬϵͳ���ò�����ָͬ���롣��X86ƽ̨�ϵ�intָ��
 * �޷�����������ͬ��ָ���룬���ͨ��eax�Ĵ�������ϵͳ���ú���Ϊindex��
 * 
 * eax�д��ϵͳ���úţ���Ϊ������ڱ��к�����index��
 * ebx��ʼ����û�̬�����ṩ��ϵͳ���õ�һ��������ecx�ڶ��������Դ�����
 * ebp������Ĳ�������������6����������ʵUNIX V6��ϵͳ���ò������ֻ��4����
 *��
 * ���Ὣ�������ת�浽u.u_arg[5]�С�
 */
class SystemCall
{
public:
	/*ϵͳ���ô���������ڱ��Ĵ�С*/
	static const unsigned int SYSTEM_CALL_NUM = 64;

public:
	SystemCall();
	~SystemCall();

public:
	/* ƫ�Ƶ�ַ�����IDT[0x80]�������е�ϵͳ������ں���
	 *
	 *	UNIX V6�е�����"��ں���"�����ֽ�trap(@line 752)���������û��д�ģ�
	 * ��C����trap(dev, sp, r1, nps, r0, pc, ps)ͬ�������𱣴��ֳ�����dev��
	 * sp�Ȳ���ѹ�����ջ��Ȼ�����C����д��trap(dev, sp, r1, nps, r0, pc, ps)
	 * 
	 * "���trap" @line 0755����Ļ������������봦���ӳ��򷵻��Ժ��ж�
	 * �Ƿ���Ҫswtch()���Լ��ָ��ֳ��Ĺ�����
	 */
	static void SystemCallEntrance();

	/* ��ӦUNIX V6�е�trap(dev, sp, r1, nps, r0, pc, ps)����,
	 * ��Ҫ����V6��ϵͳ���õ�switch��֧��case 6+USER: // sys call
	 * �������쳣��X86ƽ̨����INT 0-31��handler����������V6����
	 * ��trap(dev,...)��ͨ��switch�����ֲ�ͬ������(�쳣)��
	 */
	static void Trap(struct pt_regs* regs, struct pt_context* context);

	/* ��ӦUNIX V6�е�trap1( int (*f)() )����@line 2841
	 * �˺�����trap(dev,...)�������ã�trap(dev,...)����
	 * �ṩ����ڱ��л�ȡ�ĺ���ָ�룬��Ϊ�������ݸ�trap1( int (*f)());
	 */
	static void Trap1(int (*func)());
	

private:
	/* ����ĺ�����Ӧϵͳ������ڱ��еĴ���������ڵ�ַ,
	 * ���Ǹ���ϵͳ�����ں���̬�½��еľ��崦���߼���
	 *
	 * ���ﺯ��ͳһ����Ϊint func(void);��ϵͳ���õķ���ֵ
	 * ������ͨ��int���أ�ֻ��Ϊ�˺�int (*call)()����ƥ�䡣
	 *
	 * UNIX V6�з���ֵ����u.u_ar0[R0]�У�Ҳ����ͨ��r0�Ĵ���
	 * ���أ������￼��ʹ��EAX�Ĵ�������ϵͳ���ý�����û�̬
	 * ����
	 */

	/*	0 = indir	count = 0	*/
	static int Sys_NullSystemCall();	/*��V6���������ϵͳ���ã�x86�ϲ���Ҫ�����ᱻ���õ��Ŀպ��� */

	/*	1 = rexit	count = 0	*/
	static int Sys_Rexit();

	/*	2 = fork	count = 0	*/
	static int Sys_Fork();
	
	/*	3 = read	count = 2	*/
	static int Sys_Read();
	
	/*	4 = write	count = 2	*/
	static int Sys_Write();
	
	/*	5 = open	count = 2	*/
	static int Sys_Open();
	
	/*	6 = close	count = 0	*/
	static int Sys_Close();
	
	/*	7 = wait	count = 0	*/
	static int Sys_Wait();
	
	/*	8 = creat	count = 2	*/
	static int Sys_Creat();
	
	/*	9 = link	count = 2	*/
	static int Sys_Link();
	
	/*	10 = unlink	count = 1	*/
	static int Sys_UnLink();
	
	/*	11 = exec	count = 2	*/
	static int Sys_Exec();
	
	/*	12 = chdir	count = 1	*/
	static int Sys_ChDir();
	
	/*	13 = gtime	count = 0	*/
	static int	Sys_GTime();
	
	/*	14 = mknod	count = 3	*/
	static int Sys_MkNod();
	
	/*	15 = chmod	count = 2	*/
	static int Sys_ChMod();
	
	/*	16 = chown	count = 2	*/
	static int Sys_ChOwn();
	
	/*	17 = sbreak	count = 1	*/
	static int Sys_SBreak();
	
	/*	18 = stat	count = 2	*/
	static int Sys_Stat();
	
	/*	19 = seek	count = 2	*/
	static int Sys_Seek();
	
	/*	20 = getpid	count = 0	*/
	static int Sys_Getpid();
	
	/*	21 = mount	count = 3	*/
	static int Sys_Smount();
	
	/*	22 = umount  count = 1	*/
	static int Sys_Sumount();
	
	/*	23 = setuid	count = 0	*/
	static int Sys_Setuid();
	
	/*	24 = getuid	count = 0	*/
	static int Sys_Getuid();
	
	/*	25 = stime	count = 0	*/
	static int Sys_Stime();
	
	/*	26 = ptrace	count = 3	*/
	static int Sys_Ptrace();
	
	/*	27 = nosys	count = 0	*/
	static int Sys_Nosys();		/* ��ʾ��ǰϵͳ���úű���δʹ�ã�����������չ */
	
	/*	28 = fstat	count = 1	*/
	static int Sys_FStat();
	
	/*	29 = trace	count = 1	*/
	static int Sys_Trace();
	
	/*	30 =  smdate; inoperative	count = 1	handler = nullsys	*/
	
	/*	31 = stty	count = 1	*/
	static int Sys_Stty();
	
	/*	32 = gtty	count = 1	*/
	static int Sys_Gtty();
	
	/*	33 = nosys	count = 0	*/
	
	/*	34 = nice	count = 0	*/
	static int Sys_Nice();
	
	/*	35 = sleep	count = 0	*/
	static int Sys_Sslep();		/* Don't Confused with sleep(chan, pri) */
	
	/*	36 = sync	count	= 0	*/
	static int Sys_Sync();
	
	/*	37 = kill	count = 1	*/
	static int Sys_Kill();
	
	/*	38 = switch	count = 0	*/
	static int Sys_Getswit();
	
	/*	39 = pwd	count = 1	*/
	static int Sys_Pwd();
	
	/*	40 = nosys	count = 0	*/
	
	/*	41 = dup	count = 0	*/
	static int Sys_Dup();
	
	/*	42 = pipe	count = 0	*/
	static int Sys_Pipe();
	
	/*	43 = times	count = 1	*/
	static int Sys_Times();
	
	/*	44 = prof	count = 4	*/
	static int Sys_Profil();
	
	/*	45 = nosys	count = 0	*/
	
	/*	46 = setgid	count = 0	*/
	static int Sys_Setgid();
	
	/*	47 = getgid	count = 0	*/
	static int Sys_Getgid();
	
	/*	48 = sig	count = 2	*/
	static int Sys_Ssig();


	static int Sys_Getppid();

	static int Sys_Getpids(); 
	
	static int Sys_Getproc(); 
	/*	49 ~ 63 = nosys	count = 0	*/	

private:
	/*ϵͳ������ڱ�������*/
	static SystemCallTableEntry m_SystemEntranceTable[SYSTEM_CALL_NUM];
};

#endif
