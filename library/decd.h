#ifndef __DECD_H__
#define __DECD_H__



#define FAST_FORWARD		0
#define	FAST_BACKWARD		1


// EQ
#define 	EQ_BANDS_NUM 			10
// 定义总的EQ种类数目
// 用户可根据需要，减少实际使用的EQ种类
#define 	DECD_EQ_SUM				11
	
// 定义每种EQ的索引
#define		DECD_EQ_NORMAL			0
#define		DECD_EQ_ROCK			1
#define		DECD_EQ_POP				2
#define		DECD_EQ_CLASSIC			3
#define		DECD_EQ_JASS			4		
#define		DECD_EQ_BLUE			5
#define		DECD_EQ_HALL			6
#define		DECD_EQ_BASS			7
#define		DECD_EQ_SOFT			8
#define		DECD_EQ_COUNTRY			9
#define		DECD_EQ_OPERA			10


typedef enum _SONG_TYPE
{
	SONG_TYPE_UNKOWN,
	SONG_TYPE_MP3,
	SONG_TYPE_PCM_1CH,
	SONG_TYPE_PCM_2CH,
	//SONG_TYPE_PCM_AUDIO

} SONG_TYPE;


typedef struct _SONG_INFO
{
	BYTE	SongType;			//曲目类型
	BYTE	ChannelNum;			//声道个数
	WORD	SamplesPerSecond;	//采样率
	DWORD	BytesPerSecond;		//码率
	DWORD	TotalPlayTime;		//总播放时间	
	DWORD	CurPlayTime;		//当前播放时间	
	DWORD	FrameLen;			//MP3帧长度、WMA包长度、ADPCM块长度	
	DWORD	FrameNum;			//帧总数
	DWORD	FileHeaderLen;		//文件中，有效数据前的文件头字节数

	WORD	PlayBuffer;			//当前输入数据缓冲区首地址
	WORD	DataOffset;			//
	
	BOOL	IsVBR;				//VBR flag
	BYTE	Mp3MpegVersion;		//mpeg version value
	BYTE	Mp3Layer;			//layer value

} SONG_INFO;


extern SONG_INFO gSongInfo;


// Set decoder EQ value.
VOID DecoderSetEQ(BYTE* EQ_Table);

// Pause decoding
// Description: Send pause signal to decoder
VOID DecoderPause(VOID);

// Resume decoder
// Description: Send resume signal to decoder
VOID DecoderStartPlay(VOID);	

// 歌曲播放前的分析
BOOL DecoderAnalysis(FILE* File);				
				
// Judge fifo state
// If Fifo empty return TRUE,otherwise return FALSE.
BOOL	IsFifoEmpty(VOID);					

// Set decoder mode
BOOL SetDecoderMode(BYTE DecoderMode);

// Set decoder mode
// Calling this will notify the decoder that data is prepared,Decoder will start the DMA transfer.
BOOL SetDecoderData(WORD Addr, WORD Len);

BOOL IsDecoderDone(VOID);

// 定位到指定时间
BOOL DecoderSeekToTime(DWORD PlayTime);

// 设置快进，快退类型和倍率
BOOL DecoderFastPlaySet(BYTE FastPlayType, BYTE FastPlayRate);

// fast playing process
BOOL DecoderFastPlayProc(VOID);


#endif
