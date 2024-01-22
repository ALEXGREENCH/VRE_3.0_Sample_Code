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
 * http.c��ʾ���ʹ��VREӦ�ýӿ�������һ��ʹ��http�����������ӵ�Ӧ�ó���
 *  
 * 
 *
 * Copyright (c) 2009 Vogins Network Technology (China). All rights reserved.
 */

#include <string.h>
#include <stdio.h>

#include "vmtimer.h"
#include "vmhttp.h"
#include "vmstdlib.h"
#include "vmio.h"
#include "vmchset.h"
#include "Network.h"
#include "vmlog.h"

/*�����ļ���ַ*/
#define DOWNLOAD_URL	"http://www.baidu.com/img/baidu_logo.gif"	//"http://61.236.127.192/ucenter/data/avatar/000/00/00/01_avatar_middle.jpg"

#define BIGFILE_URL		"http://www.myweddingday.cn/chuanqi.mp3"//"http://xm.dreammail.org/DreamMail4_Setup.exe"

#define POST_URL		"http://www.vspace.net.cn/vresps/test_http.jsp?qs=sb"

#define POST_URL_CHUNK	"http://www.vspace.net.cn/vresps/test_http.jsp"

#define POST_DATA		"name=linminqi&sex=male&company=vogins"

/*HTTP�˿�*/
#define PORT 80esst
/*���طֿ��С*/
#define CHUNK_SIZE 200

/*�����ļ���С*/
static VMUINT file_size = 0;
static VMCHAR file_name[50] = {0};
static VMFILE file_hdl = -1;
/*HTTP���*/
static VMINT http_hdl[12] = {-1};

/*������������*/
static VMINT HttpDownload(const VMCHAR* url, const VMCHAR* cnt,
						  VM_HTTP_PROXY_TYPE apn,	HTTP_METHOD mth, VMINT* handle,
						  void (*http_hook_cb)(VMINT bResponse, void* pSession), 
						  void (*http_state_notify_cb)(VMINT state, VMINT param, void* session));
/*HTTP״̬�ص�����*/
static void http_state_notify_cb(VMINT state, VMINT param, void* session);
static void http_state_notify_cb_ex(VMINT state, VMINT param);
/*HTTP��Ӧ�ص�����*/
static void http_hook_cb(VMINT bResponse, void* pSession);

void SetChar(char* c, int len);
void vm_http_log_debug(char* c, int len);

void http_test_origin(void);	//1
void http_test_new(void);
void http_test_url_header_body(void);	//2,3,4,5
void http_test_sender_receiver(void);	//6,8,10
void http_test_sender_chunk(void);	//7
void http_test_receiver_bigfile(void);	//11
void http_test_receiver_chunk(void);	//9
void http_test_vm_http_request(void);	//12
void http_test_new_vm_http_request(void);	//13 
void http_test_timer_http(void);

void vm_http_log_debug(char* c, int len)
{
	char temp[100];
	int i = 0;
	vm_log_debug("\n");
	while (i < len)
	{
		memset(temp, 0x00,  sizeof(temp));
		memcpy(temp, c+i, sizeof(temp)-1<len-i ? sizeof(temp)-1 : len-i);
		vm_log_debug("\n%s", temp);
		i += sizeof(temp)-1;
	}
	vm_log_debug("\n");
}

static void http_state_notify_cb_ex(VMINT state, VMINT param)
{
	http_state_notify_cb(state, param, NULL);
}

static void http_state_notify_cb(VMINT state, VMINT param, void* session)
{
	VMCHAR text[255] = "";
	
	switch(state)
	{
	case HTTP_STATE_GET_HOSTNAME:
		strcpy(text, "���ڻ�ȡ����");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_CONNECTING:
		strcpy(text, "��������");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_SENDING:
		strcpy(text, "���ڷ���������");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_STATUS:
		strcpy(text, "���ڽ���״̬��Ϣ");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_HEADS:
		strcpy(text, "���ڽ�����Ӧͷ");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_BODY:
		strcpy(text, "���ڽ�����Ӧ��");
		Http_Output(text, FALSE);
		break;	
	default:
		strcpy(text, "δ֪�¼�");
		Http_Output(text, FALSE);
		break;
	}
	vm_log_debug("http_state_notify_cb - %s, %d", text, param);
}

static void http_hook_cb(VMINT bResponse, void* pSession)
{
	VMCHAR head[255] = {0};
	VMCHAR text[255] = {0};

	VMUINT written;
	VMUINT start_size = 0;

	http_session_t* session = NULL;
	VMINT	drv;
	
	/* ��ȡ�̷� */
	drv= vm_get_removable_driver();	//���Ȼ�ȡ�ƶ��̷�
	if(drv<0)							//�������ƶ���
		drv=vm_get_system_driver();		//��ȡϵͳ��
	sprintf(file_name, "%c:\\vre\\logo_vre.tmp", drv);
	/*��ȷ����Ӧ*/
	if(0 == bResponse)
	{
		session = (http_session_t*)pSession;
		
		/*����������*/
		if( 0 > session->res_code || 500 == session->res_code )
		{
			strcpy(text, "����������");
			Http_Output(text, TRUE);
			return;
		}
		
		/*��ȡContent-Type*/
		if(get_http_head(session, "Content-Type", head))
		{
			strcpy(text, "��ȡContent-Typeʧ��");
			Http_Output(text, TRUE);
			return;
		}
		
		/*��"text/vnd.wap.wml"��ͷ�ģ���ʾ���õ���CMWAP���ӣ���Ҫ��������һ��*/
		if(!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) 
		{
			StartHttpDownload();
			strcpy(text, "CMWAP��������");
			Http_Output(text, TRUE);
			return;
		}

		/*�������ɵ���ʱ�ļ�ȫ·����*/
		sprintf(file_name, "%c:\\vre\\logo_vre.tmp", drv);
		/*��һ�����ظ��ļ�*/
		if(0 == file_size)
		{
			/*������ʱ�ļ�*/
			file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);
			if(file_hdl >= 0)
			{				
				/*��ȡContent-Range*/
				if(get_http_head(session, "Content-Range", head))
				{ 
					vm_file_close(file_hdl);
					vm_file_delete(vm_ucs2_string(file_name));

					strcpy(text, "��ȡ�ļ���Сʧ��");
					Http_Output(text, TRUE);
					return;
				}

				/*��ȡ�ļ���С*/
				file_size = strtoi(strstr(head, "/") + 1);
				/*��ʱ�ļ���ͷ4���ֽڴ���ļ���С*/
				vm_file_write(file_hdl, &file_size, 4, &written);
				/*������4���ֽڴ���´����ؿ�ʼ��λ��*/
				vm_file_write(file_hdl, &start_size, 4, &written);
				vm_file_close(file_hdl);
			}		

			memset(text, 0, 255);
			sprintf(text, "�ļ���С��%d�ֽ�", file_size);
			Http_Output(text, TRUE);
		}
		/*���ǵ�һ�����ظ��ļ�*/
		else
		{		
			/*�����´����ؿ�ʼ��λ��*/
			Http_ReadFile(vm_ucs2_string(file_name), 4, 4, &start_size);
			start_size += session->nresbody;
			file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_WRITE, TRUE);
			vm_file_seek(file_hdl, 4, BASE_BEGIN);
			vm_file_write(file_hdl, &start_size, 4, &written);
			/*�����ص�����д����ʱ�ļ�*/
			vm_file_seek(file_hdl, 0, BASE_END);
			vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
			vm_file_close(file_hdl);
			
			memset(text, 0, 255);
			sprintf(text, "���سɹ���������%d�ֽ�", session->nresbody);	
			Http_Output(text, TRUE);

			/*�������*/
			if(start_size == file_size)
			{
				VMCHAR* data;
				data = vm_malloc(file_size);
				Http_ReadFile(vm_ucs2_string(file_name), 8, file_size, data);
				vm_file_delete(vm_ucs2_string(file_name));
				/*���Ϊ���ļ�*/
				sprintf(file_name, "%c:\\vre\\logo_vre.jpg", drv);
				file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);
				vm_file_write(file_hdl, data, file_size, &written);
				vm_file_close(file_hdl);
				vm_free(data);
				data = NULL;
				http_hdl[0] = -1;
			}			
		}
	}
	
	/*��������ʣ��Ĳ���*/
	StartHttpDownload();
}

static VMINT HttpDownload(const VMCHAR* url, const VMCHAR* cnt,
							VM_HTTP_PROXY_TYPE apn,	HTTP_METHOD mth, VMINT* handle,
							void (*http_hook_cb)(VMINT bResponse, void* pSession), 
							void (*http_state_notify_cb)(VMINT state, VMINT param, void* session))
{
	asyn_http_req_t req;
	VMINT ret;
	http_head_t head[1];
	VMCHAR full_name[50];
	VMINT find_hdl;
	VMUINT start_size;
	struct vm_fileinfo_t info;

	VMCHAR text[255] = {0};
	VMINT	drv;
	
	/*��ȡ�ļ�����״̬*/
	drv= vm_get_removable_driver();	//���Ȼ�ȡ�ƶ��̷�
	if(drv<0)							//�������ƶ���
		drv=vm_get_system_driver();		//��ȡϵͳ��
		
	sprintf(full_name, "%c:\\vre\\logo_vre.*", drv);
	vm_log_debug("full_name:%s", full_name);
	if((find_hdl = vm_find_first(vm_ucs2_string(full_name), &info)) >= 0)
	{
		vm_ucs2_to_gb2312(file_name, 50, info.filename);
		if(!vm_ends_with(file_name, "jpg"))
		{
			sprintf(text, "�ļ���������");
			Http_Output(text, TRUE);
			return -1;
		}
		else if(!vm_ends_with(file_name, "tmp"))
		{
			sprintf(full_name, "%c:\\vre\\%s", drv, file_name);
			Http_ReadFile(vm_ucs2_string(full_name), 0, 4, &file_size);	
			Http_ReadFile(vm_ucs2_string(full_name), 4, 4, &start_size);	
		}
		vm_find_close(find_hdl);
	}
	
	/*����ʽ*/
	req.req_method = mth;
	/*������뷽ʽ*/
	req.use_proxy = apn;
	
	/*��װHTTP����*/
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t));
	if(NULL == req.http_request)
	{
		return -1;
	}
	
	/*��װURL*/
	if(strncmp(url, "http://", strlen("http://")))
	{
		strcat(req.http_request->url, "http://");
	}
	strcat(req.http_request->url, url);
	
	/*��װHTTP����ͷ*/
	if(0 == file_size)
	{
		sprintf(head[0].name, "RANGE");		
		sprintf(head[0].value, "bytes=-1");

		memset(text, 0, 255);
		sprintf(text, "�����ȡ�ļ���С");
		Http_Output(text, FALSE);
	}
	else 
	{
		sprintf(head[0].name, "RANGE");
		memset(text, 0, 255);		
		sprintf(head[0].value, "bytes=%d-%d", start_size,
					((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
		sprintf(text, "���������ļ�:�ӵ�%d�ֽڵ���%d�ֽ�", start_size, 
					((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
		Http_Output(text, FALSE);
	}	
	req.http_request->nhead = 1;
	req.http_request->heads = head;
	
	/*����HTTP����*/
	ret = vm_asyn_http_req(&req, http_hook_cb, http_state_notify_cb);
	
	/*��ȡHTTP���*/
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
						GET, &http_hdl[0], http_hook_cb, http_state_notify_cb);
}

void StopHttpDownload(VMINT handle)
{
	VMCHAR text[255] = {0};
	if(!vm_cancel_asyn_http_req(handle))
	{
		handle = -1;
		sprintf(text, "�ж�HTTP����\n");
		Http_Output(text, TRUE);
	}
}

void DropAllHttpConnection(void)
{
	vm_cancel_all_http_sessions();
	http_hdl[0] = -1;
}

VMINT getHttpHandle(void)
{
	return http_hdl[0];
}













/*
 * Just save response info into file
 */
void http_hook_cb_save2file(VMINT bResponse, void* pSession)
{
	VMCHAR text[255] = {0};
	VMCHAR head[255] = {0};
	VMUINT written;
	//VMUINT start_size = 0;
	http_session_t* session = NULL;
	//VMINT	drv;
	
	/* ��ȡ�̷� 
	drv= vm_get_removable_driver();	//���Ȼ�ȡ�ƶ��̷�
	if(drv<0)							//�������ƶ���
		drv=vm_get_system_driver();		//��ȡϵͳ��
	sprintf(file_name, "%c:\\vre\\http_result.txt", drv);*/
	
	vm_log_debug("enter http_hook_cb_save2file - bResponse:%d", bResponse);
	/*��ȷ����Ӧ*/
	if(0 == bResponse)
	{
		session = (http_session_t*)pSession;
		
		vm_log_debug("http_hook_cb_save2file - res_code:%d, nresbuf:%d", session->res_code, session->nresbuf);
		vm_http_log_debug((char*)session->resbuf, session->nresbuf);
		
		/*��ȡ�ļ���С*/
		file_size = strtoi(head);
		vm_log_debug("http_hook_cb_save2file - file_size:%d", file_size);
		
		/*���� �ļ�
		file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);*/
		
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, session->resbuf, session->nresbuf, &written);
		
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, "\r\n", strlen("\r\n"), &written);

		/*�����ص�����д�� �ļ�*/
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
		vm_file_close(file_hdl);
				
		strcpy(text, "���ص��ļ��ѱ���");
		Http_Output(text, FALSE);
		
		vm_log_debug("http_hook_cb_save2file - nresbody:%d", session->nresbody); 
		vm_http_log_debug((char*)session->resbody, session->nresbody);
	}
	else
	{
		strcpy(text, "�����HTTP���");
		Http_Output(text, FALSE);
		
		session = (http_session_t*)pSession;
		vm_log_debug("http_hook_cb_save2file - res_code:%d, nresbuf:%d, nresbody:%d", 
			session->res_code, session->nresbuf, session->nresbody);
		vm_http_log_debug((char*)session->resbuf, session->nresbuf);
		vm_http_log_debug((char*)session->resbody, session->nresbody);
	}
	vm_file_close(file_hdl);
}

void http_hook_cb1(VMINT bResponse, void* pSession)
{
	VMCHAR text[255] = {0};
	VMCHAR head[255] = {0};
	VMUINT written;
	//VMUINT start_size = 0;
	http_session_t* session = NULL;
	//VMINT	drv;
	
	/* ��ȡ�̷� 
	drv= vm_get_removable_driver();	//���Ȼ�ȡ�ƶ��̷�
	if(drv<0)							//�������ƶ���
		drv=vm_get_system_driver();		//��ȡϵͳ��
	sprintf(file_name, "%c:\\vre\\logo_vre2_http.jpg", drv);*/

	vm_log_debug("enter http_hook_cb1 - bResponse:%d", bResponse);
	/*��ȷ����Ӧ*/
	if(0 == bResponse)
	{
		session = (http_session_t*)pSession;

		vm_log_debug("http_hook_cb1 - res_code:%d, nresbuf:%d", session->res_code, session->nresbuf);
		vm_http_log_debug((char*)session->resbuf, session->nresbuf);

		if (session->resbody == NULL)
		{
			strcpy(text, "Session�е���Ӧ��Ϊ��");
			Http_Output(text, TRUE);
			//return;
		}

		/*����������*/
		if( 0 > session->res_code || 500 == session->res_code )
		{
			strcpy(text, "����������");
			Http_Output(text, TRUE);
			return;
		}

		/*��ȡContent-Type*/
		if(get_http_head(session, "Content-Type", head))
		{
			strcpy(text, "��ȡContent-Typeʧ��");
			Http_Output(text, TRUE);
			//return;
		}
		
		/*��"text/vnd.wap.wml"��ͷ�ģ���ʾ���õ���CMWAP���ӣ���Ҫ��������һ��*/
		if(!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) 
		{
			// StartHttpDownload();
			strcpy(text, "CMWAP��������");
			Http_Output(text, FALSE);
			vm_cancel_asyn_http_req(http_hdl[0]);
			http_test_receiver_bigfile();
			return;
		}

		/*��ȡContent-Length*/
		if(get_http_head(session, "Content-Length", head))
		{ 
			strcpy(text, "��ȡ�ļ���Сʧ��");
			Http_Output(text, TRUE);
			return;
		}

		/*��ȡ�ļ���С*/
		file_size = strtoi(head);
		vm_log_debug("http_hook_cb1 - file_hdl:%d, file_size:%d", file_hdl, file_size);

		/*�����ļ�
		file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);*/

		/*�����ص�����д���ļ�*/
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
		vm_file_close(file_hdl);

		strcpy(text, "���ص��ļ��ѱ���");
		Http_Output(text, FALSE);

		vm_log_debug("http_hook_cb1 - nresbody:%d", session->nresbody); 
		vm_http_log_debug((char*)session->resbody, session->nresbody);
	}
	else
	{
		strcpy(text, "�����HTTP���");
		Http_Output(text, FALSE);

		session = (http_session_t*)pSession;
		vm_log_debug("http_hook_cb1 - res_code:%d, nresbuf:%d, nresbody:%d", 
			session->res_code, session->nresbuf, session->nresbody);
		vm_http_log_debug((char*)session->resbuf, session->nresbuf);
		vm_http_log_debug((char*)session->resbody, session->nresbody);
	}
	vm_file_close(file_hdl);
}


/* 
 * ����ԭHTTP���� (OK)
 */
void http_test_origin()
{ 
	VMINT ret;
//	VMINT http_handle;
	asyn_http_req_t req;

	vm_log_debug("enter http_test_origin .");

	req.req_method = GET;
	req.use_proxy = HTTP_USE_CMNET_PRIORITY;
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t)); 
	if(NULL == req.http_request)
	{ 
		vm_log_error("http_test_origin - NULL == req.http_request .");
		return; 
	}

	sprintf(req.http_request->url, DOWNLOAD_URL);
	vm_log_debug("http_test_origin - req.http_request->url %s .", req.http_request->url);

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_origin.jpg", MODE_CREATE_ALWAYS_WRITE, TRUE);
	vm_log_debug("d:\\vre\\http_test_origin.jpg");
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);
	vm_log_debug("http_test_origin - after vm_asyn_http_req %d .", ret);

	/*��ȡHTTP���*/
	if(ASYN_HTTP_REQ_ACCEPT_SUCCESS == ret)
	{
		ret = vm_get_asyn_http_req_handle(&req, &http_hdl[0]); 
	}

	vm_free(req.http_request);

	return; 
} 

/* 
 * ����ԭHTTP���� (OK)
 */
void http_test_new()
{ 
	VMINT ret;
	VMINT http_handle;
	asyn_http_req_t req;
//	char *longurl, *longname, *longvalue, *longbody;
	
	vm_log_debug("enter http_test_url_header_body .");
	
	req.req_method = POST;
	req.use_proxy = HTTP_USE_CMNET_PRIORITY;
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t)); 
	if(NULL == req.http_request)
	{ 
		vm_log_error("http_test_url - NULL == req.http_request.");
		return; 
	}
	sprintf(req.http_request->url, POST_URL);

	req.http_request->body = vm_calloc(100); 
	strcpy(req.http_request->body, POST_DATA);
	req.http_request->nbody = strlen(req.http_request->body);

	/* ȡ���õ�HTTP��� */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
		return;
	vm_log_debug("http_test_url_header_body - after vm_http_fetch_handle http_handle:%d .", http_handle);
	
	/* ���HTTP ����URL�����Ȳ�Ҫ����400�ֽ� 
	longurl = (char*)vm_malloc(400);
	strcpy(longurl, POST_URL);
	strcat(longurl, "&");
	SetChar(longurl+strlen(POST_URL)+1, 400-strlen(POST_URL)-1);
	vm_http_set_url(http_handle, longurl, strlen(longurl));
	vm_log_debug("http_test_url_header_body - after vm_http_set_url http_handle:%d, longurl:%d.", http_handle, strlen(longurl));
	vm_http_log_debug(longurl, strlen(longurl));*/
	
	/* ���HTTP������ͷ��name+value�ĳ��Ȳ�Ҫ����400�ֽ� 
	longname = (char*)vm_malloc(100);
	SetChar(longname, 100);
	longvalue = (char*)vm_malloc(300);
	SetChar(longvalue, 300);
	vm_http_add_header(http_handle, longname, longvalue);
	//vm_http_add_header(http_handle, "name2", "value2");
	vm_log_debug("http_test_url_header_body - after vm_http_add_header http_handle:%d .", http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_add_header longname:%s .", longname);
	vm_http_log_debug(longvalue, strlen(longvalue));*/
	
	/* ���HTTP�������壬���������� 
	longbody = (char*)vm_malloc(10240);
	//strcpy(longbody, POST_DATA);
	SetChar(longbody, 10240);
	vm_http_set_body(http_handle, (VMUINT8*)longbody, strlen(longbody));
	vm_log_debug("http_test_url_header_body - after vm_http_set_body http_handle:%d .", http_handle);
	vm_http_log_debug(longbody, strlen(longbody));*/
	
	/* ��������ǰ��鵱ǰϵͳHTTP�������֤��ǰHTTP��������̴��� */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_set_current_handle http_handle:%d .", http_handle);
	
	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_new.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb_save2file, http_state_notify_cb);
	
	vm_log_debug("http_test_url_header_body - after http_test_url_header_body %d .", ret);
	
	vm_free(req.http_request->body);
	vm_free(req.http_request);
//	vm_free(longurl);
//	vm_free(longname);
//	vm_free(longvalue);
//	vm_free(longbody);
	
	return; 
} 

/* 
 * �����Զ���HTTP����URL 
 * vm_http_set_url - OK
 * vm_http_add_header - OK
 * vm_http_set_body - OK
 */
void http_test_url_header_body()
{ 
	VMINT ret;
	VMINT http_handle;
	asyn_http_req_t req;
	char *longurl, *longname, *longvalue, *longbody;

	vm_log_debug("enter http_test_url_header_body .");

	req.req_method = GET;
	req.use_proxy = HTTP_USE_CMNET_PRIORITY;
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t)); 
	if(NULL == req.http_request)
	{ 
		vm_log_error("http_test_url - NULL == req.http_request.");
		return; 
	}

	/* ȡ���õ�HTTP��� */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
		return;
	vm_log_debug("http_test_url_header_body - after vm_http_fetch_handle http_handle:%d .", http_handle);

	/* ���HTTP ����URL�����Ȳ�Ҫ����400�ֽ� */
	longurl = (char*)vm_malloc(400);
	strcpy(longurl, POST_URL);
	strcat(longurl, "&");
	SetChar(longurl+strlen(POST_URL)+1, 400-strlen(POST_URL)-1);
	vm_http_set_url(http_handle, longurl, strlen(longurl));
	vm_log_debug("http_test_url_header_body - after vm_http_set_url http_handle:%d, longurl:%d.", http_handle, strlen(longurl));
	vm_http_log_debug(longurl, strlen(longurl));

	/* ���HTTP������ͷ��name+value�ĳ��Ȳ�Ҫ����400�ֽ� */
	longname = (char*)vm_malloc(100);
	SetChar(longname, 100);
	longvalue = (char*)vm_malloc(300);
	SetChar(longvalue, 300);
	vm_http_add_header(http_handle, longname, longvalue);
	// vm_http_add_header(http_handle, "name2", "value2");
	vm_log_debug("http_test_url_header_body - after vm_http_add_header http_handle:%d .", http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_add_header longname:%s .", longname);
	vm_http_log_debug(longvalue, strlen(longvalue));

	/* ���HTTP�������壬���������� */
	longbody = (char*)vm_malloc(10240);
	SetChar(longbody, 10240);
	vm_http_set_body(http_handle, (VMUINT8*)longbody, strlen(longbody));
	vm_log_debug("http_test_url_header_body - after vm_http_set_body http_handle:%d .", http_handle);
	vm_http_log_debug(longbody, strlen(longbody));

	/* ��������ǰ��鵱ǰϵͳHTTP�������֤��ǰHTTP��������̴��� */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_set_current_handle http_handle:%d .", http_handle);

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_url_header_body.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb_save2file, http_state_notify_cb);

	vm_log_debug("http_test_url_header_body - after http_test_url_header_body %d .", ret);

	vm_free(req.http_request);
	vm_free(longurl);
	vm_free(longname);
	vm_free(longvalue);
	vm_free(longbody);

	return; 
} 

/* 
 * ����HTTP��Ӧ���������ݷֿ���յĻص����� 
 * vm_http_recv_data - OK
 */ 
int receiver(VMINT handle, void* p) 
{ 
	int ret = -1;
	int data_size = 1024*2+1;
	static int received = 0;   
	VMINT res_size = 0;
	VMCHAR head[255] = {0};
	//VMCHAR data[500] = {0};
	VMCHAR *data;
	http_session_t* session = NULL;
//	VMINT	drv;
	VMUINT nSize = 0;
	
	vm_log_debug("enter receiver - handle:%d .", handle);

	data = (char*)vm_malloc(data_size);

	/* ��ȡ�̷� 
	drv= vm_get_removable_driver();	//���Ȼ�ȡ�ƶ��̷�
	if(drv<0)							//�������ƶ���
		drv=vm_get_system_driver();		//��ȡϵͳ��
	sprintf(file_name, "%c:\\vre\\receiver.jpg", drv);

	file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);*/

	if (handle < 0 || p == NULL)
	{
		strcpy(data, "��������");
		Http_Output(data, TRUE);
		return -1;
	}

	session = (http_session_t*)p;

	vm_log_debug("receiver - res_code:%d", session->res_code);
	vm_http_log_debug((char*)session->resbuf, session->nresbuf);


	/*����������*/
	if( 0 > session->res_code || 500 == session->res_code )
	{
		strcpy(data, "����������");
		Http_Output(data, TRUE);
		return -1;
	}

	/*��ȡContent-Type*/
	if(get_http_head(session, "Content-Type", head))
	{
		strcpy(data, "��ȡContent-Typeʧ��");
		Http_Output(data, TRUE);
		return -1;
	}
	
	/*��"text/vnd.wap.wml"��ͷ�ģ���ʾ���õ���CMWAP���ӣ���Ҫ��������һ��*/
	if(!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) 
	{
		//http_test_receiver_chunk();
		strcpy(data, "CMWAP��������");
		Http_Output(data, FALSE);

		vm_log_debug("receiver(chunked) - need reconnect by CMWAP , head:%s .", head);
		//return -1;
	}

	/*��ȡContent-Length*/
	if(get_http_head(session, "Content-Length", data))
	{ 
		strcpy(data, "��ȡ�ļ���Сʧ��");
		Http_Output(data, TRUE);

		strcpy(data, "��ʼ�������ݷֿ�(chunked)");
		Http_Output(data, FALSE);

		while(1)  /* ѭ������ֱ���յ�chunk end�ź�*/
		{

			ret = data_size;
			memset(data, 0x00, ret--);

			ret = vm_http_recv_data(handle, (VMUINT8*)data, ret);

			vm_log_debug("receiver(chunked) - after vm_http_recv_data - %d, %p .", ret, data);
			vm_http_log_debug(data, strlen(data));

			vm_log_debug("receiver(chunked) - to vm_file_write, file_name:%s . ", file_name);
			vm_file_getfilesize(file_hdl, &nSize);
			vm_file_seek(file_hdl, 0, BASE_END);
			nSize = vm_file_write(file_hdl, data, strlen(data), &nSize);
			vm_log_debug("receiver(chunked) - vm_file_write, file_name:%s, nSize:%d . ", file_name, nSize);

			//�ж�retlen��ֵ��>0; ==0; <0
			if (ret > 0) //������յ����ݣ�����ļ���������ý�岥��
			{
				if (ret == 2 && !strncmp(data, "\r\n", 2) )
				{
					vm_log_debug("receiver - chunk end, data:%s, ret:%d, received:%d .", data, ret, received);
					break;
				}
				//save_file(data, retlen);
				received += ret; 
			}
			else  //retlen<0 
			{
				if (ret == 0)
					strcpy(data, "�������ݷֿ�ʱ��������");
				else
					strcpy(data, "�������ݷֿ�ʱ����");
				Http_Output(data, FALSE);
				return ret;  //error 
			}
		}

		strcpy(data, "��ɽ������ݷֿ�(chunked)");
		Http_Output(data, FALSE);

		vm_file_close(file_hdl);
		vm_free(data);
 
		vm_log_debug("receiver - acomplished received:%d .", received);
		return received;
	}

	/*��ȡ��Ӧ�峤��*/
	res_size = strtoi(data);
	vm_log_debug("receiver - res_size:%d", res_size);

	strcpy(data, "��ʼ�������ݷֿ�");
	Http_Output(data, FALSE);

	while(received < res_size)  //����δ��ɽ���
	{ 
		ret = data_size;
		memset(data, 0x00, ret--);

		ret = vm_http_recv_data(handle, (VMUINT8*)data, ret);

		vm_log_debug("receiver - after vm_http_recv_data - ret:%d, received:%d, %p .", ret, received, data);
		vm_http_log_debug(data, ret);

		vm_log_debug("receiver - to vm_file_write, file_name:%s . ", file_name);
		vm_file_getfilesize(file_hdl, &nSize);
		vm_file_seek(file_hdl, 0, BASE_END);
		nSize = vm_file_write(file_hdl, data, ret, &nSize);
		vm_log_debug("receiver - vm_file_write, file_name:%s, nSize:%d . ", file_name, nSize);

		//�ж�retlen��ֵ��>0; ==0; <0
		if (ret > 0) //������յ����ݣ�����ļ���������ý�岥��
		{
			//save_file(data, retlen);
			received += ret; 
		}
		else  //retlen<0 
		{
			if (ret == 0)
				strcpy(data, "�������ݷֿ�ʱ��������");
			else
				strcpy(data, "�������ݷֿ�ʱ����");
			Http_Output(data, FALSE);
			if (data != NULL)
			{
				vm_free(data);
				data = NULL;
			}
			return ret;  //error 
		}
	}

	strcpy(data, "��ɽ������ݷֿ�");
	Http_Output(data, FALSE);

	vm_file_close(file_hdl);
	vm_free(data);

	vm_log_debug("receiver - acomplished received:%d .", received);
	return received;	//�����ѽ����ܵ��ֽ��� 
}

/*
 * ��ָ�����ݻ���д��ָ�����ȵ��ַ�
 */
void SetChar(char* c, int len)
{
	int i = 0;
	for (i=0; i<len-1; i++)
	{
		*(c+i) = 'a'+i % 26;
	}
	*(c+i) = '\0';
}

/* 
 * ����HTTP�����������ݷֿ鷢�͵Ļص����� 
 * vm_http_send_data - OK
 */ 
int sender(VMINT handle, void* p) 
{ 
	int ret = -1;
	//int data_size = 100, total_size = 1024*10;	//OK
	//int data_size = 1024, total_size = 1024*10;	//OK
	static int sent = 0;
	//VMCHAR data[500] = {0};
	VMCHAR* data;
	int data_size = 100, total_size = strlen(POST_DATA);	//OK
	
	vm_log_debug("enter sender - handle:%d .", handle);

	data = (char*)vm_malloc(data_size);

	strcpy(data, "���ڷ������ݷֿ�");
	Http_Output(data, FALSE);

	//while((len = read_file(data, len)) > 0)  
	while(sent < total_size)		//����δ��ɷ���
	{ 		
		//SetChar(data, data_size);	//׼��Ҫ���͵����ݿ飬����ļ�����
		strcpy(data, POST_DATA);

		ret = strlen(data) < total_size-sent ? strlen(data) : total_size-sent;

		vm_log_debug("sender - before vm_http_send_data - %d, %p .", ret, data);

		ret = vm_http_send_data(handle, (VMUINT8*)data, ret);

		vm_log_debug("sender - after vm_http_send_data - %d, %p .", ret, data);
		vm_http_log_debug(data, strlen(data));

		//�ж�ret ��ֵ��>0; ==0; <0
		if (ret > 0) 
		{
			if (1 == ret)  //���ݷ�����ɣ�����Content-Length�Ƚϡ�
			{
				vm_log_debug("sender - nodified finish by VRE, ret:%d .", ret);
				break;
			}
			sent += ret; 
		}
		else  //retlen  ==0, <0 
		{
			if (ret == 0) 
				strcpy(data, "�������ݷֿ�ʱ��������");
			else
				strcpy(data, "�������ݷֿ����");
			Http_Output(data, FALSE);
			return ret;  //error 
		}
	}

	strcpy(data, "��ɷ������ݷֿ�");
	Http_Output(data, FALSE);

	vm_log_debug("sender - acomplished sent:%d .", sent);
	return sent;	//�����ѷ����ܵ��ֽ��� 
}

/* 
 * ����HTTP������ֿ��ϴ������أ��ϴ�ʱ����������
 */
void http_test_sender_receiver()
{ 
	VMINT ret;
	VMINT http_handle;
	asyn_http_req_t req;

	vm_log_error("enter http_test_sender_receiver . ");

	req.req_method = POST;
	req.use_proxy = HTTP_USE_CMNET_PRIORITY;
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t)); 
	if(NULL == req.http_request)
	{ 
		vm_log_error("http_test_sender_receiver - NULL == req.http_request.");
		return; 
	}

	sprintf(req.http_request->url, POST_URL);
	vm_log_debug("http_test_origin - req.http_request->url %s .", req.http_request->url);

	/* ȡ���õ�HTTP��� */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* �����������Content-Length */
	vm_http_set_body(http_handle, NULL, 10240);

	/* ע��HTTP��������ֿ鷢�ͻص����� */
	vm_http_reg_sender(http_handle, sender);

	/* ע��HTTP��Ӧ������ֿ���ջص����� */
	vm_http_reg_receiver(http_handle, receiver);

	/* ��������ǰ��鵱ǰϵͳHTTP�������֤��ǰHTTP��������̴��� */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_sender_receiver.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb_save2file, http_state_notify_cb);

	vm_log_debug("http_test_sender_receiver - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_sender_receiver . ");
	return; 
} 

/* 
 * ����HTTP������ֿ��ϴ�(chunked)������������
 */
void http_test_sender_chunk()
{ 
	VMINT ret;
	VMINT http_handle;
	asyn_http_req_t req;

	vm_log_error("enter http_test_sender_chunk . ");

	req.req_method = POST;
	req.use_proxy = HTTP_USE_CMNET_PRIORITY;
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t)); 
	if(NULL == req.http_request)
	{ 
		vm_log_error("http_test_url - NULL == req.http_request.");
		return; 
	}

	sprintf(req.http_request->url, POST_URL_CHUNK);
	vm_log_debug("http_test_origin - req.http_request->url %s .", req.http_request->url);

	/* ȡ���õ�HTTP��� */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* ע��HTTP��������ֿ鷢�ͻص����� */
	vm_http_reg_sender(http_handle, sender);

	/* ע��HTTP��Ӧ������ֿ���ջص����� */
	vm_http_reg_receiver(http_handle, receiver);

	/* ��������ǰ��鵱ǰϵͳHTTP�������֤��ǰHTTP��������̴��� */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_sender_chunk.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);

	vm_log_debug("http_test_sender_chunk - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_sender_chunk . ");
	return; 
} 

/* 
 * ����HTTP������ֿ����أ�����������
 */
void http_test_receiver_bigfile()
{ 
	VMINT ret;
	VMINT http_handle;
	asyn_http_req_t req;

	vm_log_error("enter http_test_receiver_bigfile . ");

	req.req_method = GET;
	req.use_proxy = HTTP_USE_CMWAP_PRIORITY;
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t)); 
	if(NULL == req.http_request)
	{ 
		vm_log_error("http_test_url - NULL == req.http_request.");
		return; 
	}

	sprintf(req.http_request->url, BIGFILE_URL);
	vm_log_debug("http_test_origin - req.http_request->url %s .", req.http_request->url);

	/* ȡ���õ�HTTP��� */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* ע��HTTP��Ӧ������ֿ���ջص����� */
	vm_http_reg_receiver(http_handle, receiver);

	/* ��������ǰ��鵱ǰϵͳHTTP�������֤��ǰHTTP��������̴��� */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"e:\\vre\\baidu_logo.gif", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);

	vm_log_debug("http_test_receiver_bigfile - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_receiver_bigfile . ");
	return; 
} 

/* 
 * ����HTTP������ֿ�����(chunked)������������
 */
void http_test_receiver_chunk()
{ 
	VMINT ret;
	VMCHAR* url = "http://www.google.cn/webhp?source=g_cn";
	VMINT http_handle;
	asyn_http_req_t req;

	vm_log_error("enter http_test_receiver_chunk . ");

	req.req_method = GET;
	req.use_proxy = HTTP_USE_CMNET_PRIORITY;
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t)); 
	if(NULL == req.http_request)
	{ 
		vm_log_error("http_test_receiver_chunk - NULL == req.http_request.");
		return; 
	}
	sprintf(req.http_request->url, url);
	vm_log_debug("http_test_receiver_chunk - req.http_request->url %s .", req.http_request->url);

	/* ȡ���õ�HTTP��� */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* ע��HTTP��Ӧ������ֿ���ջص����� */
	vm_http_reg_receiver(http_handle, receiver);

	/* ��������ǰ��鵱ǰϵͳHTTP�������֤��ǰHTTP��������̴��� */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_receiver_chunk.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);

	vm_log_debug("http_test_receiver_chunk - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_receiver_chunk . ");
	return; 
} 

/* 
 * ����ԭvm_http_request����
 */
void http_test_vm_http_request()
{ 
	VMINT ret;
	VMCHAR* url = DOWNLOAD_URL;
//	VMINT http_handle;
	http_request_t* req;
	http_session_t* session;

	vm_log_debug("enter http_test_vm_http_request .");

	req = (http_request_t*)vm_malloc(sizeof(http_request_t));
	memset(req, 0x00, sizeof(http_request_t));
	session = (http_session_t*)vm_malloc(sizeof(http_session_t));
	memset(session, 0x00, sizeof(http_session_t));

	/*��װURL*/
	if(strncmp(url, "http://", strlen("http://")))
	{ 
		strcpy(req->url, "http://"); 
	}
	strcat(req->url, url);
	vm_log_debug("http_test_vm_http_request - req->url %s .", req->url);

	vm_enable_proxy(HTTP_USE_CMNET_PRIORITY);

	vm_reg_http_notification(http_state_notify_cb_ex);

	session->hook = http_hook_cb1;

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_vm_http_request.jpg", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_http_request(req, session);

	vm_log_debug("leave http_test_vm_http_request .");
	return; 
} 

/* 
 * ����ԭvm_http_request����
 */
void http_test_new_vm_http_request()
{ 
	VMINT ret;
	VMCHAR* url = DOWNLOAD_URL;
	VMINT http_handle;
	http_request_t* req;
	http_session_t* session;

	vm_log_debug("enter http_test_new_vm_http_request .");

	req = (http_request_t*)vm_malloc(sizeof(http_request_t));
	memset(req, 0x00, sizeof(http_request_t));
	session = (http_session_t*)vm_malloc(sizeof(http_session_t));
	memset(session, 0x00, sizeof(http_session_t));

	/*��װURL*/
	if(strncmp(url, "http://", strlen("http://")))
	{ 
		strcpy(req->url, "http://"); 
	}
	strcat(req->url, url);
	vm_log_debug("http_test_new_vm_http_request - req->url %s .", req->url);

	/* ȡ���õ�HTTP��� */
	http_handle = vm_http_fetch_handle(HTTP_REQUEST, req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_receiver_chunk - http_handle < 0.");
		return;
	}

	/* ��������ǰ��鵱ǰϵͳHTTP�������֤��ǰHTTP��������̴��� */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	vm_enable_proxy(HTTP_USE_CMNET_PRIORITY);

	vm_reg_http_notification(http_state_notify_cb_ex);

	session->hook = http_hook_cb1;

	/*����HTTP����*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_new_vm_http_request.jpg", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_http_request(req, session);

	vm_log_debug("leave http_test_new_vm_http_request .");
	return; 
} 

//static i_data = 0;
// int GetLongPostData(char* data, int len)
// {
// 	int ret = 0;
// 	char tmp[50];
// 	while(ret < len)
// 	{
// 		memset(tmp, 0x00, sizeof(tmp));
// 		sprintf(tmp, "name%d=value%d&", i_data, i_data);
// 		strncpy(data, tmp, strlen(tmp));
// 		strcat(data, tmp);
// 		ret += strlen(tmp);
// 	}
// }

void ShowAllHTTPConnection()
{
	int i;
	char text[100];

	sprintf(text, "Active HTTPs:");
	Http_Output(text, TRUE);
	sprintf(text, "---------------");
	Http_Output(text, TRUE);
	for (i=0; i<sizeof(http_hdl); i++)
	{
		if (http_hdl[i] > -1)
		{
			sprintf(text, "HTTP[%d] is processing", http_hdl[i]);
			Http_Output(text, FALSE);
		}
	}
	sprintf(text, "------------");
	Http_Output(text, TRUE);
}

// static void http_hook_cb_timer_http(VMINT bResponse, void* pSession)
// {	
// 	vm_log_debug("enter http_hook_cb_timer_http .");
// 	if(0 == bResponse)
// 	{
// 		vm_log_debug("http_hook_cb_timer_http - before http_test_timer_http .");
// 		http_test_timer_http();
// 		vm_log_debug("http_hook_cb_timer_http - after http_test_timer_http .");
// 	}
// 	vm_log_debug("leave http_hook_cb_timer_http .");
// }

static void http_test_timer_http_timer_proc(VMINT tid)
{
	http_test_origin();
}

/* 
 * ����ԭHTTP���� (OK)
 */
void http_test_timer_http()
{ 
	vm_log_debug("enter http_test_timer_http .");
	vm_create_timer(3000, (VM_TIMERPROC_T)http_test_timer_http_timer_proc);
	vm_log_debug("leave http_test_timer_http .");
} 