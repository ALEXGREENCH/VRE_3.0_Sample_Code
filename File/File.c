/* ============================================================================
 * VRE GRAPHIC Library demonstration application.
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

/* ============================================================================
 * 本示例程序包含文件系统中创建、删除、读取、写入、重命名文件，以及创建删除目录操作。
 * This sample application demonstrates how to create, delete, read, write and rename files
 *
 * Copyright (c) 2009 Vogins Network Technology (China). All rights reserved.
 * By Steven Fang, Mar 2009 V1.0
 * ========================================================================= */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "vmsys.h"
#include "vmio.h"
#include "vmchset.h"
#include "vmgraph.h"
#include "vmlog.h"
#include "vmstdlib.h"
#include "File.h"
#include "Fileb.h"


VMINT layer_hdl[2] = {0};

struct app_t gApp;

static VMCHAR g_pszText[255];

static VMCHAR g_szNameFile[50];
static VMCHAR g_szNameFileNew[50];
static VMCHAR g_szNameDir[50];


#define LEN_CACHE_TEXT		256

#define NAME_FILE			"%c:\\vre\\abc.txt"
#define NAME_FILE_NEW		"%c:\\vre\\def.txt"
//#define NAME_FILE_NEW		"%c:\\erv\\def.txt"
#define NAME_DIR			"%c:\\vre\\test_dir"
// int a[10], b[10];
// struct c{
// 	int* d;
// 	int* e;
// };
// struct c f = {a, b};
// void func1(void) {}
// typedef struct struct1 {
// 	int a;
// 	void (*fun)(void);
// } struct1;
// struct1 array1[1] = {
// 	0, func1
// };
// char *str = "test";
// char * list[] = {"zero", "one", "two"};

/*
* System event
*/
static void		sys_evt_process(VMINT message, VMINT param);

/*
* Key event 
*/
static void		sys_key_process(VMINT message, VMINT param);

/*
* Pen event  
*/
static void		sys_pen_process(VMINT event, VMINT x, VMINT y);

/*
* Create abc.txt 
*/
static void menu_createFile(void);

/*
* Delete abc.txt
*/
static void menu_deleteFile(void);

/*
* Read from abc.txt
*/
static void menu_readFile(void);

/*
* Write "Hello world!" to abc.txt. 
*/
static void menu_appendFile(void);

/*
* Rename abc.txt to def.txt   
*/
static void menu_renameFile(void);

/*
* Create folder
*/
static void menu_createDir(void);

/*
* Delete folder
*/
static void menu_delDir(void);

/*
* List all the vxp files under current directory
*/
static void menu_listFile(app_state_t i);

void draw_app(void);
void draw_memu(void);
void init_app(void);
static void menu_key_proc(VMINT message, VMINT param);
static void notify_key_proc(VMINT message, VMINT param);
void show_message(void);
void find_file(void);

/*
*	Program Entry 
*/
void vm_main(void) 
{	
	VMINT	drv;
	VMUINT  sys_driver_size;
	VMCHAR  driver_str[2] = {0};
	VMINT8	logfile[50];	
	VMCHAR	path[50];
	VMWCHAR w_path[50];
	
	/* Get Letter */
	drv= vm_get_removable_driver();		//Get mobile drive letter
	if(drv<0)							//Mobile drive does not exist
		drv=vm_get_system_driver();		//Get system drive letter
		
	sprintf(path, "%c:\\vre", drv);
	vm_ascii_to_ucs2(w_path, 100, path);
	vm_file_mkdir(w_path);

	sprintf(path, "%c:\\vre\\Save", drv);
	vm_ascii_to_ucs2(w_path, 100, path);
	vm_file_mkdir(w_path);
	
	sprintf(path, "%c:\\vre\\Save\\%s", drv, APP_SERIAL_NUM);
	vm_ascii_to_ucs2(w_path, 100, path);
	vm_file_mkdir(w_path);
	
	/* Initialize log system */
	sprintf(logfile, "%c:\\File_demo.log", drv);
	vm_log_init(logfile, VM_DEBUG_LEVEL);
	vm_log_debug("File Demo Start!");
	
	/* Get the free space of the disk */ 
	sprintf(driver_str, "%c", drv);
	vm_ascii_to_ucs2(w_path, 100, driver_str);
	sys_driver_size = vm_get_disk_free_space(w_path);
	vm_log_debug("free space:%d", sys_driver_size);

	/* Register event handler */
	vm_reg_sysevt_callback(sys_evt_process);
	vm_reg_keyboard_callback(sys_key_process);
	vm_reg_pen_callback(sys_pen_process);
}


static void sys_evt_process(VMINT message, VMINT param)
{	
	switch (message)
	{
	/* when the program starts, the system will send this message */
	case VM_MSG_CREATE:
		/* Create base layer */
		if((layer_hdl[0] = vm_graphic_create_layer(0, 0, 
			vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1)) != 0)
		{
			vm_log_fatal("create base layer failure, break");
			vm_exit_app();
			break;
		}
		/* Get base layer buffer */
		if((gApp.dst_buf = vm_graphic_get_layer_buffer(layer_hdl[0])) == NULL)
		{
			vm_graphic_delete_layer(layer_hdl[0]);
			vm_log_fatal("get base layer buffer failure, break");
			vm_exit_app();
		}
		vm_graphic_set_clip(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height());

		/* Initialize global data */
		init_app();
		break;
	/* this message always comes after VM_MSG_CREATE or VM_MSG_PAINT message */
	case VM_MSG_PAINT:
		find_file();
// 		draw_app();
		break;	
	/* if there is an external event (incoming calls or SMS) occurs, the system will send this message */
	case VM_MSG_INACTIVE:
		vm_graphic_delete_layer(layer_hdl[0]);
		break;
	/*  after the processing of external events completed, the system will send this message */
	case VM_MSG_ACTIVE:
		/* Create base layer */
		if((layer_hdl[0] = vm_graphic_create_layer(0, 0, 
			vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1)) != 0)
		{
			vm_log_fatal("create base layer failure, break");
			vm_exit_app();
			break;
		}
		/* Get base layer buffer */
		if((gApp.dst_buf = vm_graphic_get_layer_buffer(layer_hdl[0])) == NULL)
		{
			vm_graphic_delete_layer(layer_hdl[0]);
			vm_log_fatal("get base layer buffer failure, break");
			vm_exit_app();
		}
		break;
	/* if you press hang-up button, the system will send this message */
	case VM_MSG_QUIT:
		vm_graphic_delete_layer(layer_hdl[0]);
		vm_exit_app();				
		break;
	}
}

static void sys_key_process(VMINT message, VMINT param)
{	
	switch(gApp.app_state)
	{
	case STATE_MENU:
		menu_key_proc(message, param);
		break;
	case STATE_NOTIFY:
		notify_key_proc(message,param);
		break;
	default:
	    break;
	}
}

static void sys_pen_process(VMINT event, VMINT x, VMINT y)
{
	//to do
}

static void notify_key_proc(VMINT message, VMINT param) {
	if (message == VM_KEY_EVENT_UP)
	{
		switch(param)
		{
		/* if press the right soft key, return to the main menu */
		case VM_KEY_RIGHT_SOFTKEY:
			gApp.app_state = STATE_MENU;
			draw_app();
			break;
		}
	}
}

static void menu_key_proc(VMINT message, VMINT param) {
	if (message == VM_KEY_EVENT_UP)
	{
		switch(param)
		{
		/* if press the right soft key, exit the application */
		case VM_KEY_RIGHT_SOFTKEY:
			vm_graphic_delete_layer(layer_hdl[0]);
			vm_exit_app();				
			break;
		/* if press the number keys 1, create a file named "abc.txt" */
		case VM_KEY_NUM1:
			menu_createFile();
			break;
		/* if press the number keys 2, delete the file named "abc.txt" */
		case VM_KEY_NUM2:
			menu_deleteFile();
			break;
		/* if press the number keys 3, read from the file named "abc.txt" */
		case VM_KEY_NUM3:
			menu_readFile();
			break;
		/* if press the number keys 4, read from the file named "abc.txt" */
		case VM_KEY_NUM4:
			menu_appendFile();
			break;
		/* if press the number keys 5, rename the name of file "abc.txt" to "def.txt" */
		case VM_KEY_NUM5:
			menu_renameFile();
			break;
		/* if press the number keys 6, create the folder named "test_dir" */
		case VM_KEY_NUM6:
			menu_createDir();
			break;
		/* if press the number keys 7, delete the folder named "test_dir" */
		case VM_KEY_NUM7:
			menu_delDir();
			break;
		/* if press the number keys 8, list all vxp files under current folder */
		case VM_KEY_NUM8:
			menu_listFile(1);
			break;
		}
	}
}

void draw_memu(void){
	VMCHAR	str[255];
	VMWCHAR	wstr[255];

	sprintf(str, "%s", "File Demo");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 60, 10, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "NUM 1: Create abc.txt");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 30, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	sprintf(str, "%s", "NUM 2: Delete abc.txt");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 50, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "NUM 3: Read from abc.txt");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 70, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	sprintf(str, "%s", "NUM 4: Write to abc.txt");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 90, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "NUM 5: Rename file abc.txt");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 110, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	sprintf(str, "%s", "NUM 6: Create folder test_dir");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 130, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "NUM 7: Remove folder test_dir");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 150, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
	
	sprintf(str, "%s", "NUM 8: List all vxp files");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 170, wstr, vm_wstrlen(wstr), VM_COLOR_RED);

	sprintf(str, "%s", "RIGHT SOFTKEY: EXIT");				
	vm_ascii_to_ucs2(wstr, 255, str);
	vm_graphic_textout(gApp.dst_buf, 20, 190, wstr, vm_wstrlen(wstr), VM_COLOR_RED);
}


void draw_app(void)
{
	vm_graphic_fill_rect(gApp.dst_buf, 0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), VM_COLOR_BLACK, VM_COLOR_BLACK);
	switch(gApp.app_state)
	{
	case STATE_MENU:
		draw_memu();
		break;
	case STATE_NOTIFY:
		show_message();
	default:
	    break;
	}
	vm_graphic_flush_layer(layer_hdl, 1);
}

void init_app(void){
	VMINT	drv;   

	gApp.dst_buf = vm_graphic_get_layer_buffer(layer_hdl[0]);	
	gApp.app_state = STATE_MENU;

	drv= vm_get_removable_driver();	
	if(drv<0)						
		drv=vm_get_system_driver();	

	sprintf(g_szNameFile, NAME_FILE, drv);
	sprintf(g_szNameFileNew, NAME_FILE_NEW, drv);
	sprintf(g_szNameDir, NAME_DIR, drv);
}

static void menu_createFile(void)
{
	VMWCHAR g_wszNameFile[50];
	VMFILE file;
	vm_ascii_to_ucs2(g_wszNameFile, 100, g_szNameFile);
	file = vm_file_open(g_wszNameFile, MODE_CREATE_ALWAYS_WRITE, FALSE);
	memset(g_pszText, 0, 255);
	if( 0 > file )
		strcpy(g_pszText, "Create file failure");
	else
		strcpy(g_pszText, "Create file success");

	vm_file_close(file);

	gApp.app_state = STATE_NOTIFY;
	draw_app();

}

static void menu_deleteFile(void)
{
	VMWCHAR g_wszNameFile[50];
	VMINT nResult;
	vm_ascii_to_ucs2(g_wszNameFile, 100, g_szNameFile);
	nResult = vm_file_delete(g_wszNameFile); // if file is opened, this operation will be fail
	memset(g_pszText, 0, 255);
	if( nResult )
		strcpy(g_pszText, "Delete file failure");
	else
		strcpy(g_pszText, "Delete file success"); 
	
	gApp.app_state = STATE_NOTIFY;
	draw_app();
}

static void menu_readFile(void)
{
	VMUINT nSize = 0, fSize = 0;
	VMWCHAR g_wszNameFile[50];
	VMFILE file;
	vm_ascii_to_ucs2(g_wszNameFile, 100, g_szNameFile);
	file = vm_file_open(g_wszNameFile, MODE_READ, FALSE);
	memset(g_pszText, 0, 255);
	if( 0 > file )
	{
		strcpy(g_pszText, "Open file failure");
		gApp.app_state = STATE_NOTIFY;
		draw_app();
		return;
	}
	
	strcpy(g_pszText, "Open file success\n"); 

	if( vm_file_getfilesize(file, &fSize) )
	{
		strcat(g_pszText, " Get file size failure\n");
		vm_file_close(file);
		gApp.app_state = STATE_NOTIFY;
		draw_app();
		return;
	}
	
	strcat(g_pszText, "size:");
	sprintf(g_pszText + strlen(g_pszText), "%d", fSize);
	strcat(g_pszText, "Bytes\n");
	strcat(g_pszText, "contents:\n");	
	nSize = vm_file_read(file, g_pszText + strlen(g_pszText),
						(fSize <= (LEN_CACHE_TEXT - strlen(g_pszText)) ? fSize : (LEN_CACHE_TEXT - strlen(g_pszText))), &nSize);
	vm_file_close(file);
	gApp.app_state = STATE_NOTIFY;
	draw_app();
}

static void menu_appendFile(void)
{
	VMUINT nSize = 0;
	VMWCHAR g_wszNameFile[50];
	VMFILE file;
	vm_ascii_to_ucs2(g_wszNameFile, 100, g_szNameFile);
	file = vm_file_open(g_wszNameFile, MODE_WRITE, FALSE);
	memset(g_pszText, 0, 255);
	if( 0 > file )
	{
		strcpy(g_pszText, "Open file failure");
		gApp.app_state = STATE_NOTIFY;
		draw_app();
		return;
	}
	
	strcpy(g_pszText, "Open file Success\n");
	
	if( vm_file_getfilesize(file, &nSize) )
	{
		strcat(g_pszText, "get file size failure\n");
		vm_file_close(file);
		gApp.app_state = STATE_NOTIFY;
		draw_app();
		return;
	}
	
	strcat(g_pszText, "Original size:"); 
	sprintf(g_pszText+strlen(g_pszText), "%d", nSize);
	strcat(g_pszText, "Bytes\n");

	vm_file_seek(file, 0, BASE_END); // move the read-write pointer to the end of file
	nSize = vm_file_write(file, "Hello world!", strlen("Hello world!"), &nSize);

	if( !nSize )
		strcat(g_pszText, "write file failure\n");
	else
	{
		strcat(g_pszText, "write file success\n");

		vm_file_getfilesize(file, &nSize);
		strcat(g_pszText, "Current size:"); 
		sprintf(g_pszText+strlen(g_pszText), "%d", nSize);
		strcat(g_pszText, "Bytes\n");
	}
	vm_file_close(file);

	gApp.app_state = STATE_NOTIFY;
	draw_app();
}

static void menu_renameFile(void)
{
	VMWCHAR szName[50];
	VMWCHAR szNewName[50];
	VMINT nResult = 0;

	vm_ascii_to_ucs2(szName, 100, g_szNameFile);
	vm_ascii_to_ucs2(szNewName, 100, g_szNameFileNew);
	nResult = vm_file_rename(szName, szNewName);
	if( nResult != 0 )
		strcpy(g_pszText, "Rename file failure");
	else
		strcpy(g_pszText, "Rename file success");

	gApp.app_state = STATE_NOTIFY;
	draw_app();
}

static void menu_createDir(void)
{
	VMWCHAR g_wszNameDir[50];
	
	vm_ascii_to_ucs2(g_wszNameDir, 100, g_szNameDir);
	memset(g_pszText, 0, 255);
	if(vm_file_mkdir(g_wszNameDir))
		strcpy(g_pszText, "create folder failure");
	else
		strcpy(g_pszText, "create folder success");

	gApp.app_state = STATE_NOTIFY;
	draw_app();
}

static void menu_delDir(void)
{
	VMWCHAR g_wszNameDir[50];

	vm_ascii_to_ucs2(g_wszNameDir, 100, g_szNameDir);
	memset(g_pszText, 0, 255);
	if(vm_file_rmdir(g_wszNameDir))	//if there are some files under the directory, this operation will be fail
		strcpy(g_pszText, "Remove folder failure");
	else
		strcpy(g_pszText, "Remove folder success");
	
	gApp.app_state = STATE_NOTIFY;
	draw_app();
}

static void menu_listFile(app_state_t i)
{
	struct vm_fileinfo_ext info;
	VMCHAR szPath[50] = "";
	VMWCHAR w_szPath[50];
	VMINT handle = -1;
	VMINT	drv;
	VMINT j = i;

	memset(g_pszText, 0, 255);
	
	drv= vm_get_removable_driver();	 
	if(drv<0)						
		drv=vm_get_system_driver();	
	sprintf(szPath, "%c:\\vre\\*.vxp", drv);

	vm_ascii_to_ucs2(w_szPath, 100, szPath);

	handle = vm_find_first_ext(w_szPath, &info);

	if(0 > handle)
		strcpy(g_pszText, "No matching files"); 
	else
	{
		VMCHAR szName[255] = "";
		vm_ucs2_to_ascii(szName, 255, info.filefullname);
		strcpy(g_pszText, "File list:\n");
		strcat(g_pszText, szName);

		sprintf(g_pszText+strlen(g_pszText), " size:%dBytes\n", info.filesize);

		while(!vm_find_next_ext(handle, &info))
		{
			vm_ucs2_to_ascii(szName, 255, info.filefullname);
			strcat(g_pszText, szName);
			sprintf(g_pszText+strlen(g_pszText), " size:%dBytes\n", info.filesize);
		}

		vm_find_close_ext(handle);	// when search is complete, you must close search handle 
	}	
	gApp.app_state = STATE_NOTIFY;
	draw_app();
}

void show_message(void)
{
	VMWCHAR* pText = vm_malloc(LEN_CACHE_TEXT*sizeof(VMWCHAR));
	VMWCHAR *ptempText = pText;
	int i = 0;
	VMWCHAR szTemp[256];
	VMWCHAR szLine[2];
	VMINT nStartY = 10;
	VMCHAR szBack[10];
	VMINT length;
	VMWCHAR w_szBack[10];

	memset(szTemp, 0, sizeof(VMWCHAR)* 256);
	memset(pText, 0 , sizeof(VMWCHAR) * LEN_CACHE_TEXT);	
	
 	vm_ascii_to_ucs2(pText, LEN_CACHE_TEXT, g_pszText);

	while(*pText)
	{
		szTemp[i++] = *pText;
		szLine[0] = *pText;
		szLine[1] = 0;
		if( !wstrcmp(szLine, (VMWSTR)(L"\n")) || vm_graphic_get_string_width(szTemp) > vm_graphic_get_screen_width() )
		{			
			if( !wstrcmp(szLine, (VMWSTR)(L"\n")) )
				pText++;
			
			szTemp[i-1] = 0;
			vm_graphic_textout(gApp.dst_buf, 5, nStartY, szTemp, wstrlen(szTemp), VM_COLOR_RED);			
			
			nStartY += vm_graphic_get_character_height() + 2;
			if( nStartY + vm_graphic_get_character_height() > vm_graphic_get_screen_height()-30 )
			{
				vm_graphic_textout(gApp.dst_buf, 5, nStartY, (VMWSTR)(L"..."), 3, VM_COLOR_RED);
				memset(szTemp, 0, sizeof(VMWCHAR)*256);
				break;
			}
			
			i = 0;
			memset(szTemp, 0, sizeof(VMWCHAR)*256);
			continue;
		}		
		pText++;
	}
	sprintf(szBack, "return");
	vm_ascii_to_ucs2(w_szBack, 20, szBack);
	length = vm_graphic_get_string_width(w_szBack);
	vm_graphic_textout(gApp.dst_buf, vm_graphic_get_screen_width() - length,
						vm_graphic_get_screen_height() - vm_graphic_get_character_height(),
						w_szBack, length, VM_COLOR_RED);
	
	if(wstrlen(szTemp))
	{
		vm_graphic_textout(gApp.dst_buf, 5, nStartY, szTemp, wstrlen(szTemp), VM_COLOR_RED);
	}
	vm_free(ptempText);
}

void find_file(void)
{
	struct vm_fileinfo_ext info;
	VMCHAR szPath[50] = "";
	VMWCHAR w_szPath[50];
	VMINT handle = -1;
	VMINT	drv;
	VMINT i = 0;
	VMINT cvs_hdl;
	VMBYTE * cvs_buff;

	cvs_hdl = vm_graphic_create_canvas(vm_graphic_get_screen_width(),vm_graphic_get_screen_height());
	cvs_buff = vm_graphic_get_canvas_buffer(cvs_hdl);
	

	vm_graphic_fill_rect(cvs_buff, 0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), VM_COLOR_BLACK, VM_COLOR_BLACK);
	memset(g_pszText, 0, 255);
	
	drv= vm_get_removable_driver();	 
	if(drv<0)						
		drv=vm_get_system_driver();	
	sprintf(szPath, "%c:\\*.txt", drv);
	
	vm_ascii_to_ucs2(w_szPath, 100, szPath);
// 	vm_chset_convert(VM_CHSET_GB2312,VM_CHSET_UCS2,szPath,w_szPath,100);

// 	handle = vm_file_open(w_szPath,MODE_READ,TRUE);
// // 	vm_file_seek(handle,0,BASE_BEGIN);
// 	i = vm_file_tell(handle);
// 	vm_file_close(handle);

	handle = vm_find_first_ext(w_szPath, &info);
	
	if(0 > handle)
		strcpy(g_pszText, "No matching files"); 
	else
	{
		VMCHAR szName[255] = "";

		vm_graphic_textout(cvs_buff,0,i,info.filefullname,wstrlen(info.filefullname),VM_COLOR_RED);
		
		i += 20;
		while(!vm_find_next_ext(handle, &info))
		{
			vm_graphic_textout(cvs_buff,0,i,info.filefullname,wstrlen(info.filefullname),VM_COLOR_RED);
			
			i += 20;
		}
		
		vm_find_close_ext(handle);	// when search is complete, you must close search handle 
	}	
	gApp.app_state = STATE_NOTIFY;
	vm_graphic_blt(gApp.dst_buf,0,0,cvs_buff,0,0,vm_graphic_get_screen_width(),vm_graphic_get_screen_height(),1);
	vm_graphic_flush_layer(layer_hdl, 1);

}