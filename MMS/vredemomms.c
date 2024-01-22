/* ============================================================================
 * VRE demonstration application.
 *
 * Copyright (c) 2006-2009 Vogins Network Technology (China). All rights 
 * reserved.
 *
 * YOU MAY MODIFY OR DISTRIBUTE SOURCE CODE BELOW FOR PERSONAL OR 
 * COMMERCIAL PURPOSE UNDER THOSE AGGREMENTS:
 *
 * 1) VOGINS COPYRIGHT MUST BE KEPT IN REDISTRIBUTED SOURCE CODE OR
 *    BINARY FILE.
 * 2) YOU CAN REUSE THOSE SOURCE CODE IN COMMERCIAL PURPOSE WITHOUT
 *    VOGINS COMFIRMATION.
 * 3) THIS PROGRAM JUST USE TO DEMONSTRATE HOW TO WRITE A VRE BASED
 *    APPLICATION. VOGINS WON'T MAKE ANY GUARANTEE THAT IT COULD BE
 *    USED IN PRODUCTION ENVIRONMENT.
 * ========================================================================= */

/*============================================================================
 * GDI Sample show the VRE usage of GDI interface
 *
 * Copyright (c) 2006 - 2009 Vogins Network Technology (China). All rights reserved.
 * By Steven Fang Feb. 1, 2009 V1.0
 *============================================================================*/

#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmlog.h"
#include "vredemomms.h"
#include "vmmms.h"
#include "functions.h"
/* ---------------------------------------------------------------------------
 * global variables
 * ------------------------------------------------------------------------ */
sms_state app_mms_state = MAMU;
VMINT		layer_hdl[1];				////layer handle array.
VMUINT8* screenbuf = NULL;	
VMINT ncharacter_height = -1;
VMINT nscreen_width = -1;
VMINT nscreen_height = -1;

vm_send_mms_req_t * req = NULL;
/* ---------------------------------------------------------------------------
 * local variables
 * ------------------------------------------------------------------------ */
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
 * demo
 */
static void draw_hello(void);

/**
 * entry
 */
void vm_main(void) {
	VMCHAR path[50];
	VMINT dir;
	layer_hdl[0] = -1;
	dir = vm_get_removable_driver();
	if (dir<0)
	{
		dir = vm_get_system_driver();
	}
	
	sprintf(path,"%c:\\vre\\vredebug.log",dir);// log path
	vm_log_init(path,VM_DEBUG_LEVEL);
	vm_log_error("event start!");
	
	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
}


void handle_sysevt(VMINT message, VMINT param) {
#ifdef		SUPPORT_BG	
	/* The application updates the screen when receiving the message VM_MSG_PAINT 
	*  what is sent after the application is activated. The application can skip 
	*  the process on screen when the VM_MSG_ACTIVE or VM_MSG_INACTIVE is received.
	*/
	switch (message) {
		case VM_MSG_CREATE:
			/* the GDI operation is not recommended as the response of the message*/
			break;
		case VM_MSG_PAINT:
			/* cerate base layer that has same size as the screen*/
			layer_hdl[0] = vm_graphic_create_layer(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height(), -1);	
			
			/* set clip area */
			vm_graphic_set_clip(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height());

			draw_hello();
			break;
		case VM_MSG_HIDE:
		case VM_MSG_QUIT:
			if( layer_hdl[0] != -1 )
			{
				vm_graphic_delete_layer(layer_hdl[0]);
				layer_hdl[0] = -1;
			}

			break;
	}
#else
	switch (message) {
		case VM_MSG_CREATE:
		case VM_MSG_ACTIVE:
			app_start();
			/*cerate base layer that has same size as the screen*/
			layer_hdl[0] = vm_graphic_create_layer(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height(), -1);	
			/* set clip area*/
			vm_graphic_set_clip(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height());
			break;

		case VM_MSG_PAINT:
			screenbuf = vm_graphic_get_layer_buffer(layer_hdl[0]);
			draw();
			break;

		case VM_MSG_INACTIVE:
		case VM_MSG_QUIT:
			if( layer_hdl[0] != -1 )
				vm_graphic_delete_layer(layer_hdl[0]);
			layer_hdl[0] = -1;

			break;	
	}
#endif
}

void handle_keyevt(VMINT event, VMINT keycode) {
	
	if (VM_KEY_EVENT_UP == event)
	{
		if (app_mms_state == MAMU)
		{
			switch(keycode)
			{
			case VM_KEY_RIGHT_SOFTKEY:
				
				if( layer_hdl[0] != -1 )
					vm_graphic_delete_layer(layer_hdl[0]);
				app_quit();
				return;
			case VM_KEY_LEFT_SOFTKEY:
				vm_mms_send_smil();
				break;
			}
		}else 
		{
			;
		}
		
		draw();
	}
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{
	/* touch and return*/
	if( layer_hdl[0] != -1 )
	{
		vm_graphic_delete_layer(layer_hdl[0]);
		layer_hdl[0] = -1;
	}

	vm_exit_app();
}


static void draw_hello(void) {
	VMWCHAR s[50];
	VMUINT8* buf;	
	int x, y, w;
	
	vm_ascii_to_ucs2(s, 100, "Hello, world!");
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height()) / 2;
	
	/* get the target buffer*/
	buf = vm_graphic_get_layer_buffer(layer_hdl[0]);		
	
	/* fill the screen*/
	vm_graphic_fill_rect(buf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_WHITE, VM_COLOR_BLACK);
	
	/* draw text */
	vm_graphic_textout(buf,	x, y, s, wstrlen(s), VM_COLOR_BLUE);
	
	/* flush the screen with data in the buffer*/
	vm_graphic_flush_layer(layer_hdl, 1);
}

void draw()
{
	vm_graphic_fill_rect(screenbuf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_BLACK, VM_COLOR_BLACK);
	switch(app_mms_state)
	{
	case MAMU:
		draw_text_muiltline(screenbuf,"send mms",0,nscreen_height-ncharacter_height,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"exit",nscreen_width-70,nscreen_height-ncharacter_height,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
		//////////////////////////////////////////////////////////////////////////////////

	case SEND_MMS_ERROR:
		draw_text_muiltline(screenbuf,"vm_mms_send_mms  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case SEND_MMS_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_mms_send_mms success,please wait",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case SEND_MMS_CB_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_mms_send_mms cb success",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case SEND_MMS_CB_ERROR:
		draw_text_muiltline(screenbuf,"vm_mms_send_mms cb error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	}
	vm_graphic_flush_layer(layer_hdl, 1);
}
void app_start()
{
	ncharacter_height = vm_graphic_get_character_height();
	nscreen_width = vm_graphic_get_screen_width();
	nscreen_height = vm_graphic_get_screen_height();
}
void app_quit()
{
	if( layer_hdl[0] != -1 )
	vm_graphic_delete_layer(layer_hdl[0]);
	vm_exit_app();
}
static void vm_mms_send_mix_callback(VMUINT  result)
{
	VMCHAR s[100];
	

	vm_log_debug("mix result=%d",result);
    switch(result)
    {
	case VM_MMS_FAIL:
		vm_log_debug("VM_MMS_FAIL");

		app_mms_state = SEND_MMS_CB_ERROR;
		break;
	case VM_MMS_MEMORY_NOT_ENOUGH:
		vm_log_debug("VM_MMS_MEMORY_NOT_ENOUGH");
		app_mms_state = SEND_MMS_CB_ERROR;

		break;
	case VM_MMS_SUCCESS:
		app_mms_state = SEND_MMS_CB_SUCCESS;
		break;
	default:
		app_mms_state = SEND_MMS_CB_ERROR;

		if(result >0)		
		{
			;
		}
		else
		{
			vm_log_debug("mix error");
		//	vm_mms_show_info("mixÎ´Öª´íÎó");
		}
    }
	draw();
}

static VMINT vm_mms_send_mix(void)
{
	VMINT driver;
	VMCHAR s[200];
	VMWCHAR filename[100];
	VMFILE file;
	VMUINT size;
	VMINT res;

	
	vm_log_debug("vm_mms_send_mix start");
	driver = vm_get_removable_driver();
	if(driver < 0)
		driver = vm_get_system_driver();
	
	sprintf(s, "%c:\\vre\\mix\\mms_mix.xml", driver);
	vm_log_debug("s=%s", s);
	vm_ascii_to_ucs2(filename, 200, s);
	
	file = vm_file_open(filename, MODE_READ, FALSE);
	if(file <0)
	{
		vm_log_debug("vm_mms_send_mix:file <0");

		return -1;
	}
	vm_file_close(file);

	vm_file_getfilesize(file, &size);
	vm_log_debug("vm_mms_send_mix:size = %d", size);
	req = vm_calloc(sizeof(vm_send_mms_req_t));
	if(req == NULL)
	{
		vm_log_debug("vm_mms_send_mix:req == NULL");
		return -1;
	}
	req ->mms_type = VM_MMS_TYPE_MIXED_MMS;
	req->sim_id  = VM_MMS_SIM_ID_SIM1;
//	vm_log_debug("vm_mms_send_mix:s_vm_mms_sim_id = %d",s_vm_mms_sim_id);
	vm_wstrncpy((VMWSTR)req->xml_filepath, (VMWSTR)filename, MAX_APP_NAME_LEN);
	req->xml_size = size;
	
	vm_log_debug("vm_mms_send_mix:vm_send_mms");
	vm_log_debug("vm_mms_send_mms:req = %x,sizeof(vm_send_mms_req_t) = %d",req,sizeof(vm_send_mms_req_t));
	vm_log_debug("vm_mms_send_mix:req->sim_id = %d",req->sim_id);
	res=vm_mms_send_mms(req, (vm_mms_callback_type)vm_mms_send_mix_callback);
	
	//vm_mms_show_info(s);
	vm_log_error("res = %d",res);
	if (TRUE == res )
	{
		app_mms_state = SEND_MMS_SUCCESS;
	}
	else 
	{
		app_mms_state = SEND_MMS_ERROR;
		vm_log_error("vm_mms_send_mix  error res=%d", res); 
		
	//	vm_mms_show_info(s);
	}
	vm_free(req);
	
	
	vm_log_debug("vm_mms_send_mix end"); 
	return 0;
}
static void vm_mms_send_smil_callback(VMUINT  result)
{
	VMCHAR s[100];

	
	vm_log_debug("mix result=%d",result);
    switch(result)
    {
	case VM_MMS_FAIL:
		vm_log_debug("VM_MMS_FAIL");
		
		app_mms_state = SEND_MMS_CB_ERROR;
		break;
	case VM_MMS_MEMORY_NOT_ENOUGH:
		vm_log_debug("VM_MMS_MEMORY_NOT_ENOUGH");
		app_mms_state = SEND_MMS_CB_ERROR;
		
		break;
	case VM_MMS_SUCCESS:
		app_mms_state = SEND_MMS_CB_SUCCESS;
		break;
	default:
		app_mms_state = SEND_MMS_CB_ERROR;
		
		if(result >0)		
		{
			;
		}
		else
		{
			vm_log_debug("mix error");
			//	vm_mms_show_info("mixÎ´Öª´íÎó");
		}
    }
	draw();
}

static VMINT vm_mms_send_smil(void)
{
   	VMINT driver;
	VMCHAR s[100];
	VMWCHAR filename[50];
	VMFILE file;
	VMUINT size;
	VMINT res;
	vm_send_mms_req_t * req;
	
	vm_log_debug("vm_mms_send_smil start");
	driver = vm_get_removable_driver();
	if(driver < 0)
		driver = vm_get_system_driver();
	
	sprintf(s, "%c:\\vre\\smil\\mms_smil.xml", driver);
	vm_log_debug("s=%s", s);
	vm_ascii_to_ucs2(filename, 100, s);
	
	file = vm_file_open(filename, MODE_READ, FALSE);
	vm_log_debug("vm_mms_send_smil:file = %d",file);
	if(file <0)
	{
		vm_log_debug("vm_mms_send_smil:file <0");
		return -1;
	}
	vm_file_getfilesize(file, &size);
	vm_file_close(file);
	vm_log_debug("vm_mms_send_smil:size = %d", size);
	req = vm_calloc(sizeof(vm_send_mms_req_t));
	if(req == NULL)
	{
		vm_log_debug("vm_mms_send_smil:req == NULL");
		return -1;
	}
	req ->mms_type = VM_MMS_TYPE_SMIL_MMS;
	req->sim_id  = VM_MMS_SIM_ID_SIM1;
	vm_wstrncpy((VMWSTR)req->xml_filepath, (VMWSTR)filename, MAX_APP_NAME_LEN);
	req->xml_size = size;
	res=vm_mms_send_mms(req, (vm_mms_callback_type)vm_mms_send_smil_callback);

	if (TRUE == res )
	{
		app_mms_state = SEND_MMS_SUCCESS;
	}
	else 
	{
		app_mms_state = SEND_MMS_ERROR;
		vm_log_error("vm_mms_send_mix  error res=%d", res); 
		
		//	vm_mms_show_info(s);
	}
	vm_free(req);
	
	vm_log_debug("vm_mms_send_smil end"); 
}

