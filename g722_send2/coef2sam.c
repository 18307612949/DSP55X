/*****************************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   2004 Polycom, Inc.
**
**   All rights reserved.
**
*****************************************************************************/

/*****************************************************************************
* Filename: rmlt_coefs_to_samples.c
*
* Purpose:  Convert Reversed MLT (Modulated Lapped Transform)
*           Coefficients to Samples
*
*     The "Reversed MLT" is an overlapped block transform which uses
*     even symmetry * on the left, odd symmetry on the right and a
*     Type IV DCT as the block transform.  * It is thus similar to a
*     MLT which uses odd symmetry on the left, even symmetry * on the
*     right and a Type IV DST as the block transform.  In fact, it is
*     equivalent * to reversing the order of the samples, performing
*     an MLT and then negating all * the even-numbered coefficients.
*
*****************************************************************************/

/***************************************************************************
 Include files
***************************************************************************/
#include "defs.h"
#include "tables.h"

#include "basop32.h"

/***************************************************************************
 Function:     rmlt_coefs_to_samples 

 Syntax:       void rmlt_coefs_to_samples(Int16 *coefs,       
                                          Int16 *old_samples, 
                                          Int16 *out_samples, 
                                          Int16 mag_shift)    
            
               inputs:    Int16 *coefs
                          Int16 *old_samples
                          Int16 mag_shift


               outputs:   Int16 *out_samples

 Description:  Converts the mlt_coefs to samples

 Design Notes:

 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |     1.91     |    1.91
          -------|--------------|----------------
            MAX  |     1.91     |    1.91
          -------|--------------|----------------
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |     3.97     |    3.97        |     3.97
          -------|--------------|----------------|----------------
            MAX  |     3.97     |    3.97        |     3.97
          -------|--------------|----------------|----------------

***************************************************************************/
extern Int16 windowed_data[];
#pragma DATA_ALIGN (old_samples, 2)
static Int16 old_samples[MAX_DCT_LENGTH*2];		// left and right channel
void rmlt_coefs_to_samples(Int16 *coefs,     
                           Int16 *out_samples,           
                           Int16 dct_length,
                           Int16 mag_shift,
                           Uint16 chn)
{
    Int16	i;
    Int16	*new_ptr, *old_ptr;
    Int16	*win_new, *win_old;
    Int16	*out_ptr;
    Int16   half_dct_size;
    Int32   sum;

    half_dct_size = dct_length >> 1;

    /* Perform a Type IV (inverse) DCT on the coefficients */
    dct_type_iv_s(coefs, windowed_data, dct_length);

	if(mag_shift != 0)
	    for(i =  dct_length; i--; )
    	    windowed_data[i] = shr(windowed_data[i], mag_shift);

    /* Get the first half of the windowed samples */

    out_ptr = out_samples;
    if (dct_length != MAX_DCT_LENGTH)
        win_new = rmlt_to_samples_window;
    else
        win_new = max_rmlt_to_samples_window;

	win_old = win_new + dct_length;

    old_ptr = &old_samples[chn * dct_length];
    new_ptr = windowed_data + half_dct_size;

    for (i = half_dct_size; i--; )
    {
        sum = L_mult(*win_new++, *--new_ptr);
        sum = L_mac(sum, *--win_old, *old_ptr++);
        *out_ptr++ = round16(L_shl(sum, 2));
    }

    /* Get the second half of the windowed samples */

    for (i = half_dct_size; i--; )
    {
        sum = L_mult(*win_new++, *new_ptr++);
        sum = L_msu(sum, *--win_old, *--old_ptr);
        *out_ptr++ = round16(L_shl(sum, 2));
    }

    /* Save the second half of the new samples for   */
    /* next time, when they will be the old samples. */

    /* pointer arithmetic */

	DSP_memcpy(&old_samples[chn*dct_length], &windowed_data[half_dct_size], half_dct_size>>1);
//
//  new_ptr = windowed_data + (DCT_LENGTH>>1);
//    old_ptr = old_samples;
//    for (i = 0; i < (DCT_LENGTH>>1); i++)
//        *old_ptr++ = *new_ptr++;
}

void coef2sam_init()
{
	DSP_zero(old_samples, MAX_DCT_LENGTH);
}

