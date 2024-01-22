/* ============================================================================
 *  VRE demonstration application.
 *  module name: XML
 *  source name: xml_demo.c
 *  function: parse XML data
 *  author: heping.miao
 *  version: 1.0
 *  date: 2010-4-18
 * ========================================================================= */

/* ---------------------------------------------------------------------------
 *                              standard VRE header file
 * ------------------------------------------------------------------------ */

#include "vmio.h"
#include "vmsock.h"
#include "vmgraph.h"
#include "vmgfxold.h"
#include "vmchset.h"
#include "vmxml.h"
#include "vmlog.h"
#include "vmstdlib.h"


/* ---------------------------------------------------------------------------
 *                              non-standard VRE header file
 * ------------------------------------------------------------------------ */

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "xml_demo.h"


#define SCREEN_WIDTH vm_graphic_get_screen_width() 
#define SCREEN_HEIGHT vm_graphic_get_screen_height() 
#define FONT_HEIGHT (vm_graphic_get_character_height() + 5)
#define HAS_BREAK(str)		('|'==str[strlen(str)-1])
#define XML_BUF_LEN (512)
/* XML Data */
const static VMCHAR xml_parser_buffer[] = {"<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE note SYSTEM \"Note.dtd\"><adobehelp type=\"product\"><title>Adobe Reader 8</title><files type=\"nav\"><file type=\"contents\">nav_toc.xml</file><file type=\"search\">final_search_entries.xml</file></files></adobehelp>"};

VMCHAR xml_buf[XML_BUF_LEN];
VMINT layer[1];
VMUINT8 *layer_buffer;
VMINT ProgramState;
VMINT LETTER_WIDTH;
VMINT FONT_WIDTH;
VMINT timerid;

void init_vre(void);//initialization 
void key_event(VMINT event,VMINT keycode);//key events
void sys_event(VMINT handel,VMINT event);//system events

void timer_proc(VMINT tid);
void Font_Size(void);
void exit_demo(void);

void key_logo(VMINT event,VMINT keycode);//key events on logo screen
void key_xml_menu(VMINT event,VMINT keycode);//key events on XML menu screen
void key_xml_info(VMINT event,VMINT keycode);//key events on XML info screen

void draw_logo(void);//draw logo screen
void draw_xml_menu(void);//draw XML menu screen
void draw_xml_info(VMCHAR * ptr);//draw XML info screen

void xml_parse_buffer(void);//start parse
void xml_data_app_hdlr(void *resv, const VMCHAR *el, const VMCHAR *data, VMINT len, VMINT error);//data handler
void xml_doctype_start_app_hdlr(void *data, const VMCHAR *doctypeName, const VMCHAR **sysid, const VMCHAR **pubid, VMINT internal_subset, VMINT error); //DOCTYPE tag start handler
void xml_doctype_end_app_hdlr(void *data, VMINT error); //DOCTYPE tag end handler
static void xml_elem_start_app_hdlr(void *data, const VMCHAR *el, const VMCHAR **attr, VMINT error); //Element tag start handler
void xml_elem_end_app_hdlr(void *data, const VMCHAR *el, VMINT error); //Element tag end handler

/* entry */
void vm_main(void)
{
    vm_reg_keyboard_callback(key_event);
	vm_reg_sysevt_callback(sys_event);	
}

void init_vre(void)
{     
     ProgramState = LOGO;
	 Font_Size();
	 timerid = vm_create_timer(100,timer_proc);
}

void key_event(VMINT event,VMINT keycode)
{
     switch(ProgramState)
	 {
	 case LOGO:
		 key_logo(event,keycode);
		 break;
     case XML_STATE:
		 key_xml_menu(event,keycode);
		 break;
	 case XML_INFO:
		 key_xml_info(event,keycode);
		 break;
	 default:
		 break;
	 }
}

void sys_event(VMINT event,VMINT param)
{
    switch(event)
	{
	case VM_MSG_INACTIVE:
		break;
    case VM_MSG_ACTIVE:
		break;
	case VM_MSG_CREATE:
		init_vre();
		layer[0] = vm_graphic_create_layer(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,-1);
		vm_graphic_set_clip(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
		layer_buffer = vm_graphic_get_layer_buffer(layer[0]);
		break;
	case VM_MSG_PAINT:
		break;
	case VM_MSG_QUIT:
		exit_demo();
		break;
	default:
		break;
	}
}

void xml_parse_buffer(void) 
{
	VMCHAR 	*ret = "OK";
	VMINT 	xml_ret;	
	VM_XML_PARSER_STRUCT parser;
	memset(xml_buf, 0, XML_BUF_LEN);
	strcat(xml_buf, "xml from buffer:|");
	xml_ret = vm_xml_new_parser(&parser);//initialize parser
	if (VM_XML_RESULT_OK != xml_ret)
		ret = vm_xml_get_error();//get error code
	vm_xml_set_doctype_handlers(&parser, xml_doctype_start_app_hdlr, xml_doctype_end_app_hdlr);//set DOCTYPE tag handler
	vm_xml_set_element_handlers(&parser, xml_elem_start_app_hdlr, xml_elem_end_app_hdlr);//set element tag handler
	vm_xml_set_data_handler(&parser, xml_data_app_hdlr);//set data handler
	
	/* Parse the XML buffer */
	xml_ret = vm_xml_parse(&parser, xml_parser_buffer, strlen(xml_parser_buffer));
	if (VM_XML_RESULT_OK != xml_ret)
		ret = vm_xml_get_error();
}

void xml_data_app_hdlr(void *resv, const VMCHAR *el, const VMCHAR *data, VMINT len, VMINT error)
{
    if (VM_XML_NO_ERROR != error)
    {
		strcat(xml_buf, "<!ERROR: ");
		strcat(xml_buf, vm_xml_get_error());
		strcat(xml_buf, " !>|");
	}

	strcat(xml_buf, data);
}

void xml_doctype_start_app_hdlr(void *data, const VMCHAR *doctypeName, const VMCHAR **sysid, const VMCHAR **pubid, VMINT internal_subset, VMINT error)
{
	VMUINT i = 0;

    if (VM_XML_NO_ERROR != error)
    {
		strcat(xml_buf, "<!ERROR: ");
		strcat(xml_buf, vm_xml_get_error());
		strcat(xml_buf, " !>|");		
	}
	
	strcat(xml_buf, "<!DOCTYPE ");
	strcat(xml_buf, doctypeName);	
	if (sysid != NULL)
		while (sysid[i])
		{
			strcat(xml_buf, " ");
			strcat(xml_buf, sysid[i]);			
			i++;
		}
	i = 0;
	if (pubid != NULL)
		while (pubid[i])
		{
			strcat(xml_buf, " ");
			strcat(xml_buf, pubid[i]);			
			i++;
		}
}

void xml_doctype_end_app_hdlr(void *data, VMINT error)
{
    if (VM_XML_NO_ERROR != error)
    {
		strcat(xml_buf, "<!ERROR: ");
		strcat(xml_buf, vm_xml_get_error());
		strcat(xml_buf, " !>|");		
	}

	strcat(xml_buf, ">|");
}

static void xml_elem_start_app_hdlr(void *data, const VMCHAR *el, const VMCHAR **attr, VMINT error)
{
	VMINT index = 0;

    if (VM_XML_NO_ERROR != error)
    {
		strcat(xml_buf, "<!ERROR: ");
		strcat(xml_buf, vm_xml_get_error());
		strcat(xml_buf, " !>|");	
	}

	strcat(xml_buf, "<"); 
	strcat(xml_buf, el);	
	while ((NULL != attr[index]) && (NULL != attr[index + 1]))
	{
		strcat(xml_buf, " ");
		strcat(xml_buf, attr[index]);
		strcat(xml_buf, "=");
		strcat(xml_buf, attr[index+1]);
		strcat(xml_buf, ";");
	index += 2;

	}
	strcat(xml_buf, ">|"); 
}

void xml_elem_end_app_hdlr(void *data, const VMCHAR *el, VMINT error)
{
    if (VM_XML_NO_ERROR != error)
    {
		strcat(xml_buf, "<!ERROR: ");
		strcat(xml_buf, vm_xml_get_error());
		strcat(xml_buf, " !>|");
	}

	if(!HAS_BREAK(xml_buf))
		strcat(xml_buf, "|");  
	strcat(xml_buf, "</"); 
	strcat(xml_buf, el);
	strcat(xml_buf, ">|"); 
}

void timer_proc(VMINT tid)
{
	switch(ProgramState)
	{
	case LOGO:
		draw_logo();
		break;
	case XML_STATE:
		draw_xml_menu();
		break;
	case XML_INFO:
		draw_xml_info(xml_buf);
		break;
	default:
		break;
	}
}

void Font_Size(void)
{
    VMWCHAR wstr[10];
	VMCHAR str[10];
	sprintf(str,"ร็");
	vm_gb2312_to_ucs2(wstr,10,str);
	FONT_WIDTH = vm_graphic_get_string_width(wstr);
	sprintf(str,"v");
	vm_gb2312_to_ucs2(wstr,10,str);
	LETTER_WIDTH = vm_graphic_get_string_width(wstr);
}

void exit_demo(void)
{
    vm_graphic_delete_layer(layer[0]);
	vm_delete_timer(timerid);
    vm_exit_app();
}

void key_logo(VMINT event,VMINT keycode)
{
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{
             switch(keycode)
			 {
			 case VM_KEY_LEFT_SOFTKEY:
				 ProgramState = XML_STATE;
				 break;
			 case VM_KEY_RIGHT_SOFTKEY:
				 exit_demo();
				 break;
			 default:
				 break;
			 }
		}
		break;
	default:
		break;
	}
}

void key_xml_menu(VMINT event,VMINT keycode)
{	
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{    			 
             switch(keycode)
			 {			 
			 case VM_KEY_RIGHT_SOFTKEY://if press left soft key, return to logo screen
				 ProgramState = LOGO;
				 break;
			 case VM_KEY_NUM1:
				 ProgramState = XML_INFO;
                 xml_parse_buffer();
                 break;
			 default:
				 break;
			 }
		}
		break;
	default:
		break;
	}
}

void key_xml_info(VMINT event,VMINT keycode)
{
    switch(event)
	{
	case VM_KEY_EVENT_DOWN:
		{    			 
             switch(keycode)
			 {			 
			 case VM_KEY_RIGHT_SOFTKEY://if press left soft key, return to xml menu screen
				 ProgramState = XML_STATE;				 	 
				 break;				 
			 default:
				 break;
			 }
		}
		break;
	default:
		break;
	}
}

void draw_logo(void)
{
	VMWCHAR wstr[20];
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_WHITE,VM_COLOR_BLACK);

	vm_ascii_to_ucs2(wstr,40,"Xml_Demo");
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,
		(SCREEN_HEIGHT-vm_graphic_get_character_height())/2,wstr,wstrlen(wstr),VM_COLOR_WHITE);

	vm_ascii_to_ucs2(wstr,40,"Start");
	vm_graphic_textout(layer_buffer,2,SCREEN_HEIGHT-FONT_HEIGHT - 2,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_ascii_to_ucs2(wstr,40,"Exit");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-FONT_WIDTH*2 -2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_graphic_flush_layer(layer,1);
}

void draw_xml_menu(void)
{
	VMWCHAR wstr[20];
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_GREEN,VM_COLOR_BLACK);
	vm_ascii_to_ucs2(wstr,40,"Xml_Menu");
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,13,wstr,wstrlen(wstr),VM_COLOR_888_TO_565(129,220,10));
    vm_ascii_to_ucs2(wstr,40,"Num 1 Start parse");
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,SCREEN_HEIGHT/2-FONT_HEIGHT,wstr,wstrlen(wstr),VM_COLOR_888_TO_565(129,120,220));
    vm_ascii_to_ucs2(wstr,40,"Return");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr)-2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),VM_COLOR_RED);
	vm_graphic_flush_layer(layer,1);
} 

void draw_xml_info(VMCHAR * ptr)
{
	VMWCHAR wstr[20];	
    VMWCHAR s[XML_BUF_LEN], temp[32];
	VMWCHAR * p;
	VMCHAR *ch = "|";
	VMWCHAR ch1[50];	
	VMWCHAR ch2[2];		
	VMINT x=0, y=30, i=0;
	vm_graphic_fill_rect(layer_buffer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VM_COLOR_RED,VM_COLOR_BLACK);
	vm_ascii_to_ucs2(wstr,40,"Get_Xml_Info");
	vm_graphic_textout(layer_buffer,(SCREEN_WIDTH-vm_graphic_get_string_width(wstr))/2,13,wstr,wstrlen(wstr),VM_COLOR_888_TO_565(129,220,10));	
	vm_ascii_to_ucs2(ch1, 100, ch);		
	vm_ascii_to_ucs2(s, XML_BUF_LEN * 2, ptr);
	p = s;
	memset(temp , 0 , sizeof(temp));
	while (*p)		
	{
		temp[i++] = *p ;		
		ch2[0] = *p;		
		ch2[1] = 0;		
		if(vm_graphic_get_string_width(temp) > vm_graphic_get_screen_width() || wstrcmp(ch2 , ch1) == 0)			
		{			
			if(!wstrcmp(ch2,ch1))			
				p++;			
			temp[i-1] = 0;

			vm_graphic_textout(layer_buffer, x , y , temp , wstrlen(temp), VM_COLOR_BLUE);		
			i = 0;		
			y += vm_graphic_get_character_height() + (vm_graphic_get_character_height()>>3);			
			memset(temp , 0 , sizeof(temp));			
			continue;
		}		
		p++;		
	}
	if(wstrlen(temp))		
	{		
		vm_graphic_textout(layer_buffer, x , y , temp , wstrlen(temp), VM_COLOR_BLUE);		
	}	    	
	vm_ascii_to_ucs2(wstr,40,"Return");
	vm_graphic_textout(layer_buffer,SCREEN_WIDTH-vm_graphic_get_string_width(wstr)-2,SCREEN_HEIGHT-FONT_HEIGHT-2,wstr,wstrlen(wstr),VM_COLOR_WHITE);
	vm_graphic_flush_layer(layer,1);
}
