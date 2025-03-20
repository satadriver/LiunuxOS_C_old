
#include "video.h"
#include "Utils.h"
#include "exception.h"
#include "task.h"
#include "process.h"
#include "Pe.h"
#include "memory.h"
#include "Thread.h"
#include "hardware.h"
#include "vectorRoutine.h"
#include "cmosPeriodTimer.h"
#include "cmosAlarm.h"
#include "cmosExactTimer.h"
#include "ata.h"
#include "coprocessor.h"





__declspec(naked) void DivideError(LIGHT_ENVIRONMENT* stack) {
	__asm {
		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss,ax
	}
	{
		__kException((const char*)"DivideError", 0, stack);
	}

	__asm {

		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

//caution here!!!!!

		iretd
	}

}

//NMIs occur for RAM errors and unrecoverable hardware problems. 
//For newer computers these things may be handled using machine check exceptions and/or SMI. 
//For the newest chipsets (at least for Intel) there's also a pile of TCO stuff ("total cost of ownership") 
//that is tied into it all (with a special "TCO IRQ" and connections to SMI/SMM, etc).




//When the CPU is in Protected Mode, System Management Mode (SMM) is still invisibly active, 
//and cannot be shut off. SMM also seems to use the EBDA. So the EBDA memory area should never be overwritten.

//Note: the EBDA is a variable - sized memory area(on different BIOSes).If it exists, 
//it is always immediately below 0xA0000 in memory.It is absolutely guaranteed to be at most 128 KiB in size.
//Older computers typically uses 1 KiB from 0x9FC00 - 0x9FFFF, modern firmware can be found using significantly more.
//You can determine the size of the EBDA by using BIOS function INT 12h, or by examining the word at 0x413 in the BDA(see below).
//Both of those methods will tell you how much conventional memory is usable before the EBDA

//The CMOS RTC expects a read from or write to the data port 0x71 after any write to index port 0x70 or it may go into an undefined state.
//There may also need to be an I/O delay between accessing the index and data registers. 
//The index port 0x70 may be a write-only port and always return 0xFF on read. 
//Hence the bit masking below to preserve bits 0 through 6 of the CMOS index register may not work, 
//nor may it be possible to retrieve the current state of the NMI mask from port 0x70.

/*
System Control Port A (0x92) layout:

BIT	Description
0	Alternate hot reset
1	Alternate gate A20
2	Reserved
3	Security Lock
4*	Watchdog timer status
5	Reserved
6	HDD 2 drive activity
7	HDD 1 drive activity
System Control Port B (0x61)

Bit	Description
0	Timer 2 tied to speaker
1	Speaker data enable
2	Parity check enable
3	Channel check enable
4	Refresh request
5	Timer 2 output
6*	Channel check
7*	Parity check
*/

void __declspec(naked) NmiInterrupt(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss,ax

	}
	{
		int v1 = inportb(0x92);

		int v2 = inportb(0x61);

		int v3 = inportb(0x70);

		LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
		LPPROCESS_INFO current = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

		char szout[1024];
		__printf(szout, "NMI interruption 0x61 port:%x, 0x92 port:%x,0x70 port:%x\r\n", v1, v2,v3);
		//__kException((const char*)"NmiInterrupt", 2, stack);

	}
	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}

}

//�Ƿ��ܴ�����Ҫ�� eflags �Ĵ����е� OF λ�Ƿ�Ϊ 1������Ϊ 1����ֱ�����ӡ�
//0xCE(INTO):Generate overflow trap if overflow flag is 1.
void __declspec(naked) OverflowException(LIGHT_ENVIRONMENT* stack) {
	__asm {
		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		//__kException((const char*)"OverflowException", 4, stack);

		LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
		LPPROCESS_INFO current = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

		char szout[1024];
		__printf(szout, "OverflowException eip:%x,cs:%x,pid:%d,tid:%d\r\n", stack->eip,stack->cs, current->pid, current->tid);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}



void __declspec(naked) BoundRangeExceed(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"BoundRangeExceed", 5, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}



void __declspec(naked) UndefinedOpcode(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		__kCoprocessor();
		//__kException((const char*)"UndefinedOpcode", 6, stack);

	}

	__asm {

		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}


void __declspec(naked) DeviceUnavailable(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX

		mov ss, ax
	}

	__kCoprocessor();

	{
		//__kException((const char*)"DeviceUnavailable", 7, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}


//A double fault will always generate an error code with a value of zero.
//A Double Fault occurs when an exception is unhandled or when an exception occurs while the CPU is trying to call an exception handler. 
void __declspec(naked) DoubleFault(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4

		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"DoubleFault", 8, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp, 4
		iretd
	}
}

void __declspec(naked) CoprocSegOverrun(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax

		clts
		fnclex
		FNINIT
	}

	{
		__kException((const char*)"CoprocSegOverrun", 9, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) InvalidTss(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"InvalidTss", 10, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp, 4

		clts
		iretd
		jmp InvalidTss
	}
}

void __declspec(naked) SegmentUnpresent(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"SegmentUnpresent", 11, stack);

	}

	__asm {

		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp, 4
		iretd
	}
}

void __declspec(naked) StackSegFault(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"StackSegFault", 12, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp, 4
		iretd
	}
}

void __declspec(naked) GeneralProtection(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"GeneralProtection", 13, stack);

	}

	__asm {

		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp, 4
		iretd
	}
}

void __declspec(naked) PageFault(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax

	}
	{
		DWORD regcr2 = 0;
		__asm {
			mov eax, cr2
			mov regcr2,eax
		}

		__kException((const char*)"PageFault", 14, stack);

	}

	__asm {

		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp, 4
		iretd
	}
}


void __declspec(naked) AnonymousException(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4

		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"AnonymousException", 15, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}


void __declspec(naked) FloatPointError(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	__kCoprocessor();

	{
		//__kException((const char*)"FloatPointError", 16, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}





void __declspec(naked) AlignmentCheck(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"AlignmentCheck", 17, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp,4
		iretd
	}
}


void __declspec(naked) MachineCheck(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"MachineCheck", 18, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

//The SIMD Floating-Point Exception occurs when an unmasked 128-bit media floating-point exception occurs and the CR4.
//OSXMMEXCPT bit is set to 1. If the OSXMMEXCPT flag is not set, 
//then SIMD floating-point exceptions will cause an Undefined Opcode exception instead of this.
__declspec(naked) void SIMDException(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"SIMDException", 19, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

__declspec(naked) void VirtualizationException(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"VirtualizationException", 20, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}


__declspec(naked) void CtrlProtectException(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"CtrlProtectException", 21, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp,4
		iretd
	}
}



__declspec(naked) void HypervisorInjectException(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"HypervisorInjectException", 28, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}


__declspec(naked) void VMMCommException(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"VMMCommException", 29, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp, 4
		iretd
	}
}

__declspec(naked) void SecurityException(LIGHT_ENVIRONMENT* stack) {
	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}
	{
		__kException((const char*)"SecurityException", 30, stack);

	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		add esp,4
		iretd
	}
}


extern "C" void __declspec(naked) TimerInterrupt(LIGHT_ENVIRONMENT * stack) {

	__asm {
		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax

		//clts
		cli
	}

	{
		LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
		char szout[1024];
		//__printf(szout,"TimerInterrupt\r\n");

		__kTaskSchedule((LIGHT_ENVIRONMENT*)stack);

		outportb(0x20, 0x20);

	}

	__asm {
#ifdef SINGLE_TASK_TSS
		mov eax, dword ptr ds: [CURRENT_TASK_TSS_BASE + PROCESS_INFO.tss.cr3]
		mov cr3, eax
#endif

		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad
#ifdef SINGLE_TASK_TSS
		nop
		mov esp, ss: [esp - 20]
		nop
#endif	
		//clts
		sti

		iretd

		jmp TimerInterrupt
	}
}

//Prefixes ָ������ǰ׺��ÿָ������ܹ���4��/��ǰ׺���Ρ�
//�в�������Сǰ׺����ǰ���ᵽ�� 66����ָ�� 32 - bit ��������С��FE ǰ׺���ʾ 8 - bit ���������� 16 λ���ϵĴ�ͳ��Ĭ�ϵĲ���Ϊ 16 λ����ʹ��ǰ׺��
//�з�������ǰ׺��������� F3 ��ʾ REP��REPE��REPZ ����ǰ׺������ F2 ��ʾ REPNE��REPNZ ǰ׺��
//�жγ�Խǰ׺��2E ��Ӧ CS��36 ��Ӧ SS��3E ��Ӧ DS��26 ��Ӧ ES��64��65 ����Ӧ FS��GS���γ�Խ�ǿ��ڴ�Ѱַ�йص����ݡ�
//����Ѱַ��ַ��Сǰ׺��67 ��ʾ 32 - bit �ڴ�Ѱַ���Լ��ٷ��փ����ἰ��һЩ�ر��ܵ�ǰ׺����Щǰ׺�ܹ��������˳����ָ������ϣ�






void __declspec(naked) Com2IntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp,4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "Com2IntProc!\r\n");
		outportb(0x20, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp,4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) Com1IntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "Com1IntProc!\r\n");

		outportb(0x20, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) Parallel2IntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "Parallel2IntProc!\r\n");
		outportb(0x20, 0x20);

		int v = inportb(0x278 + 1);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) FloppyDiskIntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "FloppyDiskIntProc!\r\n");
		outportb(0x20, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) Parallel1IntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax

		
	}

	{
		char szout[1024];
		__printf(szout, "Parallel1IntProc!\r\n");
		outportb(0x20, 0x20);
		int v = inportb(0x378 + 1);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}



extern "C" void __declspec(naked) CmosInterrupt(LIGHT_ENVIRONMENT * stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		outportb(0x70, 0x0c);

		int flag = inportb(0x71);
		//IRQF = (PF * PIE) + (AF * AIE) + (UF * UFE), if double interruptions, will not be 1
		if (flag & 0x20) {
			__kAlarmTimerProc();
		}
		else if (flag & 0x40) {
			__kExactTimerProc();
		}
		else if (flag & 0x10) {
			__kPeriodTimer();
		}

		outportb(0x20, 0x20);		//ocw2
		outportb(0xa0, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) SlaveIntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "SlaveIntProc!\r\n");
		outportb(0x20, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}


void __declspec(naked) Slave1IntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "Slave1IntProc!\r\n");
		outportb(0x20, 0x20);
		outportb(0xa0, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) NetcardIntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "NetcardIntProc!\r\n");
		outportb(0x20, 0x20);
		outportb(0xa0, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) USBIntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];
		__printf(szout, "USBIntProc!\r\n");
		outportb(0x20, 0x20);
		outportb(0xa0, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) CoprocessorIntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax

		clts
		fnclex
		fninit
	}

	{
		char szout[1024];
		__printf(szout, "CoprocessorIntProc!\r\n");
		outportb(0x20, 0x20);
		outportb(0xa0, 0x20);

		outportb(0xf0, 0xf0);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

//��������ȡһ���������Զ�����״̬�Ĵ���1F7H��DRQ��������λ�������BSYλæ�źš� 
//DRQλ֪ͨ�������ڿ��Դӻ������ж�ȡ512�ֽڻ��������ݣ�ͬʱ��������INTRQ�ж������ź�
// A sectorcount of 0 means 256 sectors = 128K.
//One way of trying to reduce the number of IRQs in multitasking PIO mode is to use the READ MULTIPLE (0xC4), 
//and WRITE MULTIPLE (0xC5) commands. These commands make the drive buffer "blocks" of sectors, 
//and only send one IRQ per block, rather than one IRQ per sector. 
//it is necessary to read the Regular Status Register once, to make the disk clear its interrupt flag. 
//If the ERR bit in the Status Register is set (bit 0, value = 1),
//you may want to read and save the "error details" value from the Error IO port (0x1F1 on the Primary bus).
void __declspec(naked) IDEMasterIntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];

		outportb(0x376, 0);	//IRQ14
		
		int sta = inportb(0x376);	//IRQ14
		
		int status = inportb(gAtaBasePort + 7) ;
		int err = inportb(gAtaBasePort + 1);
		//below 2 line codes why can not be removed?
		int high = inportb(gAtaBasePort + 2);
		int low = inportb(gAtaBasePort + 2);
		int size = (high << 8) | low;

		LPPROCESS_INFO proc = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
		//__printf(szout, "IDEMasterIntProc size:%x tid:%d port:%x status:%x\r\n", size,proc->tid,gAtaBasePort+7,status);

		outportb(0x20, 0x20);
		outportb(0xa0, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}

void __declspec(naked) IDESlaveIntProc(LIGHT_ENVIRONMENT* stack) {

	__asm {

		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
		mov ss, ax
	}

	{
		char szout[1024];

		int sta = inportb(0x3f6); //IRQ15
		
		outportb(0x3f6, 0); //IRQ15
		
		int status = inportb(gAtapiBasePort + 7);
		int err = inportb(gAtapiBasePort + 1);
		//below 2 line codes why can not be removed?
		int low = inportb(gAtapiBasePort + 4);
		int high = inportb(gAtapiBasePort + 5);
		int size = (high << 8) | low;
		LPPROCESS_INFO proc = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
		__printf(szout, "IDESlaveIntProc size:%x tid:%d port:%x status:%x\r\n",size, proc->tid, gAtapiBasePort + 7, status);

		outportb(0x20, 0x20);
		outportb(0xa0, 0x20);
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp

		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		iretd
	}
}