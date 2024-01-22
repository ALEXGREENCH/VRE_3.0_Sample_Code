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

#include "GIF_DEMO.h"


VMINT		layer_hdl[2];				////layer handle array.
VMUINT8* buf=NULL;
VMINT coordinate_x,coordinate_y;
VMUINT8* img=NULL;
VMINT hanvas=-1;

VMINT height1 =0;
VMINT width1 =0;
void gif();

/**
* entry
*/
void vm_main(void) {
	layer_hdl[0] = -1;
	
	init_img();
	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
	
	
	
}

void handle_sysevt(VMINT message, VMINT param) {	
/* The application updates the screen when receiving the message VM_MSG_PAINT 
*  what is sent after the application is activated. The application can skip 
*  the process on screen when the VM_MSG_ACTIVE or VM_MSG_INACTIVE is received.
	*/
	
	switch (message) {
	case VM_MSG_CREATE:
	case VM_MSG_ACTIVE:
		/* cerate base layer that has same size as the screen*/
		layer_hdl[0] = vm_graphic_create_layer(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);	
		
		
		layer_hdl[1] =vm_graphic_create_layer_ex(0, 0, 80,80,
			VM_COLOR_RED ,VM_FAST_LAYER ,NULL);
		
		/* set clip area */
		vm_graphic_set_clip(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height());
		break;
		
	case VM_MSG_PAINT:
		gif();
		break;			
	case VM_MSG_INACTIVE:
		vm_graphic_delete_layer(layer_hdl[1]);	
		vm_graphic_delete_layer(layer_hdl[0]);
		break;
	case VM_MSG_QUIT:
		vm_graphic_delete_layer(layer_hdl[1]);	
		vm_graphic_delete_layer(layer_hdl[0]);
		vm_graphic_release_canvas(hanvas);
		vm_exit_app();
		
		break;	
	}
	
}

void handle_keyevt(VMINT event, VMINT keycode) {
	/* press any key and return*/
	static VMINT num=0;
	
	if (event == VM_KEY_EVENT_UP)
	{
		num = (num + 1) % 4;
		switch(num)
		{
		case 0:
			vm_graphic_rotate_layer(layer_hdl[1] , VM_GDI_ROTATE_90);
			break;
		case 1:
			vm_graphic_rotate_layer(layer_hdl[1] , VM_GDI_ROTATE_180);
			break;
		case 2:
			vm_graphic_rotate_layer(layer_hdl[1] , VM_GDI_ROTATE_270);
			break;
		case 3:
			vm_graphic_rotate_layer(layer_hdl[1] , VM_GDI_ROTATE_NORMAL);
		}
		vm_graphic_flush_layer(layer_hdl, 2);		
	}
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{
	/* touch and return*/
	
	if (event == VM_PEN_EVENT_MOVE)
	{
		coordinate_x = x;
		coordinate_y = y;
		vm_graphic_translate_layer(layer_hdl[1], coordinate_x, coordinate_y);	
		vm_graphic_flush_layer(layer_hdl, 2);
	}
}



void gif()
{	
	
	/* get the target buffer*/
	buf = vm_graphic_get_layer_buffer(layer_hdl[0]);	
	
	/* fill the screen*/
	vm_graphic_fill_rect(buf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_WHITE, VM_COLOR_WHITE);
	
	buf = vm_graphic_get_layer_buffer(layer_hdl[1]);	
	
	vm_graphic_blt(buf,0,0,img,0,0,width1,height1,1);
	
	vm_graphic_flush_layer(layer_hdl,2); 
}

void init_img()
{
	VMUINT8 *tempPoint=NULL; 			
	VMINT NUMBer=-1;

	VMINT size=-1;
	
	tempPoint= vm_load_resource("transformer.gif", &size); //load image data from vxp file
	if (tempPoint == NULL )
	{
		handle_sysevt(VM_MSG_QUIT,0);	//if load failed, exit program
	}
	
	hanvas= vm_graphic_load_image(tempPoint, size);	 //decode image data
	img = vm_graphic_get_canvas_buffer(hanvas);	//get decoded image buffer	 
	
	vm_free(tempPoint);	

	/* get image property */
	height1= (vm_graphic_get_img_property(hanvas, 1)->height);	
	width1 = (vm_graphic_get_img_property(hanvas, 1)->width);	
	
}