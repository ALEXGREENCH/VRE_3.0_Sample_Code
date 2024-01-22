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
#include "vresmsdemo.h"
#include "functions.h"
#include "vmsms.h"
#include "vmlog.h"
/* ---------------------------------------------------------------------------
 * global variables
 * ------------------------------------------------------------------------ */
sms_state app_sms_state = MAMU;

VMINT		layer_hdl[1];				////layer handle array.
VMUINT8* screenbuf = NULL;	
VMINT ncharacter_height = -1;
VMINT nscreen_width = -1;
VMINT nscreen_height = -1;
//VMCHAR *read_msg_content;
vm_sms_msg_data_t * msg_data = NULL;
VMUINT16 *msg_id_list = NULL;  
vm_sms_query_msg_cb_t  *query_msg = NULL;
VMCHAR get_msg_id[100];
VMCHAR get_box_size[100];
VMCHAR get_sc_addres[100];
/* --------------------srcreenbuf-------------------------------------------------------
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

/**
 * entry
 */
void vm_main(void) {
	VMCHAR path[50];
	VMINT dir;
	layer_hdl[0] = -1;
	dir = vm_get_removable_driver();
	if (dir<0)
	{
		dir = vm_get_system_driver();
	}
	
	sprintf(path,"%c:\\vre\\vredebug.log",dir);// log path
	vm_log_init(path,VM_ERROR_LEVEL);
	vm_log_error("event start!");

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
	vm_reg_pen_callback(handle_penevt);
}

void handle_sysevt(VMINT message, VMINT param) {
#ifdef		SUPPORT_BG	
	/* The application updates the screen when receiving the message VM_MSG_PAINT 
	*  what is sent after the application is activated. The application can skip 
	*  the process on screen when the VM_MSG_ACTIVE or VM_MSG_INACTIVE is received.
	*/
	switch (message) {

		case VM_MSG_CREATE:
			/* the GDI operation is not recommended as the response of the message*/
			break;
		case VM_MSG_PAINT:
			/* cerate base layer that has same size as the screen*/
			layer_hdl[0] = vm_graphic_create_layer(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height(), -1);	
			
			/* set clip area */
			vm_graphic_set_clip(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height());

			break;
		case VM_MSG_HIDE:
		case VM_MSG_QUIT:
			if( layer_hdl[0] != -1 )
			{
				vm_graphic_delete_layer(layer_hdl[0]);
				layer_hdl[0] = -1;
			}

			break;
	}
#else
	switch (message) {
		case VM_MSG_CREATE:
		case VM_MSG_ACTIVE:
		app_start();
			/*cerate base layer that has same size as the screen*/
			layer_hdl[0] = vm_graphic_create_layer(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height(), -1);	
			
			/* set clip area*/
			vm_graphic_set_clip(0, 0, 
				vm_graphic_get_screen_width(), 
				vm_graphic_get_screen_height());
			break;

		case VM_MSG_PAINT:
			screenbuf = vm_graphic_get_layer_buffer(layer_hdl[0]);
			draw();
			break;

		case VM_MSG_INACTIVE:
			if( layer_hdl[0] != -1 )
				vm_graphic_delete_layer(layer_hdl[0]);
			break;
		case VM_MSG_QUIT:
			if( layer_hdl[0] != -1 )
				vm_graphic_delete_layer(layer_hdl[0]);
	
			break;	
	}
#endif
}

void handle_keyevt(VMINT event, VMINT keycode) {

	if (VM_KEY_EVENT_UP == event)
	{
		if (app_sms_state == MAMU)
		{
			if (VM_KEY_RIGHT_SOFTKEY == keycode )
			{
				if( layer_hdl[0] != -1 )
				vm_graphic_delete_layer(layer_hdl[0]);
				app_quit();
				return;
			}
		switch(keycode)
		{
		case VM_KEY_NUM1:
			read_msg_test();
			break;

		case VM_KEY_NUM2:
			add_msg_test();
			break;
		case VM_KEY_NUM3:
			delete_msg_test();
			break;	
		case VM_KEY_NUM4:
			delete_msg_list_test();
		    break;
		case VM_KEY_NUM5:
			get_msg_id_test();
			break;
		case VM_KEY_NUM6:
			get_msg_box_size_test();
			break;
		case VM_KEY_NUM7:
			get_msg_id_list_test();
			break;
		case VM_KEY_NUM8:
			get_sc_address_test();
			vm_log_error("get_sc_address_test");
			break;
		}
		}
		else if(VM_KEY_RIGHT_SOFTKEY == keycode) 
		{
			app_sms_state = MAMU;
		}
		draw();
	}
}

void handle_penevt(VMINT event, VMINT x, VMINT y)
{
	if (VM_PEN_EVENT_RELEASE == event)
	{
		if (x<=nscreen_width&&y<ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM1);
		}	else if (x<=nscreen_width&&y>ncharacter_height&&y<2*ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM2);
		}
		else if (x<=nscreen_width&&y>2*ncharacter_height&&y<3*ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM3);
		}	
		else if (x<=nscreen_width&&y>3*ncharacter_height&&y<4*ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM4);
		}
		else if (x<=nscreen_width&&y>4*ncharacter_height&&y<5*ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM5);
		}
		else if (x<=nscreen_width&&y>5*ncharacter_height&&y<6*ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM6);
		}
		else if (x<=nscreen_width&&y>6*ncharacter_height&&y<7*ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM7);
		}
		else if (x<=nscreen_width&&y>7*ncharacter_height&&y<8*ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_NUM8);
		}
		else if (x<=nscreen_width&&x>nscreen_width/2&&y>nscreen_height-ncharacter_height)
		{
			handle_keyevt(VM_KEY_EVENT_UP,VM_KEY_RIGHT_SOFTKEY);
		}
	}	
}

void draw()
{
	vm_graphic_fill_rect(screenbuf, 0, 0, vm_graphic_get_screen_width(), 
		vm_graphic_get_screen_height(), VM_COLOR_BLACK, VM_COLOR_BLACK);

	if (app_sms_state!=MAMU)
	{
		draw_text_muiltline(screenbuf,"back",nscreen_width-70,nscreen_height-ncharacter_height,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);

	}
	switch(app_sms_state)
	{
	case MAMU:
		draw_text_muiltline(screenbuf,"1.read sms",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"2.add sms",0,(ncharacter_height),nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"3.delete sms",0,(ncharacter_height*2),nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"4.delete sms list",0,(ncharacter_height*3),nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"5.get msg id",0,(ncharacter_height*4),nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"6.get box size",0,(ncharacter_height*5),nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"7.get msg id list",0,(ncharacter_height*6),nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"8.get sc address list",0,(ncharacter_height*7),nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		draw_text_muiltline(screenbuf,"exit",nscreen_width-70,nscreen_height-ncharacter_height,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;	
	case READ_SMS_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_read_msg  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case READ_SMS_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_read_msg success,please wait",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case READ_SMS_CB_SUCCESS:
		draw_text_muiltline(screenbuf,msg_data->content_buff,0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case READ_SMS_CB_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_read_msg CB error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
//////////////////////////////////////////////////////////////////////////////////
	case ADD_SMS_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_add_msg  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case ADD_SMS_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_add_msg success,please wait",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case ADD_SMS_CB_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_add_msg cb success",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case ADD_SMS_CB_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_add_msg cb error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
///////////////////////////////////////////////////
	case DELETE_SMS_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case DELETE_SMS_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg success,please wait",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case DELETE_SMS_CB_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg cb success",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case DELETE_SMS_CB_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg cb error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;

////////////////DELETE MSGLIST/////////////////////////////////////////////////
	case DELETE_SMS_LIST_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg_list  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case DELETE_SMS_LIST_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg_list success,please wait",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case DELETE_SMS_LIST_CB_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg_list cb success",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case DELETE_SMS_LIST_CB_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_delete_msg_list cb error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
///////////////////////GET MSG ID///////////////////////////////
	case GET_MSG_ID_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_get_msg_id  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case GET_MSG_ID_SUCCESS:
		draw_text_muiltline(screenbuf,get_msg_id,0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
//////////////get box size//////////////////////////////////
	case GET_BOX_SIZE_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_get_box_size  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case GET_BOX_SIZE_SUCCESS:
		draw_text_muiltline(screenbuf,get_box_size,0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
////////////////get msg id list/////////////////////////////////////////////////
	case GET_MSG_ID_LIST_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_get_msg_id_list  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case GET_MSG_ID_LIST_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_get_msg_id_list success,please wait",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case GET_MSG_ID_LIST_CB_SUCCESS:
		{
			VMCHAR id_list_content[100];
			int i,j;
			j=0;
		for(i=0; i <query_msg->msg_num; i++)
		{
		//	query_msg->msg_id_list[i] =  query_msg->msg_id_list[i];
			sprintf(id_list_content, "msg_id_list[%d]=%d num=%d",i, query_msg->msg_id_list[i],query_msg->msg_num);
			// output1(c, 0, j*vm_graphic_get_character_height());
			j++;
			vm_log_error("id_list_content=%s  num = %d", id_list_content, query_msg->msg_num);
			draw_text_muiltline(screenbuf,id_list_content,0,j*ncharacter_height,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
			}
		}
		break;
	case GET_MSG_ID_LIST_CB_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_get_msg_id_list cb error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
		////////////////get /////////////////////////////////////////////////
	case GET_SC_ADDRESS_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_get_sc_address  error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case GET_SC_ADDRESS_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_get_sc_address success,please wait",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case GET_SC_ADDRESS_CB_ERROR:
		draw_text_muiltline(screenbuf,"vm_sms_get_sc_address cb error",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	case GET_SC_ADDRESS_CB_SUCCESS:
		draw_text_muiltline(screenbuf,"vm_sms_get_sc_address cb success ",0,0,nscreen_width,nscreen_height,2,1,VM_COLOR_WHITE);
		break;
	}
	vm_graphic_flush_layer(layer_hdl, 1);
}
void app_start()
{
	ncharacter_height = vm_graphic_get_character_height();
	nscreen_width = vm_graphic_get_screen_width();
	nscreen_height = vm_graphic_get_screen_height();
}
void app_quit()
{
	if (msg_data)
	{
	if (msg_data->content_buff)
	{
		vm_free(msg_data->content_buff);
		msg_data->content_buff = NULL;
	}
	}
	if (msg_data)
	{
		vm_free(msg_data);
		msg_data = NULL;
	}
	if (query_msg)
	{
		vm_free(query_msg);
		query_msg = NULL;
	}
	vm_exit_app();
}	
void read_msg_test()
{
	VMINT change_status = 1;

	void* user_data = NULL;
	VMINT res;
	VMUINT16 msg_id;
	vm_sms_box_enum box_type;
	VMUINT16 list_index;
	VMINT8 * content_buff;
	
	msg_data = vm_calloc(sizeof(vm_sms_msg_data_t));
	if(msg_data == NULL)
	{
		vm_log_error("msg_data == NULL");
		return;
	}

	content_buff = vm_calloc((500+1)*2);
	if(content_buff == NULL)
	{
		vm_free(msg_data);
		vm_log_error("content_buff == NULL");
	    return;
	}
	vm_log_error("msg_data =%d",msg_data);
	msg_data->content_buff = content_buff;
	msg_data->content_buff_size = 500*2;
//	vm_log_error("msg.content = %d",msg_data->content_buff);
	
	// box_type = VM_SMS_BOX_DRAFTS;
	box_type = VM_SMS_BOX_INBOX;
	list_index = 0;
	msg_id = vm_sms_get_msg_id(box_type, list_index);
	vm_log_error("msg_id = %d",msg_id);
	res = vm_sms_read_msg(msg_id, change_status,  msg_data,  user_sms_read_callback, user_data);
	vm_log_error("read");
	if(res < 0)
	{
		vm_log_debug("read_msg_test1 fail");
		app_sms_state = READ_SMS_ERROR;
	}
	else
	{
		vm_log_debug("read_msg_test1 success");
		app_sms_state = READ_SMS_SUCCESS;
	}
}
void user_sms_add_callback(vm_sms_callback_t* callback_data)
{
	vm_sms_callback_t   callback_data_ptr;
	
	vm_sms_save_msg_cb_t  *add_msg;
			
	VMUINT16 msg_id;  
	VMCHAR c[50];
	
	if(!callback_data)
	{
		app_sms_state = ADD_SMS_CB_ERROR;
		vm_log_error("vm_sms_callback:callback_data == NULL");
		return;
	}
	if (callback_data->result ==0)
	{
		app_sms_state = ADD_SMS_CB_ERROR;
		draw();
		return;
	}
	vm_log_debug("callback_data->result = %d",callback_data->result);
	vm_log_debug("user_sms_callback:callback_data->cause=%d, action=%d",callback_data->cause, callback_data->action);
	//vm_log_debug();
    callback_data_ptr.cause = callback_data->cause;
	
	switch(callback_data->action )
	{
		
	case VM_SMS_ACTION_READ:
		break;
		
	   case VM_SMS_ACTION_SAVE:
		   
		   callback_data_ptr.result = callback_data->result;
		   callback_data_ptr.action = VM_SMS_ACTION_SAVE;
		   
		   if(callback_data_ptr.cause == VM_SMS_CAUSE_NO_ERROR)
		   {
			   app_sms_state = ADD_SMS_CB_SUCCESS;
			   if(!callback_data->action_data)
			   {
				   break;
			   }
			   
			   add_msg = (vm_sms_save_msg_cb_t*)callback_data->action_data;
			   
			   msg_id = add_msg->msg_id;
			   
		   }
		   else 
		   {
			   app_sms_state = ADD_SMS_CB_ERROR;
		   }
		   sprintf(c, "add c=%d,r=%d,id=%d",callback_data_ptr.cause, callback_data_ptr.result, msg_id);
		   //   output(c);
		   vm_log_debug("%s", c);
		   break;
		   
	   case VM_SMS_ACTION_DELETE:
		   break;
	}
	vm_log_error("end add");
	draw();
}
void add_msg_test()
{
	vm_sms_add_msg_data_t  msg_data;
	void* user_data = NULL;
	VMCHAR content[VM_SMS_MSG_CONTENT_LEN] = {0};
	VMWCHAR wcontent[VM_SMS_MSG_CONTENT_LEN] = {0};
	VMINT res;
	VMCHAR number[(VM_SMS_MAX_ADDR_LEN + 1) ]={0};
	VMWCHAR wnumber[(VM_SMS_MAX_ADDR_LEN + 1)]={0};
	
	msg_data.sim_id = VM_SMS_SIM_1;
	
	// msg_data.status = VM_SMS_STATUS_UNREAD; /*Unread */
    msg_data.status = VM_SMS_STATUS_READ ;         /*Read*/
    //msg_data.status = VM_SMS_STATUS_SENT ;        /*Sent*/
	//  msg_data.status = VM_SMS_STATUS_UNSENT;      /*Unsent(Unsent box)*/
	 //msg_data.status = VM_SMS_STATUS_DRAFT;          /*DRAFT*/
	//	 msg_data.storage_type = VM_SMS_STORAGE_ME;


	msg_data.storage_type = VM_SMS_STORAGE_SIM;
	strcpy(content,"This is test program");
//	vm_ascii_to_ucs2((VMWSTR)wcontent, VM_SMS_MSG_CONTENT_LEN * 2, content);
	
	strcpy(number,"13816559000");
    vm_ascii_to_ucs2((VMWSTR)wnumber, (VM_SMS_MAX_ADDR_LEN + 1) * 2, number);
	vm_log_debug("number=%s", number);
	
	msg_data.content_size = VM_SMS_MSG_CONTENT_LEN;
	msg_data.content = content;
    vm_safe_wstrcpy((VMWSTR)msg_data.number,VM_SMS_MAX_ADDR_LEN, (VMWSTR)wnumber);
	
    msg_data.timestamp.year = 2009;
	msg_data.timestamp.mon  =11;
	msg_data.timestamp.day =14;
	msg_data.timestamp.hour =13 ;
	msg_data.timestamp.min =3;
	msg_data.timestamp.sec = 3;
	
	
	res = vm_sms_add_msg(&msg_data, user_sms_add_callback, user_data);
	if(res < 0)
	{
		vm_log_debug("add_msg_test1 fail,res=%d",res);
		app_sms_state = ADD_SMS_ERROR;
	}
	else 
	{
		app_sms_state = ADD_SMS_SUCCESS;
		vm_log_debug("add_msg_test1 success");
	}
}
// void user_sms_callback(vm_sms_callback_t* callback_data)
//  {
//       vm_sms_callback_t   callback_data_ptr;
// 	  
//       vm_sms_save_msg_cb_t  *add_msg;
//       
//       vm_sms_query_msg_cb_t  *query_msg ;
// 
//      VMINT8 *get_data;
// 	 vm_sms_get_sc_addr_cb_t  *sc_addr_cb;
// 
//       VMUINT16 msg_id;  
//       VMUINT16 *msg_id_list;    
//       VMINT i;
// 	VMCHAR c[50];
// 	VMINT j;
// 
// 	if(!callback_data)
// 	{
// 	     vm_log_error("vm_sms_callback:callback_data == NULL");
// 	     return;
// 	}
// 	vm_log_debug("callback_data->result = %d",callback_data->result);
// 	vm_log_debug("user_sms_callback:callback_data->cause=%d, action=%d",callback_data->cause, callback_data->action);
// 	//vm_log_debug();
//     callback_data_ptr.cause = callback_data->cause;
// 
//       switch(callback_data->action )
//       {
//       
//            case VM_SMS_ACTION_READ:
// 			break;
// 	
// 	   case VM_SMS_ACTION_SAVE:
// 	   	
//                      callback_data_ptr.result = callback_data->result;
// 			callback_data_ptr.action = VM_SMS_ACTION_SAVE;
// 			
// 			if(callback_data_ptr.cause == VM_SMS_CAUSE_NO_ERROR)
// 			{
// 				app_sms_state = ADD_SMS_CB_SUCCESS;
// 			   if(!callback_data->action_data)
// 		          {
// 		             break;
// 		          }
// 			
// 			   add_msg = (vm_sms_save_msg_cb_t*)callback_data->action_data;
// 			                     
// 			   msg_id = add_msg->msg_id;
//                         
// 			}
// 			else 
// 			{
// 				app_sms_state = ADD_SMS_CB_ERROR;
// 			}
// 			 sprintf(c, "add c=%d,r=%d,id=%d",callback_data_ptr.cause, callback_data_ptr.result, msg_id);
//                    //   output(c);
// 			 vm_log_debug("%s", c);
// 			break;
// 	 
// 	  case VM_SMS_ACTION_DELETE:
// 			break;
// 	  }
// 	 vm_log_error("end add");
// 	 draw();
// }

void user_sms_read_callback(vm_sms_callback_t* callback_data)
{
      vm_sms_callback_t   callback_data_ptr;
	  
      vm_sms_read_msg_cb_t  * read_msg;
   //   vm_sms_msg_data_t *msg_data = NULL;

	VMCHAR c[50];

	if(!callback_data)
	{
		app_sms_state = READ_SMS_CB_ERROR;
	     vm_log_error("vm_sms_callback:callback_data == NULL");
		 draw();
	     return;
	}
	if (callback_data->result ==0)
	{
		app_sms_state = READ_SMS_CB_ERROR;
		draw();
		return;
	}
	vm_log_debug("callback_data->result = %d",callback_data->result);
	vm_log_debug("user_sms_callback:callback_data->cause=%d, action=%d",callback_data->cause, callback_data->action);
	//vm_log_debug();
    callback_data_ptr.cause = callback_data->cause;

      switch(callback_data->action )
      {
           case VM_SMS_ACTION_READ:
		   	vm_log_debug("VM_SMS_ACTION_READ start"); 
		   	callback_data_ptr.result = callback_data->result;
			
                     callback_data_ptr.action = VM_SMS_ACTION_READ;

		       if(callback_data_ptr.cause == VM_SMS_CAUSE_NO_ERROR)
		       {
				   app_sms_state = READ_SMS_CB_SUCCESS;
		          if(!callback_data->action_data)
		          {
		             break;
		          }
		   				
			   read_msg =callback_data->action_data;
				vm_log_error("callback_data->action_data= %d",callback_data->action_data);
			   msg_data = read_msg->msg_data;
			   vm_log_error("read_msg->msg_data=%d",read_msg->msg_data);
			   vm_log_error("msg_data.contentbuf%d",msg_data->content_buff);;
			   vm_log_debug("msg_data->status=%d",msg_data->status);
			   vm_log_debug("msg_data->mti =%d",msg_data->mti);
			   vm_log_debug("msg_data->tp_st =%d",msg_data->tp_st);
			   vm_log_debug("msg_data->folder_id =%d",msg_data->folder_id);
               vm_log_debug("msg_data->para_flag=%d",msg_data->para_flag);
						
			   vm_log_debug("msg_data->timestamp.year=%d",msg_data->timestamp.year);
			   vm_log_debug("msg_data->timestamp.mon=%d ",msg_data->timestamp.mon );
			   vm_log_debug("msg_data->timestamp.day=%d",msg_data->timestamp.day);
			   vm_log_debug("msg_data->timestamp.hour=%d",msg_data->timestamp.hour);
			   vm_log_debug("msg_data->timestamp.min=%d",msg_data->timestamp.min);
			   vm_log_debug("msg_data->timestamp.sec=%d",msg_data->timestamp.sec);
			   vm_log_debug("msg_data->storage_type =%d",msg_data->timestamp.sec);
			   vm_log_debug("msg_data->sim_id =%d",msg_data->sim_id);
			   vm_log_debug("msg_data->pid =%d",msg_data->pid);
			   vm_log_debug(" msg_data->vp =%d", msg_data->vp);
		          vm_log_debug("msg_data->status_report=%d",msg_data->status_report);
			   vm_log_debug("msg_data->reply_path =%d", msg_data->reply_path);
	                 vm_log_debug("msg_data->src_port =%d", msg_data->src_port);
			   vm_log_debug("msg_data->dest_port =%d", msg_data->dest_port);
		          vm_log_debug("msg_data->dcs  =%d", msg_data->dcs);

			   vm_log_debug("msg_data->msg_class  =%d", msg_data->msg_class);
			   vm_log_debug("msg_data->content_buff_size  =%d", msg_data->content_buff_size);
			   vm_log_debug("msg_data->content_buff =%s", msg_data->content_buff);
				
		        vm_log_debug("VM_SMS_ACTION_READ test1");

		       }
			   else 
			   {
				   app_sms_state = READ_SMS_CB_ERROR;
			   }
			   
			   vm_log_debug("VM_SMS_ACTION_READ test2");
			 sprintf(c, "read ca=%d, re=%d",callback_data_ptr.cause, callback_data_ptr.result);
			 vm_log_debug("%s", c); 
                  //    output(c);
             
			vm_log_debug("VM_SMS_ACTION_READ end"); 
			break;
	
	   case VM_SMS_ACTION_SAVE:
		 
	  case VM_SMS_ACTION_DELETE:	
          case VM_SMS_ACTION_QUERY:
              
			break;
		case VM_SMS_ACTION_GET_SC_ADDR:	 
			break;
         default:
		 	callback_data_ptr.result = callback_data->result;
	              callback_data_ptr.action = VM_SMS_ACTION_NONE;
			vm_log_debug("vm_sms_callback:callback_data_ptr->action is invalid!");
		 	break;
      }
     callback_data_ptr.user_data = callback_data->user_data;

	draw();
}

void user_sms_delete_msglist_callback(vm_sms_callback_t* callback_data)
{
		
	vm_log_debug("user_sms_callback:callback_data->cause=%d, action=%d",callback_data->cause, callback_data->action);
	if(!callback_data)
	{
		app_sms_state =  DELETE_SMS_LIST_CB_ERROR;
		vm_log_error("vm_sms_callback:callback_data == NULL");
		draw();
		return;
	}
	if (callback_data->result == 0)
	{
		app_sms_state =  DELETE_SMS_LIST_CB_ERROR;
		draw();
		return;
	}
	switch(callback_data->action)
	{
	case VM_SMS_ACTION_DELETE:
		if (callback_data->cause == VM_SMS_CAUSE_NO_ERROR)
		{
			app_sms_state = DELETE_SMS_LIST_CB_SUCCESS;
		}
		else 
		{
			app_sms_state =  DELETE_SMS_LIST_CB_ERROR;
		}
		break;
	}
	draw();
}
void delete_msg_list_test()
{
	void* user_data = NULL;
	VMINT res;
	vm_sms_sim_enum sim_id;
	vm_sms_box_enum box_type;
	
	sim_id = VM_SMS_SIM_1;
	// box_type = VM_SMS_BOX_INBOX + VM_SMS_BOX_DRAFTS +VM_SMS_BOX_UNSENT;
	//	box_type = VM_SMS_BOX_INBOX;
	
	box_type = VM_SMS_BOX_INBOX;
	res = vm_sms_delete_msg_list(box_type, sim_id,user_sms_delete_msglist_callback, user_data);
	if(res < 0)
	{
		vm_log_debug("delete_msg_list_test1 fail");
		app_sms_state = DELETE_SMS_LIST_ERROR;
	}
	else 
	{
		app_sms_state = DELETE_SMS_LIST_SUCCESS;
		vm_log_debug("delete_msg_list_test1 success");
	}
}
void user_sms_delete_msg_callback(vm_sms_callback_t* callback_data)
{
	
	
	vm_log_debug("user_sms_callback:callback_data->cause=%d, action=%d",callback_data->cause, callback_data->action);
	if(!callback_data)
	{		
		app_sms_state =  DELETE_SMS_CB_ERROR;
		vm_log_error("vm_sms_callback:callback_data == NULL");
		draw();
		return;
	}
	if (callback_data->result ==0)
	{
		app_sms_state =  DELETE_SMS_CB_ERROR;
		draw();
		return;
	}
	switch(callback_data->action)
	{
	case VM_SMS_ACTION_DELETE:
		if (callback_data->cause == VM_SMS_CAUSE_NO_ERROR)
		{
			app_sms_state = DELETE_SMS_CB_SUCCESS;
		}
		else 
		{
			app_sms_state =  DELETE_SMS_CB_ERROR;
		}
		break;
	}
	draw();
}
void delete_msg_test()
{
	void* user_data = NULL;
	VMINT res;
	VMUINT16 msg_id;
	vm_sms_box_enum box_type;
	VMUINT16 msg_index;
	
	 box_type = VM_SMS_BOX_INBOX;
	//box_type = VM_SMS_BOX_DRAFTS;
	msg_index = 0;
	
	msg_id = vm_sms_get_msg_id(box_type, msg_index);
	res = vm_sms_delete_msg(msg_id, user_sms_delete_msg_callback, user_data);

		if(res < 0)
		{
			vm_log_debug("delete_msg_test fail");
			app_sms_state = DELETE_SMS_ERROR;
		}
		else 
		{
			app_sms_state = DELETE_SMS_SUCCESS;
			vm_log_debug("delete_msg_test success");
		}
}
void get_msg_id_test()
{
	
	VMINT res;
	VMCHAR c[50] = {0};
	VMUINT16 msg_index;
	vm_sms_box_enum box_type;
	
	
		box_type = VM_SMS_BOX_INBOX      ;                 /*Inbox*/
	//	box_type =  VM_SMS_BOX_OUTBOX   ;             /*Outbox*/
	//	box_type =  VM_SMS_BOX_DRAFTS   ;                  /*Draft box*/
	//  box_type =  VM_SMS_BOX_UNSENT   ;                  /*Unsent box*/
	//  box_type =  VM_SMS_BOX_SIM     ;               /*simcard*/
    //  box_type =  VM_SMS_BOX_ARCHIVE    ;

	msg_index = 0;
	res =vm_sms_get_msg_id( box_type,  msg_index);
	if (res<0)
	{
		app_sms_state = GET_MSG_ID_ERROR;
	}
	else
	{
		app_sms_state = GET_MSG_ID_SUCCESS;
	}
	sprintf(get_msg_id,"msg_id[%d][%d]=%d", box_type,msg_index,res);
	vm_log_debug("get_msg_id_test1 %s", get_msg_id);
	draw();
}
void get_msg_box_size_test()
{
	VMINT res;
	VMCHAR c[50] = {0};
	vm_sms_box_enum box_type;
	VMINT j=0;
	   
	box_type = VM_SMS_BOX_INBOX ;                 /*Inbox*/
	res = vm_sms_get_box_size( box_type);
	if (res<0)
	{
		app_sms_state = GET_BOX_SIZE_ERROR;
	}
	else 
	{
		app_sms_state = GET_BOX_SIZE_SUCCESS;
	}
	sprintf(get_box_size,"[%d]_INBOX_size=%d", box_type,res);
	vm_log_error("boxsize = %s",c);
}
void user_sms_msg_idlist_callback(vm_sms_callback_t* callback_data)
{
	vm_sms_callback_t   callback_data_ptr;
   //   vm_sms_query_msg_cb_t  *query_msg ;
	  
  
      VMINT i;
	  VMCHAR c[50];
	  VMINT j;

	  if (!callback_data)
	  {
		  app_sms_state = GET_MSG_ID_LIST_CB_ERROR;
		  draw();
		  return;
	  }
	  if (callback_data->result == 0)
	  {
		  app_sms_state = GET_MSG_ID_LIST_CB_ERROR;
		  vm_log_error("callback_data->result == 0");
		  draw();
		  return;
	  }  
	  switch(callback_data->action)
	  {
		case VM_SMS_ACTION_QUERY:
		 callback_data_ptr.result = callback_data->result;
		 callback_data_ptr.action = VM_SMS_ACTION_QUERY;
		 callback_data_ptr.cause = callback_data->cause;
		 if(callback_data_ptr.cause == VM_SMS_CAUSE_NO_ERROR)	
		 {
			 if(!callback_data->action_data)
			 {
				 app_sms_state = GET_MSG_ID_LIST_CB_ERROR;
				  draw();
				 return;
			 }
			 if (query_msg == NULL)
			 {
				query_msg =(vm_sms_query_msg_cb_t*) vm_malloc(sizeof(vm_sms_query_msg_cb_t));
			 }
			  query_msg->msg_num = ((vm_sms_query_msg_cb_t*)callback_data->action_data)->msg_num;
			 query_msg->msg_id_list = vm_malloc(query_msg->msg_num*sizeof(VMUINT16));
			 if(!query_msg->msg_id_list)
			 {
				 //vm_log_error("vm_sms_callback:query_msg.msg_id_list == NULL");
				 app_sms_state = GET_MSG_ID_LIST_CB_ERROR;
				 draw();
				 	 return;
			 }
			 
			 j=0;

			 if(query_msg->msg_num <=0)
			 {
				 app_sms_state = GET_MSG_ID_LIST_CB_ERROR;
				  draw();
				  return;	
			 }
			 for(i=0; i <query_msg->msg_num; i++)
			 {
				 query_msg->msg_id_list[i] =  query_msg->msg_id_list[i];
				 sprintf(c, "msg_id_list[%d]=%d num=%d",i, query_msg->msg_id_list[i],query_msg->msg_num);
				// output1(c, 0, j*vm_graphic_get_character_height());
				 j++;
				 vm_log_error("%s  num = %d", c, query_msg->msg_num);
			 }
		 }
		 else
		 {
			 app_sms_state = GET_MSG_ID_LIST_CB_ERROR;
			 draw();
			 return;
		 }
	  }
	  app_sms_state = GET_MSG_ID_LIST_CB_SUCCESS;
		 draw();
			// callback_data_ptr.user_data = callback_data->user_data;
}
void get_msg_id_list_test()
{
	vm_sms_query_t query_data;
	void* user_data = NULL;
	VMINT res;


	query_data.sort_flag = VM_SMS_SORT_TIMESTAMP;
	query_data.order_flag = VM_SMS_ORDER_DEFAULT;
	// query_data.order_flag = VM_SMS_ORDER_ASC;
	//query_data.order_flag = VM_SMS_ORDER_DESC;
	 	// query_data.status = VM_SMS_STATUS_DRAFT; 
		// 	query_data.status = VM_SMS_STATUS_UNREAD;
		 query_data.status = VM_SMS_STATUS_READ;
		 //	query_data.status =  VM_SMS_STATUS_SENT;
		 res = vm_sms_get_msg_id_list(&query_data,  user_sms_msg_idlist_callback, user_data);
		 if(res < 0)
			 vm_log_debug("get_msg_id_list_test1 fail");
		 else
			vm_log_debug("get_msg_id_list_test1 success");
}
void user_sms_get_addres_callback(vm_sms_callback_t* callback_data)
{
	
	vm_sms_get_sc_addr_cb_t  *sc_addr_cb;
	if (!callback_data)
	{
		app_sms_state = GET_SC_ADDRESS_CB_ERROR;
		draw();
		return;
	}
	if (callback_data->result == 0)
	{
		app_sms_state = GET_SC_ADDRESS_CB_ERROR;
		draw();
		return;
	}
		
	switch(callback_data->action )
     {
	case VM_SMS_ACTION_GET_SC_ADDR:	 
		if (callback_data->cause == VM_SMS_CAUSE_NO_ERROR)
		{
			sc_addr_cb = callback_data->action_data;
		
			memcpy(get_sc_addres,sc_addr_cb->sc_addr,22);
			//sprintf(c, "sc_addr=%s",sc_addr_cb->sc_addr);
			//output(c);
		}
		else
		{
			app_sms_state = GET_SC_ADDRESS_CB_ERROR;
			draw();
			return;
		//	output("get sc_addr fail");
		}
			break;
	}
	app_sms_state = GET_SC_ADDRESS_CB_SUCCESS;
	draw();
	
}
void get_sc_address_test()
{
	VMINT res;
	vm_sms_sim_enum sim_id;
	
	sim_id = VM_SMS_SIM_1;
	res = vm_sms_get_sc_address(sim_id, user_sms_get_addres_callback, NULL);
	if(res < 0)
	{	//	out
		app_sms_state = GET_SC_ADDRESS_ERROR;
//		put("get_sc_address_test1 fail");
	}
	else 
	{
		app_sms_state = GET_SC_ADDRESS_SUCCESS;
	}
	
}