
#include "slab.h"
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

DWORD getAlignSize(DWORD s,int flag) {

	DWORD result = 0;

	__asm {
		xor eax, eax
		xor edx, edx

		mov ecx,dword ptr s
		bsr eax,ecx
		jz _Over
		
		bts edx,eax
		cmp flag,0
		jz _minsize
		cmp edx,s
		jz _minsize

		shl edx, 1
		mov result, edx
		jmp _Over
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
				
				gAllocLimitSize = getAlignSize(gAllocLimitSize, 0);

				int len = __printf(szout, "available memory address:%x,size:%x,alloc max size:%x\n",gAvailableBase,gAvailableSize, gAllocLimitSize);
				__drawGraphChars((unsigned char*)szout, 0);
			}
		}

		ards++;
	}

	return 0;
}

//检查物理地址是否存在，而不是虚拟地址
LPMEMALLOCINFO checkAddrExist(DWORD addr) {
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gMemAllocList;
	do
	{
		if (info == 0 )
		{
			break;
		}

		if ( (info->addr <= addr) && (addr < (info->addr + info->size )) )
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

	DWORD size = getAlignSize(s, 1);
	if ( size > gAllocLimitSize)
	{
		return FALSE;
	}

	*retsize = size;

	if (size < PAGE_SIZE)
	{
		size = PAGE_SIZE;
	}

	__enterSpinLock(&gAllocLock);

	int factor = 1;
	DWORD addr = MEMMORY_HEAP_BASE + size*factor;
	if (addr + size > gAvailableBase + gAvailableSize)
	{
		goto __kProcessMalloc_end;
	}

	while (1)
	{
		LPMEMALLOCINFO info = checkAddrExist(addr);
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
			if ( (info->size <= size) && (factor > 1) )
			{
				for (int i = 0; i < factor - 1; i++)
				{
					addr += size;
					if (addr + size > gAvailableBase + gAvailableSize)
					{
						goto __kProcessMalloc_end;
					}

					info = checkAddrExist(addr);
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
	__drawGraphChars((unsigned char*)szout, 0);
	return FALSE;
}

//__kMalloc返回的是物理地址，因此释放的也是物理地址,如果释放线性地址会出错
int __kFree(DWORD physicalAddr) {

	__enterSpinLock(&gAllocLock);

	LPMEMALLOCINFO info = checkAddrExist(physicalAddr);
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
	__drawGraphChars((unsigned char*)szout, 0);
	return FALSE;
}


//__malloc返回的是虚拟地址，应用程序调用
int __free(DWORD linearAddr) {

	__enterSpinLock(&gAllocLock);

	DWORD phyaddr = linear2phy(linearAddr);
	if (phyaddr)
	{
		LPMEMALLOCINFO info = checkAddrExist(phyaddr);
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
	__drawGraphChars((unsigned char*)szout, 0);
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