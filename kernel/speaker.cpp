#include "speaker.h"
#include "Utils.h"
#include "video.h"
#include "def.h"

#define PULSE_VALUE 2801*50

//�����ߵ�
WORD freq[] = { 
	262, 294, 330, 262, 262, 294, 330, 262 ,
	330, 349, 392,330, 349, 392,
	392, 440, 392, 349, 330, 262,392, 440, 392, 349, 330, 262,
	294, 196, 262,294, 196, 262 };

//; ������ı�    һ������Ϊһ�ģ�25����56���»���Ϊÿһ�����ְ�����ģ�12��  5 ����Ϊ�������ģ�50��
//; �������  ���ľ����˸���Ƶ������ʱ��
WORD beat[] = { 
	25, 25, 25, 25, 25, 25, 25, 25, 25,25, 50,
	25,25, 50, 12,12,12,12, 25, 25, 12,12,12,12, 25, 25,
	25, 25, 50, 25, 25, 50 
};



void __kSpeakerProc() {
	//__drawGraphChars((unsigned char*)"speaker running\r\n", 0);


	__asm {
		mov al, 0b6h
		out 43h, al

		mov ax, 0
		out 42h, al
		mov al, ah
		out 42h, al

		in al, 61h

		and al, 0fch

		out 61h, al
	}

	for (int i = 0;i < 32; i ++)
	{
		speakerPlayer(freq[i], beat[i]);
	}

	__asm{
		in al,61h

		and al, 0fch

		out 61h, al
	}
}

void speakerPlayer(WORD f,WORD b){

	int sound = PULSE_VALUE / f;

	int len = b * 120;

	unsigned char old61h = 0;

	__asm {
		in al, 61h

		mov old61h, al

		and al, 0fch

		out 61h, al
	}

	unsigned char control = 0;

	for (int j = 0; j < len; j ++)
	{
		__asm {
			mov al, control
			xor al, 2
			mov control,al

			out 61h, al
		}

		for (int i = 0; i < sound; i++)
		{
			;
		}
	}

	return;

	__asm{

		; ����һ���ַ�

		; ��������Ƶ��ת��Ϊ���������ȵļ���ֵ   75ҳ ���� = 2801 * 50 / freq

		mov eax, PULSE_VALUE

		mov edx, 0

		movzx ebx, f; ȡ����freq�еĶ�Ӧ��������Ƶ��

		div ebx; ��������

		mov edx, eax; �������

		push edx; 

		; �������  ���ľ����˸���Ƶ������ʱ��

		movzx ebx, b

		mov eax, 120;  8̫��

		mul ebx; 

		mov ebx, eax

		; λ����ģʽ����

		in al, 61h

		mov ah, al

		and al, 0fch

		pop edx

		_silent :

		xor al, 2

		out 61h, al

		mov ecx, edx

		_wait :

		loop _wait

		dec ebx

		jnz _silent

		mov al, ah

		out 61h, al
	}
}