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

/*下载文件地址*/
#define DOWNLOAD_URL	"http://www.baidu.com/img/baidu_logo.gif"	//"http://61.236.127.192/ucenter/data/avatar/000/00/00/01_avatar_middle.jpg"

#define BIGFILE_URL		"http://www.myweddingday.cn/chuanqi.mp3"//"http://xm.dreammail.org/DreamMail4_Setup.exe"

#define POST_URL		"http://www.vspace.net.cn/vresps/test_http.jsp?qs=sb"

#define POST_URL_CHUNK	"http://www.vspace.net.cn/vresps/test_http.jsp"

#define POST_DATA		"name=linminqi&sex=male&company=vogins"

/*HTTP端口*/
#define PORT 80esst
/*下载分块大小*/
#define CHUNK_SIZE 200

/*下载文件大小*/
static VMUINT file_size = 0;
static VMCHAR file_name[50] = {0};
static VMFILE file_hdl = -1;
/*HTTP句柄*/
static VMINT http_hdl[12] = {-1};

/*发起下载请求*/
static VMINT HttpDownload(const VMCHAR* url, const VMCHAR* cnt,
						  VM_HTTP_PROXY_TYPE apn,	HTTP_METHOD mth, VMINT* handle,
						  void (*http_hook_cb)(VMINT bResponse, void* pSession), 
						  void (*http_state_notify_cb)(VMINT state, VMINT param, void* session));
/*HTTP状态回调函数*/
static void http_state_notify_cb(VMINT state, VMINT param, void* session);
static void http_state_notify_cb_ex(VMINT state, VMINT param);
/*HTTP响应回调函数*/
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
		strcpy(text, "正在获取主机");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_CONNECTING:
		strcpy(text, "正在连接");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_SENDING:
		strcpy(text, "正在发送请求报文");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_STATUS:
		strcpy(text, "正在接收状态信息");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_HEADS:
		strcpy(text, "正在接收响应头");
		Http_Output(text, FALSE);
		break;
	case HTTP_STATE_RECV_BODY:
		strcpy(text, "正在接收响应体");
		Http_Output(text, FALSE);
		break;	
	default:
		strcpy(text, "未知事件");
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
	
	/* 获取盘符 */
	drv= vm_get_removable_driver();	//优先获取移动盘符
	if(drv<0)							//不存在移动盘
		drv=vm_get_system_driver();		//获取系统盘
	sprintf(file_name, "%c:\\vre\\logo_vre.tmp", drv);
	/*正确的响应*/
	if(0 == bResponse)
	{
		session = (http_session_t*)pSession;
		
		/*服务器错误*/
		if( 0 > session->res_code || 500 == session->res_code )
		{
			strcpy(text, "服务器出错");
			Http_Output(text, TRUE);
			return;
		}
		
		/*获取Content-Type*/
		if(get_http_head(session, "Content-Type", head))
		{
			strcpy(text, "获取Content-Type失败");
			Http_Output(text, TRUE);
			return;
		}
		
		/*以"text/vnd.wap.wml"开头的，表示采用的是CMWAP连接，需要重新连接一次*/
		if(!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) 
		{
			StartHttpDownload();
			strcpy(text, "CMWAP重新连接");
			Http_Output(text, TRUE);
			return;
		}

		/*下载生成的临时文件全路径名*/
		sprintf(file_name, "%c:\\vre\\logo_vre.tmp", drv);
		/*第一次下载该文件*/
		if(0 == file_size)
		{
			/*创建临时文件*/
			file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);
			if(file_hdl >= 0)
			{				
				/*获取Content-Range*/
				if(get_http_head(session, "Content-Range", head))
				{ 
					vm_file_close(file_hdl);
					vm_file_delete(vm_ucs2_string(file_name));

					strcpy(text, "获取文件大小失败");
					Http_Output(text, TRUE);
					return;
				}

				/*获取文件大小*/
				file_size = strtoi(strstr(head, "/") + 1);
				/*临时文件的头4个字节存放文件大小*/
				vm_file_write(file_hdl, &file_size, 4, &written);
				/*接下来4个字节存放下次下载开始的位置*/
				vm_file_write(file_hdl, &start_size, 4, &written);
				vm_file_close(file_hdl);
			}		

			memset(text, 0, 255);
			sprintf(text, "文件大小：%d字节", file_size);
			Http_Output(text, TRUE);
		}
		/*不是第一次下载该文件*/
		else
		{		
			/*更新下次下载开始的位置*/
			Http_ReadFile(vm_ucs2_string(file_name), 4, 4, &start_size);
			start_size += session->nresbody;
			file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_WRITE, TRUE);
			vm_file_seek(file_hdl, 4, BASE_BEGIN);
			vm_file_write(file_hdl, &start_size, 4, &written);
			/*将下载的数据写入临时文件*/
			vm_file_seek(file_hdl, 0, BASE_END);
			vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
			vm_file_close(file_hdl);
			
			memset(text, 0, 255);
			sprintf(text, "下载成功，共下载%d字节", session->nresbody);	
			Http_Output(text, TRUE);

			/*下载完成*/
			if(start_size == file_size)
			{
				VMCHAR* data;
				data = vm_malloc(file_size);
				Http_ReadFile(vm_ucs2_string(file_name), 8, file_size, data);
				vm_file_delete(vm_ucs2_string(file_name));
				/*另存为新文件*/
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
	
	/*继续下载剩余的部分*/
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
	
	/*获取文件下载状态*/
	drv= vm_get_removable_driver();	//优先获取移动盘符
	if(drv<0)							//不存在移动盘
		drv=vm_get_system_driver();		//获取系统盘
		
	sprintf(full_name, "%c:\\vre\\logo_vre.*", drv);
	vm_log_debug("full_name:%s", full_name);
	if((find_hdl = vm_find_first(vm_ucs2_string(full_name), &info)) >= 0)
	{
		vm_ucs2_to_gb2312(file_name, 50, info.filename);
		if(!vm_ends_with(file_name, "jpg"))
		{
			sprintf(text, "文件已下载完");
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
	
	/*请求方式*/
	req.req_method = mth;
	/*网络接入方式*/
	req.use_proxy = apn;
	
	/*组装HTTP请求*/
	req.http_request = (http_request_t*)vm_calloc(sizeof(http_request_t));
	if(NULL == req.http_request)
	{
		return -1;
	}
	
	/*组装URL*/
	if(strncmp(url, "http://", strlen("http://")))
	{
		strcat(req.http_request->url, "http://");
	}
	strcat(req.http_request->url, url);
	
	/*组装HTTP请求头*/
	if(0 == file_size)
	{
		sprintf(head[0].name, "RANGE");		
		sprintf(head[0].value, "bytes=-1");

		memset(text, 0, 255);
		sprintf(text, "请求获取文件大小");
		Http_Output(text, FALSE);
	}
	else 
	{
		sprintf(head[0].name, "RANGE");
		memset(text, 0, 255);		
		sprintf(head[0].value, "bytes=%d-%d", start_size,
					((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
		sprintf(text, "请求下载文件:从第%d字节到第%d字节", start_size, 
					((start_size + CHUNK_SIZE - 1) > file_size ? file_size : (start_size + CHUNK_SIZE - 1)));
		Http_Output(text, FALSE);
	}	
	req.http_request->nhead = 1;
	req.http_request->heads = head;
	
	/*发送HTTP请求*/
	ret = vm_asyn_http_req(&req, http_hook_cb, http_state_notify_cb);
	
	/*获取HTTP句柄*/
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
		sprintf(text, "中断HTTP下载\n");
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
	
	/* 获取盘符 
	drv= vm_get_removable_driver();	//优先获取移动盘符
	if(drv<0)							//不存在移动盘
		drv=vm_get_system_driver();		//获取系统盘
	sprintf(file_name, "%c:\\vre\\http_result.txt", drv);*/
	
	vm_log_debug("enter http_hook_cb_save2file - bResponse:%d", bResponse);
	/*正确的响应*/
	if(0 == bResponse)
	{
		session = (http_session_t*)pSession;
		
		vm_log_debug("http_hook_cb_save2file - res_code:%d, nresbuf:%d", session->res_code, session->nresbuf);
		vm_http_log_debug((char*)session->resbuf, session->nresbuf);
		
		/*获取文件大小*/
		file_size = strtoi(head);
		vm_log_debug("http_hook_cb_save2file - file_size:%d", file_size);
		
		/*创建 文件
		file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);*/
		
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, session->resbuf, session->nresbuf, &written);
		
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, "\r\n", strlen("\r\n"), &written);

		/*将下载的数据写入 文件*/
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
		vm_file_close(file_hdl);
				
		strcpy(text, "下载的文件已保存");
		Http_Output(text, FALSE);
		
		vm_log_debug("http_hook_cb_save2file - nresbody:%d", session->nresbody); 
		vm_http_log_debug((char*)session->resbody, session->nresbody);
	}
	else
	{
		strcpy(text, "错误的HTTP结果");
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
	
	/* 获取盘符 
	drv= vm_get_removable_driver();	//优先获取移动盘符
	if(drv<0)							//不存在移动盘
		drv=vm_get_system_driver();		//获取系统盘
	sprintf(file_name, "%c:\\vre\\logo_vre2_http.jpg", drv);*/

	vm_log_debug("enter http_hook_cb1 - bResponse:%d", bResponse);
	/*正确的响应*/
	if(0 == bResponse)
	{
		session = (http_session_t*)pSession;

		vm_log_debug("http_hook_cb1 - res_code:%d, nresbuf:%d", session->res_code, session->nresbuf);
		vm_http_log_debug((char*)session->resbuf, session->nresbuf);

		if (session->resbody == NULL)
		{
			strcpy(text, "Session中的响应体为空");
			Http_Output(text, TRUE);
			//return;
		}

		/*服务器错误*/
		if( 0 > session->res_code || 500 == session->res_code )
		{
			strcpy(text, "服务器出错");
			Http_Output(text, TRUE);
			return;
		}

		/*获取Content-Type*/
		if(get_http_head(session, "Content-Type", head))
		{
			strcpy(text, "获取Content-Type失败");
			Http_Output(text, TRUE);
			//return;
		}
		
		/*以"text/vnd.wap.wml"开头的，表示采用的是CMWAP连接，需要重新连接一次*/
		if(!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) 
		{
			// StartHttpDownload();
			strcpy(text, "CMWAP重新连接");
			Http_Output(text, FALSE);
			vm_cancel_asyn_http_req(http_hdl[0]);
			http_test_receiver_bigfile();
			return;
		}

		/*获取Content-Length*/
		if(get_http_head(session, "Content-Length", head))
		{ 
			strcpy(text, "获取文件大小失败");
			Http_Output(text, TRUE);
			return;
		}

		/*获取文件大小*/
		file_size = strtoi(head);
		vm_log_debug("http_hook_cb1 - file_hdl:%d, file_size:%d", file_hdl, file_size);

		/*创建文件
		file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);*/

		/*将下载的数据写入文件*/
		vm_file_seek(file_hdl, 0, BASE_END);
		vm_file_write(file_hdl, session->resbody, session->nresbody, &written);
		vm_file_close(file_hdl);

		strcpy(text, "下载的文件已保存");
		Http_Output(text, FALSE);

		vm_log_debug("http_hook_cb1 - nresbody:%d", session->nresbody); 
		vm_http_log_debug((char*)session->resbody, session->nresbody);
	}
	else
	{
		strcpy(text, "错误的HTTP结果");
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
 * 测试原HTTP流程 (OK)
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

	/*发送HTTP请求*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_origin.jpg", MODE_CREATE_ALWAYS_WRITE, TRUE);
	vm_log_debug("d:\\vre\\http_test_origin.jpg");
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);
	vm_log_debug("http_test_origin - after vm_asyn_http_req %d .", ret);

	/*获取HTTP句柄*/
	if(ASYN_HTTP_REQ_ACCEPT_SUCCESS == ret)
	{
		ret = vm_get_asyn_http_req_handle(&req, &http_hdl[0]); 
	}

	vm_free(req.http_request);

	return; 
} 

/* 
 * 测试原HTTP流程 (OK)
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

	/* 取可用的HTTP句柄 */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
		return;
	vm_log_debug("http_test_url_header_body - after vm_http_fetch_handle http_handle:%d .", http_handle);
	
	/* 添加HTTP 请求URL，长度不要超过400字节 
	longurl = (char*)vm_malloc(400);
	strcpy(longurl, POST_URL);
	strcat(longurl, "&");
	SetChar(longurl+strlen(POST_URL)+1, 400-strlen(POST_URL)-1);
	vm_http_set_url(http_handle, longurl, strlen(longurl));
	vm_log_debug("http_test_url_header_body - after vm_http_set_url http_handle:%d, longurl:%d.", http_handle, strlen(longurl));
	vm_http_log_debug(longurl, strlen(longurl));*/
	
	/* 添加HTTP请求报文头，name+value的长度不要超过400字节 
	longname = (char*)vm_malloc(100);
	SetChar(longname, 100);
	longvalue = (char*)vm_malloc(300);
	SetChar(longvalue, 300);
	vm_http_add_header(http_handle, longname, longvalue);
	//vm_http_add_header(http_handle, "name2", "value2");
	vm_log_debug("http_test_url_header_body - after vm_http_add_header http_handle:%d .", http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_add_header longname:%s .", longname);
	vm_http_log_debug(longvalue, strlen(longvalue));*/
	
	/* 添加HTTP请求报文体，长度无限制 
	longbody = (char*)vm_malloc(10240);
	//strcpy(longbody, POST_DATA);
	SetChar(longbody, 10240);
	vm_http_set_body(http_handle, (VMUINT8*)longbody, strlen(longbody));
	vm_log_debug("http_test_url_header_body - after vm_http_set_body http_handle:%d .", http_handle);
	vm_http_log_debug(longbody, strlen(longbody));*/
	
	/* 在请求发送前检查当前系统HTTP句柄，保证当前HTTP句柄被立刻处理 */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_set_current_handle http_handle:%d .", http_handle);
	
	/*发送HTTP请求*/
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
 * 测试自定义HTTP请求URL 
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

	/* 取可用的HTTP句柄 */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
		return;
	vm_log_debug("http_test_url_header_body - after vm_http_fetch_handle http_handle:%d .", http_handle);

	/* 添加HTTP 请求URL，长度不要超过400字节 */
	longurl = (char*)vm_malloc(400);
	strcpy(longurl, POST_URL);
	strcat(longurl, "&");
	SetChar(longurl+strlen(POST_URL)+1, 400-strlen(POST_URL)-1);
	vm_http_set_url(http_handle, longurl, strlen(longurl));
	vm_log_debug("http_test_url_header_body - after vm_http_set_url http_handle:%d, longurl:%d.", http_handle, strlen(longurl));
	vm_http_log_debug(longurl, strlen(longurl));

	/* 添加HTTP请求报文头，name+value的长度不要超过400字节 */
	longname = (char*)vm_malloc(100);
	SetChar(longname, 100);
	longvalue = (char*)vm_malloc(300);
	SetChar(longvalue, 300);
	vm_http_add_header(http_handle, longname, longvalue);
	// vm_http_add_header(http_handle, "name2", "value2");
	vm_log_debug("http_test_url_header_body - after vm_http_add_header http_handle:%d .", http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_add_header longname:%s .", longname);
	vm_http_log_debug(longvalue, strlen(longvalue));

	/* 添加HTTP请求报文体，长度无限制 */
	longbody = (char*)vm_malloc(10240);
	SetChar(longbody, 10240);
	vm_http_set_body(http_handle, (VMUINT8*)longbody, strlen(longbody));
	vm_log_debug("http_test_url_header_body - after vm_http_set_body http_handle:%d .", http_handle);
	vm_http_log_debug(longbody, strlen(longbody));

	/* 在请求发送前检查当前系统HTTP句柄，保证当前HTTP句柄被立刻处理 */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);
	vm_log_debug("http_test_url_header_body - after vm_http_set_current_handle http_handle:%d .", http_handle);

	/*发送HTTP请求*/
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
 * 处理HTTP相应报文体数据分块接收的回调函数 
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

	/* 获取盘符 
	drv= vm_get_removable_driver();	//优先获取移动盘符
	if(drv<0)							//不存在移动盘
		drv=vm_get_system_driver();		//获取系统盘
	sprintf(file_name, "%c:\\vre\\receiver.jpg", drv);

	file_hdl = vm_file_open(vm_ucs2_string(file_name), MODE_CREATE_ALWAYS_WRITE, TRUE);*/

	if (handle < 0 || p == NULL)
	{
		strcpy(data, "参数错误");
		Http_Output(data, TRUE);
		return -1;
	}

	session = (http_session_t*)p;

	vm_log_debug("receiver - res_code:%d", session->res_code);
	vm_http_log_debug((char*)session->resbuf, session->nresbuf);


	/*服务器错误*/
	if( 0 > session->res_code || 500 == session->res_code )
	{
		strcpy(data, "服务器出错");
		Http_Output(data, TRUE);
		return -1;
	}

	/*获取Content-Type*/
	if(get_http_head(session, "Content-Type", head))
	{
		strcpy(data, "获取Content-Type失败");
		Http_Output(data, TRUE);
		return -1;
	}
	
	/*以"text/vnd.wap.wml"开头的，表示采用的是CMWAP连接，需要重新连接一次*/
	if(!strncmp(head, "text/vnd.wap.wml", strlen("text/vnd.wap.wml"))) 
	{
		//http_test_receiver_chunk();
		strcpy(data, "CMWAP重新连接");
		Http_Output(data, FALSE);

		vm_log_debug("receiver(chunked) - need reconnect by CMWAP , head:%s .", head);
		//return -1;
	}

	/*获取Content-Length*/
	if(get_http_head(session, "Content-Length", data))
	{ 
		strcpy(data, "获取文件大小失败");
		Http_Output(data, TRUE);

		strcpy(data, "开始接收数据分块(chunked)");
		Http_Output(data, FALSE);

		while(1)  /* 循环接收直至收到chunk end信号*/
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

			//判断retlen的值：>0; ==0; <0
			if (ret > 0) //处理接收的数据，如存文件操作，流媒体播放
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
					strcpy(data, "接收数据分块时网络阻塞");
				else
					strcpy(data, "接收数据分块时出错");
				Http_Output(data, FALSE);
				return ret;  //error 
			}
		}

		strcpy(data, "完成接收数据分块(chunked)");
		Http_Output(data, FALSE);

		vm_file_close(file_hdl);
		vm_free(data);
 
		vm_log_debug("receiver - acomplished received:%d .", received);
		return received;
	}

	/*获取响应体长度*/
	res_size = strtoi(data);
	vm_log_debug("receiver - res_size:%d", res_size);

	strcpy(data, "开始接收数据分块");
	Http_Output(data, FALSE);

	while(received < res_size)  //数据未完成接收
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

		//判断retlen的值：>0; ==0; <0
		if (ret > 0) //处理接收的数据，如存文件操作，流媒体播放
		{
			//save_file(data, retlen);
			received += ret; 
		}
		else  //retlen<0 
		{
			if (ret == 0)
				strcpy(data, "接收数据分块时网络阻塞");
			else
				strcpy(data, "接收数据分块时出错");
			Http_Output(data, FALSE);
			if (data != NULL)
			{
				vm_free(data);
				data = NULL;
			}
			return ret;  //error 
		}
	}

	strcpy(data, "完成接收数据分块");
	Http_Output(data, FALSE);

	vm_file_close(file_hdl);
	vm_free(data);

	vm_log_debug("receiver - acomplished received:%d .", received);
	return received;	//返回已接收总的字节数 
}

/*
 * 向指定数据缓存写入指定长度的字符
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
 * 处理HTTP请求报文体数据分块发送的回调函数 
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

	strcpy(data, "正在发送数据分块");
	Http_Output(data, FALSE);

	//while((len = read_file(data, len)) > 0)  
	while(sent < total_size)		//数据未完成发送
	{ 		
		//SetChar(data, data_size);	//准备要发送的数据块，如读文件操作
		strcpy(data, POST_DATA);

		ret = strlen(data) < total_size-sent ? strlen(data) : total_size-sent;

		vm_log_debug("sender - before vm_http_send_data - %d, %p .", ret, data);

		ret = vm_http_send_data(handle, (VMUINT8*)data, ret);

		vm_log_debug("sender - after vm_http_send_data - %d, %p .", ret, data);
		vm_http_log_debug(data, strlen(data));

		//判断ret 的值：>0; ==0; <0
		if (ret > 0) 
		{
			if (1 == ret)  //数据发送完成，经与Content-Length比较。
			{
				vm_log_debug("sender - nodified finish by VRE, ret:%d .", ret);
				break;
			}
			sent += ret; 
		}
		else  //retlen  ==0, <0 
		{
			if (ret == 0) 
				strcpy(data, "发送数据分块时网络阻塞");
			else
				strcpy(data, "发送数据分块出错");
			Http_Output(data, FALSE);
			return ret;  //error 
		}
	}

	strcpy(data, "完成发送数据分块");
	Http_Output(data, FALSE);

	vm_log_debug("sender - acomplished sent:%d .", sent);
	return sent;	//返回已发送总的字节数 
}

/* 
 * 测试HTTP请求体分块上传和下载，上传时可能有阻塞
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

	/* 取可用的HTTP句柄 */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* 设置请求体的Content-Length */
	vm_http_set_body(http_handle, NULL, 10240);

	/* 注册HTTP请求报文体分块发送回调函数 */
	vm_http_reg_sender(http_handle, sender);

	/* 注册HTTP响应报文体分块接收回调函数 */
	vm_http_reg_receiver(http_handle, receiver);

	/* 在请求发送前检查当前系统HTTP句柄，保证当前HTTP句柄被立刻处理 */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*发送HTTP请求*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_sender_receiver.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb_save2file, http_state_notify_cb);

	vm_log_debug("http_test_sender_receiver - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_sender_receiver . ");
	return; 
} 

/* 
 * 测试HTTP请求体分块上传(chunked)，可能有阻塞
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

	/* 取可用的HTTP句柄 */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* 注册HTTP请求报文体分块发送回调函数 */
	vm_http_reg_sender(http_handle, sender);

	/* 注册HTTP响应报文体分块接收回调函数 */
	vm_http_reg_receiver(http_handle, receiver);

	/* 在请求发送前检查当前系统HTTP句柄，保证当前HTTP句柄被立刻处理 */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*发送HTTP请求*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_sender_chunk.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);

	vm_log_debug("http_test_sender_chunk - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_sender_chunk . ");
	return; 
} 

/* 
 * 测试HTTP请求体分块下载，可能有阻塞
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

	/* 取可用的HTTP句柄 */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* 注册HTTP响应报文体分块接收回调函数 */
	vm_http_reg_receiver(http_handle, receiver);

	/* 在请求发送前检查当前系统HTTP句柄，保证当前HTTP句柄被立刻处理 */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*发送HTTP请求*/
	file_hdl = vm_file_open((VMWSTR)L"e:\\vre\\baidu_logo.gif", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);

	vm_log_debug("http_test_receiver_bigfile - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_receiver_bigfile . ");
	return; 
} 

/* 
 * 测试HTTP请求体分块下载(chunked)，可能有阻塞
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

	/* 取可用的HTTP句柄 */
	http_handle = vm_http_fetch_handle(ASYN_HTTP_REQ, &req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_url - http_handle < 0.");
		return;
	}

	/* 注册HTTP响应报文体分块接收回调函数 */
	vm_http_reg_receiver(http_handle, receiver);

	/* 在请求发送前检查当前系统HTTP句柄，保证当前HTTP句柄被立刻处理 */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	/*发送HTTP请求*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_receiver_chunk.txt", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_asyn_http_req(&req, http_hook_cb1, http_state_notify_cb);

	vm_log_debug("http_test_receiver_chunk - after vm_asyn_http_req, ret:%d. ", ret);

	vm_free(req.http_request);

	vm_log_error("leave http_test_receiver_chunk . ");
	return; 
} 

/* 
 * 测试原vm_http_request流程
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

	/*组装URL*/
	if(strncmp(url, "http://", strlen("http://")))
	{ 
		strcpy(req->url, "http://"); 
	}
	strcat(req->url, url);
	vm_log_debug("http_test_vm_http_request - req->url %s .", req->url);

	vm_enable_proxy(HTTP_USE_CMNET_PRIORITY);

	vm_reg_http_notification(http_state_notify_cb_ex);

	session->hook = http_hook_cb1;

	/*发送HTTP请求*/
	file_hdl = vm_file_open((VMWSTR)L"d:\\vre\\http_test_vm_http_request.jpg", MODE_CREATE_ALWAYS_WRITE, TRUE);
	ret = vm_http_request(req, session);

	vm_log_debug("leave http_test_vm_http_request .");
	return; 
} 

/* 
 * 测试原vm_http_request流程
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

	/*组装URL*/
	if(strncmp(url, "http://", strlen("http://")))
	{ 
		strcpy(req->url, "http://"); 
	}
	strcat(req->url, url);
	vm_log_debug("http_test_new_vm_http_request - req->url %s .", req->url);

	/* 取可用的HTTP句柄 */
	http_handle = vm_http_fetch_handle(HTTP_REQUEST, req);
	if (http_handle < 0) 
	{
		vm_log_error("http_test_receiver_chunk - http_handle < 0.");
		return;
	}

	/* 在请求发送前检查当前系统HTTP句柄，保证当前HTTP句柄被立刻处理 */
	if (http_handle != vm_http_get_current_handle()) 
		vm_http_set_current_handle(http_handle);

	vm_enable_proxy(HTTP_USE_CMNET_PRIORITY);

	vm_reg_http_notification(http_state_notify_cb_ex);

	session->hook = http_hook_cb1;

	/*发送HTTP请求*/
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
 * 测试原HTTP流程 (OK)
 */
void http_test_timer_http()
{ 
	vm_log_debug("enter http_test_timer_http .");
	vm_create_timer(3000, (VM_TIMERPROC_T)http_test_timer_http_timer_proc);
	vm_log_debug("leave http_test_timer_http .");
} 