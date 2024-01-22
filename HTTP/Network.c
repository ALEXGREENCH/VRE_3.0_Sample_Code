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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmres.h"
#include "vmstdlib.h"
#include "vmlog.h"
#include "Network.h"

typedef enum {
	MAIN_MENU,
		TCP_CONNECT,
		HTTP_CONNECT
}DEMO_STATE;

VMINT		app_state;
VMINT		need_update = FALSE;
VMINT		layer_hdl = -1;	
VMUINT8*	layer_buf = NULL;
VMINT		timer = -1;	
VMCHAR*		app_text = NULL;	
VMINT		count = 0;

/*---------------------------------------------------------------- */
/* System event */ 
void handle_sysevt(VMINT message, VMINT param);

/* Key event */ 
void handle_keyevt(VMINT event, VMINT keycode);

/* Pen event */ 
void handle_penevt(VMINT event, VMINT x, VMINT y);

/* Initialization */
void init(void);
/* Draw view */ 
void draw(void);
/* Paused */ 
void pause(void);
/* Resume */
void resume(void);
/* Quit */ 
void quit(void);

void run(VMINT tid);
/* Draw menu view */ 
static void draw_menu(void);
/* Draw TCP view */
static void draw_tcp(void);
/* Draw HTTP view */ 
static void draw_download(void);
/* Draw multi-line text */ 
static void draw_text_muiltline(VMWCHAR *msg, VMUINT8* buf, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color);
/* Draw Menu bar */
static void draw_menu_bar(VMSTR left_str, VMSTR right_str);
/* Draw HTTP download progress */ 
static void draw_grid(VMWSTR file_name);

/**
 * Program Entry
 */
void vm_main(void) {
	VMINT8 logfile[50];
	VMINT drv;

	drv = vm_get_removable_driver();
	if (drv < 0)
	{
		drv = vm_get_system_driver();
	}
	sprintf(logfile,"%c:\\LogTest.log",drv);
	vm_log_init(logfile,VM_DEBUG_LEVEL);

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
}

void handle_sysevt(VMINT message, VMINT param) {	
	switch (message) 
	{
	/* when the program starts, the system will send this message */
	case VM_MSG_CREATE:
		init();
		break;
	/* this message always comes after VM_MSG_CREATE or VM_MSG_PAINT message */
	case VM_MSG_ACTIVE:
		resume();			
		break;
	/* if there is an external event (incoming calls or SMS) occurs, the system will send this message */
	case VM_MSG_PAINT:
		draw();
		break;
	/*  after the processing of external events completed, the system will send this message */
	case VM_MSG_INACTIVE:
		pause();
		break;
	/* if you press hang-up button, the system will send this message */
	case VM_MSG_QUIT:
		quit();	
		break;	
	}
}

void handle_keyevt(VMINT event, VMINT keycode) 
{
	if(VM_KEY_EVENT_UP == event)
	{
		if(MAIN_MENU == app_state)					/* Main Menu */
		{
			/* If press the left soft key, it will start download a picture */ 
			if(VM_KEY_LEFT_SOFTKEY == keycode)			
			{
				app_state = HTTP_CONNECT;
				need_update = TRUE;
				memset(app_text, 0, 1024);
				StartHttpDownload();
			}
			/* If press the right soft key, it will setup TCP link */
			else if(VM_KEY_RIGHT_SOFTKEY == keycode)	
			{
				app_state = TCP_CONNECT;
				need_update = TRUE;
				memset(app_text, 0, 1024);
				StartTcpDemo();
			}
			else										/* else quit*/
			{
				quit();
			}
		}
		else if(HTTP_CONNECT == app_state)				/* HTTP view*/
		{
			/* If press the right soft key, it will return to the main menu*/
			if(VM_KEY_RIGHT_SOFTKEY == keycode)			
			{
				if(getHttpHandle() >= 0)
				{
					DropAllHttpConnection();
					memset(app_text, 0, 1024);
				}
				app_state = MAIN_MENU;
				need_update = TRUE;
			}
			/* If press the left soft key, it will pause or resume the download */
			if(VM_KEY_LEFT_SOFTKEY == keycode)			
			{
				if(getHttpHandle() >= 0)
				{					
					DropAllHttpConnection();
					memset(app_text, 0, 1024);
				}
				else
				{					
					StartHttpDownload();					
				}
				handle_sysevt(VM_MSG_PAINT, 0);
			}
		}
		else if(TCP_CONNECT == app_state)				/* TCP view */
		{
			/* If press the right soft key, it will return to the main menu*/
			if(VM_KEY_RIGHT_SOFTKEY == keycode)			
			{
				StopTcpDemo();				
				app_state = MAIN_MENU;
				need_update = TRUE;
			}
		}
	}

	
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{
  // to do 
}

void init(void)
{
	 /*Create Layer*/
	if(0 > layer_hdl)
	{
		layer_hdl = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);
		if(0 > layer_hdl)
		{
			quit();
		}
		
		/*Get Buffer*/
		if ((layer_buf = vm_graphic_get_layer_buffer(layer_hdl)) == NULL)
		{
			quit();
		}

		vm_graphic_set_clip(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	}	
	
	app_state = MAIN_MENU;
	need_update = TRUE;

	app_text = vm_calloc(1024);
		
	if(timer < 0)
	{
		timer = vm_create_timer(TIMEOUT_TIME, run);
	}	
}

void draw(void)
{
	switch(app_state)
	{
	case MAIN_MENU:
		draw_menu();
		break;
	case TCP_CONNECT:		
		draw_tcp();
		break;
	case HTTP_CONNECT:		
		draw_download();
		break;	
	default:
		break;
	}
}

void pause(void)
{
	/*Delete Timer*/
	if(timer >= 0)
	{
		vm_delete_timer(timer);
		timer = -1;
	}	
	
	/*Delete Layer*/
	if(layer_hdl >= 0)
	{		
		vm_graphic_delete_layer(layer_hdl);
		layer_hdl = -1;
	}
	
	/*Delete buffer*/ 
	if(layer_buf)
	{
		layer_buf = NULL;
	}	
}

void resume(void)
{
	/*Create layer*/
	if(0 > layer_hdl)
	{
		layer_hdl = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);
		if(0 > layer_hdl)
		{
			quit();
		}
		
		/*Get Buffer*/
		if ((layer_buf = vm_graphic_get_layer_buffer(layer_hdl)) == NULL)
		{
			quit();
		}
	}	
	
	vm_graphic_set_clip(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	/*Create Timer*/
	if(timer < 0)
	{
		timer = vm_create_timer(TIMEOUT_TIME, run);
	}	
}

void quit(void)
{
	if(getHttpHandle() >= 0)
	{
		DropAllHttpConnection();
	}

	vm_free(app_text);
	app_text = NULL;

	if(timer >= 0)
	{
		vm_delete_timer(timer);
		timer = -1;
	}
	
	/*  Delete layer */
	if(layer_hdl >= 0)
	{
		vm_graphic_delete_layer(layer_hdl);
		layer_hdl = -1;
	}
	/*  Delete buffer */
	if(layer_buf)
	{
		vm_free(layer_buf);
		layer_buf = NULL;
	}	
	/* Quit Application */
	vm_exit_app();
}

void run(VMINT tid)
{
	if(need_update)
	{
		draw();		
		need_update = FALSE;
	}	
}

static void draw_menu(void)
{
	vm_graphic_fill_rect(layer_buf, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0); 
	draw_menu_bar("HTTP", "TCP");
	
	vm_graphic_flush_layer(&layer_hdl, 1);
}

static void draw_tcp(void)
{
	VMWCHAR w_app_text[1024];
	memset(w_app_text, 0, sizeof(w_app_text));
	vm_ascii_to_ucs2(w_app_text, sizeof(w_app_text), app_text);

	vm_graphic_fill_rect(layer_buf, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0); 

	draw_text_muiltline(w_app_text, layer_buf, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - SOFTKEY_HEIGHT, 0xabcd);

	draw_menu_bar(NULL, "Return"); 
	
	vm_graphic_flush_layer(&layer_hdl, 1);
}


static void draw_download(void)
{
	VMCHAR file_name[50];
	VMCHAR full_name[50];
	VMWCHAR w_full_name[50];
	struct vm_fileinfo_t info;
	VMINT find_hdl;
	VMINT	drv;

	static	VMWCHAR w_app_text[1024];
	memset(w_app_text, 0, sizeof(w_app_text));
	vm_ascii_to_ucs2(w_app_text, sizeof(w_app_text), app_text);
	
	drv= vm_get_removable_driver();		// Get removable drive letter first
	if(drv<0)							// Not exist removable drive
		drv=vm_get_system_driver();		// Get system driver letter
	
	vm_graphic_fill_rect(layer_buf, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0); 
	vm_graphic_line(layer_buf, 0, 125, SCREEN_WIDTH, 125, VM_COLOR_RED);

	sprintf(full_name, "%c:\\vre\\logo_vre.*", drv);
	vm_ascii_to_ucs2(w_full_name, 100, full_name);
	if((find_hdl = vm_find_first(w_full_name, &info)) >= 0)
	{
		vm_ucs2_to_ascii(file_name, 50, info.filename);
		if(!vm_ends_with(file_name, "jpg"))
		{
			VMUINT8* res;
			VMINT canvas_hdl;
			VMUINT8* canvas_buf;
			struct frame_prop* img_prop; 

			sprintf(full_name, "%c:\\vre\\%s", drv, file_name);
			vm_ascii_to_ucs2(w_full_name, 100, full_name);
			res = (VMUINT8*)vm_malloc(info.size);
			if(!Http_ReadFile(w_full_name, 0, info.size, res))
			{
				canvas_hdl = vm_graphic_load_image(res, info.size);
				canvas_buf = vm_graphic_get_canvas_buffer(canvas_hdl);
				img_prop = vm_graphic_get_img_property(canvas_hdl, 1);
				vm_graphic_blt(layer_buf, (SCREEN_WIDTH - img_prop->width) / 2, 3,
					canvas_buf, 0, 0, img_prop->width, img_prop->height, 1);
				vm_graphic_release_canvas(canvas_hdl);
				vm_free(res);
				res = NULL;	
			}			
		}
		else if(!vm_ends_with(file_name, "tmp"))
		{
			sprintf(full_name, "%c:\\vre\\%s", drv, file_name);
			vm_ascii_to_ucs2(w_full_name, 100, full_name);
			draw_grid(w_full_name);
		}

		vm_find_close(find_hdl);
	}	

	draw_text_muiltline(w_app_text, layer_buf, 0, 126, SCREEN_WIDTH, SCREEN_HEIGHT - 126, 0xabcd);
	
	if(getHttpHandle() >= 0)
	{
		draw_menu_bar("Pause", "return");   
	}
	else
	{
		draw_menu_bar("Resume", " return ");  
	}	
	
	vm_graphic_flush_layer(&layer_hdl, 1);
}
static void draw_text_muiltline(VMWCHAR *msg, VMUINT8* buf, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color)
{
	VMWCHAR *tmp;
	VMWCHAR ch;
	VMINT base, count, row, nrow;
	VMINT ch_height;
	VMINT nword = 0;
	VMINT total = wstrlen(msg);
	
	ch_height = vm_graphic_get_character_height() + 2;
	row = base = count = 0;
	nrow = height / ch_height;
	if ((tmp = (vm_malloc(total * 2))) == NULL)
		return;
	memset(tmp, 0, total * 2);
	
	while (nword <= total) {
		ch = msg[base + count];
		if (ch == 0)
			break;
		if (ch == '\n' || (ch == '\r' && msg[base + count + 1] != '\n')) {
			wstrncpy(tmp, msg + base, count);
			vm_graphic_textout(buf, x, y + row * ch_height, tmp, wstrlen(tmp), color);
			tmp[0] = 0;
			base += count;
			count = 0;
			row++;
		}
		if ((vm_graphic_get_string_width(tmp)) <= width - 5) {
			count++;
			nword++;
			wstrncpy(tmp, msg + base, count);
		}
		else {
			count--;
			nword--;
			wstrncpy(tmp, msg + base, count);
			vm_graphic_textout(buf, x, y + row * ch_height, tmp, wstrlen(tmp), color);
			tmp[0] = 0;
			base += count;
			count = 0;
			row++;
		}
	}
	if ((wstrlen(tmp) > 0) && (row < nrow))
	{
		wstrncpy(tmp, msg + base, count);
		vm_graphic_textout(buf, x, y + row * ch_height, tmp, wstrlen(tmp), color);
	}
	
	if (tmp != NULL)
		vm_free(tmp);
}

static void draw_menu_bar(VMSTR left_str, VMSTR right_str)
{
	VMINT text_y;
	VMWCHAR wstr[50];

	/* Draw menu bar*/
	vm_graphic_fill_rect(layer_buf, 0, SCREEN_HEIGHT-SOFTKEY_HEIGHT,
							SCREEN_WIDTH, SOFTKEY_HEIGHT, VM_COLOR_WHITE, VM_COLOR_WHITE);
	text_y = SCREEN_HEIGHT - (SOFTKEY_HEIGHT + CHARACTER_HEIGHT) / 2;

	if(left_str != NULL)
	{
		vm_ascii_to_ucs2(wstr, 100, left_str);
		vm_graphic_textout(layer_buf, 5, text_y, wstr, wstrlen(wstr), 0xabcd);
	}
	
	if(right_str != NULL)
	{
		vm_ascii_to_ucs2(wstr, 100, right_str);
		vm_graphic_textout(layer_buf, SCREEN_WIDTH - 5 - vm_graphic_get_string_width(wstr),
						text_y, wstr, wstrlen(wstr), 0xabcd);
	}	
}

static void draw_grid(VMWSTR file_name)
{
	VMUINT file_size = 0, download_size = 0;
	VMINT fill_grid_num;
	VMINT i = 0;

	vm_graphic_fill_rect(layer_buf, (SCREEN_WIDTH - 120) / 2, 2, 121, 121,
							VM_COLOR_BLUE, VM_COLOR_WHITE);
	for(i = 1; i < 10; i++)
	{
		vm_graphic_line(layer_buf, (SCREEN_WIDTH - 120) / 2, i * 12 + 2,
						(SCREEN_WIDTH + 120) / 2, i * 12 + 2, VM_COLOR_BLUE);
		vm_graphic_line(layer_buf, i * 12 + (SCREEN_WIDTH - 120) / 2, 2,
						i * 12 + (SCREEN_WIDTH - 120) / 2, 122, VM_COLOR_BLUE);
	}
	
	Http_ReadFile(file_name, 0, 4, &file_size);
	if(0 == file_size)
	{
		return;
	}

	Http_ReadFile(file_name, 4, 4, &download_size);

	fill_grid_num = download_size * 100 / file_size;

	if(fill_grid_num > 10)
	{
		vm_graphic_fill_rect(layer_buf, (SCREEN_WIDTH - 120) / 2, 2,
						121, fill_grid_num / 10 * 12 + 1, VM_COLOR_BLUE, VM_COLOR_BLUE);
	}

	if(fill_grid_num > 0)
	{
		vm_graphic_fill_rect(layer_buf, (SCREEN_WIDTH - 120) / 2, fill_grid_num / 10 * 12 + 2,
						fill_grid_num % 10 * 12 + 1, 13, VM_COLOR_BLUE, VM_COLOR_BLUE);
	}
}

void Http_Output(VMCHAR* state_text, VMINT clear)
{
	if(strlen(app_text) > 0)
		strcat(app_text, "\n");
	
	if(state_text)
	{
		strcat(app_text, state_text);
	}

	handle_sysevt(VM_MSG_PAINT, 0);

	if(clear)
	{
		memset(app_text, 0, 1024);
	}	
}

void Tcp_Output(VMCHAR* state_text, VMINT text_len, VMINT clear)
{
	if(strlen(app_text) > 0)
		strcat(app_text, "\n");
	
	if(state_text)
	{
		if( text_len > 0 )
			strncat(app_text, state_text, text_len);
		else
			strcat(app_text, state_text);
	}
	
	handle_sysevt(VM_MSG_PAINT, 0);
}

VMINT Http_ReadFile(VMWSTR file_name, VMINT offset, VMINT num, void* data)
{
	VMFILE file_hdl;
	VMUINT nread;
	file_hdl = vm_file_open(file_name, MODE_READ, TRUE);
	if(file_hdl >= 0)
	{		
		vm_file_seek(file_hdl, offset, BASE_BEGIN);
		vm_file_read(file_hdl, data, num, &nread);
		vm_file_close(file_hdl);
		return 0;
	}
	return -1;

}
