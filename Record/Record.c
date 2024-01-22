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


#include "Record.h"

CHARACTER_T record_start, record_pause, record_resume ,record_stop, record_exit, record_return;

APP_INFO_T app_all;

static void draw_Start(void);

static void draw_Recording(void);

static void draw_Pause(void);

static void draw_Exit(void);

/* entry */
void vm_main(void) 
{
	VMCHAR drv;
	VMCHAR path[50]={0};

	if ((drv = vm_get_removable_driver()) < 0)
		drv = vm_get_system_driver();

	sprintf(path,"%c:\\record_log.txt",drv);
	vm_log_init(path,VM_DEBUG_LEVEL);
	vm_log_fatal("start_log");

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
		App_init();		
		break;
	case VM_MSG_PAINT:
		/* cerate base layer that has same size as the screen*/
		app_all.layer_hdl[0] = vm_graphic_create_layer(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height(), -1);	
			
			/* set clip area*/
		vm_graphic_set_clip(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height());
		
		app_all.timer_id = vm_create_timer(100, App_Creat_Timer_CallBack);
		
		draw_Start();
		break;
	case VM_MSG_HIDE:
		vm_delete_timer(app_all.timer_id);
		vm_graphic_delete_layer(app_all.layer_hdl[0]);
		break;
	case VM_MSG_QUIT:
		vm_delete_timer(app_all.timer_id);
		vm_graphic_delete_layer(app_all.layer_hdl[0]);
		vm_log_close();
		vm_exit_app();
		break;
	}
#else	
	
	switch (message) {
	case VM_MSG_CREATE:
		App_init();
	case VM_MSG_ACTIVE:
		/*cerate base layer that has same size as the screen*/
		app_all.layer_hdl[0] = vm_graphic_create_layer(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);	
			
			/* set clip area*/
		vm_graphic_set_clip(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height());
				
		break;
		
	case VM_MSG_PAINT:
		app_all.timer_id = vm_create_timer(100, App_Creat_Timer_CallBack);
		draw_Start();
		break;
		
	case VM_MSG_INACTIVE:
		vm_delete_timer(app_all.timer_id);
		vm_graphic_delete_layer(app_all.layer_hdl[0]);
		break;
	case VM_MSG_QUIT:
		vm_delete_timer(app_all.timer_id);
		vm_graphic_delete_layer(app_all.layer_hdl[0]);
		vm_log_close();
		vm_exit_app();
		break;	
	}
#endif
}

void handle_keyevt(VMINT event, VMINT keycode) 
{
	
	if (VM_KEY_EVENT_UP ==event )
	{
		switch(keycode)
		{
		case VM_KEY_NUM1:
		case VM_KEY_OK:
		case VM_KEY_LEFT_SOFTKEY:
			if ( STATE_MENU != app_all.app_state)
			{
				return ;
			}
			else
			{
				app_all.app_state = STATE_RECORDING;
			}
			
			break;
			
		case VM_KEY_NUM2:
			if (STATE_RECORDING == app_all.app_state || STATE_RECORD_RESUME == app_all.app_state )
			{
				app_all.app_state = STATE_RECORD_PAUSE;
			}
			break;
			
		case VM_KEY_NUM3:
			if (STATE_RECORD_PAUSE == app_all.app_state )
			{
				app_all.app_state = STATE_RECORD_RESUME;
			}
			break;
			
		case VM_KEY_NUM4:
			if ( STATE_RECORDING  == app_all.app_state || STATE_RECORD_PAUSE == app_all.app_state ||STATE_RECORD_RESUME == app_all.app_state)
			{
				app_all.app_state = STATE_RECORD_STOP;
			}			
			break;
			
		case VM_KEY_NUM5:
		case VM_KEY_RIGHT_SOFTKEY:
			app_all.app_state = STATE_RECORD_EXIT;
			handle_sysevt(VM_MSG_QUIT,1);
			break;
		default:
			break;
		}
		
		App_Record();
	}
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{
	if (VM_PEN_EVENT_RELEASE == event )
	{
		switch(app_all.app_state)
		{
		case STATE_MENU:
			{
				
				if (App_IsInArea(x, y, record_start.x, record_start.y, record_start.width, record_start.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM1);
				}
				if (App_IsInArea(x, y, record_exit.x, record_exit.y, record_exit.width, record_exit.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM5);
				}
			}
			break;
		case STATE_RECORDING:
			{
				
				if (App_IsInArea(x, y, record_pause.x, record_pause.y, record_pause.width, record_pause.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM2);
				}
				if (App_IsInArea(x, y, record_stop.x, record_stop.y, record_stop.width, record_stop.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM4);
				}
				if (App_IsInArea(x, y, record_exit.x, record_exit.y, record_exit.width, record_exit.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM5);
				}
			}
			break;
		case STATE_RECORD_PAUSE:
			{
				
				if (App_IsInArea(x, y, record_resume.x, record_resume.y, record_resume.width, record_resume.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM3);
				}
				if (App_IsInArea(x, y, record_stop.x, record_stop.y, record_stop.width, record_stop.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM4);
				}
				if (App_IsInArea(x, y, record_exit.x, record_exit.y, record_exit.width, record_exit.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM5);
				}
			}
			
			break;
		case STATE_RECORD_RESUME:
			{

				if (App_IsInArea(x, y, record_pause.x, record_pause.y, record_pause.width, record_pause.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM2);
				}
				if (App_IsInArea(x, y, record_stop.x, record_stop.y, record_stop.width, record_stop.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM4);
				}
				if (App_IsInArea(x, y, record_exit.x, record_exit.y, record_exit.width, record_exit.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM5);
				}
			}
			break;
		case STATE_RECORD_STOP:
		case STATE_RECORD_EXIT:
			
			{				
				if (App_IsInArea(x, y, record_exit.x, record_exit.y, record_exit.width, record_exit.height))
				{
					handle_keyevt(VM_KEY_EVENT_UP , VM_KEY_NUM5);
				}
				
			}
			break;
			
		default:
			break;
		}
		
	}
}

static void draw_Start(void) {
	VMWCHAR s[50];
	
	int x, y, w;
	
	vm_ascii_to_ucs2(s, 100, "record menu");
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height()) / 4;
	
	/* get the target buffer*/
	app_all.buffer_base = vm_graphic_get_layer_buffer(app_all.layer_hdl[0]);		
	
	/* fill the screen*/
	vm_graphic_fill_rect(app_all.buffer_base, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_GREEN, VM_COLOR_BLACK);
	
	/* draw text */
	vm_graphic_textout(app_all.buffer_base,	x, y, s, wstrlen(s), VM_COLOR_BLUE);
	
	vm_graphic_textout(app_all.buffer_base,	record_start.x,  record_start.y,  record_start.Wstr,  record_start.width,  VM_COLOR_RED);
	
	vm_graphic_textout(app_all.buffer_base,	record_exit.x,   record_exit.y,   record_exit.Wstr,   record_exit.width,   VM_COLOR_BLUE);
	
}





static void draw_Recording(void)
{
	VMWCHAR s[50];	
	int x, y, w;
	
	vm_ascii_to_ucs2(s, 100, "record menu");
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height()) / 4;
	
	/* get the target buffer*/
	app_all.buffer_base = vm_graphic_get_layer_buffer(app_all.layer_hdl[0]);		
	
	/* fill the screen*/
	vm_graphic_fill_rect(app_all.buffer_base, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_GREEN, VM_COLOR_BLACK);
	
	/* draw text */
	vm_graphic_textout(app_all.buffer_base,	x, y, s, wstrlen(s), VM_COLOR_BLUE);
	
	vm_ascii_to_ucs2(s, 100, "recording ....");
	
	vm_graphic_textout(app_all.buffer_base,	x, y+30, s, wstrlen(s), VM_COLOR_GREEN);
	
	
	vm_graphic_textout(app_all.buffer_base,	record_pause.x,  record_pause.y,  record_pause.Wstr,  wstrlen(record_pause.Wstr),  VM_COLOR_BLUE);
	
	
	vm_graphic_textout(app_all.buffer_base,	record_stop.x,   record_stop.y,   record_stop.Wstr,   wstrlen(record_stop.Wstr),   VM_COLOR_BLUE);
	
	
	vm_graphic_textout(app_all.buffer_base,	record_exit.x,   record_exit.y,   record_exit.Wstr,   wstrlen(record_exit.Wstr),   VM_COLOR_BLUE);
}


static void draw_Pause(void)
{
	VMWCHAR s[50];
	
	int x, y, w;
	
	vm_ascii_to_ucs2(s, 100, "record menu");
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height()) / 4;
	
	/* get the target buffer*/
	app_all.buffer_base = vm_graphic_get_layer_buffer(app_all.layer_hdl[0]);		
	
	/* fill the screen*/
	vm_graphic_fill_rect(app_all.buffer_base, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_GREEN, VM_COLOR_BLACK);
	
	/* draw text */
	vm_graphic_textout(app_all.buffer_base,	x, y, s, wstrlen(s), VM_COLOR_BLUE);
	vm_ascii_to_ucs2(s, 100, "record pause...");
	vm_graphic_textout(app_all.buffer_base,	x, y+30, s, wstrlen(s), VM_COLOR_RED);
	
	vm_graphic_textout(app_all.buffer_base,	record_resume.x, record_resume.y, record_resume.Wstr, wstrlen(record_pause.Wstr), VM_COLOR_BLUE);
	
	vm_graphic_textout(app_all.buffer_base,	record_stop.x,   record_stop.y,   record_stop.Wstr,   wstrlen(record_stop.Wstr),   VM_COLOR_BLUE);
	
	vm_graphic_textout(app_all.buffer_base,	record_exit.x,   record_exit.y,   record_exit.Wstr,   wstrlen(record_exit.Wstr),   VM_COLOR_BLUE);
	
}

static void draw_Exit(void)
{
	VMWCHAR s[50];	
	int x, y, w;
	
	vm_ascii_to_ucs2(s, 100, "record menu");
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height()) / 4;
	
	/* get the target buffer*/
	app_all.buffer_base = vm_graphic_get_layer_buffer(app_all.layer_hdl[0]);		
	
	/* fill the screen*/
	vm_graphic_fill_rect(app_all.buffer_base, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_GREEN, VM_COLOR_BLACK);
	
	/* draw text */
	vm_graphic_textout(app_all.buffer_base,	x, y, s, wstrlen(s), VM_COLOR_BLUE);
	vm_ascii_to_ucs2(s, 100, "record stop");
	vm_graphic_textout(app_all.buffer_base,	x, y+30, s, wstrlen(s), VM_COLOR_RED);
	
	vm_graphic_textout(app_all.buffer_base,	record_exit.x,   record_exit.y,   record_exit.Wstr,   wstrlen(record_exit.Wstr),   VM_COLOR_BLUE);
}



VMINT App_IsInArea(VMINT x,VMINT y,VMINT rectangleX,VMINT rectangleY,VMINT rectangleWidth,VMINT rectangleHeight)
{
	if(x>=rectangleX&&x<=(rectangleX+rectangleWidth) && y>=rectangleY&&y<=(rectangleY+rectangleHeight))
		return 1;
	else
		return 0;
}



void App_init()
{
	VMINT y_distance				= 10;
	app_all.timer_id				= 10;
	app_all.app_state				= STATE_MENU; 
	
    app_all.screen_width		= vm_graphic_get_screen_width();
	app_all.screen_height			= vm_graphic_get_screen_height();
	app_all.character_height		= vm_graphic_get_character_height();
	
	vm_ascii_to_ucs2(record_start.Wstr, 100,"NUM1:record start");
	record_start.height		= app_all.character_height;
	record_start.width		= vm_graphic_get_string_width(record_start.Wstr);
	record_start.x			= (app_all.screen_width  - record_start.width ) >>1 ;
	record_start.y			= ((app_all.screen_height ) >>1 )  - record_start.height;
	
	
	vm_ascii_to_ucs2(record_pause.Wstr, 100,"NUM2:record pause");
	record_pause.height		= app_all.character_height;
	record_pause.width		= vm_graphic_get_string_width(record_pause.Wstr);
	record_pause.x			= (app_all.screen_width  - record_pause.width ) >> 1;
	record_pause.y			= record_start.y + app_all.character_height + y_distance ;
	
	
	vm_ascii_to_ucs2(record_resume.Wstr, 100,"NUM3:record resume");
	record_resume.height	= app_all.character_height;
	record_resume.width		= vm_graphic_get_string_width(record_resume.Wstr);
	record_resume.x			= (app_all.screen_width  - record_resume.width ) >> 1;
	record_resume.y			= record_pause.y + app_all.character_height + y_distance ;
	
	
	vm_ascii_to_ucs2(record_stop.Wstr, 100,"NUM4:record stop");
	record_stop.height		= app_all.character_height;
	record_stop.width		= vm_graphic_get_string_width(record_stop.Wstr);
	record_stop.x			= (app_all.screen_width  - record_stop.width ) >> 1;
	record_stop.y			= record_resume.y + app_all.character_height + y_distance;
	
	
	
	vm_ascii_to_ucs2(record_exit.Wstr, 100,"NUM5:exit");
	record_exit.height		= app_all.character_height;
	record_exit.width		= vm_graphic_get_string_width(record_exit.Wstr);
	record_exit.x			= app_all.screen_width  - record_exit.width  - 3;
	record_exit.y			= app_all.screen_height - record_exit.height - 5;
	
	vm_ascii_to_ucs2(record_return.Wstr, 100,"NUM6:return");
	record_return.height		= app_all.character_height;
	record_return.width		= vm_graphic_get_string_width(record_return.Wstr);
	record_return.x			= app_all.screen_width  - record_return.width  - 3;
	record_return.y			= app_all.screen_height - record_return.height - 5;
	
	return ;
	
	
}



void App_Creat_Timer_CallBack(VMINT tid)
{
	switch(app_all.app_state)
	{
	case  STATE_MENU:
		draw_Start();
		break;
	case  STATE_RECORDING:
		draw_Recording();
		break;
	case  STATE_RECORD_PAUSE:
		draw_Pause();
		break;
	case  STATE_RECORD_STOP:
		draw_Exit();
		break;
	case  STATE_RECORD_RESUME:
		draw_Recording();
		break;
		
	case  STATE_RECORD_EXIT:
		draw_Exit();
		break;
	default:
		break;
	}
	/* flush the screen with data in the buffer*/
	vm_graphic_flush_layer(app_all.layer_hdl, 1);
	
}




void App_Sleep(VMINT timer)
{
	VMINT timer_start=0;	
	timer_start = vm_get_tick_count();	
	while(vm_get_tick_count() -timer_start != timer*1000) 
	{
		;
	}	
}


void App_Record_CallBack(VMINT result)
{
	VMINT a=-10;
}

VMINT App_Record()
{
	VMINT res =-10;	

	switch(app_all.app_state)
	{
		
	case STATE_RECORDING:
		{						
			VMCHAR  drv[2] = {0};
			VMCHAR  dir[100] = {0};
			VMCHAR  filename[10] = {0};
			VMWCHAR	wpath[120] = {0};			
			

			sprintf(drv, "%c", (vm_get_removable_driver() < 0 ? vm_get_system_driver() : vm_get_removable_driver()));
			sprintf(dir, "record1");		/*dir*/
			sprintf(filename, "record2");	/*file name*/

			vm_log_fatal("STATE_RECORDING , res = %d ",res);
			res=vm_record_start(drv, dir, filename, VM_FORMAT_AMR, wpath, App_Record_CallBack);
			vm_log_fatal("STATE_RECORDING , res = %d ",res);				
		}
		
		break;
		
	case STATE_RECORD_PAUSE:
		res=vm_record_pause();
		break;
		
	case STATE_RECORD_RESUME:
		res=vm_record_resume();
		
		break;
		
	case STATE_RECORD_STOP:
	case STATE_RECORD_EXIT:

		res=vm_record_stop();
		

		break;
		
	default:
		break;
		
	}
	return res;
}
