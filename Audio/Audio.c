/* ============================================================================
 * VRE GRAPHIC Library demonstration application.
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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "vmsys.h"
#include "vmio.h"
#include "vmchset.h"
#include "vmres.h"
#include "vmgraph.h"
#include "vmlog.h"
#include "vmmm.h"
#include "vmstdlib.h"
#include "Audio.h"

APP_INFO_T g_app;

RES_INFO_T g_audio_res[RES_NUM];

/*
* system events
*/
static void	sys_evt_process(VMINT message, VMINT param);

/*
* key events
*/
static void	sys_key_process(VMINT message, VMINT param);

/*
* pen events
*/
static void	sys_pen_process(VMINT event, VMINT x, VMINT y);

void draw_app(void);
void draw_memu(void);
void draw_play_res(void);
void draw_play_file(void);
void draw_record(void);
void init_app(void);
void inactive_app(void);
void active_app(void);
void quit_app(void);
void menu_key_proc(VMINT message, VMINT param);
void res_play_key_proc(VMINT message, VMINT param);
void file_play_key_proc(VMINT message, VMINT param);
void record_key_proc(VMINT message, VMINT param);
VMINT play_res(void);
VMINT play_file(void);
VMINT record(void);
void load_res(VMINT start, VMINT end);
void un_load_res(VMINT start, VMINT end);
void play_file_cb(VMINT result);
void play_res_cb(VMINT handle, VMINT event);
void get_time(VMINT timer_id);
void trans_time(VMINT millisecond, VMINT* hour, VMINT* minute, VMINT* second);
void stop_timer(VMINT is_clear);
void start_timer(void);
void stop_audio(void);

/**
 *	Program Entry
 */
void vm_main(void) 
{	
	VMINT	drv;
	VMINT8	logfile[50];	
	VMCHAR	path[255];
	
	drv= vm_get_removable_driver();		//Get removable driver letter first
	if(drv<0)							//Removable driver is not exist
		drv=vm_get_system_driver();		//Get system driver letter
	
	/* Create folder to save record */
	sprintf(path, "%c:\\vre", drv);
	vm_file_mkdir(vm_ucs2_string(path));
	sprintf(path, "%c:\\vre\\Save", drv);
	vm_file_mkdir(vm_ucs2_string(path));
	sprintf(path, "%c:\\vre\\Save\\%s", drv, APP_SERIAL_NUM);
	vm_file_mkdir(vm_ucs2_string(path));
	
	/* Initialize log system */
	sprintf(logfile, "%c:\\Audio_demo.log", drv);
	vm_log_init(logfile, VM_DEBUG_LEVEL);
	vm_log_debug("Audio Demo Start!");
	
	/* Register event handler */
	vm_reg_sysevt_callback(sys_evt_process);
	vm_reg_keyboard_callback(sys_key_process);
	vm_reg_pen_callback(sys_pen_process);
}


static void sys_evt_process(VMINT message, VMINT param)
{	
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
	/*  after the processing of external events completed, the system will send this message */
	case VM_MSG_ACTIVE:
		active_app();
		break;
	/* if you press hang-up button, the system will send this message */
	case VM_MSG_QUIT:
		quit_app();
		break;
	}
}

static void sys_key_process(VMINT message, VMINT param)
{	
	switch(g_app.app_state)
	{
	/* Main menu */
	case STATE_MENU:
		menu_key_proc(message, param);
		break;
	/* Midi play menu */
	case STATE_RES_PLAYING:
		res_play_key_proc(message, param);
		break;
	/* Audio file play menu */
	case STATE_FILE_PLAYING:
	case STATE_FILE_PAUSE:
		file_play_key_proc(message, param);
		break;
	/* Recording menu */
	case STATE_RECORDING:
	case STATE_RECORD_PAUSE:
		record_key_proc(message, param);
		break;
	default:
		break;
	}
	draw_app();
}

static void sys_pen_process(VMINT event, VMINT x, VMINT y)
{
	//to do
}

void play_res_cb(VMINT handle, VMINT event)
{
	vm_log_debug("play_res_cb,hdl=%d,event=%d",handle,event);

	switch(event)
	{
	case VM_STATE_MIDI_START:
		vm_log_debug("VM_STATE_MIDI_START");
		break;
	case VM_STATE_MIDI_STOP:
		vm_log_debug("VM_STATE_MIDI_STOP");
		break;
	/* This message means the end of midi playing */
	case VM_AUDIO_RESULT_END_OF_FILE:
		vm_log_debug("VM_AUDIO_RESULT_END_OF_FILE");
		g_app.app_state = STATE_MENU;
		draw_app();
		break;
// 	case VM_MDI_AUDIO_OTHER:
// 		vm_log_debug("VM_MDI_AUDIO_OTHER");
// 		break;
	default:
		vm_log_debug("unknown event");
		break;
	}
}

void play_file_cb(VMINT result)
{
	vm_log_debug("play_file_cb,result=%d",result);
	switch(result)
	{
	/* This message means the end of audio playing or the play has been stopped */
	case VM_AUDIO_RESULT_STOP:
		vm_log_debug("VM_AUDIO_RESULT_STOP");
		g_app.app_state = STATE_MENU;
		draw_app();
		break;
	case VM_AUDIO_RESULT_PAUSE:
		vm_log_debug("VM_AUDIO_RESULT_PAUSE");
		break;
	case VM_AUDIO_RESULT_RESUME:
		vm_log_debug("VM_AUDIO_RESULT_RESUME");
		break;
	default:
		vm_log_debug("unknown event");
		break;
	}
}

void record_cb(VMINT result)
{
	vm_log_debug("result = %d", result);
}

void menu_key_proc(VMINT message, VMINT param){
	if (message == VM_KEY_EVENT_DOWN)
	{
		switch(param)
		{
		/* If press the right soft key, it will exit */ 
		case VM_KEY_RIGHT_SOFTKEY:	
			quit_app();
			break;
		/* If press the left key, it will start play midi */ 
		case VM_KEY_LEFT:
			if (play_res() >= 0) {
				g_app.app_state = STATE_RES_PLAYING;
				start_timer();
			}
			break;
		/* If press the right key, it will start play audio */
		case VM_KEY_RIGHT:
			if (play_file() >= 0) {
				g_app.app_state = STATE_FILE_PLAYING;
				start_timer();			
			}
			break;
		/* If press the down key, it will vibrate */
		case VM_KEY_DOWN:
			vm_vibrator_once();
			break;
		/* If press the OK key, it will start record */
		case VM_KEY_OK:
			if (record() >= 0) {
				g_app.app_state = STATE_RECORDING;
				start_timer();
				
			}
			break;
		}
	}
}

void res_play_key_proc(VMINT message, VMINT param) {
	if (message == VM_KEY_EVENT_DOWN) {
		switch(param) {
		/* If press the right soft key, it will stop play midi */
		case VM_KEY_RIGHT_SOFTKEY:	
			vm_midi_stop(g_audio_res[MIDI_DD].handle);
			g_app.app_state = STATE_MENU;
			stop_timer(TRUE);
			break;
		/* If press the up key, it will turn up the volume */
		case VM_KEY_UP:	
			if (g_app.volume < 6)
				vm_set_volume(++g_app.volume);
			break;
		/* If press the down key, it will turn down the volume */
		case VM_KEY_DOWN:
			if (g_app.volume > 0)
				vm_set_volume(--g_app.volume);
			break;
		}
	}	
}
void file_play_key_proc(VMINT message, VMINT param) {
	if (message == VM_KEY_EVENT_DOWN) {
		switch(param) {
		/* If press the right soft key, it will stop play audio */
		case VM_KEY_RIGHT_SOFTKEY:
			if (STATE_FILE_PAUSE == g_app.app_state || STATE_FILE_PLAYING == g_app.app_state) {
				if (0 == vm_audio_stop(play_file_cb))
					g_app.app_state = STATE_MENU;
					stop_timer(TRUE);
			}
			break;
		/* If press the left soft key, it will pause of resume the play */
		case VM_KEY_LEFT_SOFTKEY:
			/* resume play audio file */
			if (STATE_FILE_PAUSE == g_app.app_state) {
				if (0 == vm_audio_resume(play_file_cb))
					g_app.app_state = STATE_FILE_PLAYING;
					start_timer();
			}
			/* pause play audio file */
			else if (STATE_FILE_PLAYING == g_app.app_state) {
				if (0 == vm_audio_pause(play_file_cb))
					g_app.app_state = STATE_FILE_PAUSE;
					stop_timer(FALSE);				
			}			
			break;
		/* If press the up key, it will turn up the volume */
		case VM_KEY_UP:			
			if (g_app.volume < 6)
				vm_set_volume(++g_app.volume);
			break;
		/* If press the down key, it will turn down the volume */
		case VM_KEY_DOWN:
			if (g_app.volume > 0)
				vm_set_volume(--g_app.volume);
			break;
		}
	}	
}

void record_key_proc(VMINT message, VMINT param) {
	if (message == VM_KEY_EVENT_DOWN) {
		switch(param) {
		/* If press the right soft key, it will stop record */
		case VM_KEY_RIGHT_SOFTKEY:
			if (STATE_RECORDING == g_app.app_state || STATE_RECORD_PAUSE == g_app.app_state) {
				if (TRUE == vm_record_stop())
				{
					g_app.app_state = STATE_MENU;
					stop_timer(TRUE);
				}
			}
			break;
		/* If press the left soft key, it will pause of resume the record */
		case VM_KEY_LEFT_SOFTKEY:
			/* pause record */
			if (STATE_RECORDING == g_app.app_state) {
				if (TRUE == vm_record_pause())
				{
					g_app.app_state = STATE_RECORD_PAUSE;
					stop_timer(FALSE);
				}
			}
			/* resume record */
			else if (STATE_RECORD_PAUSE == g_app.app_state) {
				if (TRUE == vm_record_resume())
				{
					g_app.app_state = STATE_RECORDING;
					start_timer();
				}
			}
			break;
		}
	}	
}

void load_res(VMINT start, VMINT end) {	
	VMINT size;
	VMUINT8* res;
	VMINT i;
	
	/* load audio resource */
	for(i = start; i <= end ; i++)
	{
		if((res = vm_load_resource(res_name[i], &size)) == NULL)
		{
			vm_log_debug("load resource failed: res_name[%d] - %s", i, res_name[i]);
			quit_app();
		}

		g_audio_res[i].buffer = res;
		g_audio_res[i].length = size;
	}
}

void un_load_res(VMINT start ,VMINT end){
	VMINT i ;

	/* unload audio resource */
	for( i = start; i <= end ; i ++ )
	{
		if (g_audio_res[i].buffer) {
			vm_free(g_audio_res[i].buffer);
			g_audio_res[i].buffer = NULL;
			g_audio_res[i].length = 0;
		}		
	}
}

void draw_memu(void){
	VMCHAR	str[50] = {0};
	VMWCHAR	wstr[50] = {0};

	sprintf(str, "%s", "Audio Demo");				
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 60, 50, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "KEY LEFT: PLAY MIDI");				
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 30, 100, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "KEY RIGHT: PLAY MP3");				
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 30, 130, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "KEY OK: RECORD");				
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 30, 160, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "KEY DOWN: VIBRATOR");				
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 30, 190, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "RIGHT SOFTKEY: EXIT");				
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 30, 220, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

}

void draw_play_res(void){
	VMCHAR	str[50];
	VMWCHAR	wstr[50];
	VMINT w;
	VMINT play_time;
	VMINT hour;
	VMINT minute;
	VMINT second;
	
	sprintf(str, "%s", "MIDI Playing...");				
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 50, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "Volume:%d", g_app.volume);			
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 80, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	play_time = g_app.total_time + g_app.current_time - g_app.start_time;
	trans_time(play_time, &hour, &minute, &second);
	sprintf(str, "%02d:%02d:%02d", hour, minute, second);			
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 110, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	sprintf(str, "%s", "Return");				
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, g_app.screen_width - w - 2, g_app.screen_height - g_app.character_height - 2, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
}

void draw_play_file(void){
	VMCHAR	str[50];
	VMWCHAR	wstr[50];
	VMINT w;
	VMINT play_time;
	VMINT play_hour;
	VMINT play_minute;
	VMINT play_second;
	VMINT total_hour;
	VMINT total_minute;
	VMINT total_second;
	
	if (g_app.app_state == STATE_FILE_PLAYING)
		sprintf(str, "%s", "MP3 Playing...");				
	else if (g_app.app_state == STATE_FILE_PAUSE)
		sprintf(str, "%s", "MP3 Pause...");				
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 50, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "Volume:%d", g_app.volume);			
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 80, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	play_time = g_app.total_time + g_app.current_time - g_app.start_time;
	trans_time(play_time, &play_hour, &play_minute, &play_second);
	total_hour = g_app.file_duration / 3600;
	g_app.file_duration = g_app.file_duration % 3600;
	total_minute = g_app.file_duration / 60;
	total_second = g_app.file_duration % 60;
	sprintf(str, "%02d:%02d:%02d/%02d:%02d:%02d", play_hour, play_minute, play_second, total_hour, total_minute, total_second);			
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 110, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	if (g_app.app_state == STATE_FILE_PLAYING)
		sprintf(str, "%s", "Pause");				
	else if (g_app.app_state == STATE_FILE_PAUSE)
		sprintf(str, "%s", "Resume");			
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 2, g_app.screen_height - g_app.character_height - 2, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	sprintf(str, "%s", "Return");				
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, g_app.screen_width - w - 2, g_app.screen_height - g_app.character_height - 2, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
}

void draw_record(void){
	VMCHAR	str[50];
	VMWCHAR	wstr[50];
	VMINT w;
	VMINT play_time;
	VMINT hour;
	VMINT minute;
	VMINT second;
	
	if (g_app.app_state == STATE_RECORDING)
		sprintf(str, "%s", "Recording...");				
	else if (g_app.app_state == STATE_RECORD_PAUSE)
		sprintf(str, "%s", "Record Pause...");				
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 50, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	play_time = g_app.total_time + g_app.current_time - g_app.start_time;
	trans_time(play_time, &hour, &minute, &second);
	sprintf(str, "%02d:%02d:%02d", hour, minute, second);	
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, (g_app.screen_width - w) >> 1, 110, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	if (g_app.app_state == STATE_RECORDING)
		sprintf(str, "%s", "Pause");				
	else if (g_app.app_state == STATE_RECORD_PAUSE)
		sprintf(str, "%s", "Resume");			
	vm_ascii_to_ucs2(wstr, 100, str);
	vm_graphic_textout(g_app.buffer, 2, g_app.screen_height - g_app.character_height - 2, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	sprintf(str, "%s", "Return");				
	vm_ascii_to_ucs2(wstr, 100, str);
	w = vm_graphic_get_string_width(wstr);
	vm_graphic_textout(g_app.buffer, g_app.screen_width - w - 2, g_app.screen_height - g_app.character_height - 2, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
}

void draw_app(void){
	vm_graphic_fill_rect(g_app.buffer, 0, 0, 
		vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_BLACK, VM_COLOR_BLACK);	
	switch(g_app.app_state) {
	case STATE_MENU:
		draw_memu();
		break;
	case STATE_RES_PLAYING:
		draw_play_res();
		break;
	case STATE_FILE_PLAYING:
	case STATE_FILE_PAUSE:
		draw_play_file();
	    break;
	case STATE_RECORDING:
	case STATE_RECORD_PAUSE:
		draw_record();
	    break;
	default:
	    break;
	}
	vm_graphic_flush_layer(&g_app.handle, 1);
}

void init_app(void) {
	/*Create base layer */
	if ((g_app.handle = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1)) != 0)
	{
		vm_log_fatal("create base layer fail,break");
		vm_exit_app();
		return;
	}
	/*Get base layer buffer */
	if ((g_app.buffer = vm_graphic_get_layer_buffer(g_app.handle)) == NULL) {
		vm_log_fatal("get base layer buffer fail,break");
		vm_graphic_delete_layer(g_app.handle);		
		vm_exit_app();
		return;
	}

	g_app.app_state = STATE_MENU;
	g_app.volume = 3;
	g_app.screen_width = vm_graphic_get_screen_width();
	g_app.screen_height = vm_graphic_get_screen_height();
	g_app.character_height = vm_graphic_get_character_height();

	/* load resource */
	load_res(MIDI_DD, MIDI_DD);
}

void active_app(void) {
	/* Create base layer */
	if ((g_app.handle = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1)) != 0)
	{
		vm_log_fatal("create base layer fail,break");
		vm_exit_app();
		return;
	}
	/* Get base layer buffer */
	if ((g_app.buffer = vm_graphic_get_layer_buffer(g_app.handle)) == NULL){
		vm_log_fatal("get base layer buffer fail,break");
		vm_graphic_delete_layer(g_app.handle);		
		vm_exit_app();
		return;
	}	
}

void inactive_app(void) {	
	stop_audio();	
	vm_graphic_delete_layer(g_app.handle);
	g_app.buffer = NULL;
}

void quit_app(void) {
	stop_audio();
	un_load_res(MIDI_DD, MIDI_DD);
	vm_graphic_delete_layer(g_app.handle);
	g_app.buffer = NULL;
	vm_exit_app();
}

VMINT play_res(void) {
	if (NULL == g_audio_res[MIDI_DD].buffer || g_audio_res[MIDI_DD].length <= 0)
		return -1;
	vm_set_volume(g_app.volume);
	g_audio_res[MIDI_DD].handle = vm_midi_play_by_bytes(g_audio_res[MIDI_DD].buffer, g_audio_res[MIDI_DD].length, 1, play_res_cb); 
	vm_log_debug("play_res start");
	return g_audio_res[MIDI_DD].handle;
}

VMINT play_file(void) {
	VMINT drv;
	VMCHAR path[255]= {0};
	VMWCHAR wpath[255] = {0};
	drv= vm_get_removable_driver();	
	if(drv<0)							
		drv=vm_get_system_driver();		
	sprintf(path, "%c:\\vre\\dingdang.mid", drv);
	vm_ascii_to_ucs2(wpath, 510, path);
	g_app.file_duration = vm_audio_duration(wpath);
	vm_set_volume(g_app.volume);
	return vm_audio_play_file(wpath, play_file_cb);
}

VMINT record(void) {
	VMCHAR  drv[2] = {0};
	VMCHAR  dir[100] = {0};
	VMCHAR  filename[10] = {0};
	VMWCHAR	wpath[120] = {0};
		
	sprintf(dir, "record");
	sprintf(filename, "record");
	sprintf(drv, "%c", (vm_get_removable_driver() < 0 ? vm_get_system_driver() : vm_get_removable_driver()));
	return vm_record_start(drv, dir, filename, VM_FORMAT_WAV, wpath, record_cb);
}

void get_time(VMINT timer_id) {
	g_app.current_time = vm_get_tick_count();
	draw_app();
}

void trans_time(VMINT millisecond, VMINT* hour, VMINT* minute, VMINT* second) {
	*hour = millisecond / MILLISECOND_PER_HOUR;
	millisecond = millisecond % MILLISECOND_PER_HOUR;
	*minute = millisecond / MILLISECOND_PER_MINUTE;
	millisecond = millisecond % MILLISECOND_PER_MINUTE;
	*second = millisecond / MILLISECOND_PER_SECOND;
}

void stop_timer(VMINT is_clear) {
	vm_delete_timer(g_app.timer_id);
	if (TRUE == is_clear)
		g_app.total_time = 0;
	else
		g_app.total_time += (g_app.current_time - g_app.start_time);
	g_app.start_time = 0;
	g_app.current_time = 0;
}

void start_timer(void) {
	g_app.timer_id = vm_create_timer(50, get_time);
	g_app.start_time = vm_get_tick_count();
	g_app.current_time = g_app.start_time;
}

void stop_audio(void) {
	if (STATE_FILE_PAUSE == g_app.app_state || STATE_FILE_PLAYING == g_app.app_state) {
		vm_audio_stop(play_file_cb);
		vm_log_debug("vm_audio_stop");
		g_app.app_state = STATE_MENU;
		stop_timer(TRUE);
	}
	
	if (STATE_RES_PLAYING == g_app.app_state) {
// 		vm_midi_stop(g_audio_res[MIDI_DD].handle);
		vm_log_debug("vm_midi_stop");
		g_app.app_state = STATE_MENU;
		stop_timer(TRUE);
	}
	
	if (STATE_RECORD_PAUSE == g_app.app_state || STATE_RECORDING == g_app.app_state) {
		vm_record_stop();
		vm_log_debug("vm_record_stop");
		g_app.app_state = STATE_MENU;
		stop_timer(TRUE);
	}
}
