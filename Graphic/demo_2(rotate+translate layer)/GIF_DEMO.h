#ifndef _VRE_APP_GIF_DEMO_
#define	_VRE_APP_GIF_DEMO_

#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmres.h"
#include "vmlog.h"


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

void init_img();

#endif

