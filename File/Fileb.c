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

const VMINT abc = 0;

VMINT file_test(void)
{
	VMINT a;
	a = abc;
}