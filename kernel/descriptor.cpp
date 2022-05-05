#include "descriptor.h"
#include "def.h"
#include "process.h"
#include "Utils.h"
#include "Pe.h"
#include "slab.h"
#include "page.h"
#include "video.h"
#include "Utils.h"
#include "Kernel.h"



void initLdt(LPSEGDESCRIPTOR lpldt) {
	//return;

	lpldt->attr = 0xe2;
	lpldt->baseHigh = (unsigned char)(LDT_BASE >> 24);
	lpldt->baseLow = (unsigned short)(LDT_BASE & 0xffff);
	lpldt->baseMid = (unsigned char)(LDT_BASE >> 16);
	lpldt->gd0a_lh = 0;
	lpldt->limitLow = 0x27;

	LPSEGDESCRIPTOR selectors = (LPSEGDESCRIPTOR)LDT_BASE;

	selectors[0].baseHigh = 0;
	selectors[0].baseMid = 0;
	selectors[0].baseLow = 0;
	selectors[0].attr = 0;
	selectors[0].gd0a_lh = 0;
	selectors[0].limitLow = 0;

	selectors[1].baseHigh = 0;
	selectors[1].baseMid = 0;
	selectors[1].baseLow = 0;
	selectors[1].attr = 0x9a;
	selectors[1].gd0a_lh = 0xcf;
	selectors[1].limitLow = 0xffff;


	selectors[2].baseHigh = 0;
	selectors[2].baseMid = 0;
	selectors[2].baseLow = 0;
	selectors[2].attr = 0x92;
	selectors[2].gd0a_lh = 0xcf;
	selectors[2].limitLow = 0xffff;

	selectors[3].baseHigh = 0;
	selectors[3].baseMid = 0;
	selectors[3].baseLow = 0;
	selectors[3].attr = 0xfa;
	selectors[3].gd0a_lh = 0xcf;
	selectors[3].limitLow = 0xffff;

	selectors[4].baseHigh = 0;
	selectors[4].baseMid = 0;
	selectors[4].baseLow = 0;
	selectors[4].attr = 0xf2;
	selectors[4].gd0a_lh = 0xcf;
	selectors[4].limitLow = 0xffff;

	unsigned short ldtno = (unsigned short)((DWORD)lpldt - (DWORD)glpGdt);
	__asm {
		movzx eax, ldtno
		lldt ax
	}

	// 	char szout[1024];
	// 	int len = __printf(szout, "ldt selector:%d,base:%x\r\n", ldtno, lpldt);
	// 	__drawGraphChars((unsigned char*)szout, 0);
}

extern "C" __declspec(naked) void __kCallGateProc() {
	DWORD *params;
	DWORD paramLen;

	//ebp
	//eip3
	//cs3
	//param1
	//param2
	//esp3
	//ss3

	//思考编译器对局部变量的定义过程
	__asm {
		pushad
		pushfd
		mov ebp,esp
		sub esp,0x1000

		mov eax,[ebp + 36 + 8]
		mov params,eax
		mov eax,[ebp + 36 + 12]
		mov paramLen,eax
	}

// 	char szout[1024];
// 	__printf(szout, "__kCallGateProc running,param1:%x,param2:%x\r\n", params,paramLen);
// 	__drawGraphChars((unsigned char*)szout, 0xff0000);

	//[bits 16] iret ;编译后的机器码为CF 
	//iretd ;编译后的机器码为66 CF
	//C2 RET immed16
	//C3 RET
	//cb retf
	//cf iret
	__asm {
		mov esp, ebp
		popfd
		popad

		retf 0x08		//ca 08 00
	}
	//机器码对应表：
	//https://defuse.ca/online-x86-assembler.htm#disassembly
}

void initCallGate(LPSYSDESCRIPTOR lpcg) {
	lpcg->attr = 0xec;
	lpcg->paramCnt = 2;
	lpcg->selector = KERNEL_MODE_CODE;

	lpcg->addrHigh = (DWORD)__kCallGateProc >> 16;
	lpcg->addrLow = (DWORD)__kCallGateProc &0xffff;
}



extern "C" __declspec(dllexport) void callgateEntry(DWORD * params,DWORD paramLen) {

	//__drawGraphChars((unsigned char*)"callgateEntry entry\r\n", 0);

	DWORD seg = (DWORD)glpCallGate - (DWORD)glpGdt;
	__asm {
		
		push paramLen
		push params

		_emit 0x9a
		_emit 0
		_emit 0
		_emit 0
		_emit 0
		_emit 0x80
		_emit 0

		add esp,8
	}

	//__drawGraphChars((unsigned char*)"callgateEntry leave\r\n", 0);

// 	CALL_LONG calllong;
// 	calllong.callcode = 0x9a;
// 	calllong.seg = seg;
// 	calllong.offset = (DWORD)__kCallGateProc;
// 	__asm {
// 		lea eax, calllong
// 		jmp eax
// 	}
}

void initDescriptor() {

}






//Extended Feature Enable Register(EFER) is a model - specific register added in the AMD K6 processor, 
//to allow enabling the SYSCALL / SYSRET instruction, and later for entering and exiting long mode.
//This register becomes architectural in AMD64 and has been adopted by Intel.Its MSR number is 0xC0000080.
void initEfer() {
	DWORD highpart,lowpart;
	readmsr(0xC0000080, &lowpart, &highpart);

	//readmsr(0x1f80, &highpart, &lowpart);

	__asm {
		mov eax,[lowpart]
		or eax,0x4001
		mov [lowpart],eax
	}

	//writemsr(0xC0000080, lowpart, highpart);
}




void readmsr(DWORD no, DWORD *lowpart, DWORD * highpart) {
	__asm {
		xor eax, eax
		xor edx, edx

		mov ecx, no
		rdmsr

		mov ecx, lowpart
		mov[ecx], eax

		mov ecx, highpart
		mov[ecx], edx
	}

	char szout[1024];
	__printf(szout, "msr:%x,high:%x,low:%x\r\n", no, *highpart, *lowpart);
	__drawGraphChars((unsigned char*)szout, 0);
}

void writemsr(DWORD no, DWORD lowpart, DWORD highpart) {
	__asm {
		mov ecx, no

		mov eax, lowpart

		mov edx, highpart

		wrmsr
	}
}

void syscall() {

}

void sysleave() {

}

int sysenterInit(DWORD entryaddr) {
	LPTSS tss = (LPTSS)CURRENT_TASK_TSS_BASE;
	if (tss->cs & 3)
	{
		return -1;
	}

	DWORD csseg = KERNEL_MODE_CODE;

	DWORD high = 0;

	writemsr(0x174, csseg, high);

	DWORD esp0 = SYSCALL_STACK0 - STACK_TOP_DUMMY;

	writemsr(0x175, esp0, high);

	DWORD eip = (DWORD)entryaddr;

	writemsr(0x176, eip, high);

	return 0;
}




DWORD g_sysenterInitFlag = 0;

DWORD * g_sysenterParams = 0;
DWORD g_sysenterParamsLen = 0;

DWORD g_sysenterStack3 = 0;
DWORD g_sysenterEip3 = 0;




void sysenterProc(DWORD * params, DWORD paramslen) {
	LPTSS tss = (LPTSS)CURRENT_TASK_TSS_BASE;

	WORD rcs = 0;
	DWORD resp = 0;
	WORD rss = 0;
	__asm {
		mov ax, cs
		mov rcs, ax

		mov ax, ss
		mov rss, ax

		mov resp, esp
	}
	char szout[1024];
	__printf(szout, "sysenterProc current cs:%x,tss cs:%x,ss:%x,esp:%x\r\n", rcs, tss->cs, rss, resp);
	__drawGraphChars((unsigned char*)szout, 0);
}

//only be invoked in ring3,in ring0 will cause exception 0dh
extern "C" __declspec(dllexport) void sysenterEntry(DWORD * params, DWORD paramslen) {

	//__drawGraphChars((unsigned char*)"sysenterProc entry\r\n", 0);

	g_sysenterParams = params;
	g_sysenterParamsLen = paramslen;

	__asm {

		cmp dword ptr ds : [g_sysenterInitFlag], 0
		jnz __sysenterInitFlagOK

		lea eax, __sysenterEntry
		push eax
		call sysenterInit
		add esp, 4
		cmp eax, 0
		jnz __sysenterExit
		mov dword ptr ds : [g_sysenterInitFlag], 1
		jmp __sysenterExit

		__sysenterInitFlagOK :

		mov ax, cs
			test ax, 3
			jz __sysenterExit

			mov ds : [g_sysenterStack3], esp
			lea eax, __sysenterExit
			mov ds : [g_sysenterEip3], eax


			_emit 0x0f
			_emit 0x34

			__sysenterEntry :

			mov eax, ds : [g_sysenterParamsLen]
			push eax
			mov eax, ds : [g_sysenterParams]
			push eax
			call sysenterProc
			add esp, 8

			sti

			mov edx, ds:[g_sysenterEip3]
			mov ecx, ds : [g_sysenterStack3]
			_emit 0x0f
			_emit 0x35

			__sysenterExit :
	}

	//__drawGraphChars((unsigned char*)"sysenterProc leave\r\n", 0);
}

