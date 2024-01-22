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
 * tcp.c Demonstrates how to use the VRE interface to develop a tcp application
 *
 * Copyright (c) 2009 Vogins Network Technology (China). All rights reserved.
 */

#include "Network.h"
#include "vmsock.h"
#include "vmstdlib.h"
#include <stdio.h>
#include <string.h>

#define DOMAIN_1	"www.vogins.com"						
#define DOMAIN_2	"www.51vre.com"							
#define DOMAIN_3	"forum.vogins.com"						
#define IP_1		"10.0.0.172"							
#define IP_2		"10.0.0.172"							
#define IP_3		"61.236.127.192"						
#define PORT		80										 
#define CONTENT_1	"Hi! I'm First"							
#define CONTENT_2	"Hi! I'm Second"						
#define CONTENT_3	"Hi! I'm Third"							

static VMINT h_tcp1, h_tcp2, h_tcp3;	/* VRE currently supports a maximum of three concurrent connections */


 /*TCP link1 callback */
static void tcp_cb_1(VMINT handle, VMINT event)
{
	switch(event)
	{
	/* This message means the TCP connection is established */
	case VM_TCP_EVT_CONNECTED:
		{
			Tcp_Output("TCP link1 connected...", -1, FALSE);
		}
	/* This message means you can send data to server */
	case VM_TCP_EVT_CAN_WRITE: 
		{  
			int nLen = strlen(CONTENT_1);

			Tcp_Output("TCP link1: sending...", -1, FALSE);
			Tcp_Output(CONTENT_1, -1, FALSE);
			
			if(nLen != vm_tcp_write(h_tcp1, CONTENT_1, nLen))
			{
				Tcp_Output("TCP link1: send failed", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP link1: send success", -1, FALSE);
			}
		}
		return;
	/* This message means you can receive data from server */
	case VM_TCP_EVT_CAN_READ:
		{			
			VMCHAR* pTemp = vm_calloc(1024);
			VMINT len = 0;
			if(!pTemp)
			{
				Tcp_Output("TCP link1: Memory allocation failed", -1, TRUE);
				return;
			}

			len = vm_tcp_read(h_tcp1, pTemp, 1024);
			if(0 > len)
			{
				vm_tcp_close(handle);
				Tcp_Output("TCP link1: receive data failed", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP link1: receive data success!", -1, FALSE);
				Tcp_Output(pTemp, len, TRUE);
			}			
			vm_free(pTemp);
		}	
		return;
	/* This message means cannot establish TCP connection to the server */
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			Tcp_Output("TCP link1: failed to connect", -1, TRUE);
		}
		return;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			Tcp_Output("TCP link1: DNS resolution failure", -1, TRUE);
		}
		return;
	/* This message means the TCP connection is closed by server */
	case VM_TCP_EVT_PIPE_CLOSED:
		{
			Tcp_Output("TCP link1: link closed by server", -1, TRUE);
		}
		return;
	}

	Tcp_Output("TCP link1: other event", -1, TRUE);// TCP Line1 other event
}

VMINT StartTcpDemo(void)
{
	h_tcp1 = vm_tcp_connect (DOMAIN_1, PORT, VM_TCP_APN_CMNET, tcp_cb_1);
	if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp1 || VM_TCP_ERR_CREATE_FAILED == h_tcp1 )
	{
		if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp1 )
			Tcp_Output("TCP link1 failed to connect, because there is no enough resource!", -1, TRUE);
		else
			Tcp_Output("TCP link1 failed to connect, cannot create socket.", -1, TRUE);		
		return h_tcp1;
	}

	Tcp_Output("TCP link1 connecting...", -1, FALSE);
	
	return 0;
}

void StopTcpDemo(void)
{
  /* close all connection */
	vm_tcp_close(h_tcp1);
}