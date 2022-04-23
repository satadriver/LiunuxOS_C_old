
#include "def.h"
#include "process.h"
#include "Utils.h"
#include "Pe.h"
#include "slab.h"
#include "page.h"
#include "video.h"
#include "Utils.h"
#include "memory.h"

//当执行程序至少有部分代码和数据在线性地址空间和物理地址空间中具有相同地址时，我们才能改变PG位的设置。
//此时这部分具有相同地址的代码在分页和未分页世界之间起着桥梁的作用。无论是否开启分页机制，这部分代码都具有相同的地址。
//另外，在开启分页（PG=1）之前必须先刷新页高速缓冲TLB
//CR3含有存放页目录表页面的物理地址（注意，是物理地址！！！），因此CR3也被称为PDBR。
//因为页目录表页面是页对齐的，所以该寄存器只有高20位是有效的。而低12位保留供更高级处理器使用，因此在往CR3中加载一个新值时低12位必须设置为0。

//在任务切换时，处理器并不把换出任务的寄存器CR3和LDTR的内容保存到TSS中的地址映射寄存器区域。
//事实上，处理器也从来不向该区域自动写入。因此，如果程序改变了LDTR或CR3，那么必须把新值人为地保存到TSS中的地址映射寄存器区域相应字段中。

//我们知道，操作系统会存储一张全局描述符表GDT在内存中，而这张GDT是通过GDTR寄存器指出它的位置的。
//但是，GDTR中的地址是线性地址，也就是说，如果开启了分页机制的话，需要经过CR3寄存器的帮助，才能映射到物理内存地址，从而找到GDT
DWORD initCr3(DWORD pefiledata) {
	char szout[1024];

	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	DWORD pedata = pefiledata;

	DWORD allocsize = getSizeOfImage((char*)pedata);
	allocsize = getAlignSize(allocsize, 1);

	int tablecnt = allocsize / tablesize;
	if (allocsize % tablesize)
	{
		tablecnt++;
	}

	DWORD mappgTotal = allocsize / PAGE_SIZE;

	int mappgcnt = 0;

	DWORD imagebase = getImageBase((char*)pedata);
	DWORD tboffset = imagebase / tablesize;
	DWORD pgoffset = (imagebase / PAGE_SIZE) % ITEM_IN_PAGE;

	DWORD * cr3 = (DWORD*)__kPageAlloc(PAGE_SIZE);

	// 	__printf(szout, "initCr3:%x,pe:%x,imagebase:%x,imagesize:%x,table offset:%x,page offset:%x\r\n",
	// 		cr3,pedata, imagebase, allocsize,tboffset,pgoffset);
	// 	__drawGraphChars((unsigned char*)szout, 0);

	for (int i = tboffset; i < tboffset + tablecnt; i++)
	{
		DWORD * pagetable = (DWORD*)__kPageAlloc(PAGE_SIZE);
		if (pagetable == 0)
		{
			clearCr3(cr3);
			return FALSE;
		}
		__memset((char*)pagetable, 0, PAGE_SIZE);

		cr3[i] = (DWORD)pagetable | 7;

		int j = 0;
		if (i == tboffset)
		{
			j = pgoffset;
		}
		else {
			j = 0;
		}

		for (; j < ITEM_IN_PAGE; j++)
		{
			pagetable[j] = (DWORD)pedata | 7;

// 			__printf(szout, "remap cr3[%d]:%x[%d]:%x\r\n", i, (DWORD)pagetable, j,(DWORD)pedata | 7);
// 			__drawGraphChars((unsigned char*)szout, 0);

			pedata += PAGE_SIZE;

			mappgcnt++;
			if (mappgcnt >= mappgTotal)
			{
				break;
			}
		}
	}

	return (DWORD)cr3;
}




int clearCr3(DWORD *cr3) {
	int cnt = 0;

	if (cr3)
	{
		for (int i = 0; i < ITEM_IN_PAGE; i++)
		{
			DWORD * p = (DWORD*)(cr3[i] & PAGE_MASK);
			if (p)
			{
				for (int j = 0; j < ITEM_IN_PAGE; j++)
				{
					DWORD * page = (DWORD *)(p[j] & PAGE_MASK);
					if (page)
					{
						__kFree((DWORD)page);
						cnt++;
					}
				}
				__kFreePage((DWORD)p);
			}
		}
		__kFreePage((DWORD)cr3);
	}
	return cnt;
}


DWORD getCurrentCr3() {
	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	return process->tss.cr3;
}


DWORD copyBackupTables(DWORD addr, DWORD size, DWORD *tables) {

	if (size % PAGE_SIZE)
	{
		return FALSE;
	}

	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	int tablecnt = size / tablesize;
	if (size % tablesize)
	{
		tablecnt++;
	}

	DWORD tboffset = addr / tablesize;
	//DWORD pgoffset = (phyaddr / PAGE_SIZE) % ITEM_IN_PAGE;

	DWORD * backcr3 = (DWORD*)BACKUP_PAGE_TABLES;

	for (int i = tboffset; i <tboffset + tablecnt; i++)
	{
		tables[i] = backcr3[i];
	}

	return tablecnt;
}

DWORD phy2linear(DWORD linearaddr, DWORD physicaladdr, DWORD physize, DWORD * cr3) {

	char szout[1024];

	if (physize % PAGE_SIZE)
	{
		return FALSE;
	}

	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	int tablecnt = physize / tablesize;
	if (physize % tablesize)
	{
		tablecnt++;
	}

	DWORD tboffset = linearaddr / tablesize;
	DWORD pgoffset = (linearaddr / PAGE_SIZE) % ITEM_IN_PAGE;

	DWORD remapTotal = physize / PAGE_SIZE;

	DWORD remapcnt = 0;

	DWORD phyaddr = physicaladdr;

	for (int i = tboffset; i < tboffset + tablecnt; i++)
	{
		DWORD * pagetable = (DWORD *)(cr3[i] & PAGE_MASK);
		if (pagetable == 0)
		{
			pagetable = (DWORD*)__kPageAlloc(PAGE_SIZE);
			if (pagetable == FALSE)
			{
				return FALSE;
			}
			__memset((char*)pagetable, 0, PAGE_SIZE);

			cr3[i] = (DWORD)pagetable | 7;
		}
		

		int j = 0;
		if (i == tboffset)
		{
			j = pgoffset;
		}
		else {
			j = 0;
		}

		for (; j < ITEM_IN_PAGE; j++)
		{
			pagetable[j] = phyaddr | 7;

			phyaddr += PAGE_SIZE;

			remapcnt++;
			if (remapcnt >= remapTotal)
			{
				break;
			}
		}
	}

	return remapcnt;
}

DWORD linear2phy(DWORD linearAddr,int pid) {

	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	DWORD tboffset = linearAddr / tablesize;
	DWORD pgoffset = (linearAddr / PAGE_SIZE) % ITEM_IN_PAGE;

	LPPROCESS_INFO base = (LPPROCESS_INFO)TASKS_TSS_BASE;
	LPPROCESS_INFO process = base + pid;
	DWORD * cr3 = (DWORD *)process->tss.cr3;

	if (cr3[tboffset] & 1)
	{
		DWORD * tablepage = (DWORD *)(cr3[tboffset] & PAGE_MASK);
		if (tablepage[pgoffset] & 1)
		{
			DWORD offsetInPage = linearAddr & PAGE_IN_MASK;
			DWORD addr = (tablepage[pgoffset] & PAGE_MASK) + offsetInPage;

			return addr;
		}
	}
	return 0;
}


DWORD linear2phy(DWORD linearAddr) {

	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	DWORD tboffset = linearAddr / tablesize;
	DWORD pgoffset = (linearAddr / PAGE_SIZE) % ITEM_IN_PAGE;

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	DWORD * cr3 = (DWORD *)process->tss.cr3;

	if (cr3[tboffset] & 1)
	{
		DWORD * tablepage = (DWORD *)(cr3[tboffset] & PAGE_MASK);
		if (tablepage[pgoffset] & 1)
		{
			DWORD offsetInPage = linearAddr & PAGE_IN_MASK;
			DWORD addr = (tablepage[pgoffset] & PAGE_MASK) + offsetInPage;

			return addr;
		}
	}
	return 0;
}


DWORD checkPhysicalAddrExist(DWORD linearAddr) {

	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	DWORD tboffset = linearAddr / tablesize;
	DWORD pgoffset = (linearAddr / PAGE_SIZE) % ITEM_IN_PAGE;

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	DWORD * cr3 = (DWORD *)process->tss.cr3;

	if (cr3[tboffset] & 1)
	{
		DWORD * tablepage = (DWORD *)(cr3[tboffset] & PAGE_MASK);
		if (tablepage[pgoffset] & 1)
		{
			DWORD offsetInPage = linearAddr & PAGE_IN_MASK;
			DWORD addr = (tablepage[pgoffset] & PAGE_MASK) + offsetInPage;

			return addr;
		}
		else {
			return PAGE_TABLE_NOTEXIST;
		}
	}
	else {
		return PAGE_TABLE_INDEX_NOTEXIST;
	}
}


DWORD getTbPgOff(DWORD phyaddr, DWORD * tboff, DWORD *pgoff) {
	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	DWORD tboffset = phyaddr / tablesize;
	DWORD pgoffset = (phyaddr / PAGE_SIZE) % ITEM_IN_PAGE;
	*tboff = tboffset;
	*pgoff = pgoffset;
	return TRUE;
}

