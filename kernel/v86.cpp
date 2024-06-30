#include "v86.h"
#include "kernel.h"

void v86VideoMode(int mode) {
	DWORD* param = (DWORD*)gV86VMParam;
	param[0] = 0x10;
	param[1] = 0x4f02;
	param[4] = 0x4000 + mode;		//bx
	__asm {
		int 0xff
	}
}


void v86ReadSector(int sectorno) {

}


void v86WriteSector(int sectorno) {

}