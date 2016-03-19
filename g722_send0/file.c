/***************************************************************************
* Filename: file.c
**************************************************************************/

#define IN_FILE

#include "tistdtypes.h"
#include "fat.h"

MY_FILE FileInfo[MAX_OPEN_FILES];        /* ���ļ���Ϣ�� */
extern Disk_Info DiskInfo;

Uint32 _GetFDTInfo(Int8 *FDTName, Int8 *DirFileName);
Int8 *FsStrCopy(Int8 *source);
Uint8 FS_GetDateTime(SYS_TIME *CurTime);

/*********************************************************************************************************
** ��������: FileInit
** ��������: ��ʼ���ļ�ָ��ϵͳ
**
** �䡡��: ��
**
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
********************************************************************************************************/
void FileInit(void)
{
    HANDLE i;
    
    for (i = 0; i < MAX_OPEN_FILES; i++)
    {
        FileInfo[i].Flags = 0;
    }
}

/*********************************************************************************************************
** ��������: FindOpenFile
** ��������: �����Ѵ򿪵�ָ���ļ����ļ�������ڲ�ʹ��
**
** �䡡��:DirClus:�ļ�����Ŀ¼�Ŀ�ʼ�غ�
**        FileName:�ڲ��ļ���
** �䡡��: �ļ������Not_Open_FILEΪû�д�
**         
** ȫ�ֱ���: FileInfo
** ����ģ��:
**
********************************************************************************************************/
HANDLE FindOpenFile(Uint32 DirClus, Int8 *FileName, Uint16 type)
{
    HANDLE Rt;
    MY_FILE *fp;
    
    fp = FileInfo;
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (fp->Flags & type)
        if (fp->DirClus == DirClus)
        if (fp->Name[0] == FileName[0])
        if (fp->Name[1] == FileName[1])
        if (fp->Name[2] == FileName[2])
        if (fp->Name[3] == FileName[3])
        if (fp->Name[4] == FileName[4])
        if (fp->Name[5] == FileName[5])
        if (fp->Name[6] == FileName[6])
        if (fp->Name[7] == FileName[7])
        if (fp->Name[8] == FileName[8])
        if (fp->Name[9] == FileName[9])
        if (fp->Name[10] == FileName[10])
        {
            break;
        }
        fp++;
    }
    if (Rt >= MAX_OPEN_FILES)
    {
        Rt = Not_Open_FILE;
    }
    return Rt;
}

/******************************************************************************
** ��������: RemoveFile
** ��������: ɾ���ļ�
**
** �䡡��: DirFileName:�û�ʹ�õ��ļ���
**
** �䡡��: RETURN_OK���ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵��
** ȫ�ֱ���: ��
** ����ģ��: _GetFDTInfo,FindFDTInfo,FATDelClusChain,DelFDT
**
*****************************************************************************/
Uint8 RemoveFile(Int8 *DirFileName)
{
    Uint32 ClusIndex, ClusIndex1;
    Uint8 Rt;
    Int8 DirName[12];
    FDT temp;

    DirFileName = FsStrCopy(DirFileName);
    ClusIndex = _GetFDTInfo(DirName, DirFileName);
    Rt = PATH_NOT_FIND;
    if (ClusIndex != BAD_CLUS)
    {
        Rt = FindFDTInfo(&temp, ClusIndex, DirName);
        if (Rt == RETURN_OK)
        {
            Rt = NOT_FIND_FILE;
            if ((temp.Attr & ATTR_DIRECTORY) == 0)  /* ���ļ���ɾ�� */
            {
                Rt = FILE_LOCK;
                if (FindOpenFile(ClusIndex, DirName,
						FILE_FLAGS_READ|FILE_FLAGS_WRITE) == Not_Open_FILE)
                {
                    /* �ļ�û�д򿪲�ɾ�� */
                    ClusIndex1 = temp.FstClusLO + ((Uint32)temp.FstClusHI << 16);
                    FATDelClusChain(ClusIndex1);
                    Rt = DelFDT(ClusIndex, DirName);
                }
            }
        }
    }
    return Rt;
}

/******************************************************************************
** ��������: _FileOpenR
** ��������: ֻ����ʽ���ļ����ڲ�ʹ��
**
** �䡡��: DirFileName:�û�ʹ�õ��ļ���
**
** �䡡��: �ļ������Not_Open_FILEΪ���ܴ�
**         
** ȫ�ֱ���: FileInfo
** ����ģ��: _GetFDTInfo,FindFDTInfo
**
*****************************************************************************/
HANDLE _FileOpenR(Int8 *DirFileName)
{
    MY_FILE *fp;
    HANDLE Rt;
    FDT FileFDT;
    
    /* ���ҿ����ļ��Ǽ��� */
    for (Rt = 0; FileInfo[Rt].Flags; Rt++)
    {
		if(Rt >= MAX_OPEN_FILES)
            return Not_Open_FILE;
    }

    fp = FileInfo + Rt;
    
    /* ��ȡĿ¼��ʼ�غź��ļ��� */
    fp->DirClus = _GetFDTInfo(fp->Name, DirFileName);
    if (fp->DirClus < BAD_CLUS)
    {
        /* ��ȡ�ļ���Ϣ */
        if (FindFDTInfo(&FileFDT, fp->DirClus, fp->Name) == RETURN_OK)
        {
            if ((FileFDT.Attr & ATTR_DIRECTORY) == 0)
            {
                fp->Flags = FILE_FLAGS_READ;
                fp->FileSize = FileFDT.FileSize;
                fp->FstClus = FileFDT.FstClusLO | (Uint32)FileFDT.FstClusHI << 16;
                fp->Clus = fp->FstClus;
                fp->Offset = 0;
                return Rt;
            }
        }
    }
    return Not_Open_FILE;
}

/******************************************************************************
** ��������: _FileOpenW
** ��������: ֻд��ʽ���ļ����ڲ�ʹ��
**
** �䡡��: DirFileName:�û�ʹ�õ��ļ���
**
** �䡡��: �ļ������Not_Open_FILEΪ���ܴ�
**         
** ȫ�ֱ���: FileInfo
** ����ģ��: _GetFDTInfo,FindFDTInfo
**
*****************************************************************************/
HANDLE _FileOpenW(Int8 *DirFileName)
{
    MY_FILE *fp;
    FDT temp;
    HANDLE Rt;
    Uint8 i;
    SYS_TIME CurTime,CrtTime;	// ���浱ǰʱ������

    /* ���ҿ����ļ��Ǽ��� */
    for (Rt = 0; FileInfo[Rt].Flags; Rt++)
    {
		if(Rt >= MAX_OPEN_FILES)
            return Not_Open_FILE;
    }
    
    fp = FileInfo + Rt;					// ָ���õĿ��еǼ���
    
    /* ��ȡĿ¼��ʼ�غź��ļ��� */
    fp->DirClus = _GetFDTInfo(fp->Name, DirFileName);	// ��ȡ���ļ���FDT����״غţ����ļ���
    if (fp->DirClus < BAD_CLUS)
    {
        /* �ļ��Ѿ��Զ�д��ʽ�򿪣������ٴ��Զ�д��ʽ�� */
        if (FindOpenFile(fp->DirClus, fp->Name,
					FILE_FLAGS_WRITE) == Not_Open_FILE)
        {
            if(FS_GetDateTime(&CurTime) != RETURN_OK) // ��ȡ��ǰϵͳʱ��
            {
            	return GET_TIME_ERR;
            }
            
            if (FindFDTInfo(&temp, fp->DirClus, fp->Name) == RETURN_OK)
            {
                if ((temp.Attr & ATTR_DIRECTORY) != 0)
                {
                   return Not_Open_FILE;		// ������ļ�ΪĿ¼������ɾ��
                }
                
                CrtTime.date = temp.CrtDate;    // �����ļ�����ʱ��
                CrtTime.time = temp.CrtTime;
                CrtTime.msec = temp.CrtTimeTenth;
                
                if (RemoveFile(DirFileName) != RETURN_OK)   /* ɾ���ļ� */
                {
                   return Not_Open_FILE;
                }
            }
            else
            {
                CrtTime = CurTime;              // �����ļ�����ʱ��Ϊ��ǰʱ��
            }                
            
            /* �����ļ� */
            for (i = 0; i < 11; i++)			// ������Ҫ�����ļ���FDT��
            {
                temp.Name[i] = fp->Name[i];		// ��������
            }
            temp.Attr = 0;						// ����Ϊ0
            temp.FileSize = 0;

            temp.NTRes = 0;
            
            /*-------�ڴ˼��봴��ʱ��ĳ���---------*/
            temp.CrtTimeTenth = CrtTime.msec;	
            temp.CrtTime = CrtTime.time;
            temp.CrtDate = CrtTime.date;
            
            temp.LstAccDate = CurTime.date;
            temp.WrtTime = CurTime.time + 1;
            temp.WrtDate = CurTime.date;
			/*--------------------------------------*/
			
            temp.FstClusLO = 0;
            temp.FstClusHI = 0;
            if (AddFDT(fp->DirClus, &temp) == RETURN_OK)       /* �����ļ� */
            {
                /* �����ļ���Ϣ */
                fp->Flags = FILE_FLAGS_WRITE;
                fp->FileSize = 0;
                fp->FstClus = 0;
                fp->Clus = 0;
                fp->Offset = 0;
                return Rt;
            } // if(Add..)
        } // if(Find..)
    } // if(fp..)
    return Not_Open_FILE;
}

/*********************************************************************************************************
** ��������: _FileOpenRW
** ��������: ֻ��д��ʽ���ļ����ڲ�ʹ��
**
** �䡡��: DirFileName:�û�ʹ�õ��ļ���
**
** �䡡��: �ļ������Not_Open_FILEΪ���ܴ�
**         
** ȫ�ֱ���: ��
** ����ģ��: _FileOpenR,_FileOpenW
**
********************************************************************************************************/
HANDLE _FileOpenRW(char *DirFileName)
{
    HANDLE Rt;
    
    Rt = _FileOpenR(DirFileName);
    if (Rt == Not_Open_FILE)
    {
        Rt = _FileOpenW(DirFileName);
    }
    else
    {
        if (FindOpenFile(FileInfo[Rt].DirClus,
					FileInfo[Rt].Name, FILE_FLAGS_WRITE) == Not_Open_FILE)
        {
            FileInfo[Rt].Flags = (FILE_FLAGS_WRITE | FILE_FLAGS_READ);
        }
        else
        {
            FileInfo[Rt].Flags = 0;
            Rt = Not_Open_FILE;
        }
    }
    return Rt;
}

/******************************************************************************
** ��������: FileOpen
** ��������: ��ָ����ʽ���ļ�
**
** �䡡��: DirFileName:�û�ʹ�õ��ļ���
**        Type:�򿪷�ʽ
** �䡡��: �ļ������Not_Open_FILEΪ���ܴ�
**         
** ȫ�ֱ���: ��
** ����ģ��: _FileOpenR,_FileOpenW,_FileOpenRW
**
*****************************************************************************/
HANDLE FileOpen(Int8 *DirFileName, Uint16 type)
{
	DirFileName = FsStrCopy(DirFileName);

	if (type == (FILE_FLAGS_READ|FILE_FLAGS_WRITE))
        return _FileOpenRW(DirFileName);
	else if (type == FILE_FLAGS_WRITE)
        return _FileOpenW(DirFileName);
	else if(type == FILE_FLAGS_READ)
        return _FileOpenR(DirFileName);
	else
		return Not_Open_FILE;
}

/******************************************************************************
** ��������: FileClose
** ��������: �ر�ָ���ļ�
**
** �䡡��: Handle:�ļ����
**
** �䡡��: RETURN_OK:�ɹ�
**        �����ο�fat.h�й��ڷ���ֵ��˵�� 
** ȫ�ֱ���: ��
** ����ģ��: ��
**
** ˵  ��: �����ļ���������ʱ�����Ժ�����޸�ʱ������
**
** ��  ��: �ж��ļ��Ƿ���д״̬-Y->ȡ�����ļ�FDT��-->�ж��ļ���С�Ƿ�仯-Y->����FDT���״غ�-->����FDT��
** ע  ��: �ر��ļ��󣬽��ͷ���Ǽ�������޸Ĺ���������д���棬������ռ��ԭ����cache��
****************************************************************************/
Uint8 FileClose(HANDLE Handle)
{
    Uint8 Rt;
    FDT FileFDT;
    MY_FILE *fp;
    SYS_TIME CurTime;
    
    Rt = PARAMETER_ERR;
    if (Handle < MAX_OPEN_FILES)
    {
        if(FS_GetDateTime(&CurTime) != RETURN_OK)       // ��ȡ��ǰʱ��
        {
            return GET_TIME_ERR;
        }
                                                        
        fp = FileInfo + Handle;		                    // ��ȡ���ļ���FDT��
        Rt = FindFDTInfo(&FileFDT, fp->DirClus, fp->Name);
        if (Rt != RETURN_OK)
        {
            return Rt;
        }
        
        FileFDT.LstAccDate = CurTime.date;              // �����ļ�������ʱ��
                                                        // ����ļ�������ֻд��ʽ�򿪾Ͳ���Ҫ����
        if (fp->Flags & FILE_FLAGS_WRITE)
        {
            if (FileFDT.FileSize < fp->FileSize)		// �鿴�ļ���С�Ƿ����仯
            {
                FileFDT.FileSize = fp->FileSize;	
                                                        // �����ļ�����޸�ʱ��
                FileFDT.WrtTime = CurTime.time + 1;     // �޸�ʱ����Ҫ��2�룬��Ϊ�����ܼ��㵽�����
                FileFDT.WrtDate = CurTime.date;
                
                if (FileFDT.FstClusLO == 0)
                if (FileFDT.FstClusHI == 0)
                {
                    FileFDT.FstClusLO = fp->FstClus & 0xffff;
                    FileFDT.FstClusHI = fp->FstClus >> 16;
                }
            }
        }
        ChangeFDT(fp->DirClus, &FileFDT);	// ���޸ĺ��FDT���
        fp->Flags = 0;
    }
    return Rt;
}


/******************************************************************************
** ��������: FileRead
** ��������: ��ȡ�ļ�
**
** �䡡��: Buf:������ص�����
**        Size:Ҫ�����ֽ���
**        Handle:�ļ����
** �䡡��: ʵ�ʶ������ֽ���
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
*****************************************************************************/
Uint32 FileRead(void *Buf, Uint32 Size, HANDLE Handle)
{
    Uint32 i, j, k, SecIndex;
    MY_FILE *fp;
    Uint32 Rt;
    Uint8 *Buf1, *Buf2;
	Int16 ptr;
    
    Rt = 0;
    fp = FileInfo + Handle;
    Buf2 = (Uint8 *)Buf;
    if (Handle < MAX_OPEN_FILES)     /* Handle�Ƿ���Ч */
    if (fp->Flags)                   /* ��Ӧ�Ĵ��ļ���Ϣ���Ƿ���ʹ�� */
    {
        if (fp->Offset + Size >= fp->FileSize)
        {
            /* ��������ݳ����ļ��ĳ��ȣ���һ���Ƿ��б�ĳ�������д����ļ� */
            /* ����У�������ļ��ĳ����б仯������֮*/
            Handle = FindOpenFile(fp->DirClus, fp->Name, FILE_FLAGS_WRITE);
            if (Handle < MAX_OPEN_FILES)
            {
                fp->FileSize = FileInfo[Handle].FileSize;
                if (fp->Offset == 0)
                {
                    fp->FstClus = FileInfo[Handle].FstClus;
                    fp->Clus = fp->FstClus;
                }
            }
        }

        while (Size != 0)
        {
            if (fp->Offset >= fp->FileSize)              /* �Ƿ��ļ����� */
                break;

            /* ���������������� */
            j = fp->Offset % (DiskInfo.SecPerClus * DiskInfo.BytsPerSec);
            i = j / DiskInfo.BytsPerSec;
            j = j % DiskInfo.BytsPerSec;
            SecIndex = (fp->Clus - 2) * DiskInfo.SecPerClus + 
                       DiskInfo.DataStartSec + i;
            /* ������ */
            Buf1 = OpenSec(SecIndex);
            if (Buf1 == NULL)
                break;

            /* ��ȡ�������� */
            if (ReadSec(SecIndex, CACHE_DATA) != RETURN_OK)
                break;

            /* ��ȡ���� */
            k = DiskInfo.BytsPerSec - j;
            if (Size < k)
                k = Size;

            if ((fp->Offset + k) > fp->FileSize)
                k = fp->FileSize - fp->Offset;

            Size -= k;
            Rt += k;
            fp->Offset += k;

			for(ptr = 0; ptr < (k); ptr++)
				*Buf2++ = Buf1[j + ptr];

            /* �ر����� */
            j += k;    
            if (j >= DiskInfo.BytsPerSec)
            {
                i++;
                if (i >= DiskInfo.SecPerClus)
                    fp->Clus = FATGetNextClus(fp->Clus);
            }
        }
    }
    return Rt;
}


/*********************************************************************************************************
** ��������: FileWrite
** ��������: д�ļ�
**
** �䡡��: Buf:Ҫд������
**        Size:Ҫд���ֽ���
**        Handle:�ļ����
** �䡡��: ʵ��д���ֽ���
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
********************************************************************************************************/
Uint32 FileWrite(void *Buf, Uint32 Size, HANDLE Handle)
{
    Uint32 i, j, k, SecIndex;
    MY_FILE *fp;
    Uint32 Rt;
    Uint8 *Buf1, *Buf2;
	Int16 ptr;

    Rt = 0;
    fp = FileInfo + Handle;             // ��õǼ�����
    Buf2 = (Uint8 *)Buf;                // ����Դ
    if (Handle < MAX_OPEN_FILES)        /* Handle�Ƿ���Ч */
    if (fp->Flags & FILE_FLAGS_WRITE)   /* �ļ��Ƿ��д */
    {
        if (fp->Offset > fp->FileSize)  // ƫ�Ƴ����ļ���Χ������
        {
            return 0;
        }

        while (Size != 0)
        {
            /* ���������������� */
            j = fp->Offset % (DiskInfo.SecPerClus * DiskInfo.BytsPerSec);   // �ֽ�ƫ����(����)
            
            if (j == 0)
            if (fp->Offset == fp->FileSize)  
            {           // ���ָ��ص����һ�ֽڣ�����ƫ���������ļ���С����ָ���ļ����һ���ֽ�
                i = FATAddClus(fp->Clus);  // ����Ҫ�����´�
                if (i >= BAD_CLUS)
                {
                    return DISK_FULL;
                }
                fp->Clus = i;
                if (fp->FstClus == EMPTY_CLUS)
                {
                    fp->FstClus = i;
                }
            }

            i = j / DiskInfo.BytsPerSec;   // ����ƫ��(����)
            j = j % DiskInfo.BytsPerSec;   // �ֽ�ƫ��(������)
            SecIndex = (fp->Clus - 2) * DiskInfo.SecPerClus + 
                       DiskInfo.DataStartSec + i;

            k = DiskInfo.BytsPerSec - j;   // ����ʣ���ֽ�
            if (Size < k)                   // ����������Ŀ����ֽڴ���Ҫ������ļ���С
            {
                k = Size;
            }

            /* ������ */
            Buf1 = OpenSec(SecIndex);
            if (Buf1 == NULL)
            {
                break;
            }
            /* ��ȡ�������� */
            if (fp->Flags & FILE_FLAGS_READ)
            if (ReadSec(SecIndex, CACHE_DATA) != RETURN_OK)
            {
                break;
            }

            // ���ݸ��ƣ�������ԴBuf2����k�ֽ����ݵ�Buf1+j
            // ����Դָ���k
			for(ptr = 0; ptr < (k); ptr++)
				Buf1[j + ptr] = *Buf2++;

            /* �ر����� */
            WriteSec(SecIndex, CACHE_DATA);  // ���ñ��޸ı��

            Size -= k;                      // ʣ���д����������k
            Rt += k;
            /* �����ļ�ָ�� */
            fp->Offset += k;                // д���ָ��ƫ��λ��+k
            if (fp->Offset > fp->FileSize)  // ����ļ����󣬸����ļ���Сֵ
            {
                fp->FileSize = fp->Offset;
            }

            if ((j + k) >= DiskInfo.BytsPerSec)
            {
                if ((i + 1) >= DiskInfo.SecPerClus)
                if (fp->Offset < fp->FileSize)
                {
                    fp->Clus = FATGetNextClus(fp->Clus);
                }
            }
        }
    }
    return Rt;
}

/*********************************************************************************************************
** ��������: FileCloseAll
** ��������: �ر����д򿪵��ļ�
**
** �䡡��: ��
**
** �䡡��: ��
**         
** ȫ�ֱ���: FileInfo
** ����ģ��: AllCacheWriteBack
**
********************************************************************************************************/
void FileCloseAll(void)
{
    Uint32 i;

    for (i = 0; i < MAX_OPEN_FILES; i++)
    {
        FileClose(i);
    }
    AllCacheWriteBack();
}


/******************************************************************************
** ��������: FileSeek
** ��������: �ƶ��ļ���\дλ��
**
** �䡡��: Handle:�ļ����
**        offset:�ƶ�ƫ����
**        Whence:�ƶ�ģʽSEEK_SET:���ļ�ͷ����SEEK_CUR:�ӵ�ǰλ�ü���SEEK_END:���ļ�β����
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
*****************************************************************************/
Uint8 FileSeek(HANDLE Handle, Int32 offset, Uint8 Whence)
{
    Uint8 Rt;
    Uint32 i, Clus;
    MY_FILE *fp;
    
    Rt = PARAMETER_ERR;
    fp = FileInfo + Handle;
    if (Handle < MAX_OPEN_FILES)                    /* Handle�Ƿ���Ч */
    if (fp->Flags)                               /* ��Ӧ�Ĵ��ļ���Ϣ���Ƿ���ʹ�� */
    {
        Rt = RETURN_OK;
        switch (Whence)
        {
            case SEEK_END:             /* ���ļ�β���� */
                fp->Offset = fp->FileSize - offset;
                offset = -offset;
                break;
            case SEEK_SET:
                fp->Offset = offset;
                break;
            case SEEK_CUR:             /* �ӵ�ǰλ�ü��� ���ﲻ��break����ȷ��*/
                i = fp->Offset + offset;
                break;
            default:
                Rt = PARAMETER_ERR;
                break;
        }
        /* �ı䵱ǰ�غ� */
        if (Rt == RETURN_OK)
        {
            if (fp->Offset > fp->FileSize)
            {
                if (offset > 0)
                    fp->Offset = fp->FileSize;
                else
                    fp->Offset = 0;
            }
            Rt = RETURN_OK;
            i = fp->Offset / (DiskInfo.BytsPerSec * DiskInfo.SecPerClus);
            Clus = fp->FstClus;
            for (; i != 0; i--)
            {
                Clus = FATGetNextClus(Clus);
                if (Clus >= BAD_CLUS)
                {
                    Rt = FAT_ERR;
                    break;
                }
            }
            fp->Clus = Clus;
        }
    }
    return Rt;
}

