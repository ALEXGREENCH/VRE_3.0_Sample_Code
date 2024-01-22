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

/*
 * http.c演示如何使用VRE应用接口来开发一个使用http进行网络链接的应用程序。
 *  
 * http.c demonstrates how to use the VRE API to develop a HTTP application
 *
 * Copyright (c) 2009 Vogins Network Technology (China). All rights reserved.
 */

#include <string.h>
#include <stdio.h>

#include "vmhttp.h"
#include "vmstdlib.h"
#include "vmio.h"
#include "vmchset.h"
#include "Network.h"

#define DOWNLOAD_URL "http://www.metmuseum.org/Works_of_Art/images/ma/landing_page_image.jpg"//"http://www.vogins.com/images/logo_vre.gif"
#define PORT 80
#define CHUNK_SIZE 200

static VMUINT file_size = 0;
static VMINT http_hdl = -1;

/* Send download request */ 
static VMINT HttpDownload(const VMCHAR* url, const VMCHAR* cnt,
						  VM_HTTP_PROXY_TYPE apn,	HTTP_METHOD mth, VMINT* handle,
						  void (*http_hook_cb)(VMINT bResponse, void* pSession), 
						  void (*http_state_notify_cb)(VMINT state, VMINT param, void* session));
/* HTTP status callback */
static void http_state_notify_cb(VMINT state, VMINT param, void* session);
/* HTTP response callback */
static void http_hook_cb(VMINT bResponse, void* pSession);

static void http_state_notify_cb(VMINT state, VMINT param, void* session)
{
	VMCHAR text[255] = "";
	
	switch(state)
	{
	case HTTP_STATE_GET_HOSTNAME:
		strcpy(text, "Getting Host Name...");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_CONNECTING:
		strcpy(text, "Connecting");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_SENDING:
		strcpy(text, "Sending Request...");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_STATUS:
		strcpy(text, "Receiving Response Status...");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_HEADS:
		strcpy(text, "Receiving Response Header...");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_BODY:
		strcpy(text, "Receiving Response Body");
		Http_Output(text, FALSE);
		break;	
	default:
		strcpy(text, "Unknown Event");
		Http_Output(text, FALSE);
		break;
	}
}

static void http_hook_cb(VMINT bResponse, void* pSession)
{
	static VMCHAR head[255] = {0};
	static VMCHAR text[255] = {0};
	static VMINT counts = 0;

	VMCHAR file_name[50];
	VMINT file_hdl;
	VMUINT written;
	VMUINT start_size = 0;
	VMBYTE buf = NULL;
	http_session_t* session = NULL;
	VMINT	drv;
	
	/*  Specifies the temporary file */
// 	drv= vm_get_removable_driver();	
// 	if(drv<0)							
// 		drv=vm_get_system_driver();	
// 	sprintf(file_name, "%c:\\vre\\logo_vre.tmp", drv);

	sprintf(file_name, "%d %d", counts,bResponse);

	memset(head,0,sizeof(head));
	memset(text,0,sizeof(text));
	memset(file_name,0,sizeof(file_name));

// 	buf = vm_graphic_get_buffer();
// 	vm_graphic_fill_rect(buf,0,0,50,50,0xffff,0xffff);
// 	vm_graphic_textout(buf,0,0,vm_ucs2_string(file_name),strlen(file_name) * 2,0x0);
// 	vm_graphic_flush_screen();
// 
	/* response correct */
	if(0 == bResponse)
	{
		session = (http_session_t*)pSession;
		
		/* Server Error */
		if( 0 > session->res_code || 500 == session->res_code )
		{
			strcpy(text, "Server Error");
			Http_Output(text, TRUE);
			return;
		}
		
		/* Get the value of "Content-Type" header */
		if(get_http_head(session, "Content-Type", head))
		{
			strcpy(text, "Failed to get Content-Type");
			Http_Output(text, TRUE);
			return;
		}
		
		/* if the value is "text/vnd.wap.wml", you need to request it again*/
		if(!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) 
		{
			StartHttpDownload();
			strcpy(text, "Need to Reconnect");
			Http_Output(text, TRUE);
			return;
		}
		memset(text, 0, 255);
		sprintf(text, "download size=%d", session->nresbody);
		Http_Output(text, TRUE);
		vm_log_debug("download size=%d", session->nresbody);


		/* If size of file is zero, means it's the first time to download this file */
// 		if(0 == file_size)
// 		{
// 			/* Create the temporary file */
// 			file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);
// 			if(file_hdl >= 0)
// 			{				
// 				/* Get the "Content-Range" header */
// 				if(get_http_head(session, "Content-Range", head))
// 				{ 
// 					vm_file_close(file_hdl);
// 					vm_file_delete(vm_ucs2_string(file_name));
// 
// 					strcpy(text, "Failed to get the Content-Range header");
// 					Http_Output(text, TRUE);
// 					return;
// 				}
// 
// 				/* Get the size of file */
// 				file_size = strtoi(strstr(head, "/") + 1);
// 				/* The first four bytes stores the size of file */
// 				vm_file_write(file_hdl, &file_size, 4, &written);
// 				/* The next four bytes stores the location of the next download starts */
// 				vm_file_write(file_hdl, &start_size, 4, &written);
// 				vm_file_close(file_hdl);
// 			}		
// 
// 			memset(text, 0, 255);
// 			sprintf(text, "File size：%dBytes", file_size);
// 			Http_Output(text, TRUE);
// 		}
// 		/* Not the first time to download the file */
// 		else
// 		{		
// 			/* Update the location of the next download starts */
// 			Http_ReadFile(vm_ucs2_string(file_name), 4, 4, &start_size);
// 			start_size += session->nresbody;
// 			file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_WRITE, TRUE);
// 			vm_file_seek(file_hdl, 4, BASE_BEGIN);
// 			vm_file_write(file_hdl, &start_size, 4, &written);
// 			/* Write the downloaded data to temporary files */
// 			vm_file_seek(file_hdl, 0, BASE_END);
// 			vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
// 			vm_file_close(file_hdl);
// 			
// 			memset(text, 0, 255);
// 			sprintf(text, "Downloaded %d Bytes", session->nresbody);
// 			Http_Output(text, TRUE);
// 
// 			/* Download finished */
// 			if(start_size == file_size)
// 			{
// 				VMCHAR* data;
// 				data = vm_malloc(file_size);
// 				Http_ReadFile(vm_ucs2_string(file_name), 8, file_size, data);
// 				vm_file_delete(vm_ucs2_string(file_name));
// 				/* Save as a new file */
// 				sprintf(file_name, "%c:\\vre\\logo_vre.jpg", drv);
// 				file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);
// 				vm_file_write(file_hdl, data, file_size, &written);
// 				vm_file_close(file_hdl);
// 				vm_free(data);
// 				data = NULL;
// 				http_hdl = -1;
// 			}			
// 		}
	}
	else
	{
		vm_log_debug("get image error!");
	}
	/* Continue download the remaining part of the picture */
//	StartHttpDownload();
}

static VMINT HttpDownload(const VMCHAR* url, const VMCHAR* cnt,
							VM_HTTP_PROXY_TYPE apn,	HTTP_METHOD mth, VMINT* handle,
							void (*http_hook_cb)(VMINT bResponse, void* pSession), 
							void (*http_state_notify_cb)(VMINT state, VMINT param, void* session))
{
	asyn_http_req_t req;
	VMINT ret;
	http_head_t head[1];
	VMCHAR file_name[50];
	VMCHAR full_name[50];
	VMINT find_hdl;
	VMUINT start_size;
	struct vm_fileinfo_t info;

	VMCHAR text[255] = {0};
	VMINT	drv;
	
// 	drv= vm_get_removable_driver();
// 	if(drv<0)						
// 		drv=vm_get_system_driver();	
// 		
// 	sprintf(full_name, "%c:\\vre\\logo_vre.*", drv);
// 	if((find_hdl = vm_find_first(vm_ucs2_string(full_name), &info)) >= 0)
// 	{
// 		vm_ucs2_to_ascii(file_name, 100, info.filename);
// 		if(!vm_ends_with(file_name, "jpg"))
// 		{
// 			sprintf(text, "Done");
// 			Http_Output(text, TRUE);
// 			return -1;
// 		}
// 		else if(!vm_ends_with(file_name, "tmp"))
// 		{
// 			VMWCHAR w_full_name[50];
// 			sprintf(full_name, "%c:\\vre\\%s", drv, file_name);
// 			vm_ascii_to_ucs2(w_full_name, 100, full_name);
// 			Http_ReadFile(w_full_name, 0, 4, &file_size);	
// 			vm_ascii_to_ucs2(w_full_name, 100, full_name);
// 			Http_ReadFile(w_full_name, 4, 4, &start_size);	
// 		}
// 		vm_find_close(find_hdl);
// 	}
	
	/* Request method */
	req.req_method = mth;
	/* APN */
	req.use_proxy = apn;
	
	/* The assembly of HTTP request*/
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t));
	if(NULL == req.http_request)
	{
		return -1;
	}
	
	/* The assembly of URL */
	if(strncmp(url, "http://", strlen("http://")))
	{
		strcat(req.http_request->url, "http://");
	}
	strcat(req.http_request->url, url);
	
	/* The assembly of HTTP request header */
// 	if(0 == file_size)
// 	{
// 		sprintf(head[0].name, "RANGE");		
// 		sprintf(head[0].value, "bytes=-1");
// 
// 		memset(text, 0, 255);
// 		sprintf(text, "Getting File Size...");
// 		Http_Output(text, FALSE);
// 	}
// 	else 
// 	{
// 		sprintf(head[0].name, "RANGE");
// 		memset(text, 0, 255);		
// 		sprintf(head[0].value, "bytes=%d-%d", start_size,
// 					((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
// 		sprintf(text, "Getting File... From %d To %d Bytes", start_size, 
// 					((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
// 		Http_Output(text, FALSE);
// 	}	
// 	req.http_request->nhead = 1;
// 	req.http_request->heads = head;
	
	/* Send HTTP Request */
	ret = vm_asyn_http_req(&req, http_hook_cb, http_state_notify_cb);
	
	/* Get HTTP handle */
	if(ASYN_HTTP_REQ_ACCEPT_SUCCESS == ret)
	{
		ret = vm_get_asyn_http_req_handle(&req, handle); 
	}

	vm_free(req.http_request);

	return ret;	
}


void StartHttpDownload(void)
{
	HttpDownload(DOWNLOAD_URL, "", HTTP_USE_CMNET_PRIORITY,
						GET, &http_hdl, http_hook_cb, http_state_notify_cb);
}

void StopHttpDownload(VMINT handle)
{
	VMCHAR text[255] = {0};
	if(!vm_cancel_asyn_http_req(handle))
	{
		handle = -1;
		sprintf(text, "Downloading Paused...\n");
		Http_Output(text, TRUE);
	}
}

void DropAllHttpConnection(void)
{
	vm_cancel_all_http_sessions();
	http_hdl = -1;
}

VMINT getHttpHandle(void)
{
	return http_hdl;
}
