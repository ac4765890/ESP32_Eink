#ifndef __SDIO_H__
#define __SDIO_H__

#include "common.h"
#include "ff.h"

extern esp_err_t sdInit(const u8 d0, const u8 d1, const u8 d2, const u8 d3, const u8 cmd, const char *mountPoint);
extern bool sdCheckExistsFile(const char *path);
extern int sdRenameFile(const char *src, const char *target);
extern int sdDelFile(const char *path);
extern void sdUnmount();
extern u32 sdGetItemListCount(const char* dir, const char* suffix);
extern FILE *sdOpenfileNumber(const char *rootPath, const char *opr, const u32 number);

#endif