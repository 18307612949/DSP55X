//////////////////////////////////////////////////////////////////////////////
// * File name: ezdsp5535_uart.c
//////////////////////////////////////////////////////////////////////////////

#include "ezdsp5535_uart.h"
#include "ezdsp5535_lcd.h"
#include "g722record.h"
#include "rtcM41Txx_i2c.h"
#include "ff.h"
#include "fileops.h"

#include "csl_intc.h"
static
void mem_cpy (void* dst, const void* src, int cnt) {
    char *d = (char*)dst;
    const char *s = (const char *)src;
    while (cnt--) *d++ = *s++;
}

static
int mem_cmp (const void* dst, const void* src, int cnt) {
    const char *d = (const char *)dst, *s = (const char *)src;
    int r = 0;
    while (cnt-- && (r = *d++ - *s++) == 0) ;
    return r;
}

Int8 tx_buffer[64];
Int8 rx_buffer[64];
static int rcvCounter = 0;
static int xmtCounter = 0, XmtSize = 0;

extern Uint16 taskList;
extern recordStruct recordFile;
extern Uint16 gDct_length;
extern Uint16 number_of_bytes_per_frame;
extern FATFS *fs;
extern FIL file;

void interrupt uart_isr(void)
{
	Uint8 c;
	Uint16 int_type;

	int_type = CSL_UART_REGS->FCR;		// same address as IIR

	if((int_type & 7) == 2) {			//transmitter holding register empty
		xmtCounter &= 63;
		CSL_UART_REGS->THR = tx_buffer[xmtCounter++];
		if(xmtCounter >= XmtSize) {
			CSL_UART_REGS->IER &= ~0x0002;
		}
	} else if((int_type & 7) == 4) {	// receiver data available
		c = CSL_UART_REGS->THR;
		if(c == '#')	rcvCounter = 0;
		rcvCounter &= 63;
		rx_buffer[rcvCounter++] = c;
		if(c == '!') {
//			CSL_UART_REGS->IER &= ~0x0001;		// RX interrupt disabled
			taskList |= UARTPROCESSING;
		}
	} else if((int_type & 7) == 6) {	// error, read LSR to clear
		c = CSL_UART_REGS->LSR;
	}
}

Int16 ascii2bcd(Int8 *ascii, Int8 *bcd)
{
	Uint8 cl, ch;
	int i;

	for(i = 3; i--; ) {
		ch = (*ascii++) - '0';
		cl = (*ascii++) - '0';
		*bcd-- = (ch << 4) | cl;
		*ascii++;
	}
	return 0;
}

Int16 bcd2ascii(Int8 *bcd, Int8 *ascii)
{
// time format  56 34 12 -> 12:34:56 (HH:MM:SS)
// date format  15 06 13 -> 13-06-15 (YY-MM-DD)
	int i;

	for(i = 3; i--; ) {
		*ascii++ = ((*bcd >> 4) & 0x0f) | '0';
		*ascii++ = ((*bcd--) & 0x0f) | '0';
		*ascii++;
	}

	return 0;
}

/*
    命令：#10-10-19-09:12:07$Time!  //设定时间
    回报：#Y$Time! //时间设置成功
          #N$Time! //时间设置失败

    命令：#?$Time! ,请求返回时间
    回报：#10-09-15-09:12:07R$Time!  //时间正确
          #07-12-29-09:12:07E$Time!  //时间错误,需要重新设置
*/
Int16 timer_settings(void)
{
	Int8 time_string[64];
	Int8 *format_time = "00-00-00-00:00:00";

	if((rx_buffer[1] == '?') && (rcvCounter == 8)) {
		rtc_get_time(time_string, 8);
//	rtc_set_time("\x00\x00\x00\x00\x12\x05\x22\x03\x13", 9);

		bcd2ascii(&time_string[3], format_time + 9);
		bcd2ascii(&time_string[7], format_time);

		XmtSize = 17;
		mem_cpy(tx_buffer, format_time, XmtSize);
	} else if(rcvCounter == 24) {
		ascii2bcd(&rx_buffer[1], time_string + 7);
		ascii2bcd(&rx_buffer[10], time_string + 3);
		rtc_set_time(time_string, 9);

		XmtSize = 8;
		mem_cpy(tx_buffer, "#Y$Time!", XmtSize);
	} else {
		XmtSize = 8;
		mem_cpy(tx_buffer, "#N$Time!", XmtSize);
	}

	return 0;
}

/*
    命令：#xyz$Mode!
           x=l/r/s  左声道/右声道/立体声
            y=h/m/l 音质高/中/低
             z=0/1 加密否/是
    回报：#Y$Mode! //设置成功
          #N$Mode! //设置失败
*/
Int16 set_record_mode(void)
{
	Uint16 encode;

	XmtSize = 8;
	mem_cpy(tx_buffer, "#Y$Mode!", XmtSize);

	switch(rx_buffer[1]) {
	case 'l':
		recordFile.channel = 1;
		break;
	case 'r':
		recordFile.channel = 2;
		break;
	case 's':
		recordFile.channel = 3;
		break;
	default:
		return 'N';
	}

	switch(rx_buffer[2]) {
	case 'h':
		recordFile.code = 0x0132;		// 32kbps
		recordFile.sampleRate = 3;		// 32kHz
		break;
	case 'm':
		recordFile.code = 0x0124;		// 24kbps
		recordFile.sampleRate = 6;		// 16kHz
		break;
	case 'l':
		recordFile.code = 0x0116;		// 16kbps
		recordFile.sampleRate = 6;		// 16kHz
		break;
	default:
		return 'N';
	}
	/* set codec sampling rate(16kHz, 32kHz) */
	asm("   bit(ST1, #ST1_INTM) = #1");
//	DMA_audio_init(DCT_LENGTH);	/* code In/Out as DMA */
	aic3204_set_frequency(recordFile.sampleRate);
	asm("   bit(ST1, #ST1_INTM) = #0");

	encode = rx_buffer[3] & 1;			//code=0(raw), code=1(g722.1)
	recordFile.code *= encode;
	if(encode == 0) {
		recordFile.framesize = gDct_length * 2;
	} else {
		recordFile.framesize = number_of_bytes_per_frame;
	}									// data for each channel in bytes

	return 0;
}

/*
    命令：#xx$MCod!  //xx=机号 0<=xx<=99
    回报：#Y$MCod! //设置成功
          #N$MCod! //设置失败
*/
Int16 set_machine_id(void)
{
	Uint8 h, l;
	Uint16 MachineID = 0;

	h = rx_buffer[1] - '0';
	l = rx_buffer[2] - '0';
	MachineID = h * 10 + l;

	XmtSize = 8;
	mem_cpy(tx_buffer, "#Y$MCod!", XmtSize);

	return 0;
}

/*
    命令：#xx$SSpk!  //xx=speaker 音量 0<=xx<=35, -6dB~+29dB
    回报：#Y$SSpk! //设置成功
          #N$SSpk! //设置失败
*/
Int16 set_spk_level(void)
{
	Uint8 h, l;
	Int8 spkGain;

	XmtSize = 8;
	mem_cpy(tx_buffer, "#Y$SSpk!", XmtSize);	// default set to YES

	h = rx_buffer[1] - '0';
	l = rx_buffer[2] - '0';
	spkGain = h * 10 + l - 6;
	if((spkGain > 35) || (spkGain < -6)) {
		return 'N';
	}

	aic3204_set_spk_gain(spkGain);		// -6~+29dB
	return 0;
}

/*
    命令：#xx$SCod!  //xx=录音灵敏度 0<=xx<=255
    回报：#Y$SCod! //设置成功
          #N$SCod! //设置失败
*/
Int16 set_rec_level(void)
{
	Uint8 h, l;
	Int8 micGain;

	XmtSize = 8;
	mem_cpy(tx_buffer, "#Y$SCod!", XmtSize);	// default set to YES
	h = rx_buffer[1] - '0';
	l = rx_buffer[2] - '0';
	micGain = 2 * (h * 10 + l);
	if((micGain > 95) || (micGain < 0)) {
		return 'N';
	}

	aic3204_set_mic_gain(micGain);		// 0~47.5dB, 0.5dB step
	return 0;
}

/*
    命令：#xx$Splt!  //xx=分段时间(单位:分钟)  5<=xx<=60
    回报：#Y$Splt! //设置成功
          #N$Splt! //设置失败
*/
Int16 set_split(void)
{
	Uint8 h, l;

	h = rx_buffer[1] - '0';
	l = rx_buffer[2] - '0';		// split size in minutes

	/* 20ms/frame */
	recordFile.split = (Int32)(h * 10 + l) * (Int32)3000;
	recordFile.frames = recordFile.split;

	XmtSize = 8;
	mem_cpy(tx_buffer, "#Y$Splt!", XmtSize);

	return 0;
}

/*
    命令：$Recx!
    回报：#Y$Rec! //开始录音
          #R$Rec! //已经处于录音状态
          #P$Rec! //已经处于放音状态
          #N$Rec! //未录音(无卡、卡错误等原因)
*/
Int16 start_record(void)
{
	Int8 bcd[64];
	Int8 *src, *dst;
	int res;
	int i;
	Uint16 tempFunc;

	XmtSize = 7;
	mem_cpy(tx_buffer, "#Y$Rec!", XmtSize);		// default set to YES
	if(recordFile.function & RECORD) {
		return 'R';								// already in Record mode
	} else if(recordFile.function & REPLAY) {
		return 'P';								// in Playback mode
	}

	rtc_get_time(bcd, 8);
//	rtc_set_time("\x00\x00\x00\x00\x12\x05\x22\x03\x13", 9);
	dst = recordFile.curFileName;
	src = &bcd[7];

	*dst++ = '2';
	*dst++ = '0';
	for(i = 3; i--; ){
		*dst++ = ((*src >> 4) & 0x0f) | '0';
		*dst++ = ((*src--) & 0x0f) | '0';
	}

	*dst++ = '\0';
	for(i = 3; i--; ){
		*dst++ = ((*--src >> 4) & 0x0f) | '0';
		*dst++ = ((*src) & 0x0f) | '0';
	}

	if(recordFile.channel & (1<<LEFT))
		*dst++ = 'L';
	else
		*dst++ = 'X';

	if(recordFile.channel & (1<<RIGHT))
		*dst++ = 'R';
	else
		*dst++ = 'X';

	*dst++ = '.';

	if(recordFile.code == 0) {
		*dst++ = 'R';
		*dst++ = 'A';
		*dst++ = 'W';
		tempFunc = FILE_RECORD_RAW;
	} else {							// 16, 24, 32kbps
		*dst++ = ((recordFile.code & 0x00f0) >> 4) + '0';
		*dst++ = (recordFile.code & 0x000f) + '0';
		*dst++ = 'K';
		tempFunc = FILE_RECORD_COD1;
	}
	*dst++ = '\0';

	f_mkdir(recordFile.curFileName);
	recordFile.curFileName[8] = '/';
	res = f_open(&file, recordFile.curFileName, FA_CREATE_ALWAYS | FA_WRITE);
	if(res)									// can't open
		return 'N';

	recordFile.function = tempFunc;
	recordFile.frames = recordFile.split;

	return 0;
}

/*
    命令：$Play!   //放当前列出的文件
    回报：#Y$Play! //开始放音
          #R$Play! //已经处于录音状态
          #P$Play! //已经处于放音状态
          #N$Play! //未放音(无卡、卡错误等原因)
*/
Int16 start_play(void)
{
	int i;
	int res;
	Uint16 tempFunc, tempCode, tempSize, tempChannel;

	XmtSize = 8;
	mem_cpy(tx_buffer, "#Y$Play!", XmtSize);	// default set to YES
	if(recordFile.function & RECORD) {
		return 'R';
	} else if(recordFile.function & REPLAY) {
		return 'P';
	}

	for(i = 0; i < 64; i++)
		if(recordFile.curFileName[i] == 0)
			break;

	i -= 4;
	if (mem_cmp(".RAW", &recordFile.curFileName[i], 4) == 0)
	{
		tempFunc = FILE_REPLAY_RAW;
		tempCode = 0;
		tempSize = gDct_length * 2;
	} else if (mem_cmp(".16K", &recordFile.curFileName[i], 4) == 0)
	{
		tempFunc = FILE_REPLAY_COD1;
		tempCode = 0x0116;
		tempSize = number_of_bytes_per_frame;
	} else if (mem_cmp(".24K", &recordFile.curFileName[i], 4) == 0)
	{
		tempFunc = FILE_REPLAY_COD1;
		tempCode = 0x0124;
		tempSize = number_of_bytes_per_frame;
	} else if (mem_cmp(".32K", &recordFile.curFileName[i], 4) == 0)
	{
		tempFunc = FILE_REPLAY_COD1;
		tempCode = 0x0132;
		tempSize = number_of_bytes_per_frame;
	} else {								// wrong filename
		return 'E';
	}

	tempChannel = 0;
	if(recordFile.curFileName[--i] == 'R')
		tempChannel |= 2;
	if(recordFile.curFileName[--i] == 'L')
		tempChannel |= 1;

	if(tempChannel == 0) {			// wrong filename
		return 'N';
	}

	res = f_open(&file, recordFile.curFileName, FA_READ);
	if(res)									// can't open
		return 'N';
	else {
		recordFile.channel = tempChannel;
		recordFile.function = tempFunc;
		recordFile.code = tempCode;
		recordFile.framesize = tempSize;
	}

	return 0;
}

/*
    命令：$Stop!
    回报：#Y$Stop! //成功执行停止
          #I$Stop! //已经处于Idle状态
          #N$Stop! //未执行停止(无卡、卡错误等原因)
*/
Int16 stop_command(void)
{
	XmtSize = 8;
	mem_cpy(tx_buffer, "#Y$Stop!", XmtSize);	// default set to YES

	if(recordFile.function == DIRECT_IO)
		return 'I';
	else if((recordFile.function & RECORD) ||
			(recordFile.function & REPLAY)) {
		recordFile.function = DIRECT_IO;
		if(f_close(&file))
            return 'N';
	}

	return 0;
}

Int16 delete_all(void)
{
	return -1;
}

Int16 status_inquery(void)
{
	return -1;
}

/*
    命令：$Cap?!
    回报：#N$Cap?!    //无法显示容量(无卡)
          #967MB$Cap?!  //967MB剩余容量(小于1MB显示0)
*/
Int16 card_capacity(void)
{
	Uint32 nclst;
	Int16 res;
	Uint8 c;

	res = f_getfree ("/", &nclst, &fs);
	if(res == FR_OK) {
		nclst >>= 8;   // in MB
		tx_buffer[0] = '#';
		c = (Uint8)(nclst / 1000); nclst -= 1000*c;
		tx_buffer[1] = c + '0';
		c = (Uint8)(nclst / 100);  nclst -= 100*c;
		tx_buffer[2] = c + '0';
		c = (Uint8)(nclst / 10);   nclst -= 10*c;
		tx_buffer[3] = c + '0';
		c = (Uint8)nclst;
		tx_buffer[4] = c + '0';
		mem_cpy(&tx_buffer[5], "MB$Cap?!", 8);
		XmtSize = 13;
	} else {
		XmtSize = 8;
		mem_cpy(tx_buffer, "#N$Cap?!", XmtSize);
	}
	return 0;
}

/*
    命令：#0$List!    //顺序列表
          #1$List!    //倒序列表
    回报：#N$List!    //无法执行指令
          #234138-1227M04.MP3$List! //显示文件名 每发1次显示1条文件名
          #End$List!  //没有文件了(下面将循环显示)
     #/$List! 从根目录开始显示
*/
Int16 dir_list(void)
{
	static DIR struc_dir;
	FILINFO f_info;
	Int8 *ptr_fn;
	Int8 localdir[64];
	int i, j;

	ptr_fn = &rx_buffer[1];
	for(i = 0; i < rcvCounter; i++) {
		localdir[i] = *ptr_fn++;
		if(localdir[i] == '$') {
			localdir[i] = '\0';
			break;
		}
	}

	if(mem_cmp(recordFile.curFileName, localdir, i)) {
		if (FR_OK != f_opendir(&struc_dir, localdir)) {
			XmtSize = 8;
			mem_cpy(tx_buffer, "#N$List!", XmtSize);
			return 0;
		}
	}

	mem_cpy(recordFile.curFileName, localdir, i);
	recordFile.curFileName[i++] = '/';
	ptr_fn = tx_buffer;
	if(f_readdir(&struc_dir, &f_info)) {		// open error
		XmtSize = 10;
		mem_cpy(tx_buffer, "#End$List!", XmtSize);
		recordFile.curFileName[0] = 0;
	} else {
		*ptr_fn++ = '#';
		for(j = 0; j < 13; j++) {
			*ptr_fn++ = f_info.fname[j];
			recordFile.curFileName[i+j] = f_info.fname[j];
			if(f_info.fname[j] == '\0')
				break;
		}
		mem_cpy(ptr_fn - 1, "$List!", 6);
		XmtSize = j + 6;
	}

	return 0;
}

/*
    命令：#234138-071227M04.MP3$Dele! //删除指定文件
    回报：#N$Del!    //无法执行指令(可能没有此文件或者数据错误)
          #Y$Del!    //文件成功删除
    删除文件需要绝对路径
*/
Int16 file_delete(void)
{
	Int8 *ptr_fn;
	Int8 localdir[64];
	int i;

	XmtSize = 7;
	mem_cpy(tx_buffer, "#Y$Del!", XmtSize);		// default set to YES
	ptr_fn = &rx_buffer[1];
	for(i = 0; i < rcvCounter; i++) {
		localdir[i] = *ptr_fn++;
		if(localdir[i] == '$') {
			localdir[i] = '\0';
			break;
		}
	}

	if(FR_OK != f_unlink(localdir))
		return 'N';

	return 0;
}

typedef struct _UartList {
	Uint32 cmdID;
	Int16 (*function)(void);
} UartList;


UartList commandList[] = {
	{0x54696d65, timer_settings},	//时间相关 "#10-10-19-09:12:07$Time!"
	{0x4d6f6465, set_record_mode},	//设置录音模式 #xyz$Mode!"
									//x=l/r/s 左/右/立体声
									//y=h/m/l 音质高/中/低
									//z=0/1 加密否/是
	{0x4d436f64, set_machine_id},	//设置机器ID "#xx$MCod!"
	{0x53436f64, set_rec_level},	//录音灵敏度设置 #xx$SCod!"
	{0x5353706b, set_spk_level},	//输出音量设置 #xx$SSpk!"
	{0x53706c74, set_split},		//录音分段时间设置 "#xx$Splt!"
	{0x52656378, start_record},		//录音指令 "$Recx!"
	{0x53746f70, stop_command},		//停止指令 "$Stop!"
	{0x506c6179, start_play},		//放音指令 "$Play!"
	{0x44656c41, delete_all},		//全删指令 "$DelA!"
	{0x53746174, status_inquery},	//状态查询 "$Stat!"
	{0x4361703f, card_capacity},	//卡容量查询 "$Cap?!"
	{0x4c697374, dir_list},			//列目录.x=0顺序,x=1倒序"#x$List!"
	{0x44656c65, file_delete},		//删除指定文件 "#234138-071227M04.MP3$Dele!"
};

void uartProcessing(void)
{
	Uint32 uartID = 0;
	Uint16 n, i;
	Uint8 stat = 0;

	if((taskList & UARTPROCESSING) == 0)	return;

	n = rcvCounter - 6;
	while(n < rcvCounter - 1) {
		uartID = (uartID << 8) | rx_buffer[n++];
	}

	n = sizeof(commandList) / sizeof(UartList);
	for(i = 0; i < n; i++) {
		if(uartID == commandList[i].cmdID) {
			stat = commandList[i].function();
			break;
		}
	}

	if(stat) { 
		tx_buffer[1] = stat;
	}
	tx_buffer[XmtSize] = 0;
	LCD_print(tx_buffer, 0);
	xmtCounter = 0;
	CSL_UART_REGS->FCR = 0x08;		// Clear UART TX & RX FIFOs, DMAMODE1
	CSL_UART_REGS->IER |= 0x0002;	// TX interrupt enabled

	taskList &= ~UARTPROCESSING;
}

/*
 *  UART_init( )
 *    Open UART handle and configure it for 115200bps, 8N1
 * 
 */
void EZDSP5535_UART_init( )
{
#ifndef	M41TXX_ADDR
	CSL_GPIO_REGS->IODIR1 |= 0x8000;
	CSL_GPIO_REGS->IOOUTDATA1 = 0x0000; /* FT2232 enable */
#endif

//	Transmitter and receiver are disabled and in reset state.
	CSL_UART_REGS->PWREMU_MGMT = 0;

    /*  
     * Configuring for baud rate of 115200
     * Divisor = UART input clock frequency / (Desired baud rate*16)
     * = 100MHz / 115200 / 16
     * = 54 = 0x36
     */
	CSL_UART_REGS->LCR = 0x80;			// DLAB=1
	CSL_UART_REGS->DLL = 0x36;
	CSL_UART_REGS->DLH = 0x00;
	CSL_UART_REGS->FCR = 0x08;			// Clear UART TX & RX FIFOs, DMAMODE1
	CSL_UART_REGS->LCR = 0x03;			// 8-bit words,
										// 1 STOP bit generated,
										// No Parity, No Stick paritiy,
										// No Break control

	CSL_UART_REGS->MCR = 0x0000;		// RTS & CTS disabled,
										// Loopback mode disabled,
										// Autoflow disabled

	CSL_UART_REGS->IER = 0x0001;		// RX interrupt enabled
//	Transmitter and receiver are enabled
	CSL_UART_REGS->PWREMU_MGMT = 0x6000;

	CSL_CPU_REGS->IER0 |= (1 << UART_EVENT);	// UART interrupt enabled

//	rtc_set_time("\x00\x00\x00\x00\x12\x05\x02\x13\x13", 9);
	rtc_set_time("\x00\x00\x00\x05\x12\x09\x12\x13\x13", 9);
	// set time at 2013-03-22-12:00:00.00, Friday
	taskList &= ~UARTPROCESSING;
}

