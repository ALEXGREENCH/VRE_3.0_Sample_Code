#ifndef _VRE_APP_Audio_
#define	_VRE_APP_Audio_

#define	APP_SERIAL_NUM	"WQ-TST-10000"		//product ID

#define MILLISECOND_PER_HOUR (60 * 60 * 1000)
#define MILLISECOND_PER_MINUTE (60 * 1000)
#define MILLISECOND_PER_SECOND (1000)

VMCHAR res_name[][32] = {					//resource name
	"dingdang.mid"
};

#define RES_NUM	(sizeof(res_name) >> 5)		//the number of resource 

enum {										//source ID
	MIDI_DD = 0
};

typedef struct res_info {						
	VMUINT8*	buffer;						
	VMINT		length;						
	VMINT		handle;						
} RES_INFO_T;

/**
 * app state
 */
typedef enum app_state {
	STATE_MENU	= 0,						//main menu
	STATE_RES_PLAYING,						//midi playing
	STATE_FILE_PLAYING,						//audio file playing
	STATE_FILE_PAUSE,						//audio file play paused
	STATE_RECORDING,						//recording
	STATE_RECORD_PAUSE						//record paused
} APP_STATE_ENUM;

typedef struct app_info {
	VMUINT8			*buffer;				
	VMUSHORT		timer_id;				
	APP_STATE_ENUM	app_state;				
	VMINT			handle;						
	VMINT			volume;					
	VMINT			screen_width;			
	VMINT			screen_height;			
	VMINT			character_height;		
	VMINT			start_time;				
	VMINT			current_time;			
	VMINT			total_time;				
	VMINT			file_duration;			
} APP_INFO_T;

#endif

