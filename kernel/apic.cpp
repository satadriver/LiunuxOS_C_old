

#include "apic.h"
#include "hardware.h"
#include "Utils.h"
#include "descriptor.h"

#include "task.h"
#include "cmosAlarm.h"
#include "cmosExactTimer.h"
#include "cmosPeriodTimer.h"
#include "core.h"

#define APIC_CORE_MAX_COUNT	64

DWORD * gApicBase = 0;
DWORD * gSvrBase = 0;
DWORD * gOicBase = 0;
DWORD * gHpetBase = 0;
DWORD * gRcbaBase = 0;

int g_ApNumber = 0;

int g_LocalApicID[APIC_CORE_MAX_COUNT];

int g_IoApicID[APIC_CORE_MAX_COUNT];


void enableRcba() {
	*gRcbaBase = *gRcbaBase | 1;
}

void enableFerr() {

	*gOicBase = *gOicBase | 0x200;
}

void iomfence() {
	__asm {
		mfence
	}
}

void setIoApicID(int id) {
	iomfence();
	*(DWORD*)(0xfee00000) = 0;
	iomfence();
	*(DWORD*)(0xfee00010) = (id & 0x0f) << 24 ;

}

void setIoRedirect(int id,int idx,int vector,int mode) {
	*(DWORD*)(0xfee00000) = idx ;
	iomfence();
	*(DWORD*)(0xfee00010) = vector + ( mode << 8);
	iomfence();
	*(DWORD*)(0xfee00000) = idx + 1;
	iomfence();
	*(DWORD*)(0xfee00010) = ((id & 0x0f) << 24) ;
}


void enableIoApic() {

	outportd(0xcf8, 0x8000f8f0);

	gRcbaBase = (DWORD*)(inportd(0xcfc) & 0xffffc000);

	gOicBase = (DWORD*)((DWORD)gRcbaBase + 0x31fe);

	DWORD v = *gOicBase;

	v = (v & 0xffffff00) | 0x100;

	*gOicBase = v;
}


//fec00000
//fed00000
//fee00000
DWORD* getOicBase() {
	outportd(0xcf8, 0x8000f8f0);
	DWORD rcba = inportd(0xcfc) & 0xffffc000;

	gOicBase = (DWORD*)(rcba + 0x31fe);

	return gOicBase;
}

DWORD* getRcbaBase() {
	outportd(0xcf8, 0x8000f8f0);
	DWORD base = inportd(0xcfc) & 0xffffc000;

	gRcbaBase = (DWORD*)base;

	return gRcbaBase;
}

int enableLocalApic() {

	DWORD high = 0;
	DWORD low = 0;
	int res = 0;
	readmsr(0x1b,&low,&high);
	low = low | 0xc00;
	writemsr(0x1b, low, high);

	gApicBase = (DWORD*)(low & 0xfffff000);

	gSvrBase = (DWORD*)((DWORD)gApicBase + 0xf0);
	*gSvrBase = *gSvrBase | 0x1100;

	int id = getLocalApicID();

	return id;
}

int enableHpet() {
	outportd(0xcf8, 0x8000f8f0);

	DWORD addr = inportd(0xcfc) & 0xffffc000;

	gHpetBase = (DWORD*)(addr + 0x3404);

	DWORD v = *gHpetBase;

	v = (v | 0x80) & 0xfffffffc;

	*gHpetBase = v;

	return 0;
}



void enableIMCR() {
	outportb(0x22, 0x70);
	outportb(0x23, 0x01);
}


int getLVTCount(int n) {
	int cnt = *(long long*)(0xfee00030) >>16;
	return cnt;
}

int getLocalApicVersion() {
	int ver = *(long long*)(0xfee00030) & 0xff;
	return ver;
}


//cpuid.01h:ebx[31:24]
int getLocalApicID() {
	__asm {
		mov eax,1
		mov ecx,0
		cpuid

		test ebx,0x200000
		jnz _x2apic

		shr ebx,24
		movzx eax,bl
		ret

		_x2apic:
		mov eax,0bh
		mov ecx, 0
		cpuid
		mov eax,edx
		
	}
}


extern "C" void __declspec(dllexport) __kBspInitProc() {

	enableLocalApic();

	int id = getLocalApicID();

	g_LocalApicID[g_ApNumber] = id;

	enableIoApic();

	setIoApicID(g_ApNumber);

	setIoRedirect(0x12, id, INTR_8259_MASTER + 1, 0);
	setIoRedirect(0x14, id, INTR_8259_MASTER , 0);
	setIoRedirect(0x16, id, INTR_8259_MASTER + 3, 0);
	setIoRedirect(0x18, id, INTR_8259_MASTER + 4, 0);
	setIoRedirect(0x1a, id, INTR_8259_MASTER + 5, 0);
	setIoRedirect(0x1c, id, INTR_8259_MASTER + 6, 0);
	setIoRedirect(0x1e, id, INTR_8259_MASTER + 7, 0);

	setIoRedirect(0x20, id, INTR_8259_SLAVE + 0, 0);
	setIoRedirect(0x22, id, INTR_8259_SLAVE + 1, 0);
	setIoRedirect(0x24, id, INTR_8259_SLAVE + 2, 0);
	setIoRedirect(0x26, id, INTR_8259_SLAVE + 3, 0);
	setIoRedirect(0x28, id, INTR_8259_SLAVE + 4, 0);
	setIoRedirect(0x2a, id, INTR_8259_SLAVE + 5, 0);
	setIoRedirect(0x2c, id, INTR_8259_SLAVE + 6, 0);
	setIoRedirect(0x2e, id, INTR_8259_SLAVE + 7, 0);

	setIoRedirect(0x30, id, INTR_8259_SLAVE + 8, 0);

	*(DWORD*)0xfee00300 = 0xc4500;	//���� INIT IPI, ʹ���� processor ִ�� INIT
	iomfence();

	DWORD addr = AP_INIT_ADDRESS >> 12;

	*(DWORD*)0xfee00300 = 0xc4600 | addr;	//���� Start - up IPI��AP����ʼ�����ַΪ0x1B000
	iomfence();

	*(DWORD*)0xfee00300 = 0xc4600 | addr;	//�ٴη��� Start - up IPI��AP����ʼ�����ַΪ0x1B000
	iomfence();

	enableIMCR();

	outportb(0x21, 0xff);
	outportb(0xa1, 0xff);

	*(DWORD*)0xfee00350 = 0x10000;

	initHpet();
}


int initHpet() {
	int res = 0;

	enableHpet();

	long long id = *(long long*)APIC_HPET_BASE;

	HPET_GCAP_ID_REG * gcap = (HPET_GCAP_ID_REG*)&id;
	if (gcap->tick == 0x0429b17f) {
		int cnt = gcap->count;

		DWORD tick = gcap->tick;

		long long total = 143182;		// 14318179 = 1000ms,0x0429b17f

		*(long long*)(APIC_HPET_BASE + 0x10) = 3;

		*(long long*)(APIC_HPET_BASE + 0x20) = 0;

		long long* regs = (long long*)(APIC_HPET_BASE + 0x100);	

		for (int i = 0; i < cnt; i++) {
			if (i == 0) {
				regs[i] = 0x40 + 8 + 4 + 2 - 2;
			}
			else if (i == 2) {
				regs[i] = 0x1000 + 0x40 + 4 + 2 - 2;
			}
			else if (i % 2 == 0) {
				regs[i] = 0x40 + 2 - 2;
			}
			else {
				regs[i] = total;
			}
			i += 2;
		}

		*(long long*)(APIC_HPET_BASE + 0xf0) = 0;

		return TRUE;
	}

	return FALSE;
}







extern "C" void __declspec(naked) HpetInterrupt(LIGHT_ENVIRONMENT * stack) {
	__asm {
		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
	}

	{
		LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
		char szout[1024];

		int tag = *(int*)(APIC_HPET_BASE + 0x20);
		if (tag & 1) {
			__kTaskSchedule((LIGHT_ENVIRONMENT*)stack);
			*(long*)(APIC_HPET_BASE + 0x108) = 0;
		}
		else if (tag & 2) {
			DWORD c = *(long*)(APIC_HPET_BASE + 0xf0);
			DWORD cmp = *(long*)(APIC_HPET_BASE + 0x128);
		}

		*(long *)0xFEE000B0 = 0;
	}

	__asm {
#ifdef SINGLE_TASK_TSS
		mov eax, dword ptr ds : [CURRENT_TASK_TSS_BASE + PROCESS_INFO.tss.cr3]
		mov cr3, eax
#endif

		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad
#ifdef SINGLE_TASK_TSS
		mov esp, dword ptr ss : [esp - 20]
#endif	

		clts
		iretd

		jmp HpetInterrupt
	}
}



extern "C" void __declspec(dllexport) __kApInitProc_old() {
	DescriptTableReg idtbase;
	idtbase.size = 256 * sizeof(SegDescriptor) - 1;
	idtbase.addr = IDT_BASE;

	initKernelTss((TSS*)AP_TSS_BASE + 0x4000*g_ApNumber, AP_STACK0_BASE + TASK_STACK0_SIZE* (g_ApNumber+1) - STACK_TOP_DUMMY,
		TASKS_AP_STACK_BASE + KTASK_STACK_SIZE*(g_ApNumber + 1)- STACK_TOP_DUMMY, 0, PDE_ENTRY_VALUE, 0);
	makeTssDescriptor(AP_TSS_BASE + 0x4000 * g_ApNumber, 3, sizeof(TSS) - 1, 
		(TssDescriptor*)(GDT_BASE + AP_TSS_DESCRIPTOR + sizeof(TssDescriptor) * g_ApNumber));

	DescriptTableReg gdtbase;
	__asm {
		sgdt gdtbase
	}

	DWORD lptssdesc = (DWORD)(AP_TSS_DESCRIPTOR + sizeof(TssDescriptor) * g_ApNumber);

	gdtbase.addr = GDT_BASE;
	gdtbase.size = AP_TSS_DESCRIPTOR + sizeof(TssDescriptor) * g_ApNumber - 1;
	__asm{
		lgdt gdtbase

		mov eax, lptssdesc
		ltr ax

		mov ax, ldtSelector
		lldt ax

		lidt idtbase
	}

	enableLocalApic();

	int id = getLocalApicID();

	g_LocalApicID[g_ApNumber+1] = id;

	enableIoApic();

	setIoApicID(g_ApNumber+1);

	g_ApNumber++;

	char szout[1024];
	__printf(szout, "idt base:%x,size:%x\r\n", idtbase.addr, idtbase.size);

	*(DWORD*)0xFEE00310 = 0;
	*(DWORD*)0xFEE00300 = 0x4030;

	__asm {
		sti
		hlt
	}
}

/*
����ת��Short Jmp��ֻ����ת��256�ֽڵķ�Χ�ڣ�����Ӧ�����룺EB
����ת��Near Jmp��������ͬһ�η�Χ�ڵĵ�ַ������Ӧ�����룺E9
����ת��Near call��������ͬһ�η�Χ�ڵĵ�ַ������Ӧ�����룺E8
Զ��ת��Far Jmp�������������ַ������Ӧ�����룺 EA
Զ��ת��Far call�������������ַ������Ӧ�����룺 9A
ff 15 call
ff 25 call
*/




extern "C" void __declspec(naked) IPIIntHandler(LIGHT_ENVIRONMENT * stack) {
	__asm {
		pushad
		push ds
		push es
		push fs
		push gs
		push ss

		push esp
		sub esp, 4
		push ebp
		mov ebp, esp

		mov eax, KERNEL_MODE_DATA
		mov ds, ax
		mov es, ax
		MOV FS, ax
		MOV GS, AX
	}

	{
		*(DWORD*)0xFEE000B0 = 0;
	}

	__asm {
		mov esp, ebp
		pop ebp
		add esp, 4
		pop esp
		pop ss
		pop gs
		pop fs
		pop es
		pop ds
		popad

		clts
		iretd
	}
}



void __kApInitProc() {
	char szout[1024];
	__printf(szout, "__kApInitProc start\r\n");

	DescriptTableReg gdtbase;
	__asm {
		cli

		sgdt gdtbase
	}

	gdtbase.addr = GDT_BASE;
	//gdtbase.size = 0x7f;

	__asm {
		//do not use lgdt lpgdt,why?
		lgdt gdtbase

		mov eax, PDE_ENTRY_VALUE
		mov cr3, eax

		mov eax, cr0
		or eax, 0x80000000
		mov cr0, eax
	}

	int seq = *(DWORD*) 0xFEE00020;
	seq = seq >> 24;

	
	__printf(szout,"AP:%d ready\r\n",seq);

	__asm {
		sti
		hlt
	}
}





void BPCodeStart() {

	DWORD v = *(DWORD*)0xFEE000F0;
	v = v | 0x100;
	*(DWORD*)0xFEE000F0 = v;

	v = 0xc4500;
	*(DWORD*)0xFEE00300 = v;

	for (int i = 0; i < 0x10000; i++) {
		;
	}

	v = 0xc4600 | (AP_INIT_ADDRESS >> 12);
	*(DWORD*)0xFEE00300 = v;

	for (int i = 0; i < 0x10000; i++) {
		;
	}

	while (1) {
		break;
	}

	return;
}