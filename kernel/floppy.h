#pragma once

/*
���̿�����
ͨ�����¼������ƼĴ�������FDC
I / O address    Read or Write    Register
0x3f2          Write            DOR : Digital Output Register
0x3f4          Read             FDC Status : Floppy Disk Status Register
0x3f5          Read / Write       FDC Data : Floppy Disk Data Register
0x3f7          Read             DIR : Digital Input Register
Write            DCR : Disk Control Register



DOR��DOR��һ��8λ�Ĵ������������������������Ŀ�����������ѡ������ / ��λFDC�Լ����� / ��ֹDMA����
λ    Name    Description
7    MOT_EN3    Driver D motor��1 - start��0 - stop
6    MOT_EN2    Driver C motor��1 - start��0 - stop
5    MOT_EN1    Driver B motor��1 - start��0 - stop
4    MOT_EN0    Driver A motor��1 - start��0 - stop
3    DMA_INT    DMA interrupt�� 1 enable; 0 - disable
2    RESET    FDC Reset
1    DRV_SEL1  Select driver 1 / 0
0    DRV_SEL0



FDC status��FDC״̬�Ĵ���
FDC status���ڷ�ӳ����������FDC�Ļ���״̬��ͨ������CPU��FDC����������FDC��ȡ���ǰ����Ҫ��ȡFDC��״̬Ϊ�����жϵ�ǰ��FDC data�Ĵ����Ƿ���裬�Լ�ȷ�����ݴ��䷽��
λ    Name    Description
7    RQM    Data ready : FDD ready
6    DIO    Direction : 1 - FDD to CPU; 0 �C CPU to FDD
5    NDM    DMA set : 1 - not DMA; 0 - DMA
4    CB    Controller busy
3    DDB    Driver D busy
2    DCB    Driver C busy
1    DBB    Driver B busy
0    DAB    Driver A busy



FDC Data��FDC���ݼĴ���
FDC Data�Ĵ���������FDC���Ϳ���������FDC��ȡ״̬��ʵ�����ݶ�д�ȡ�FDC��ʹ�ñȽϸ��ӣ���֧�ֶ������ÿ�����ͨ��һ����������ʵ�֣�����׶Ρ�ִ�н׶κͽ���׶Ρ�
1��  ����У�����FD_RECALIBRATE��
��������ʱ����
�׶�    ��    D7  D6  D5  D4  D3  D2  D1  D0  ˵��
cmd      0    0   0   0   0   0   1   1   1   0x07
1    0   0   0   0   0   0   US1 US2 Drive no.
ִ��                                        ��ͷ�ƶ���track0
���    ��                                    ��

2��  ��ͷѰ�����FD_SEEK��
�Ѵ�ͷ��λ���ƶ�λ�ã��ڶ�дǰִ��
�׶�    ��  D7  D6  D5  D4  D3  D2  D1  D0  ˵��
cmd      0   0   0   0   0   0   1   1   1   0x0F
1   0   0   0   0   0   HD  US1 US2 ��ͷ�š���������
2   C                               �ŵ���
ִ��                                        ��ͷ�ƶ����ƶ��ŵ�
���    ��                                  ��

3��  �������������FD_READ��
�׶�    ��  D7  D6  D5  D4  D3  D2  D1  D0  ˵��
cmd     0   MT  MF  SK  0   0   1   1   0   0xE6��MT = MF = SK = 1��
1   0   0   0   0   0   0   US1 US2 ��������
2   C                               �ŵ���track
3   H                               ��ͷ��head
4   R                               ��ʼ������start sector
5   N                               �����ֽ���
6   EOT                             �ŵ����������
7   GPL                             ������������ȣ�3��
8   DTL                             N = 0ʱ���ƶ������ֽ���
ִ��                                        �����̶�ȡ����
���    1    ST0    ״̬�ֽ�0
2    ST1    ״̬�ֽ�1
3    ST2    ״̬�ֽ�2
4    C    �ŵ���track
5    H    ��ͷ��head
6    R    ��ʼ������
7    N    �����ֽ���
ע��
MT����ŵ�������MT = 1��ʾ�����ŵ�����
MF����¼��ʽ��MF = 1��ʾѡ��MFM��¼��ʽ��������FM��¼��ʽd
SK���Ƿ�������ɾ����־��������SK = 1��ʾ������

���صķ��ص�״̬ST0��ST1��ST2�ĺ������£�
ST0��
λ    ����        ˵��
7 - 6    ST0_INTR    �ж�ԭ��00 - ����������01 - �쳣������10 - ������Ч��11 - ����������״̬�ı�
5    ST0_SE        Ѱ������������У������������seek end��
4    ST0_ECE       �豸������0�ŵ�У�����󣩣�Equip.Check Error��
3    ST0_NR        ����δ������Not Ready��
2    ST0_HA        ��ͷ��ַ���ж�ʱ��ͷ��ַ��Head Address��
1 - 0  ST0_DS   �������ţ�Driver Select��

ST1��
λ    ����    ˵��
7    ST1_EOC    ���ĳ����ŵ���������ţ�End of Cylinder��
6    Reserve
5    ST1_CRC    CRCУ�����
4    ST1_OR    ���ݴ��䳬ʱ��Over Run��
3    Reserve
2    ST1_ND    δ�ҵ��ƶ�������No Data��
1    ST1_WP    д������Write Protect��
0    ST1_MAM    δ�ҵ�������ַ��־ID��Miss Address Mask��

ST2��
λ    ����    ˵��
7    Reserve
6    ST2_CM    SK = 0ʱ������������ɾ����־��Control Mark��
5    ST2_CRC    CRCУ�����
4    ST2_WC    ����ID��Ϣ�Ĵŵ���C��������Wrong Cylinder��
3    ST2_SEH    �����������㣨Scan Equal Hit��
2    ST2_SNS    �������������㣨Scan Not Satisfied��
1    ST2_BC    �ŵ�����Bad Cylinder��
0    ST2_MAM    δ�ҵ�������ַ��־ID��Miss Address Mask��



4��  д�����������FD_WRITE��
�׶�  ��  D7  D6  D5  D4  D3  D2  D1  D0  ˵��
cmd   0   MT  MF  0   0   0   1   0   1   0xC5��MT = MF = 1��
1   0   0   0   0   0   0   US1 US2 ��ͷ�š���������
2   C                               �ŵ���track
3   H                               ��ͷ��head
4   R                               ��ʼ������start sector
5   N                               �����ֽ���
6   EOT                             �ŵ����������
7   GPL                             ������������ȣ�3��
8   DTL                             N = 0ʱ���ƶ������ֽ���
ִ��                                      ������д������
���  1  ST0   ״̬�ֽ�0
2  ST1   ״̬�ֽ�1
3  ST2   ״̬�ֽ�2



5��  ����ж�״̬���FD_SENSEI��
�׶�  ��  D7  D6  D5  D4  D3  D2  D1  D0  ˵��
cmd   0   0   0   0   0   0   1   1   1   0x08
ִ��
���  1    ST0  ״̬�ֽ�0
2 ��ͷ���ڴŵ���



6��  �趨�������������FD_SPECIFY��
�׶�  ��  D7  D6  D5  D4  D3  D2  D1  D0  ˵��
cmd   0   0   0   0   0   0   1   1   1   0x03
1   4 - 7:SRT��λ2ms, 0 - 3 : HUT��λ32ms  ����ٶȡ���ͷж��ʱ��
2   1 - 7 : HLT��λ4ms, 0 : ND             ��ͷ����ʱ�䣬��DMAģʽ
ִ��                                      ���ÿ�����
���      ��                              ��

DIR����������Ĵ���
DIR�Ĵ���ֻ��D7λ��Ч�����ڱ�ʾ���̸���״̬����������Ӳ�̿�������

DCR�����̿��ƼĴ���
DCR�����û�D0��D1λ�����ڱ�ʾ���ݴ����ʡ�
00 - 500kpbs, 01 - 300kpbs, 10 - 250kpbs��
*/




#define FD_SEEK			0x0f
#define FD_RECALIBRATE	7
#define FD_READ			0xe6
#define FD_SPECIFY		7




#define FLOPPY_REG_DOR	0x3f2
#define FD_STATUS		0x3f4
#define FD_DATA			0x3f5
#define FD_DCR			0x3f7

#define STATUS_READY	8
#define STATUS_DIR		0x40

#define SPT  18
#define HPC  2


#pragma pack(1)

typedef struct {
	unsigned int size, sect, head, track, stretch;
	unsigned char gap, rate, spec1;
}floppy_struct;

#pragma pack()


void output_byte(unsigned char byte);

extern "C" __declspec(dllexport) void floppyInit();

void FloppyReadSector(unsigned int sectNo,int seccnt, unsigned char *buf);

void SetDMA(unsigned char *buf,int seccnt, unsigned char cmd);

extern "C" __declspec(dllexport) void __kFloppyIntrProc();


int getCylinder(int lba);

int getHeader(int lba);


int getSector(int lba);