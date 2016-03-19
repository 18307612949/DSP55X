//////////////////////////////////////////////////////////////////////////////
// * File name: ezdsp5535_sar.h
//////////////////////////////////////////////////////////////////////////////

#include "ezdsp5535.h"

#define SW1   0x2A8  // Switch 1 value
#define SW2   0x1FE  // Switch 2 value
#define SW12  0x195  // Switch 1 + 2 value
#define NoKey 0x3FE

/* ------------------------------------------------------------------------ *
 *  Prototypes                                                              *
 * ------------------------------------------------------------------------ */
void   EZDSP5535_SAR_init(void);
Uint16 EZDSP5535_SAR_getKey(void);
int setCommandByKey(Uint8 *buf);
