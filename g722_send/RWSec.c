/****************************************************************************
* Filename: rwsec.c
****************************************************************************/
#define IN_RWSEC

#include "tistdtypes.h"
#include "fat.h"

#define BASE_CACHE_NUM      1
#define MAX_DISK_CACHES     16

Disk_cache DiskCache[MAX_DISK_CACHES];      /* ����cache */
extern Disk_Info DiskInfo;

/*****************************************************************************
** ��������: CacheInit
** ��������: ��ʼ������cache
**
** �䡡��: ��
**
** �䡡��: ��
**         
** ȫ�ֱ���: DiskCache
** ����ģ��: ��
**
****************************************************************************/
void CacheInit(void)
{
    Uint16 i;
    
    for (i = 0; i < MAX_DISK_CACHES; i++)
    {
        DiskCache[i].Flag = 0;              /* ���������д */
        DiskCache[i].RW_ID = (Uint16)(~0);  /* ���ʼ�¼ */
        DiskCache[i].SecIndex = 0;          /* �����������  */
    }
}


/*****************************************************************************
** ��������: CacheWriteBack2
** ��������: ��ָ������д���߼���
**
** �䡡��: Index��cache����
**
** �䡡��: ��
**         
** ȫ�ֱ���: DiskCache
** ����ģ��:
**
** ˵  ��: �ڻ�дcacheʱ���жϸ������Ƿ�ΪFAT������ǣ������������ǵ�
           ��������FAT��ʵ��ͬ����
****************************************************************************/
void CacheWriteBack2(Uint16 Index)
{
    Disk_RW_Parameter Pa;
    Int16 i;

    if (DiskInfo.DiakCommand != NULL)
    {
        i = 1;
        if(DiskCache[Index].SecIndex >= DiskInfo.FATStartSec)  /* �ж�д�������Ƿ�Ϊ��FAT��Ŀռ� */
        if(DiskCache[Index].SecIndex < DiskInfo.FATStartSec + DiskInfo.FATSecCnt)
        {
            i = DiskInfo.NumFATs;                        /* ��Ҫͬ����FAT����� */
        }
        
        /* ���ݲ��� */
        Pa.SectorIndex = DiskCache[Index].SecIndex + DiskInfo.SecOffset;
        Pa.RsvdForLow = DiskInfo.RsvdForLow;
        Pa.Buf = DiskCache[Index].buf;              

        do             /* ѭ��д���FAT�����Ϊ��ͨ������ֻ��һ�� */
        {
//			if(DiskCache[Index].Flag & CACHE_DATA)
//				DiskInfo.DiakCommand(DISK_WRITE_RAW, &Pa);
//			else
				DiskInfo.DiakCommand(DISK_WRITE_SECTOR, &Pa);
            /* ���õײ�����д���� */ 
			Pa.SectorIndex += DiskInfo.FATSecCnt;
        } while (--i > 0);
        /* cache ������Ҫ��д */
        DiskCache[Index].Flag &= ~(CACHE_WRITTEN|CACHE_DATA);
    }
}

/*****************************************************************************
** ��������: AllCacheWriteBack
** ��������: �������Ѹı������д���߼���
**
** �䡡��: ��
**
** �䡡��: ��
**         
** ȫ�ֱ���: DiskCache
** ����ģ��: CacheWriteBack2
**
****************************************************************************/
void AllCacheWriteBack(void)
{
    Uint16 i;

    for (i = 0; i < MAX_DISK_CACHES; i++)
    {
        if (DiskCache[i].Flag & CACHE_WRITTEN)
        {
            CacheWriteBack2(i);
        }
    }
}

/*********************************************************************************************************
** ��������: GetCache
** ��������: ��ȡһ��cache
**
** �䡡��: ��
**
** �䡡��: cache����
**         
** ȫ�ֱ���: DiskCache
** ����ģ��: ��
**
********************************************************************************************************/
Uint16 GetCache(Uint32 Index)
{
    Uint16 Max;
    Uint16 i, j;
    Uint16 temp;
    
    Max = 0;
    j = ~0;
    temp = Index & ((1 << BASE_CACHE_NUM) - 1);
    for (i = 0; i < (1 << BASE_CACHE_NUM); i++)
    {
        if (Max <= DiskCache[temp].RW_ID)
        {
            Max = DiskCache[temp].RW_ID;
            j = temp;
        }
        if (Max == ~0)
        {
            break;
        }
        temp += MAX_DISK_CACHES >> BASE_CACHE_NUM;
    }
    if (j < MAX_DISK_CACHES)
    {
        if (DiskCache[j].Flag & CACHE_WRITTEN)
        {
            CacheWriteBack2(j);
        }
    }
    return j;
}

/*********************************************************************************************************
** ��������: OpenSec
** ��������: Ϊ�߼����ϵ�һ��������һ��cache������
**
** �䡡��: Index��������
** �䡡��: ָ��ָ���������ݵ�ָ��
**         
** ȫ�ֱ���: DiskCache
** ����ģ��: ��
**
********************************************************************************************************/
Uint8 *OpenSec(Uint32 Index)
{
    Uint16 i;
    Uint8 *Rt;
    Uint16 temp;
    
    /* ���ӷ��ʼ�� */
    temp = Index & ((1 << BASE_CACHE_NUM) - 1);
    for (i = 0; i < (1 << BASE_CACHE_NUM); i++)
    {   
        if (DiskCache[temp].RW_ID < (Uint16)(~0))
        {
            DiskCache[temp].RW_ID++;
        }

        temp += MAX_DISK_CACHES >> BASE_CACHE_NUM;
    }

    /* �������Ƿ��Ѿ����� */
    Rt = NULL;
    temp = Index & ((1 << BASE_CACHE_NUM) - 1);
    for (i = 0; i < (1 << BASE_CACHE_NUM); i++)
    {   
        if (DiskCache[temp].SecIndex == Index)
        {
            Rt = DiskCache[temp].buf;
            DiskCache[temp].RW_ID = 0;
            break;
        }
        temp += MAX_DISK_CACHES >> BASE_CACHE_NUM;
    }
    
    /* Rt == NULL��δ���� */
    if (Rt == NULL)
    {
        if (DiskInfo.SecPerDisk > Index)
        {
            i = GetCache(Index);                     /* ��ȡcache */
            if (i < MAX_DISK_CACHES)
            {
                /* ��ʼ��cache  */
                DiskCache[i].RW_ID = 0;
                DiskCache[i].Flag = 0;
                DiskCache[i].SecIndex = Index;
                Rt = DiskCache[i].buf;
            }
        }
    }
    return Rt;
}

/*********************************************************************************************************
** ��������: ReadSec
** ��������: ���߼��̶�����
**
** �䡡��: Index��������
** �䡡��: TRUE:�ɹ�
**         FALSE:ʧ��
** ȫ�ֱ���: DiskCache
** ����ģ��:
**
********************************************************************************************************/
Uint8 ReadSec(Uint32 Index, Uint16 isData)
{
    Uint16 i;
    Disk_RW_Parameter Pa;
    Uint16 Rt;
    Uint16 temp;
    
    temp = Index & ((1 << BASE_CACHE_NUM) - 1);
    for (i = 0; i < (1 << BASE_CACHE_NUM); i++)
    {
        if (DiskCache[temp].SecIndex == Index)
        {
            if (DiskCache[temp].Flag & CACHE_READ)
            {
                return RETURN_OK;
            }
            break;
        }
        temp += MAX_DISK_CACHES >> BASE_CACHE_NUM;
    }
    Rt = SECTOR_NOT_IN_CACHE;
    if (temp < MAX_DISK_CACHES)
    {
        DiskCache[temp].Flag |= CACHE_READ;

        /* �Ӵ��̶�ȡ�������� */
        Pa.SectorIndex = Index + DiskInfo.SecOffset;
        Pa.RsvdForLow = DiskInfo.RsvdForLow;
        Pa.Buf = DiskCache[temp].buf;
        Rt = NOT_FIND_DISK;
        if (DiskInfo.DiakCommand != NULL)
        {
            if (isData)
            	DiskInfo.DiakCommand(DISK_READ_RAW, &Pa);
            else
            	DiskInfo.DiakCommand(DISK_READ_SECTOR, &Pa);
            return RETURN_OK;
        }
    }
    return Rt;
}

/*********************************************************************************************************
** ��������: WriteSec
** ��������: ˵��ָ���߼��̵�ָ��һ����������д
**
** �䡡��: Index��������
** �䡡��: ��
**         
** ȫ�ֱ���: DiskCache
** ����ģ��: ��
**
********************************************************************************************************/
void WriteSec(Uint32 Index, Uint16 isData)
{
    Uint16 i, temp;

    temp = Index & ((1 << BASE_CACHE_NUM) - 1);
    for (i = 0; i < (1 << BASE_CACHE_NUM); i++)
    {   
        if (DiskCache[temp].SecIndex == Index)
        {
            DiskCache[temp].Flag = (CACHE_WRITTEN | CACHE_READ | isData);
            break;
        }
        temp += MAX_DISK_CACHES >> BASE_CACHE_NUM;
    }
}

/*********************************************************************************************************
** ��������: FreeDriveCache
** ��������: �ͷ�ָ���߼��̵�����
**
** �䡡��: 
**
** �䡡��: ��
**         
********************************************************************************************************/
void FreeDriveCache(void)
{
    Uint16 i;
    
    for (i = 0; i < MAX_DISK_CACHES; i++)
    {
        if (DiskCache[i].Flag & CACHE_WRITTEN) 
        {
            CacheWriteBack2(i);
        }
        DiskCache[i].RW_ID = (Uint16)(~0);
    }
}

/*********************************************************************************************************
** ��������: FreeCache
** ��������: �ͷ�ָ��������
**
** �䡡��:
** �䡡��: ��
**         
********************************************************************************************************/
void FreeCache(Uint32 Index)
{
    Uint16 i, temp;
    
    temp = Index & ((1 << BASE_CACHE_NUM) - 1);
    for (i = 0; i < (1 << BASE_CACHE_NUM); i++)
    {
        temp += MAX_DISK_CACHES >> BASE_CACHE_NUM;

        if (DiskCache[temp].SecIndex == Index)
        {
            if (DiskCache[temp].Flag & CACHE_WRITTEN)
            {
                CacheWriteBack2(temp);
            }
            DiskCache[temp].RW_ID = (Uint16)(~0);
            break;
        }
    }
}

