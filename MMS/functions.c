#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmres.h"
#include "vmlog.h"
#include "vmstdlib.h"
#include "functions.h"

void draw_text_muiltline(VMUINT8 *disp_buf,VMSTR str, VMINT x, VMINT y, VMINT width, VMINT height,VMINT betlines,VMINT startLine, VMINT color )
{
	VMWCHAR *ucstr;
	VMWCHAR *ucshead;
	VMWCHAR *ucstail;
	VMINT is_end = FALSE;
	VMINT nheight = y; 
	VMINT nline_height ;
	VMINT nlines = 0;
	if (str == NULL||disp_buf==NULL||betlines < 0)
		return;
	nline_height = vm_graphic_get_character_height() + betlines;
	ucstr = (VMWCHAR*)vm_malloc(2*(strlen(str)+1));
	if (ucstr == NULL)
		return;
	
	if(0 != vm_ascii_to_ucs2(ucstr,2*(strlen(str)+1),str))
	{
		vm_free(ucstr);
		return ;
	}
	ucshead = ucstr;
	ucstail = ucshead+1;
	
	while(is_end == FALSE)
	{
		if (nheight+nline_height>y+height)
			break;

		while (1)
		{
			if (get_string_width(ucshead,ucstail)<=width)
			{
				ucstail ++;
			}
			else
			{
				nlines++;
				ucstail --;
				break;
			}
			if (0==vm_wstrlen(ucstail))
			{
				is_end = TRUE;
				nlines++;
				break;
			}
		}
		if ( nlines >= startLine)
		{
			vm_graphic_textout(disp_buf, x, nheight, ucshead, (ucstail-ucshead), (VMUINT16)(color));
			nheight += nline_height;
		}
		ucshead = ucstail;
		ucstail ++;
	}
	vm_free(ucstr);
}

void draw_text_muiltline_wstr(VMUINT8 *disp_buf,VMWSTR str, VMINT x, VMINT y, VMINT width, VMINT height,VMINT betlines,VMINT startLine, VMINT color )
{
	VMWCHAR *ucshead;
	VMWCHAR *ucstail;
	VMINT is_end = FALSE;
	VMINT nheight = y; 
	VMINT nline_height ;
	VMINT nlines = 0;
	if (str == NULL||disp_buf==NULL||betlines < 0)
		return;
	nline_height = vm_graphic_get_character_height() + betlines;

	
	ucshead = str;
	ucstail = ucshead+1;
	
	while(is_end == FALSE)
	{
		if (nheight+nline_height>y+height)
			break;
		
		while (1)
		{
			if (get_string_width(ucshead,ucstail)<=width)
			{
				ucstail ++;
			}
			else
			{
				nlines++;
				ucstail --;
				break;
			}
			if (0==vm_wstrlen(ucstail))
			{
				is_end = TRUE;
				nlines++;
				break;
			}
		}
		if ( nlines >= startLine)
		{
			vm_graphic_textout(disp_buf, x, nheight, ucshead, (ucstail-ucshead), (VMUINT16)(color));
			nheight += nline_height;
		}
		ucshead = ucstail;
		ucstail ++;
	}
}
VMINT  text_lines(VMSTR str, VMINT width )
{
	VMWCHAR *ucstr;
	VMWCHAR *ucshead;
	VMWCHAR *ucstail;
	VMINT is_end = FALSE;

	VMINT nlines = 0;

	if (width<=0)
	{
		return -1;
	}
	if (str == NULL)
		return -1;
	ucstr = (VMWCHAR*)vm_malloc(2*(strlen(str)+1));
	if (ucstr == NULL)
		return -1;
	
	if(0 != vm_ascii_to_ucs2(ucstr,2*(strlen(str)+1),str))
	{
		vm_free(ucstr);
		return -1;
	}
	ucshead = ucstr;
	ucstail = ucshead+1;
	
	while(is_end == FALSE)
	{
		nlines ++;
		while (1)
		{
			if (get_string_width(ucshead,ucstail)<=width)
			{
				ucstail ++;
			}
			else
			{
				ucstail --;
				break;
			}
			if (0==vm_wstrlen(ucstail))
			{
				is_end = TRUE;
				break;
			}
		}
		if (ucshead == ucstail)
		{
			nlines = -2;
			break;
		}
//		nheight += nline_height;
		ucshead = ucstail;
		ucstail ++;
	}
	vm_free(ucstr);
	return nlines;
}
VMINT  text_lines_wstr(VMWSTR str, VMINT width )
{
	VMWCHAR *ucshead;
	VMWCHAR *ucstail;
	VMINT is_end = FALSE;
	
	VMINT nlines = 0;
	
	if (width<=0)
	{
		return -1;
	}
	if (str == NULL)
		return -1;

	ucshead = str;
	ucstail = ucshead+1;
	while(is_end == FALSE)
	{

		nlines ++;
		while (1)
		{
			if (get_string_width(ucshead,ucstail)<=width)
			{
				ucstail ++;
			}
			else
			{
				ucstail --;
				break;
			}
			if (0==vm_wstrlen(ucstail))
			{
				is_end = TRUE;
				break;
			}
		}
		if (ucshead == ucstail)
		{
			nlines = -2;
			break;
		}
		ucshead = ucstail;
		ucstail ++;
	}
	return nlines;
}

VMINT get_lines_inrect(VMINT height,VMINT betlines)
{
	VMINT nline_height ;
	VMINT nlines_inrect ;
	if ( betlines < 0)
	return -1;
	nline_height = vm_graphic_get_character_height()+betlines;
	nlines_inrect = (height+betlines)/nline_height;
	return nlines_inrect;
}

VMINT get_string_width(VMWCHAR *whead,VMWCHAR *wtail)
{
	VMWCHAR * wtemp = NULL;
	VMINT width = 0;
	if (whead == NULL || wtail == NULL)
	return 0;
	wtemp = (VMWCHAR *)vm_malloc((wtail-whead)*2+2);
	
	if (wtemp == NULL)
	return 0;
	memset(wtemp,0,(wtail-whead)*2+2);
	memcpy(wtemp,whead,(wtail-whead)*2);

	width = vm_graphic_get_string_width(wtemp);
	vm_free(wtemp);
	return width;
}
