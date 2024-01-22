#ifndef _VRE_APP_Audio_demo_3_
#define	_VRE_APP_Audio_demo_3_

#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmmm.h"
#include "vmtimer.h"
#include <stdio.h>
#include "vmlog.h"

//#define		SUPPORT_BG
#define		APP_SERIAL_NUM	"LEON-RECORD-10000"			//product id

/************************************************************************/
/*																		*/
/* program running status                                               */
/*																		*/
/************************************************************************/
typedef enum app_state {
	STATE_MENU	= 0,						//main menu					Ö÷²Ëµ¥Ì¬
		STATE_RECORDING,					//recording					Â¼ÒôÌ¬
		STATE_RECORD_PAUSE,					//record paused				Â¼ÒôÔÝÍ£Ì¬
		STATE_RECORD_RESUME,				//record resume				Â¼Òô»Ö¸´
		STATE_RECORD_STOP,					//record stop				Â¼ÒôÍ£Ö¹
		STATE_RECORD_EXIT					//record exit				Â¼ÒôÍË³ö
		
} APP_STATE;


/************************************************************************/
/*																		*/
/*				global variable struct                                  */ 
/*																		*/
/************************************************************************/
typedef struct app_info {
	VMINT		    layer_hdl[1];		//layer handle
	VMUINT8			*buffer_base;		//layer buf
	VMUSHORT		timer_id;			//timer ID	
	APP_STATE	    app_state;			//program running status	
	
	VMINT			screen_width;		//screen width
	VMINT			screen_height;		//screen height
	VMINT			character_height;	//character height
} APP_INFO_T;

/************************************************************************/
/*																		*/
/*	character sturct													*/
/*																		*/
/************************************************************************/
typedef struct character_t						
{
	VMWCHAR				Wstr[50];				
	VMUINT16			width;					
	VMUINT16			height;					
	VMINT				x;						
	VMINT				y;						
}CHARACTER_T;



/*
* system events
*/
void handle_sysevt(VMINT message, VMINT param);

/*
* key events
*/
void handle_keyevt(VMINT event, VMINT keycode);

/*
* pen events
*/
void handle_penevt(VMINT event, VMINT x, VMINT y);

/*
* initialization
*/
void App_init();

VMINT App_IsInArea(VMINT x,VMINT y,VMINT rectangleX,VMINT rectangleY,VMINT rectangleWidth,VMINT rectangleHeight);

void App_Creat_Timer_CallBack(VMINT tid);

void App_Sleep(VMINT timer);

void App_Record_CallBack(VMINT result);

VMINT App_Record();

#endif

