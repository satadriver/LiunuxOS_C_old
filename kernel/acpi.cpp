
#include "acpi.h"
#include "hardware.h"
#include "video.h"


DWORD gHPET_EOI = 0;

static const ACPIFadt *acpiFadt = 0;

static const ACPIHeader *acpiSsdt = 0;

static const ACPIHeader *acpiDsdt = 0;


void heptEOI() {
	if (gHPET_EOI)
	{
		DWORD base = 0xfed00000;
		unsigned __int64 * gintr_sta = (unsigned __int64 *)(base + 0x20);
		*gintr_sta = 0xff;
	}
}


void initHPET() {
	DWORD base = 0xfed00000;

	unsigned __int64 * gcap_id = (unsigned __int64 *)(base + 0);
	unsigned __int64 counter = (*gcap_id >> 32);
	unsigned short provider = (*gcap_id & 0xffff0000) >> 16;
	if (counter == 0x0429b17f)
	{
		unsigned __int64 * gen_conf = (unsigned __int64 *)(base + 0x10);

		unsigned __int64 * gintr_sta = (unsigned __int64 *)(base + 0x20);

		unsigned __int64 * main_cnt = (unsigned __int64 *)(base + 0xf0);

		unsigned __int64 * tim0_conf = (unsigned __int64 *)(base + 0x100);

		unsigned __int64 * tim0_comp = (unsigned __int64 *)(base + 0x108);

		unsigned __int64 * tim1_conf = (unsigned __int64 *)(base + 0x120);

		unsigned __int64 * tim1_comp= (unsigned __int64 *)(base + 0x128);

		*gen_conf = 3;

		*gintr_sta = 0;

		*tim0_conf = 0x4c;
		if (*tim0_conf & 2)
		{
			gHPET_EOI = TRUE;
		}

		*tim0_comp = 14300;	//69.841279ns

		*main_cnt = 0;
	}
}


//fec00000
//fed00000
//fee00000
DWORD getRCBA() {
	outportd(0xcf8, 0x8000f8f0);
	DWORD addr = inportd(0xcfc);
	addr = addr & 0xffffc000;
	if (addr >= 0xfec00000 && addr <= 0xfee00000)
	{
		DWORD * v = (DWORD*)(addr + 0x3404);
		*v = 0x80;

		initHPET();

		char szout[1024];
		__printf(szout, "RCBA:%x\n", v);

	}

	return TRUE;
}


//bit 9:enable irq 13
//bit 8:enable apic io
DWORD enableFloatIRQ() {
	outportd(0xcf8, 0x8000f8f0);
	DWORD addr = inportd(0xcfc);
	addr = addr & 0xffffc000;

	DWORD * v = (DWORD*)(addr + 0x31fe);
	*v = *v | 0x200;

	char szout[1024];
	__printf(szout, "OIC:%x\n", v);
	return TRUE;
}

static int parseApic(ACPIHeaderApic *apic)
{
	char szout[1024];
	char temp[20];
	temp[0] = '0'; 
	temp[1] = 'x';
	//atoi(apic->localApicAddress, temp + 2, 0x10, 8, '0', 1);
	//__printf("\nLocal Apic Address: %s\n", temp);
	u8*localApicAddress = (u8 *)apic->localApicAddress;

	u8 *start = (u8*) apic + sizeof(ACPIHeaderApic);
	u8 *end = ((u8 *)apic) + apic->header.length;
	while (start < end)
	{
		ApicHeader *apicHeader = (ApicHeader *)start;
		u8 type = apicHeader->type;
		u8 length = apicHeader->length;
		switch (type)
		{
			case APIC_TYPE_LOCAL_APIC:
			{
				LocalApic *localApic = (LocalApic *)apicHeader;
				//__printf("Found CPU: processor ID => %d, apic ID => %d \n",(int)localApic->apicProcessorID, (int)localApic->apicID);
				//cpus[cpuCount++] = localApic->apicID;
				break;
			}
			case APIC_TYPE_IO_APIC:
			{
				IOApic *ioApic = (IOApic *)apicHeader;
				//itoa(ioApic->ioApicAddress, temp + 2, 0x10, 8, '0', 1);
				//__printf("Found I/O Apic : I/O Apic ID => %d, I/O Apic Address => %s\n",(int)ioApic->ioApicID, temp);
				u8 *ioApicAddress = (u8 *)ioApic->ioApicAddress;
				break;
			}
			case APIC_TYPE_INTERRUPT_OVERRIDE: {
				break;
			}
			default: {
				__printf(szout,"Unknow Apic information type:%d,length:%d.\n", (int)type, (int)length);
				break;
			}
		}
		start += length;
	}

	return TRUE;
}

int acpiEnable(const void *fadt, const void *ssdt)
{
	/*��Ϊfadt��ssdt�����ηֱ𴫽�����,�������Ҫ�ദ��һ��*/
	if (!acpiSsdt && ssdt)
		acpiSsdt = (const ACPIHeader *)ssdt; /*�����������ssdt,����acpiSsdt���ǿյľ�������*/
	if (acpiFadt) /*���fadt�Ѿ����ù��˾Ͳ���enable��,ֱ�ӷ���*/
		return 0;
	if (!fadt) /*���acpiFadtû������,fadtҲ��0,��ô����ʧ����,����*/
		return -1;
	acpiFadt = (const ACPIFadt *)fadt; /*����fadt��dsdt*/
	//acpiDsdt = (const ACPIHeader *)pa2va(acpiFadt->dsdt);
	acpiDsdt = (const ACPIHeader *)(acpiFadt->dsdt);
	if (acpiFadt->smiCommandPort)
		if (acpiFadt->acpiEnable || acpiFadt->acpiDisable) /*�ж��費��Ҫ�����Լ�enable,�п���bios�Ѿ�enable����,�����Ļ�����ͻ�����*/
			outportb(acpiFadt->smiCommandPort, acpiFadt->acpiEnable);
	while (!(inportw(acpiFadt->controlRegister1a) & SCI_ENABLED)) /*�ȴ�enable���*/
	{
		__asm {
			pause
		}
	}
	if (acpiFadt->controlRegister1b) /*����������,Ҳ�ȴ�������*/
	{
		while (!(inportw(acpiFadt->controlRegister1b) & SCI_ENABLED)) {
			__asm {
				pause
			}
		}
	}
	return TRUE;
}

static int parseDT(ACPIHeader *dt)
{
	u32 signature = dt->signature;
	char signatureString[5];
	__memcpy((char *)signatureString, (char *)&signature, 4);
	signatureString[4] = '\0';

	if (signature == *(u32 *)"APIC") {
		parseApic((ACPIHeaderApic *)dt);
	}
	else if (signature == *(u32 *)"HPET") {

	}
	else if (signature == *(u32 *)"FACP") {
		acpiEnable(dt, 0); /*FADT!!!*/
	}
	else if (signature == *(u32 *)"SSDT") {
		acpiEnable(0, dt);  /*SSDT!!!*/
	}
	return TRUE;
}



static int parseRSDT(ACPIHeader *rsdt)
{
	//u32 *start = rsdt->data;
	u32 *start = (u32 *) &rsdt[1];
	u32 *end = (u32 *)((u8 *)rsdt + rsdt->length);
	while (start < end)		//��length-36��/4����RSDT��ָ���table����
	{
		u32 dt = *(start++);
		parseDT((ACPIHeader *)dt);
	}
	return TRUE;
}


static int parseXSDT(ACPIHeader* rsdt)
{
	//u32 *start = rsdt->data;
	u32* start = (u32*)&rsdt[1];
	u32* end = (u32*)((u8*)rsdt + rsdt->length);
	while (start < end)		//��length-36��/8����RSDT��ָ���table����
	{
		u32 dt = *start;
		start += 2;
		parseDT((ACPIHeader*)dt);
	}
	return TRUE;
}



static int parseRSDP(XSDP_HEADER* xsdp)
{
	int res = 0;

	char szout[1024];
	u8 sum = 0;
	char * ptr = (char*)xsdp;
	for (int i = 0; i < 20; ++i)
	{
		sum += ptr[i];
	}
	if (sum)
	{
		__printf(szout,( char*)"parseRSDP checksum error\r\n");
		return FALSE;
	}

	char oem[7];
	__memset(oem, 0, sizeof(oem));
	__memcpy(oem, (char *)xsdp->OEMID , sizeof(oem) / sizeof(char) - 1);

	u8 version = xsdp->Revision;

	__printf(szout,"ACPI rsdp:%x,OEM:%s,version:%u\r\n", xsdp, oem,version);
	
	if (version == 0)
	{
		u32 rsdt = xsdp->RsdtAddress;
		res = parseRSDT((ACPIHeader *)rsdt);
	}
	else if (version == 2)
	{
		u64 xsdt = xsdp->XsdtAddress;
		u32 rsdt = xsdp->RsdtAddress;
		if (xsdt) {
			//parseXSDT((ACPIHeader *)xsdt);
			res = parseRSDT((ACPIHeader*)rsdt);
		}
		else {
			res = parseRSDT((ACPIHeader *)rsdt);
		}
	}
	else {
		return 0;
	}

	return res;
}


int initACPI(void)
{
	u8 *start = (u8 *)0xe0000;
	u8 * const end = (u8 *)0xfffff;
	while (start < end) {
		u64 signature = *(u64 *)start;
		if (signature == *(const u64 *)"RSD PTR ")
		{
			if (parseRSDP((XSDP_HEADER * )start) == 0)
			{
				return TRUE;
			}
		}
		start += 0x10;
	}
	u8 *ebda = (u8 *)( * (u16 *)0x40E << 4);
	if (ebda != (u8 *)0x9FC00) {
		return -1;
	}

	u8 * const ebdaEnd = ebda + 0x3FF;
	while (ebda < ebdaEnd) {
		u64 signature = *(u64 *)ebda;
		if (signature == *(const u64 *)"RSD PTR ")
		{
			if (parseRSDP((XSDP_HEADER*)ebda) == 0)
			{
				return TRUE;
			}
		}

		ebda += 0x10;
	}

	return FALSE;
}



extern "C"  __declspec(dllexport) int doPowerOff(void)
{
	__asm {
		//cli
	}
	const ACPIHeader *dt = acpiDsdt; /*����dsdt*/
retry:;
	u8 *data = (u8 *)dt + sizeof(ACPIHeader); /*��ȡdata�ͳ���*/
	u32 length = dt->length - sizeof(*dt);
	for (; length-- > 0; ++data)
	{
		if (data[0] != '_' || data[1] != 'S' || data[2] != '5' || data[3] != '_')
			continue;   /*���_S5_*/
		if (data[-1] != 0x8)
			if (data[-1] != '\\' || data[-2] != 0x8)
				continue;   /*����Ƿ���ϸ�ʽ*/
		if (data[4] != 0x12)
			continue;
		length = 1; /*�ҵ���!*/
		break;
	}
	if (length != 1) /*���dsdt��û��*/
	{
		if (!acpiSsdt)  /*���û��ssdt,ʧ��*/
			for (;;);
		if (dt == acpiSsdt) /*����Ѿ�������ssdt��dsdt,ʧ��*/
			for (;;);
		dt = acpiSsdt; /*��ssdt*/
		goto retry;
	}

	data += 5; /*����һЩ����*/
	data += ((data[0] & 0xc0) >> 6) + 2;
	if (data[0] == 0xa)
		++data;
	u16 s5a = (*data++) << 10; /*��ȡ��һ������*/
	if (data[0] == 0xa)
		++data;
	u16 s5b = (*data++) << 10; /*��ȡ�ڶ�������*/

	for (;;)
	{
		outportw(acpiFadt->controlRegister1a, s5a | (1 << 13)); /*��������*/
		if (acpiFadt->controlRegister1b)
			outportw(acpiFadt->controlRegister1b, s5b | (1 << 13));
	}
	return -1;
}


extern "C"  __declspec(dllexport) int doReboot(void)
{
	__asm {
		//cli
	}
	if (acpiFadt->header.length < 129)
		goto next;  /*����Ƿ�֧��reset�Ĵ���*/
	for (;;) /*����Reset����*/
		outportb(acpiFadt->resetRegister.address, acpiFadt->resetValue);
next: /*��֧�־Ͳ��ü��̿�����*/
	for (;;)
	{
		while (inportb(0x64) & 0x3)
			inportb(0x60);  /*��buf����*/
		outportb(0x64, 0xfe); /*���� Reset CPU ָ��*/
	}
	return -1;
}

