/**************************************************************************
* Filename:  dir.c
**************************************************************************/
#define IN_DIR

#include "tistdtypes.h"
#include "fat.h"

extern Disk_Info DiskInfo;
Int8 FsPathFileName[MAX_PATHFILE_LENGTH + 1];
Uint32 GetDirClusIndex(Int8 *Path);

/******************************************************************************
** ��������: FsStrCopy
** ��������: ���ַ��������ڲ����棬Сд���д
**
** �䡡��: source:�ַ���
**        
** �䡡��: �ڲ������ַ
**
** ȫ�ֱ���: ��
** ����ģ��: ��
********************************************************************************************************/
Int8 *FsStrCopy(Int8 *source)
{
    Uint16 i;
    Int8 *destin;
    
    if (source == NULL)
    {
        return NULL;
    }
    destin = FsPathFileName;
    for(i = 0; i < MAX_PATHFILE_LENGTH; i++)
    {
        destin[i] = source[i];
        if (destin[i] == 0)
            break;
		if(destin[i] >= 'a' && destin[i] <= 'z')
			destin[i] &= ~0x20;
	}

    return FsPathFileName;
}

/******************************************************************************
** ��������: FS_GetDateTime
** ��������: ʱ���ʽת����������DATE_TIME��ʽ������ת��ΪSYS_TIME��ʽ��ʱ������
**
** �䡡��: CurTime  ָ�򱣴���
**
** �䡡��: ������룬RETURN_OKΪ��ȷ
**         
** ȫ�ֱ���: ��
** ����ģ��: GetDataTime
**
*****************************************************************************/
Uint8 FS_GetDateTime(SYS_TIME *CurTime)
{
   Uint8       temp8;
   Uint16      temp16 = 0;
   DATE_TIME   Time;
   
//   temp8 = GetDateTime(&Time);            // ��ȡ��ǰʱ��


   /* ʱ���ʽת�� */
   Time.da_year = 2012;
   Time.da_mon =     2;
   Time.da_day =    29;
    Time.ti_hour  = 0;
    Time.ti_min   = 0;
    Time.ti_sec   = 0;
    Time.ti_hund  = 0;
   if(Time.da_year > 1980)
   {
      temp16  = Time.da_year - 1980;      // �꣬�����ǰ��С��1980�꣬��Ϊ0
   }
   temp16  = (temp16 & 0x7F) << 9;        // �꣺9��15λ
   temp16 |= (Time.da_mon & 0x0F) << 5;   // �£�5��8λ
   temp16 |= Time.da_day & 0x1F ;         // �գ�0��4λ
   CurTime->date = temp16;
	
   temp16  = (Time.ti_hour & 0x1F) << 11; // ʱ��11��15λ
   temp16 |= (Time.ti_min  & 0x3F) << 5;  // �֣�5��10λ
   temp16 |= (Time.ti_sec  & 0x3F) >> 1;  // ��,��Ϊ2��ı�����0��4λ
   CurTime->time = temp16; 
   
   if((Time.ti_sec & 0x01) == 0x01)
   {
      temp8   = 100;                   // �����ǰ��Ϊ���������������100
   } 
   temp8   += Time.ti_hund;            // ���ϰٷ�֮һ����
   CurTime->msec = temp8;
   
   return RETURN_OK;
}

/******************************************************************************
** ��������: StrToFDTName
** ��������: �û��ļ�\Ŀ¼��ת��Ϊϵͳ��
**
** �䡡��: Str:�û�����
**        
** �䡡��: ��������
**
** ȫ�ֱ���: ��
** ����ģ��: ��
**
*****************************************************************************/
Uint8 StrToFDTName(Int8 *FDTName, Int8 *Str)
{
    Uint8 i, temp;
    
    /* �ļ�\Ŀ¼���Ƿ���Ч */
    if (Str[0] == ' ' || Str[0] == 0)
        return FILE_NAME_ERR; 

    for (i = 0; i < 11; i++)
        FDTName[i] = ' ';

    /* �Ƿ�Ϊ"." */
    if (Str[0] == '.')
    if (Str[1] == 0 || Str[1] == '\\')
    {
        FDTName[0] = '.';
        return RETURN_OK;
    }

    /* �Ƿ�Ϊ".." */
    if (Str[0] == '.')
    if (Str[1] == '.' )
    if (Str[2] == 0 || Str[2] == '\\')
    {
        FDTName[0] = '.';
        FDTName[1] = '.';
        return RETURN_OK;
    }

    /* ��ȡ���ļ�/Ŀ¼�� */
    for (i = 0; i < 8; i++)
    {
        temp = *Str;
        if ((temp == 0) || (temp == '\\'))
            break;
        Str++;
        if (temp == '.')
            break;
        FDTName[i] = temp;
    }

    if (*Str == '.')
        Str++;

    /* ��ȡ�ļ�\Ŀ¼��չ�� */
    for (i = 8; i < 11; i++)
    {
        temp = *Str++;
        if (temp == 0 || temp == '\\')
            break;
        FDTName[i] = temp;
    }
    return RETURN_OK;
}

/******************************************************************************
** ��������: _GetFDTInfo
** ��������: ��ȡFDT���ڵ�Ŀ¼�Ŀ�ʼ�غż�ϵͳ�����ƣ��ڲ�ʹ��
**
** �䡡��: DirFileName:�û�ʹ�õ�FDT��������·����
**         FDTName:���ڷ���ϵͳʹ�õ�FDT����������·����
** �䡡��: FDT���ڵ�Ŀ¼�Ŀ�ʼ�غţ�BAD_CLUS��ʾ�Ҳ���·��
**         
** ȫ�ֱ���: ��
** ����ģ��: GetDirClusIndex,StrToFDTName
**
*****************************************************************************/
Uint32 _GetFDTInfo(Int8 *FDTName, Int8 *DirFileName)
{
    Uint32 Rt;
    Int8 *temp;
    Int8 ch;
    
    /* ��ȡ�ַ�������λ�� */
    temp = DirFileName;
    while (*temp++ != 0)	;
    temp--;
    if (*(--temp) == '\\')  /* ����ַ�Ϊ\������Ч�ļ�/Ŀ¼�� */
    {
        return BAD_CLUS;
    }
    
    /* ��ȡĿ¼��ʼ�غ� */
    Rt = BAD_CLUS;
    while (1)
    {
        if (*temp == '\\' || *temp == ':')
        {
            /* �ҵ�Ŀ¼�ָ����'\' �� */
            /* �ҵ��߼��̷ָ����':'��������ָ���߼��̵�ǰĿ¼ */
            ch = *(++temp);
            *temp = 0;
            Rt = GetDirClusIndex(DirFileName);
            *temp = ch;
            break;
        }
        if (temp == DirFileName)
        {
            /* ֻ���ļ�\Ŀ¼���������ǵ�ǰ�߼��̵�ǰĿ¼ */
            //Rt = GetDirClusIndex(".");
            Rt = DiskInfo.PathClusIndex;
            break;
        }
        temp--;
    }
    /* ��ȡϵͳ���ļ�\Ŀ¼�� */
    if (StrToFDTName(FDTName, temp) != RETURN_OK)
    {
        Rt = BAD_CLUS;
    }
    return Rt;
}

/******************************************************************************
** ��������: GetDirClusIndex
** ��������: ��ȡָ��Ŀ¼��ʼ�غ�
**
** �䡡��: Path:·����
**        
** �䡡��: ��ʼ�غţ�EMPTY_CLUS��Ϊ��Ŀ¼
**
** ȫ�ֱ���: ��
** ����ģ��: FindFDTInfo
**
*****************************************************************************/
Uint32 GetDirClusIndex(Int8 *Path)
{
    Int8 DirName[12];
    Uint32 Rt;
    FDT temp;
    
    Rt = BAD_CLUS;
    if (Path != NULL)
    {
        Path = FsStrCopy(Path);
        if (Path[1] == ':')
            Path += 2;

        Rt = 0;
        if (DiskInfo.FATType == FAT32)         /* FAT32 ��Ŀ¼ */
            Rt = DiskInfo.RootDirTable;

        if (Path[0] != '\\')       /* ����Ŀ¼�ָ����ţ���������ǵ�ǰ·�� */
            Rt = DiskInfo.PathClusIndex;
        else
            Path++;

        if (Path[0] == '.')        /* '\.'��������ǵ�ǰ·�� */
        {
            Rt = DiskInfo.PathClusIndex;
            if (Path[1] == 0 || Path[1] == '\\')
            {
                Path++;
            }
        }
        if (Path[0] == '\\')
        {
            Path++;
        }
        
        DirName[11] = 0;
        while (Path[0] != 0)
        {
            /* ��ȡ��Ŀ¼�� */
            StrToFDTName(DirName , Path);

            /* ��Ŀ¼����ʼ�غ� */
            if (DirName[0] == 0x20)
            {
                Rt = BAD_CLUS;
                break;
            }
            /* ��ȡFDT��Ϣ */
            if (FindFDTInfo(&temp, Rt, DirName) != RETURN_OK)
            {
                Rt = BAD_CLUS;
                break;
            }
            /* FDT�Ƿ���Ŀ¼ */
            if ((temp.Attr & ATTR_DIRECTORY) == 0)
            {
                Rt = BAD_CLUS;
                break;
            }
            Rt = temp.FstClusLO + ((Uint32)temp.FstClusHI << 16);
            /* �ַ�������һ��Ŀ¼ */
            while (*Path != 0)
            {
                if (*Path++ == '\\')
                    break;
            }
        }

        if (DiskInfo.FATType == FAT32)
        if (Rt != BAD_CLUS)
        if (Rt == DiskInfo.RootDirTable)
        {
            Rt = 0;
        }
    }
    return Rt;
}

/******************************************************************************
** ��������: MakeDir
** ��������: ����Ŀ¼
**
** �䡡��: Path:·����
**
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: ClearClus,AddFDT
**
** ˵  ��: �����ļ��н���ʱ��"����ʱ��"����
*****************************************************************************/
Uint8 MakeDir(Int8 *Path)
{
    Uint32 ClusIndex, Temp1;
    Uint8 Rt;
    FDT temp;
    SYS_TIME CurTime;
    
    Path = FsStrCopy(Path);

    ClusIndex = _GetFDTInfo(temp.Name, Path);
    if (ClusIndex == BAD_CLUS)
    {
        return PATH_NOT_FIND;
    }

    /* FDT�Ƿ���� */
    Rt = FDTIsLie(ClusIndex, temp.Name);	// �鿴Ҫ������Ŀ¼�Ƿ��Ѿ�����
    if (Rt != NOT_FIND_FDT)
    {
        return Rt;								// �������ͬ��Ŀ¼������󷵻�
    }
    
    /* ������ */
    Temp1 = FATAddClus(0);                      /* ��ȡĿ¼������̿ռ� */
    if ((Temp1 <= EMPTY_CLUS_1) || (Temp1 >= BAD_CLUS))
    {
        /* û�п��пռ� */
        return  DISK_FULL;
    }

    ClearClus(Temp1);                    /* ��մ� */
    
    if(FS_GetDateTime(&CurTime) != RETURN_OK)   // ��ȡ��ǰʱ��
    {
        return GET_TIME_ERR;    
    }    
        
        /* ����FDT���� */
    temp.Attr = ATTR_DIRECTORY;      		    // FDT����Ϊ�ļ���       
    temp.FileSize = 0;						    // ��СΪ0

    temp.NTRes = 0;

    /*-------�ڴ˼��봴��ʱ��ĳ���---------*/
    temp.CrtTimeTenth = CurTime.msec;
    temp.CrtTime = CurTime.time;
    temp.CrtDate = CurTime.date;
    
    temp.LstAccDate = CurTime.date;
    temp.WrtTime = CurTime.time + 1;            // �޸�ʱ����Ҫ��2�룬��Ϊ�����ܼ��㵽�����
    temp.WrtDate = CurTime.date;
	/*--------------------------------------*/
  
    temp.FstClusLO = Temp1 & 0xffff;            // ������һ�صĵ�16λ
    temp.FstClusHI = Temp1 >> 16;               // ��16λ

    Rt = AddFDT(ClusIndex, &temp);       /* ����Ŀ¼�� */
    if (Rt == RETURN_OK)
    {
        /* ����'.'Ŀ¼ */
        temp.Name[0] = '.';
        temp.Name[1] = 0x20;
        temp.Name[2] = 0x20;
        temp.Name[3] = 0x20;
        temp.Name[4] = 0x20;
        temp.Name[5] = 0x20;
        temp.Name[6] = 0x20;
        temp.Name[7] = 0x20;
        temp.Name[8] = 0x20;
        temp.Name[9] = 0x20;
        temp.Name[10] = 0x20;
        
        AddFDT(Temp1, &temp);		// �����ļ���Ϊ'.'��Ŀ¼�ָ��ǰĿ¼

        /* ����'..'Ŀ¼ */
        temp.Name[1] = '.';					// �����ļ���Ϊ'..'��Ŀ¼�ָ����һ��Ŀ¼
        
        temp.FstClusLO = ClusIndex & 0xffff;
        temp.FstClusHI = ClusIndex >> 16;
        Rt = AddFDT(Temp1, &temp);
    }
    else
    {
        FATDelClusChain(Temp1);		// ���Ŀ¼����ʧ�ܣ��ͷŸջ�õĴ���
    }
    
    return Rt;
}

/*********************************************************************************************************
** ��������: RemoveDir
** ��������: ɾ��Ŀ¼
**
** �䡡��: Path:·����
**
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: DelFDT
**
********************************************************************************************************/
Uint8 RemoveDir(Int8 *Path)
{
    Uint32 ClusIndex, ClusIndex1;
    Uint8 Rt;
    Int8 DirName[12];
    FDT temp;
    
    Path = FsStrCopy(Path);

    ClusIndex = _GetFDTInfo(DirName, Path);
    if (ClusIndex == BAD_CLUS)
    {
        return PATH_NOT_FIND;
    }

    /* ��ȡFDT����Ϣ */
    Rt = FindFDTInfo(&temp, ClusIndex, DirName);
    if (Rt == RETURN_OK)
    {
        /* �Ƿ���Ŀ¼ */
        if ((temp.Attr & ATTR_DIRECTORY) != 0)
        {
            /* �� */
            ClusIndex1 = temp.FstClusLO + ((Uint32)temp.FstClusHI << 16);
            /* �Ƿ��ǿ�Ŀ¼ */
            Rt = DirIsEmpty(ClusIndex1);
            if (Rt == DIR_EMPTY)
            {
                /* �ǣ�ɾ�� */
                FATDelClusChain(ClusIndex1);
                Rt = DelFDT(ClusIndex, DirName);
            }
        }
        else
        {
            return PATH_NOT_FIND;
        }
    }
    return Rt;
}

/*********************************************************************************************************
** ��������: ChangeDir
** ��������: �ı䵱ǰĿ¼
**
** �䡡��: Path:·����
**
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: GetDirClusIndex
**
********************************************************************************************************/
Uint8 ChangeDir(Int8 *Path)
{
    Uint32 ClusIndex;
    Uint8 Rt;

    Rt = PATH_NOT_FIND;
    ClusIndex = GetDirClusIndex(Path);
    if (ClusIndex != BAD_CLUS)
    {
        DiskInfo.PathClusIndex = ClusIndex;
        Rt = RETURN_OK;
    }
    return Rt;
}

