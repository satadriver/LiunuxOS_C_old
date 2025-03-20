
#include "malloc.h"
#include "def.h"
#include "Utils.h"
#include "video.h"
#include "descriptor.h"
#include "page.h"
#include "process.h"
#include "task.h"
#include "memory.h"
#include "heap.h"


DWORD gAvailableSize = 0;

DWORD gAvailableBase = 0;

DWORD gAllocLimitSize = 0;

DWORD gAllocLock = FALSE;

LPMEMALLOCINFO gMemAllocList = 0;


DWORD getBorderAddr() {
	return gAvailableBase + gAvailableSize;
}

int getAlignSize(int size, int allignsize) {
	int allocsize = size;
	int mod = size % allignsize;
	if (mod)
	{
		allocsize = allocsize + (allignsize - mod);
	}
	return allocsize;
}


//Bit Scan Forward
//��ʽ: BSF dest, src
//Ӱ���־λ : ZF
//���ܣ���Դ�������ĵ����λ���λ�������������ĵ�һ����1�����ڵ�λ��Ŵ���Ŀ��Ĵ����У�������λ����0����ZF = 1������ZF = 0��

//Bit Scan Reverse
//BSR dest, src
//Ӱ���־λ : ZF
//���ܣ���Դ�������ĵ����λ���λ�������������ĵ�һ����1�����ڵ�λ��Ŵ���Ŀ��Ĵ����У�������λ����0����ZF = 1������ZF = 0��

//BTSָ��
//��ʽ: BTS OPD, OPS
//���� :  Դ������OPSָ����λ��CF��־, Ŀ�Ĳ�����OPD����һλ��λ.


//direction 1:upward 0:downward
DWORD pageAlignSize(DWORD blocksize,int direction)
{
	DWORD result = 0;

	__asm {
		xor eax, eax
		mov ecx,dword ptr blocksize
		bsr eax,ecx
		jz _Over
		
		xor edx, edx
		bts edx,eax

		cmp edx, blocksize
		jz _minimumSize
		cmp direction,0
		jz _minimumSize

		shl edx, 1
		_minimumSize:
		mov result, edx
		_Over :
	}

	return result;
}


int initMemory() {
	char szout[1024];

	resetAllMemAllocInfo();
	
	int cnt = *(int*)MEMORYINFO_LOAD_ADDRESS;
	if (cnt <= 0)
	{
		return FALSE;
	}

	ADDRESS_RANGE_DESCRIPTOR_STRUCTURE * ards = (ADDRESS_RANGE_DESCRIPTOR_STRUCTURE*)(MEMORYINFO_LOAD_ADDRESS + sizeof(int));
	for ( int i = 0;i < cnt ;i ++)
	{
// 		int len = __printf(szout, "Memory address:%I64d,length:%I64d,type:%x\n",
// 			ards->BaseAddrLow, ards->BaseAddrHigh, ards->LengthLow, ards->LengthHigh, ards->Type);
		
		if (ards->Type == 1 )
		{
			__int64 low = ards->BaseAddrLow;
			__int64 high = ards->BaseAddrHigh;
			__int64 b = (high << 32) + low;
			if (b == 0x100000)	//0x100000 is the extended mem base
			{
				__int64 sl = ards->LengthLow;
				__int64 sh = ards->LengthHigh;
				__int64 s = (sh << 32) + sl;

				gAvailableBase = (DWORD)b;
				gAvailableSize = (DWORD)s;

				if (gAvailableBase > MEMMORY_ALLOC_BASE )
				{
					gAllocLimitSize = (gAvailableSize - gAvailableBase) / 1;
				}
				else {
					gAllocLimitSize = (gAvailableSize - MEMMORY_ALLOC_BASE) / 1;
				}
				
				//gAllocLimitSize = pageAlignmentSize(gAllocLimitSize, 0);

				int len = __printf(szout, "available memory address:%x,size:%x,alloc limit size:%x\n",
					gAvailableBase,gAvailableSize, gAllocLimitSize);
			}
		}

		ards++;
	}

	return 0;
}


LPMEMALLOCINFO isAddrExist(DWORD addr,int size) {

	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList->list.next;

	LPMEMALLOCINFO base = info;

	do
	{
		if (info == 0)
		{
			return (LPMEMALLOCINFO)0;
		}	
		else if (info->addr == addr) 
		{
			return info;
		}
		else if ( (info->addr < addr) && ( info->addr + info->size > addr) )
		{
			return info;
		}
		else {
			info = (LPMEMALLOCINFO)info->list.next;
		}
	} while (info != base);

	return 0;
}

LPMEMALLOCINFO findAddr(DWORD addr) {

	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList->list.next;

	LPMEMALLOCINFO base = info;

	do
	{
		if (info == 0)
		{
			return (LPMEMALLOCINFO)0;
		}
		else if (info->addr == addr)
		{
			return info;
		}
		else {
			info = (LPMEMALLOCINFO)info->list.next;
		}
	} while (info != base);

	return 0;
}



void resetAllMemAllocInfo() {
	
	LPMEMALLOCINFO item = (LPMEMALLOCINFO)MEMORY_ALLOC_BUFLIST;
	int cnt = MEMORY_ALLOC_BUFLIST_SIZE / sizeof(MEMALLOCINFO);
	for (int i = 0; i < cnt; i++)
	{
		__memset((char*)&item[i], 0, sizeof(MEMALLOCINFO));
	}

	gMemAllocList = (LPMEMALLOCINFO)MEMORY_ALLOC_BUFLIST;
	initListEntry(&gMemAllocList->list);
}


int resetMemAllocInfo(LPMEMALLOCINFO item) {
	DWORD size = item->size;

	removelist(&gMemAllocList->list,(LPLIST_ENTRY)&item->list);
	item->addr = 0;
	item->size = 0;
	item->vaddr = 0;
	item->pid = 0;

	return size;
}

LPMEMALLOCINFO getMemAllocInfo() {
	LPMEMALLOCINFO item = (LPMEMALLOCINFO)MEMORY_ALLOC_BUFLIST;

	int cnt = MEMORY_ALLOC_BUFLIST_SIZE / sizeof(MEMALLOCINFO);
	for ( int i = 1;i < cnt;i ++)
	{
		if ( //item[i].list.next == 0 && item[i].list.prev == 0 &&
			item[i].size == 0 && item[i].addr == 0 && item[i].vaddr == 0 && item[i].pid == 0)
		{
			return &item[i];
		}
	}
	return 0;
}


int setMemAllocInfo(LPMEMALLOCINFO item,DWORD addr,DWORD vaddr,int size,int pid) {
	if (vaddr)
	{
		item->vaddr = vaddr;
	}
	else {
		item->vaddr = addr;
	}
	item->pid = pid;
	item->size = size;
	item->addr = addr;


	addlistTail(& (gMemAllocList->list), & item->list);
	return 0;
}


DWORD __kProcessMalloc(DWORD s,DWORD *retsize, int pid,DWORD vaddr,int tag) {

	DWORD res = 0;

	char szout[1024];

	DWORD size = pageAlignSize(s, 1);
	if ( size > gAllocLimitSize)
	{
		__printf(szout, "__kProcessMalloc pageAlignmentSize:%x gAllocLimitSize:%x\r\n", size, gAllocLimitSize);
		return FALSE;
	}
	else if (size < PAGE_SIZE)
	{
		size = PAGE_SIZE;
	}

	*retsize = size;

	__enterSpinlock(&gAllocLock);

	int factor = 1;

	do
	{
		for (int n = factor/2 ; n && n < factor; )
		{
			DWORD addr = MEMMORY_ALLOC_BASE + size * n;
			if ( (addr + size > gAvailableBase + gAvailableSize) )
			{
				res = -1;
				__printf(szout, "__kProcessMalloc addr:%x, size:%x exceed available addr:%x,size:%x\r\n", 
					addr, size, gAvailableBase, gAvailableSize);
				break;
			}

			LPMEMALLOCINFO info = isAddrExist(addr, size);
			if (info == 0)
			{
				info = getMemAllocInfo();
				if (info)
				{
					setMemAllocInfo(info, addr, vaddr, size, pid);

					res = addr;
					break;
				}
				else {
					__printf(szout, "getMemAllocInfo failed\r\n");
					res = -1;
					break;
				}
			}
			else {
				if (info->size > size) {
					if (info->size % size) {
						__printf(szout, "isAddrExist size:%x size:%x error\r\n",info->size,size);
					}

					int t = info->size / size;
					n += t;
					if (n >= factor) {
						while (n >= factor) {
							factor = factor << 1;
						}
					}

					continue;
				}
			}

			n++;
		}

		if (res) {
			break;
		}

		factor = (factor << 1);
		
	} while (res == 0);

	if (res == -1) {
		res = 0;
	}

#ifndef DISABLE_PAGE_MAPPING
	if (res ) {		
		int vmtag = TRUE;
		if (vaddr == 0) {
			vaddr = res;
			vmtag = 0;
		}
		
		LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
		if (process->pid == pid)
		{
			if (vmtag) {
				vaddr = process->vaddr + process->vasize;
				process->vasize += size;
			}
			else {
				//process->vasize = vaddr + size;
			}
			
			DWORD* cr3 = (DWORD*)process->tss.cr3;
			DWORD pagecnt = mapPhyToLinear(vaddr, res, size, cr3, tag);
		}
		else {
			LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE + pid;
			if (vmtag) {
				vaddr = tss->vaddr + tss->vasize;
				tss->vasize += size;
			}
			else {
				//tss->vasize = vaddr + size;
			}
			
			DWORD* cr3 = (DWORD*)tss->tss.cr3;
			DWORD pagecnt = mapPhyToLinear(vaddr, res, size, cr3, tag);

			cr3 = (DWORD*)process->tss.cr3;
			pagecnt = mapPhyToLinear(vaddr, res, size, cr3,tag);
		}
	}
#endif

	__leaveSpinlock(&gAllocLock);

	return res;
}

//R/W--λ1�Ƕ�/д��Read/Write����־���������1����ʾҳ����Ա�����д��ִ�С����Ϊ0����ʾҳ��ֻ�����ִ�С�
//�������������ڳ����û���Ȩ��������0��1��2��ʱ����R/Wλ�������á�ҳĿ¼���е�R/Wλ������ӳ�������ҳ�������á�
//U/S--λ2���û�/�����û���User / Supervisor����־�����Ϊ1����ô�������κ���Ȩ���ϵĳ��򶼿��Է��ʸ�ҳ�档
//���Ϊ0����ôҳ��ֻ�ܱ������ڳ����û���Ȩ����0��1��2���ϵĳ�����ʡ�ҳĿ¼���е�U / Sλ������ӳ�������ҳ�������á�

//return phisical address
DWORD __kMalloc(DWORD s) {

	char szout[1024];
	DWORD size = 0;
	int len = 0;
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	//DWORD ret = __kProcessMalloc(s, &size,process->pid,0);
	DWORD ret = __kProcessMalloc(s, &size, 0, 0, PAGE_READWRITE | PAGE_USERPRIVILEGE | PAGE_PRESENT);
	if (ret == 0) {
		
		len = __printf(szout, "__kMalloc size:%x realSize:%x pid:%d error\n",s,size,process->pid);
	}
	else {
		//len = __printf(szout, "__kMalloc size:%x realSize:%x pid:%d addr:%x\n", s,size, process->pid,ret);
	}
	return ret;
}



int __kFree(DWORD physicalAddr) {

	char szout[1024];
	__enterSpinlock(&gAllocLock);

	LPMEMALLOCINFO info = findAddr(physicalAddr);
	if (info)
	{
		//int len = __printf(szout, "__kFree address:%x size:%x pid:%d vaddr:%x\n", physicalAddr, info->size, info->pid, info->vaddr);
		DWORD size = resetMemAllocInfo(info);
		
	}
	else {
		
		int len = __printf(szout, "__kFree not found address:%x\n", physicalAddr);
	}

	__leaveSpinlock(&gAllocLock);

	return FALSE;
}

//return virtual address
DWORD __malloc(DWORD s) {
	DWORD res = 0;
	if (s <= HEAP_SIZE/2)
	{
		res = __heapAlloc(s);
		if (res) {
			return res;
		}
		else {

		}
	}

	char szout[1024];

	DWORD size = 0;
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	DWORD vaddr = process->vaddr + process->vasize;
	res = __kProcessMalloc(s,&size, process->pid,vaddr, PAGE_READWRITE | PAGE_USERPRIVILEGE | PAGE_PRESENT);
	if (res)
	{
		if (vaddr >= USER_SPACE_END)
		{
			__kFree(res);
			return FALSE;
		}

		DWORD * cr3 = (DWORD *)process->tss.cr3;
		DWORD pagecnt = mapPhyToLinear(vaddr, res, size, cr3, PAGE_READWRITE | PAGE_USERPRIVILEGE | PAGE_PRESENT);
		if (pagecnt)
		{
			return vaddr;
		}
	}

	int len = __printf(szout, "__malloc size:%x error\n", size);

	return FALSE;
}



int __free(DWORD linearAddr) {

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	if (linearAddr >= process->heapbase + process->heapsize)
	{
		return __heapFree(linearAddr);
	}

	__enterSpinlock(&gAllocLock);

	DWORD phyaddr = linear2phy(linearAddr);
	if (phyaddr)
	{
		LPMEMALLOCINFO info = findAddr(phyaddr);
		if (info)
		{
			DWORD size = resetMemAllocInfo(info);
		}
		else {
			char szout[1024];
			int len = __printf(szout, "__free not found linear address:%x,physical address:%x\n", linearAddr, phyaddr);
		}
	}

	__leaveSpinlock(&gAllocLock);

	return FALSE;
}



//make sure the first in the list is not to be deleted,or else will be locked
void freeProcessMemory(int pid) {

	__enterSpinlock(&gAllocLock);

	LPMEMALLOCINFO base = (LPMEMALLOCINFO)gMemAllocList->list.next;
	LPMEMALLOCINFO info =(LPMEMALLOCINFO) base;
	do
	{
		if (info == 0)
		{
			break;
		}
		else if (info->pid == pid)
		{
			resetMemAllocInfo(info);
		}

		info = (LPMEMALLOCINFO)info->list.next;

	} while (info != (LPMEMALLOCINFO)base);

	__leaveSpinlock(&gAllocLock);
}




unsigned char* __slab_malloc(int size) {
	return 0;
}


int getProcMemory(int pid, char* szout) {
	int offset = 0;

	LPPROCESS_INFO processes = (LPPROCESS_INFO)TASKS_TSS_BASE;
	LPPROCESS_INFO tss = processes + pid;
	if (tss->status != TASK_RUN)
	{
		return FALSE;
	}

	LPMEMALLOCINFO base = (LPMEMALLOCINFO)gMemAllocList->list.next;
	LPMEMALLOCINFO info = base;
	do
	{
		if (info == 0) {
			break;
		}
		if (info->pid == pid)
		{
			int len = __printf(szout + offset, 
				"memory:%x,virtual memory:%x,size:%x,pid:%x\n", info->addr, info->vaddr, info->size, info->pid);
			offset += len;
		}

		info = (LPMEMALLOCINFO)info->list.next;

	} while (info != (LPMEMALLOCINFO)base);

	return offset;
}