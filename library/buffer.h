#ifndef __BUFFER_H__
#define __BUFFER_H__


// 0x0000~0x0DFF	3.5KB		XDATA		系统变量使用，非DMA访问区域
// 0x0E00~0x0FFF	512B		FAT BUFFER	存放FAT表数据用
// 0x1000~0x11FF	512B		DIR BUFFER	存放DIR信息用(与DATA_BUFFER_A地址重合)
// 0x1000~0x17FF	2KB			INPUT BUFFER 存放MP3/WMA数据,Decoder解码时从此取数据，分为DATA_BUFFER_A/B/C/D四块，乒乓方式用
// 0x1800~0x24FF	3.25KB 		PCM BUFFER	PCM播放可用，FPGA测试时Decoder解码后数据存放在此
//	
#define READ_DIR_BUFFER			0x0F00//0x1000					//directory cache	
#define	READ_FAT_BUFFER			0x0D00//0x0D00					//FAT cache for read file, 512 bytes

//PLAY BUFFER
//4个连续0.5K input buf
#define	PLAY_BUFFER_START		0x0F00
#define	PLAY_BUFFER_SIZE		0x0800
#define	PLAY_BUFFER_END			(PLAY_BUFFER_START+PLAY_BUFFER_SIZE)
#define DATA_BUFFER_TOTALSIZE	0x0800
#define	PLAY_READSIZE			512//1024


//VMEM FIFO :8KB
#define	VMEM_BUFF_START			0x2000
#define	VMEM_BUFF_END			0x4000
#define	VMEM_BUFF_LEN			(VMEM_BUFF_END-VMEM_BUFF_START)
//#define	DATA_BUFFER_VMEM_A		0x4000 					//buffer for PCM data, 4K bytes
//#define	DATA_BUFFER_VMEM_B		0x5000 					//buffer for PCM data, 4K bytes
#define	DATA_BUFFER_VMEM_LEN	0x1000

#define VMEM_START_ADDR     	0x4000

//ISO(USB AUDIO) FIFO: 6KB
#define AUDIO_BUF_START			VMEM_BUFF_START
#define AUDIO_BUF_END			(AUDIO_BUF_START + 1024*6)	//6KB --> 大于6KB，与采样率相关
#define AUDIO_BUF_LEN			(AUDIO_BUF_END - AUDIO_BUF_START)
#define	AUDIO_PCM_LEN			2048

//PCM FIFO: 3KB	
#define PARSE_BUFF_START        0x1700        
#define PARSE_BUFF_LEN			1024



extern BYTE XDATA* DATA g_pParser;		//point of current position


//Init Buffer Module
BOOL InitBuffer(VOID);


//move parser pointer 
BOOL MoveParserPos(SDWORD nBytes, WORD leastbytes);


#endif

