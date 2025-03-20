#include "floppy.h"
#include "hardware.h"
#include "video.h"
#include "Utils.h"
#include "task.h"
#include "device.h"

floppy_struct floppy_type ={ 2880,18,2,80,0,0x1B,0x00,0xCF }; /* 1.44MB diskette */


//80 18 2
int getCylinder(int lba) {
	return lba / (SPT * HPC);
}

int getHeader(int lba) {
	return  (lba / SPT) % HPC;
}

int getSector(int lba) {
	return lba % SPT + 1;
}


void wait_status() {
	int cnt = 0;
	do {
		__delay();

		int v = inportb(0x3f4);
		if (v & 80) {
			break;
		}
		else {
			__sleep(0);
		}
		cnt++;
	} while (cnt < 3);
}

//https://0cch.com/2013/09/21/floppy-disk-controllere7bc96e7a88b/



extern "C"  __declspec(dllexport)int readFloppySector(int dev,DWORD buf,int secnum,int seccnt) {
	int v = 0;

	int cylinder = getCylinder(secnum);
	int header = getHeader(secnum);
	int sector = getSector(secnum);

	int size = HPC * BYTES_PER_SECTOR * seccnt - 1;

	outportb(0x0d, 0xff);
	outportb(0x0a, 6);
	outportb(0x0c, 0xff);
	outportb(0x04, buf & 0xff);
	outportb(0x04, (buf >>8) & 0xff);
	outportb(0x81, (buf >> 16) & 0xff);
	outportb(0x0c, 0xff);
	outportb(0x5, size && 0xff);
	outportb(0x5, (size >> 8)&0xff);
	outportb(0xa, 0x2);

	outportb(0x3f2, 0x0);
	outportb(0x3f2, 0x0c);

	__sleep(0);

	for (int i = 0; i < 4; i++) {
		wait_status();
		outportb(0x3f5, 0x8);
		wait_status();
		v = inportb(0x3f5);
		wait_status();
		v = inportb(0x3f5);
		wait_status();
	}

	outportb(0x3f7, 0x0);

	wait_status();
	outportb(0x3f5, 3);
	wait_status();
	outportb(0x3f5, 0x0df);
	wait_status();
	outportb(0x3f5, 2);
	wait_status();

	outportb(0x3f2, 0x1c);
	wait_status();

	outportb(0x3f5, 7);
	wait_status();
	outportb(0x3f5, dev);
	wait_status();

	outportb(0x3f5, 8);
	wait_status();
	v = inportb(0x3f5);
	wait_status();
	v = inportb(0x3f5);

	outportb(0x3f2, 4);

	outportb(0x3f2, 0x1c);
	wait_status();

	outportb(0x3f5, 0xf);
	wait_status();
	outportb(0x3f5, 0);
	wait_status();
	outportb(0x3f5, 0);
	wait_status();

	outportb(0x3f5, 8);
	wait_status();
	v = inportb(0x3f5);
	wait_status();
	v = inportb(0x3f5);

	outportb(0x0a, 6);
	outportb(0x0b, 0x46);
	outportb(0x0a, 2);

	wait_status();
	outportb(0x3f5, 0xe6);
	wait_status();
	outportb(0x3f5, 0);
	wait_status();
	outportb(0x3f5, cylinder);
	wait_status();
	outportb(0x3f5, header);
	wait_status();
	outportb(0x3f5, sector);
	wait_status();
	outportb(0x3f5, HPC);
	wait_status();
	outportb(0x3f5, seccnt);
	wait_status();
	outportb(0x3f5, 0x1b);
	wait_status();
	outportb(0x3f5, 0xff);
	wait_status();

	__sleep(0);

	for (int i = 0; i < 7; i++) {
		wait_status();
		v = inportb(0x3f5);
	}

	wait_status();
	outportb(0x3f5, 8);
	wait_status();
	v = inportb(0x3f5);
	wait_status();
	v = inportb(0x3f5);
	wait_status();
	outportb(0x3f2, 4);
	return TRUE;
}



void __declspec(naked) FloppyIntProc(LIGHT_ENVIRONMENT* stack) {

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
	}

	{
		int v1 = inportb(0x3f5);
		int v2 = inportb(0x3f2);
		int v3 = inportb(0x3f4);

		char szout[1024];
		__printf(szout, "FloppyIntProc!\r\n");

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


/*
__asm {
		; MASM�ĺ�Ӧ�ò�İ���ɣ��Ͳ��������ˡ�
	outb macro port:req, b:req
		mov dx, port
		mov al, b
		out dx, al
	endm

	inb macro port:req
		mov dx, port
		in al, dx
	endm

	wait_status macro
		inb 3f4h
	@@:
		test al, 80h
		jz @B
	endm

	; ISA DMA ��ʼ�����֣� ����ISA DMA������ƪ���µ��ص�
	; ��������ֻ˵����ŵ���;��
	outb 0dh, 0ffh      ; ����DMA������
	outb 0ah, 6h        ; ѡ������2��DMAͨ��
	outb 0ch, 0ffh      ; ����Flipflop�Ĵ���
	outb 4h, 0h         ; ����DMA�������ַ����Ҫ��������
	outb 4h, 0f0h       ; ���������õ���0xf000
	outb 81h, 0h        ; ���õ�ַ24bit�ĸ�8bitΪ0��Ҳ����0x00f000
	outb 0ch, 0ffh      ; ����Flipflop�Ĵ���
	outb 5h, 0ffh       ; ���������ڴ��С�����СΪLength-1
	outb 5h, 0fh        ; ��������ڴ��С��0x1000����������Ϊ0xfff
	outb 0ah, 2h        ; ѡ�����2��DMAͨ��

	; FDC�ĳ�ʼ������
	outb 3f2h, 0h       ; 1.������������Ĵ���
	outb 3f2h, 0ch
	call WaitIrq

	mov ecx, 4
	check_int:
	wait_status
	outb 3f5h, 8h       ; ����8���������������������������ж�
	wait_status         ; ���ҷ��ؽ���������ظ�4�Σ���Ϊ�����4��������״̬
	inb 3f5h
	wait_status
	inb 3f5h
	wait_status
	loop check_int

	outb 3f7h, 0h       ; 2.���ô����ٶ�Ϊ500kb/s
	wait_status
	outb 3f5h, 3h       ; 3.����FDC��������ʱ���Լ�DMA��
	wait_status
	outb 3f5h, 0dfh
	wait_status
	outb 3f5h, 2h

	outb 3f2h, 1ch      ; ���������綯��
	wait_status
	outb 3f5h, 7h       ; ����У������
	wait_status
	outb 3f5h, 0h       ; ѡ��0������
	wait_status
	outb 3f5h, 8h       ; ��������ж������ý��
	wait_status
	inb 3f5h
	wait_status
	inb 3f5h
	outb 3f2h, 4h       ; �رյ綯��

	; �����������̵Ķ�ȡ����
	outb 3f2h, 1ch      ; �����綯��
	wait_status
	outb 3f5h, 0fh      ; 4.����0fѰ������
	wait_status
	outb 3f5h, 0h
	wait_status
	outb 3f5h, 0h
	wait_status
	outb 3f5h, 8h       ; ��������ж������ý��
	wait_status
	inb 3f5h
	wait_status
	inb 3f5h

	; ����ISA DMAΪ��ȡ
	outb 0ah, 6h
	outb 0bh, 46h
	outb 0ah, 2h

	wait_status
	outb 3f5h, 0e6h     ; 5.���Ͷ�ȡ��������
	wait_status
	outb 3f5h, 0h       ; ���ô�ͷ����������
	wait_status
	outb 3f5h, 0h       ; ���ôŵ�
	wait_status
	outb 3f5h, 0h       ; ���ô�ͷ
	wait_status
	outb 3f5h, 1h       ; ����������
	wait_status
	outb 3f5h, 2h       ; ����������С
	wait_status
	outb 3f5h, 8h       ; ���ö�ȡ��������
	wait_status
	outb 3f5h, 1bh      ; ���ô���Ϊ3.5Ӣ��
	wait_status
	outb 3f5h, 0ffh     ; ���ö�ȡ���ȣ�����0xff

	call WaitIrq

	mov ecx, 7
	loop_ret:
	wait_status
	inb 3f5h
	loop loop_ret

	wait_status
	outb 3f5h, 8h       ; ��������ж������ý��
	wait_status
	inb 3f5h
	wait_status
	inb 3f5h
	outb 3f2h, 4h       ; �رյ綯��
}
*/