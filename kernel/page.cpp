#include "page.h"
#include "Utils.h"
#include "video.h"
#include "malloc.h"
#include "process.h"



LPMEMALLOCINFO gPageAllocList = 0;

DWORD gPageAllocLock = FALSE;

DWORD gPageTableBase = 0;


void initPage() {
	gPageAllocList = (LPMEMALLOCINFO)PAGE_ALLOC_LIST;
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

	int c = PAGE_ALLOC_LIST_SIZE / sizeof(MEMALLOCINFO);
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
	DWORD res = 0;

	if (size % PAGE_SIZE)
	{
		return FALSE;
	}

	int factor = 1;
	DWORD addr = gPageTableBase + size*factor;
	if (addr + size > gPageTableBase + PAGE_TABLE_SIZE)
	{
		return FALSE;
	}

	__enterSpinLock(&gPageAllocLock);

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
			res = addr;
		}
		else {
			res = -1;
		}
	}
	else {
		while (1)
		{
			if (factor > 1 && info->size <= size)
			{
				for (int i = 0; i < factor - 1; i++)
				{
					addr += size;
					if (addr + size > gPageTableBase + PAGE_TABLE_SIZE)
					{
						res = -1;
						break;
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
				//noting to do
			}

			if (res == 0) {
				factor = (factor << 1);
				addr = gPageTableBase + size * factor;
			}
			else {
				break;
			}
		}
	}

	__leaveSpinLock(&gPageAllocLock);

	if (res == -1) {
		res = 0;
	}

	return res;
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


//R/W--λ1�Ƕ�/д��Read/Write����־���������1����ʾҳ����Ա�����д��ִ�С����Ϊ0����ʾҳ��ֻ�����ִ�С�
//�������������ڳ����û���Ȩ��������0��1��2��ʱ����R/Wλ�������á�ҳĿ¼���е�R/Wλ������ӳ�������ҳ�������á�
//U/S--λ2���û�/�����û���User / Supervisor����־�����Ϊ1����ô�������κ���Ȩ���ϵĳ��򶼿��Է��ʸ�ҳ�档
//���Ϊ0����ôҳ��ֻ�ܱ������ڳ����û���Ȩ����0��1��2���ϵĳ�����ʡ�ҳĿ¼���е�U / Sλ������ӳ�������ҳ�������á�

void initPageTable() {

	DWORD* idx = (DWORD*)PTE_ENTRY_VALUE;
	DWORD* entry = (DWORD*)PDE_ENTRY_VALUE;

	DWORD buf = PAGE_PRESENT | PAGE_READWRITE| PAGE_USERPRIVILEGE;
	for (int i = 0; i < ITEM_IN_PAGE; i++) {
		entry[i] = (DWORD)idx | (PAGE_PRESENT | PAGE_READWRITE | PAGE_USERPRIVILEGE);

		for (int j = 0; j < ITEM_IN_PAGE; j++) {
			idx[j] = buf;
			buf += PAGE_SIZE;
		}
		idx += ITEM_IN_PAGE;
	}
}


void enablePage() {

	initPageTable();

	__asm {
		mov eax, PDE_ENTRY_VALUE
		mov cr3,eax

		mov eax,cr0
		or eax,0x80000000
		mov cr0,eax
	}

}