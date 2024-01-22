/* ============================================================================
 *  VRE demonstration application.
 *  module name: socket
 *  source name: socket.c
 *  function: exchange data between client and server through TCP link
 *  author: heping.miao
 *  version: 1.0
 *  date: 2010-4-7
 *  note: please open server.exe before run this program in emulator
 * ========================================================================= */

/* ---------------------------------------------------------------------------
 *                              standard VRE header file
 * ------------------------------------------------------------------------ */
#include "vmio.h"
#include "vmsock.h"
#include "vmgraph.h"
#include "vmgfxold.h"
#include "vmchset.h"
#include "vmlog.h"
#include "vmstdlib.h"

/* ---------------------------------------------------------------------------
 *                              non-standard VRE header file
 * ------------------------------------------------------------------------ */

#include "string.h"
#include "socket.h"
#include "stdio.h"


/* ---------------------------------------------------------------------------
 *                          global define
 * ------------------------------------------------------------------------ */

#define SCREEN_WIDTH vm_graphic_get_screen_width() 
#define SCREEN_HEIGHT vm_graphic_get_screen_height() 
#define FONT_HEIGHT (vm_graphic_get_character_height() + 4) 
#define PORT 9099 
#define IP "127.0.0.1"  
#define CONTENTS "tcp_test" 


/* ---------------------------------------------------------------------------
 *                           variable define
 * ------------------------------------------------------------------------ */

VMINT layer[1];
VMUINT8 *layer_buffer;
VMINT tcp_handle;
VMINT ProgramState;
VMINT timerid;
VMINT tcp_state;
VMINT tcp_state_back_value;
VMCHAR tcp_read[1024];
VMCHAR tcp_state_info[][60] = {
	"Create connection failed, lack of resource",
		"Create socket failed",
		"Create socket success",
		"Send data success",
		"Read data success",
		"Read data failed",
		"Connection is not exist",
		"Close connection success",
		"Cannot connect to the server",
		"Cannot find host",
		"Connection has been established",
		"Connection closed by server"
};
VMINT8 TCP_INFO[][30] = {
	"Num 1 Create connection",
		"Num 2 Send data",
		"Num 3 Close connection",
		"TCP link status"
};

/* ---------------------------------------------------------------------------
 *                           function declaration
 * ------------------------------------------------------------------------ */

void init_vre(void);//initialization
void Init_Log(void);
void key_event(VMINT event,VMINT keycode);//key events
void sys_event(VMINT handel,VMINT event);//system events

VMINT tcp_test(void);
void tcp_callback(VMINT handle,VMINT event);

void key_logo(VMINT event,VMINT keycode);//key events on logo screen
void key_tcp_state(VMINT event,VMINT keycode);//key events on tcp screen

void draw_logo(void);//draw logo screen
void draw_tcp_state(void);//draw tcp screen

void timer_proc(VMINT tid);

void exit_tcp(void);//exit program


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
	 tcp_state = -2;
	 tcp_state_back_value = -1;
	 timerid = vm_create_timer(100,timer_proc);
	 Init_Log();
}

void key_event(VMINT event,VMINT keycode)
{
     switch(ProgramState)
	 {
	 case LOGO:
		 key_logo(event,keycode);
		 break;
     case TCPIP:
		 key_tcp_state(event,keycode);
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
		layer[0] = vm_graphic_create_layer(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,-1);
		vm_graphic_set_clip(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
		layer_buffer = vm_graphic_get_layer_buffer(layer[0]);
		break;
	case VM_MSG_PAINT:
		break;
	case VM_MSG_QUIT:
		exit_tcp();
		break;
	default:
		break;
	}
}


VMINT tcp_test(void)
{
	tcp_handle = vm_tcp_connect(IP,PORT,VM_TCP_APN_CMNET,tcp_callback);
	if( VM_TCP_ERR_NO_ENOUGH_RES == tcp_handle|| tcp_handle == VM_TCP_ERR_CREATE_FAILED)
	{	
		if(tcp_handle == VM_TCP_ERR_NO_ENOUGH_RES)
			tcp_state = 0;	//Create connection failed, lack of resource	
		else
			tcp_state = 1; //Create socket failed
		return tcp_handle;
	}
    tcp_state = 2;//Create socket success
	return 0;
}

void tcp_callback(VMINT handle,VMINT event)
{  
	tcp_state_back_value = event;
    switch(event)
	{
	case VM_TCP_EVT_CONNECTED:
		{
            tcp_state = 10;//Connection has been established
		}
		break;
	case VM_TCP_EVT_CAN_WRITE:
		{
			VMINT len = strlen(CONTENTS);
			len = vm_tcp_write(tcp_handle,CONTENTS,len);
			if(len>0)
			{
				tcp_state = 3; //Send data success
			}	
			else
			{
				vm_tcp_close(tcp_handle);
			}
		}
		break;
    case VM_TCP_EVT_CAN_READ:
		{		
			VMINT len = 0;
			memset(tcp_read,0,1024);
			len = vm_tcp_read(tcp_handle,tcp_read,1024);			
			if(len>0)
			{	
				tcp_state = 4;//Read data success
			}
			else
			{
				tcp_state = 5; //Read data failed
				vm_tcp_close(tcp_handle);
			}  		
		}
		break ;
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			 tcp_state = 8;//Cannot connect to the server
			 vm_tcp_close(tcp_handle);
		}
		break ;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			tcp_state = 9;//Cannot find host
			vm_tcp_close(tcp_handle);
		}
		break;		
	case VM_TCP_EVT_PIPE_CLOSED:
	    {
			tcp_state = 11;//Connection closed by server
			vm_tcp_close(tcp_handle);
	    }
		break;
	default:
	    vm_tcp_close(tcp_handle);
		break;
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
				 ProgramState = TCPIP;
				 break;
			 case VM_KEY_RIGHT_SOFTKEY:
				 exit_tcp();
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

void key_tcp_state(VMINT event,VMINT keycode)
{
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{
             switch(keycode)
			 {			 
			 case VM_KEY_RIGHT_SOFTKEY:
				 ProgramState = LOGO;
				 tcp_state = -2;
				 break;
			 case VM_KEY_NUM1:
				 tcp_state = -1;
                 tcp_test();				 
                 break;
			 case VM_KEY_NUM2:
				 if(tcp_state_back_value==1 || tcp_state_back_value==3)
				 {
                      tcp_callback(0,2);
				 }				 
				 break;
			 case VM_KEY_NUM3:
				 if(tcp_handle>=0)
				 {
                     vm_tcp_close(tcp_handle);
					 tcp_state = 7;//Close connection success
				 }
				 else
				 {
                     tcp_state = 6;//Connection is not exist
				 }
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
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_WHITE,VM_COLOR_BLACK);
	vm_ascii_to_ucs2(wstr,40,"TCP TEST");
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,
		(SCREEN_HEIGHT-vm_graphic_get_character_height())/2,wstr,wstrlen(wstr),VM_COLOR_WHITE);
	vm_ascii_to_ucs2(wstr,40,"Begin");
	vm_graphic_textout(layer_buffer,2,SCREEN_HEIGHT-18,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_ascii_to_ucs2(wstr,40,"Exit");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-34,SCREEN_HEIGHT-18,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_graphic_flush_layer(layer,1);
}

void draw_tcp_state(void)
{
    VMINT i = 0; 
	VMWCHAR wstr[60];
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_WHITE,VM_COLOR_BLACK);
	for(i=0;i<4;i++)
	{
		vm_ascii_to_ucs2(wstr,120,TCP_INFO[i]);
		vm_graphic_textout(layer_buffer,12,20+FONT_HEIGHT*i,wstr,wstrlen(wstr),VM_COLOR_RED);
	}
	vm_ascii_to_ucs2(wstr,120,TCP_INFO[4]);
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-110)/2,2,wstr,wstrlen(wstr),VM_COLOR_BLUE);
    vm_graphic_fillrect(10,100,SCREEN_WIDTH-20,SCREEN_HEIGHT-110,VM_COLOR_888_TO_565(112,112,225));
	vm_ascii_to_ucs2(wstr,120,tcp_state_info[tcp_state]);
    vm_graphic_textout(layer_buffer,12,130,wstr,wstrlen(wstr),VM_COLOR_GREEN);    
	if(tcp_state == 4)
	{
		vm_ascii_to_ucs2(wstr,120,tcp_read);
        vm_graphic_textout(layer_buffer,12,140,wstr,wstrlen(wstr),VM_COLOR_BLUE);    
	}
	if (tcp_state == -1) 
	{
		vm_ascii_to_ucs2(wstr,120,tcp_read);
        vm_graphic_textout(layer_buffer,12,100,wstr,wstrlen(wstr),VM_COLOR_GREEN);   
	}
	vm_ascii_to_ucs2(wstr,120,"return");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr)-10,SCREEN_HEIGHT-28,wstr,wstrlen(wstr),VM_COLOR_WHITE);
	vm_graphic_flush_layer(layer,1);
} 

/**定时器的回调函数**/
void timer_proc(VMINT tid)
{
	switch(ProgramState)
	{
	case LOGO:
		draw_logo();
		break;
	case TCPIP:
		draw_tcp_state();
		break;
	default:
		break;
	}
	vm_log_fatal("%d",tcp_state);
}

void Init_Log(void)
{
	 VMINT8 logfile[30];
	 VMINT drv;

	 if ((drv = vm_get_removable_driver()) < 0)
		 drv = vm_get_system_driver();

	 sprintf(logfile,"%c:\\tcptest.log", drv);
	 vm_log_init(logfile,VM_DEBUG_LEVEL);
     vm_log_fatal("Start Log!");	 
}

void exit_tcp(void)
{
	if(tcp_handle>=0)
	{
        vm_tcp_close(tcp_handle);
	}
    vm_graphic_delete_layer(layer[0]);
	vm_delete_timer(timerid);
    vm_exit_app();
}