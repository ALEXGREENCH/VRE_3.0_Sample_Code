/* ---------------------------------------------------------------------------
 *                              standard VRE header file
 * ------------------------------------------------------------------------ */
#include "vmio.h"
#include "vmsock.h"
#include "vmgraph.h"
#include "vmgfxold.h"
#include "vmchset.h"
#include "vmgps.h"
#include "vmlog.h"
#include "vmstdlib.h"

/* ---------------------------------------------------------------------------
 *                              non-standard VRE header file
 * ------------------------------------------------------------------------ */

#include "gps_demo.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

/* ---------------------------------------------------------------------------
 *                          global define
 * ------------------------------------------------------------------------ */

#define SCREEN_WIDTH vm_graphic_get_screen_width() 
#define SCREEN_HEIGHT vm_graphic_get_screen_height() 
#define FONT_HEIGHT (vm_graphic_get_character_height() + 5)

/* ---------------------------------------------------------------------------
 *                           variable define
 * ------------------------------------------------------------------------ */

VMINT layer[1];
VMUINT8 *layer_buffer;
VMINT ProgramState;
VMINT LETTER_WIDTH;
VMINT FONT_WIDTH;
VMINT timerid;
VMINT KEYCODE;

vm_gps_gga_data_struct * gps_gga_data;
vm_gps_rmc_data_struct * gps_rmc_data;
vm_gps_gll_data_struct * gps_gll_data;
vm_gps_gsv_data_struct * gps_gsv_data;
vm_gps_gsa_data_struct * gps_gsa_data;
vm_gps_vtg_data_struct * gps_vtg_data;


VMINT const gps_hint_len = 8;
VMINT8  GPS_HINT[][40]={
	"Num1: Open GPS",
		"Num2: Get the GGA info",
		"Num3: Get the RMC info",
		"Num4: Get the GLL info",
		"Num5: Get the GSV info",
		"Num6: Get the GSA info",
		"Num7: Get the VTG info",
		"Num8: Close GPS"
};

VMINT gps_open_return;
VMCHAR GPS_OPEN_VALUE[200];

/* ---------------------------------------------------------------------------
 *                           function declaration
 * ------------------------------------------------------------------------ */

void init_vre(void);//initialization
void timer_proc(VMINT tid);

void key_event(VMINT event,VMINT keycode);//key events
void sys_event(VMINT handel,VMINT event);//system events

void key_logo(VMINT event,VMINT keycode);//key events on logo screen
void key_gps_hint(VMINT event,VMINT keycode);//key events on gps menu screen
void key_gps_info(VMINT event,VMINT keycode);//key events on gps info screen

void draw_logo(void);// draw logo
void draw_gps_hint(void);//draw gps menu screen
void draw_gps_get_info(void);//draw gps info screen

void exit_demo(void);//exit program

void open_gps(void);//Open GPS
void close_gps(void);//Close GPS
void get_gps_gga(void);//Get the GGA info
void get_gps_rmc(void);//Get the RMC info
void get_gps_gll(void);//Get the GLL info
void get_gps_gsv(void);//Get the GSV info
void get_gps_gsa(void);//Get the GSA info
void get_gps_vtg(void);//Get the VTG info

/* entry */
void vm_main(void)
{
    vm_reg_keyboard_callback(key_event);
	vm_reg_sysevt_callback(sys_event);	
}

/* ---------------------------------------------------------------------------
 *                           function implementation
 * ------------------------------------------------------------------------ */

void init_vre(void)
{     
     ProgramState = LOGO;
	 KEYCODE = 0;
     gps_open_return = - 5;
	 memset(GPS_OPEN_VALUE,0,200);
	 timerid = vm_create_timer(100,timer_proc);
}

void key_event(VMINT event,VMINT keycode)
{
     switch(ProgramState)
	 {
	 case LOGO:
		 key_logo(event,keycode);
		 break;
     case GPS_INFO:
		 key_gps_hint(event,keycode);
		 break;
	 case GPS_RESULT:
		 key_gps_info(event,keycode);
		 break;
	 default:
		 break;
	 }
}

void sys_event(VMINT event,VMINT param)
{
    switch(event)
	{
	case VM_MSG_INACTIVE:
		break;
    case VM_MSG_ACTIVE:
		break;
	case VM_MSG_CREATE:
		init_vre();
		layer[0] = vm_graphic_create_layer(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,-1);//create base layer
		vm_graphic_set_clip(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
		layer_buffer = vm_graphic_get_layer_buffer(layer[0]);
		break;
	case VM_MSG_PAINT:
		break;
	case VM_MSG_QUIT:
		exit_demo();
		break;
	default:
		break;
	}
}

void open_gps(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
    gps_open_return = 0;
#else
	gps_open_return = vm_gps_open();
#endif
	switch(gps_open_return)
	{
	case VM_GPS_OPEN_SUCCESS:
		sprintf(GPS_OPEN_VALUE,"OPEN GPS SUCCESS");
		break;
	case VM_GPS_OPEN_ALREADY_OPEN:
		sprintf(GPS_OPEN_VALUE,"GPS HAS ALREADY OPENED");
		break;
	case VM_GPS_OPEN_PORT_NUMBER_WRONG:
		sprintf(GPS_OPEN_VALUE,"PORT NUMBER ERROR");
		break;
	case VM_GPS_OPEN_PORT_HANDLE_WRONG:
		sprintf(GPS_OPEN_VALUE,"OPEN UART HANDLE ERROR");
		break;
	case VM_GPS_OPEN_RES_LESS:
		sprintf(GPS_OPEN_VALUE,"LACK OF RESOURCE");
		break;
	}
}

void close_gps(void)
{
	VMINT close_return;
#ifdef _DEBUG //if running in emulator, implement code below 
    close_return = gps_open_return;
#else
	close_return = vm_gps_close();
#endif
    switch(close_return)
	{
	case VM_GPS_CLOSE_SUCCESS:
		sprintf(GPS_OPEN_VALUE,"CLOSE GPS SUCCESS");
		break;
	case VM_GPS_CLOSE_STILL_APP_USE_DATA:
		sprintf(GPS_OPEN_VALUE,"GPS IS STILL IN USE");
		break;
	case VM_GPS_CLOSE_FAILED:
		sprintf(GPS_OPEN_VALUE,"CLOSE GPS FAILED");
		break;
	default:
		sprintf(GPS_OPEN_VALUE,"GPS IS NOT EXIST");
		break;
	}
#ifdef _DEBUG //if running in emulator, implement code below 
	gps_open_return = -5;
#endif
}

void get_gps_gga(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
	if(gps_open_return == 0)
	{
        sprintf(GPS_OPEN_VALUE,"GGA quality:%d",1);
	}
#else
    if(gps_open_return == 0 || gps_open_return == 1)
	{
		gps_gga_data = vm_gps_get_gga_data();
        sprintf(GPS_OPEN_VALUE,"GGA quality:%d",gps_gga_data->gga_quality);
	}
#endif
	else
	{
        sprintf(GPS_OPEN_VALUE,"%s","Get the GGA info failed");
	}
}

void get_gps_rmc(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
	if(gps_open_return == 0)
	{
        sprintf(GPS_OPEN_VALUE,"Direction of declination:%c",'E');
	}
#else
    if(gps_open_return == 0 || gps_open_return == 1)
	{
		gps_rmc_data = vm_gps_get_rmc_data();
        sprintf(GPS_OPEN_VALUE,"Direction of declination:%c",gps_rmc_data->rmc_magnetic_e_w);
	}
#endif
	else
	{
        sprintf(GPS_OPEN_VALUE,"%s","Get the RMC info failed");
	}
}

void get_gps_gll(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
	if(gps_open_return == 0)
	{
        sprintf(GPS_OPEN_VALUE,"GLL Status:%c",'A');
	}
#else
    if(gps_open_return == 0 || gps_open_return == 1)
	{
		gps_gll_data = vm_gps_get_gll_data();
        sprintf(GPS_OPEN_VALUE,"GLL Status:%c",gps_gll_data->gll_status);
	}
#endif
	else
	{
        sprintf(GPS_OPEN_VALUE,"%s","Get the GLL info failed");
	}
}

void get_gps_gsv(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
	if(gps_open_return == 0)
	{
        sprintf(GPS_OPEN_VALUE,"Number of satellites:%d",4);
	}
#else
    if(gps_open_return == 0 || gps_open_return == 1)
	{
		gps_gsv_data = vm_gps_get_gsv_data();
        sprintf(GPS_OPEN_VALUE,"Number of satellites:%c",gps_gsv_data->gsv_num_sv_trk);
	}
#endif
	else
	{
        sprintf(GPS_OPEN_VALUE,"%s","Get the GSV info failed");
	}
}

void get_gps_gsa(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
	if(gps_open_return == 0)
	{
        sprintf(GPS_OPEN_VALUE,"Position mode:%c",'A');
	}
#else
    if(gps_open_return == 0 || gps_open_return == 1)
	{
		gps_gsa_data = vm_gps_get_gsa_data();
        sprintf(GPS_OPEN_VALUE,"Position mode:%c",gps_gsa_data->gsa_op_mode);
	}
#endif         
	else
	{
        sprintf(GPS_OPEN_VALUE,"%s","Get the GSA info failed");
	}
}

void get_gps_vtg(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
	if(gps_open_return == 0)
	{
        sprintf(GPS_OPEN_VALUE,"Mode indicator:%c",'D');
	}
#else
    if(gps_open_return == 0 || gps_open_return == 1)
	{
		gps_vtg_data = vm_gps_get_vtg_data();
        sprintf(GPS_OPEN_VALUE,"Mode indicator:%c",gps_vtg_data->vtg_mode);
	}
#endif         
	else              
	{
        sprintf(GPS_OPEN_VALUE,"%s","Get the VTG info failed");
	}
}

void key_logo(VMINT event,VMINT keycode)
{
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{
             switch(keycode)
			 {
			 case VM_KEY_LEFT_SOFTKEY:
				 ProgramState = GPS_INFO;
				 break;
			 case VM_KEY_RIGHT_SOFTKEY:
				 exit_demo();
				 break;
			 default:
				 break;
			 }
		}
		break;
	default:
		break;
	}
}

void key_gps_hint(VMINT event,VMINT keycode)
{	
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{    			 
             switch(keycode)
			 {			 
			 case VM_KEY_RIGHT_SOFTKEY://if press right soft key, return to logo screen
				 ProgramState = LOGO;
				 break;
			 case VM_KEY_NUM1:
				 ProgramState = GPS_RESULT;				 
				 open_gps();	
				 KEYCODE = 1;
                 break;
			 case VM_KEY_NUM2:
			     ProgramState = GPS_RESULT;
				 get_gps_gga();
                 KEYCODE = 1;
				 break;
			 case VM_KEY_NUM3:
				 ProgramState = GPS_RESULT;
				 get_gps_rmc();
				 KEYCODE = 1;
				 break;
			 case VM_KEY_NUM4:
                 ProgramState = GPS_RESULT;
				 get_gps_gll();
				 KEYCODE = 1;
				 break;
			 case VM_KEY_NUM5:
				 ProgramState = GPS_RESULT;
				 get_gps_gsv();
				 KEYCODE = 1;
				 break;
			case VM_KEY_NUM6:
				 ProgramState = GPS_RESULT;
				 get_gps_gsa();
				 KEYCODE = 1;
				 break;
			case VM_KEY_NUM7:
				 ProgramState = GPS_RESULT;
				 get_gps_vtg();
				 KEYCODE = 1;
				 break;
			case VM_KEY_NUM8:
				 ProgramState = GPS_RESULT;
				 close_gps();
				 KEYCODE = 1;
				 break;
			 default:
				 break;
			 }
		}
		break;
	default:
		break;
	}
}

void key_gps_info(VMINT event,VMINT keycode)
{
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{    			 
             switch(keycode)
			 {			 
			 case VM_KEY_RIGHT_SOFTKEY://if press right soft key, return to GPS menu screen
				 ProgramState = GPS_INFO;
				 KEYCODE = 0;
				 break;			
			 default:
				 break;
			 }
		}
		break;
	default:
		break;
	}
}

void draw_logo(void)
{
	VMWCHAR wstr[20];
	VMINT i = rand()%3;
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_WHITE,VM_COLOR_BLACK);
	vm_ascii_to_ucs2(wstr,40,"GPS_DEMO");
			vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,
		(SCREEN_HEIGHT-vm_graphic_get_character_height())/2,wstr,wstrlen(wstr),COLOR[i]);
	vm_ascii_to_ucs2(wstr,40,"start");
	vm_graphic_textout(layer_buffer,2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),COLOR[i]);
	vm_ascii_to_ucs2(wstr,40,"exit");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr)-2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_graphic_flush_layer(layer,1);
}

void draw_gps_hint(void)
{
    VMINT i = 0; 
	VMWCHAR wstr[40];
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_GREEN,VM_COLOR_BLACK);
	vm_ascii_to_ucs2(wstr,80,"GPS MENU");
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,13,wstr,wstrlen(wstr),VM_COLOR_888_TO_565(129,220,10));
	for(i=0;i<gps_hint_len;i++)
	{
		vm_ascii_to_ucs2(wstr,80,GPS_HINT[i]);
		vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,(SCREEN_HEIGHT-FONT_HEIGHT*gps_hint_len)/2+i*FONT_HEIGHT,wstr,wstrlen(wstr),VM_COLOR_BLUE);
	}
	vm_ascii_to_ucs2(wstr,80,"Return");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr) -2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_graphic_flush_layer(layer,1);
} 

void draw_gps_info(void)
{
	VMWCHAR wstr[40];
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_RED,VM_COLOR_BLACK);
	vm_ascii_to_ucs2(wstr,80,"GPS INFO");
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,13,wstr,wstrlen(wstr),VM_COLOR_888_TO_565(129,220,10));
    if(KEYCODE)
	{
		vm_ascii_to_ucs2(wstr,80,GPS_OPEN_VALUE);
        vm_graphic_textout(layer_buffer,2,30,wstr,wstrlen(wstr),VM_COLOR_888_TO_565(110,119,120));
	}
	vm_ascii_to_ucs2(wstr,80,"Return");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr)-2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),VM_COLOR_WHITE);
	vm_graphic_flush_layer(layer,1);
}

void timer_proc(VMINT tid)
{
	switch(ProgramState)
	{
	case LOGO:
		draw_logo();
		break;
	case GPS_INFO:
		draw_gps_hint();
		break;
	case GPS_RESULT:
		draw_gps_info();
		break;
	default:
		break;
	}
}

void exit_demo(void)
{
    vm_graphic_delete_layer(layer[0]);
	vm_delete_timer(timerid);
    vm_exit_app();
}