#include "sysregs.h"
#include "def.h"
#include "video.h"
#include "Utils.h"
#include "Kernel.h"
#include "task.h"
#include "malloc.h"


int getcrs(char * szout) {

	if (szout)
	{
		*szout = 0;
	}
	else {
		return 0;
	}

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	int dsreg = process->tss.cs;
	if (dsreg & 7)
	{
		__drawGraphChars((unsigned char*)"you have no privilege to get crs\r\n", 0);
		return 0;
	}

	DWORD rcr0 = 0;
	DWORD rcr2 = 0;
	DWORD rcr3 = 0;
	DWORD rcr4 = 0;

	__asm {
		mov eax, cr0
		mov rcr0, eax
		mov eax, cr2
		mov rcr2, eax
		mov eax, cr3
		mov rcr3, eax
		//mov eax, cr4
		//db 0fh, 20h, 0e0h
		__emit 0xf
		__emit 0x20
		__emit 0xe0
		mov rcr4, eax
	}

	int len = __printf(szout, "cr0:%x,cr2:%x,cr3:%x,cr4:%x\n", rcr0, rcr2, rcr3, rcr4);
	return len;
}

int getpids(char * szout) {
	int outlen = 0;
	int len = 0;

	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (list->valid && list->process )
		{
			len = __sprintf(szout + outlen, "filename:%s funcname:%s address:%x pid:%d,tid:%d,level:%d\r\n",
				list->process->filename, list->process->funcname, list->process->moduleaddr, 
				list->process->pid, list->process->tid, list->process->level);
			outlen += len;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

// 	LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
// 	for (int i = 0;i < TASK_LIMIT_TOTAL;i ++)
// 	{
// 		if (tss->status == TASK_RUN)
// 		{
// 			len = __printf(szout + outlen, "task:%d filename:%s funcname:%s address:%x pid:%d,tid:%d,level:%d\r\n",
// 				tss->tid, tss->filename,tss->funcname,tss->moduleaddr, tss->pid,tss->tid, tss->level);
// 			outlen += len;
// 		}
// 		tss++;
// 	}

	return outlen;
}


int getmemmap(int pid,char * szout) {
	return formatProcMem(pid, szout);
}


int getpid(int pid,char * szout) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (list->valid && list->process && list->process->tid == pid)
		{
			int len = __sprintf(szout, "filename:%s funcname:%s address:%x pid:%d,tid:%d,level:%d\r\n",
				list->process->filename, list->process->funcname, list->process->moduleaddr, 
				list->process->pid, list->process->tid, list->process->level);
			return len;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

// 	LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
// 	for (int i = 0; i < TASK_LIMIT_TOTAL; i++)
// 	{
// 		if (tss->tid == pid && tss->status == TASK_RUN)
// 		{
// 			int len = __printf(szout, "task:%d filename:%s funcname:%s address:%x pid:%d,tid:%d,level:%d\r\n",
// 				tss->tid, tss->filename, tss->funcname, tss->moduleaddr, tss->pid,tss->pid, tss->level);
// 			return len;
// 		}
// 		tss++;
// 	}

	*szout = 0;
	return 0;
}

int getGeneralRegs(char * szout) {
	DWORD reax;
	DWORD recx;
	DWORD redx;
	DWORD rebx;
	DWORD rebp, resp, resi, redi;
	DWORD reip;

	DWORD eflags;

	DWORD res, rds, rfs, rgs, rss, rcs;
	__asm {
		mov reax,eax
		mov recx,ecx
		mov redx,edx
		mov rebx,ebx

		mov rebp, ebp
		mov resp, esp
		mov resi, esi
		mov redi, edi
		call _next

		_next:
		mov eax, dword ptr ss : [esp]
		mov reip,eax
		add esp,4

		xor eax,eax
		mov ax,cs
		mov rcs,eax

		mov ax,ds
		mov rds,eax

		mov ax,es
		mov res,eax

		mov ax,ss
		mov rss,eax

		mov ax,fs
		mov rfs,eax

		mov ax,gs
		mov rgs,eax

		pushfd
		pop eax
		mov eflags,eax
	}

	int len = __sprintf(szout, "eax:%x,ecx:%x,edx:%x,ebx:%x,ebp:%x,esp:%x,esi:%x,edi:%x,eflags:%x,ss:%x,es:%x,ds:%x,fs:%x,gs:%x,cs:%x,eip:%x\r\n",
		reax, recx, redx, rebx, rebp, resp, resi, redi, eflags,rss, res, rds, rfs, rgs, rcs, reip);
	return len;
}

int getldt(char * szout) {
	//char strldt[8];

	WORD ldt = 0;

	__asm {
		sldt ax
		mov ldt,ax
		//sldt fword ptr [strldt]
	}

	int len = 0;
	int outlen = 0;

	//int ldtlen = *(WORD*)strldt + 1;
	//DWORD ldtbase = *(DWORD*)(strldt + 2);

	LPSEGDESCRIPTOR ldtbase = (LPSEGDESCRIPTOR)((DWORD)glpGdt + ldt);

	int ldtlen = ldtbase->limitLow + ((ldtbase->gd0a_lh & 0x0f) << 16) + 1;

	__printf(szout, "ldt selector:%d,base:%x,size:%d\r\n",ldt, ldtbase, ldtlen);

	//__int64 * pldts = (__int64*)ldtbase;
	__int64 * pldts = (__int64*)((ldtbase->baseLow) + (ldtbase->baseMid << 16) + (ldtbase->baseHigh << 24));

	int cnt = ldtlen >> 3;

	for (int i = 0; i < cnt; i++)
	{
		len = __sprintf(szout + outlen, "ldt %d:%I64u\n", i, pldts[i]);
		outlen += len;
		//__drawGraphChars((unsigned char*)szout, 0);
	}

	return outlen;
}

int getgdt(char * szout) {
	char strgdt[8];
	
	__asm {
		sgdt fword ptr[strgdt];
	}

	int l = 0;
	int outlen = 0;

	int gdtlen = *(WORD*)strgdt + 1;

	DWORD gdtbase = *(DWORD*)(strgdt + 2);

	__int64 * pgdts = (__int64*)gdtbase;

	int cnt = gdtlen >> 3;

	for (int i = 0; i < cnt; i++)
	{
		l = __sprintf(szout + outlen, "gdt %d:%I64u\n", i, pgdts[i]);
		outlen += l;
		//__drawGraphChars((unsigned char*)szout, 0);
	}

	return outlen;
}


int getidt(char * szout) {
	char stridt[8];

	__asm {
		sidt fword ptr[stridt];
	}

	int l = 0;

	int outlen = 0;

	int idtlen = *(WORD*)stridt + 1;

	DWORD idtbase = *(DWORD*)(stridt + 2);

	__int64 * pidts = (__int64*)idtbase; 

	//__int64 * pidts = (__int64*)(stridt + 2);

	int cnt = idtlen >> 3;

	for (int i = 0; i < cnt; i++)
	{
		l =  __sprintf(szout + outlen, "idt %d:%I64u\n", i, pidts[i]);
		outlen += l;
		//__drawGraphChars((unsigned char*)szout, 0);
	}

	return outlen;
}

void __kSysRegs() {

// 	__printf(szout, "assembly gdt:%x,sgdt gdt:%x,GDT0:%I64d,GDT1:%I64d,GDT2:%I64d,GDT3:%I64d,GDT4:%I64d,assembly idt:%x,sidt idt:%x,IDT0:%I64d,IDT1:%I64d\n", 
// 		gdt,*(DWORD*)(strgdt+2),
// 		gdt[0],gdt[1], gdt[2], gdt[3], gdt[4], gdt[5],gdt[6], gdt[7], gdt[8], gdt[9],
// 		idt, *(DWORD*)(stridt + 2),
// 		idt[0],idt[1], idt[2], idt[3]);
// 	__drawGraphChars((unsigned char*)szout, 0);
}