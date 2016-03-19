/************************************************************************
* Filename: disk.c
************************************************************************/
#define IN_DISK

#include "tistdtypes.h"
#include "fat.h"

Disk_Info DiskInfo;     //�߼�����Ϣ

/*********************************************************************************************************
** ��������: DiskInit
** ��������: ��ʼ���߼��̹���ģ��
**
** �䡡��: ��
**
** �䡡��: ��
**         
** ȫ�ֱ���: DiskInfo
** ����ģ��: ��
**
********************************************************************************************************/
void DiskInit(void)
{
    DiskInfo.FATType = 0xff;         // ��Ч���ļ�ϵͳ
    DiskInfo.SecPerClus = 0;         // ÿ��������
    DiskInfo.NumFATs = 0;            // FAT����
    DiskInfo.SecPerDisk = 0xffffffff;// �߼�����������������
    DiskInfo.BytsPerSec = 0;         // ÿ�����ֽ���
    DiskInfo.RootDirTable = 0;       // ��Ŀ¼��ʼ�����ţ�FAT32Ϊ��ʼ�غţ�
    DiskInfo.RootSecCnt = 0;         // ��Ŀ¼ռ��������
    DiskInfo.FATStartSec = 0;        // FAT��ʼ������
    DiskInfo.FATSecCnt = 0;          // ÿ��FATռ��������
    DiskInfo.DataStartSec = 0;       // ��������ʼ������
    DiskInfo.PathClusIndex = 0;      // ��ǰĿ¼
    DiskInfo.DiakCommand = NULL;     // ��������

    CacheInit();                //��ʼ������cache
}

/*********************************************************************************************************
** ��������: FsMount
** ��������: �ڲ����ؾ�
**
** �䡡��: Disk���߼�����Ϣָ��
**         Buf������0����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
**
********************************************************************************************************/
Uint8 FsMount(Uint8 *Buf)
{
    Uint32 temp1;
    
    if (Buf[510] != 0x55 || Buf[511] != 0xaa)      /* ��Ч��־ */
    {
        return DISK_NO_FORMAT;
    }
    if (Buf[21] != 0xF0)
    if (Buf[21] < 0xF8)
    {
        return DISK_NO_FORMAT;
    }
    
    /* ÿ�����ֽ��� */
    DiskInfo.BytsPerSec = Buf[11] | (Buf[12] << 8);
    /* ÿ�������� */
    DiskInfo.SecPerClus = Buf[13];
    /* FAT��ʼ������ */
    DiskInfo.FATStartSec = Buf[14] | (Buf[15] << 8);
    /* FAT����� */
    DiskInfo.NumFATs = Buf[16];
    /* ��Ŀ¼ռ������ */ 
    DiskInfo.RootSecCnt = ((Buf[17] | (Buf[18] << 8)) * 32 + 
                               DiskInfo.BytsPerSec - 1) /
                               DiskInfo.BytsPerSec;

    /* �߼��̣���ռ������ */
    temp1 = Buf[19] | (Buf[20] << 8);
    if (temp1 == 0)
    {
        temp1 = Buf[35];
        temp1 = (temp1 << 8) | Buf[34];
        temp1 = (temp1 << 8) | Buf[33];
        temp1 = (temp1 << 8) | Buf[32];
    }
    DiskInfo.SecPerDisk = temp1;

    /* FAT��ռ�������� */
    temp1 = Buf[22] | (Buf[23] << 8);
    if (temp1 == 0)
    {
        temp1 = Buf[39];
        temp1 = (temp1 << 8) | Buf[38];
        temp1 = (temp1 << 8) | Buf[37];
        temp1 = (temp1 << 8) | Buf[36];
    }
    DiskInfo.FATSecCnt = temp1;
                
    /* ��ǰĿ¼Ϊ��Ŀ¼ */
    DiskInfo.PathClusIndex = 0;
    /* ��Ŀ¼��ʼ������ */
    DiskInfo.RootDirTable = DiskInfo.NumFATs * DiskInfo.FATSecCnt + 
                               DiskInfo.FATStartSec;
    /* ��������ʼ������ */
    DiskInfo.DataStartSec = DiskInfo.RootDirTable + DiskInfo.RootSecCnt;
    temp1 = DiskInfo.SecPerDisk - DiskInfo.DataStartSec;
    temp1 = temp1 / DiskInfo.SecPerClus;
    DiskInfo.ClusPerData = temp1;
    /* �ж���FAT12��FAT16����FAT32 */
    if (temp1 < 4085)
    {
        DiskInfo.FATType = FAT12;
    }
    else if (temp1 < 65525)
    {
        DiskInfo.FATType = FAT16;
    }
    else
    {
        DiskInfo.FATType = FAT32;
        /* FAT32 RootDirTableΪ��ʼ�غ� */

        DiskInfo.RootDirTable = Buf[47];
        DiskInfo.RootDirTable = (DiskInfo.RootDirTable << 8) | Buf[46];
        DiskInfo.RootDirTable = (DiskInfo.RootDirTable << 8) | Buf[45];
        DiskInfo.RootDirTable = (DiskInfo.RootDirTable << 8) | Buf[44];
        DiskInfo.PathClusIndex = DiskInfo.RootDirTable;
    }

    return RETURN_OK;
}


/*********************************************************************************************************
** ��������: FsUMount
** ��������: ж�ؾ�
**
** �䡡��: Disk���߼�����Ϣָ��
**         Buf���߼�����0����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
**
********************************************************************************************************/
Uint8 FsUMount(void)
{
    FreeDriveCache();

    return RETURN_OK;
}

/*********************************************************************************************************
** ��������: FloppyDiskInit
** ��������: ���̸�ʽӲ����ʼ��
**
** �䡡��: DiakCommand����������ӿں���
**         RsvdForLow���������ײ��ָ��
**         Buf���߼�����0����
**         SecOffset���߼�����ƫ��
** �䡡��: ��
**         
********************************************************************************************************/
void FloppyDiskInit(Uint16  (* DiakCommand)(Uint8 Cammand, void *Parameter),
                       void *RsvdForLow , Uint8 *Buf, Uint32 SecOffset)
{
   	DiskInfo.DiakCommand = DiakCommand;
   	DiskInfo.SecOffset = SecOffset;
   	DiskInfo.RsvdForLow = RsvdForLow;

    FsMount(Buf);
}

/*********************************************************************************************************
** ��������: HardDiskInit
** ��������: Ӳ�̸�ʽӲ����ʼ��
**
** �䡡��: DiakCommand����������ӿں���
**         RsvdForLow���������ײ��ָ��
**         Buf������0����
** �䡡��: ��
**         
********************************************************************************************************/

void _HardDiskInit(Uint16  (* DiakCommand)(Uint8 Cammand, void *Parameter),
      void *RsvdForLow , Uint8 *Buf, Uint32 SecOffset, Uint16 offset)
{
    Uint32 temp1;
    Disk_RW_Parameter Pa;
    
    Pa.SectorIndex = SecOffset;
    Pa.RsvdForLow = RsvdForLow;
    Pa.Buf = Buf;
    if(DiakCommand(DISK_READ_SECTOR, &Pa) != DISK_READ_OK)
        return;

    if (Buf[510] != 0x55 || Buf[511] != 0xaa)       /* ��Ч��־ */
    {
        FloppyDiskInit(DiakCommand, RsvdForLow, Buf, SecOffset);
        return;
    }
    
    if ((Buf[0x1be] & 0xff7f) == 0x00)                /* ��1��������Ч */
    if ((Buf[0x1ce] & 0xff7f) == 0x00)                /* ��2��������Ч */
    if ((Buf[0x1de] & 0xff7f) == 0x00)                /* ��3��������Ч */
    if ((Buf[0x1ee] & 0xff7f) == 0x00)                /* ��4��������Ч */
    {
        temp1 = Buf[0x1be] + Buf[0x1ce] + Buf[0x1de] + Buf[0x1ee];
        if (temp1 != 0 && temp1 != 0x80)
        {
            FloppyDiskInit(DiakCommand, RsvdForLow, Buf, SecOffset);
            return;
        }
    }

    switch (Buf[offset + 4])    /* ��1���� */
    {
        case 0x01:              /* FAT12*/
        case 0x04:              /* FAT16*/
        case 0x06:              /* FAT16*/
        case 0x0b:              /* FAT32*/
        case 0x0c:              /* FAT32*/
        case 0x0e:              /* FAT16*/
        case 0x1b:              /* FAT32*/
        case 0x1c:              /* FAT32*/
			temp1 = Buf[offset + 11];
			temp1 = (temp1 << 8) | Buf[offset + 10];
			temp1 = (temp1 << 8) | Buf[offset + 9];
			temp1 = (temp1 << 8) | Buf[offset + 8];
            
            SecOffset = temp1 + SecOffset;

            Pa.SectorIndex = SecOffset;
            Pa.RsvdForLow = RsvdForLow;
            Pa.Buf = Buf;
            if(DiakCommand(DISK_READ_SECTOR, &Pa) != DISK_READ_OK)
            {
                return;
            }

            FloppyDiskInit(DiakCommand, RsvdForLow, Buf, SecOffset);
            break;
        default:
            break;
    }
}

/******************************************************************************
** ��������: AddFileDriver
** ��������: ����һ���ײ���������
**
** �䡡��: DiakCommand����������ӿں���
**
** �䡡��: ��
**         
** ȫ�ֱ���: DiskInfo
** ����ģ��: ��
**
*****************************************************************************/
void AddFileDriver(Uint16 (* DiakCommand)(Uint8 Cammand, void *Parameter), void *RsvdForLow)
{
    Uint8 Buf[DISK_CACHE_SIZE];
    Disk_RW_Parameter Pa;

    DiakCommand(DISK_INIT, &Pa);

    Pa.SectorIndex = 0;
    Pa.RsvdForLow = RsvdForLow;
    Pa.Buf = Buf;
    if(DiakCommand(DISK_READ_SECTOR, &Pa) != DISK_READ_OK)
        return;

    if ((Buf[510] != 0x55) || (Buf[511] != 0xaa))        /* ��Ч��־ */
		return;
	if ((Buf[0x1be] & 0xff7f) == 0)    /* ��1��������Ч */
		_HardDiskInit(DiakCommand, RsvdForLow, Buf, 0, 0x1be);
	if ((Buf[0x1ce] & 0xff7f) == 0)    /* ��2��������Ч */
		_HardDiskInit(DiakCommand, RsvdForLow, Buf, 0, 0x1ce);
	if ((Buf[0x1de] & 0xff7f) == 0)    /* ��3��������Ч */
		_HardDiskInit(DiakCommand, RsvdForLow, Buf, 0, 0x1de);
	if ((Buf[0x1ee] & 0xff7f) == 0)    /* ��4��������Ч */
		_HardDiskInit(DiakCommand, RsvdForLow, Buf, 0, 0x1ee);
}

/******************************************************************************
** ��������: RemoveFileDriver
** ��������: ɾ��һ���ײ���������
**
** �䡡��: DiakCommand:��������
**
** �䡡��: ��
**         
** ȫ�ֱ���: DiskInfo
** ����ģ��: ��
**
*****************************************************************************/
void RemoveFileDriver(Uint16 (* DiakCommand)(Uint8 Cammand, void *Parameter))
{
    Disk_RW_Parameter Pa;
    
    if (DiskInfo.DiakCommand != DiakCommand)
    {
        FsUMount();
        Pa.RsvdForLow = DiskInfo.RsvdForLow;
        DiskInfo.DiakCommand(DISK_CLOSE, &Pa);

        DiskInfo.FATType = 0xff;         // ��Ч���ļ�ϵͳ
        DiskInfo.SecPerClus = 0;         // ÿ��������
        DiskInfo.NumFATs = 0;            // FAT����
        DiskInfo.SecPerDisk = 0xffffffff;// �߼�����������������
        DiskInfo.BytsPerSec = 0;         // ÿ�����ֽ���
        DiskInfo.RootDirTable = 0;       // ��Ŀ¼��ʼ�����ţ�FAT32Ϊ��ʼ�غţ�
        DiskInfo.RootSecCnt = 0;         // ��Ŀ¼ռ��������
        DiskInfo.FATStartSec = 0;        // FAT��ʼ������
        DiskInfo.FATSecCnt = 0;          // ÿ��FATռ��������
        DiskInfo.DataStartSec = 0;       // ��������ʼ������
        DiskInfo.PathClusIndex = 0;      // ��ǰĿ¼
        DiskInfo.DiakCommand = NULL;     // ��������
    }
}

