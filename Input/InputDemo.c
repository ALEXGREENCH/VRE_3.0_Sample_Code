/* ============================================================================
* VRE demonstration application.
*
* Copyright (c) 2006-2007 Vogins Network Technology (China). All rights 
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

#include <string.h>

#include "vmsys.h"
#include "vmchset.h"
#include "vmgraph.h"
#include "vmio.h"
#include "vmstdlib.h"

#include "InputDemo.h"

VMINT layer_hdl[2];
VMUINT8* layer_buf[2];
VMINT screen_width;
VMINT screen_height;
VMINT character_height;
VMWCHAR w_text[256] = {0};
VMCHAR softkey_text[][8] = {"Input", "Exit"};

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
 * draw screen
 */
void draw_screen(void);

void input_callback(VMINT state, VMWSTR text);

/*
 * draw text in specail region
 */
void draw_text_in_rect(VMUINT8* buf, VMWCHAR *msg, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color);
 
 /*
 * VRE程序入口函数。
 */
void vm_main(void) {
	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
}

void handle_sysevt(VMINT message, VMINT param)
{	
	switch(message)
	{
	case VM_MSG_CREATE:
		screen_width = vm_graphic_get_screen_width();		
		screen_height = vm_graphic_get_screen_height();
		character_height = vm_graphic_get_character_height();
		layer_hdl[0] = vm_graphic_create_layer(0, 0, screen_width, screen_height, -1);
		layer_buf[0] = vm_graphic_get_layer_buffer(layer_hdl[0]);
		break;
	case VM_MSG_PAINT:
		draw_screen();
		break;
	/* this message will be triggered when call input function */
	case VM_MSG_INACTIVE:						
		vm_graphic_delete_layer(layer_hdl[0]);
		break;
	/* this message will be triggered when come back from input window */
	case VM_MSG_ACTIVE:							
		layer_hdl[0] = vm_graphic_create_layer(0, 0, screen_width, screen_height, -1);
		layer_buf[0] = vm_graphic_get_layer_buffer(layer_hdl[0]);
		break;
	case VM_MSG_QUIT:
		vm_graphic_delete_layer(layer_hdl[0]);
		vm_exit_app();
		break;
	}
}

void handle_keyevt(VMINT event, VMINT keycode)
{
	if(VM_KEY_EVENT_UP == event)
	{
		switch(keycode)
		{
		case VM_KEY_LEFT_SOFTKEY:
			/* It will trigger VM_MSG_INACTIVE message when call this function*/
			vm_input_text3(w_text, 20, VM_INPUT_METHOD_TEXT, input_callback);
			break;
		case VM_KEY_RIGHT_SOFTKEY:
			vm_graphic_delete_layer(layer_hdl[0]);
			vm_exit_app();
			break;
		}
	}
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{

}

void draw_screen(void)
{
	VMWCHAR w_softkey_text[8];
	VMINT string_width;
	
//	vm_graphic_clear_layer_bg(layer_hdl[0]);
	vm_graphic_fill_rect(layer_buf[0], 0, 0, screen_width, screen_height, VM_COLOR_BLACK, VM_COLOR_BLACK);
	vm_graphic_fill_rect(layer_buf[0], 0, 0, screen_width, screen_height / 2, VM_COLOR_RED, VM_COLOR_WHITE);
	
	/*draw input text*/
	draw_text_in_rect(layer_buf[0], w_text, 2, 2, screen_width, screen_height / 2, VM_COLOR_BLACK);
	
	/*draw soft key*/
	vm_gb2312_to_ucs2(w_softkey_text, 8, softkey_text[0]);
	vm_graphic_textout(layer_buf[0], 2, screen_height - character_height - 2, w_softkey_text, wstrlen(w_softkey_text), VM_COLOR_WHITE);
	vm_gb2312_to_ucs2(w_softkey_text, 8, softkey_text[1]);
	string_width = vm_graphic_get_string_width(w_softkey_text);
	vm_graphic_textout(layer_buf[0], screen_width - string_width - 2, screen_height - character_height - 2, w_softkey_text, wstrlen(w_softkey_text), VM_COLOR_WHITE);

	vm_graphic_flush_layer(layer_hdl, 1);

}

/**
 * this function will be called after 
 * VM_MSG_ACTIVE and VM_MSG_PAINT are handled
 */
void input_callback(VMINT state, VMWSTR text)
{
	/* user has confirmed input characters */
	if(TRUE == state)
	{
		if(wstrlen(text) > 255)
		{
			return;
		}
		wstrcpy(w_text, text);
		
		/* trigger VM_MSG_PAINT message */
		vm_graphic_flush_screen();
	}
}

void draw_text_in_rect(VMUINT8* buf, VMWCHAR *msg, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color)
{
	VMWCHAR *tmp, ch;
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
			vm_graphic_textout(layer_buf[0], x, y + row * ch_height, tmp, wstrlen(tmp), color);
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
			vm_graphic_textout(layer_buf[0], x, y + row * ch_height, tmp, wstrlen(tmp), color);
			tmp[0] = 0;
			base += count;
			count = 0;
			row++;
		}
	}
	if ((wstrlen(tmp) > 0) && (row < nrow))
	{
		wstrncpy(tmp, msg + base, count);
		vm_graphic_textout(layer_buf[0], x, y + row * ch_height, tmp, wstrlen(tmp), color);
	}
	
	if (tmp != NULL)
		vm_free(tmp);
}



