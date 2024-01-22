#ifndef _VRE_APP_graphic_demo_4_
#define	_VRE_APP_graphic_demo_4_


#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include <stdlib.h>
#include <string.h>


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

/*
 * demo
 */
static void draw_hello(void);

#endif

