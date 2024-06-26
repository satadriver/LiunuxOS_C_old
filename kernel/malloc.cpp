
#include "malloc.h"
#include "def.h"
#include "Utils.h"
#include "video.h"
#include "descriptor.h"
#include "page.h"
#include "process.h"
#include "task.h"
#include "memory.h"

DWORD gAvailableSize = 0;
DWORD gAvailableBase = 0;

LPMEMALLOCINFO gMemAllocList = 0;

DWORD gAllocLimitSize = 0;

DWORD gAllocLock = FALSE;

//Bit Scan Forward
//格式: BSF dest, src
//	影响标志位 : ZF
//	功能：从源操作数的的最低位向高位搜索，将遇到的第一个“1”所在的位序号存入目标寄存器中，
//	若所有位都是0，则ZF = 1，否则ZF = 0。

//Bit Scan Reverse
//BSR dest, src
//	影响标志位 : ZF
//	功能：从源操作数的的最高位向低位搜索，将遇到的第一个“1”所在的位序号存入目标寄存器中，
//	若所有位都是0，则ZF = 1，否则ZF = 0。

//BTS指令
//格式: BTS OPD, OPS
//功能 :  源操作数OPS指定的位送CF标志, 目的操作数OPD中那一位置位.

DWORD pageAlignmentSize(DWORD blocksize,int max)
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
		cmp max,0
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

	gMemAllocList = (LPMEMALLOCINFO)MEMORY_ALLOC_INFO;
	initListEntry(&gMemAllocList->list);
	gMemAllocList->addr = 0;
	gMemAllocList->size = 0;
	gMemAllocList->vaddr = 0;
	
	int cnt = *(int*)MEMORYINFO_LOAD_ADDRESS;
	if (cnt <= 0)
	{
		return FALSE;
	}

	ADDRESS_RANGE_DESCRIPTOR_STRUCTURE * ards = (ADDRESS_RANGE_DESCRIPTOR_STRUCTURE*)(MEMORYINFO_LOAD_ADDRESS + 4);
	for ( int i = 0;i < cnt ;i ++)
	{
// 		int len = __printf(szout, "Memory address:%I64d,length:%I64d,type:%x\n",
// 			ards->BaseAddrLow, ards->BaseAddrHigh, ards->LengthLow, ards->LengthHigh, ards->Type);
// 		__drawGraphChars((unsigned char*)szout, 0);
		
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

				if (gAvailableBase - MEMMORY_HEAP_BASE > 0)
				{
					gAllocLimitSize = (gAvailableSize - gAvailableBase) / 2;
				}
				else {
					gAllocLimitSize = (gAvailableSize - MEMMORY_HEAP_BASE) / 2;
				}
				
				gAllocLimitSize = pageAlignmentSize(gAllocLimitSize, 0);

				int len = __printf(szout, "available memory address:%x,size:%x,alloc max size:%x\n",
					gAvailableBase,gAvailableSize, gAllocLimitSize);
			}
		}

		ards++;
	}

	return 0;
}

//检查物理地址是否存在，而不是虚拟地址
LPMEMALLOCINFO getExistAddrInfo(DWORD addr,int size) {
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList;
	do
	{
		if (info == 0 )
		{
			return (LPMEMALLOCINFO)-1;
		}

		if ( (info->addr <= addr) && ( info->addr + info->size > addr) )
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

	int c = MEMORY_ALLOC_INFO_SIZE / sizeof(MEMALLOCINFO);
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

	DWORD ret = 0;

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

	__enterSpinLock(&gAllocLock);

	int factor = 1;
	DWORD addr = MEMMORY_HEAP_BASE + size*factor;
	if (addr + size > gAvailableBase + gAvailableSize)
	{
		goto __kProcessMalloc_end;
	}

	while (1)
	{
		LPMEMALLOCINFO info = getExistAddrInfo(addr,size);
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

				ret = addr;
			}
			goto __kProcessMalloc_end;
		}
		else {
			if (info == (LPMEMALLOCINFO )-1)
			{
				goto __kProcessMalloc_end;
			}
			if ( (info->size <= size) && (factor > 1) )
			{
				for (int i = 0; i < factor - 1; i++)
				{
					addr += size;
					if (addr + size > gAvailableBase + gAvailableSize)
					{
						goto __kProcessMalloc_end;
					}

					info = getExistAddrInfo(addr,size);
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

							ret = addr;
						}
						goto __kProcessMalloc_end;
					}
				}
			}

			factor = (factor << 1);
			addr = MEMMORY_HEAP_BASE + size * factor;
		}
	}

__kProcessMalloc_end:

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	if (process->pid == pid)
	{
		process->vasize += size;
	}

	TASK_LIST_ENTRY * head = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	TASK_LIST_ENTRY * list = head;
	do
	{
		if (list->valid && list->process->pid == pid)
		{
			list->process->vasize += size;
		}
		list = (TASK_LIST_ENTRY*)list->list.next;
	} while (list != head);

	__leaveSpinLock(&gAllocLock);

	return ret;
}



DWORD __kMalloc(DWORD s) {

	DWORD size = 0;
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	DWORD ret = __kProcessMalloc(s, &size,process->pid,0);
	return ret;
}


DWORD __malloc(DWORD s) {
	if (s < PAGE_SIZE)
	{
		return __heapAlloc(s);
	}

	char szout[1024];

	DWORD size = 0;

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	DWORD vaddr = process->vaddr + process->vasize;

	DWORD ret = __kProcessMalloc(s,&size, process->pid,vaddr);
	if (ret)
	{
		if (vaddr >= USER_SPACE_END)
		{
			__kFree(ret);
			return FALSE;
		}

		DWORD * cr3 = (DWORD *)process->tss.cr3;
		DWORD pagecnt = phy2linear(vaddr, ret, size, cr3);
		if (pagecnt)
		{
			return vaddr;
		}
	}

	int len = __printf(szout, "__malloc size:%x error\n", size);

	return FALSE;
}

//__kMalloc返回的是物理地址，因此释放的也是物理地址,如果释放线性地址会出错
int __kFree(DWORD physicalAddr) {

	__enterSpinLock(&gAllocLock);

	LPMEMALLOCINFO info = getExistAddrInfo(physicalAddr,0);
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


//__malloc返回的是虚拟地址，应用程序调用
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
		LPMEMALLOCINFO info = getExistAddrInfo(phyaddr,0);
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

	MS_HEAP_STRUCT * lpheap = (MS_HEAP_STRUCT *)((UCHAR*)addr - sizeof(MS_HEAP_STRUCT));

	if (lpheap->addr == addr)
	{
		MS_HEAP_STRUCT * prev = (MS_HEAP_STRUCT *)((UCHAR*)lpheap - sizeof(MS_HEAP_STRUCT) );

		MS_HEAP_STRUCT * next = (MS_HEAP_STRUCT *)((UCHAR*)lpheap + (lpheap->size ) + (sizeof(MS_HEAP_STRUCT) << 1));

		if ((prev->flag & 1)== 1 && (next->flag & 1)== 1)
		{
			lpheap->flag = 0;
		}
		else if ((prev->flag & 1) == 0 && (next->flag & 1)== 0)
		{
			prev->size = prev->size + lpheap->size + next->size + (sizeof(MS_HEAP_STRUCT)<<1) + (sizeof(MS_HEAP_STRUCT) << 1);
			prev->flag = 0;
		}
		else if ((prev->flag & 1) == 1 && (next->flag & 1) == 0)
		{
			lpheap->size = lpheap->size + next->size + (sizeof(MS_HEAP_STRUCT) << 1);
			lpheap->flag = 0;
		}
		else if ((prev->flag & 1) == 0 && (next->flag & 1) == 1)
		{
			prev->size = prev->size + lpheap->size + (sizeof(MS_HEAP_STRUCT) << 1);
			prev->flag = 0;
		}

		return TRUE;
	}

	return FALSE;
}


DWORD __heapAlloc(int size) {

	int allocsize = getAlignedSize(size, sizeof(MS_HEAP_STRUCT));

	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	MS_HEAP_STRUCT * lpheap = (MS_HEAP_STRUCT *)tss->heapbase;

	while ((DWORD)lpheap + allocsize + (sizeof(MS_HEAP_STRUCT) << 1) < tss->heapbase + tss->heapsize)
	{
		if ( (lpheap->flag & 1) && lpheap->size && lpheap->addr)
		{
			lpheap = (MS_HEAP_STRUCT *)((UCHAR*)lpheap + (lpheap->size ) + (sizeof(MS_HEAP_STRUCT) << 1));
			continue;
		}
		else {
			if (lpheap->size && lpheap->addr)
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
	}
	return 0;
}




unsigned char * splitBlock(unsigned char * base, int size) {
	int blocksize = PAGE_SIZE;

	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList;

	unsigned char * ptr = base;

	int max = pageAlignmentSize(size, TRUE);

	while (blocksize < max)
	{

		addlistHead((LPLIST_ENTRY)gMemAllocList, (LPLIST_ENTRY)info);
		blocksize = blocksize << 1;

	}
	
	return 0;
}


unsigned char * __slab_malloc(int size) {
	return 0;
}