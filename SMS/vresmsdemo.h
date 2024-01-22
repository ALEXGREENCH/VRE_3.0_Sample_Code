#ifndef _VRE_APP_vresmsdemo_
#define	_VRE_APP_vresmsdemo_
#include "vmsms.h"

static void draw(void);

void app_start(void);
void app_quit(void);
void read_msg_test(void);
//void user_sms_callback(vm_sms_callback_t* callback_data);
void user_sms_read_callback(vm_sms_callback_t* callback_data);

void add_msg_test(void);
void delete_msg_test(void);
void delete_msg_list_test(void);
void get_msg_id_test(void);
void get_msg_box_size_test(void);
void get_msg_id_list_test(void);
void get_sc_address_test(void);
typedef enum 
{
	MAMU = 1,

	READ_SMS_ERROR,
	READ_SMS_SUCCESS,
	READ_SMS_CB_SUCCESS,
	READ_SMS_CB_ERROR,

	ADD_SMS_ERROR,
	ADD_SMS_SUCCESS,
	ADD_SMS_CB_ERROR,
	ADD_SMS_CB_SUCCESS,	
	
	DELETE_SMS_ERROR,
	DELETE_SMS_SUCCESS,
	DELETE_SMS_CB_ERROR,
	DELETE_SMS_CB_SUCCESS,

	DELETE_SMS_LIST_ERROR,
	DELETE_SMS_LIST_SUCCESS,
	DELETE_SMS_LIST_CB_ERROR,
	DELETE_SMS_LIST_CB_SUCCESS,

	GET_MSG_ID_ERROR,
	GET_MSG_ID_SUCCESS,

	GET_BOX_SIZE_ERROR,
	GET_BOX_SIZE_SUCCESS,

	GET_MSG_ID_LIST_ERROR,
	GET_MSG_ID_LIST_SUCCESS,
	GET_MSG_ID_LIST_CB_ERROR,
	GET_MSG_ID_LIST_CB_SUCCESS,

	GET_SC_ADDRESS_ERROR,
	GET_SC_ADDRESS_SUCCESS,
	GET_SC_ADDRESS_CB_ERROR,
	GET_SC_ADDRESS_CB_SUCCESS
} sms_state;


#endif

