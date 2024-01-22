#ifndef _VRE_APP_graphic_demo_6_
#define	_VRE_APP_graphic_demo_6_

#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include <stdio.h>
#include <stdlib.h>


VMINT		layer_hdl[6];				
VMINT		count_layer=0;

/*
 * system events
 */
void handle_sysevt(VMINT message, VMINT param);

/*
 * key events
 */
void handle_keyevt(VMINT event, VMINT keycode);

/*
 * pen events
 */
void handle_penevt(VMINT event, VMINT x, VMINT y);

static void draw_hello(void);

void App_init();


void App_quit();


#endif

