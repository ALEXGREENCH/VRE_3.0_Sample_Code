#ifndef _VRE_APP_File_
#define	_VRE_APP_File_

#define		APP_SERIAL_NUM	"WQ-TST-10000"			/* Product code */

/**
*   Appliction states
*/
typedef enum{
	STATE_MENU	= 0,
		STATE_NOTIFY
}app_state_t;

typedef struct app_t {
	VMUINT8		*dst_buf;
	VMUSHORT	timer_id;
	VMINT		app_state;
}app_t;


#endif

