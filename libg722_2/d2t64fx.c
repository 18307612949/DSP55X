/*-------------------------------------------------------------------*
 *                         D2T64FX.C							     *
 *-------------------------------------------------------------------*
 * 12 bits algebraic codebook decoder.                               *
 * 2 tracks x 32 positions per track = 64 samples.                   *
 *                                                                   *
 * 12 bits --> 2 pulses in a frame of 64 samples.                    *
 *                                                                   *
 * All pulses can have two (2) possible amplitudes: +1 or -1.        *
 * Each pulse can have 32 possible positions.                        *
 *                                                                   *
 * See dec2t64.c for more details of the algebraic code.             *
 *-------------------------------------------------------------------*/

#include "typedef.h"
#include "acelp.h"

#define L_CODE    64                       /* codevector length  */
#define NB_TRACK  2                        /* number of track    */
#define NB_POS    32                       /* number of position */


void DEC_ACELP_2t64_fx(
     Word16 index,     /* (i) :    12 bits index                           */
     Word16 code[]     /* (o) :Q9  algebraic (fixed) codebook excitation   */
)
{
    Word16 i0, i1;

    Set_zero(code, L_CODE);

    /* decode the positions and signs of pulses and build the codeword */

    i0 = (index >> 5) & 0x003e;
    i1 = ((index & 0x001f) << 1) + 1;
    if (((index >> 6) & NB_POS) == 0)
        code[i0] = 512;
    else
        code[i0] = -512;

    if ((index & NB_POS) == 0)
        code[i1] = 512;
    else
        code[i1] = -512;
    return;
}
