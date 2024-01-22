#ifndef _VRE_APP_fun_
#define	_VRE_APP_fun_

#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmres.h"
#include "vmlog.h"
#include "vmstdlib.h"

void draw_text_muiltline(VMUINT8 *disp_buf,VMSTR str, VMINT x, VMINT y, VMINT width, VMINT height, VMINT betlines,VMINT startLine,VMINT color );

void draw_text_muiltline_wstr(VMUINT8 *disp_buf,VMWSTR str, VMINT x, VMINT y, VMINT width, VMINT height, VMINT betlines,VMINT startLine,VMINT color );


VMINT  text_lines(VMSTR str, VMINT width );

VMINT  text_lines_wstr(VMWSTR str, VMINT width );

VMINT get_lines_inrect(VMINT height,VMINT betlines);

VMINT get_string_width(VMWCHAR *whead,VMWCHAR *wtail);

void textoutCenter(VMUINT8* disp_buf,  VMINT y, VMWSTR s,VMUINT16 color);

#endif

