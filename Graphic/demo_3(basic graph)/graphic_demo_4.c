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

#include "graphic_demo_4.h"

VMINT layer_hdl[1];

/**
 * entry
 */
void vm_main(void) {
	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
}

void handle_sysevt(VMINT message, VMINT param) {
	switch (message) {
		case VM_MSG_CREATE:
		case VM_MSG_ACTIVE:
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
			draw_hello();
			break;

		case VM_MSG_INACTIVE:
			vm_graphic_delete_layer(layer_hdl[0]);
		case VM_MSG_QUIT:
			vm_graphic_delete_layer(layer_hdl[0]);
			vm_exit_app();
			break;	
	}
}

void handle_keyevt(VMINT event, VMINT keycode) {
	/* press any key and return*/
	if( layer_hdl[0] != -1 )
	{
		vm_graphic_delete_layer(layer_hdl[0]);
		layer_hdl[0] = -1;
	}

	vm_exit_app();
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
	VMINT res =-10;

	vm_graphic_point test_graphic_point[8] ;
	VMINT SW =vm_graphic_get_screen_width();
	VMINT SH =vm_graphic_get_screen_height();


	vm_graphic_color* color;


	 VMBYTE bitvalues[] = {1,0,0,0,1};
	 vm_graphic_pen pen;
	 pen.cycle = sizeof(bitvalues);
     pen.bitvalues = bitvalues;



	 res = vm_graphic_setpen(&pen);

	 pen = vm_graphic_getpen();



	color = (vm_graphic_color*)malloc(sizeof(vm_graphic_color));

	memset(test_graphic_point,0,sizeof(vm_graphic_point) * 8 ) ;
	
	vm_ascii_to_ucs2(s, 50, "Hello, world!");
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height()) / 2;
	
	/* get the target buffer*/
	buf = vm_graphic_get_layer_buffer(layer_hdl[0]);		
	
	/* fill the screen*/
	vm_graphic_fill_rect(buf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_WHITE, VM_COLOR_WHITE);
	
	//vm_graphic_layer_set_trans_color(layer_hdl[0], VM_COLOR_RED);

	/* draw text */
	vm_graphic_textout_to_layer(layer_hdl[0],	x, y, s, wstrlen(s));



	vm_graphic_line_ex(layer_hdl[0] ,100 ,100 ,100 ,180);	
	
	vm_graphic_line_style(layer_hdl[0] ,120 ,100 ,120 ,180); 



	test_graphic_point[0].x = 10 ; test_graphic_point[0].y = 10 ;
	test_graphic_point[1].x = 50 ; test_graphic_point[1].y = 10 ;
	test_graphic_point[2].x = 10 ; test_graphic_point[2].y = 60 ;
	vm_graphic_fill_polygon(layer_hdl[0],test_graphic_point,3);


	color->vm_color_565 = VM_COLOR_GREEN ;
	color->vm_color_888 = VM_COLOR_565_TO_888(VM_COLOR_GREEN);
	 vm_graphic_setcolor(color);
	test_graphic_point[0].x = 10+50 ; test_graphic_point[0].y = 10 ;
	test_graphic_point[1].x = 50+50 ; test_graphic_point[1].y = 10 ;
	test_graphic_point[2].x = 10+50 ; test_graphic_point[2].y = 60 ;
	vm_graphic_polygon(layer_hdl[0],test_graphic_point,3);



	test_graphic_point[0].x = 10 ; test_graphic_point[0].y = 70 ;
	test_graphic_point[1].x = 50 ; test_graphic_point[1].y = 70 ;
	test_graphic_point[2].x = 50 ; test_graphic_point[2].y = 100 ;
	test_graphic_point[3].x = 10 ; test_graphic_point[3].y = 100 ;
	vm_graphic_fill_polygon(layer_hdl[0],test_graphic_point,4);





	color->vm_color_565 = VM_COLOR_BLUE ;
	color->vm_color_888 = VM_COLOR_565_TO_888(VM_COLOR_BLUE);
	 vm_graphic_setcolor(color);
	test_graphic_point[0].x = 10 ; test_graphic_point[0].y = 110 ;
	test_graphic_point[1].x = 50 ; test_graphic_point[1].y = 110 ;
	test_graphic_point[2].x = 50 ; test_graphic_point[2].y = 150 ;
	test_graphic_point[3].x = 30 ; test_graphic_point[3].y = 180 ;
	test_graphic_point[4].x = 10 ; test_graphic_point[4].y = 150 ;
	vm_graphic_fill_polygon(layer_hdl[0],test_graphic_point,5);


	

	color->vm_color_565 = VM_COLOR_RED ;
	 color->vm_color_888 = VM_COLOR_565_TO_888(VM_COLOR_RED);
	 vm_graphic_setcolor(color);
	test_graphic_point[0].x = 20 ; test_graphic_point[0].y = 200 ;
	test_graphic_point[1].x = 90 ; test_graphic_point[1].y = 200 ;
	test_graphic_point[2].x = 110 ; test_graphic_point[2].y = 250 ;
	test_graphic_point[3].x = 90 ; test_graphic_point[3].y = 300 ;
	test_graphic_point[4].x = 20 ; test_graphic_point[4].y = 300  ;
	test_graphic_point[5].x = 0 ; test_graphic_point[5].y = 250 ;
	vm_graphic_fill_polygon(layer_hdl[0],test_graphic_point,6);

	test_graphic_point[0].x = 20+120 ; test_graphic_point[0].y = 200 ;
	test_graphic_point[1].x = 90+120 ; test_graphic_point[1].y = 200 ;
	test_graphic_point[2].x = 110+120 ; test_graphic_point[2].y = 250 ;
	test_graphic_point[3].x = 90+120 ; test_graphic_point[3].y = 300 ;
	test_graphic_point[4].x = 20+120 ; test_graphic_point[4].y = 300  ;
	test_graphic_point[5].x = 0+120 ; test_graphic_point[5].y = 250 ;
	vm_graphic_polygon(layer_hdl[0],test_graphic_point,6);

	/* flush the screen with data in the buffer*/
	vm_graphic_flush_layer(layer_hdl, 1);
}

