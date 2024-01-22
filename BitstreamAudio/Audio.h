#ifndef _AUDIO_H_
#define _AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Engine.h"

typedef enum {
	AUDIO_CLOSED,
		AUDIO_STOPPED,		
		AUDIO_OPENED,
		AUDIO_PLAYING,
		AUDIO_BUFFERING		
} AUDIO_PLAY_STATE_ENUM;

typedef enum {
	AUDIO_NOT_CLOSED = -7,
		AUDIO_OPEN_FAILED,
		AUDIO_PLAY_FAILED,
		READ_EOF,
		MALLOC_MEM_FAILED,
		READ_FILE_FAILED,
		AUDIO_PLAY_FINISHED
} AUDIO_PLAY_ERROR_ENUM;

typedef enum {
	AUDIO_ALREADY_CLOSED = -4,
		AUDIO_STOP_FAILED,
		AUDIO_CLOSE_FAILED,
		FILE_CLOSE_FAILED
} AUDIO_STOP_ERROR_ENUM;

typedef void(*AUDIO_CALLBACK_T)(VMUINT);

VMINT ba_audio_play(VMINT file_hdl, VMUINT file_size, AUDIO_CALLBACK_T audio_cb);

VMINT ba_audio_stop(void);

VMINT ba_get_audio_state(void);

#ifdef __cplusplus
}
#endif

#endif
