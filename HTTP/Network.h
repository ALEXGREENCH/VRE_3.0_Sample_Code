#ifndef _VRE_APP_Network_
#define	_VRE_APP_Network_

#include "vmsys.h"

#define APP_NAME			"Network Demo"

#define	SCREEN_WIDTH		vm_graphic_get_screen_width()
#define	SCREEN_HEIGHT		vm_graphic_get_screen_height()
#define CHARACTER_HEIGHT	vm_graphic_get_character_height()
#define SOFTKEY_HEIGHT		(CHARACTER_HEIGHT + 6)

#define TIMEOUT_TIME		50
#define LOG_LEVEL			VM_DEBUG_LEVEL

/**
*  Start http download
*/
void StartHttpDownload(void);

/**
*     Stop http download
*/
void StopHttpDownload(VMINT handle);

/**
* Cancel all http connection
*/
void DropAllHttpConnection(void);

/**
*   Get http connection handle
*/
VMINT getHttpHandle(void);

void Http_Output(VMCHAR* state_text, VMINT clear);

VMINT Http_ReadFile(VMWSTR file_name, VMINT offset, VMINT num, void* data);

/**
*  Start TCP connect
*/
VMINT StartTcpDemo(void);

/**
* Stop TCP connect
*/
void StopTcpDemo(void);

void Tcp_Output(VMCHAR* state_text, VMINT text_len, VMINT clear);

#endif

