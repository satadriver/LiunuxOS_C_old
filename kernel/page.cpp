#include "page.h"
#include "Utils.h"
#include "video.h"
#include "slab.h"
#include "process.h"



LPMEMALLOCINFO gPageAllocList = 0;
DWORD gPageAllocLock = FALSE;

DWORD gPageTableBase = 0;


void initPage() {
	gPageAllocList = (LPMEMALLOCINFO)PAGE_INFO_PAGE;
	initListEntry(&gPageAllocList->list);
	gPageAllocList->addr = 0;
	gPageAllocList->size = 0;
	gPageAllocList->vaddr = 0;

	//gPageTableBase = __kMalloc(PAGE_TABLE_SIZE);
	gPageTableBase = PAGE_TABLE_BASE;
}

LPMEMALLOCINFO checkPageExist(DWORD addr) {
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gPageAllocList;
	do
	{
		if (info->addr == addr)
		{
			return info;
		}
		else {
			info = (LPMEMALLOCINFO)info->list.next;
		}
	} while (info != gPageAllocList);

	return 0;
}

LPMEMALLOCINFO getFreePageItem() {
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gPageAllocList;

	int c = PAGE_INFO_PAGE_SIZE / sizeof(MEMALLOCINFO);
	for (int i = 0; i < c; i++)
	{
		if (info[i].size == 0 && info[i].addr == 0)
		{
			return &info[i];
		}
	}
	return 0;
}




extern "C"  __declspec(dllexport) DWORD __kPageAlloc(int size) {
	DWORD ret = 0;

	if (size % PAGE_SIZE)
	{
		return FALSE;
	}

	__enterSpinLock(&gPageAllocLock);

	int factor = 1;
	DWORD addr = gPageTableBase + size*factor;
	if (addr + size > gPageTableBase + PAGE_TABLE_SIZE)
	{
		goto __kPageAlloc_quit;
	}

	LPMEMALLOCINFO info = checkPageExist(addr);
	if (info == 0)
	{
		info = getFreePageItem();
		if (info)
		{
			info->size = size;
			info->addr = addr;
			LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
			info->pid = tss->pid;
			addlistTail((LPLIST_ENTRY)gPageAllocList, (LPLIST_ENTRY)info);
			ret = addr;
		}

		goto __kPageAlloc_quit;
	}

	while (1)
	{
		if (factor > 1 && info->size <= size)
		{
			for (int i = 0; i < factor - 1; i++)
			{
				addr += size;
				if (addr + size > gPageTableBase + PAGE_TABLE_SIZE)
				{
					goto __kPageAlloc_quit;
				}

				info = checkPageExist(addr);
				if (info == 0)
				{
					info = getFreePageItem();
					if (info)
					{
						info->size = size;
						info->addr = addr;
						LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
						info->pid = tss->pid;
						addlistTail((LPLIST_ENTRY)gPageAllocList, (LPLIST_ENTRY)info);
						ret = addr;
					}
					goto __kPageAlloc_quit;
				}
			}
		}

		factor = (factor << 1);
		addr = gPageTableBase + size * factor;
	}

__kPageAlloc_quit:
	__leaveSpinLock(&gPageAllocLock);

	return ret;
}

extern "C"  __declspec(dllexport) int __kFreePage(DWORD addr) {

	__enterSpinLock(&gPageAllocLock);

	LPMEMALLOCINFO info = checkPageExist(addr);
	if (info)
	{
		DWORD size = info->size;
		removelist((LPLIST_ENTRY)info);
		info->addr = 0;
		info->size = 0;
		info->vaddr = 0;
		info->pid = 0;
		__leaveSpinLock(&gPageAllocLock);
		return size;
	}

	__leaveSpinLock(&gPageAllocLock);

	char szout[1024];
	int len = __printf(szout, "__kFreePage not found address:%x\n", addr);
	__drawGraphChars((unsigned char*)szout, 0);
	return FALSE;
}






extern "C"  __declspec(dllexport) void __kPageFaultProc() {

	DWORD rcr2 = 0;
	DWORD rcr3 = 0;

	__asm {
		mov eax,cr2
		mov rcr2,eax

		mov eax,cr3
		mov rcr3,eax
	}

	
}



//make sure the first in the list is not to be deleted,or else will be locked
void freeProcessPages() {

	__enterSpinLock(&gPageAllocLock);
	
	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	LPMEMALLOCINFO info = (LPMEMALLOCINFO)gPageAllocList;
	do
	{
		if (info->pid == tss->pid)
		{
			removelist( (LPLIST_ENTRY)info);
			info->addr = 0;
			info->size = 0;
			info->pid = 0;
			info->vaddr = 0;
		}

		info = (LPMEMALLOCINFO)info->list.next;

	} while (info != gPageAllocList);

	__leaveSpinLock(&gPageAllocLock);
}