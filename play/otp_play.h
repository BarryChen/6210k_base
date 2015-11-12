#ifndef _OTP_PLAY_H_
#define _OTP_PLAY_H_

#define OTPPLAY_NUM_POWERON		0
#define OTPPLAY_NUM_PLAY		1
#define OTPPLAY_NUM_PAUSE		2
#define OTPPLAY_NUM_VOLUP		3
#define OTPPLAY_NUM_VOLDN		4

BOOL OTP_SongPlayStart(VOID);
VOID OTP_PlaySelNum(BYTE OtpPlayNum, BOOL PlayResumeFlag);

#endif

