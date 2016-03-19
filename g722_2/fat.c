/**************************************************************************
* Filename : fat.c
**************************************************************************/

#define IN_FAT

#include "typedef.h"
#include "fat.h"

extern Disk_Info DiskInfo;

/*********************************************************************************************************
** ��������: FATGetNextClus
** ��������: ����FAT��ָ���ص���һ���غ�
**
** �䡡��: Index���غ�
** �䡡��: ��һ���غ�
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
********************************************************************************************************/
Uint32 FATGetNextClus(Uint32 Index)
{
    Uint16 temp, ByteIndex;
    Uint32 SecIndex;
    Uint8 *Buf;
    Uint32 Rt;
    
    if (Index >= (DiskInfo.ClusPerData))
    {
        return BAD_CLUS;
    }
    
    /* ���������ź��ֽ����� */
    switch (DiskInfo.FATType)
    {
        case FAT12:
            SecIndex = Index * 3 / (2 * DiskInfo.BytsPerSec);
            ByteIndex = ((Index * 3) / 2) - (SecIndex * DiskInfo.BytsPerSec);
            SecIndex += DiskInfo.FATStartSec;
            break;
        case FAT16:
            SecIndex = Index * 2 / DiskInfo.BytsPerSec + DiskInfo.FATStartSec;
            ByteIndex = (Index * 2) & (DiskInfo.BytsPerSec - 1);
            break;
        case FAT32:
            SecIndex = Index * 4 / DiskInfo.BytsPerSec + DiskInfo.FATStartSec;
            ByteIndex = (Index * 4) & (DiskInfo.BytsPerSec - 1);
            break;
        default:
            return BAD_CLUS;
    }

    Buf = OpenSec(SecIndex);
    if (Buf == NULL)
    {
        return BAD_CLUS;
    }
    ReadSec(SecIndex, 0);
    
    /* ��ȡFAT������ */
    switch (DiskInfo.FATType)
    {
        case FAT12:
            temp = Buf[ByteIndex];
            ByteIndex++;
            if (ByteIndex >= DiskInfo.BytsPerSec)          /* ��һ���ֽ��Ƿ�����һ������ */
            {
                Buf = OpenSec(SecIndex + 1);
                if (Buf == NULL)
                {
                    return BAD_CLUS;
                }
                ReadSec(SecIndex + 1, 0);
                temp = temp | (Buf[0] << 8);
            }
            else
            {
                temp = temp | (Buf[ByteIndex] << 8);
            }
            if ((Index & 0x01) != 0)                /* �ж���12λ��Ч */
            {
                temp = temp >> 4;
            }
            else
            {
                temp = temp & 0x0fff;
            }
            Rt = temp;
            if (temp >= (BAD_CLUS & 0x0fff))        /* �Ƿ����������� */
            {
                Rt = ((Uint32)0x0fffL << 16) | (temp | 0xf000);
            }
            break;
        case FAT16:
            temp = Buf[ByteIndex] | (Buf[ByteIndex + 1] << 8);
            Rt = temp;
            if (temp >= (BAD_CLUS & 0xffff))        /* �Ƿ����������� */
            {
                Rt = ((Uint32)0x0fffL << 16) | temp;
            }
            break;
        case FAT32:
			Rt = Buf[ByteIndex + 3];
			Rt = (Rt << 8) | Buf[ByteIndex + 2];
			Rt = (Rt << 8) | Buf[ByteIndex + 1];
			Rt = (Rt << 8) | Buf[ByteIndex + 0];
            Rt = Rt & 0x0fffffff;
            break;
        default:
            Rt = BAD_CLUS;
            break;
    }
    return Rt;
}

/*********************************************************************************************************
** ��������: FATSetNextClus
** ��������: ������һ����
**
** �䡡��:Index���غ�
**        Next����һ���غ�
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
********************************************************************************************************/
void FATSetNextClus(Uint32 Index, Uint32 Next)
{
    Uint16 temp;
    Uint16 SecIndex, ByteIndex;
    Uint8 *Buf;
    
    if (Index <= EMPTY_CLUS_1)
    {
        return;
    }

    if (Index >= DiskInfo.ClusPerData)
    {
        return;
    }
    
    /* ���������ź��ֽ����� */
    switch (DiskInfo.FATType)
    {
        case FAT12:
            SecIndex = Index * 3 / (2 * DiskInfo.BytsPerSec);
            ByteIndex = ((Index * 3) / 2) - (SecIndex * DiskInfo.BytsPerSec);
            SecIndex += DiskInfo.FATStartSec;
            break;
        case FAT16:
            SecIndex = Index * 2 / DiskInfo.BytsPerSec + DiskInfo.FATStartSec;
            ByteIndex = (Index * 2) & (DiskInfo.BytsPerSec - 1);
            break;
        case FAT32:
            SecIndex = Index * 4 / DiskInfo.BytsPerSec + DiskInfo.FATStartSec;
            ByteIndex = (Index * 4) & (DiskInfo.BytsPerSec - 1);
            break;
        default:
            return;
    }

    Buf = OpenSec(SecIndex);
    if (Buf == NULL)
    {
        return;
    }
    ReadSec(SecIndex, 0);

    switch (DiskInfo.FATType)
    {
        case FAT12:
            temp = Next & 0x0fff;
            if ((Index & 0x01) != 0)                /* �ж���12λ��Ч */
            {
                temp = temp * 16;
                temp |= (Buf[ByteIndex] & 0x0f);
                Buf[ByteIndex] = temp;
            }
            else
            {
                Buf[ByteIndex] = temp;
            }
            ByteIndex++;
            temp = temp >> 8;
            if (ByteIndex >= DiskInfo.BytsPerSec)          /* ��һ���ֽ��Ƿ�����һ������ */
            {
                Buf = OpenSec(SecIndex + 1);
                if (Buf == NULL)
                {
                    break;
                }
                ReadSec(SecIndex + 1, 0);
                if ((Index & 0x01) != 0)                /* �ж���12λ��Ч */
                {
                    Buf[0] = temp;
                }
                else
                {
                    Buf[0] = (Buf[0] & 0xf0) | temp;
                }
                WriteSec(SecIndex + 1, 0);
            }
            else
            {
                if ((Index & 0x01) != 0)                /* �ж���12λ��Ч */
                {
                    Buf[ByteIndex] = temp;
                }
                else
                {
                    Buf[ByteIndex] = (Buf[ByteIndex] & 0xf0) | temp;
                }
            }
            break;
        case FAT16:
            Buf[ByteIndex] = Next;
            Buf[ByteIndex + 1] = Next >> 8;
            break;
        case FAT32:
            Buf[ByteIndex] = Next;
            Buf[ByteIndex + 1] = Next >> 8;
            Buf[ByteIndex + 2] = Next >> 16;
            Buf[ByteIndex + 3] = (Buf[ByteIndex + 3] & 0xf0) | ((Next >> 24) & 0x0f);
            break;
        default:
            break;
    }
    WriteSec(SecIndex, 0);
    return ;
}

/*********************************************************************************************************
** ��������: FATAddClus
** ��������: Ϊָ����������һ����
**
** �䡡��: Index������������һ���غţ����Ϊ0����Ϊһ����������һ����
** �䡡��: ���ӵĴغ�
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
********************************************************************************************************/
Uint32 FATAddClus(Uint32 Index)
{
    Uint32 NextClus,ThisClus,MaxClus;

    if (Index >= BAD_CLUS)
    {
        return BAD_CLUS;
    }

    MaxClus = DiskInfo.ClusPerData;

    /* �������һ���� */
    ThisClus = Index;
    if (ThisClus != EMPTY_CLUS && ThisClus != EMPTY_CLUS_1)
    {
        while (1)
        {
            NextClus = FATGetNextClus(ThisClus);
            if (NextClus >= EOF_CLUS_1)
            {
                break;
            }
            if (NextClus <= EMPTY_CLUS_1)
            {
                break;
            }
            if (NextClus == BAD_CLUS)
            {
                return BAD_CLUS;
            }
            ThisClus = NextClus;
        }
    }
    else
    {
        ThisClus = EMPTY_CLUS_1;
    }
    
    for (NextClus = ThisClus + 1; NextClus < MaxClus; NextClus++)
    {
        if (FATGetNextClus(NextClus) == EMPTY_CLUS)
        {
            break;
        }
    }
    if (NextClus >= MaxClus)
    {
        for (NextClus = EMPTY_CLUS_1 + 1; NextClus < ThisClus; NextClus++)
        {
            if (FATGetNextClus(NextClus) == EMPTY_CLUS)
            {
                break;
            }
        }
    }
    if (FATGetNextClus(NextClus) == EMPTY_CLUS)
    {
        if (ThisClus > EMPTY_CLUS_1)
        {
            FATSetNextClus(ThisClus, NextClus);
        }
        FATSetNextClus(NextClus, EOF_CLUS_END);
        return NextClus;
    }
    else
    {
        return BAD_CLUS;
    }
}

/******************************************************************************
** ��������: FreeClus
** ��������: ��ָ����������������
**
** �䡡��: Index���غ�
** �䡡��: RETURN_OK:�ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: OpenSec,WriteSec,CloseSec
**
*****************************************************************************/
Uint8 FreeClus(Uint32 Index)
{
    Int16 temp;
    Uint32 SecIndex;
    Disk_RW_Parameter parameter;

    if (Index < DiskInfo.ClusPerData)
    {
        temp = DiskInfo.SecPerClus;
        Index -= 2;                     /* 2���������� */
        SecIndex = DiskInfo.DataStartSec + Index * temp;
        parameter.RsvdForLow = DiskInfo.RsvdForLow;
        do
        {
            parameter.SectorIndex = SecIndex;
            DiskInfo.DiakCommand(DISK_FREE_SECTOR, &parameter);
            FreeCache(SecIndex);
            SecIndex++;
        } while (--temp > 0);
        return RETURN_OK;
    }
    else
    {
        return CLUSTER_NOT_IN_DISK;
    }
}

/******************************************************************************
** ��������: FATDelClusChain
** ��������: ɾ��ָ������
**
** �䡡��: Index���������״غ�
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: FATGetNextClus,FATSetNextClus
*****************************************************************************/
void FATDelClusChain(Uint32 Index)
{
    Uint32 NextClus, ThisClus;
    
    if (Index <= EMPTY_CLUS_1)
    {
        return;
    }
    if (Index >= BAD_CLUS)
    {
        return;
    }
    ThisClus = Index;
    while (1)
    {
        NextClus = FATGetNextClus(ThisClus);
        FATSetNextClus(ThisClus, EMPTY_CLUS);
        FreeClus(ThisClus);

        if (NextClus >= BAD_CLUS)
        {
            break;
        }
        if (NextClus <= EMPTY_CLUS_1)
        {
            break;
        }
        ThisClus = NextClus;
    }
}

