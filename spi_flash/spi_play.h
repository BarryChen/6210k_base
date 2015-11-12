#ifndef __SPI_PLAY_H__
#define __SPI_PLAY_H__

typedef	enum	
{	
	SPIPLAY_SONG_PLAY_BT = 1,
	SPIPLAY_SONG_PLAY_AUX,
	SPIPLAY_SONG_PLAY_TF,
	SPIPLAY_SONG_LOW_PWR,
	SPIPLAY_SONG_MAX_VOLUME,
	SPIPLAY_SONG_MIN_VOLUME,
	SPIPLAY_SONG_POWEROFF,//用作关机提示音
	SPIPLAY_SONG_POWER_ON_SOUND1,
	SPIPLAY_SONG_IR_ON_OFF,
	SPIPLAY_SONG_IR_KEY,
	SPIPLAY_SONG_LOCK_OFF,
	SPIPLAY_SONG_LOCK_ON,
	SPIPLAY_SONG_LOCKING,
/*	SPIPLAY_SONG_NEXT=1,
	SPIPLAY_SONG_PLAY,
	SPIPLAY_SONG_PAUSE,
	SPIPLAY_SONG_REPLAY_START,
	SPIPLAY_SONG_REPLAY_END,
	SPIPLAY_SONG_REC_START,
	SPIPLAY_SONG_REC_PLAY,
	SPIPLAY_SONG_PLAY_USB,
	SPIPLAY_SONG_PLAY_TF,
	SPIPLAY_SONG_PLAY_AUX,
	SPIPLAY_SONG_VOLUP,
	SPIPLAY_SONG_VOLDN,
	SPIPLAY_SONG_AUTO_SEARCH,
	SPIPLAY_SONG_PREV_STATION,	
	SPIPLAY_SONG_NEXT_STATION,
	SPIPLAY_SONG_REP_ALL,
	SPIPLAY_SONG_REP_ONE,
	SPIPLAY_SONG_LOW_PWR,
	SPIPLAY_SONG_IR_0,
	SPIPLAY_SONG_IR_1,
	SPIPLAY_SONG_IR_2,
	SPIPLAY_SONG_IR_3,
	SPIPLAY_SONG_IR_4,
	SPIPLAY_SONG_IR_5,
	SPIPLAY_SONG_IR_6,
	SPIPLAY_SONG_IR_7,
	SPIPLAY_SONG_IR_8,
	SPIPLAY_SONG_IR_9,
	SPIPLAY_SONG_MUTE,	
	SPIPLAY_SONG_UNMUTE,
	SPIPLAY_SONG_ENTER_STB,
	SPIPLAY_SONG_NOW_TIME,
	SPIPLAY_SONG_HOUR_0,
	SPIPLAY_SONG_HOUR_1,
	SPIPLAY_SONG_HOUR_2,
	SPIPLAY_SONG_HOUR_3,
	SPIPLAY_SONG_HOUR_4,
	SPIPLAY_SONG_HOUR_5,
	SPIPLAY_SONG_HOUR_6,
	SPIPLAY_SONG_HOUR_7,
	SPIPLAY_SONG_HOUR_8,
	SPIPLAY_SONG_HOUR_9,
	SPIPLAY_SONG_HOUR_10,
	SPIPLAY_SONG_HOUR_11,	
	SPIPLAY_SONG_HOUR_12,
	SPIPLAY_SONG_HOUR_13,
	SPIPLAY_SONG_HOUR_14,
	SPIPLAY_SONG_HOUR_15,
	SPIPLAY_SONG_HOUR_16,
	SPIPLAY_SONG_HOUR_17,
	SPIPLAY_SONG_HOUR_18,
	SPIPLAY_SONG_HOUR_19,
	SPIPLAY_SONG_HOUR_20,
	SPIPLAY_SONG_HOUR_21,
	SPIPLAY_SONG_HOUR_22,
	SPIPLAY_SONG_HOUR_23,
	SPIPLAY_SONG_MIN_30,
	SPIPLAY_SONG_PREV,
	SPIPLAY_SONG_ENTER_RADIO,
	SPIPLAY_SONG_ENTER_RTC,
	SPIPLAY_SONG_OPEN_TELLTIME,
	SPIPLAY_SONG_CLOSE_TELLTIME,
	SPIPLAY_SONG_POWER_ON_SOUND1,
	SPIPLAY_SONG_POWER_ON_SOUND2,*/
} SPIPLAY_SONG;


BOOL SPI_PlaySelectNum(BYTE SpiPlayNum, BOOL PlayResumeFlag);


#endif
