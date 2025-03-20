
#include "def.h"
#include "process.h"
#include "Utils.h"
#include "Pe.h"
#include "malloc.h"
#include "page.h"
#include "video.h"
#include "Utils.h"
#include "memory.h"

//��ִ�г��������в��ִ�������������Ե�ַ�ռ�������ַ�ռ��о�����ͬ��ַʱ�����ǲ��ܸı�PGλ�����á�
//��ʱ�ⲿ�־�����ͬ��ַ�Ĵ����ڷ�ҳ��δ��ҳ����֮���������������á������Ƿ�����ҳ���ƣ��ⲿ�ִ��붼������ͬ�ĵ�ַ��
//���⣬�ڿ�����ҳ��PG=1��֮ǰ������ˢ��ҳ���ٻ���TLB
//CR3���д��ҳĿ¼��ҳ��������ַ��ע�⣬�������ַ�������������CR3Ҳ����ΪPDBR��
//��ΪҳĿ¼��ҳ����ҳ����ģ����ԸüĴ���ֻ�и�20λ����Ч�ġ�
//����12λ���������߼�������ʹ�ã��������CR3�м���һ����ֵʱ��12λ��������Ϊ0��

//�������л�ʱ�������������ѻ�������ļĴ���CR3��LDTR�����ݱ��浽TSS�еĵ�ַӳ��Ĵ�������
//��ʵ�ϣ�������Ҳ��������������Զ�д�롣��ˣ��������ı���LDTR��CR3����ô�������ֵ��Ϊ�ر��浽TSS�еĵ�ַӳ��Ĵ���������Ӧ�ֶ��С�

//����ϵͳ��洢һ��ȫ����������GDT���ڴ��У�������GDT��ͨ��GDTR�Ĵ���ָ������λ�õġ�
//���ǣ�GDTR�еĵ�ַ�����Ե�ַ��Ҳ����˵����������˷�ҳ���ƵĻ�����Ҫ����CR3�Ĵ����İ���������ӳ�䵽�����ڴ��ַ���Ӷ��ҵ�GDT







int clearCR3(DWORD *cr3) {
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




DWORD copyKernelCR3(DWORD addr, DWORD size, DWORD *cr3) {

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

	DWORD* kcr3 = (DWORD*)PDE_ENTRY_VALUE;

	if (size == 0 && addr == 0) {
		tablecnt = ITEM_IN_PAGE;
		tboffset = 0;
	}

	for (DWORD i = tboffset; i <tboffset + tablecnt; i++)
	{
		cr3[i] = kcr3[i];
	}

	return tablecnt;
}

DWORD mapPhyToLinear(DWORD linearaddr, DWORD physaddr, DWORD size, DWORD * cr3,int tag) {

	char szout[1024];

	if (size % PAGE_SIZE)
	{
		return FALSE;
	}

	tag = tag | PAGE_READWRITE | PAGE_PRESENT;

	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	int tablecnt = size / tablesize;
	if (size % tablesize)
	{
		tablecnt++;
	}

	DWORD tboffset = linearaddr / tablesize;

	DWORD pgoffset = (linearaddr / PAGE_SIZE) % ITEM_IN_PAGE;

	DWORD remapTotal = size / PAGE_SIZE;

	DWORD remapcnt = 0;

	DWORD phyaddr = physaddr;

	for (DWORD i = tboffset; i < tboffset + tablecnt; i++)
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

			cr3[i] = (DWORD)pagetable | (PAGE_PRESENT| (tag ));
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
			pagetable[j] = phyaddr | (PAGE_PRESENT | (tag ));
			remapcnt++;
			if (remapcnt >= remapTotal)
			{
				return remapcnt;
				break;
			}

			phyaddr += PAGE_SIZE;
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
			DWORD offsetInPage = linearAddr & PAGE_INSIZE_MASK;
			DWORD addr = (tablepage[pgoffset] & PAGE_MASK) + offsetInPage;

			return addr;
		}
	}
	return 0;
}


DWORD linear2phy(DWORD linearAddr) {
	return linearAddr;

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	int pid = process->pid;
	return linear2phy(linearAddr, pid);
}



DWORD getTbPgOff(DWORD phyaddr, DWORD * tboff, DWORD *pgoff) {
	int tablesize = ITEM_IN_PAGE*PAGE_SIZE;

	DWORD tboffset = phyaddr / tablesize;
	DWORD pgoffset = (phyaddr / PAGE_SIZE) % ITEM_IN_PAGE;
	*tboff = tboffset;
	*pgoff = pgoffset;
	return TRUE;
}

