#include "ezdsp5535_sdcard.h"

#include "fat.h"

int SD_ReadBlock(Uint32 sect, Uint8 *p);
int SD_WriteBlock(Uint32 sect, Uint8 *p);
int SD_ReadBlock_Raw(Uint32 sect, Uint8 *p);
int SD_WriteBlock_Raw(Uint32 sect, Uint8 *p);

/**--------------File Info-------------------------------------------------------------------------------
** File name:			sd.c

** ��������: Empty      ---> ʹ�����Լ��ĺ�������
** ��������: �ײ������������ϲ�Ľӿڳ���
**
** �䡡��: Cammand��define DISK_INIT:                 ��ʼ��������ʵ��
**                  define DISK_CLOSE:                �رգ�����ʵ��
**                  define DISK_READ_SECTOR:          ������������ʵ��
**                  define DISK_WRITE_SECTOR:         д����������ʵ��
**                  define DISK_DRIVER_VER:           �鿴��������ӿڰ汾�ţ�����ʵ��
**                  define DISK_CHECK_CMD:            �鿴�����Ƿ�ʵ�֣�����ʵ��
**                  define DISK_LOW_FORMAT:           �ͼ���ʽ������ѡ����
**                  define DISK_FREE_SECTOR:          �ͷ���������ѡ����
**                  define DISK_GET_SECTOR_NUMBER:    ����豸������������ѡ����
**                  define DISK_GET_BYTES_PER_SECTOR: ���ÿ�����ֽ�������ѡ����
**                  define DISK_CHECK_CHANGE:         �鿴�����Ƿ�ı䣬��ѡ����
**                  define DISK_GET_SECTORS_PER_BLOCK:��ȡÿ������������ѡ����
**                  define DISK_READ_BLOCK:           �����ݿ飬��ѡ����
**                  define DISK_WRITE_BLOCK:          д���ݿ飬��ѡ����
**        Parameter: ʣ�����������Parameter->RsvdForLow���ڴ洢��������֧�ֵ������豸�����Ϣ��
**                             ��ʹ�õ���Դ�ȡ�ʹ��������ʹһ������֧�ֶ�������豸��                 
** �䡡��: DISK_READ_OK:      ���������
**        DISK_READ_NOT_OK:   ������ʧ��
**        DISK_WRITE_OK:      д�������
**        DISK_WRITE_NOT_OK:  д����ʧ��
**        DISK_INIT_OK:       ��ʼ�����
**        DISK_INIT_NOT_OK:   ��ʼ��ʧ��
**        DISK_FALSE:         ��
**        DISK_TRUE:          ��
**        BAD_DISK_COMMAND:   ��Ч������
** ȫ�ֱ���: ��
** ����ģ��: ��
**
********************************************************************************************************/
Uint16 SDCammand(Uint8 Cammand, void *Parameter) /*ʹ�����Լ��ĺ������� */
{
    Uint16 rt;
    Disk_RW_Parameter * Dp;
    //FFSDisk *Disk;
    
    Dp = (Disk_RW_Parameter *)Parameter;
    //Disk = (FFSDisk *) Dp->RsvdForLow;
    
    /* ��ѡ�������û��ʵ�֣���rt =  BAD_DISK_COMMAND*/
    switch (Cammand)
    {
        case DISK_INIT:
			/* Initialize SD card interface */
			EZDSP5535_SDCARD_init();
           	rt = DISK_INIT_OK;
            break;
            
        case DISK_CLOSE:
            /* �ر��������򣬱���ʵ�� */
            /* Parameterû��ʹ�� */
		    EZDSP5535_SDCARD_close();
			rt = DISK_RETURN_OK;
            //rt = BAD_DISK_COMMAND;
            break;
            
        case DISK_READ_SECTOR:
        	SD_ReadBlock(Dp->SectorIndex, Dp->Buf);
        	rt = DISK_READ_OK;
            break;
            
        case DISK_WRITE_SECTOR:
        	SD_WriteBlock(Dp->SectorIndex, Dp->Buf);
    		rt = DISK_WRITE_OK;
            break;

        case DISK_READ_RAW:
        	SD_ReadBlock_Raw(Dp->SectorIndex, Dp->Buf);
        	rt = DISK_READ_OK;
            break;
            
        case DISK_WRITE_RAW:
        	SD_WriteBlock_Raw(Dp->SectorIndex, Dp->Buf);
    		rt = DISK_WRITE_OK;
            break;

        case DISK_CHECK_CMD:
            rt = DISK_FALSE;
            /* �����Ǹ�����û��ʵ�־�ע�͵���Ӧ����� */
            if ((Dp->SectorIndex == DISK_INIT)
               || (Dp->SectorIndex == DISK_CLOSE)
               || (Dp->SectorIndex == DISK_READ_SECTOR)
               || (Dp->SectorIndex == DISK_WRITE_SECTOR)
               || (Dp->SectorIndex == DISK_DRIVER_VER)
               || (Dp->SectorIndex == DISK_CHECK_CMD)
             //  || (Dp->SectorIndex == DISK_LOW_FORMAT)
             //  || (Dp->SectorIndex == DISK_FREE_SECTOR)
             //  || (Dp->SectorIndex == DISK_GET_SECTOR_NUMBER)
             //  || (Dp->SectorIndex == DISK_GET_BYTES_PER_SECTOR)
             //  || (Dp->SectorIndex == DISK_CHECK_CHANGE)
             //  || (Dp->SectorIndex == DISK_GET_SECTORS_PER_BLOCK)
             //  || (Dp->SectorIndex == DISK_READ_BLOCK)
             //  || (Dp->SectorIndex == DISK_WRITE_BLOCK)
               )
            {
                rt = DISK_TRUE;
            }
            break;
            
        //case DISK_LOW_FORMAT:
            /* �ͼ���ʽ������ѡ���� */
            /* Dp->SectorIndex��0:һ��ͼ���ʽ�� */
            /* Dp->SectorIndex������:��һ�εͼ���ʽ�� */
        //    rt = DISK_RETURN_OK;
        //    break;
            
        //case DISK_FREE_SECTOR:
            /* �ͷ���������ѡ���ZLG/FFSʹ�ô����� */
            /* ����������������֪����Щ�������ٱ����������� */
            /* Dp->SectorIndex�������������� */
            //rt = DISK_RETURN_OK;
        //    break;
            
        case DISK_GET_SECTOR_NUMBER:
            /* ����豸������������ѡ���� */
            //Dp->SectorIndex = ��Ч��������������;
            Dp->SectorIndex = 5124288;
            rt = DISK_RETURN_OK;
            break;
            
        case DISK_GET_BYTES_PER_SECTOR:
            /* ���ÿ�����ֽ�������ѡ���� */
            //Dp->SectorIndex = ÿ�����ֽ���;
            Dp->SectorIndex = 512;
            rt = DISK_RETURN_OK;
            break;
            
        //case DISK_CHECK_CHANGE:
            /* �鿴�����Ƿ�ı䣬��ѡ���� */
            //if (���ʸı�(�������SD��))
            //{
            //    rt = DISK_TRUE;
            //}
            //else
            //{
            //    rt = DISK_FALSE;
            //}
            
        //case DISK_GET_SECTORS_PER_BLOCK:
            /* ��ȡÿ������������ѡ���� */
            //Dp->SectorIndex = ÿ��������;
        //  rt = DISK_RETURN_OK;
        //  break;
            
        //case DISK_READ_BLOCK:
            /* �����ݿ飬��ѡ���� */
            /* Dp->Buf���洢���������� */
            /* Dp->SectorIndex����ĵ�һ�������������� */
            /* rt=DISK_READ_OK��DISK_READ_NOT_OK*/
        //  break;
            
        //case DISK_WRITE_BLOCK:
            /* д���ݿ飬��ѡ���� */
            /* Dp->Buf����Ҫд������ */
            /* Dp->SectorIndex����ĵ�һ�������������� */
            /* rt=DISK_WRITE_OK��DISK_WRITE_NOT_OK*/
        //  break;
            
        default:
            rt = BAD_DISK_COMMAND;
            break;
    }
    return rt;
}

