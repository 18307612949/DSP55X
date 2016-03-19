/***************************************************************************
* Filename : floppy.c
****************************************************************************/
#define IN_FDT

#include "tistdtypes.h"
#include "fat.h"

extern Disk_Info DiskInfo;

/*********************************************************************************************************
** ��������: ClearClus
** ��������: ��ָ����������������
**
** �䡡��: Index���غ�
** �䡡��: RETURN_OK:�ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: OpenSec,WriteSec,CloseSec
**
********************************************************************************************************/
Uint8 ClearClus(Uint32 Index)
{
    Uint8 i, temp;
    Uint32 SecIndex;

    if (Index < DiskInfo.ClusPerData)
    {
        temp = DiskInfo.SecPerClus;
        Index -= 2;                     /* 2���������� */
        SecIndex = DiskInfo.DataStartSec + Index * temp;
        for (i = 0; i < temp; i++)
        {
            OpenSec(SecIndex);
            WriteSec(SecIndex, 0);
            SecIndex++;
        }
        return RETURN_OK;
    }
    else
    {
        return CLUSTER_NOT_IN_DISK;
    }
}

/*********************************************************************************************************
** ��������: ReadFDTInfo
** ��������: ��ȡFDT��Ϣ
**
** �䡡��:SecIndex��������
**        ByteIndex��ƫ����
**        Rt���洢������Ϣ��ָ��
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: OpenSec,ReadSec,CloseSec
**
********************************************************************************************************/
Uint8  ReadFDTInfo(Uint32 SecIndex, Uint16 ByteIndex, FDT *Rt)
{
    Uint8 *Buf;
    Uint8 temp;
    
    Buf = OpenSec(SecIndex);
    temp = NOT_EMPTY_CACHE;
    if (Buf != NULL)
    {
        temp = ReadSec(SecIndex, 0);
        if (temp == RETURN_OK)
        {
            Buf = Buf + ByteIndex;
            Rt->Name[0] = *Buf++;
            Rt->Name[1] = *Buf++;
            Rt->Name[2] = *Buf++;
            Rt->Name[3] = *Buf++;
            Rt->Name[4] = *Buf++;
            Rt->Name[5] = *Buf++;
            Rt->Name[6] = *Buf++;
            Rt->Name[7] = *Buf++;
            Rt->Name[8] = *Buf++;
            Rt->Name[9] = *Buf++;
            Rt->Name[10] = *Buf++;
            Rt->Attr = *Buf++;
            Rt->NTRes = *Buf++;
            Rt->CrtTimeTenth = *Buf++;
            Rt->CrtTime = Buf[0] | (Buf[1] << 8);
            Rt->CrtDate = Buf[2] | (Buf[3] << 8);
            Rt->LstAccDate = Buf[4] | (Buf[5] << 8);
            Rt->FstClusHI = Buf[6] | (Buf[7] << 8);
            Rt->WrtTime = Buf[8] | (Buf[9] << 8);
            Rt->WrtDate = Buf[10] | (Buf[11] << 8);
            Rt->FstClusLO = Buf[12] | (Buf[13] << 8);

            Rt->FileSize = Buf[17];
            Rt->FileSize = (Rt->FileSize << 8) | Buf[16];
            Rt->FileSize = (Rt->FileSize << 8) | Buf[15];
            Rt->FileSize = (Rt->FileSize << 8) | Buf[14];
            temp = RETURN_OK;
        }
    }
    return temp;
}

/*********************************************************************************************************
** ��������: WriteFDTInfo
** ��������: дFDT��Ϣ
**
** �䡡��:
**        SecIndex��������
**        ByteIndex��ƫ����
**        FDT *FDTData:����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: OpenSec,ReadSec,CloseSec
**
********************************************************************************************************/
Uint8  WriteFDTInfo(Uint32 SecIndex, Uint16 ByteIndex, FDT *FDTData)
{
    Uint8 *Buf;
    Uint8 temp;
    
    temp = NOT_EMPTY_CACHE;
    Buf = OpenSec(SecIndex);
    if (Buf != NULL)
    {
        temp = ReadSec(SecIndex, 0);
        if (temp == RETURN_OK)
        {
            Buf = Buf + ByteIndex;
            *Buf++ = FDTData->Name[0];
            *Buf++ = FDTData->Name[1];
            *Buf++ = FDTData->Name[2];
            *Buf++ = FDTData->Name[3];
            *Buf++ = FDTData->Name[4];
            *Buf++ = FDTData->Name[5];
            *Buf++ = FDTData->Name[6];
            *Buf++ = FDTData->Name[7];
            *Buf++ = FDTData->Name[8];
            *Buf++ = FDTData->Name[9];
            *Buf++ = FDTData->Name[10];
            *Buf++ = FDTData->Attr;
            *Buf++ = FDTData->NTRes;
            *Buf++ = FDTData->CrtTimeTenth;
            *Buf++ = FDTData->CrtTime;
            *Buf++ = (FDTData->CrtTime) >> 8;
            *Buf++ = FDTData->CrtDate;
            *Buf++ = (FDTData->CrtDate) >> 8;
            *Buf++ = FDTData->LstAccDate;
            *Buf++ = (FDTData->LstAccDate) >> 8;
            *Buf++ = FDTData->FstClusHI;
            *Buf++ = (FDTData->FstClusHI) >> 8;
            *Buf++ = FDTData->WrtTime;
            *Buf++ = (FDTData->WrtTime) >> 8;
            *Buf++ = FDTData->WrtDate;
            *Buf++ = (FDTData->WrtDate) >> 8;
            *Buf++ = FDTData->FstClusLO;
            *Buf++ = (FDTData->FstClusLO) >> 8;
            *Buf++ = (FDTData->FileSize);
            *Buf++ = (FDTData->FileSize) >> 8;
            *Buf++ = (FDTData->FileSize) >> 16;
            *Buf = (FDTData->FileSize) >> 24;
            WriteSec(SecIndex, 0);
        }
    }
    return temp;
}

/*********************************************************************************************************
** ��������: RootFDTInfo
** ��������: ��ȡ/���ø�Ŀ¼ָ���ļ���Ŀ¼����Ϣ
**
** �䡡��: Index���ļ���Ŀ¼����FDT�е�λ��
**         RDTData���洢������Ϣ��ָ��
**         Get/Set�� get or set
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: ReadFDTInfo
**
********************************************************************************************************/
Uint8 RootFDTInfo(Uint32 Index, FDT *FDTData, Uint16 type)
{
    Uint16 ByteIndex;
    Uint32 SecIndex;
    Uint8 Rt;
    
    Rt = NOT_FAT_DISK;

    Index = Index * 32;        /* 32:sizeof(FDT ) */
    if (DiskInfo.FATType == FAT12 || DiskInfo.FATType == FAT16)
    {
        Rt = FDT_OVER;
        if (Index < (DiskInfo.RootSecCnt * DiskInfo.BytsPerSec))
        {
            ByteIndex = Index % DiskInfo.BytsPerSec;
            SecIndex = Index / DiskInfo.BytsPerSec + DiskInfo.RootDirTable;
			if(type == FDTGET)
                Rt = ReadFDTInfo(SecIndex, ByteIndex, FDTData);
            else
                Rt = WriteFDTInfo(SecIndex, ByteIndex, FDTData);
        }
    }

    return Rt;
}

/*********************************************************************************************************
** ��������: FDTInfo
** ��������: ��ȡ/����ָ��Ŀ¼ָ���ļ���Ŀ¼����Ϣ
**
** �䡡��: ClusIndex��Ŀ¼�״غ�
**         Index���ļ���Ŀ¼����FDT�е�λ��
**         FDTData��д��/������Ϣ��ָ��
** �䡡��: RETURN_OK���ɹ�
**
** ȫ�ֱ���: ��
** ����ģ��: ReadFDTInfo
**
********************************************************************************************************/
Uint8 FDTInfo(Uint32 ClusIndex, Uint32 Index, FDT *FDTData, Uint16 type)
{
    Uint16 ByteIndex, i;
    Uint32 SecIndex;
    Uint8  Rt;

    if (ClusIndex == EMPTY_CLUS)
    {
        if (DiskInfo.FATType == FAT32)
            ClusIndex = DiskInfo.RootDirTable;
        else
            return RootFDTInfo(Index, FDTData, type);
    }

    Rt = NOT_FAT_DISK;
    if (DiskInfo.FATType == FAT12 ||
        DiskInfo.FATType == FAT16 ||
        DiskInfo.FATType == FAT32)
    {
        Index = Index * 32;
        ByteIndex = Index % DiskInfo.BytsPerSec;
        SecIndex = Index / DiskInfo.BytsPerSec;
        /* ����Ŀ¼���������� */
        i = DiskInfo.SecPerClus;
        while(SecIndex >= i)
        {
            ClusIndex = FATGetNextClus(ClusIndex);
            if (ClusIndex <= EMPTY_CLUS_1 ||
                ClusIndex >= BAD_CLUS) 
            {
                return FDT_OVER;
            }
            SecIndex -= i;
        }
        SecIndex = (ClusIndex - 2) * i + SecIndex + DiskInfo.DataStartSec;
        if(type == FDTGET)
            Rt = ReadFDTInfo(SecIndex, ByteIndex, FDTData);
        else
            Rt = WriteFDTInfo(SecIndex, ByteIndex, FDTData);
    }
    return Rt;
}


/*********************************************************************************************************
** ��������: FindFDTInfo
** ��������: ��ָ��Ŀ¼����ָ���ļ���Ŀ¼��
**
** �䡡��: Rt���洢������Ϣ��ָ��
**        ClusIndex��Ŀ¼�״غ�
**        FileName���ļ���Ŀ¼����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: FDTInfo
********************************************************************************************************/
Uint8 FindFDTInfo(FDT *Rt, Uint32 ClusIndex, Int8 FileName[])
{
    Uint32 i;
    Uint8 temp;
    
    if (FileName[0] == DEL_FDT)
        FileName[0] = ESC_FDT;

	for(i = 0;; i++)
    {
        temp = FDTInfo(ClusIndex, i, Rt, FDTGET);
        if (temp != RETURN_OK)
        {
            break;
        }

        if (Rt->Name[0] == EMPTY_FDT)
        {
            temp = NOT_FIND_FDT;
            break;
        }
        if ((Rt->Attr & ATTR_VOLUME_ID) == 0)
        if (FileName[0] == Rt->Name[0])
        if (FileName[1] == Rt->Name[1])
        if (FileName[2] == Rt->Name[2])
        if (FileName[3] == Rt->Name[3])
        if (FileName[4] == Rt->Name[4])
        if (FileName[5] == Rt->Name[5])
        if (FileName[6] == Rt->Name[6])
        if (FileName[7] == Rt->Name[7])
        if (FileName[8] == Rt->Name[8])
        if (FileName[9] == Rt->Name[9])
        if (FileName[10] == Rt->Name[10])
        {
            temp = RETURN_OK;
            break;
        }
    }
    if (FileName[0] == ESC_FDT)
        FileName[0] = DEL_FDT;

    return temp;
}

/*********************************************************************************************************
** ��������: AddFDT
** ��������: ��ָ��Ŀ¼������ָ���ļ���Ŀ¼��
**
** �䡡��: ClusIndex��Ŀ¼�״غ�
**         FileName���ļ���Ŀ¼����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: FindFDTInfo,FDTInfo
**
********************************************************************************************************/
Uint8 AddFDT(Uint32 ClusIndex, FDT *FDTData)
{
    Uint32 i;
    FDT TempFDT;
    Uint8 temp;

    if (ClusIndex == EMPTY_CLUS)
    if (DiskInfo.FATType == FAT32)
    {
        ClusIndex = DiskInfo.RootDirTable;
    }

    temp = FindFDTInfo(&TempFDT, ClusIndex, FDTData->Name);
    if (temp == RETURN_OK)
    {
        return FDT_EXISTS;
    }
    if (temp != NOT_FIND_FDT)
    if (temp != FDT_OVER)
    {
        return temp;
    }

    if (FDTData->Name[0] == DEL_FDT)
        FDTData->Name[0] = ESC_FDT;

    temp = RETURN_OK;
    for(i = 0; temp == RETURN_OK; i++)
    {
        temp = FDTInfo(ClusIndex, i, &TempFDT, FDTGET);
        if (temp == RETURN_OK)
        {
            if (TempFDT.Name[0] == DEL_FDT || TempFDT.Name[0] == EMPTY_FDT)
            {
                temp = FDTInfo(ClusIndex, i, FDTData, FDTSET);
                break;
            }
        }
    }

    if (temp == FDT_OVER)
    if (ClusIndex != EMPTY_CLUS)
    {
        i = FATAddClus(ClusIndex);
        temp = DISK_FULL;
        if (i != BAD_CLUS)
        {
            ClearClus(i); 
            temp = FDTInfo(i, 0, FDTData, FDTSET);
        }
    }
    if (FDTData->Name[0] == ESC_FDT)
        FDTData->Name[0] = DEL_FDT;

    return temp;
}

/*********************************************************************************************************
** ��������: DelFDT
** ��������: ��ָ��Ŀ¼ɾ��ָ���ļ���Ŀ¼��
**
** �䡡��: ClusIndex��Ŀ¼�״غ�
**         FileName���ļ���Ŀ¼����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: FDTInfo
**
********************************************************************************************************/
Uint8 DelFDT(Uint32 ClusIndex, Int8 FileName[])
{
    Uint32 i;
    FDT TempFDT;
    Uint8 temp;

    if (FileName[0] == DEL_FDT)
        FileName[0] = ESC_FDT;

    for(i = 0;; i++)
    {
        temp = FDTInfo(ClusIndex, i, &TempFDT, FDTGET);
        if (temp != RETURN_OK)
        {
            break;
        }
            
        if (TempFDT.Name[0] == EMPTY_FDT)
        {
            temp = NOT_FIND_FDT;
            break;
        }
        if ((TempFDT.Attr & ATTR_VOLUME_ID) == 0)
        if (FileName[0] == TempFDT.Name[0])
        if (FileName[1] == TempFDT.Name[1])
        if (FileName[2] == TempFDT.Name[2])
        if (FileName[3] == TempFDT.Name[3])
        if (FileName[4] == TempFDT.Name[4])
        if (FileName[5] == TempFDT.Name[5])
        if (FileName[6] == TempFDT.Name[6])
        if (FileName[7] == TempFDT.Name[7])
        if (FileName[8] == TempFDT.Name[8])
        if (FileName[9] == TempFDT.Name[9])
        if (FileName[10] == TempFDT.Name[10])
        {
            TempFDT.Name[0] = DEL_FDT;
            temp = FDTInfo(ClusIndex, i, &TempFDT, FDTSET);
            break;
        }
    }

    if (FileName[0] == ESC_FDT)
        FileName[0] = DEL_FDT;

    return temp;
}

/*********************************************************************************************************
** ��������: ChangeFDT
** ��������: �ı�ָ��Ŀ¼ָ���ļ���Ŀ¼��������
**
** �䡡��: ClusIndex��Ŀ¼�״غ�
**         FileName���ļ���Ŀ¼����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: FDTInfo
**
********************************************************************************************************/
Uint8 ChangeFDT(Uint32 ClusIndex, FDT *FDTData)
{
    Uint32 i;
    Uint8 temp;
    FDT TempFDT;

    if (FDTData->Name[0] == DEL_FDT)
        FDTData->Name[0] = ESC_FDT;

    for(i = 0;; i++)
    {
        temp = FDTInfo(ClusIndex, i, &TempFDT, FDTGET);
        if (temp != RETURN_OK)
        {
            break;
        }
            
        if (TempFDT.Name[0] == EMPTY_FDT)
        {
            temp = NOT_FIND_FDT;
            break;
        }
        if ((TempFDT.Attr & ATTR_VOLUME_ID) == 0)
        if (FDTData->Name[0] == TempFDT.Name[0])
        if (FDTData->Name[1] == TempFDT.Name[1])
        if (FDTData->Name[2] == TempFDT.Name[2])
        if (FDTData->Name[3] == TempFDT.Name[3])
        if (FDTData->Name[4] == TempFDT.Name[4])
        if (FDTData->Name[5] == TempFDT.Name[5])
        if (FDTData->Name[6] == TempFDT.Name[6])
        if (FDTData->Name[7] == TempFDT.Name[7])
        if (FDTData->Name[8] == TempFDT.Name[8])
        if (FDTData->Name[9] == TempFDT.Name[9])
        if (FDTData->Name[10] == TempFDT.Name[10])
        {
            temp = FDTInfo(ClusIndex, i, FDTData, FDTSET);
            break;
        }
    }

    if (FDTData->Name[0] == ESC_FDT)
        FDTData->Name[0] = DEL_FDT;

    return temp;
}

/*********************************************************************************************************
** ��������: DirIsEmpty
** ��������: 
**
** �䡡��: Drive����������
**        ClusIndex��Ŀ¼�״غ�
** �䡡��: DIR_EMPTY����
**        DIR_NOT_EMPTY������
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: FDTInfo
**
********************************************************************************************************/
Uint8 DirIsEmpty(Uint32 ClusIndex)
{
    Uint32 i;
    FDT TempFDT;
    Uint8 temp;

    if (ClusIndex == EMPTY_CLUS)
    {
        return DIR_NOT_EMPTY;
    }

    /* �ж��Ƿ���fat32 �ĸ�Ŀ¼ */
    if (DiskInfo.FATType == FAT32)
    if (ClusIndex == DiskInfo.RootDirTable )
    {
        return DIR_NOT_EMPTY;
    }
    
	for(i = 2;; i++)
    {
        temp = FDTInfo(ClusIndex, i, &TempFDT, FDTGET);
        if (temp != RETURN_OK)
        {
           return temp; 
        }
    
        if ((TempFDT.Attr & ATTR_VOLUME_ID) == 0)
        if (TempFDT.Name[0] != DEL_FDT)
        {
            break;
        }
    }

    if (TempFDT.Name[0] == EMPTY_FDT)
    {
        return DIR_EMPTY;
    }
    else
    {
        return DIR_NOT_EMPTY;
    }
}

/*********************************************************************************************************
** ��������: FDTIsLie
** ��������: ��ָ��Ŀ¼�鿴ָ���ļ���Ŀ¼���Ƿ����
**
** �䡡��: Drive����������
**        ClusIndex��Ŀ¼�״غ�
**        FileName���ļ���Ŀ¼����
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: FDTInfo
**
********************************************************************************************************/
Uint8 FDTIsLie(Uint32 ClusIndex, Int8 FileName[])
{
    Uint32 i;
    Uint8 temp;
    FDT temp1;
    
    if (FileName[0] == DEL_FDT)
        FileName[0] = ESC_FDT;

	for(i = 0;; i++)
    {
        temp = FDTInfo(ClusIndex, i, &temp1, FDTGET);
        if (temp == FDT_OVER)
        {
            temp = NOT_FIND_FDT;
            break;
        }

        if (temp != RETURN_OK)
        {
            break;
        }

        if (temp1.Name[0] == EMPTY_FDT)
        {
            temp = NOT_FIND_FDT;
            break;
        }
        if ((temp1.Attr & ATTR_VOLUME_ID) == 0)
        if (FileName[0] == temp1.Name[0])
        if (FileName[1] == temp1.Name[1])
        if (FileName[2] == temp1.Name[2])
        if (FileName[3] == temp1.Name[3])
        if (FileName[4] == temp1.Name[4])
        if (FileName[5] == temp1.Name[5])
        if (FileName[6] == temp1.Name[6])
        if (FileName[7] == temp1.Name[7])
        if (FileName[8] == temp1.Name[8])
        if (FileName[9] == temp1.Name[9])
        if (FileName[10] == temp1.Name[10])
        {
            temp = FDT_EXISTS;
            break;
        }
    }
    if (FileName[0] == ESC_FDT)
        FileName[0] = DEL_FDT;

    return temp;
}

