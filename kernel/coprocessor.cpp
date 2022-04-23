#include "coprocessor.h"
#include "process.h"
#include "task.h"
#include "Utils.h"
#include "video.h"
#include "hpet.h"

int gFpuStatus = 0;


void initCoprocessor() {

	enableIRQ13();

	__asm {
		mov eax,cr0
		or eax,0x10
		mov cr0,eax

		//mov eax,cr4
		__emit 0x0f
		__emit 0x20
		__emit 0xe0

		or eax,0x40600
		//mov cr4,eax
		__emit 0x0f
		__emit 0x22
		__emit 0xe0

		clts
		fwait
		finit
	}
}


//EM = 1,all float instruction exception
//MP = 1 && TS =1,fwait exception
//MP=1 or TS = 1,float instruction exception
void __kCoprocessor() {
	
// 	char szout[1024];
// 	__printf(szout, "coprocessor exceiton\n");
// 	__drawGraphChars((unsigned char*)szout, 0);

	if (gFpuStatus == 0)
	{
		__asm {
			clts
			fwait
			finit
			//load_mxcsr(0x1f80)
		}
	}
	else {
		LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
		char * fenv = (char*)FPU_STATUS_BUFFER + (tss->tid << 9);
		__asm {
			clts
			fwait
			finit
			mov eax,fenv
			//frstor [fenv]
			fxrstor [eax]
		}
	}

	gFpuStatus++;
	if (gFpuStatus == 0xffffffff)
	{
		gFpuStatus = 1;
	}
}