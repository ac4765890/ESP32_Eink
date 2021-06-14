#ifndef __EINK_H__
#define __EINK_H__

#include "common.h"

extern void einkInit();
extern void einkResetPinStatus();
extern void einkPowerUp();
extern void einkPowerDown();

extern void einkStartFrame();
extern void einkEndFrame();
extern void einkSendRowDataGray(const u8* data, const u32 highTime, const u32 lowTime, const short rowCount);
extern void einkSkipRow(const short rowCount);
extern void einkSendRowData(const u8* data, const short rowCount);


#endif // !__EINK_H__