#include <stdio.h>
#include <string.h>

#include "vmio.h"
#include "vmlog.h"
#include "vmbitstream.h"
#include "vmmm.h"

#include "Audio.h"
#include "Engine.h"

typedef struct {
	VMINT audio_hdl;
	VMINT file_hdl;
	VMUINT file_size;
	VMINT state;
	VMUINT offset;
	VMUINT total_play_time;
	VMUINT current_play_time;
	VMINT put_timer;
	AUDIO_CALLBACK_T audio_cb;
} audio_info_t;

static audio_info_t audio_info;

/**
 * Put audio data into audio buffer periodically.
 *
 * @param timer_id			timer id.
 *
 * @return
 */
static void audio_put_data(VMINT timer_id);

/**
 * Read audio data from file.
 *
 * @param data			pointer to the buffer that receives the data from file.
 * @param len			the number of bytes to be read.
 *
 * @return				the number of bytes actually read.
 */			
static VMINT audio_read_file(void* data, VMINT len);

static void audio_callback(VMINT handle, VMINT result);	

VMINT ba_audio_play(VMINT file_hdl, VMUINT file_size, AUDIO_CALLBACK_T audio_cb)
{	
	vm_bitstream_audio_cfg_struct audio_cfg;

	if (AUDIO_CLOSED != audio_info.state)
		return AUDIO_NOT_CLOSED;
	
	audio_cfg.vm_codec_type = VM_BITSTREAM_CODEC_TYPE_DAF;		//audio stream format: mp3
	
	/* open audio stream handle */
	if (VM_BITSTREAM_SUCCEED != vm_bitstream_audio_open(&audio_info.audio_hdl, &audio_cfg, audio_callback))
	{
		if (audio_cb)
			audio_cb(audio_info.current_play_time + audio_info.total_play_time);
		return AUDIO_OPEN_FAILED;
	}
	
	/* initialize audio playing info */
	audio_info.file_hdl = file_hdl;
	audio_info.file_size = file_size;
	audio_info.audio_cb = audio_cb;
	audio_info.put_timer = vm_create_timer(30, (VM_TIMERPROC_T)audio_put_data);	// create timer to put audio data
	audio_info.state = AUDIO_OPENED;

	/* notify elapsed time */
	if (audio_info.audio_cb)
		audio_info.audio_cb(audio_info.current_play_time + audio_info.total_play_time);
		
	return 0;
}

VMINT ba_audio_stop(void)
{
	vm_bitstream_audio_buffer_status status;
	
	if(AUDIO_CLOSED == audio_info.state)
		return AUDIO_ALREADY_CLOSED;

	if (AUDIO_PLAYING == audio_info.state && VM_BITSTREAM_SUCCEED != vm_bitstream_audio_stop(audio_info.audio_hdl))
		return AUDIO_STOP_FAILED;

	/* stop timer */
	if (audio_info.put_timer)
		vm_delete_timer(audio_info.put_timer);
	
	audio_info.state = AUDIO_STOPPED;

	vm_bitstream_audio_get_buffer_status(audio_info.audio_hdl, &status);
	
	/* save playback position */
	audio_info.offset -= (status.total_buf_size - status.free_buf_size);
	audio_info.total_play_time += audio_info.current_play_time;
	audio_info.current_play_time = 0;
	
	/* notify elapsed time */
	if (audio_info.audio_cb)
		audio_info.audio_cb(audio_info.current_play_time + audio_info.total_play_time);

	/* close audio stream handle */
	if (VM_BITSTREAM_SUCCEED != vm_bitstream_audio_close(audio_info.audio_hdl))
		return AUDIO_CLOSE_FAILED;

	audio_info.state = AUDIO_CLOSED;
	return 0;
}

VMINT ba_get_audio_state(void)
{
	return audio_info.state;
}

static void audio_put_data(VMINT timer_id)
{
	VMINT ret;
	VMUINT8* data = NULL;
	VMINT readed;
	VMUINT used_size;
	vm_bitstream_audio_buffer_status status;
	vm_bitstream_audio_start_param param;

	/* stop timer if the pointer reached the end of file */
	if (audio_info.offset >= audio_info.file_size)
	{
		vm_delete_timer(audio_info.put_timer);
		audio_info.put_timer = 0;
		return;
	}

	vm_bitstream_audio_get_buffer_status(audio_info.audio_hdl, &status);

	/* put data into buffer if there is free space */
	if (status.free_buf_size > 0) 
	{
		data = (VMUINT8*)vm_calloc(status.free_buf_size);
		if (NULL == data) 
		{
			ba_audio_stop();
			return;
		}

		readed = audio_read_file(data, status.free_buf_size);
		/* read failed, stop audio playing */
		if (readed < 0)
		{
			vm_free(data);
			ba_audio_stop();
			return;
		}

		ret = vm_bitstream_audio_put_data(audio_info.audio_hdl, data, readed, &used_size);

		if(VM_BITSTREAM_SUCCEED == ret)
			audio_info.offset += used_size;

		vm_free(data);

		vm_bitstream_audio_get_buffer_status(audio_info.audio_hdl, &status);

		if (AUDIO_OPENED == audio_info.state)
		{
			/* if buffer is full or the pointer reached the end of file, start play audio */
			if (0 == status.free_buf_size || audio_info.offset >= audio_info.file_size)
			{	
				param.start_time = 0;
				param.audio_path = VM_DEVICE_SPEAKER2;
				param.volume= 3;
				
				ret = vm_bitstream_audio_start(audio_info.audio_hdl, &param);
				
				if (VM_BITSTREAM_SUCCEED == ret)
					audio_info.state = AUDIO_PLAYING;
				else			
				ba_audio_stop();
			}
			else
			{
				audio_info.state = AUDIO_BUFFERING;
			}			
		}		
	}
	
	vm_bitstream_audio_get_play_time(audio_info.audio_hdl, &audio_info.current_play_time);

	if (audio_info.audio_cb)
		audio_info.audio_cb(audio_info.current_play_time + audio_info.total_play_time);
}

static VMINT audio_read_file(void* data, VMINT len)
{
	VMUINT readed = 0;
	VMUINT file_size = 0;

	if(len <= 0 || audio_info.file_hdl < 0)
		return -1;	

	if (0 == audio_info.file_size)
		vm_file_getfilesize(audio_info.file_hdl, &file_size);
	else
		file_size = audio_info.file_size;

	if (audio_info.offset + len > file_size)
		len = file_size - audio_info.offset;
	
	vm_file_seek(audio_info.file_hdl, audio_info.offset, BASE_BEGIN);
	vm_file_read(audio_info.file_hdl, data, len, &readed);

	return readed;
}

static void audio_callback(VMINT handle, VMINT result)
{
	/* do nothing*/
}


