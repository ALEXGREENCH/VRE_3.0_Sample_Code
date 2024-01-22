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

#include "vmlog.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmres.h"
#include "vmstdlib.h"
#include "Network.h"

/* ---------------------------------------------------------------------------
 * 全局数据。
 * ------------------------------------------------------------------------ */

typedef enum {
	HIDDEN,
	MAIN_MENU,
		TCP_CONNECT,
		HTTP_CONNECT,
		HTTP_CONNECT1,
		HTTP_CONNECT2,
		HTTP_CONNECT3,
		HTTP_CONNECT4,
		HTTP_CONNECT5,
		HTTP_CONNECT6,
		HTTP_CONNECT7,
		HTTP_CONNECT9,
		HTTP_CONNECT8
}DEMO_STATE;

VMINT		app_state;
VMINT		need_update = FALSE;
VMINT		layer_hdl = -1;			//图层句柄数组。VRE支持两层。
VMUINT8*	layer_buf = NULL;
VMINT		timer = -1;			//定时器ID。
VMCHAR*		app_text = NULL;	
VMINT		count = 0;

/** 触屏程序的菜单项，最多12项 */
const char MenuItemArray[12][99] = {
"+http_test_origin();",	//1
"+http_test_new();",
"+http_test_url_header_body();",	//2,3,4,5
"-http_test_sender_receiver();",	//6,8,10
"-http_test_sender_chunk();",	//7
"+http_test_receiver_bigfile();",	//11
"+http_test_receiver_chunk();",	//9
"+http_test_vm_http_request();",	//12
"+http_test_new_vm_http_request();",	//13 
"+http_test_timer_http();",
"",
"",
};

/* ---------------------------------------------------------------------------
 * 内部函数声明。
 * ------------------------------------------------------------------------ */
/*处理系统事件*/
void handle_sysevt(VMINT message, VMINT param);

/*处理按键事件*/
void handle_keyevt(VMINT event, VMINT keycode);

/*处理笔触事件*/
void handle_penevt(VMINT event, VMINT x, VMINT y);

/*初始化*/
void init(void);
/*绘制界面*/
void draw(void);
/*暂停*/
void pause(void);
/*恢复*/
void resume(void);
/*退出*/
void quit(void);

void run(VMINT tid);
/*绘制菜单界面*/
static void draw_menu(void);
/*绘制TCP界面*/
static void draw_tcp(void);
/*绘制HTTP界面*/
static void draw_download(void);
/*绘制多行文本*/
static void draw_text_muiltline(VMINT x, VMINT y, VMINT width, VMINT height, VMINT color);
/*绘制软键栏*/
static void draw_menu_bar(VMSTR left_str, VMSTR right_str);
/*绘制HTTP下载进度*/
static void draw_grid(VMWSTR file_name);

static void draw_grid_xy(int, int);
static void draw_menu_h(int);
static void draw_text(char * ptr);

extern void http_test_origin(void);
extern void http_test_url_header_body(void);
extern void http_test_sender_receiver(void);

/**
 * 应用程序入口函数。
 */
void vm_main(void) {
	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);

	vm_log_init("e:\\app_mibr.log", VM_DEBUG_LEVEL);
	vm_log_debug("vm_main start");


}

void handle_sysevt(VMINT message, VMINT param) {	
	switch (message) 
	{
		case VM_MSG_CREATE:
			init();
			break;
#ifdef _VRE_MIBR_
		case VM_MSG_PAINT:
			vm_log_debug("VM_MSG_PAINT");
			draw();
			break;
		case 6:  //hide
			vm_log_debug("VM_MSG_HIDE");
			pause();
			break;
#else
		case VM_MSG_ACTIVE:
			vm_log_debug("VM_MSG_ACTIVE");
			resume();			
			break;
		case VM_MSG_INACTIVE:
			vm_log_debug("VM_MSG_INACTIVE");
			pause();
			break;
#endif
		case VM_MSG_QUIT:
			quit();	
			break;	
	}
}

void handle_keyevt(VMINT event, VMINT keycode) 
{
	if(VM_KEY_EVENT_UP == event)
	{
		if(MAIN_MENU == app_state)						//菜单界面
		{
			if(VM_KEY_LEFT_SOFTKEY == keycode)			//左软键进入HTTP界面
			{
				app_state = HTTP_CONNECT;
				need_update = TRUE;
				memset(app_text, 0, 10240);
				StartHttpDownload();
			}
			else if(VM_KEY_RIGHT_SOFTKEY == keycode)	//右软键进入TCP界面
			{
				app_state = TCP_CONNECT;
				need_update = TRUE;
				memset(app_text, 0, 10240);
				StartTcpDemo();
			}
			else										//其他键退出
			{
				quit();
			}
		}
		else if(HTTP_CONNECT == app_state)				//HTTP界面
		{
			if(VM_KEY_RIGHT_SOFTKEY == keycode)			//右软键返回菜单界面
			{
				if(getHttpHandle() >= 0)
				{
					DropAllHttpConnection();
					memset(app_text, 0, 10240);
				}
				app_state = MAIN_MENU;
				need_update = TRUE;
			}
			if(VM_KEY_LEFT_SOFTKEY == keycode)			//左软键暂停/继续下载
			{
				if(getHttpHandle() >= 0)
				{					
					DropAllHttpConnection();
					memset(app_text, 0, 10240);
				}
				else
				{					
					StartHttpDownload();					
				}
				handle_sysevt(VM_MSG_PAINT, 0);
			}
		}
		else if(TCP_CONNECT == app_state)
		{
			if(VM_KEY_RIGHT_SOFTKEY == keycode)			//右软键返回菜单界面
			{
				StopTcpDemo();				
				app_state = MAIN_MENU;
				need_update = TRUE;
			}
		}
	}
}
static VMWSTR wtext[100];
static void vm_input_text_cb(VMINT state, VMWSTR text)
{
	memset(wtext, 0, sizeof(wtext)*sizeof(VMWSTR));

	//wstrcpy(wtext, text);
}
void handle_penevt(VMINT event, VMINT x, VMINT y)
{
	int index = get_index(x, y);
	/**
	 * 屏幕中不同格子支持不同的笔触事件。（可继续添加事件）
	 */
	if(event == VM_PEN_EVENT_TAP) {
		switch (index)
		{
		case 7:
			ShowAllHTTPConnection();
			break;
		case 88:
		case 89:
		case 90:
		case 91:
		/* 菜单左键 */
			if(MAIN_MENU == app_state)						//菜单界面
			{
				app_state = HTTP_CONNECT;
				need_update = TRUE;
				memset(app_text, 0, 10240);
				StartHttpDownload();
			}
			else if(HTTP_CONNECT == app_state)				//HTTP界面
			{
				StartHttpDownload();
			}
			else if(HTTP_CONNECT1 == app_state)				//HTTP界面
			{
				http_test_origin();
			}
			else if(HTTP_CONNECT2 == app_state)				//HTTP界面
			{
				http_test_url_header_body();
			}
			else if(HTTP_CONNECT3 == app_state)				//HTTP界面
			{
				http_test_sender_receiver();
			}
			else if(HTTP_CONNECT4 == app_state)				//HTTP界面
			{
				http_test_sender_chunk();
			}
			else if(HTTP_CONNECT5 == app_state)				//HTTP界面
			{
				http_test_receiver_bigfile();
			}
			else if(HTTP_CONNECT6 == app_state)				//HTTP界面
			{
				http_test_receiver_chunk();
			}
			else if(HTTP_CONNECT7 == app_state)				//HTTP界面
			{
				http_test_vm_http_request();
			}
			else if(HTTP_CONNECT8 == app_state)				//HTTP界面
			{
				http_test_new_vm_http_request();
			}
			else if(HTTP_CONNECT9 == app_state)				//HTTP界面
			{
				http_test_timer_http();
			}
			else
			{
				quit();
			}
			handle_sysevt(VM_MSG_PAINT, 0);
			break;
		case 92:
		case 93:
		case 94:
		case 95:
		/* 菜单右键 */
			if(MAIN_MENU == app_state)						//菜单界面
			{
				quit();
			}
			else if(HTTP_CONNECT == app_state)				//HTTP界面
			{
				StopHttpDownload(getHttpHandle());

				memset(app_text, 0, 10240);

				app_state = MAIN_MENU;
				need_update = TRUE;
			}
/*
			else if(HTTP_CONNECT1 == app_state)				//test origin
			{
				vm_exit_app();
			}
*/
			else
			{
				DropAllHttpConnection();
				memset(app_text, 0, 10240);
				app_state = MAIN_MENU;
				need_update = TRUE;
			}
			break;
		default:
			if (MAIN_MENU == app_state){
				if (index >= 0 && index <= 7)
				{
					app_state = HTTP_CONNECT1;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_origin();
				}
				else if (index >= 8 && index <= 15)
				{
					app_state = HTTP_CONNECT2;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_new();
				}
				else if (index >= 16 && index <= 23)
				{
					app_state = HTTP_CONNECT2;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_url_header_body();
				}
				else if (index >= 24 && index <= 31)
				{
					app_state = HTTP_CONNECT3;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_sender_receiver();
				}
				else if (index >= 32 && index <= 39)
				{
					app_state = HTTP_CONNECT4;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_sender_chunk();
				}
				else if (index >= 40 && index <= 47)
				{
					app_state = HTTP_CONNECT5;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_receiver_bigfile();
				}
				else if (index >= 48 && index <= 55)
				{
					app_state = HTTP_CONNECT6;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_receiver_chunk();
				}
				else if (index >= 56 && index <= 63)
				{
					app_state = HTTP_CONNECT7;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_vm_http_request();
				}
				else if (index >= 64 && index <= 71)
				{
					app_state = HTTP_CONNECT8;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_new_vm_http_request();
				}
				else if (index >= 72 && index <= 79)
				{
					app_state = HTTP_CONNECT9;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					http_test_timer_http();
				}
				else if (index >= 80 && index <= 87)
				{
					app_state = 99;
					need_update = TRUE;
					memset(app_text, 0, 10240);
					vm_input_text(vm_input_text_cb);
				}
				else
					quit();
			}
			break;
		}
	}
}

void init(void)
{
	
	app_state = MAIN_MENU;
	need_update = TRUE;

	app_text = vm_calloc(10240);
		
	if(timer < 0)
	{
		timer = vm_create_timer(TIMEOUT_TIME, run);
	}	
}

void draw(void)
{
	/*创建Layer*/
	if(0 > layer_hdl)
	{
		layer_hdl = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);
		if(0 > layer_hdl)
		{
			return;//quit();
		}
		
		/*创建Buffer*/
		if ((layer_buf = vm_graphic_get_layer_buffer(layer_hdl)) == NULL)
		{
			return;//quit();
		}

		vm_graphic_set_clip(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		vm_log_debug("draw - width:%d, height:%d", vm_graphic_get_screen_width(), vm_graphic_get_screen_height());
	}	

	switch(app_state)
	{
	case MAIN_MENU:
		draw_menu();
		break;
	case TCP_CONNECT:		
		draw_tcp();
		break;
	case HTTP_CONNECT:		
	case HTTP_CONNECT1:		
	case HTTP_CONNECT2:		
	case HTTP_CONNECT3:		
	case HTTP_CONNECT4:		
	case HTTP_CONNECT5:		
	case HTTP_CONNECT6:		
	case HTTP_CONNECT7:		
	case HTTP_CONNECT8:		
	case HTTP_CONNECT9:		
		draw_download();
	case 99:			
		/* 输出文字 */
		//vm_graphic_textout(layer_buf, 5, 5, wtext, wstrlen(wtext), VM_COLOR_BLUE);
		break;	
	default:
		break;
	}

	draw_text("			mibr");

	vm_graphic_fill_rect(layer_buf, SCREEN_WIDTH-40, 0, 40, 40, VM_COLOR_RED, VM_COLOR_RED); 

	vm_graphic_flush_layer(&layer_hdl, 1);

}

void pause(void)
{
	//删除定时器
	if(timer >= 0)
	{
		vm_delete_timer(timer);
		timer = -1;
	}	
	
	//删除Layer
	if(layer_hdl >= 0)
	{		
		vm_graphic_delete_layer(layer_hdl);
		layer_hdl = -1;
	}
	
	//删除Buffer
	if(layer_buf)
	{
		layer_buf = NULL;
	}	
}

void resume(void)
{
	//创建Layer
	if(0 > layer_hdl)
	{
		layer_hdl = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);
		if(0 > layer_hdl)
		{
			quit();
		}
		
		//创建Buffer
		if ((layer_buf = vm_graphic_get_layer_buffer(layer_hdl)) == NULL)
		{
			quit();
		}
	}	
	
	vm_graphic_set_clip(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	//创建定时器
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
	
	//删除Layer
	if(layer_hdl >= 0)
	{
		vm_graphic_delete_layer(layer_hdl);
		layer_hdl = -1;
	}
	//删除Buffer
	if(layer_buf)
	{
		vm_free(layer_buf);
		layer_buf = NULL;
	}	
	//退出应用
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

	draw_menu_bar("HTTP2", "QUIT");
	
	//draw_grid_xy(40, 40);
	draw_menu_h(40);

	vm_graphic_flush_layer(&layer_hdl, 1);
}

static void draw_tcp(void)
{
	vm_graphic_fill_rect(layer_buf, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0); 

	draw_text_muiltline(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - SOFTKEY_HEIGHT, 0xabcd);

	draw_menu_bar(NULL, "返回");
	
	vm_graphic_flush_layer(&layer_hdl, 1);
}


static void draw_download(void)
{
	VMCHAR file_name[50];
	VMCHAR full_name[50];
	struct vm_fileinfo_t info;
	VMINT find_hdl;
	VMINT	drv;
	
	/* 获取盘符 */
	drv= vm_get_removable_driver();	//优先获取移动盘符
	if(drv<0)							//不存在移动盘
		drv=vm_get_system_driver();		//获取系统盘
	
	vm_graphic_fill_rect(layer_buf, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0); 
	vm_graphic_line(layer_buf, 0, 125, SCREEN_WIDTH, 125, VM_COLOR_RED);

	sprintf(full_name, "%c:\\vre\\logo_vre2.*", drv);
	if((find_hdl = vm_find_first(vm_ucs2_string(full_name), &info)) >= 0)
	{
		vm_ucs2_to_gb2312(file_name, 50, info.filename);
		if(!vm_ends_with(file_name, "jpg"))
		{
			VMUINT8* res;
			VMINT canvas_hdl;
			VMUINT8* canvas_buf;
			struct frame_prop* img_prop; 

			sprintf(full_name, "%c:\\vre\\%s", drv, file_name);
			res = (VMUINT8*)vm_malloc(info.size);
			if(!Http_ReadFile(vm_ucs2_string(full_name), 0, info.size, res))
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
			draw_grid(vm_ucs2_string(full_name));
		}

		vm_find_close(find_hdl);
	}	

	draw_text_muiltline(0, 126 , SCREEN_WIDTH, SCREEN_HEIGHT - 126, 0xabcd);
	
	if(getHttpHandle() >= 0)
	{
		draw_menu_bar("暂停", "返回");
	}
	else
	{
		draw_menu_bar("继续", "返回");
	}	
	
	vm_graphic_flush_layer(&layer_hdl, 1);
}

static void draw_text_muiltline(VMINT x, VMINT y, VMINT width, VMINT height, VMINT color)
{
	VMINT double_char_num = 0, char_num = 0, num_per_line = 0;
	VMCHAR* buffer_start = app_text;
	VMCHAR* line_start = app_text;
	VMCHAR line_str[128] = {0};
	VMWCHAR line_wstr[128] = {0};
	//先得到每一行可容纳的中文字符数
	VMWCHAR wstr[2];
	wstrcpy(wstr, vm_ucs2_string("一"));
	num_per_line = width / vm_graphic_get_string_width(wstr);

	//逐行获取和绘制
	while(*buffer_start)  //逐字符分析
	{
		/* 因为ARM编译器认定char为无符号类型，所以加上条件编译 */
#ifdef _DEBUG
		if(*buffer_start >= 0 && *buffer_start <= 127)  //ascii字符
#else
		if(*buffer_start <= 127)  //ascii字符   
#endif
		{
			if(*buffer_start == '\n') //换行
			{
				if(buffer_start - line_start > 0)
				{
					strncpy(line_str, line_start, buffer_start - line_start);
					vm_gb2312_to_ucs2(line_wstr, 128, line_str);
					//vm_log_debug("line_str:%s, line_wstr:%s", line_str, line_wstr);
					vm_graphic_textout(layer_buf, x, y, line_wstr, vm_wstrlen(line_wstr), (VMUINT16)(color));
				}
				y += CHARACTER_HEIGHT;
				memset(line_str, 0, 128);
				line_start = buffer_start + 1;
				double_char_num = 0;
			}
			else
			{
				char_num++;
				if(char_num == 2)
				{
					char_num = 0;
					double_char_num++;
				}
			}			
			buffer_start++;
		}
		else
		{
			buffer_start += 2;
			double_char_num++;
		}
			
		if(double_char_num == num_per_line) //达到一行
		{
			if(char_num)			//多出一个半角字符，修正
				buffer_start -= 2;

			if(*buffer_start && buffer_start - line_start)
			{
				strncpy(line_str, line_start, buffer_start - line_start);
				vm_gb2312_to_ucs2(line_wstr, 128, line_str);
				vm_graphic_textout(layer_buf, x, y, line_wstr, vm_wstrlen(line_wstr), (VMUINT16)(color));
			}
			y += CHARACTER_HEIGHT;
			memset(line_str, 0, 128);
			if(y > (height - CHARACTER_HEIGHT))
				break;			
			line_start = buffer_start;
			double_char_num = 0;
		}
	}
	
	if(line_start)
	{
		if(buffer_start - line_start > 0)
		{
			strncpy(line_str, line_start, buffer_start - line_start);
			vm_gb2312_to_ucs2(line_wstr, 128, line_str);
			vm_graphic_textout(layer_buf, x, y, line_wstr, vm_wstrlen(line_wstr), (VMUINT16)(color));
		}
	}
}

static void draw_menu_bar(VMSTR left_str, VMSTR right_str)
{
	VMINT text_y;
	VMWCHAR wstr[50];

	/*绘制menubar*/
	vm_graphic_fill_rect(layer_buf, 0, SCREEN_HEIGHT-SOFTKEY_HEIGHT,
							SCREEN_WIDTH, SOFTKEY_HEIGHT, VM_COLOR_WHITE, VM_COLOR_WHITE);
	text_y = SCREEN_HEIGHT - (SOFTKEY_HEIGHT + CHARACTER_HEIGHT) / 2;

	if(left_str != NULL)
	{
		vm_gb2312_to_ucs2(wstr, 50, left_str);
		vm_graphic_textout(layer_buf, 5, text_y, wstr, wstrlen(wstr), 0xabcd);
	}
	
	if(right_str != NULL)
	{
		vm_gb2312_to_ucs2(wstr, 50, right_str);
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

	draw();//handle_sysevt(VM_MSG_PAINT, 0);

	if(clear)
	{
		memset(app_text, 0, 10240);
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



#define GRID_WIDTH (40)
#define GRID_HEIGHT (40)

/*
0	1	2	3	4	5	6	7
8							15
16							31
32							31
40							31
48							31
56							31
64							31
72							31
80							31
88							31
*/
/**
 * 根据坐标获得格子的索引。
 */
int get_index(int x, int y)
{
	return x/GRID_WIDTH + (y/GRID_HEIGHT)*(vm_graphic_get_screen_width()/GRID_WIDTH);
}

/**
 * 在屏幕上画格子，让每个格子支持不同的笔触事件。
 */
static void draw_grid_xy(int width, int height)
{
	int nRow;
	int nCol;
//	int n=0;
//	char tmp[128];
//	VMWCHAR s[50];

	/* 获取目标缓冲 
	VMUINT8* buf = vm_graphic_get_layer_buffer(layer_hdl[0]);	*/
	VMUINT8* buf = layer_buf;

	/** Draw rows */
	for (nRow=0; nRow<vm_graphic_get_screen_height(); nRow+=height)
	{
		/* 线 */
		vm_graphic_line(buf, 0, nRow,
			vm_graphic_get_screen_width()-1, nRow, 
			VM_COLOR_BLUE);
	}
	/* 线 */
	vm_graphic_line(buf, 0, nRow-1,
		vm_graphic_get_screen_width()-1, nRow-1, 
		VM_COLOR_BLUE);

	/** Draw columns */
	for (nCol=0; nCol<vm_graphic_get_screen_width(); nCol+=width)
	{
		/* 线 */
		vm_graphic_line(buf, nCol, 0,
			nCol, vm_graphic_get_screen_height()-1, 
			VM_COLOR_BLUE);
	}
	/* 线 */
	vm_graphic_line(buf, nCol-1, 0,
		nCol-1, vm_graphic_get_screen_height()-1, 
		VM_COLOR_BLUE);

	/* 把层显示缓冲区的数据刷新到屏幕上。
	vm_graphic_flush_layer(layer_hdl, 1);*/
}



/**
 * 绘制横向菜单，菜单项存放在字符串数组MenuItemArray中。
 */
static void draw_menu_h(int height)
{
	int nRow, nCol, n=0;
	char tmp[128];
	VMWCHAR s[100];

	/* 获取目标缓冲 
	VMUINT8* buf = vm_graphic_get_layer_buffer(layer_hdl[0]);	*/
	VMUINT8* buf = layer_buf;

	/* 用白色黑底清屏 
	vm_graphic_fill_rect(buf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_WHITE, VM_COLOR_BLACK);*/

	/** Draw rows */
	for (nRow=0; nRow<vm_graphic_get_screen_height(); nRow+=height)
	{
		/* 线 */
		vm_graphic_line(buf, 0, nRow,
			vm_graphic_get_screen_height()-1, nRow, 
			VM_COLOR_WHITE);

		/* 输出文字 */
		sprintf(tmp, "%s", MenuItemArray[nRow/height]);
		vm_gb2312_to_ucs2(s, 100, tmp);
		vm_graphic_textout(buf,	0, nRow+height/2, s, wstrlen(s), VM_COLOR_WHITE);
	}
	/* 线 */
	vm_graphic_line(buf, 0, nRow-1,
		vm_graphic_get_screen_height()-1, nRow-1, 
		VM_COLOR_WHITE);

	/* 把层显示缓冲区的数据刷新到屏幕上。
	vm_graphic_flush_layer(layer_hdl, 1);*/
}
/**
 * 在屏幕底部输出字符串，不支持换行。
 */
#define SCREEN_MAX_STR_LEN	(512)   //Max screen string length
static void draw_text(char * ptr) {
	VMWCHAR s[SCREEN_MAX_STR_LEN];
	VMUINT8* buf;	
	int x, y, w;
	
	/* 手机操作系统默认使用UCS2编码的字符串，在执行字符串输出之前必须将其
	   转换为UCS2编码。（VC6默认使用GB2312编码，可以使用L前缀将字符串转换
	   为UCS2编码格式，但是ADS1.2的L前缀不支持中文，所以建议使用VRE提供的
	   字符集转换函数实现转码）
	*/
	char* text = (char*)ptr;
	vm_gb2312_to_ucs2(s, SCREEN_MAX_STR_LEN * 2 , text);

	/* 设置文字在屏幕中央输出 */
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height());// / 2;
	
	/* 获取目标缓冲 
	buf = vm_graphic_get_layer_buffer(layer_hdl[0]);	*/	
	buf = layer_buf;
	
	/* 用白色黑底清屏 
	vm_graphic_fill_rect(buf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_WHITE, VM_COLOR_BLACK);*/
	
	/* 输出文字 */
	vm_graphic_textout(buf,	x, y, s, wstrlen(s), VM_COLOR_BLUE);

	/* 把层显示缓冲区的数据刷新到屏幕上。
	vm_graphic_flush_layer(layer_hdl, 1);*/
}
