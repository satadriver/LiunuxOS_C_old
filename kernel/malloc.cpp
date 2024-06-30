
#include "malloc.h"
#include "def.h"
#include "Utils.h"
#include "video.h"
#include "descriptor.h"
#include "page.h"
#include "process.h"
#include "task.h"
#include "memory.h"

DWORD gAllocLimitSize = 0;
DWORD gAvailableSize = 0;
DWORD gAvailableBase = 0;

LPMEMALLOCINFO gMemAllocList = 0;

DWORD gAllocLock = FALSE;

//Bit Scan Forward
//��ʽ: BSF dest, src
//	Ӱ���־λ : ZF
//	���ܣ���Դ�������ĵ����λ���λ�������������ĵ�һ����1�����ڵ�λ��Ŵ���Ŀ��Ĵ����У�
//	������λ����0����ZF = 1������ZF = 0��

//Bit Scan Reverse
//BSR dest, src
//	Ӱ���־λ : ZF
//	���ܣ���Դ�������ĵ����λ���λ�������������ĵ�һ����1�����ڵ�λ��Ŵ���Ŀ��Ĵ����У�
//	������λ����0����ZF = 1������ZF = 0��

//BTSָ��
//��ʽ: BTS OPD, OPS
//���� :  Դ������OPSָ����λ��CF��־, Ŀ�Ĳ�����OPD����һλ��λ.


//direction 1:upward 0:downward
DWORD pageAlignmentSize(DWORD blocksize,int direction)
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
		jz _minsize
		cmp direction,0
		jz _minsize

		shl edx, 1
		_minsize:
		mov result, edx
		_Over :
	}

	return result;
}


int initMemory() {
	char szout[1024];

	gMemAllocList = (LPMEMALLOCINFO)MEMORY_ALLOC_BUFLIST;
	initListEntry(&gMemAllocList->list);
	gMemAllocList->addr = 0;
	gMemAllocList->size = 0;
	gMemAllocList->vaddr = 0;
	
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

				if (gAvailableBase - MEMMORY_ALLOC_BASE > 0)
				{
					gAllocLimitSize = (gAvailableSize - gAvailableBase) / 2;
				}
				else {
					gAllocLimitSize = (gAvailableSize - MEMMORY_ALLOC_BASE) / 2;
				}
				
				gAllocLimitSize = pageAlignmentSize(gAllocLimitSize, 0);

				int len = __printf(szout, "available memory address:%x,size:%x,alloc limit size:%x\n",
					gAvailableBase,gAvailableSize, gAllocLimitSize);
			}
		}

		ards++;
	}

	return 0;
}

//include 4 types of overlapped memory
LPMEMALLOCINFO getExistAddr(DWORD addr,int size) {
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList;
	do
	{
		if (info == 0 )
		{
			return (LPMEMALLOCINFO)-1;
		}
		else if ( (info->addr <= addr) && ( info->addr + info->size > addr) )
		{
			return info;
		}
		else if ( (info->addr >= addr) && (  info->addr < addr + size ) )
		{
			return info;
		}
		else {
			info = (LPMEMALLOCINFO)info->list.next;
		}
	} while (info != gMemAllocList);

	return 0;
}

LPMEMALLOCINFO getFreeMemItem() {
	LPMEMALLOCINFO item = (LPMEMALLOCINFO)gMemAllocList;

	int c = MEMORY_ALLOC_BUFLIST_SIZE / sizeof(MEMALLOCINFO);
	for ( int i = 0;i < c;i ++)
	{
		if (item[i].size == 0 && item[i].addr == 0)
		{
			return &item[i];
		}
	}
	return 0;
}


DWORD __kProcessMalloc(DWORD s,DWORD *retsize, int pid,DWORD vaddr) {

	DWORD res = 0;

	DWORD size = pageAlignmentSize(s, 1);
	if ( size > gAllocLimitSize)
	{
		return FALSE;
	}

	if (size < PAGE_SIZE)
	{
		size = PAGE_SIZE;
	}

	*retsize = size;

	int factor = 1;
	DWORD addr = MEMMORY_ALLOC_BASE + size*factor;
	if (addr + size > gAvailableBase + gAvailableSize)
	{
		return FALSE;
	}

	__enterSpinLock(&gAllocLock);

	while (TRUE)
	{
		LPMEMALLOCINFO info = getExistAddr(addr,size);
		if (info == 0)
		{
			info = getFreeMemItem();
			if (info)
			{
				info->size = size;
				info->addr = addr;
				if (vaddr)
				{
					info->vaddr = vaddr;
				}
				else {
					info->vaddr = addr;
				}

				info->pid = pid;

				addlistTail((LPLIST_ENTRY)gMemAllocList, (LPLIST_ENTRY)info);

				res = addr;
			}
			else {
				res = -1;
			}

			break;
		}
		else if (info == (LPMEMALLOCINFO)-1)
		{
			res = -1;
			break;
		}
		else {

			if ( (info->size <= size) && (factor > 1) )		// important
			{
				for (int i = 0; i < factor - 1; i++)
				{
					addr += size;
					if (addr + size > gAvailableBase + gAvailableSize)
					{
						res = -1;
						break;
					}

					info = getExistAddr(addr,size);
					if (info == 0)
					{
						info = getFreeMemItem();
						if (info)
						{
							if (vaddr)
							{
								info->vaddr = vaddr;
							}
							else {
								info->vaddr = addr;
							}
							info->pid = pid;
							info->size = size;
							info->addr = addr;
							addlistTail((LPLIST_ENTRY)gMemAllocList, (LPLIST_ENTRY)info);

							res = addr;
						}
						else {
							res = -1;
						}
						break;
					}
					else {
						break;
					}
				}
			}
			else {
				//nothing to do
			}

			if (res == 0) {
				factor = (factor << 1);
				addr = MEMMORY_ALLOC_BASE + size * factor;
			}
			else {
				break;
			}
		}
	}

	if (res == -1) {
		res = 0;
	}

	if (res ) {
		LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
		if (process->pid == pid)
		{
			process->vasize += size;
		}
		else {
			//process += pid;
			//process->vasize += size;
		}

		TASK_LIST_ENTRY* head = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
		TASK_LIST_ENTRY* list = head;
		do
		{
			if (list->valid && list->process->pid == pid && list->process->status == TASK_RUN)
			{
				list->process->vasize += size;
			}
			list = (TASK_LIST_ENTRY*)list->list.next;
		} while (list != head);
	}

	__leaveSpinLock(&gAllocLock);

	return res;
}


//return phisical address
DWORD __kMalloc(DWORD s) {

	DWORD size = 0;
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	DWORD ret = __kProcessMalloc(s, &size,process->pid,0);
	return ret;
}

//return virtual address
DWORD __malloc(DWORD s) {
	if (s < PAGE_SIZE)
	{
		return __heapAlloc(s);
	}

	char szout[1024];

	DWORD size = 0;
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	DWORD vaddr = process->vaddr + process->vasize;
	DWORD res = __kProcessMalloc(s,&size, process->pid,vaddr);
	if (res)
	{
		if (vaddr >= USER_SPACE_END)
		{
			__kFree(res);
			return FALSE;
		}

		DWORD * cr3 = (DWORD *)process->tss.cr3;
		DWORD pagecnt = mapPhyToLinear(vaddr, res, size, cr3);
		if (pagecnt)
		{
			return vaddr;
		}
	}

	int len = __printf(szout, "__malloc size:%x error\n", size);

	return FALSE;
}

//__kMalloc���ص��������ַ������ͷŵ�Ҳ�������ַ,����ͷ����Ե�ַ�����
int __kFree(DWORD physicalAddr) {

	__enterSpinLock(&gAllocLock);

	LPMEMALLOCINFO info = getExistAddr(physicalAddr,0);
	if (info)
	{
		DWORD size = info->size;
		removelist((LPLIST_ENTRY)info);
		info->addr = 0;
		info->size = 0;
		info->pid = 0;
		info->vaddr = 0;

		__leaveSpinLock(&gAllocLock);

		return size;
	}

	__leaveSpinLock(&gAllocLock);

	char szout[1024];
	int len = __printf(szout, "__kFree not found physical address:%x\n", physicalAddr);

	return FALSE;
}


//__malloc���ص��������ַ��Ӧ�ó������
int __free(DWORD linearAddr) {

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	if (linearAddr >= process->heapbase + process->heapsize)
	{
		return __heapFree(linearAddr);
	}

	__enterSpinLock(&gAllocLock);

	DWORD phyaddr = linear2phy(linearAddr);
	if (phyaddr)
	{
		LPMEMALLOCINFO info = getExistAddr(phyaddr,0);
		if (info)
		{
			DWORD size = info->size;

			removelist((LPLIST_ENTRY)info);
			info->addr = 0;
			info->size = 0;
			info->vaddr = 0;
			info->pid = 0;

			__leaveSpinLock(&gAllocLock);

			return size;
		}
	}

	__leaveSpinLock(&gAllocLock);

	char szout[1024];
	int len = __printf(szout , "__free not found linear address:%x,physical address:%x\n", linearAddr,phyaddr);

	return FALSE;
}

int formatProcMem(int pid,char * szout) {
	int offset = 0;

	LPPROCESS_INFO processes = (LPPROCESS_INFO)TASKS_TSS_BASE;
	LPPROCESS_INFO tss = processes + pid;
	if (tss->status != TASK_RUN)
	{
		return FALSE;
	}

	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList;
	do
	{
		if (info->pid == pid)
		{
			int len = __printf(szout + offset, "base:%x,vaddr:%x,size:%x,pid:%x\n", info->addr,info->vaddr, info->size, info->pid);
			offset += len;
		}

		info = (LPMEMALLOCINFO)info->list.next;

	} while (info != gMemAllocList);
	return offset;
}

//make sure the first in the list is not to be deleted,or else will be locked
void freeProcessMemory() {
	__enterSpinLock(&gAllocLock);

	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList;
	do
	{
		if (info == 0)
		{
			break;
		}

		if (info->pid == tss->pid)
		{
			removelist( (LPLIST_ENTRY)info);
			info->addr = 0;
			info->size = 0;
			info->pid = 0;
			info->vaddr = 0;
		}

		info = (LPMEMALLOCINFO)info->list.next;

	} while (info != gMemAllocList);

	__leaveSpinLock(&gAllocLock);
}


int getAlignedSize(int size, int allignsize) {
	int allocsize = size;
	int mod = size % allignsize;
	if (mod)
	{
		allocsize = allocsize + (allignsize - mod);
	}
	return allocsize;
}


DWORD __heapFree(DWORD addr) {
	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	MS_HEAP_STRUCT test ;
	test.flag = 1;

	MS_HEAP_STRUCT * heap = (MS_HEAP_STRUCT *)((UCHAR*)addr - sizeof(MS_HEAP_STRUCT));
	MS_HEAP_STRUCT* heapEnd = (MS_HEAP_STRUCT*)((UCHAR*)heap + heap->size + sizeof(MS_HEAP_STRUCT));
	if (heap->addr == addr)
	{
		MS_HEAP_STRUCT* prevEnd = 0;
		MS_HEAP_STRUCT* prev = 0;
		if ((DWORD)heap <= tss->heapbase) {
			prevEnd = (MS_HEAP_STRUCT*)&test;
			prev = (MS_HEAP_STRUCT*)&test;
		}
		else {
			prevEnd = (MS_HEAP_STRUCT*)((UCHAR*)heap - sizeof(MS_HEAP_STRUCT));
			prev = (MS_HEAP_STRUCT*)((UCHAR*)prevEnd - prevEnd->size - sizeof(MS_HEAP_STRUCT));
		}

		MS_HEAP_STRUCT* next = (MS_HEAP_STRUCT*)((UCHAR*)heap + (heap->size) + (sizeof(MS_HEAP_STRUCT) << 1));
		MS_HEAP_STRUCT* nextEnd = 0;
		if (next->addr == 0 && next->size == 0 && next->flag == 0 && next->reserved == 0) {
			next = (MS_HEAP_STRUCT*)&test;
			nextEnd = (MS_HEAP_STRUCT*)&test;
		}
		else if (tss->heapbase + tss->heapsize - (DWORD)next < 3 * sizeof(MS_HEAP_STRUCT)) {
			next = (MS_HEAP_STRUCT*)&test;
			nextEnd = (MS_HEAP_STRUCT*)&test;
		}
		else {
			nextEnd = (MS_HEAP_STRUCT*)((UCHAR*)next + (next->size) + sizeof(MS_HEAP_STRUCT));
		}

		if ((prev->flag & 1)== 1 && (next->flag & 1)== 1)
		{
			heap->flag = 0;
			heapEnd->flag = 0;
		}
		else if ((prev->flag & 1) == 0 && (next->flag & 1)== 0)
		{
			prev->addr = (DWORD)prev + sizeof(MS_HEAP_STRUCT);
			prev->size = prev->size + heap->size + next->size + (sizeof(MS_HEAP_STRUCT)<<1) + (sizeof(MS_HEAP_STRUCT) << 1);
			prev->flag = 0;
			nextEnd->size = prev->size;
			nextEnd->addr = prev->addr;
			nextEnd->flag = prev->flag;
		}
		else if ((prev->flag & 1) == 1 && (next->flag & 1) == 0)
		{
			heap->addr = (DWORD)heap + sizeof(MS_HEAP_STRUCT);
			heap->size = heap->size + next->size + (sizeof(MS_HEAP_STRUCT) << 1);
			heap->flag = 0;
			nextEnd->addr = heap->addr;
			nextEnd->flag = heap->flag;
			nextEnd->size = heap->size;
		}
		else if ((prev->flag & 1) == 0 && (next->flag & 1) == 1)
		{
			prev->addr = (DWORD)prev + sizeof(MS_HEAP_STRUCT);
			prev->size = prev->size + heap->size + (sizeof(MS_HEAP_STRUCT) << 1);
			prev->flag = 0;
			heapEnd->addr = prev->addr;
			heapEnd->flag = prev->flag;
			heapEnd->size = prev->size;
		}

		return TRUE;
	}

	return FALSE;
}


DWORD __heapAlloc(int size) {

	int allocsize = getAlignedSize(size, sizeof(MS_HEAP_STRUCT));

	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	MS_HEAP_STRUCT * lpheap = (MS_HEAP_STRUCT *)tss->heapbase;

	while ((DWORD)lpheap + allocsize + (sizeof(MS_HEAP_STRUCT) << 1) <= tss->heapbase + tss->heapsize)
	{
		if ( (lpheap->flag & 1) && lpheap->size && lpheap->addr)
		{
			lpheap = (MS_HEAP_STRUCT *)((UCHAR*)lpheap + (lpheap->size ) + (sizeof(MS_HEAP_STRUCT) << 1));
			continue;
		}
		else if (lpheap->size && lpheap->addr)
		{
			if ((lpheap->size) >= allocsize )
			{
				int oldsize = (lpheap->size );

				lpheap->flag = 1;
				lpheap->addr = (DWORD)((UCHAR*)lpheap + sizeof(MS_HEAP_STRUCT));
				lpheap->size = allocsize ;

				MS_HEAP_STRUCT * heapend = (MS_HEAP_STRUCT *)((UCHAR*)lpheap + (lpheap->size ) + sizeof(MS_HEAP_STRUCT));
				heapend->addr = lpheap->addr;
				heapend->size = lpheap->size;
				heapend->flag = lpheap->flag;

				MS_HEAP_STRUCT * next = (MS_HEAP_STRUCT *)((UCHAR*)lpheap + (lpheap->size ) + (sizeof(MS_HEAP_STRUCT) << 1));
				next->size = (oldsize - allocsize - (sizeof(MS_HEAP_STRUCT) << 1));
				next->addr = (DWORD)((UCHAR*)next + sizeof(MS_HEAP_STRUCT));
				next->flag = 0;

				return lpheap->addr;
			}
			else {
				lpheap = (MS_HEAP_STRUCT *)((UCHAR*)lpheap + (lpheap->size ) + (sizeof(MS_HEAP_STRUCT) << 1));
				continue;
			}
		}
		else {
			lpheap->flag = 1;
			lpheap->addr = (DWORD)((UCHAR*)lpheap + sizeof(MS_HEAP_STRUCT));
			lpheap->size = allocsize ;

			MS_HEAP_STRUCT * heapend = (MS_HEAP_STRUCT *)((UCHAR*)lpheap + (lpheap->size ) + sizeof(MS_HEAP_STRUCT));
			heapend->addr = lpheap->addr;
			heapend->size = lpheap->size;
			heapend->flag = lpheap->flag;

			return lpheap->addr;	
		}
	}
	return 0;
}







unsigned char * __slab_malloc(int size) {
	return 0;
}