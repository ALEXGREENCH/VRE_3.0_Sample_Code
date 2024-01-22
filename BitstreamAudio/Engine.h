#ifndef _ENGINE_H_
#define _ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vmsys.h"

typedef VMUINT8 VMBOOL;

typedef struct {
	VMINT file_hdl;
	VMUINT file_size;
} audio_file_t;

#ifdef __cplusplus
}
#endif

#endif
