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


#include "graphic_demo_6.h"

/**
* entry
*/
void vm_main(void) {

	count_layer = vm_graphic_get_blt_layer_count();
				
	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
}

void handle_sysevt(VMINT message, VMINT param) {
	switch (message) {
	case VM_MSG_CREATE:
	case VM_MSG_ACTIVE:
		App_init();
		
		break;
		
	case VM_MSG_PAINT:
		draw_hello();
		break;
		
	case VM_MSG_INACTIVE:
		App_quit();
		break;
	case VM_MSG_QUIT:
		App_quit();
		vm_exit_app();
		
		
		break;	
	}

}

void handle_keyevt(VMINT event, VMINT keycode) {
	/* press any key to exit*/
	App_quit();
	
	vm_exit_app();
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{
	/* touch and return*/
	App_quit();
	vm_exit_app();
}


static void draw_hello(void) {
	VMWCHAR s[50];
	VMUINT8* buf;	
	int x, y, w ,i;
		VMCHAR str[20]={0};
	
	vm_graphic_color*color=NULL;
	color = (vm_graphic_color*)malloc(sizeof(vm_graphic_color));
	
	color->vm_color_565 = VM_COLOR_WHITE;
	color->vm_color_888=VM_COLOR_565_TO_888(VM_COLOR_WHITE);
	
	
	vm_graphic_setcolor(color); //set the glogal color
	vm_free(color);

	sprintf(str, "max support %d layers",count_layer);

	vm_ascii_to_ucs2(s, 100, str);
	w = vm_graphic_get_string_width(s);
	x = (vm_graphic_get_screen_width() - w) / 2;
	y = (vm_graphic_get_screen_height() - vm_graphic_get_character_height()) / 2;
	
	/* get the target buffer*/
	buf = vm_graphic_get_layer_buffer(layer_hdl[0]);		
	
	/* fill the screen*/
	vm_graphic_fill_rect(buf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_WHITE, VM_COLOR_WHITE);
	
	/* draw text */
		vm_graphic_textout(buf,	x, 280, s, wstrlen(s), VM_COLOR_BLUE);
	
	vm_graphic_fill_rect_ex(layer_hdl[0],0,0,120,120);
	
	
	for ( i=0 ; i<count_layer; i++)
	{

		VMUINT8 * buf;
		VMWCHAR wstr[50]	=	{0};

		buf = vm_graphic_get_layer_buffer(layer_hdl[i]);
	
		sprintf(str, " %d layer",i+1);

		vm_graphic_fill_rect(buf,0,0,120,120,VM_COLOR_WHITE,VM_COLOR_GREEN);
		vm_ascii_to_ucs2(wstr,50*2,str);
		vm_graphic_textout(buf,	0, 0, wstr,wstrlen(wstr), VM_COLOR_BLUE);

	}
	
	
	/* flush the screen with data in the buffer*/
	vm_graphic_flush_layer(layer_hdl, 6);
}



void App_init()
{
	VMINT i = 2;
	VMINT x=30;
	VMINT y=30;
	VMINT character=vm_graphic_get_character_height()*2;
	VMINT x_dis= vm_graphic_get_screen_width()/10;
	
	
	/* use vm_graphic_create_layer to create base layer and fast layer */
	layer_hdl[0] = vm_graphic_create_layer(0, 0, 
		vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), -1);	
	layer_hdl[1] = vm_graphic_create_layer(x, y, 
		120, 
		120, -1);
	
	
	/* use vm_graphic_create_layer_ex to create other four layers */
	for (i=2; i<count_layer; i++)
	{
		layer_hdl[i] = vm_graphic_create_layer_ex(x+=x_dis+5,y+=character,120,120, VM_COLOR_RED ,VM_CREATE_CANVAS , NULL);
		
	}
	
	
	/* set clip area*/
	vm_graphic_set_clip(0, 0, 
		vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height());
}

void App_quit()
{
	VMINT i=0;
	
	i =count_layer-1;
	
	for(i; i>0;i--)
	{
		if( layer_hdl[i] != -1 )
		{	vm_graphic_delete_layer(layer_hdl[i]);	layer_hdl[i] = -1;	}	//you should delete other layer before delete base layer	
		
	}
	
}