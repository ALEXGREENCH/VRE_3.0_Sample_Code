#ifndef _VRE_APP_Network_
#define	_VRE_APP_Network_

#include "vmsys.h"

#define _VRE_MIBR_	/*应用支持后台运行*/

#define APP_NAME			"Network Demo"

#define	SCREEN_WIDTH		vm_graphic_get_screen_width()
#define	SCREEN_HEIGHT		vm_graphic_get_screen_height()
#define CHARACTER_HEIGHT	vm_graphic_get_character_height()
#define SOFTKEY_HEIGHT		(CHARACTER_HEIGHT + 6)

#define TIMEOUT_TIME		50
#define LOG_LEVEL			VM_DEBUG_LEVEL

void ShowAllHTTPConnection(void);

/**
* 开始HTTP下载
*/
void StartHttpDownload(void);

/**
* 停止HTTP下载
*/
void StopHttpDownload(VMINT handle);

/**
* 取消所有HTTP连接
*/
void DropAllHttpConnection(void);

/**
* 获取HTTP连接状态
*/
VMINT getHttpHandle(void);

void Http_Output(VMCHAR* state_text, VMINT clear);

VMINT Http_ReadFile(VMWSTR file_name, VMINT offset, VMINT num, void* data);

/**
* 开始TCP连接
*/
VMINT StartTcpDemo(void);

/**
* 停止TCP连接
*/
void StopTcpDemo(void);

void Tcp_Output(VMCHAR* state_text, VMINT text_len, VMINT clear);








int get_index(int, int);

#endif

