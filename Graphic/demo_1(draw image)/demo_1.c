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
#include "vmres.h"
#include "vmlog.h"
#include "vmstdlib.h"
#include <stdio.h>

/* ---------------------------------------------------------------------------
* global variables
* ------------------------------------------------------------------------ */

VMINT		layer_hdl[2];				//layer handle array

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
static void draw(void);
static void draw_image();
static void draw_info();

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
		/* cerate base layer that has same size as the screen*/
		layer_hdl[0] = vm_graphic_create_layer(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);	
		
		/* cerate fast layer */
		layer_hdl[1] = vm_graphic_create_layer(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), VM_COLOR_RED);	
		
		/* set clip area */
		vm_graphic_set_clip(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height());
		break;
		
	case VM_MSG_PAINT:
		draw();
		break;
		
	case VM_MSG_INACTIVE:
		vm_graphic_delete_layer(layer_hdl[1]); 
		vm_graphic_delete_layer(layer_hdl[0]); //must delete other layer before delete base layer
		break;
	case VM_MSG_QUIT:
		vm_graphic_delete_layer(layer_hdl[1]); 
		vm_graphic_delete_layer(layer_hdl[0]); //must delete other layer before delete base layer
		vm_exit_app();		
		break;	
	}
}

void handle_keyevt(VMINT event, VMINT keycode) {
	/* press any key to exit program */
	vm_graphic_delete_layer(layer_hdl[0]);
	
	vm_exit_app();
}

void handle_penevt(VMINT event, VMINT x, VMINT y){
	
	vm_graphic_delete_layer(layer_hdl[0]);
	
	vm_exit_app();
}


static void draw(void) {
	draw_image();
	draw_info();
	vm_graphic_flush_layer(layer_hdl, 2);	
}

/* draw image */
static void draw_image()
{
	VMUINT8 * res = NULL;	//raw image data
	VMINT  gif_size = -100; //image size 
	
	VMINT gif_hdl=-100;		//image handle
	VMUINT8 *gif_buf=NULL;  //decoded image buffer
	VMINT gif_width =-100;	//image width
	VMINT gif_height=-100;  //image height
	
	VMUINT8 * buf=NULL;     //base layer buffer
	
	res = vm_load_resource("logo_vre.gif", &gif_size);  //load image data from vxp file
	if (res == NULL)
		handle_sysevt(VM_MSG_QUIT, 1); //load resource failed, exit program
	
	
	gif_hdl = vm_graphic_load_image(res,gif_size);	//decode image data
	if (gif_hdl < 0)
		handle_sysevt(VM_MSG_QUIT, 1); //decode failed, exit program

	vm_free(res);	//free raw image data
	
	gif_buf = vm_graphic_get_canvas_buffer(gif_hdl);				//get decoded image buffer
	gif_width = vm_graphic_get_img_property(gif_hdl,1)->width;		//get image width
	gif_height = vm_graphic_get_img_property(gif_hdl,1)->height;	//get image height
	
	buf = vm_graphic_get_layer_buffer(layer_hdl[0]);    //get base layer buffer
	
	vm_graphic_fill_rect(buf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_WHITE, VM_COLOR_GREEN); //draw background
	
	vm_graphic_blt(buf,0,10,gif_buf,0,0,gif_width,gif_height,1); //draw first frame of image
	
	vm_graphic_blt(buf,0,40,gif_buf,0,0,gif_width,gif_height,2); //draw second frame of image
	
	vm_graphic_release_canvas(gif_hdl); //release decoded image buffer
}


static void draw_info()
{
	VMUINT8 * buf=NULL; 
	VMCHAR str[60]={0};
	VMWCHAR wstr[60]={0};
	
	
	VMINT x;
	VMINT y;
	VMINT width;
	VMINT height;

	VMUINT16 color =0;

	VMINT gif_hdl;
	VMUINT8 * gif_buf=NULL;

	sprintf(str,"max support %d layer", vm_graphic_get_layer_count()); //Obtain maximum numbers of layers supported by the target platform
	vm_ascii_to_ucs2(wstr, 120, str);	
	buf = vm_graphic_get_layer_buffer(layer_hdl[1]); //get fast layer buffer

	
	vm_graphic_fill_rect(buf, 0, 40, vm_graphic_get_screen_width()-50, 
		vm_graphic_get_screen_height()-50, VM_COLOR_WHITE, VM_COLOR_BLUE);

	vm_graphic_textout(buf,0,80,wstr,wstrlen(wstr),VM_COLOR_GREEN);

	vm_graphic_get_layer_position(layer_hdl[1],&x,&y,&width,&height); //get fast layer position and size

	sprintf(str,"fast layer position X£½%d,Y=%d", x,y);
	vm_ascii_to_ucs2(wstr,120,str);	
	
	vm_graphic_textout(buf,0,100,wstr,vm_wstrlen(wstr),VM_COLOR_GREEN);	
	
	sprintf(str,"fast layer size W=%d,H£½%d",width,height);
	vm_ascii_to_ucs2(wstr,120,str);	

	vm_graphic_textout(buf,0,120,wstr,vm_wstrlen(wstr),VM_COLOR_GREEN);	

   
	color=vm_graphic_get_pixel(buf, 100, 100); //get the color of specified point on fast layer
	sprintf(str,"fast layer 100.100 color: 0x%x",color);
	vm_ascii_to_ucs2(wstr,120,str);	
	vm_graphic_textout(buf,0,140,wstr,vm_wstrlen(wstr),VM_COLOR_GREEN);	

   
	gif_hdl=vm_graphic_create_canvas(100,100); //create canvas
	gif_buf= vm_graphic_get_canvas_buffer(gif_hdl); //get canvas buffer

	sprintf(str,"canvas");
	vm_ascii_to_ucs2(wstr,120,str);	
	vm_graphic_fill_rect(gif_buf, 0, 0, 100,100, VM_COLOR_WHITE, VM_COLOR_RED); //fill rect on canvas
    vm_graphic_textout(gif_buf,10,2,wstr,wstrlen(wstr),VM_COLOR_WHITE);	//write character on canvas
	vm_graphic_blt_ex(buf,20,190,gif_buf,0,0,100,100,1,150); //	semi-transparent effect
	
	vm_graphic_set_layer_opacity(layer_hdl[1],50); //set opacity of fast layer	
}