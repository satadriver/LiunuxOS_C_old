#include "process.h"
#include "task.h"
#include "Utils.h"
#include "Kernel.h"
#include "video.h"
#include "Pe.h"
#include "processDOS.h"
#include "file.h"
#include "systemTimer.h"
#include "page.h"
#include "def.h"
#include "malloc.h"





TASK_LIST_ENTRY *gTasksListPtr = 0;

DWORD g_taskLock = 0;


void __terminateTask(int tid, char * filename, char * funcname, DWORD lpparams) {
	int retvalue = 0;
	__asm {
		mov retvalue, eax
	}

	removeTaskList(tid);
	__sleep(-1);
}


TASK_LIST_ENTRY* searchTaskList(int tid) {
	TASK_LIST_ENTRY* tasklist = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;

	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)tasklist;
	for (int i = 0; i < TASK_LIMIT_TOTAL; i++)
	{
		if (list[i].valid  && list[i].process->status == TASK_RUN && list[i].process->tid == tid) {
			return &list[i];
		}
	}
	return 0;
}


TASK_LIST_ENTRY* addTaskList(int tid) {
	LPPROCESS_INFO base = (LPPROCESS_INFO)TASKS_TSS_BASE;

	TASK_LIST_ENTRY* tasklist = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;

	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)tasklist;
	for (int i = 0; i < TASK_LIMIT_TOTAL; i++)
	{
		if (list[i].valid == 0 ) {
			list[i].valid = TRUE;

			list[i].process = base + tid;
			list[i].process->status = TASK_RUN;
			
			addlistTail((LIST_ENTRY*)&tasklist->list, (LIST_ENTRY*)&list[i].list);
			return &list[i];
		}
	}
	return 0;
}



TASK_LIST_ENTRY* removeTaskList(int tid) {

	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do 
	{
		if (list->valid && list->process && list->process->tid == tid)
		{
			removelist((LIST_ENTRY*)&list->list);

			list->process->status = TASK_OVER;
			list->process = 0;

			list->valid = FALSE;

			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
		if (list == 0)
		{
			break;
		}
	} while (list != (TASK_LIST_ENTRY *)TASKS_LIST_BASE);

	return 0;
}



int __initTask() {

	LPPROCESS_INFO tssbase = (LPPROCESS_INFO)TASKS_TSS_BASE;
	for (int i = 0; i < TASK_LIMIT_TOTAL; i++)
	{
		__memset((char*)&tssbase[i], 0, sizeof(PROCESS_INFO));
	}

	DWORD *backuptables = (DWORD*)BACKUP_PAGE_TABLES;
	DWORD * cr3 = (DWORD*)PDE_ENTRY_VALUE;
	for (int i = 0; i < ITEM_IN_PAGE; i++)
	{
		backuptables[i] = cr3[i];
	}

	LPPROCESS_INFO process0 = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	__strcpy(process0->filename, KERNEL_DLL_MODULE_NAME);
	__strcpy(process0->funcname, "__kernelEntry");
	process0->tid = 0;
	process0->pid = 0;
	process0->moduleaddr = (DWORD)KERNEL_DLL_BASE;
	process0->level = 0;
	process0->counter = 0;
	process0->tss.esp0 = TASKS_STACK0_BASE + TASK_STACK0_SIZE - STACK_TOP_DUMMY;
	process0->tss.iomapOffset = 136;
	process0->tss.iomapEnd = 0xff; 
	__asm {
		mov ax,cs
		movzx eax,ax
		//mov process0.tss.ss0,eax
	}
	//process0->tss.ss0 = 0x10;
	__memset((char*)process0->tss.intMap, 0, sizeof(process0->tss.intMap));
	__memset((char*)process0->tss.iomap, 0, sizeof(process0->tss.iomap));
	process0->tss.cr3 = PDE_ENTRY_VALUE;
	process0->status = TASK_RUN;
	process0->vaddr = KERNEL_DLL_BASE;
	process0->vasize = 0;
	process0->espbase = KERNEL_TASK_STACK_BASE;
	//process0->tss.esp0 = TASKS_STACK0_BASE + TASK_STACK0_SIZE - STACK_TOP_DUMMY;
	//process0->tss.ss0 = KERNEL_MODE_STACK;

	__memcpy((char*)TASKS_TSS_BASE, (char*)CURRENT_TASK_TSS_BASE, sizeof(PROCESS_INFO));

	__memset((char*)TASKS_LIST_BASE, 0, TASK_LIMIT_TOTAL * sizeof(TASK_LIST_ENTRY));

	gTasksListPtr = (TASK_LIST_ENTRY *)TASKS_LIST_BASE;
	initListEntry(&gTasksListPtr->list);
	gTasksListPtr->process = (LPPROCESS_INFO)TASKS_TSS_BASE;
	gTasksListPtr->valid = 1;

	__memset((char*)V86_TASKCONTROL_ADDRESS, 0, LIMIT_V86_PROC_COUNT*12);
	return 0;
}

//CF(bit 0) [Carry flag]   
//若算术操作产生的结果在最高有效位(most-significant bit)发生进位或借位则将其置1，反之清零。
//这个标志指示无符号整型运算的溢出状态，这个标志同样在多倍精度运算(multiple-precision arithmetic)中使用

//SF(bit 7) [Sign flag]   
//该标志被设置为有符号整型的最高有效位。(0指示结果为正，反之则为负) 

//OF(bit 11) [Overflow flag]   
//如果整型结果是较大的正数或较小的负数，并且无法匹配目的操作数时将该位置1，反之清零。这个标志为带符号整型运算指示溢出状态

//TF(bit 8) [Trap flag]   将该位设置为1以允许单步调试模式，清零则禁用该模式

//OF是有符号数运算结果的标志
//OF标志：这个标志有点复杂，其结果是CF标志和次最高位是否发生进位（如果进位是1，没进位是0）进行异或的结果
//OF只对有符号数运算有意义，CF对无符号数运算有意义
//MOV AX,858F
//SUB AX,7869

// IOPL是I/O保护机制中的关键之一，它位于EFLAGS寄存器的第12、13位。指令in、ins、out、outs、cli、sti只有在CPL<= IOPL时才能执行。
//这些指令被称为I/O敏感指令，如果特权级低的指令视图访问这些I/O敏感指令将会导致常规保护错误(#GP)
//可以改变IOPL的指令只有popfl和iret指令，但只有运行在特权级0的程序才能将其改变

//EM位控制浮点指令的执行是用软件模拟，还是由硬件执行。EM=0时，硬件控制浮点指令传送到协处理器；EM=1时，浮点指令由软件模拟。 
//TS位用于加快任务的切换，通过在必要时才进行协处理器切换的方法实现这一目的。每当进行任务切换时，处理器把TS置1。
//TS = 1时，浮点指令将产生设备不可用(DNA)异常。 
//MP位控制WAIT指令在TS = 1时，是否产生DNA异常。MP = 1和TS = 1时，WAIT产生异常；MP = 0时，WAIT指令忽略TS条件，不产生异常。



void prepareTss(LPPROCESS_INFO tss) {
	__memset((CHAR*)tss, 0, sizeof(PROCESS_INFO));
	tss->status = TASK_SUSPEND;

	tss->tss.iomapEnd = 0xff;
	tss->tss.iomapOffset = 136;

	//tss->tss.trap = 1;
}


int __getFreeTask(LPTASKRESULT ret) {
	if (ret == 0)
	{
		return FALSE;
	}
	ret->lptss = 0;
	ret->number = 0;

	LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
	for (int i = 0;i < TASK_LIMIT_TOTAL; i++)
	{
		if (tss[i].status == TASK_OVER)
		{
			prepareTss(&tss[i]);

			ret->number = i;
			ret->lptss = &tss[i];
			return TRUE;
		}
	}

	return FALSE;
}



int __createDosInFileTask(DWORD addr, char * filename) {
	if (__findProcessFileName(filename))
	{
		return 0;
	}
	return __kCreateProcess(addr,0x10000, filename, filename, DOS_PROCESS_RUNCODE | 3, 0);
}





TASK_LIST_ENTRY* __findProcessFuncName(char * funcname) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (__strcmp(list->process->funcname,funcname) == 0)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
		if (list == 0)
		{
			break;
		}
	} while (list != (TASK_LIST_ENTRY *)TASKS_LIST_BASE);

	return FALSE;
}

TASK_LIST_ENTRY * __findProcessFileName(char * filename) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (__strcmp(list->process->filename, filename) == 0)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
	} while (list != (TASK_LIST_ENTRY *)TASKS_LIST_BASE);

	return FALSE;
}



TASK_LIST_ENTRY* __findProcessByPid(int pid) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (list->process->pid == pid)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;

	} while (list != (TASK_LIST_ENTRY *)TASKS_LIST_BASE);

	return FALSE;
}


TASK_LIST_ENTRY* __findProcessByTid(int tid) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (list->process->tid == tid)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
	} while (list != (TASK_LIST_ENTRY *)TASKS_LIST_BASE);

	return FALSE;
}


int __terminateByFileName(char * filename) {

	TASK_LIST_ENTRY* list = __findProcessFileName(filename);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return FALSE;
}

int __terminateByFuncName(char * funcname) {

	TASK_LIST_ENTRY* list = __findProcessFuncName(funcname);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return FALSE;
}

int __terminatePid(int pid) {

	TASK_LIST_ENTRY* list = __findProcessByPid(pid);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return 0;
}


int __terminateTid(int tid) {

	TASK_LIST_ENTRY* list = __findProcessByTid(tid);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return 0;
}



int __pauseTid(int tid) {

	TASK_LIST_ENTRY* list = __findProcessByTid(tid);
	if (list)
	{
		list->process->status = TASK_SUSPEND;
	}

	return 0;
}


int __resumeTid(int tid) {

	TASK_LIST_ENTRY* list = __findProcessByTid(tid);
	if (list)
	{
		list->process->status = TASK_RUN;
	}

	return 0;
}


int __pausePid(int pid) {

	TASK_LIST_ENTRY* list = __findProcessByPid(pid);
	if (list)
	{
		list->process->status = TASK_SUSPEND;
	}

	return 0;
}


int __resumePid(int pid) {

	TASK_LIST_ENTRY* list = __findProcessByPid(pid);
	if (list)
	{
		list->process->status = TASK_RUN;
	}

	return 0;
}


#ifndef SINGLE_TASK_TSS
extern "C"  __declspec(dllexport) DWORD __kTaskSchedule(LIGHT_ENVIRONMENT* regs) {

	systimerProc();

// 	LPDOS_PE_CONTROL dos_status = (LPDOS_PE_CONTROL)V86_TASKCONTROL_ADDRESS;
// 	for (int i = 0; i < LIMIT_V86_PROC_COUNT; i++)
// 	{
// 		if (dos_status[i].status == DOS_TASK_OVER)
// 		{
// 			LPPROCESS_INFO p = __findProcessByPid(dos_status[i].pid);
// 			if (p)
// 			{
// 				p->status = TASK_OVER;
// 				removeTaskList(dos_status[i].pid);
// 			}
// 		}
// 	}


	TASK_LIST_ENTRY * prev = gTasksListPtr;

	TASK_LIST_ENTRY* next = (TASK_LIST_ENTRY*)gTasksListPtr->list.next;
	do
	{
		if (next == 0 || next == prev)
		{
			return FALSE;
		}

		if (next->process->status != TASK_RUN)
		{
			next = (TASK_LIST_ENTRY*)next->list.next;
			continue;
		}

		if (next->process->sleep)
		{
			next->process->sleep--;
		}

		if (next->process->sleep)
		{
			next = (TASK_LIST_ENTRY*)next->list.next;
			continue;
		}
		else {
			break;
		}
	} while (1);
	
	gTasksListPtr = (TASK_LIST_ENTRY*)next;

	if (prev->process->tid == gTasksListPtr->process->tid)
	{
		//return 0;
	}

	LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	
// 	if (process->tss.link)
// 	{
// 		char szout[1024];
// 		__printf(szout, "tid:%d pid:%d link:%d\r\n", process->tid, process->pid, process->tss.link);
// 		__drawGraphChars((unsigned char*)szout, 0);
// 		process->tss.link = 0;
// 	}

	//切换到新任务的cr3和ldt会被自动加载，但是iret也会加载cr3和ldt，因此不需要手动加载
// 	DWORD nextcr3 = gTasksListPtr->process->tss.cr3;
// 	LPTSS timertss = (LPTSS)gAsmTsses;
// 	timertss->cr3 = nextcr3;
// 	short ldt = ((DWORD)glpLdt - (DWORD)glpGdt);
// 	__asm {
// 		mov ax,ldt
// 		lldt ax
// 	}

	//if (prev->process->status == TASK_RUN && process->status == TASK_RUN)
	{
		process->counter++;
		__memcpy((char*)(tss + prev->process->tid), (char*)process, sizeof(PROCESS_INFO));
	}
	
	//if (gTasksListPtr->process->status == TASK_RUN)
	{
		__memcpy((char*)process, (char*)(gTasksListPtr->process->tid + tss), sizeof(PROCESS_INFO));
	}
	
	//tasktest();

 	char * fenvprev = (char*)FPU_STATUS_BUFFER + (prev->process->tid << 9);
	//If a memory operand is not aligned on a 16-byte boundary, regardless of segment
	//The assembler issues two instructions for the FSAVE instruction (an FWAIT instruction followed by an FNSAVE instruction), 
	//and the processor executes each of these instructions separately.
	//If an exception is generated for either of these instructions, the save EIP points to the instruction that caused the exception.
	__asm {
		clts			//before all fpu instructions
		fwait
		mov eax, fenvprev
		FxSAVE[eax]
		//fsave [fenv]
		//FNCLEX
	}
	prev->process->tss.fpu = 1;

	if (gTasksListPtr->process->tss.fpu)
	{
		char * fenvnext = (char*)FPU_STATUS_BUFFER + (gTasksListPtr->process->tid << 9);
		__asm {
			clts
			fwait
			finit
			mov eax, fenvnext
			//frstor [fenv]
			fxrstor[eax]
		}
	}

	return TRUE;
}

#else
extern "C"  __declspec(dllexport) DWORD __kTaskSchedule(LIGHT_ENVIRONMENT * env) {

	systimerProc();

	// 	LPDOS_PE_CONTROL dos_status = (LPDOS_PE_CONTROL)V86_TASKCONTROL_ADDRESS;
	// 	for (int i = 0; i < LIMIT_V86_PROC_COUNT; i++)
	// 	{
	// 		if (dos_status[i].status == DOS_TASK_OVER)
	// 		{
	// 			LPPROCESS_INFO p = __findProcessByPid(dos_status[i].pid);
	// 			if (p)
	// 			{
	// 				p->status = TASK_OVER;
	// 				removeTaskList(dos_status[i].pid);
	// 			}
	// 		}
	// 	}


	TASK_LIST_ENTRY* prev = gTasksListPtr;

	TASK_LIST_ENTRY* next = (TASK_LIST_ENTRY*)gTasksListPtr->list.next;
	do
	{
		if (next == 0 || next == prev)
		{
			return FALSE;
		}

		if (next->process->status != TASK_RUN)
		{
			next = (TASK_LIST_ENTRY*)next->list.next;
			continue;
		}

		if (next->process->sleep)
		{
			next->process->sleep--;
		}

		if (next->process->sleep)
		{
			next = (TASK_LIST_ENTRY*)next->list.next;
			continue;
		}
		else {
			break;
		}
	} while (1);

	gTasksListPtr = (TASK_LIST_ENTRY*)next;

	if (prev->process->tid == gTasksListPtr->process->tid)
	{
		//return 0;
	}

	LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	process->tss.eax = env->eax;
	process->tss.ecx = env->ecx;
	process->tss.edx = env->edx;
	process->tss.ebx = env->ebx;
	process->tss.esp = env->esp;
	process->tss.ebp = env->ebp;
	process->tss.esi = env->esi;
	process->tss.edi = env->edi;
	process->tss.ss = env->ss;
	process->tss.gs = env->gs;
	process->tss.fs = env->fs;
	process->tss.ds = env->ds;
	process->tss.es = env->es;
	DWORD dwcr3 = 0;
	__asm {
		mov eax,cr3
		mov dwcr3,eax
	}
	process->tss.cr3 = dwcr3;


	if ((env->cs & 3) || (env->eflags&0x23000)) {
		process->espBak = env->esp;
	}

	// 	if (process->tss.link)
	// 	{
	// 		char szout[1024];
	// 		__printf(szout, "tid:%d pid:%d link:%d\r\n", process->tid, process->pid, process->tss.link);
	// 		__drawGraphChars((unsigned char*)szout, 0);
	// 		process->tss.link = 0;
	// 	}

		//切换到新任务的cr3和ldt会被自动加载，但是iret也会加载cr3和ldt，因此不需要手动加载
	// 	DWORD nextcr3 = gTasksListPtr->process->tss.cr3;
	// 	LPTSS timertss = (LPTSS)gAsmTsses;
	// 	timertss->cr3 = nextcr3;
	// 	short ldt = ((DWORD)glpLdt - (DWORD)glpGdt);
	// 	__asm {
	// 		mov ax,ldt
	// 		lldt ax
	// 	}

		//if (prev->process->status == TASK_RUN && process->status == TASK_RUN)
	{
		process->counter++;
		__memcpy((char*)(tss + prev->process->tid), (char*)process, sizeof(PROCESS_INFO));
	}

	//if (gTasksListPtr->process->status == TASK_RUN)
	{
		__memcpy((char*)process, (char*)(gTasksListPtr->process->tid + tss), sizeof(PROCESS_INFO));
	}

	env->eax = process->tss.eax ;
	env->ecx = process->tss.ecx ;
	env->edx = process->tss.edx ;
	env->ebx = process->tss.ebx ;
	env->esp = process->tss.esp ;
	env->ebp = process->tss.ebp ;
	env->esi = process->tss.esi ;
	env->edi = process->tss.edi ;
	env->ss = process->tss.ss ;
	env->gs = process->tss.gs ;
	env->fs = process->tss.fs ;
	env->ds = process->tss.ds ;
	env->es = process->tss.es ;

	dwcr3 = process->tss.cr3;
	__asm {
		mov eax, dwcr3
		mov cr3, eax
	}
	if (process->level & 3) {
		env->esp = process->espBak;

	}

	//tasktest();

	char* fenvprev = (char*)FPU_STATUS_BUFFER + (prev->process->tid << 9);
	//If a memory operand is not aligned on a 16-byte boundary, regardless of segment
	//The assembler issues two instructions for the FSAVE instruction (an FWAIT instruction followed by an FNSAVE instruction), 
	//and the processor executes each of these instructions separately.
	//If an exception is generated for either of these instructions, the save EIP points to the instruction that caused the exception.
	__asm {
		clts			//before all fpu instructions
		fwait
		mov eax, fenvprev
		FxSAVE[eax]
		//fsave [fenv]
		//FNCLEX
	}
	prev->process->tss.fpu = 1;

	if (gTasksListPtr->process->tss.fpu)
	{
		char* fenvnext = (char*)FPU_STATUS_BUFFER + (gTasksListPtr->process->tid << 9);
		__asm {
			clts
			fwait
			finit
			mov eax, fenvnext
			//frstor [fenv]
			fxrstor[eax]
		}
	}

	return TRUE;
}
#endif


void tasktest(TASK_LIST_ENTRY *gTasksListPtr, TASK_LIST_ENTRY*gPrevTasksPtr) {
	static int gTestFlag = 0;
	if (gTestFlag >= 0 && gTestFlag <= -1)
	{
		char szout[1024];
		//TSS* procinfo = (TSS*)gAsmTsses;
		//__printf(szout, "clock tick tss link:%x,eflags:%x\r\n", procinfo->link, procinfo->eflags);
		//__drawGraphChars((unsigned char*)szout, 0);

		__printf(szout,
			"saved  cr3:%x,pid:%x,name:%s,level:%u,esp0:%x,ss0:%x,eip:%x,cs:%x,esp3:%x,ss3:%x,eflags:%x,link:%x,\r\n"
			"loaded cr3:%x,pid:%x,name:%s,level:%u,esp0:%x,ss0:%x,eip:%x,cs:%x,esp3:%x,ss3:%x,eflags:%x,link:%x.\r\n\r\n",
			gPrevTasksPtr->process->tss.cr3, gPrevTasksPtr->process->pid, gPrevTasksPtr->process->filename, gPrevTasksPtr->process->level,
			gPrevTasksPtr->process->tss.esp0, gPrevTasksPtr->process->tss.ss0, gPrevTasksPtr->process->tss.eip, gPrevTasksPtr->process->tss.cs,
			gPrevTasksPtr->process->tss.esp, gPrevTasksPtr->process->tss.ss, gPrevTasksPtr->process->tss.eflags, gPrevTasksPtr->process->tss.link,

			gTasksListPtr->process->tss.cr3, gTasksListPtr->process->pid, gTasksListPtr->process->filename, gTasksListPtr->process->level,
			gTasksListPtr->process->tss.esp0, gTasksListPtr->process->tss.ss0, gTasksListPtr->process->tss.eip, gTasksListPtr->process->tss.cs,
			gTasksListPtr->process->tss.esp, gTasksListPtr->process->tss.ss, gTasksListPtr->process->tss.eflags, gTasksListPtr->process->tss.link
		);
		__drawGraphChars((unsigned char*)szout, 0);
		gTestFlag++;
	}
}