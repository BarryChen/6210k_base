#ifndef __PLAYCTRL_H__
#define __PLAYCTRL_H__


// Define playing mode.
typedef enum _REPEAT_MODE
{
	REPEAT_ALL = 0,
	REPEAT_ONE,	
	REPEAT_RANDOM,
	REPEAT_INTRO,
	REPEAT_FOLDER,	
	REPEAT_MODE_SUM,	

} REPEAT_MODE;


typedef enum _PLAY_DIRECT
{
	PLAY_DIRECT_NEXT,
	PLAY_DIRECT_PRE,
	PLAY_DIRECT_NEXT_10,
	PLAY_DIRECT_PRE_10	

} PLAY_DIRECT;


// Define playing control state structure.
typedef enum _PLAY_STATE
{
	PLAY_STATE_IDLE = 0,
	PLAY_STATE_STOP,
	PLAY_STATE_PLAY,
	PLAY_STATE_PAUSE,
	PLAY_STATE_FF,
	PLAY_STATE_FB,
	PLAY_STATE_BROWSE

} PLAY_STATE;


// Paly error type.
typedef enum _PLAY_ERROR
{
	PLAY_ERROR_NONE = 0,
	PLAY_ERROR_SONG_INIT,
	PLAY_ERROR_SONG_CMPT,
	PLAY_ERROR_PLAY_DO
		
} PLAY_ERROR;

// Define play control structure.
typedef struct _PLAY_CTRL
{
	PLAY_STATE		State;	
	BYTE			Event;

	BYTE			RepeatMode;
	BYTE			Eq;
	BYTE			Direction;

	BOOL			FolderEnable;	
	WORD			FolderNum;	
	FOLDER			Folder;

	WORD			FileNum; 
	FILE			File;
	
//	BOOL 			LrcFlag;

} PLAY_CTRL;


extern PLAY_CTRL gPlayCtrl;
extern WORD CODE gVolArr[];

BOOL PlayCtrlInit(VOID);			
VOID PlayStateCtrl(VOID);									
BOOL PlayCtrlEnd(VOID);
BOOL SongPlayInit(VOID);

// External interface for updating song data for playing.
// Note: in AP layer code (such as LED display program),
//	the function is invoked to keeping song playing continuously.
BOOL SongPlayDo(VOID);
VOID PlayVol2Decd(VOID);							

#endif
