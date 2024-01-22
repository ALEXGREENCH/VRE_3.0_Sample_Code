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

/************************************************************************************/
/* Camera.c演示如何使用VRE应用接口来开发Camera相关的程序。							*/
/*																					*/
/* Camera.c demonstrates how to use the VRE API to develop camera-related programs.	*/
/*																					*/
/* Copyright (c) 2009 Vogins Network Technology (China). All rights reserved.		*/
/************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "vmsys.h"
#include "vmchset.h"
#include "vmgraph.h"
#include "vmstdlib.h"
#include "vmio.h"
#include "vmlog.h"

#include "Camera.h"

CAM_INFO_T g_cam;
APP_INFO_T g_app;

/* System event */
void sys_evt_procss(VMINT message, VMINT param);
/* Key event */
void sys_key_process(VMINT message, VMINT param);
/* Create the camera instance */
void create_camera_instance(void);
/* Release the camera instance */
void release_camera_instance(void);
/* Get the status of camera */
void get_camera_status(void);
/* Get the parameters of camera */
void get_camera_parameters(void);
/* Turn on the preview mode */
VMINT start_preview(void);
/* Turn off the preview mode */
VMINT stop_preview(void);
/* Take a picture */
void capture(void);
/* Set FPS */
void set_preview_fps(void);
/* Set the size of preview screen */
void set_preview_size(void);
/* Set the image size of shotting */
void set_capture_size(void);
/* callback function */
void cam_state_cb(const vm_cam_notify_data_t *notify_data, void *user_data); 

void init_app(void);
void draw_app(void);
void inactive_app(void);
void active_app(void);
void quit_app(void);

/* init log system */
void init_log(void);

void draw_text_in_specified_rect(VMWCHAR *msg, VMUINT8* buf, VMINT x, VMINT y, VMINT width, VMINT height, VMUINT16 color);
void draw_soft_key_bar(VMSTR left_str, VMSTR right_str);

void draw_menu(void);

void draw_preview(void);
/* Restart the preview mode */
void restart_preview(VMINT timer_id);

void vm_main(void) {
	vm_reg_sysevt_callback(sys_evt_procss);
	vm_reg_keyboard_callback(sys_key_process);
}

void sys_evt_procss(VMINT message, VMINT param) {
	switch (message)
	{
	/* when the program starts, the system will send this message */
	case VM_MSG_CREATE:	
		init_app();
		break;
	/* this message always comes after VM_MSG_CREATE or VM_MSG_PAINT message */
	case VM_MSG_PAINT:
		draw_app();
		break;	
	/* if there is an external event (incoming calls or SMS) occurs, the system will send this message */
	case VM_MSG_INACTIVE:
		inactive_app();
		break;
	/* after the processing of external events completed, the system will send this message */
	case VM_MSG_ACTIVE:
		active_app();
		break;
	/* if you press hang-up button, the system will send this message */
	case VM_MSG_QUIT:
		quit_app();
		break;		
	}
}

void sys_key_process(VMINT message, VMINT param) {
	VMCHAR text[64] = {0};
	if (message == VM_KEY_EVENT_DOWN) {
		if(STATE_MENU == g_app.app_state)
		{
			switch(param)
			{
			/**
			 * If press the left soft key, 
			 * according to whether the camera instance is existed,
			 * it will create or release the camera instance 
			 */
			case VM_KEY_LEFT_SOFTKEY:
				if (g_cam.camera_handle < 0)
					create_camera_instance();
				else
					release_camera_instance();
				break;	
			/**
			 * If press the left key or right key, 
			 * it will change the preview screen size 
			 */
			case VM_KEY_LEFT:
				if (g_cam.camera_handle >= 0 && g_cam.support_preview_size_num > 0) {
					g_app.preview_size_index = (g_app.preview_size_index - 1 + g_cam.support_preview_size_num) % g_cam.support_preview_size_num;
					vm_camera_set_preview_size(g_cam.camera_handle, g_cam.support_preview_size + g_app.preview_size_index);
					strcpy(g_app.text, "change the size of preview screen as\n");
					sprintf(text, "width:%dpx, height:%dpx\n", (g_cam.support_preview_size + g_app.preview_size_index)->width, (g_cam.support_preview_size + g_app.preview_size_index)->height);
					strcat(g_app.text, text);
				}
				break;
			case VM_KEY_RIGHT:
				if (g_cam.camera_handle >= 0 && g_cam.support_preview_size_num > 0) {
					g_app.preview_size_index = (g_app.preview_size_index + 1 + g_cam.support_preview_size_num) % g_cam.support_preview_size_num;
					vm_camera_set_preview_size(g_cam.camera_handle, g_cam.support_preview_size + g_app.preview_size_index);
					strcpy(g_app.text, "change the size of preview screen as\n");
					sprintf(text, "width:%dpx, height:%dpx\n", (g_cam.support_preview_size + g_app.preview_size_index)->width, (g_cam.support_preview_size + g_app.preview_size_index)->height);
					strcat(g_app.text, text);
				}
				break;
			/**
			 * If press the up key or down key, 
			 * it will change the preview FPS
			 */
			case VM_KEY_UP:
				if (g_cam.camera_handle >= 0 && g_cam.preview_fps < 30) {
					g_cam.preview_fps += 5;
					vm_camera_set_preview_fps(g_cam.camera_handle, g_cam.preview_fps);
					strcpy(g_app.text, "change the FPS as ");
					sprintf(text, "%d\n", g_cam.preview_fps);
					strcat(g_app.text, text);
				}
				break;
			case VM_KEY_DOWN:
				if (g_cam.camera_handle >= 0 && g_cam.preview_fps > 5) {
					g_cam.preview_fps -= 5;
					vm_camera_set_preview_fps(g_cam.camera_handle, g_cam.preview_fps);
					strcpy(g_app.text, "change the FPS as ");
					sprintf(text, "%d\n", g_cam.preview_fps);
					strcat(g_app.text, text);
				}
				break;
			/**
			 * If press the OK key, 
			 * it will open the preview mode
			 */
			case VM_KEY_OK:
				if (VM_CAM_SUCCESS == start_preview())
					g_app.app_state = STATE_PREVIEW;
				break;
			/**
			 * If press the right soft key, 
			 * it will exit the application
			 */
			case VM_KEY_RIGHT_SOFTKEY:
				quit_app();
				break;
			}
		}
		else if(STATE_PREVIEW == g_app.app_state)
		{
			switch(param)
			{
			/**
			 * If press the right soft key, 
			 * it will turn off the preview mode
			 */
			case VM_KEY_RIGHT_SOFTKEY:
				if (VM_CAM_SUCCESS == stop_preview())
					g_app.app_state = STATE_MENU;
				break;
			/**
			 * If press the left soft key, 
			 * it will take a picture
			 */
			case VM_KEY_LEFT_SOFTKEY:
				capture();
				break;
			}
		}		
	draw_app();
	}
}

void init_app(void) {
	init_log();

	/* Create the base layer */
	if ((g_app.handle = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1)) != 0)
	{
		vm_exit_app();
		return;
	}
	/* Get the buffer of base layer */
	if ((g_app.buffer = vm_graphic_get_layer_buffer(g_app.handle)) == NULL) {
		vm_graphic_delete_layer(g_app.handle);		
		vm_exit_app();
		return;
	}
	
	g_app.app_state = STATE_MENU;
	g_cam.camera_id = VM_CAMERA_MAIN_ID;
	g_cam.camera_handle = -1;
	g_cam.preview_fps = 15;
	g_app.screen_width = vm_graphic_get_screen_width();
	g_app.screen_height = vm_graphic_get_screen_height();
	g_app.character_height = vm_graphic_get_character_height();
}

void active_app(void) {
	/* Create the base layer */
	if ((g_app.handle = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1)) != 0)
	{
		vm_exit_app();
		return;
	}
	/* Get the buffer of base layer */
	if ((g_app.buffer = vm_graphic_get_layer_buffer(g_app.handle)) == NULL){
		vm_graphic_delete_layer(g_app.handle);		
		vm_exit_app();
		return;
	}	
	if(STATE_PREVIEW == g_app.app_state) {
		if (VM_CAM_SUCCESS != start_preview())
			g_app.app_state = STATE_MENU;
	}
}

void inactive_app(void) {	
	vm_graphic_delete_layer(g_app.handle);
	g_app.buffer = NULL;
	if(STATE_PREVIEW == g_app.app_state) {
		stop_preview();
	}
}

void quit_app(void) {
	stop_preview();
	release_camera_instance();

	vm_graphic_delete_layer(g_app.handle);
	g_app.buffer = NULL;
	vm_exit_app();
}

void draw_app(void)
{
	switch(g_app.app_state)	{
	/* draw the main menu */
	case STATE_MENU:
		draw_menu();
		break;
	/* draw the preview screen */
	case STATE_PREVIEW:
		draw_preview();
		break;
	}
}

void create_camera_instance(void) {
	VMINT result;
	if (g_cam.camera_handle > 0) {
		strcpy(g_app.text, "The camera instance is existed");	
		return;
	}

	/* Create the camera instance */
	result = vm_create_camera_instance(g_cam.camera_id, &g_cam.camera_handle);

	if(VM_CAM_SUCCESS == result) {			
		strcpy(g_app.text, "Create camera instance success\n");

		/* Register the callback function */
		result = vm_camera_register_notify(g_cam.camera_handle, cam_state_cb, "");
		
		if(VM_CAM_SUCCESS == result) {
			strcat(g_app.text, "Register callback function success\n");
			get_camera_parameters();
		}
		else {
			strcat(g_app.text, "Failed to register callback function\n");
		}			
	}
	else {
		strcpy(g_app.text, "Failed to create camera instance\n");	
	}				
}

void get_camera_parameters(void) {
	VMINT result;
	VMUINT i = 0;	
	VMCHAR text[64] = {0};

	if (g_cam.camera_handle < 0) {
		strcpy(g_app.text, "The camera instance is not exist\n");
		return;
	}

	/* Get the maximum supported size of shotting */
	result = vm_camera_get_max_capture_size(g_cam.camera_handle, &g_cam.max_capture_size);

	if (VM_CAM_SUCCESS == result) {
		strcpy(g_app.text, "The maximum supported size of shotting is:\n");
		sprintf(text, "width:%dpx, height:%dpx\n", g_cam.max_capture_size.width, g_cam.max_capture_size.height);
		strcat(g_app.text, text);
	}
	else {
		strcpy(g_app.text, "Failed to get the maximum supported size of shotting\n");
	}

	/* Get the supported size of preview screen */
	result = vm_camera_get_support_preview_size(g_cam.camera_handle, &g_cam.support_preview_size, &g_cam.support_preview_size_num);
	
	if (VM_CAM_SUCCESS == result) {	
		strcat(g_app.text, "Supported size:\n");

		for (i = 0; i < g_cam.support_preview_size_num; i++)
		{
			memset(text, 0, 64);
			sprintf(text, "width:%dpx, height:%dpx\n", (g_cam.support_preview_size + i)->width, (g_cam.support_preview_size + i)->height);
			strcat(g_app.text, text);
		}
	}
	else {
		strcat(g_app.text, "Failed to get the supported size of preview screen\n");
	}
}

void release_camera_instance(void) {	
	VMINT result;
	if(g_cam.camera_handle < 0) {
		strcpy(g_app.text, "There is no existed camera instance\n");
		return;
	}
	
	/* Release the camera instance */
	result = vm_release_camera_instance(g_cam.camera_handle);

	if (VM_CAM_SUCCESS == result) {
		g_cam.camera_handle = -1;
		strcpy(g_app.text, "Release camera instance success\n");	
	}
	else {
		strcpy(g_app.text, "Failed to release camera instance\n");			
	}
}

void draw_softkey(VMSTR left_str, VMSTR right_str) {
	VMINT pos_y;
	VMWCHAR wstr[50] = {0};
	
	/* draw the background color */
	vm_graphic_fill_rect(g_app.buffer, 0, g_app.screen_height - g_app.character_height - 4,
		g_app.screen_width, g_app.character_height + 4, VM_COLOR_WHITE, VM_COLOR_WHITE);
	pos_y = g_app.screen_height - g_app.character_height - 2;	
	/* draw the left soft key */
	if(left_str != NULL) {
		vm_ascii_to_ucs2(wstr, 50, left_str);
		vm_graphic_textout(g_app.buffer, 2, pos_y, wstr, wstrlen(wstr), 0xabcd);
	}
	/* draw the right soft key */
	if(right_str != NULL) {
		vm_ascii_to_ucs2(wstr, 50, right_str);
		vm_graphic_textout(g_app.buffer, g_app.screen_width - 2 - vm_graphic_get_string_width(wstr),
			pos_y, wstr, wstrlen(wstr), 0xabcd);
	}	
}

void draw_menu(void)
{
	VMCHAR str[][32]={
		"Left/Right:Adjust Size",
		"Up/Down:Adjust FPS",
		"OK:Start Preview"
	};
	VMINT i;

	VMWCHAR wstr[32] = {0};

	/* draw the background color */
	vm_graphic_fill_rect(g_app.buffer, 0, 0, g_app.screen_width, g_app.screen_height - g_app.character_height - 4, 0, 0);
	/* draw menu item */
	for(i = 0; i < sizeof(str) / sizeof(str[0]); i++) {
		memset(wstr, 0, sizeof(wstr));
		vm_ascii_to_ucs2(wstr, sizeof(wstr), str[i]);
		vm_graphic_textout(g_app.buffer, 2, 10 + i * g_app.character_height, wstr, 
							vm_wstrlen(wstr), VM_COLOR_888_TO_565(192,192,192));
	}
	
	/* draw the results of operation */
	if(g_app.text) {
		VMWCHAR w_text[512];
		vm_ascii_to_ucs2(w_text, sizeof(w_text), g_app.text);
		draw_text_in_specified_rect(w_text, g_app.buffer, 2, 10 + i * g_app.character_height,
							g_app.screen_width, g_app.screen_height - g_app.character_height - 4 - i * g_app.character_height,
							VM_COLOR_888_TO_565(192,192,192));
	}

	/* draw the soft key bar */
	if (g_cam.camera_handle > 0) {
		draw_softkey("Release", "Exit");	
	} else {
		draw_softkey("Create", "Exit");
	}	
	
	/* flush screen */
	vm_graphic_flush_layer(&g_app.handle, 1);	
}

void draw_preview(void)
{
	VMINT y = (g_cam.support_preview_size + g_app.preview_size_index)->height;
	/* draw the background color */
	vm_graphic_fill_rect(g_app.buffer, 0, y, g_app.screen_width, g_app.screen_height - g_app.character_height - 4 - y, VM_COLOR_BLACK, VM_COLOR_BLACK);
	
	/* draw the results of operation */
	if(g_app.text) {
		VMWCHAR w_text[512];
		vm_ascii_to_ucs2(w_text, sizeof(w_text), g_app.text);
		draw_text_in_specified_rect(w_text, g_app.buffer, 0, y, g_app.screen_width, g_app.screen_height - g_app.character_height - 4 - y, VM_COLOR_888_TO_565(192,192,192));
	}

	/* draw the soft key bar */
	draw_softkey("Shot", "Stop");	
	
	/* flush screen */
	vm_graphic_flush_layer(&g_app.handle, 1);	
}

VMINT start_preview(void) {
	VMINT result;

	if (vm_get_camera_status(g_cam.camera_handle, &g_cam.camera_status) < 0) {
		strcpy(g_app.text, "Get the status of camera failure\n");
		return -99;
	}

	if (VM_CAM_READY_STATUS != g_cam.camera_status) {
		strcpy(g_app.text, "abnormal status\n");
		return -99;
	}

	vm_camera_set_preview_fps(g_cam.camera_handle, g_cam.preview_fps);

	vm_camera_set_preview_size(g_cam.camera_handle, g_cam.support_preview_size + g_app.preview_size_index);

	result = vm_camera_preview_start(g_cam.camera_handle);

	if (VM_CAM_SUCCESS == result) {
		strcpy(g_app.text, "Open the preview mode success\n");
	}
	else {
		strcpy(g_app.text, "Open the preview mode failure\n");
	}

	return result;
}

VMINT stop_preview(void) {
	VMINT result = -99;
	
	if (vm_get_camera_status(g_cam.camera_handle, &g_cam.camera_status) < 0) {
		strcpy(g_app.text, "Get the status of camera failure\n");
		return result;
	}

	if (VM_CAM_PREVIEW_STATUS == g_cam.camera_status) {
		result = vm_camera_preview_stop(g_cam.camera_handle);
		if (VM_CAM_SUCCESS == result) {
			strcpy(g_app.text, "Stop the preview mode success\n");		
		}
		else {
			strcpy(g_app.text, "Stop the preview mode failure\n");		
		}
	} else {
		strcpy(g_app.text, "abnormal status\n");
	}

	return result;
}

void capture(void) {
	VMINT result;

	if (vm_get_camera_status(g_cam.camera_handle, &g_cam.camera_status) < 0) {
		strcpy(g_app.text, "Get the status of camera failure\n");
		return;
	}
	if (VM_CAM_PREVIEW_STATUS == g_cam.camera_status) {

		vm_camera_set_capture_size(g_cam.camera_handle, &g_cam.max_capture_size);
		
		result = vm_camera_capture(g_cam.camera_handle);
		
		if (VM_CAM_SUCCESS == result) {
			strcpy(g_app.text, "Shooting success\n");
		}
		else {
			strcpy(g_app.text, "Shooting failure\n");
		}
	} else {
		strcpy(g_app.text, "abnormal status\n");
	}
}

void init_log(void) {
	VMCHAR log_file[128] = {0};
	sprintf(log_file, "%c:\\camera.log", vm_get_removable_driver() < 0 ? vm_get_system_driver() : vm_get_removable_driver());
	vm_log_init(log_file, VM_ERROR_LEVEL);
}

void cam_state_cb(const vm_cam_notify_data_t* notify_data, void *user_data)
{
	VMINT result;
	VMCHAR text[64] = {0};
	VMINT file_handle;
	char file_name[100] = {0};
	short file_wname[100] = {0};
	VMUINT written;
 	if(notify_data != NULL) {	
		switch (notify_data->cam_status) {
 		case VM_CAM_READY_STATUS:
			strcpy(g_app.text, "Camera instance is created\n");	
 			break;
		case VM_CAM_STARTING_PREVIEW_STATUS:
			strcpy(g_app.text, "Preview mode is starting\n");
			break;
		case VM_CAM_STOPPING_PREVIEW_STATUS:
			strcpy(g_app.text, "Preview mode is stopping\n");
			break;
		case VM_CAM_PREVIEW_STATUS:
			strcpy(g_app.text, "previewing...\n");
			break;
		case VM_CAM_CAPTURING_STATUS:
			strcpy(g_app.text, "shooting...\n");
			break;
		}

 		switch (notify_data->cam_message) {
		case VM_CAM_PREVIEW_START_ABORT:
			strcat(g_app.text, "Start preview mode failure\n");
			break;
		case VM_CAM_PREVIEW_STOP_DONE:
			strcat(g_app.text, "Preview mode is stopped\n");
			break;
		case VM_CAM_PREVIEW_STOP_ABORT:
			strcat(g_app.text, "Stop preview mode failure\n");
			break;
 		case VM_CAM_PREVIEW_FRAME_RECEIVED:
			/* Get preview data */
			if (VM_CAM_SUCCESS == (result = vm_camera_get_frame(g_cam.camera_handle, &g_cam.frame_data))) {				
				VMUINT app_frame_data_size;
				if (g_cam.frame_data.pixtel_format == PIXTEL_RGB565 || g_cam.frame_data.pixtel_format == PIXTEL_BGR565)
 					app_frame_data_size = g_cam.frame_data.row_pixel * g_cam.frame_data.col_pixel * 2;
 				else if (g_cam.frame_data.pixtel_format == PIXTEL_RGB888 || g_cam.frame_data.pixtel_format == PIXTEL_BGR888)
 					app_frame_data_size = g_cam.frame_data.row_pixel * g_cam.frame_data.col_pixel * 3;
 				else
 					app_frame_data_size = g_cam.frame_data.row_pixel * g_cam.frame_data.col_pixel * 4;
				/* Copy the preview data to layer buffer */
 				memcpy(g_app.buffer, g_cam.frame_data.pixtel_data, app_frame_data_size);
				strcat(g_app.text, "Get preview data success\n");
			}
			else {
				strcat(g_app.text, "Get preview data failure\n");
			}
 			break;
		case VM_CAM_CAPTURE_DONE:
			/* Get shooting data */
			if (VM_CAM_SUCCESS == (result = vm_camera_get_capture_data(g_cam.camera_handle, &g_cam.capture_data)))
			{
				/* set the path and file name to save the picture */
				sprintf(file_name, "%c:\\capture%d.%s", ((vm_get_removable_driver() >= 0) ? vm_get_removable_driver() : vm_get_system_driver()), g_app.capture_num++, g_cam.capture_data.mime_type);
				vm_ascii_to_ucs2(file_wname, 100, file_name);
				/* write the shooting data to file */
				if ((file_handle = vm_file_open(file_wname, MODE_CREATE_ALWAYS_WRITE, TRUE)) > 0) {
					vm_file_write(file_handle, g_cam.capture_data.data, g_cam.capture_data.data_size, &written);
					vm_file_commit(file_handle);
					vm_file_close(file_handle);
				}
				strcat(g_app.text, "write file success\n");
				sprintf(text, "picture size: %dbytes\n", g_cam.capture_data.data_size);
				strcat(g_app.text, text);
			}
			else {
				strcat(g_app.text, "write file failure \n");
			}
			vm_create_timer(100, restart_preview);
			break;
		case VM_CAM_CAPTURE_ABORT:
			strcat(g_app.text, "Shooting failure\n");
			break;
		}
 	}
	draw_app();
 }

void restart_preview(VMINT timer_id) {
	vm_delete_timer(timer_id);
	start_preview();
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

