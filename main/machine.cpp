#include "machine.h"
#include "def.h"
#include "video.h"
#include "Utils.h"

//MSR ��CPU ��һ��64 λ�Ĵ��������Էֱ�ͨ��RDMSR ��WRMSR ����ָ����ж���д�Ĳ�����ǰ��Ҫ��ECX ��д��MSR �ĵ�ַ��
//MSR ��ָ�����ִ����level 0 ��ʵģʽ�¡�
//RDMSR    ��ģʽ����Ĵ���������RDMSR ָ����᷵����Ӧ��MSR ��64bit ��Ϣ��(EDX��EAX)�Ĵ�����
//WRMSR    дģʽ����Ĵ���������WRMSR ָ���Ҫд�����Ϣ����(EDX��EAX)�У�ִ��дָ��󣬼��ɽ���Ӧ����Ϣ����ECX ָ����MSR ��

//ͨ��DTS��ȡ�¶Ȳ�����ֱ�ӵõ�CPU��ʵ���¶ȣ����������¶ȵĲ
//��һ������Tjmax�����Intel��TCC activation temperature��
//��˼�ǵ�CPU�¶ȴﵽ�򳬹����ֵʱ���ͻᴥ����ص��¶ȿ��Ƶ�·��ϵͳ��ʱ���ȡ��Ҫ�Ķ���������CPU���¶ȣ�����ֱ��������ػ���
//����CPU���¶���Զ���ᳬ�����ֵ�����ֵһ����100���85�棨Ҳ������ֵ�������ھ���Ĵ�������˵����һ���̶���ֵ��
//�ڶ�������DTS��ȡ��CPU�¶����Tjmax��ƫ��ֵ�����ҽ�Toffset����CPU��ʵ���¶Ⱦ��ǣ�currentTemp=Tjmax-Toffset
int __readTemperature(DWORD * tjunction) {
	unsigned int Tjunction = 0;
	DWORD temp = 0;
	__asm {
		mov eax, 0
		cpuid
		cmp eax, 6
		jb _tmpQuit

		mov eax, 6
		cpuid
		test eax, 2
		jz _tmpQuit

		mov ecx, 0x1A2		//eax��16~23λ����Tjmax��ֵ
		rdmsr
		test eax, 0x40000000
		jnz _tmp85
		//mov eax, 100
		mov Tjunction, eax
		jmp _getdts
		_tmp85 :
		//mov eax, 85
		mov Tjunction, eax

		_getdts :
		mov ebx, eax
		and ebx, 0x00ff0000
		shr ebx, 16

		//mov ebx,eax

		mov ecx, 0x19C		//eax��16~22��ע��������7λ��λ����Toffset��ֵ
		rdmsr
		and eax, 0x007f0000
		shr eax, 16
		sub ebx, eax
		mov eax, ebx
		mov temp,eax
		_tmpQuit :
	}

	*tjunction = Tjunction;

	char szout[1024];
	__printf(szout, "tjmax:%x,temprature:%x\r\n", Tjunction,temp);
	__drawGraphChars((unsigned char*)szout, 0);

	return temp;
}





