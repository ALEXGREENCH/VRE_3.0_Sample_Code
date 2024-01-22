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
 * tcp.c演示如何使用VRE应用接口来开发一个使用tcp进行网络链接的应用程序。
 *  
 * 
 *
 * Copyright (c) 2009 Vogins Network Technology (China). All rights reserved.
 */

#include "Network.h"
#include "vmsock.h"
#include "vmstdlib.h"
#include <stdio.h>
#include <string.h>

#define DOMAIN_1	"www.vogins.com"						/*域名1*/
#define DOMAIN_2	"www.51vre.com"							/*域名2*/
#define DOMAIN_3	"forum.vogins.com"						/*域名3*/
#define IP_1		"10.0.0.172"							/*IP地址1*/
#define IP_2		"10.0.0.172"							/*IP地址2*/
#define IP_3		"61.236.127.192"						/*IP地址3*/
#define PORT		80										/*端口*/
#define CONTENT_1	"Hi! 我第1个很酷，因为我在折腾VRE！"	/*待发送内容*/
#define CONTENT_2	"Hi! 我第2个很酷，因为我在折腾VRE！"	/*待发送内容*/
#define CONTENT_3	"Hi! 我第3个很酷，因为我在折腾VRE！"	/*待发送内容*/

static VMINT h_tcp1, h_tcp2, h_tcp3;					/*目前VRE最多同时支持3个链接*/

/* TCP链接1的回调 */
static void tcp_cb_1(VMINT handle, VMINT event)
{
	switch(event)
	{
	case VM_TCP_EVT_CONNECTED:
		{
			Tcp_Output("TCP链接1已连接。", -1, FALSE);
		}	
	case VM_TCP_EVT_CAN_WRITE: //可以发送数据了
		{
			int nLen = strlen(CONTENT_1);
			//发送数据
			Tcp_Output("TCP链接1正在发送数据：", -1, FALSE);
			Tcp_Output(CONTENT_1, -1, FALSE);
			
			if(nLen != vm_tcp_write(h_tcp1, CONTENT_1, nLen))
			{
				Tcp_Output("TCP链接1发送失败！", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP链接1数据发送成功！", -1, FALSE);
			}
		}
		return;
		
	case VM_TCP_EVT_CAN_READ: //可以读数据了
		{			
			VMCHAR* pTemp = vm_calloc(10240);
			VMINT len = 0;
			if(!pTemp)
			{
				Tcp_Output("TCP链接1内存分配失败！", -1, TRUE);
				return;
			}

			//读取数据
			len = vm_tcp_read(h_tcp1, pTemp, 10240);
			if(0 > len)
			{
				vm_tcp_close(handle);
				Tcp_Output("TCP链接1接收失败！", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP链接1接收数据成功！数据为：", -1, FALSE);
				Tcp_Output(pTemp, len, TRUE);
			}			
			vm_free(pTemp);
		}
	
		return;
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			Tcp_Output("TCP链接1已中断。", -1, TRUE);
		}
		return;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			Tcp_Output("TCP链接1未发现主机，DNS解析失败。", -1, TRUE);
		}
		return;
	}

	Tcp_Output("TCP链接1其他事件", -1, TRUE);
}

/* TCP链接2的回调 */
static void tcp_cb_2(VMINT handle, VMINT event)
{
	switch(event)
	{
	case VM_TCP_EVT_CONNECTED:
		{
			Tcp_Output("TCP链接2已连接。", -1, FALSE);
		}		
	case VM_TCP_EVT_CAN_WRITE: //可以发送数据了
		{
			int nLen = strlen(CONTENT_2);
			//发送数据
			Tcp_Output("TCP链接2正在发送数据：", -1, FALSE);
			Tcp_Output(CONTENT_2, -1, FALSE);
			
			if(nLen != vm_tcp_write(h_tcp2, CONTENT_2, nLen))
			{
				Tcp_Output("TCP链接2发送失败！", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP链接2数据发送成功！", -1, FALSE);
			}
		}
		return;
	 
	case VM_TCP_EVT_CAN_READ: //可以读数据了
		{
			VMCHAR* pTemp = vm_calloc(10240);
			VMINT len = 0;

			if( !pTemp )
			{
				Tcp_Output("TCP链接2内存分配失败！", -1, TRUE);

				return;
			}

			//读取数据
			len = vm_tcp_read(h_tcp2, pTemp, 10240);
			if( 0 > len )
			{
				vm_tcp_close(handle);
				Tcp_Output("TCP链接2接收失败！", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP链接2接收数据成功！数据为：", -1, FALSE);
				Tcp_Output(pTemp, len, TRUE);
			}
			
			vm_free(pTemp);
		}
		return;
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			Tcp_Output("TCP链接2已中断。", -1, TRUE);
		}
		return;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			Tcp_Output("TCP链接2未发现主机，DNS解析失败。", -1, TRUE);
		}
		return;	
	}

	Tcp_Output("TCP链接2其他事件", -1, TRUE);
}

/* TCP链接3的回调 */
static void tcp_cb_3(VMINT handle, VMINT event)
{
	switch(event)
	{
	case VM_TCP_EVT_CONNECTED:
		{
			Tcp_Output("TCP链接3已连接。", -1, FALSE);
		}	
	case VM_TCP_EVT_CAN_WRITE: //可以发送数据了
		{
			int nLen = strlen(CONTENT_3);
			//发送数据
			Tcp_Output("TCP链接3正在发送数据：", -1, FALSE);
			Tcp_Output(CONTENT_3, -1, FALSE);

			if( nLen != vm_tcp_write(h_tcp3, CONTENT_3, nLen) )
			{
				Tcp_Output("TCP链接3发送失败！", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP链接3数据发送成功！", -1, FALSE);
			}
		}
		return;
	case VM_TCP_EVT_CAN_READ: //可以读数据了
		{
			VMCHAR* pTemp = vm_calloc(10240);
			VMINT len = 0;

			if( !pTemp )
			{
				Tcp_Output("TCP链接3内存分配失败！", -1, TRUE);

				return;
			}

			//读取数据
			len = vm_tcp_read(h_tcp3, pTemp, 10240);
			if( 0 > len )
			{
				vm_tcp_close(handle);
				Tcp_Output("TCP链接3接收失败！", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP链接3接收数据成功！数据为：", -1, FALSE);
				Tcp_Output(pTemp, len, TRUE);
			}
			
			vm_free(pTemp);
		}	
		return;
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			Tcp_Output("TCP链接3已中断。", -1, TRUE);
		}
		return;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			Tcp_Output("TCP链接3未发现主机，DNS解析失败。", -1, TRUE);
		}
		return;		
	}

	Tcp_Output("TCP链接3其他事件", -1, TRUE);
}

VMINT StartTcpDemo(void)
{
	//开始发起3个链接
	h_tcp1 = vm_tcp_connect (IP_1, PORT, VM_TCP_APN_CMWAP, tcp_cb_1);
	if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp1 || VM_TCP_ERR_CREATE_FAILED == h_tcp1 )
	{
		if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp1 )
			Tcp_Output("TCP链接1建立失败！没有足够的资源！", -1, TRUE);
		else
			Tcp_Output("TCP链接1建立失败！创建套接字失败！", -1, TRUE);
		
		return h_tcp1;
	}

	Tcp_Output("TCP链接1建立成功！", -1, FALSE);
	
	h_tcp2 = vm_tcp_connect (IP_2, PORT, VM_TCP_APN_CMWAP, tcp_cb_2);
	if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp2 || VM_TCP_ERR_CREATE_FAILED == h_tcp2 )
	{
		if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp2 )
			Tcp_Output("TCP链接2建立失败！没有足够的资源！", -1, TRUE);
		else
			Tcp_Output("TCP链接2建立失败！创建套接字失败！", -1, TRUE);
		
		return h_tcp2;
	}

	Tcp_Output("TCP链接2建立成功！", -1, FALSE);

	h_tcp3 = vm_tcp_connect (DOMAIN_3, PORT, VM_TCP_APN_CMNET, tcp_cb_3);
	if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp3 || VM_TCP_ERR_CREATE_FAILED == h_tcp3 )
	{
		if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp3 )
			Tcp_Output("TCP链接2建立失败！没有足够的资源！", -1, TRUE);
		else
			Tcp_Output("TCP链接2建立失败！创建套接字失败！", -1, TRUE);
		
		return h_tcp3;
	}

	Tcp_Output("TCP链接3建立成功！", -1, FALSE);

	return 0;
}

void StopTcpDemo(void)
{
	/*关闭所有链接*/
	vm_tcp_close(h_tcp1);
	vm_tcp_close(h_tcp2);
	vm_tcp_close(h_tcp3);
}