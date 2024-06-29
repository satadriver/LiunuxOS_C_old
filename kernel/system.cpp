#include "def.h"
#include "system.h"
#include "Utils.h"
#include "video.h"
#include "satadriver.h"
#include "task.h"



DWORD getCurrentCr3() {
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	return process->tss.cr3;
}

void saveScreen() {

	LPV86VMIPARAMS params = (LPV86VMIPARAMS)V86VMIPARAMS_ADDRESS;

	while (params->bwork == 1)
	{
		__sleep(0);
	}

	params->intno = 0x10;
	params->reax = 0x4f04;
	//cx:
	//d0:hardware
	//d1:bios
	//d2:dac
	//d3:register
	params->recx = 1;
	params->redx = 1;
	params->rebx = VESA_STATE_OFFSET;
	params->resi = 0;
	params->redi = 0;
	params->res = VESA_STATE_SEG;
	params->rds = 0;
	params->result = 0;

	params->bwork = 1;

	while (params->bwork == 1)
	{
		__sleep(0);
	}
}


void restoreScreen() {
	LPV86VMIPARAMS params = (LPV86VMIPARAMS)V86VMIPARAMS_ADDRESS;
	while (params->bwork == 1)
	{
		__sleep(0);
	}

	params->intno = 0x10;
	params->reax = 0x4f04;
	params->recx = 1;
	params->redx = 2;
	params->rebx = VESA_STATE_OFFSET;
	params->resi = 0;
	params->redi = 0;
	params->res = VESA_STATE_SEG;
	params->rds = 0;
	params->result = 0;

	params->bwork = 1;

	while (params->bwork == 1)
	{
		__sleep(0);
	}
}

int setGraphMode(int mode) {

	LPV86VMIPARAMS params = (LPV86VMIPARAMS)V86VMIPARAMS_ADDRESS;
	while (params->bwork == 1)
	{
		__sleep(0);
	}

	params->intno = 0x10;
	params->reax = 0x4f02;
	params->recx = 0;
	params->redx = 0;
	params->rebx = mode;
	params->resi = 0;
	params->redi = 0;
	params->res = 0;
	params->rds = 0;
	params->result = 0;

	params->bwork = 1;

	while (params->bwork == 1)
	{
		__sleep(0);
	}
	if (params->result)
	{
		return FALSE;
	}

	return TRUE;
}


int setVideoMode(int mode) {
	DWORD params[2];
	params[0] = 0x4f02;
	params[1] = mode;
	__asm {
		lea edi, params
		mov eax, 15
		int 80h
	}
}


