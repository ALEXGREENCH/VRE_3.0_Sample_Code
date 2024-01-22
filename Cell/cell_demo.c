/* ============================================================================
 *  VRE demonstration application.
 *  模块名：cell模块demo
 *  文件名：cell_demo.c
 *  实现功能: 在手机与模拟器上实现了打开cell、获取小区的cell信息、获取小区相邻的数组指针与
 *            当前小区所相邻的小区的数量
 *  重要的函数是: open_cell()、get_cell_info()、get_cell_nbr_info()、get_cell_nbr_num()
 *  作者:苗合平
 *  版本1.0
 *  完成日期：2010-4-9
 * ========================================================================= */

/* ============================================================================
 *  VRE demonstration application.
 *  module name: cell
 *  source name: cell_demo.c
 *  function: 
 *	1.open cell, 
 *	2.get the information of current cell, 
 *	3.get the information of adjacent cells,
 *	4.get the count of adjacent cells of current cell
 *  author:	heping.miao
 *  version: 1.0
 *  date: 2010-4-9
 * ========================================================================= */

/* ---------------------------------------------------------------------------
 *                              standard VRE header file
 * ------------------------------------------------------------------------ */
#include "vmio.h"
#include "vmsock.h"
#include "vmgraph.h"
#include "vmgfxold.h"
#include "vmchset.h"
#include "vmcell.h"
#include "vmlog.h"
#include "vmstdlib.h"

/* ---------------------------------------------------------------------------
 *                              non-standard VRE header file
 * ------------------------------------------------------------------------ */

#include "cell_demo.h"
#include "string.h"
#include "stdio.h"

/* ---------------------------------------------------------------------------
 *                          global define
 * ------------------------------------------------------------------------ */

#define SCREEN_WIDTH vm_graphic_get_screen_width() 
#define SCREEN_HEIGHT vm_graphic_get_screen_height() 
#define FONT_HEIGHT vm_graphic_get_character_height() 

/* ---------------------------------------------------------------------------
 *                           variable define
 * ------------------------------------------------------------------------ */

VMINT layer[1];				
VMUINT8 *layer_buffer;		
VMINT ProgramState;						
VMINT timerid;				
VMINT const check_cell_len = 5;
VMINT8  CHECK_CELL[][30]={
	"Num 1 Open cell",
		"Num 2 info of current cell",
		"Num 3 info of adjacent cells",
		"Num 4 count of adjacent cells",
		"Num 5 Close Cell"
};
VMINT8 cell_info_cts[][40] = {
	"Number of channel",
		"Identity code of BS",
		"Cell identity",
		"Region code",
		"Country code",
		"Network code",
		"Strength level"
};
VMINT cell_return_value;
VMCHAR CELL_OPEN_STATE[200];
VMINT cell_info[7];
vm_cell_info_struct *cell_cur_info;
vm_cell_info_struct ** cell_nbr_info;
VMINT cell_nbr_num;

/* ---------------------------------------------------------------------------
*                           function declaration
* ------------------------------------------------------------------------ */

void init_vre(void);//initialization
void timer_proc(VMINT tid);

void key_event(VMINT event,VMINT keycode);//key events
void sys_event(VMINT handel,VMINT event);//system events

void key_logo(VMINT event,VMINT keycode);//key events on logo screen
void key_cell_state(VMINT event,VMINT keycode);//key events on cell screen

void draw_logo(void);
void draw_cell_state(void);

void Init_Log(void);
void exit_demo(void);//exit program

void open_cell(void);//open cell
void close_cell(void);//close cell
void get_cell_info(void);//get the information of current cell,
void get_cell_nbr_info(void);//get the information of adjacent cells
void get_cell_nbr_num(void);//get the count of adjacent cells of current cell
void mem_free(void);//free memory
void draw_text_in_specified_rect(VMWCHAR *msg, VMUINT8* buf, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color);

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
	 VMINT i = 0;
	 for(i=0;i<8;i++)
	 {
		 cell_info[i] = 0;
	 }
     ProgramState = LOGO;
	 cell_return_value = -3;
	 cell_cur_info = NULL;
	 cell_nbr_info = NULL;
	 cell_nbr_num = -1;
	 memset(CELL_OPEN_STATE,0,200);
	 Init_Log();
	 timerid = vm_create_timer(100,timer_proc);
}

void key_event(VMINT event,VMINT keycode)
{
     switch(ProgramState)
	 {
	 case LOGO:
		 key_logo(event,keycode);
		 break;
     case CELL_STATE:
		 key_cell_state(event,keycode);
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


void open_cell(void)
{
	cell_return_value = vm_cell_open();		
#ifdef _DEBUG //if running in emulator, implement code below 
	cell_return_value = cell_return_value + 1;
#endif 
	switch(cell_return_value)
	{
	case VM_CELL_OPEN_SUCCESS:
        sprintf(CELL_OPEN_STATE,"open cell success");
		break;
	case VM_CELL_OPEN_ALREADY_OPEN:
		sprintf(CELL_OPEN_STATE,"cell has already opened");
		break;
	case VM_CELL_OPEN_RES_LESS:
        sprintf(CELL_OPEN_STATE,"lack of resource");		
		break;
	case VM_CELL_OPEN_REG_ERROR:
		sprintf(CELL_OPEN_STATE,"cell register error");
		break;
	}

}

void close_cell(void)
{
	memset(CELL_OPEN_STATE,0,200);
	if(cell_return_value == 0 || cell_return_value == 1)
	{
       vm_cell_close();
	   cell_return_value = -3;	   
	   sprintf(CELL_OPEN_STATE,"close cell success");
	}
	else
	{
       sprintf(CELL_OPEN_STATE,"cell is not exist");
	}
    mem_free();
}


void get_cell_info(void)
{
#ifdef _DEBUG //if running in emulator, implement code below
	if(cell_return_value != 0) 
	{
		sprintf(CELL_OPEN_STATE,"cell is closed, cannot get info");
	}
	else
	{
	   sprintf(CELL_OPEN_STATE,"cell information:");
       cell_cur_info = (vm_cell_info_struct *)vm_malloc(sizeof(vm_cell_info_struct));
	   cell_info[0] = cell_cur_info->arfcn = 100;
	   cell_info[1] = cell_cur_info->bsic = 101;
	   cell_info[2] = cell_cur_info->ci = 102;
	   cell_info[3] = cell_cur_info->lac = 103;
	   cell_info[4] = cell_cur_info->mcc = 104;
	   cell_info[5] = cell_cur_info->mnc = 105;
	   cell_info[6] = cell_cur_info->rxlev = 2;
	}
#else  //if running in handset, implement code below
	if(cell_return_value == 0 ||cell_return_value == 1)
	{
		sprintf(CELL_OPEN_STATE,"cell information:");
        cell_cur_info = (vm_cell_info_struct *)vm_malloc(sizeof(vm_cell_info_struct));
        cell_cur_info = vm_cell_get_cur_cell_info();
		cell_info[0] = cell_cur_info->arfcn;
	    cell_info[1] = cell_cur_info->bsic;
	    cell_info[2] = cell_cur_info->ci;
	    cell_info[3] = cell_cur_info->lac;
	    cell_info[4] = cell_cur_info->mcc;
	    cell_info[5] = cell_cur_info->mnc;
	    cell_info[6] = cell_cur_info->rxlev;
	}
    else
	{
        sprintf(CELL_OPEN_STATE,"cell is closed, cannot get info");
	}	
#endif
}

void get_cell_nbr_info(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 	
	if(cell_return_value != 0) 
	{
		sprintf(CELL_OPEN_STATE,"cell is closed, cannot get info");
	}
	else
	{
		cell_nbr_info = (vm_cell_info_struct **)vm_malloc(sizeof(vm_cell_info_struct));
        //cell_nbr_info = vm_cell_get_nbr_cell_info();
		sprintf(CELL_OPEN_STATE,"pointer to the info of adjacent cells:%o",1233);  
		
	}
#else //if running in handset, implement code below
    if(cell_return_value == 0 ||cell_return_value == 1) 
	{
		cell_nbr_info = (vm_cell_info_struct **)vm_malloc(sizeof(vm_cell_info_struct));
        cell_nbr_info = vm_cell_get_nbr_cell_info();
		sprintf(CELL_OPEN_STATE,"pointer to the info of adjacent cells:%o",cell_nbr_info);
	}
	else
	{
		sprintf(CELL_OPEN_STATE,"cell is closed, cannot get info");
	}
#endif 
}

void get_cell_nbr_num(void)
{
#ifdef _DEBUG //if running in emulator, implement code below 
	if(cell_return_value != 0) 
	{
		sprintf(CELL_OPEN_STATE,"cell is closed, cannot get info");
	}
	else
	{
        cell_nbr_num = 3;
		sprintf(CELL_OPEN_STATE,"the count of adjacent cells:%d",cell_nbr_num);
	}
#else //if running in handset, implement code below
    if(cell_return_value == 0 ||cell_return_value == 1) 
	{
         cell_nbr_num = * vm_cell_get_nbr_num();
		 sprintf(CELL_OPEN_STATE,"the count of adjacent cells:%d",cell_nbr_num);
	}
	else
	{
		sprintf(CELL_OPEN_STATE,"cell is closed, cannot get info");
	}
#endif    
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
				 ProgramState = CELL_STATE;
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

void key_cell_state(VMINT event,VMINT keycode)
{
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{    			 
             switch(keycode)
			 {			 
			 case VM_KEY_RIGHT_SOFTKEY://if press left soft key, go back to logo screen
				 ProgramState = LOGO;
				 memset(CELL_OPEN_STATE,0,200);//reset notify information
				 mem_free();//free memory
				 break;
			 case VM_KEY_NUM1:
				 mem_free();
				 open_cell();				 
                 break;
			 case VM_KEY_NUM2:
				 mem_free();
				 get_cell_info();				 
				 break;
			 case VM_KEY_NUM3:
				 mem_free();
				 get_cell_nbr_info();				 
				 break;
			 case VM_KEY_NUM4:
				 mem_free();
				 get_cell_nbr_num();
				 break;
			 case VM_KEY_NUM5:
				 close_cell();
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
	VMWCHAR wstr[20] = {0};
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_WHITE,VM_COLOR_BLACK);
	vm_ascii_to_ucs2(wstr,40,"Cell_Demo");

	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,
		(SCREEN_HEIGHT-vm_graphic_get_character_height())/2,wstr,wstrlen(wstr),VM_COLOR_WHITE);
	vm_ascii_to_ucs2(wstr,40,"start");
	vm_graphic_textout(layer_buffer,2,SCREEN_HEIGHT-FONT_HEIGHT - 2,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_ascii_to_ucs2(wstr,40,"quit");
	
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr)-2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_graphic_flush_layer(layer,1);
}

void draw_cell_state(void)
{
    VMINT i = 0; 
	VMWCHAR wstr[60] = {0};
	VMCHAR str[60] = {0};
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_WHITE,VM_COLOR_BLACK);
	for(i=0;i<check_cell_len;i++)
	{
		vm_ascii_to_ucs2(wstr, 120, CHECK_CELL[i]);
		vm_graphic_textout(layer_buffer,12,10+(FONT_HEIGHT+3)*i,wstr,wstrlen(wstr),VM_COLOR_RED);
	}
	vm_graphic_fillrect(10,10+(FONT_HEIGHT+3)*7,SCREEN_WIDTH-20,SCREEN_HEIGHT-(10+(FONT_HEIGHT+3)*7)-10,VM_COLOR_888_TO_565(112,112,225));
	vm_ascii_to_ucs2(wstr, 120, CELL_OPEN_STATE);
//	vm_graphic_textout(layer_buffer,12,12+(FONT_HEIGHT+3)*7,wstr,wstrlen(wstr),VM_COLOR_GREEN);
	
	draw_text_in_specified_rect(wstr,layer_buffer,12,12+(FONT_HEIGHT+3)*7,SCREEN_WIDTH-20,SCREEN_HEIGHT-(10+(FONT_HEIGHT+3)*7)-10, VM_COLOR_GREEN);

	if((cell_return_value == 0 ||cell_return_value == 1) &&(cell_cur_info!=NULL))
	{
        for(i=0;i<7;i++)
		{
			sprintf(str,"%s:%6d",cell_info_cts[i],cell_info[i]);
			vm_ascii_to_ucs2(wstr,120,str);
			vm_graphic_textout(layer_buffer,12,15+(FONT_HEIGHT+3)*8+(FONT_HEIGHT+3)*i,wstr,wstrlen(wstr),VM_COLOR_WHITE);
		}
	}	

	vm_ascii_to_ucs2(wstr,120,"return");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr)-14,SCREEN_HEIGHT-FONT_HEIGHT-18,wstr,wstrlen(wstr),VM_COLOR_WHITE);
 	vm_graphic_flush_layer(layer,1);
} 

void timer_proc(VMINT tid)
{
	switch(ProgramState)
	{
	case LOGO:
		draw_logo();
		break;
	case CELL_STATE:
		draw_cell_state();
		break;
	default:
		break;
	}
}

void Init_Log(void)
{
	VMINT8 logfile[30] = {0};
	VMINT8 path[30] = {0};
	VMINT drv = vm_get_removable_driver();
	if (drv < 0)
		drv = vm_get_system_driver();

	 sprintf(logfile,"%c:\\vretest.log", drv);
	 vm_log_init(logfile,VM_DEBUG_LEVEL);
     vm_log_fatal("Start Log!");	 
}

/* free memory */
void mem_free(void)
{
     if(cell_cur_info != NULL)
	 {
		 vm_free(cell_cur_info);
		 cell_cur_info = NULL;
	 }
	 if(cell_nbr_info!=NULL)
	 {
		 vm_free(cell_nbr_info);
		 cell_nbr_info = NULL;
	 }
}

/* exit program */
void exit_demo(void)
{
    vm_graphic_delete_layer(layer[0]);
	mem_free();
	vm_delete_timer(timerid);
    vm_exit_app();
}

void draw_text_in_specified_rect(VMWCHAR *msg, VMUINT8* buf, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color)
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