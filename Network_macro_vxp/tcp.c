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
 * tcp.c��ʾ���ʹ��VREӦ�ýӿ�������һ��ʹ��tcp�����������ӵ�Ӧ�ó���
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

#define DOMAIN_1	"www.vogins.com"						/*����1*/
#define DOMAIN_2	"www.51vre.com"							/*����2*/
#define DOMAIN_3	"forum.vogins.com"						/*����3*/
#define IP_1		"10.0.0.172"							/*IP��ַ1*/
#define IP_2		"10.0.0.172"							/*IP��ַ2*/
#define IP_3		"61.236.127.192"						/*IP��ַ3*/
#define PORT		80										/*�˿�*/
#define CONTENT_1	"Hi! �ҵ�1���ܿᣬ��Ϊ��������VRE��"	/*����������*/
#define CONTENT_2	"Hi! �ҵ�2���ܿᣬ��Ϊ��������VRE��"	/*����������*/
#define CONTENT_3	"Hi! �ҵ�3���ܿᣬ��Ϊ��������VRE��"	/*����������*/

static VMINT h_tcp1, h_tcp2, h_tcp3;					/*ĿǰVRE���ͬʱ֧��3������*/

/* TCP����1�Ļص� */
static void tcp_cb_1(VMINT handle, VMINT event)
{
	switch(event)
	{
	case VM_TCP_EVT_CONNECTED:
		{
			Tcp_Output("TCP����1�����ӡ�", -1, FALSE);
		}	
	case VM_TCP_EVT_CAN_WRITE: //���Է���������
		{
			int nLen = strlen(CONTENT_1);
			//��������
			Tcp_Output("TCP����1���ڷ������ݣ�", -1, FALSE);
			Tcp_Output(CONTENT_1, -1, FALSE);
			
			if(nLen != vm_tcp_write(h_tcp1, CONTENT_1, nLen))
			{
				Tcp_Output("TCP����1����ʧ�ܣ�", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP����1���ݷ��ͳɹ���", -1, FALSE);
			}
		}
		return;
		
	case VM_TCP_EVT_CAN_READ: //���Զ�������
		{			
			VMCHAR* pTemp = vm_calloc(10240);
			VMINT len = 0;
			if(!pTemp)
			{
				Tcp_Output("TCP����1�ڴ����ʧ�ܣ�", -1, TRUE);
				return;
			}

			//��ȡ����
			len = vm_tcp_read(h_tcp1, pTemp, 10240);
			if(0 > len)
			{
				vm_tcp_close(handle);
				Tcp_Output("TCP����1����ʧ�ܣ�", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP����1�������ݳɹ�������Ϊ��", -1, FALSE);
				Tcp_Output(pTemp, len, TRUE);
			}			
			vm_free(pTemp);
		}
	
		return;
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			Tcp_Output("TCP����1���жϡ�", -1, TRUE);
		}
		return;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			Tcp_Output("TCP����1δ����������DNS����ʧ�ܡ�", -1, TRUE);
		}
		return;
	}

	Tcp_Output("TCP����1�����¼�", -1, TRUE);
}

/* TCP����2�Ļص� */
static void tcp_cb_2(VMINT handle, VMINT event)
{
	switch(event)
	{
	case VM_TCP_EVT_CONNECTED:
		{
			Tcp_Output("TCP����2�����ӡ�", -1, FALSE);
		}		
	case VM_TCP_EVT_CAN_WRITE: //���Է���������
		{
			int nLen = strlen(CONTENT_2);
			//��������
			Tcp_Output("TCP����2���ڷ������ݣ�", -1, FALSE);
			Tcp_Output(CONTENT_2, -1, FALSE);
			
			if(nLen != vm_tcp_write(h_tcp2, CONTENT_2, nLen))
			{
				Tcp_Output("TCP����2����ʧ�ܣ�", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP����2���ݷ��ͳɹ���", -1, FALSE);
			}
		}
		return;
	 
	case VM_TCP_EVT_CAN_READ: //���Զ�������
		{
			VMCHAR* pTemp = vm_calloc(10240);
			VMINT len = 0;

			if( !pTemp )
			{
				Tcp_Output("TCP����2�ڴ����ʧ�ܣ�", -1, TRUE);

				return;
			}

			//��ȡ����
			len = vm_tcp_read(h_tcp2, pTemp, 10240);
			if( 0 > len )
			{
				vm_tcp_close(handle);
				Tcp_Output("TCP����2����ʧ�ܣ�", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP����2�������ݳɹ�������Ϊ��", -1, FALSE);
				Tcp_Output(pTemp, len, TRUE);
			}
			
			vm_free(pTemp);
		}
		return;
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			Tcp_Output("TCP����2���жϡ�", -1, TRUE);
		}
		return;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			Tcp_Output("TCP����2δ����������DNS����ʧ�ܡ�", -1, TRUE);
		}
		return;	
	}

	Tcp_Output("TCP����2�����¼�", -1, TRUE);
}

/* TCP����3�Ļص� */
static void tcp_cb_3(VMINT handle, VMINT event)
{
	switch(event)
	{
	case VM_TCP_EVT_CONNECTED:
		{
			Tcp_Output("TCP����3�����ӡ�", -1, FALSE);
		}	
	case VM_TCP_EVT_CAN_WRITE: //���Է���������
		{
			int nLen = strlen(CONTENT_3);
			//��������
			Tcp_Output("TCP����3���ڷ������ݣ�", -1, FALSE);
			Tcp_Output(CONTENT_3, -1, FALSE);

			if( nLen != vm_tcp_write(h_tcp3, CONTENT_3, nLen) )
			{
				Tcp_Output("TCP����3����ʧ�ܣ�", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP����3���ݷ��ͳɹ���", -1, FALSE);
			}
		}
		return;
	case VM_TCP_EVT_CAN_READ: //���Զ�������
		{
			VMCHAR* pTemp = vm_calloc(10240);
			VMINT len = 0;

			if( !pTemp )
			{
				Tcp_Output("TCP����3�ڴ����ʧ�ܣ�", -1, TRUE);

				return;
			}

			//��ȡ����
			len = vm_tcp_read(h_tcp3, pTemp, 10240);
			if( 0 > len )
			{
				vm_tcp_close(handle);
				Tcp_Output("TCP����3����ʧ�ܣ�", -1, TRUE);
			}
			else
			{
				Tcp_Output("TCP����3�������ݳɹ�������Ϊ��", -1, FALSE);
				Tcp_Output(pTemp, len, TRUE);
			}
			
			vm_free(pTemp);
		}	
		return;
	case VM_TCP_EVT_PIPE_BROKEN:
		{
			Tcp_Output("TCP����3���жϡ�", -1, TRUE);
		}
		return;
	case VM_TCP_EVT_HOST_NOT_FOUND:
		{
			Tcp_Output("TCP����3δ����������DNS����ʧ�ܡ�", -1, TRUE);
		}
		return;		
	}

	Tcp_Output("TCP����3�����¼�", -1, TRUE);
}

VMINT StartTcpDemo(void)
{
	//��ʼ����3������
	h_tcp1 = vm_tcp_connect (IP_1, PORT, VM_TCP_APN_CMWAP, tcp_cb_1);
	if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp1 || VM_TCP_ERR_CREATE_FAILED == h_tcp1 )
	{
		if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp1 )
			Tcp_Output("TCP����1����ʧ�ܣ�û���㹻����Դ��", -1, TRUE);
		else
			Tcp_Output("TCP����1����ʧ�ܣ������׽���ʧ�ܣ�", -1, TRUE);
		
		return h_tcp1;
	}

	Tcp_Output("TCP����1�����ɹ���", -1, FALSE);
	
	h_tcp2 = vm_tcp_connect (IP_2, PORT, VM_TCP_APN_CMWAP, tcp_cb_2);
	if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp2 || VM_TCP_ERR_CREATE_FAILED == h_tcp2 )
	{
		if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp2 )
			Tcp_Output("TCP����2����ʧ�ܣ�û���㹻����Դ��", -1, TRUE);
		else
			Tcp_Output("TCP����2����ʧ�ܣ������׽���ʧ�ܣ�", -1, TRUE);
		
		return h_tcp2;
	}

	Tcp_Output("TCP����2�����ɹ���", -1, FALSE);

	h_tcp3 = vm_tcp_connect (DOMAIN_3, PORT, VM_TCP_APN_CMNET, tcp_cb_3);
	if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp3 || VM_TCP_ERR_CREATE_FAILED == h_tcp3 )
	{
		if( VM_TCP_ERR_NO_ENOUGH_RES == h_tcp3 )
			Tcp_Output("TCP����2����ʧ�ܣ�û���㹻����Դ��", -1, TRUE);
		else
			Tcp_Output("TCP����2����ʧ�ܣ������׽���ʧ�ܣ�", -1, TRUE);
		
		return h_tcp3;
	}

	Tcp_Output("TCP����3�����ɹ���", -1, FALSE);

	return 0;
}

void StopTcpDemo(void)
{
	/*�ر���������*/
	vm_tcp_close(h_tcp1);
	vm_tcp_close(h_tcp2);
	vm_tcp_close(h_tcp3);
}