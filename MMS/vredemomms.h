#ifndef _VRE_APP_vredemomms_
#define	_VRE_APP_vredemomms_

void draw();

void app_start(void);
void app_quit(void);
static VMINT vm_mms_send_mix(void);
static VMINT vm_mms_send_smil(void);

typedef enum 
{
	MAMU = 1,
	
	SEND_MMS_ERROR,
	SEND_MMS_SUCCESS,
	SEND_MMS_CB_ERROR,
	SEND_MMS_CB_SUCCESS
} sms_state;
#endif

