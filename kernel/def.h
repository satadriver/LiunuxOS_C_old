

#pragma once

#define NULL		0
#define TRUE		1
#define FALSE		0

#define UCHAR unsigned char
#define CHAR char
#define LPSTR char *
#define WCHAR wchar_t
#define USHORT unsigned short
#define DWORD unsigned long
#define WORD	unsigned short
#define HINSTANCE unsigned long
#define VOID void
#define LPVOID void *
#define HMODULE HINSTANCE
#define UINT16 unsigned short
#define LONG long
#define BYTE unsigned char
#define UINT64 unsigned __int64
#define UINT32 unsigned int
#define UINT8 unsigned char
#define BOOL int
#define uint16_t unsigned short
#define uint32_t unsigned int
#define uint8_t unsigned char
#define uint64_t unsigned __int64
#define ULONG32 unsigned int
#define PULONG32 unsigned int *
#define DWORD32 unsigned int 
#define PDWORD32 unsigned int*
#define ULONG_PTR unsigned long* 
#define ULONGLONG unsigned __int64



#define LIUNUX_DEBUG_LOG_ON

#define MY_SLAB_FLAG			"LJG"
#define MAIN_DLL_MODULE_NAME	"main.dll"
#define KERNEL_DLL_MODULE_NAME	"kernel.dll"

//#define SINGLE_TASK_TSS

#define VM86_PROCESS_TASK

//#define DISABLE_PAGE_MAPPING

#define FILE_ATTRIBUTE_DIRECTORY	0x10
#define FILE_ATTRIBUTE_ARCHIVE		0x20

#define MAX_PATH_SIZE				1024

#define SYSTEM_TIMER0_FACTOR		11931
#define OSCILLATE_FREQUENCY			1193182		//1193181.6

#define BYTES_PER_SECTOR			512
#define ATAPI_SECTOR_SIZE			2048

#define PAGE_SIZE					4096
#define ITEM_IN_PAGE				1024
#define PAGE_MASK					0XFFFFF000
#define PAGE_INSIZE_MASK			0XFFF

#define INTR_8259_MASTER			0x40
#define INTR_8259_SLAVE				INTR_8259_MASTER + 8

#define STACK_TOP_DUMMY				0x20
#define TASK_STACK0_SIZE 			1024
#define V86_STACK_SIZE				0X10000
#define UTASK_STACK_SIZE			0x100000
#define KTASK_STACK_SIZE			0x10000

#define HEAP_SIZE					0x100000

#define WINDOW_NAME_LIMIT			64

#define KERNEL_TASK_LIMIT			64
#define TASK_LIMIT_TOTAL			256

#define KERNEL_MODE_CODE			8
#define KERNEL_MODE_STACK			16
#define USER_MODE_CODE				24
#define USER_MODE_STACK				32
#define KERNEL_MODE_DATA			KERNEL_MODE_STACK
#define USER_MODE_DATA				USER_MODE_STACK
#define reCode32TempSeg				0x28
#define kTssTaskSelector			0x30
#define kTssExceptSelector			0x38
#define kTssTimerSelector			0x40
#define kTssV86Selector				0x48
#define ldtSelector					0x50
#define callGateSelector			0X58
#define reCode16Seg					0x60
#define rwData16Seg 				0x68
#define eoCode32Seg					0x70
#define roData32Seg					0x78
#define AP_TSS_DESCRIPTOR			0X80

#define RM_EMS_BASE		 			0X100000
#define PTE_ENTRY_VALUE				0X110000
//��ַ����4k���� ҳĿ¼�����λ��һ����Ȼҳ��(4KB����), ���������ַ�ĵ�12λ��ȫ0
#define PDE_ENTRY_VALUE 			0X510000

#define CMOS_DATETIME_STRING 		0X512000
#define TIMER0_FREQUENCY_ADDR		CMOS_DATETIME_STRING + 0x100
#define CMOS_PERIOD_TICK_COUNT		TIMER0_FREQUENCY_ADDR + 4
#define CMOS_EXACT_TICK_COUNT 		CMOS_PERIOD_TICK_COUNT + 4
#define TIMER0_TICK_COUNT			CMOS_EXACT_TICK_COUNT + 4
#define SLEEP_TIMER_RECORD			TIMER0_TICK_COUNT + 4
#define ATA_INFO_BASE				SLEEP_TIMER_RECORD + 4
#define ATAPI_INFO_BASE				ATA_INFO_BASE + BYTES_PER_SECTOR
#define CMOS_ALARM_HOUR				ATAPI_INFO_BASE + BYTES_PER_SECTOR
#define CMOS_ALARM_MINUTE			CMOS_ALARM_HOUR + 4
#define CMOS_ALARM_SECOND			CMOS_ALARM_MINUTE + 4
#define CMOS_ALARM_MONTH			CMOS_ALARM_SECOND + 4

#define VESA_INFO_BASE			0X513000

#define CURSOR_GRAPH_BASE		0X514000

#define KEYBOARD_BUFFER			0X518000

#define MOUSE_BUFFER			0X520000

#define TASKS_LIST_BASE			0X530000
#define TASKS_LIST_BUF_SIZE		0x1000

#define CURRENT_TASK_TSS_BASE	0X540000

#define V86_TSS_BASE			0X544000

#define TIMER_TSS_BASE			0X548000

#define INVALID_TSS_BASE		0X54c000



#define GDT_BASE				0X560000

#define IDT_BASE				0X570000

#define FPU_STATUS_BUFFER		0X580000

#define LOG_BUFFER_BASE			0X5A0000

#define LDT_BASE				0x5c0000

#define LIB_INFO_SIZE			0X10000
#define LIB_INFO_BASE			0X5d0000

#define KERNEL_TASK_STACK_BASE 	 0x600000
#define KERNEL_TASK_STACK_TOP 	 (KERNEL_TASK_STACK_BASE + KTASK_STACK_SIZE - STACK_TOP_DUMMY)

#define TSSEXP_STACK_ADDRESS 	 (KERNEL_TASK_STACK_TOP + STACK_TOP_DUMMY)
#define TSSEXP_STACK_TOP 		(TSSEXP_STACK_ADDRESS + KTASK_STACK_SIZE - STACK_TOP_DUMMY)

#define TSSTIMER_STACK_ADDRESS  (TSSEXP_STACK_TOP + STACK_TOP_DUMMY)
#define TSSTIMER_STACK_TOP 		(TSSTIMER_STACK_ADDRESS + KTASK_STACK_SIZE - STACK_TOP_DUMMY)

#define TSSV86_STACK_ADDRESS 	(TSSTIMER_STACK_TOP + STACK_TOP_DUMMY)
#define TSSV86_STACK_TOP 		(TSSV86_STACK_ADDRESS + KTASK_STACK_SIZE - STACK_TOP_DUMMY)

#define SYSCALL_STACK_ADDRESS	(TSSV86_STACK_TOP + STACK_TOP_DUMMY)
#define SYSCALL_STACK_TOP		(SYSCALL_STACK_ADDRESS + KTASK_STACK_SIZE - STACK_TOP_DUMMY)

#define TSSEXP_STACK0_ADDRESS 	0x680000
#define TSSEXP_STACK0_TOP 		(TSSEXP_STACK0_ADDRESS + TASK_STACK0_SIZE - STACK_TOP_DUMMY)

#define TSSTIMER_STACK0_ADDRESS  (TSSEXP_STACK0_TOP + STACK_TOP_DUMMY)
#define TSSTIMER_STACK0_TOP 	(TSSTIMER_STACK0_ADDRESS + TASK_STACK0_SIZE - STACK_TOP_DUMMY)

#define TSSV86_STACK0_ADDRESS 	 (TSSTIMER_STACK0_TOP + STACK_TOP_DUMMY)
#define TSSV86_STACK0_TOP 		(TSSV86_STACK0_ADDRESS + TASK_STACK0_SIZE - STACK_TOP_DUMMY)

#define AP_STACK0_BASE			0X700000

#define AP_KSTACK_BASE			0X780000

#define AP_TSS_BASE				0X900000

#define ISA_DMA_BUFFER			0X800000

#define FLOPPY_DMA_BUFFER		0X840000
#define FLOPPY_DBR_BUFFER		0X850000
#define FLOPPY_FAT_BUFFER		0X860000
#define FLOPPY_ROOT_BUFFER		0X870000

#define KERNEL_DLL_BASE			0x1000000

#define KERNEL_DLL_SOURCE_BASE	0x1100000

#define MAIN_DLL_BASE			0x1200000

#define MAIN_DLL_SOURCE_BASE	0X1300000

#define TASKS_TSS_BASE			0X1400000

#define TASKS_STACK0_BASE		0x1800000

#define TASKS_AP_STACK_BASE		0x1900000

#define MEMORY_ALLOC_BUFLIST			0X1a00000
#define MEMORY_ALLOC_BUFLIST_SIZE		0X400000

#define PAGE_ALLOC_LIST					0X1e00000
#define PAGE_ALLOC_LIST_SIZE			0X200000

#define PAGE_TABLE_BASE					0x2000000
#define PAGE_TABLE_SIZE					0X1000000

#define FILE_BUFFER_ADDRESS				0x3000000

#define MEMMORY_ALLOC_BASE				0X4000000

#define USER_SPACE_START				0X40000000

#define USER_SPACE_END					0XC0000000

//low 1M memory map
#define LOADER_BASE_SEGMENT 	0x800

#define KERNEL_BASE_SEGMENT 	0x1000

#define GRAPHFONT_LOAD_SEG 		0x9000
#define GRAPHFONT_LOAD_OFFSET 	0
#define GRAPHFONT_LOAD_ADDRESS 	(GRAPHFONT_LOAD_SEG * 16 + GRAPHFONT_LOAD_OFFSET)

#define MEMORYINFO_LOAD_SEG 	 0x9000
#define MEMORYINFO_LOAD_OFFSET 	 0x1000
#define MEMORYINFO_LOAD_ADDRESS (MEMORYINFO_LOAD_SEG * 16 + MEMORYINFO_LOAD_OFFSET)

#define V86VMIPARAMS_SEG		0x9000
#define V86VMIPARAMS_OFFSET		0X1400
#define V86VMIPARAMS_ADDRESS	(V86VMIPARAMS_SEG * 16 + V86VMIPARAMS_OFFSET)

#define V86VMIDATA_SEG			0x9000
#define V86VMIDATA_OFFSET		0X1500
#define V86VMIDATA_ADDRESS		(V86VMIDATA_SEG * 16 + V86VMIDATA_OFFSET)

#define V86_INT_SEG				0x9000
#define V86_INT_OFFSET 			0x1600
#define V86_INT_ADDRESS 		(V86_INT_SEG * 16 + V86_INT_OFFSET)

//#define V86_TASKCONTROL_SEG		0x9000
//#define V86_TASKCONTROL_OFFSET	0X2200
//#define V86_TASKCONTROL_ADDRESS	(V86_TASKCONTROL_SEG * 16 + V86_TASKCONTROL_OFFSET)
//#define V86_TASKCONTROL_ADDRESS_END (LIMIT_V86_PROC_COUNT*12 + V86_TASKCONTROL_ADDRESS)

#define VESA_STATE_SEG			0x9000
#define VESA_STATE_OFFSET		0X1800
#define VESA_STATE_ADDRESS		(VESA_STATE_SEG * 16 + VESA_STATE_OFFSET)

#define AP_INIT_SEG				0x9000
#define AP_INIT_OFFSET			0X2000
#define AP_INIT_ADDRESS			(AP_INIT_SEG * 16 + AP_INIT_OFFSET)

#define INT13_RM_FILEBUF_SEG	0X8000
#define INT13_RM_FILEBUF_OFFSET 0
#define INT13_RM_FILEBUF_ADDR	(INT13_RM_FILEBUF_SEG * 16 + INT13_RM_FILEBUF_OFFSET)

#define LIMIT_V86_PROC_COUNT	6

#define DOS_LOAD_FIRST_SEG		0X2000

#define VSKDLL_LOAD_SEG 		0x2000
#define VSKDLL_LOAD_OFFSET 		0
#define VSKDLL_LOAD_ADDRESS 	(VSKDLL_LOAD_SEG * 16 + VSKDLL_LOAD_OFFSET)

#define VSMAINDLL_LOAD_SEG 		0x6000
#define VSMAINDLL_LOAD_OFFSET 	0x0
#define VSMAINDLL_LOAD_ADDRESS 	(VSMAINDLL_LOAD_SEG * 16 + VSMAINDLL_LOAD_OFFSET)

#define SHOW_WINDOW_BMP		1
#define SHOW_WINDOW_TXT		2
#define SHOW_WINDOW_JPEG	3
#define SHOW_TEST_WINDOW	4
#define SHOW_SYSTEM_LOG		5

#define UNKNOWN_FILE_SYSTEM 0
#define FAT32_FILE_SYSTEM	1
#define NTFS_FILE_SYSTEM	2
#define CDROM_FILE_SYSTEM	3
#define FLOPPY_FILE_SYSTEM	4


#pragma pack(push,1)

typedef struct {
	DWORD _flags;			//0
	WORD _loaderSecCnt;		//4
	DWORD _loaderSecOff;	//6
	WORD _kernelSecCnt;		//10
	DWORD _kernelSecOff;	//12
	DWORD _bakMbrSecOff;	//16
	DWORD _bakMbr2SecOff;	//20
	WORD _fontSecCnt;		//24
	DWORD _fontSecOff;		// 26
	WORD _kdllSecCnt;		//30
	DWORD _kdllSecOff;		//32
	WORD _maindllSecCnt;	//36
	DWORD _maindllSecOff;	//38
	char _reserved[22];		//42
}DATALOADERINFO;

typedef struct {
	DWORD link; // ����ǰһ�� TSS ��ѡ���ӣ�ʹ�� call ָ���л��Ĵ�����ʱ����CPU��д��

	DWORD esp0; //4
	DWORD ss0;  //8
	DWORD esp1; //12
	DWORD ss1;  //16
	DWORD esp2; //20
	DWORD ss2;  //24

	DWORD cr3;	//28

				// ������Щ�����������л��Ĵ���ֵ�õģ��л��Ĵ�����ʱ����CPU�Զ���д��
	DWORD eip;	//32
	DWORD eflags;
	DWORD eax;	//40
	DWORD ecx;
	DWORD edx;	//48
	DWORD ebx;
	DWORD esp;	//56
	DWORD ebp;
	DWORD esi;	//64
	DWORD edi;
	DWORD es;	//72
	DWORD cs;
	DWORD ss;	//80
	DWORD ds;
	DWORD fs;	//88
	DWORD gs;

	//static
	DWORD ldt;	//96
	unsigned short	trap;				//100
	unsigned short	iomapOffset;		//102
	unsigned char	intMap[32];
	unsigned char	iomap[8192];
	unsigned char	iomapEnd;			//104 + 32 + 8192

} TSS, * LPTSS;

#pragma pack(pop)


#define __sizeof(T) ( (size_t)(&T + 1) - (size_t)(&T) )

#define OFFSETOF(type,n) (size_t)&(((type*)0)->n)

#define SIZEOFMEMBER(type,n) sizeof(((type*)0)->n)


extern "C" int g_tagMsg;