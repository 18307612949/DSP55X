//////////////////////////////////////////////////////////////////////////////
// * File name: ezdsp5535_i2c.h
//////////////////////////////////////////////////////////////////////////////

#include "ezdsp5535.h"

/* ------------------------------------------------------------------------ *
 *  Prototypes                                                              *
 * ------------------------------------------------------------------------ */
Int16 EZDSP5535_I2C_init ( );
Int16 EZDSP5535_I2C_close( );
Int16 EZDSP5535_I2C_read( Uint16 i2c_addr, Uint16* data, Uint16 len );
Int16 EZDSP5535_I2C_write( Uint16 i2c_addr, Uint16* data, Uint16 len );

