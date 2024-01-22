#include <stdio.h>
#include <string.h>

#include "vmio.h"
#include "vmgraph.h"
#include "vmlog.h"
#include "vmchset.h"
#include "vmstdlib.h"

#include "Audio.h"
#include "Engine.h"

#define MILLISECOND_PER_HOUR (60 * 60 * 1000)
#define MILLISECOND_PER_MINUTE (60 * 1000)
#define MILLISECOND_PER_SECOND (1000)

static VMINT layer_hdl;
static VMCHAR file_name[256];		
static VMINT file_hdl;				
static VMUINT file_size;			

static VMINT audio_state;			
static VMUINT audio_play_time;		
static VMINT UI_timer;				

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

void draw_UI(VMINT timer_id);
void audio_cb(VMUINT play_time);

void vm_main(void)
{
	VMINT drv = 0;
	VMCHAR log_file[256] = {0};

	drv = vm_get_removable_driver();
	if (drv < 0)
		drv = vm_get_system_driver();

	sprintf(log_file, "%c:\\bitstream.log", drv);	//log file path
	vm_log_init(log_file, VM_ERROR_LEVEL);

	sprintf(file_name, "%c:\\audio.mp3", drv);  //audio file path

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
}

void handle_sysevt(VMINT message, VMINT param)
{
	switch (message) {
	case VM_MSG_CREATE:
	case VM_MSG_ACTIVE:
		break;
	case VM_MSG_PAINT:
		layer_hdl = vm_graphic_create_layer(0, 0, 
			vm_graphic_get_screen_width(), 
			vm_graphic_get_screen_height(), -1);
		UI_timer = vm_create_timer(50, draw_UI);
		break;
	/* external event interrupt, stop timer and delete layer */
	case VM_MSG_INACTIVE:
		vm_delete_timer(UI_timer);
		vm_graphic_delete_layer(layer_hdl);
		break;	
	case VM_MSG_QUIT:
		vm_delete_timer(UI_timer);
		vm_graphic_delete_layer(layer_hdl);
		vm_exit_app();
		break;
	}
}

void handle_keyevt(VMINT event, VMINT keycode) {
	
	if (event == VM_KEY_EVENT_DOWN)
	{
		VMINT ret;
		/* if press left soft key, start play audio */
		if (keycode == VM_KEY_LEFT_SOFTKEY)
		{	
			VMWCHAR w_file_name[256] = {0};

			vm_ascii_to_ucs2(w_file_name, 512, file_name);
			file_hdl = vm_file_open(w_file_name, MODE_READ, TRUE);
			if (file_hdl < 0)
			{
				return;
			}

			vm_file_getfilesize(file_hdl, &file_size);

			ret = ba_audio_play(file_hdl, file_size, audio_cb);
			if (ret < 0)
			{
				return;
			}			
		}
		/* if press right soft key, stop play audio */
		else if (keycode == VM_KEY_RIGHT_SOFTKEY)
		{
			vm_file_close(file_hdl);

			ret = ba_audio_stop();
			if (ret < 0)
			{
				return;
			}			
		}
	}
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{
	if (event == VM_PEN_EVENT_TAP) 
	{
		if (x < 40 && y > 300) 
		{
			handle_keyevt(VM_KEY_EVENT_DOWN, VM_KEY_LEFT_SOFTKEY);
		}
		else if (x > 200 && y >300)
		{
			handle_keyevt(VM_KEY_EVENT_DOWN, VM_KEY_RIGHT_SOFTKEY);
		}
	}
}

void draw_UI(VMINT timer_id)
{
	VMUINT8* layer_buf;
	VMINT screen_width;
	VMINT screen_height;
	VMINT character_height;
	VMCHAR txt[20] = {0};
	VMWCHAR w_txt[20] = {0};
	VMUINT current_time;
	VMUINT hour;
	VMUINT min;
	VMUINT second;

	layer_buf = vm_graphic_get_layer_buffer(layer_hdl);
	screen_width = vm_graphic_get_screen_width();
	screen_height = vm_graphic_get_screen_height();
	character_height = vm_graphic_get_character_height();
	
	vm_graphic_fill_rect(layer_buf, 0, 0, screen_width, screen_height, VM_COLOR_BLACK, VM_COLOR_BLACK);

	memset(txt, 0, 20);
	switch(ba_get_audio_state())
	{
	vm_log_fatal("state = %d", ba_get_audio_state());
	case AUDIO_STOPPED:
		sprintf(txt, "AUDIO_STOPPED");
		break;
	case AUDIO_OPENED:
		sprintf(txt, "AUDIO_OPENED");
		break;
	case AUDIO_PLAYING:
		sprintf(txt, "AUDIO_PLAYING");
		break;
	case AUDIO_BUFFERING:
		sprintf(txt, "AUDIO_BUFFERING");
		break;
	case AUDIO_CLOSED:
		sprintf(txt, "AUDIO_CLOSED");
		break;
	}

	vm_ascii_to_ucs2(w_txt, 40, txt);

	vm_graphic_textout(layer_buf, 0, 0, w_txt, wstrlen(w_txt), VM_COLOR_WHITE);	

	current_time = audio_play_time;
	hour = current_time / MILLISECOND_PER_HOUR;
	current_time = current_time % MILLISECOND_PER_HOUR;
	min = current_time / MILLISECOND_PER_MINUTE;
	current_time = current_time % MILLISECOND_PER_MINUTE;
	second = current_time / MILLISECOND_PER_SECOND;
	memset(txt, 0, 20);
	sprintf(txt, "%02d:%02d:%02d", hour, min, second);	
	
	vm_ascii_to_ucs2(w_txt, 40, txt);
	
	vm_graphic_textout(layer_buf, (screen_width - vm_graphic_get_string_width(w_txt)) >> 1, screen_height - character_height, w_txt, wstrlen(w_txt), VM_COLOR_WHITE);
	vm_graphic_textout(layer_buf, 0, screen_height - character_height, (VMWSTR)(L"start"), strlen("start"), VM_COLOR_WHITE);
	vm_graphic_textout(layer_buf, screen_width - vm_graphic_get_string_width((VMWSTR)(L"stop")), screen_height - character_height, (VMWSTR)(L"stop"), strlen("stop"), VM_COLOR_WHITE);

	vm_graphic_flush_layer(&layer_hdl, 1);
}

void audio_cb(VMUINT play_time)
{
	audio_play_time = play_time;
}