#include "processDOS.h"
#include "Pe.h"
#include "Utils.h"
#include "Kernel.h"
#include "task.h"
#include "process.h"

//����SCSI���ƿ�֧��INT31H
//Ҫ��������ڴ��л��16Kҳ������ʹ��Int 67h������EMM��������������ҳ�Ž�ҳ���֮��
//INT 18H - ROM BASIC ����rom basic������
//ִ�дŴ��ϵ� BASIC ���򣺡������ġ�IBM ���ݻ��� ROM ������ BASIC ���򣬵�����ʧ��ʱ�� BIOS ���ô����̽���ִ�С�
//��������ӡ��Boot disk error. Replace disk and press any key to continue...��������ʾ��Ϣ��
//INT 1Fh���ɵ��ã�ָ����Ƶͼ���ַ��������� 80h �� FFh �� ASCII �ַ������ݣ�����Ϣ��
//INT 1Dh���ɵ��ã�ָ����Ƶ������������Ƶģʽ�����ݣ���ָ�롣
//INT 1Eh���ɵ��ã�ָ������ģʽ���������������Ĵ�����Ϣ����ָ�롣
//INT 41h��ַָ�룺Ӳ�̲�������һӲ�̣���
//INT 46h��ַָ�룺Ӳ�̲������ڶ�Ӳ�̣���
//INT 4Ahʵʱ��������ʱ���á�
//int 19h �ὫMBR��512�ֽ�װ�ص��ڴ�0x7c00�У�Ȼ��JUMP��0x7c00������ʼִ��MBR�Ŀ�ִ�г���master booter��





int getDosPeAddr(DWORD filedata,int pid) {
	LPDOS_PE_CONTROL info = (LPDOS_PE_CONTROL)V86_TASKCONTROL_ADDRESS;
	for (int i = 0; i < LIMIT_V86_PROC_COUNT; i ++)
	{
		if (info[i].status == TASK_OVER)
		{
			info[i].status = TASK_RUN;

			info[i].pid = pid;

			if (__memcmp((char*)filedata, "MZ", 2) == 0)
			{
				info[i].address = i * 0x1000 + DOS_LOAD_FIRST_SEG + 0x10;
			}
			else
			{
				info[i].address = i * 0x1000 + DOS_LOAD_FIRST_SEG + 0x10;
			}

			return info[i].address;
		}
	}

	return 0;
}

//dosfile must be align with 16b
int relocDos(DWORD loadseg) {
	IMAGE_DOS_HEADER * hdr = (IMAGE_DOS_HEADER*)(loadseg<<4);
	int dosseg = hdr->e_cparhdr + loadseg;
	hdr->e_cs = hdr->e_cs + dosseg;
	hdr->e_ss = hdr->e_ss + dosseg;

	int relocoff = hdr->e_lfarlc;
	unsigned short * relocs = (unsigned short*)((loadseg<<4) + relocoff);
	for (int i = 0;i < hdr->e_crlc; i ++)
	{
		unsigned short relocseg = *(relocs + 1);
		unsigned short relocoff = *relocs;

		unsigned short * relocaddr = (unsigned short *) (((relocseg + dosseg) << 4) + relocoff);

		*relocaddr = (*relocaddr + dosseg);

		relocs +=2;
	}

	return hdr->e_crlc;
}


DWORD __initDosExe(DWORD filedata, int filesize,int pid) {
	int ret = 0;

	DWORD seg = getDosPeAddr( filedata,pid);
	if (seg >= INT13_RM_FILEBUF_SEG || seg <= 0)
	{
		return FALSE;
	}

	WORD offset = 0;

	DWORD dosaddr = (seg << 4) + offset;

	__memcpy((char*)dosaddr, (char*)filedata, filesize);

	if (__memcmp((char*)filedata,"MZ",2) == 0)
	{	
		ret = relocDos(seg);
	}
	else {
	}
	
	return dosaddr;
}




int __initDosTss(LPPROCESS_INFO tss, int pid, DWORD addr, char * filename, char * funcname, DWORD level, DWORD runparam) {

	int ret = 0;

	char szout[1024];

	if ((level & 3) == 0)
	{
		return FALSE;
	}
	
	tss->tss.iomapOffset = 136;
	tss->tss.iomapEnd = 0xff;
	__memset((char*)tss->tss.intMap, 0, sizeof(tss->tss.intMap));
	__memset((char*)tss->tss.iomap, 0, sizeof(tss->tss.iomap));

	//�����ǵ�������������ÿ������װ���ʱ�������ж�λ������һ������һ����ռ��Ψһ��һ��cpu�ᵼ���޷��ж�
	DWORD eflags = 0x23210;
	//eflags |= 0x4000;		//nt == 1

	WORD seg = (unsigned short)(addr >> 4);

	WORD offset = (addr & 0x0f);

	tss->tss.esp0 = TASKS_STACK0_BASE + (pid + 1) * TASK_STACK0_SIZE - STACK_TOP_DUMMY;
	tss->tss.ss0 = KERNEL_MODE_STACK;

	tss->tss.ldt = ((DWORD)glpLdt - (DWORD)glpGdt);

	tss->tss.eflags = eflags;

	tss->tss.eax = 0;
	tss->tss.ecx = 0;
	tss->tss.edx = 0;
	tss->tss.ebx = 0;
	tss->tss.ebp = 0;
	tss->tss.esi = 0;
	tss->tss.edi = 0;

	if (level & DOS_PROCESS_RUNCODE)
	{
		tss->tss.eip = offset;
		tss->tss.cs = seg;

		tss->tss.ss = KERNEL_BASE_SEGMENT;
		tss->tss.esp = V86_STACK_SIZE - STACK_TOP_DUMMY;

		tss->tss.ds = seg;
		tss->tss.es = seg;
		tss->tss.fs = seg;
		tss->tss.gs = seg;

		tss->tss.esp = tss->tss.esp - sizeof(TASKDOSPARAMS);
		LPTASKDOSPARAMS params = (LPTASKDOSPARAMS)(tss->tss.esp + (tss->tss.ss << 4));
		params->terminate = (DWORD)gV86VMLeave;
		params->pid = pid;						//param1:pid
		__strcpy(params->szFileName, filename);
		params->filename = params->szFileName;		//param2:filename
		__strcpy(params->szFuncName, funcname);
		params->funcname = params->szFuncName;		//param2:filename
		params->addr = ((seg - DOS_LOAD_FIRST_SEG) / 0x1000) * sizeof(DOS_PE_CONTROL) + V86_TASKCONTROL_ADDRESS;
		params->param = runparam;

		//__printf(szout, "__kCreateTask in file dos file:%s\r\n", filename);
		//__drawGraphChars((unsigned char*)szout, 0);
	}
	else if (__memcmp((char*)addr, "MZ", 2) == 0)
	{
		IMAGE_DOS_HEADER * dos = (IMAGE_DOS_HEADER*)addr;

		tss->tss.eip = dos->e_ip;
		tss->tss.cs = dos->e_cs;

		tss->tss.esp = dos->e_sp;
		tss->tss.ss = dos->e_ss;

		//DWORD dospsp = seg + dos->e_cparhdr - 0x10;
		DWORD dospsp = seg + dos->e_cparhdr;

		tss->tss.ds = dospsp;
		tss->tss.es = dospsp;
		tss->tss.fs = dospsp;
		tss->tss.gs = dospsp;

		//__printf(szout, "__kCreateTask dos exe file:%s\r\n", filename);
		//__drawGraphChars((unsigned char*)szout, 0);
	}
	else {
		tss->tss.eip = 0x100;

		tss->tss.cs = seg - 0x10;

		tss->tss.esp = V86_STACK_SIZE - 2;
		tss->tss.ss = tss->tss.cs;

		tss->tss.ds = tss->tss.cs;
		tss->tss.es = tss->tss.cs;
		tss->tss.fs = tss->tss.cs;
		tss->tss.gs = tss->tss.cs;

		//*(WORD*)(tss->tss.esp + addr) = 0;

		__printf(szout, "__kCreateTask dos com file:%s\r\n\r\n", filename);

	}

#ifdef TASK_SINGLE_TSS
	RETUTN_ADDRESS_V86* retv86 = (RETUTN_ADDRESS_V86*)((char*)tss->tss.esp0 - sizeof(RETUTN_ADDRESS_V86));
	retv86->ret3.ret0.cs = tss->tss.cs;
	retv86->ret3.ret0.eip = tss->tss.eip;
	retv86->ret3.ret0.eflags = tss->tss.eflags;
	retv86->ret3.esp3 = (DWORD)tss->tss.esp;
	retv86->ret3.ss3 = tss->tss.ss;
	retv86->ds = tss->tss.ds;
	retv86->es = tss->tss.es;
	retv86->fs = tss->tss.fs;
	retv86->gs = tss->tss.gs;

	tss->tss.ds = KERNEL_MODE_STACK;
	tss->tss.es = KERNEL_MODE_STACK;
	tss->tss.fs = KERNEL_MODE_STACK;
	tss->tss.gs = KERNEL_MODE_STACK;

	tss->tss.esp = (DWORD)retv86;
	tss->tss.ss = KERNEL_MODE_STACK;
#else
#endif

	tss->tss.cr3 = PDE_ENTRY_VALUE;

	tss->pid = pid;
	tss->tid = pid;

	tss->vasize = 0;
	tss->vaddr = addr;

	tss->level = level & 3;

	tss->window = 0;

	tss->counter = 0;
	
	tss->moduleaddr = addr;
	tss->moduleLinearAddr = addr;

	__strcpy(tss->filename, filename);

	__strcpy(tss->funcname, funcname);

	addTaskList(tss->tid);

	return TRUE;
}