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

/*
 * SIM.c演示如何使用VRE应用接口来获取运营商名称、SIM卡数量、是否插入SIM卡、IMSI和IMEI号。
 * SIM.c demonstrates how to use the VRE API to get the active operator, the number of SIM cards, the IMSI and IMEI number of SIM card.
 * Copyright (c) 2009 Vogins Network Technology (China). All rights reserved.
 */

#include "vmio.h"
#include "vmgraph.h"
#include "vmsys.h"
#include "vmchset.h"

#include "vmstdlib.h"
#include "vmres.h"
#include "vmtel.h"
#include "vmsim.h"
#include <string.h>
#include <stdio.h>


#define TEXT_DEFAULT	"\n\nPress any key to quit"

/*
 * System event 
 */
static void handle_sysevt(VMINT message, VMINT param);

/*
 *  Key event 
 */
static void handle_keyevt(VMINT event, VMINT keycode);

/*
 *  Draw screen 
 */
static void draw_screen(void);

static VMINT	layer_handle = -1;
static VMCHAR   g_text[256];
static VMSHORT	g_id_timer;
static VMUINT8* src_buf= NULL;

/**
 *  Entry 
 */
void vm_main(void) {	
	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
}

static void l_do_sim_check(void)
{
	operator_t op;
	VMINT temp;
	VMSTR sz_num = NULL;
	memset(g_text, 0, sizeof(g_text));

	/* Get the active operator */
	strcat(g_text, "Current Operator:");
	op = vm_get_operator();
	switch(op)
	{
	case NO_SIM_CARD:  
		strcat(g_text, "No Sim Card\n");
		break;
	case UNKOWN_OPERATOR:
		strcat(g_text, "Unknown Operator\n");
		break;
	case CMCC:	
		strcat(g_text, "China Mobile\n");
		break;
	case UNICOM:
		strcat(g_text, "China Unicom\n");
		break;
	case CNC:
		strcat(g_text, "China NetCom\n");
		break;
	case CNTELCOM:
		strcat(g_text, "China Telecom\n");
		break;
	case ALL:
		strcat(g_text, "Others\n");
		break;
	}

	/* Get the number of sim cards */
	strcat( g_text, "Sim Card Count:");
	temp = vm_sim_card_count();
	switch(temp)
	{
	case 0:
		strcat(g_text, "0\n"); 
		break;
	case 1:
		strcat(g_text, "1\n");
		break;
	case 2:
		strcat(g_text, "2\n");
		break;	
	}

	/* Check whether the SIM card is inserted */
	temp = vm_has_sim_card();
	if( temp )
	{
		strcat(g_text, "SIM Card Inserted\n"); 
		 /* If has SIM card, get the IMSI of it */
		sz_num = vm_get_imsi();
		strcat(g_text, "IMSI:");
		if (sz_num)
			strcat(g_text, sz_num);
		strcat(g_text, "\n");
		
	}
	else	
		strcat(g_text, "No Sim Card\n");

 	/* get IMEI */
	sz_num = vm_get_imei();
	strcat(g_text, "IMEI:");
	if (sz_num)
		strcat(g_text, sz_num);
	strcat(g_text, "\n");
	strcat(g_text, TEXT_DEFAULT);
	// Redraw screen 
	handle_sysevt(VM_MSG_PAINT, 0);
}

static void l_timer_proc(VMINT tid)
{
	l_do_sim_check();
}

void handle_sysevt(VMINT message, VMINT param) {
	switch (message) {
	/* this message always comes after VM_MSG_CREATE or VM_MSG_PAINT message */
	case VM_MSG_PAINT:
		/* redraw screen */
		draw_screen();
		break;
	/* when the program starts, the system will send this message */
	case VM_MSG_CREATE:
		/* initialize app and start checking timer */
		layer_handle = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1);
		src_buf = vm_graphic_get_layer_buffer(layer_handle);
		g_id_timer = vm_create_timer(1000, l_timer_proc);  
		break;
	/*  after the processing of external events completed, the system will send this message */
	case VM_MSG_ACTIVE:
		/* active app and start checking timer */
		layer_handle = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1);
		src_buf = vm_graphic_get_layer_buffer(layer_handle);
		g_id_timer = vm_create_timer(1000, l_timer_proc);
		break;
	/* if there is an external event (incoming calls or SMS) occurs, the system will send this message */
	case VM_MSG_INACTIVE:
		if( layer_handle != -1 )
			vm_graphic_delete_layer(layer_handle);
		
		vm_delete_timer(g_id_timer); //delete checking timer
		break;
	/* if you press hang-up button, the system will send this message */
	case VM_MSG_QUIT:
		if( layer_handle != -1 )
			vm_graphic_delete_layer(layer_handle);

		vm_delete_timer(g_id_timer); //delete checking timer
		vm_exit_app();
		break;
	}
}

void handle_keyevt(VMINT event, VMINT keycode) {
	/* Press any key to quit */
	handle_sysevt(VM_MSG_QUIT, 0);
    vm_exit_app();
}

static void l_draw_text_muiltline(VMWCHAR *msg, VMUINT8* buf, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color)
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


void draw_screen(void) {
	VMWCHAR w_text[256];
	vm_ascii_to_ucs2(w_text, 512, g_text);
	/* fill screen */
	vm_graphic_fill_rect(src_buf,0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), 0,0);
	/* draw text in screen */
	l_draw_text_muiltline(w_text, src_buf, 0, 0, vm_graphic_get_screen_width(),
		vm_graphic_get_screen_height(), 0xabcd);
	/* flush screen */
	vm_graphic_flush_layer(&layer_handle,1);
}

