#ifndef _VRE_APP_Test_Camera_
#define	_VRE_APP_Test_Camera_

#include "vmcamera.h"

typedef struct cam_info {
	VM_CAMERA_ID camera_id;
	VM_CAMERA_HANDLE camera_handle;
	VM_CAMERA_STATUS camera_status;
	vm_cam_size_t max_capture_size;
	const vm_cam_size_t* support_preview_size;
	VMUINT support_preview_size_num;
	vm_cam_capture_data_t capture_data;
	vm_cam_frame_data_t frame_data;
	VMUINT preview_fps;
} CAM_INFO_T;

typedef enum {
	STATE_MENU,
		STATE_PREVIEW
} APP_STATE_ENUM;

typedef struct app_info {
	VMUINT8			*buffer;				
	VMUSHORT		timer_id;				
	APP_STATE_ENUM	app_state;				
	VMINT			handle;						
	VMINT			volume;					
	VMINT			screen_width;			
	VMINT			screen_height;			
	VMINT			character_height;		
	VMCHAR			text[512];
	VMINT			preview_size_index;		
	VMINT			capture_num;
} APP_INFO_T;

#endif

