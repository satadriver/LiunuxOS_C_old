#include "page.h"
#include "Utils.h"
#include "video.h"
#include "malloc.h"
#include "process.h"
#include "memory.h"


//BIOS:
//https://blog.csdn.net/x86ipc/article/details/5303760

DWORD gPageAllocLock = FALSE;

LPMEMALLOCINFO gPageAllocList = 0;

LPMEMALLOCINFO findPageIdx(DWORD addr) {

	LPMEMALLOCINFO base = (LPMEMALLOCINFO)PAGE_ALLOC_LIST;
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)(base->list.next);
	LPMEMALLOCINFO hdr = info;

	do
	{
		if (info == 0) {
			break;
		}

		if (info->addr == addr)
		{
			return info;
		}
		else {
			info = (LPMEMALLOCINFO)info->list.next;
		}
	} while (info && (info != hdr));

	return 0;
}

LPMEMALLOCINFO isPageIdxExist(DWORD addr,int size) {

	LPMEMALLOCINFO base = (LPMEMALLOCINFO)PAGE_ALLOC_LIST;
	LPMEMALLOCINFO info = (LPMEMALLOCINFO)(base->list.next);
	LPMEMALLOCINFO hdr = info;

	do
	{
		if (info == 0) {
			break;
		}

		if (info->addr == addr)
		{
			return info;
		}
		else if ( (info->addr < addr) && (info->addr + info->size > addr) ) 
		{
			return info;
		}
		else {
			info = (LPMEMALLOCINFO)info->list.next;
		}
	} while (info && (info != hdr));

	return 0;
}

LPMEMALLOCINFO getFreePageIdx() {

	LPMEMALLOCINFO info = (LPMEMALLOCINFO)PAGE_ALLOC_LIST;

	int cnt = PAGE_ALLOC_LIST_SIZE / sizeof(MEMALLOCINFO);
	for (int i = 1; i < cnt; i++)
	{
		if (info[i].size == 0 && info[i].addr == 0 && info[i].pid == 0 && info[i].vaddr == 0)
		{
			return &info[i];
		}
	}
	return 0;
}


int resetPageIdx(LPMEMALLOCINFO pde) {
	DWORD size = pde->size;
	removelist(&gPageAllocList->list,(LPLIST_ENTRY)&pde->list);
	pde->addr = 0;
	pde->size = 0;
	pde->vaddr = 0;
	pde->pid = 0;
	return size;
}



int insertPageIdx(LPMEMALLOCINFO info, DWORD addr, int size,int pid,DWORD vaddr ) {
	info->size = size;
	info->addr = addr;
	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	info->pid = tss->pid;

	addlistTail( &((LPMEMALLOCINFO)PAGE_ALLOC_LIST)->list, (LPLIST_ENTRY)&info->list);
	return TRUE;
}



extern "C"  __declspec(dllexport) DWORD __kPageAlloc(int size) {
	DWORD res = 0;

	if (size % PAGE_SIZE)
	{
		return FALSE;
	}

	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	__enterSpinlock(&gPageAllocLock);

	int factor = 1;
	while (1)
	{
		for (int n = factor / 2; n && n < factor; )
		{
			DWORD addr = PAGE_TABLE_BASE + size * n;
			if (addr + size > PAGE_TABLE_BASE + PAGE_TABLE_SIZE)
			{
				res = -1;
				break;
			}

			LPMEMALLOCINFO info = isPageIdxExist(addr, size);
			if (info == 0)
			{
				info = getFreePageIdx();
				if (info)
				{
					insertPageIdx(info, addr, size, tss->pid, addr);
					res = addr;
				}
				else {
					res = -1;
				}
				break;
			}
			else {
				if (info->size > (DWORD)size) {
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

		if (res == 0) {
			factor = (factor << 1);
		}
		else {
			break;
		}
	}
	
	__leaveSpinlock(&gPageAllocLock);

	if (res == -1) {
		res = 0;
	}

	return res;
}






extern "C"  __declspec(dllexport) int __kFreePage(DWORD addr) {

	__enterSpinlock(&gPageAllocLock);

	LPMEMALLOCINFO info = findPageIdx(addr);
	if (info)
	{
		DWORD size = resetPageIdx(info);
	}
	else {
		char szout[1024];
		int len = __printf(szout, "__kFreePage not found address:%x\n", addr);
	}
	__leaveSpinlock(&gPageAllocLock);

	return FALSE;
}



//make sure the first in the list is not to be deleted,or else will be locked
void freeProcessPages(int pid) {

	__enterSpinlock(&gPageAllocLock);
	
	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;

	LPMEMALLOCINFO base = (LPMEMALLOCINFO)(LPMEMALLOCINFO)gPageAllocList->list.next;
	LPMEMALLOCINFO info = base;
	do
	{
		if (info->pid == pid)
		{
			resetPageIdx(info);
		}

		info = (LPMEMALLOCINFO)info->list.next;

	} while (info != (LPMEMALLOCINFO)base);

	__leaveSpinlock(&gPageAllocLock);
}





void linearMapping() {

	DWORD* idx = (DWORD*)PTE_ENTRY_VALUE;
	DWORD* entry = (DWORD*)PDE_ENTRY_VALUE;

	DWORD buf = PAGE_PRESENT | PAGE_READWRITE| PAGE_USERPRIVILEGE;
	__memset((char*)PDE_ENTRY_VALUE, 0, PAGE_SIZE);

#if 0
//#ifndef DISABLE_PAGE_MAPPING
	mapPhyToLinear(0, 0, 0x80000000, (DWORD*)PDE_ENTRY_VALUE, PAGE_PRESENT | PAGE_READWRITE | PAGE_USERPRIVILEGE);
	mapPhyToLinear(0x80000000, 0x80000000, 0x80000000, (DWORD*)PDE_ENTRY_VALUE, PAGE_PRESENT | PAGE_READWRITE | PAGE_USERPRIVILEGE);
	return;

	int cnt = MEMMORY_ALLOC_BASE / (PAGE_SIZE * ITEM_IN_PAGE);
	for (int i = 0; i < cnt; i++)
	{
		//entry[i] = (DWORD)idx | (PAGE_PRESENT | PAGE_READWRITE | PAGE_USERPRIVILEGE);
		idx += ITEM_IN_PAGE;
	}
	mapPhyToLinear(0, 0, MEMMORY_ALLOC_BASE, (DWORD*)PDE_ENTRY_VALUE, PAGE_PRESENT | PAGE_READWRITE | PAGE_USERPRIVILEGE);

	int begin = MEMMORY_ALLOC_BASE / (PAGE_SIZE * ITEM_IN_PAGE);
	int end = (getBorderAddr() & (~(PAGE_SIZE * ITEM_IN_PAGE - 1))) / (PAGE_SIZE * ITEM_IN_PAGE);
	for (int i = begin; i < end; i++)
	{
		for (int k = 0; i < ITEM_IN_PAGE; k++) {

		}
		char * pageidx =(char*) (entry[i]&0xfffff000);
		__memset(pageidx, 0, PAGE_SIZE);
	}

	mapPhyToLinear(0xc0000000, 0xc0000000, 0x40000000, (DWORD*)PDE_ENTRY_VALUE, PAGE_PRESENT | PAGE_READWRITE | PAGE_USERPRIVILEGE);

	return;
#endif

	int start = MEMMORY_ALLOC_BASE / (PAGE_SIZE * ITEM_IN_PAGE);
	int highaddr = (getBorderAddr() ) & (~((PAGE_SIZE * ITEM_IN_PAGE) - 1));
	int stop = highaddr / (PAGE_SIZE * ITEM_IN_PAGE);

	for (int i = 0; i < ITEM_IN_PAGE; i++) {
		entry[i] = (DWORD)idx | (PAGE_PRESENT | PAGE_READWRITE | PAGE_USERPRIVILEGE);
		for (int j = 0; j < ITEM_IN_PAGE; j++) {
			if (i >= start && i < stop) {
				idx[j] = 0;
				idx[j] = buf;
			}
			else {
				idx[j] = buf;
			}
			
			buf += PAGE_SIZE;
		}

		idx += ITEM_IN_PAGE;
	}


	for (int i = start; i < stop; i++)
	{
		char* pageidx = (char*)(entry[i] & 0xfffff000);
		//__memset(pageidx, 0, PAGE_SIZE);
	}

}

/*
 �����������Ỻ����ЩPλ��1����Щҳ������ң�TLB�Ĺ�����CR3�Ĵ�����PCDλ��PWT���޹صġ�
 ����ҳ������޸Ĳ���ͬʱ��ӳ��TLB�У�һ��Ҫˢ��TLB����Ȼ��ҳ������þ�����Ч�ġ�
 TLB���������ֱ�ӷ��ʵģ�ֻ��ͨ����ʽˢ��CR3�����������л���ʽˢ��TLB��
 ����Ҫע����ǣ�������ˢ�·���������Щ���Ϊȫ�֣�G=1����ҳ����Ч��

 TLB�����Ե���ˢ�£�����invlpg���invalidate TLB Entry����
 invlpg�ĸ�ʽΪinvlpg m32����ִ������ָ���ʱ�򣬴��������ø��������Ե�ַ����TLB���ҵ��Ǹ���Ŀ��Ȼ����ڴ������¼��������ݵ���Ӧ��TLBҳ��������
 invlpg����Ȩָ�����Ҫ��CPLΪ��Ȩ0��ִ�У���ָ�Ӱ���κα�־λ��
*/

void initPaging() {

	gPageAllocList = (LPMEMALLOCINFO)PAGE_ALLOC_LIST;
	LPMEMALLOCINFO pageList = (LPMEMALLOCINFO)PAGE_ALLOC_LIST;
	initListEntry(&pageList->list);
	pageList->addr = 0;
	pageList->size = 0;
	pageList->vaddr = 0;
	pageList->pid = 0;

	linearMapping();

	__asm {
		mov eax, PDE_ENTRY_VALUE
		mov cr3, eax

		mov eax, cr0
		or eax, 0x80000000
		mov cr0, eax
	}
}