#include "debugger.h"
#include "Utils.h"
#include "video.h"
#include "task.h"
#include "device.h"
#include "core.h"
#include "VM86.h"
#include "servicesProc.h"

//VME bit0
//����8086ģʽ��չ���е�λ0����1ʱ��������8086ģʽ�£������жϺ��쳣������չ����0ʱ������չ���ܡ�
//����ģʽ��չ��Ӧ����ͨ����������8086��س����8086����ִ�й����г��ֵ��жϺ��쳣�Ĵ��������ض����жϺ��쳣��8086����Ĵ������
//�Ӷ��Ľ�����8086ģʽ��Ӧ�ó�������ܡ����������жϱ�־��VIF vip����Ҳ�ṩ��Ӳ��֧�����Ľ��ڶ����񼰶ദ����������ִ��8086����Ŀɿ���

// Protected - Mode Virtual Interrupts(PVI) Bit.Bit1.
//Setting PVI to 1 enables support for protected -mode virtual interrupts.Clearing PVI to 0 disablesthis support.
//When PVI = 1, hardware support of two bits in the rFLAGS register, VIF and VIP, isenabled.
// Only the STI and CLI instructions are affected by enabling PVI.Unlike the case when CR0.VME = 1,
//the interrupt - redirection bitmap in the TSS cannot be used for selective INTn interception.
// PVI enhancements are also supported in long mode.See��Virtual Interrupts�� on page 251 for more
// information on using PVI.

//tsd stamp disable bit
//de debugging extensions bit 

// PSE bit4
// ҳ�ߴ���չ(�е�λ4)��1ʱҳ��СΪ4M�ֽڣ���0ʱҳ��СΪ4K�ֽ�

// PAE bit5
// �����ַ��չ(�е�λ5)��1ʱ���÷�ҳ����������36λ�����ַ����0ʱֻ������32λ��ַ��

// MCE bit6
// ���û������(�е�λ6)��1ʱ���û������(machine - check)�쳣����0ʱ���û�������쳣

// PGE bit7
// ����ȫ��ҳ(�е�λ7)(��P6ϵ�д�����������)��1ʱ����ȫ��ҳ����0ʱ����ȫ��ҳ��
// ȫ��ҳ��һ�����ܹ�ʹ��Щ������ʹ�û����ҳ�����е��û���־Ϊȫ�ֵ�(ͨ��ҳĿ¼����ҳ�����еĵ�8λ - ȫ�ֱ�־��ʵ��)��
// �������л�������CR3�Ĵ���дʱ��ȫ��ҳ������TLB��ˢ�¡�������ȫ��ҳ��һ����ʱ��������PGE��־֮ǰ��
// ���������÷�ҳ����(ͨ������CR0�е�PG��־)����������˳��ߵ��ˣ����ܻ�Ӱ��������ȷ���Լ������������ܻ�����

// PCE bit8
// �������ܼ�������(�е�λ8)��1ʱ������RDPMCָ��ִ�У����۳��������ĸ���Ȩ������0ʱRDPMCָ��ֻ��������0����Ȩ�ϡ�

//bit 9
// OSFXSR ����ϵͳ��FXSAVE��FXRSTORָ���֧��(�е�λ9)��1ʱ����һ��־��������
// ���ܣ�(1)��������ϵͳ֧��FXSAVE��FXRSTORָ��
// (2)����FXSAVE��FXRSTORָ��������ͻָ�XMM��MXCSR�Ĵ�����ͬx87 FPU��MMX�Ĵ���������
// (3)��������ִ�г���PAUSE��PREFETCHh��SFENCE��LFENCE��MFENCE��MOVNTI��CLFLUSHָ��֮����κ�SSE��SSE2ָ�
// �����һ��־��0����FXSAVE��FXRSTORָ���ͻָ�x87 FPU��MMX�Ĵ��������ݣ������ܲ�����ͻָ�XMM��MXCSR�Ĵ��������ݡ�
// ���⣬�����һ��־��0������������ͼִ�г���PAUSE��PREFETCHh��SFENCE��LFENCE��MFENCE�� MOVNTI��CLFLUSHָ��֮����κ�SSE��SSE2ָ��ʱ��
// ���������һ���Ƿ��������쳣(#UD)������ϵͳ������ȷ��������һ��־��
// ע�⣺
// CPUID������־FXSR��SSE��SSE2(λ24��25��26)�ֱ��ʾ���ض���IA - 32�������ϣ��Ƿ����FXSAVE / FXRESTORָ�SSE��չ�Լ�SSE2��չ��
// OSFXSRλ��Ϊ����ϵͳ������Щ�����ṩ��;���Լ���ָ���˲���ϵͳ�Ƿ�֧����Щ������

// OSXMMEXCPT bit10
// ����ϵͳ֧��δ���ε�SIMD�����쳣(�е�λ10)����������ϵͳͨ���쳣�������֧�ַ����ε�SIMD�����쳣�Ĵ���
// ���쳣���������SIMD�����쳣����ʱ�����á�
// ����ϵͳ������ȷ��������һ��־�������һ��־û�����ã�����������⵽������SIMD�����쳣ʱ���������һ���Ƿ��������쳣(#UD)

//bit10 OSXMMEXCPT ����ϵͳ֧��δ���ε�SIMD�����쳣

// bit 18 XSAVE and Extended States(OSXSAVE) Bit.Bit18.If this bit is set to 1 then the operating system
// supports the XGETBV, XSETBV, XSAVE and XRSTOR instructions.The processor will also be able
// to execute XGETBV and XSETBV instructions in order toread and write XCR0.Also, if set, the
// XSAVE and XRSTOR instructions can save and restore thex87 FPU state(including MMX registers),
// the SSE state(YMM / XMM registers and MXCSR), alongwith other processor extended states
// enabled in XCR0.


// �����������볤ģʽ
// ����ʵģʽ�µ�ջ��ע������������bios��ȡ��0x7c00λ��
// ����int13, ah = 02h��ȡ�ں˵�ָ��λ�ã�ע���豸chs������
// �ر��жϣ�����a20������32λgdt
// cr0.pe[0] = 1�����뱣��ģʽ
// ����ת��32λ�������룬16λʵģʽ����
// ����32λ�Σ����ñ���ģʽ�µ�ջ
// ׼��4kbx(2 - 4)��С�Ŀռ䣬���ҳ���ֱ��Ӧ1gb��2mb��4kb��ҳ���С�����pml4e��pdpe��pde��pte��ע��2mb��pde��ҳ����1gb��pdpe����ҳ���ˡ�
// ���е�ҳ�����Ŀ¼���64λ����512�����4kb�������ݾ���64λ�����ַ������nλ��Ų�����á�
// ����cr0.pg[31]��cr0.em[2]������cr0.mp[1]�����÷�ҳ��������sse
// ����.osfxsr[9]��.osxmmexcpt[10]��.pae[5]��.pge[7]������sse��pae��pge
// ��pml4װ��cr3
// ����efer.lme[8]������ģʽ
// ����cr0.pg[31]������ҳ������64λ����ģʽ
// ����64λgdt������64λ��ģʽ
/*����ת��64λ�������룬����ģʽ����*/

//TF(bit 8) [Trap flag]   ����λ����Ϊ1������������ģʽ����������ø�ģʽ




void initDebugger() {
	enableDE();

	__asm {
		//mov eax,cr4
		__emit 0x0f
		__emit 0x20
		__emit 0xe0

		//or eax, 8		//DE

		//mov cr4,eax
		__emit 0x0f
		__emit 0x22
		__emit 0xe0

		mov eax,0
		mov dr7,eax
		mov dr6,eax
	}
}



void __declspec(naked) BreakPointTrap(LIGHT_ENVIRONMENT* stack) {
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
		mov ss,ax
	}

	{
		__kBreakPoint(stack);
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

		iretd
	}
}

void __declspec(naked) DebugTrap(LIGHT_ENVIRONMENT* stack) {
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
		mov ss,ax
	}

	{
		__kDebugger(stack);
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

		iretd
	}
}

//Software-generated exceptions are not influenced by the IR bit map. INT1,[1] INT3, INTO, 
//and BOUND[2] are not subject to the IR bit map. Instead, these opcodes will always invoke the protected mode exception handler.[3]

void __kBreakPoint(LIGHT_ENVIRONMENT* stack) {

	char szout[1024];

#ifdef VM86_PROCESS_TASK

#else
	if (stack->eflags & 0x20000)
	{
		__kVm86IntProc();
		return;
	}
#endif

	int len = 0;

	unsigned char code = *(unsigned char*)stack->eip;
	if (code == 0xcc) {
		stack->eip++;
		__printf(szout,"int3(0xcc) is exception\r\n");
	}
	else {
		//__printf(szout, "int3(0xcc) is trap\r\n");
	}

	DWORD eflags = 0;
	__asm {
		//cpu clear IF when enter interruptions
		pushfd
		pop ss:[eflags]
	}

	if (stack->eflags & 0x20000)
	{
		len = __printf(szout,
			"BreakPoint eax:%x,ecx:%x,edx:%x,ebx:%x,esp0:%x,ebp:%x,esi:%x,edi:%x,eip:%x,vm86 cs:%x,current eflags:%x,stack eflags:%x,vm86 esp:%x,vm86 ss:%x,vm86 ds:%x,vm86 es:%x,vm86 fs:%x,vm86 gs:%x\n",
			stack->eax, stack->ecx, stack->edx, stack->ebx, stack->esp, stack->ebp, stack->esi, stack->edi, 
			stack->eip, stack->cs, eflags, stack->eflags, stack->esp3, stack->ss3,stack->ds_v86, stack->es_v86, stack->fs_v86, stack->gs_v86);
	}
	else if (stack->cs & 3)
	{
		len = __printf(szout,
			"BreakPoint eax:%x,ecx:%x,edx:%x,ebx:%x,esp:%x,ebp:%x,esi:%x,edi:%x,eip:%x,cs:%x,current eflags:%x,stack eflags:%x,esp3:%x,ss3:%x,ds:%x,es:%x,fs:%x,gs:%x,ss:%x\n",
			stack->eax, stack->ecx, stack->edx, stack->ebx, stack->esp, stack->ebp, stack->esi, stack->edi, 
			stack->eip, stack->cs, eflags, stack->eflags, stack->esp3, stack->ss3,
			stack->ds, stack->es, stack->fs, stack->gs,stack->ss);
	}
	else {
		len = __printf(szout,
			"BreakPoint eax:%x,ecx:%x,edx:%x,ebx:%x,esp:%x,ebp:%x,esi:%x,edi:%x,eip:%x,cs:%x,current eflags:%x,stack eflags:%x,ds:%x,es:%x,fs:%x,gs:%x,ss:%x\n",
			stack->eax, stack->ecx, stack->edx, stack->ebx, stack->esp, stack->ebp, stack->esi, stack->edi,stack->eip, stack->cs, eflags, stack->eflags,
			stack->ds, stack->es, stack->fs, stack->gs, stack->ss);
	}

	return;
}

//DR0 - DR3
//Contain linear addresses of up to 4 breakpoints.If paging is enabled, they are translated to physical addresses.

// DR6 It permits the debugger to determine which debug conditions have occurred.
//Bits 0 through 3 indicates, when set, that it's associated breakpoint condition was met when a debug exception was generated.
//Bit 13 indicates that the next instruction in the instruction stream accesses one of the debug registers.
//Bit 14 indicates(when set) that the debug exception was triggered by the single - step execution mode(enabled with TF bit in EFLAGS).
//Bit 15 indicates(when set) that the debug instruction resulted from a task switch where T flag in the TSS of target task was set.
//Bit 16 indicates(when clear) that the debug exception or breakpoint exception occured inside an RTM region.


void __kDebugger(LIGHT_ENVIRONMENT* stack) {

	char szout[1024];
	int len = 0;

	DWORD reg_dr6=0;
	__asm {
		mov eax,dr6
		mov [reg_dr6],eax
	}

	if (reg_dr6 & 0x2000)	//BD
	{
		__asm {
			mov eax,dr7
			and eax,0xffffdfff
			mov dr7,eax
		}
		len = __printf(szout, "BD BreakPoint eip:%x,cs:%x\r\n",stack->eip,stack->cs);

	}
	
	if (reg_dr6 & 0x4000)	//BS
	{
		len = __printf(szout, "Single Step BreakPoint eip:%x,cs:%x\r\n", stack->eip, stack->cs);

		//__kBreakPoint(stack);

		DWORD eflags = stack->eflags;
		if (eflags & 0x100)
		{
			eflags = eflags & 0xfffffeff;
			stack->eflags = eflags;
		}
	}
	
	if (reg_dr6 & 0x8000)	//BT
	{
		len = __printf(szout, "TSS Trap BreakPoint eip:%x,cs:%x\r\n", stack->eip, stack->cs);

	}

	DWORD addr = 0;
	DWORD bptype = 0;
	DWORD bplen = 0;
	if (reg_dr6 & 1)
	{
		__asm {
			mov eax, dr0
			mov[addr], eax

			mov eax, dr7
			and eax,0x30000
			mov [bptype],eax

			mov eax,dr7
			and eax,0xc0000
			mov [bplen],eax
		}
	}

	if (reg_dr6 & 2)
	{
		__asm {
			mov eax, dr1
			mov[addr], eax

			mov eax, dr7
			and eax, 0x300000
			mov[bptype], eax

			mov eax, dr7
			and eax, 0xc00000
			mov[bplen], eax
		}
	}

	if (reg_dr6 & 4)
	{
		__asm {
			mov eax, dr2
			mov[addr], eax

			mov eax, dr7
			and eax, 0x3000000
			mov[bptype], eax

			mov eax, dr7
			and eax, 0xc000000
			mov[bplen], eax
		}
	}

	if (reg_dr6 & 8)
	{
		__asm {
			mov eax, dr3
			mov[addr], eax

			mov eax, dr7
			and eax, 0x30000000
			mov[bptype], eax

			mov eax, dr7
			and eax, 0xc0000000
			mov[bplen], eax
		}
	}

	if ( (reg_dr6 & 0x2000) || (reg_dr6 & 0x4000) || (reg_dr6 & 0x8000) )
	{

	}
	
	if((reg_dr6 & 1) || (reg_dr6 & 2) || (reg_dr6 & 4) || (reg_dr6 & 8)){

		len = __printf(szout, "breakpoint address:%x,type:%d,bplen:%d\r\n",addr, bptype,bplen);

	}

	__asm {
		mov eax, 0
		mov[reg_dr6], eax
	}

	//RF ��־����Ҫ��������ɴӵ����쳣�����Զϵ������ģ�������Ǹ�ָ�ʼ����ִ�С�
	//���������������IRETD ָ��ص����жϳ���֮ǰ����ջ�е�EFLAGES ӳ���еĸ�λ��Ϊ1������ָֹ��ϵ��������ĵ����쳣��

	//�����жϻ���ִ��ָ��ǰ�������ڷ��ز��ɹ�ִ�жϵ�ָ��֮�󣬴��������Զ������λ���Ӷ���ɼ�������ָ��ϵ���ϡ�
	//���жϷ��ص�ʱ���������RF�Ļ������ٴν�������жϣ�RF����Ϊ�˷�ֹ�ظ���������ж϶�ʹ�õġ�

	DWORD eflags = stack->eflags;
	if ( (eflags & 0x10000) == 0)
	{
		//eflags = eflags & 0xfffeffff;
		eflags = eflags | 0x10000;
		stack->eflags = eflags;
	}
}

/*
dr7:

bit 0:L0
bit 1:G0

bit 2:L1
bit 3:G1

bit 4:L2
bit 5:G2

bit 6:L3
bit 7:G3

bit 13:GD

bit 17-16: rw/0
bit 19-18:len/0

bit 21-20: rw/1
bit 23-22:len/1

bit 25-24: rw/2
bit 27-26:len/2

bit 29-28: rw/3
bit 31-30:len/3
*/

//rw:
//00: exe
//01: data write
//10: I/O
//11: read write data

//len:
//00: 1 byte
//01: 2 byte
//10: 8 byte
//11: 4 byte

int dataWBreakPoint(unsigned int * addr,int len) {
	
	DWORD reg_dr7 = 0;
	__asm {
		mov eax,dr7
		mov reg_dr7,eax
	}

	DWORD mask = 0;
	DWORD value = 0;
	if ( (reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x10001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x100004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x1000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x10000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx,mask
		not ecx
		and eax,ecx

		or eax,value
		mov dr7, eax
	}
}

int dataRWBreakPoint(unsigned int * addr, int len) {
	DWORD reg_dr7 = 0;
	__asm {
		mov eax, dr7
		mov reg_dr7, eax
	}

	DWORD mask = 0;
	DWORD value = 0;
	if ((reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x30001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}
	else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x300004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x3000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x30000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx, mask
		not ecx
		and eax, ecx
		or eax, value
		mov dr7, eax
	}
}

int ioBreakPoint(unsigned int * addr, int len) {
	DWORD reg_dr7 = 0;
	__asm {
		mov eax, dr7
		mov reg_dr7, eax
	}

	int mask = 0;
	DWORD value = 0;
	if ((reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x20001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}
	else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x200004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x2000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x20000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx, mask
		not ecx
		and eax, ecx
		or eax, value
		mov dr7, eax
	}
}

int codeBreakPoint(unsigned int * addr,int len) {
	DWORD reg_dr7 = 0;
	__asm {
		mov eax, dr7
		mov reg_dr7, eax
	}

	DWORD mask = 0;
	DWORD value = 0;
	if ((reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x00001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}
	else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x000004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x0000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x00000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx, mask
		not ecx
		and eax, ecx
		or eax, value
		mov dr7, eax
	}
}



void __enableBreakPoint() {

	__asm {
		//_emit 0xcd
		//_emit 03

		int 3	//int 3 is different with int3 in linux gcc
	}
}

int enableSingleStep() {
	//single step
	__asm {
		pushfd
		bts dword ptr ss : [esp], 8
		popfd
	}
}


int disableSingleStep() {
	//single step
	__asm {
		pushfd
		btr dword ptr ss : [esp], 8
		popfd
	}
}


int enableGdDebugger() {

	__asm {
		mov eax, dr7
		bts eax, 13
		mov dr7, eax
	}
}

int disableGdDebugger() {

	__asm {
		mov eax, dr7
		btr eax, 13
		mov dr7, eax
	}
}

//only be effective in ring0
void enableOverflow() {
	__asm {
		pushfd
		or ss:[esp],0x800
		popfd

		_emit 0xce
	}
}


//int 1
void enableDebugger() {
	__asm {
		_emit 0xF1
	}
}


