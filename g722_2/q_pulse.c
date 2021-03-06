/*--------------------------------------------------------------------------*
 *                         Q_PULSE.C										*
 *--------------------------------------------------------------------------*
 * Coding and decodeing of algebraic codebook                               *
 *--------------------------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"

#define NB_POS_EXP  4
#define NB_POS 16                    /* pos in track, mask for sign bit */

Word32 quant_1p_N1(                  /* (o) return N+1 bits             */
     Word16 pos,                     /* (i) position of the pulse       */
     Word16 N)                       /* (i) number of bits for position */
{
    Word16 mask;
    Word32 index, bit;

    mask = (1L << N) - 1L;
    /*-------------------------------------------------------*
     * Quantization of 1 pulse with N+1 bits:                *
     *-------------------------------------------------------*/
    index = (Word32)(pos & mask);
    bit = (Word32)(pos >> NB_POS_EXP) & 1L;
    index = L_add(index, (bit << N));

    return (index);
}

void dec_1p_N1(Word32 index, Word16 N, Word16 offset, Word16 pos[])
{
    Word16 pos1, bit;
    Word32 mask;

    mask = (1L << N) - 1L;   /* mask = ((1<<N)-1); */
    /*-------------------------------------------------------*
     * Decode 1 pulse with N+1 bits:                         *
     *-------------------------------------------------------*/
    pos1 = add((Word16)(index & mask), offset);
    bit = (Word16)(index >> N) & 1;
    pos[0] = add(pos1, (bit << NB_POS_EXP));

    return;
}

Word32 quant_2p_2N1(                       /* (o) return (2*N)+1 bits         */
     Word16 pos1,                          /* (i) position of the pulse 1     */
     Word16 pos2,                          /* (i) position of the pulse 2     */
     Word16 N)                             /* (i) number of bits for position */
{
    Word16 mask;
    Word32 index, bit;
    Word16 mpos1, mpos2;

    mask = (1 << N) - 1;
    mpos1 = pos1 & mask;
    mpos2 = pos2 & mask;

    /*-------------------------------------------------------*
     * Quantization of 2 pulses with 2*N+1 bits:             *
     *-------------------------------------------------------*/
    if (((pos2 ^ pos1) & NB_POS) == 0)
    {
        /* sign of 1st pulse == sign of 2th pulse */
        if (pos1 < pos2)
        {
            index = (Word32)(add(shl(mpos1, N), mpos2));
        } else
        {
            index = (Word32)(add(shl(mpos2, N), mpos1));
        }

        bit = ((Word32)pos1 >> NB_POS_EXP) & 1L;
    } else
    {
        /* sign of 1st pulse != sign of 2th pulse */
        if (mpos1 <= mpos2)
        {
            /* index = ((pos2 & mask) << N) + (pos1 & mask); */
            index = (Word32)(add(shl(mpos2, N), mpos1));

            bit = ((Word32)pos2 >> NB_POS_EXP) & 1L;
        } else
        {
            /* index = ((pos1 & mask) << N) + (pos2 & mask);	 */
            index = (Word32)(add(shl(mpos1, N), mpos2));

            bit = ((Word32)pos1 >> NB_POS_EXP) & 1L;
        }
    }
    index = L_add(index, (bit << (2*N)));

    return (index);
}

void dec_2p_2N1(Word32 index, Word16 N, Word16 offset, Word16 pos[])
{
    Word16 pos1, pos2, bit;
    Word32 mask;

    mask = (1L << N) - 1L;
    /*-------------------------------------------------------*
     * Decode 2 pulses with 2*N+1 bits:                      *
     *-------------------------------------------------------*/
    /* pos1 = (((index >> N) & mask) + offset); */
    pos1 = add((Word16)((index >> N) & mask), offset);
    pos2 = add((Word16)(index & mask), offset);
    bit = (Word16)((index >> (2*N))) & 1;

    if (pos2 < pos1)
    {
        pos[1] = add(pos2, ((1 - bit) << NB_POS_EXP));
    } else
    {
        pos[1] = add(pos2, (bit << NB_POS_EXP));
    }
    pos[0] = add(pos1, (bit << NB_POS_EXP));

    return;
}

Word32 quant_3p_3N1(                       /* (o) return (3*N)+1 bits         */
     Word16 pos1,                          /* (i) position of the pulse 1     */
     Word16 pos2,                          /* (i) position of the pulse 2     */
     Word16 pos3,                          /* (i) position of the pulse 3     */
     Word16 N)                             /* (i) number of bits for position */
{
    Word16 nb_pos;
    Word32 index;
    Word16 p1, p2, p3;

    nb_pos = 1 << (N - 1);            /* nb_pos = (1<<(N-1)); */
    /*-------------------------------------------------------*
     * Quantization of 3 pulses with 3*N+1 bits:             *
     *-------------------------------------------------------*/
    if (((pos1 ^ pos2) & nb_pos) == 0)
    {
        p1 = pos1;
        p2 = pos2;
        p3 = pos3;

    } else if (((pos1 ^ pos3) & nb_pos) == 0)
    {
        p1 = pos1;
        p2 = pos3;
        p3 = pos2;
    } else
    {
        p1 = pos2;
        p2 = pos3;
        p3 = pos1;
    }

    index = (Word32)(p1 & nb_pos) << N;
    index = L_add(index, quant_2p_2N1(p1, p2, (N - 1)));
    index = L_add(index, L_shl(quant_1p_N1(p3, N), (2*N)));

    return (index);
}

void dec_3p_3N1(Word32 index, Word16 N, Word16 offset, Word16 pos[])
{
    Word16 j, tmp, bit;
    Word32 mask, idx;

    /*-------------------------------------------------------*
     * Decode 3 pulses with 3*N+1 bits:                      *
     *-------------------------------------------------------*/
    tmp = 2 * N - 1;
    mask = (1L << tmp) - 1L;

    idx = index & mask;
    bit = (Word16)(index >> tmp) & 1;

    j = add(offset, (bit << (N - 1)));
    dec_2p_2N1(idx, (N - 1), j, pos);

    mask = (1L << (N+1)) - 1;
    tmp = N * 2;
    idx = (index >> tmp) & mask;

    dec_1p_N1(idx, N, offset, pos + 2);

    return;
}

Word32 quant_4p_4N1(                       /* (o) return (4*N)+1 bits         */
     Word16 pos1,                          /* (i) position of the pulse 1     */
     Word16 pos2,                          /* (i) position of the pulse 2     */
     Word16 pos3,                          /* (i) position of the pulse 3     */
     Word16 pos4,                          /* (i) position of the pulse 4     */
     Word16 N)                             /* (i) number of bits for position */
{
    Word16 nb_pos;
    Word32 index;
    Word16 p1, p2, p3;

    nb_pos = 1 << (N - 1);
    /*-------------------------------------------------------*
     * Quantization of 4 pulses with 4*N+1 bits:             *
     *-------------------------------------------------------*/
    if (((pos1 ^ pos2) & nb_pos) == 0)
    {
		p1 = pos1;
		p2 = pos2;
		p3 = pos3;
    } else if (((pos1 ^ pos3) & nb_pos) == 0)
    {
		p1 = pos1;
		p2 = pos3;
		p3 = pos2;
    } else
    {
		p1 = pos2;
		p2 = pos3;
		p3 = pos1;
    }

    index = (Word32)(p1 & nb_pos) << N;
    index = L_add(index, quant_2p_2N1(p1, p2, (N - 1)));
    index = L_add(index, L_shl(quant_2p_2N1(p3, pos4, N), (2*N)));

    return (index);
}

void dec_4p_4N1(Word32 index, Word16 N, Word16 offset, Word16 pos[])
{
    Word16 j, tmp, bit;
    Word32 mask, idx;

    /*-------------------------------------------------------*
     * Decode 4 pulses with 4*N+1 bits:                      *
     *-------------------------------------------------------*/
    tmp = N * 2 - 1;
    mask = (1L << tmp) -1L;
    idx = index & mask;
    bit = (Word16)(index >> tmp) & 1;
    j = offset + (Word16)(bit << (N-1));

    dec_2p_2N1(idx, (N - 1), j, pos);

    tmp = N * 2 + 1;
    mask = (1L << tmp) - 1L;
    idx = (index >> (2*N)) & mask;
    dec_2p_2N1(idx, N, offset, pos + 2);

    return;
}

Word32 quant_4p_4N(                        /* (o) return 4*N bits             */
     Word16 pos[],                         /* (i) position of the pulse 1..4  */
     Word16 N)                             /* (i) number of bits for position */
{
    Word16 i, j, k, nb_pos, n_1, tmp;
    Word16 posA[4], posB[4];
    Word32 index;

    n_1 = N - 1;
    nb_pos = 1 << n_1;

    i = 0;
    j = 0;
    for (k = 0; k < 4; k++)
    {
        if ((pos[k] & nb_pos) == 0)
        {
            posA[i++] = pos[k];
        } else
        {
            posB[j++] = pos[k];
        }
    }

    switch (i)
    {
    case 0:
        tmp = 4 * N - 3;
        index = 1L << tmp;
        index = L_add(index, quant_4p_4N1(posB[0], posB[1], posB[2], posB[3], n_1));
        break;
    case 1:
        tmp = 3 * N - 2;
        index = L_shl(quant_1p_N1(posA[0], n_1), tmp);
        index = L_add(index, quant_3p_3N1(posB[0], posB[1], posB[2], n_1));
        break;
    case 2:
        tmp = 2 * N - 1;
        index = L_shl(quant_2p_2N1(posA[0], posA[1], n_1), tmp);
        index = L_add(index, quant_2p_2N1(posB[0], posB[1], n_1));
        break;
    case 3:
        index = L_shl(quant_3p_3N1(posA[0], posA[1], posA[2], n_1), N);
        index = L_add(index, quant_1p_N1(posB[0], n_1));
        break;
    case 4:
        index = quant_4p_4N1(posA[0], posA[1], posA[2], posA[3], n_1);
        break;
    default:
        index = 0;
        break;
    }

    tmp = N * 4 - 2;
    index = L_add(index, L_shl((((Word32)i) & 3L), tmp));

    return (index);
}

void dec_4p_4N(Word32 index, Word16 N, Word16 offset, Word16 pos[])
{
    Word16 j, n_1, tmp, bit;

    /*-------------------------------------------------------*
     * Decode 4 pulses with 4*N bits:                        *
     *-------------------------------------------------------*/

    n_1 = N - 1;
    j = offset + (1 << n_1);

    tmp = 4 * N - 2;
    switch (L_shr(index, tmp) & 3)
    {                                      /* ((index >> ((4*N)-2)) & 3) */
    case 0:
        tmp = 4 * N - 3;
        bit = (Word16)(index >> tmp) & 1;

        j = offset + (bit << n_1);
        dec_4p_4N1(index, n_1, j, pos);
        break;
    case 1:
        tmp = 3 * N - 2;
        dec_1p_N1(L_shr(index, tmp), n_1, offset, pos);
        dec_3p_3N1(index, n_1, j, pos + 1);
        break;
    case 2:
        tmp = 2 * N - 1;
        dec_2p_2N1(L_shr(index, tmp), n_1, offset, pos);
        dec_2p_2N1(index, n_1, j, pos + 2);
        break;
    case 3:
        tmp = N;
        dec_3p_3N1(L_shr(index, tmp), n_1, offset, pos);
        dec_1p_N1(index, n_1, j, pos + 3);
        break;
    }
    return;
}

Word32 quant_5p_5N(                        /* (o) return 5*N bits             */
     Word16 pos[],                         /* (i) position of the pulse 1..5  */
     Word16 N)                             /* (i) number of bits for position */
{
    Word16 i, j, k, nb_pos, n_1, tmp;
    Word16 posA[5], posB[5];
    Word32 index, tmp2;

    n_1 = N - 1;
    nb_pos = 1 << n_1;                  /* nb_pos = (1<<n_1); */

    i = 0;
    j = 0;
    for (k = 0; k < 5; k++)
    {
        if ((pos[k] & nb_pos) == 0)
        {
            posA[i++] = pos[k];
        } else
        {
            posB[j++] = pos[k];
        }
    }

    index = 1 << (5*N-1);
    tmp = 2 * N + 1;
    switch (i)
    {
    case 0:
        tmp2 = L_shl(quant_3p_3N1(posB[0], posB[1], posB[2], n_1), tmp);
        index = L_add(index, tmp2);
        index = L_add(index, quant_2p_2N1(posB[3], posB[4], N));
        break;
    case 1:
        tmp2 = L_shl(quant_3p_3N1(posB[0], posB[1], posB[2], n_1), tmp);
        index = L_add(index, tmp2);
        index = L_add(index, quant_2p_2N1(posB[3], posA[0], N));
        break;
    case 2:
        tmp2 = L_shl(quant_3p_3N1(posB[0], posB[1], posB[2], n_1), tmp);
        index = L_add(index, tmp2);
        index = L_add(index, quant_2p_2N1(posA[0], posA[1], N));
        break;
    case 3:
        index = L_shl(quant_3p_3N1(posA[0], posA[1], posA[2], n_1), tmp);
        index = L_add(index, quant_2p_2N1(posB[0], posB[1], N));
        break;
    case 4:
        index = L_shl(quant_3p_3N1(posA[0], posA[1], posA[2], n_1), tmp);
        index = L_add(index, quant_2p_2N1(posA[3], posB[0], N));
        break;
    case 5:
        index = L_shl(quant_3p_3N1(posA[0], posA[1], posA[2], n_1), tmp);
        index = L_add(index, quant_2p_2N1(posA[3], posA[4], N));
        break;
    default:
        index = 0;
        break;
    }

    return (index);
}

void dec_5p_5N(Word32 index, Word16 N, Word16 offset, Word16 pos[])
{
    Word16 j, n_1, tmp, bit;
    Word32 idx;

    /*-------------------------------------------------------*
     * Decode 5 pulses with 5*N bits:                        *
     *-------------------------------------------------------*/

    n_1 = N - 1;
    tmp = 2 * N + 1;
    idx = index >> tmp;
    tmp = 5 * N - 1;

    bit = (Word16)(index >> tmp) & 1;
    j = offset + (bit << n_1);
    dec_3p_3N1(idx, n_1, j, pos);
    dec_2p_2N1(index, N, offset, pos + 3);

    return;
}

Word32 quant_6p_6N_2(                      /* (o) return (6*N)-2 bits         */
     Word16 pos[],                         /* (i) position of the pulse 1..6  */
     Word16 N)                             /* (i) number of bits for position */
{
    Word16 i, j, k, nb_pos, n_1;
    Word16 posA[6], posB[6];
    Word32 index;

    /* !!  N and n_1 are constants ->
        it doesn't need to be operated by Basic Operators */

    n_1 = N - 1;
    nb_pos = 1 << n_1;                  /* nb_pos = (1<<n_1); */

    i = 0;
    j = 0;
    for (k = 0; k < 6; k++)
    {
        if ((pos[k] & nb_pos) == 0)
        {
            posA[i++] = pos[k];
        } else
        {
            posB[j++] = pos[k];
        }
    }

    switch (i)
    {
    case 0:
        index = 1 * (1L << (6 * N - 5));
        index = L_add(index, L_shl(quant_5p_5N(posB, n_1), N));
        index = L_add(index, quant_1p_N1(posB[5], n_1));
        break;
    case 1:
        index = 3 * (1L << (6 * N - 5));
        index = L_add(index, L_shl(quant_5p_5N(posB, n_1), N));
        index = L_add(index, quant_1p_N1(posA[0], n_1));
        break;
    case 2:
        index = 5 * (1L << (6 * N - 5));
        index = L_add(index, L_shl(quant_4p_4N(posB, n_1), 2*N - 1));
        index = L_add(index, quant_2p_2N1(posA[0], posA[1], n_1));
        break;
    case 3:
        index = 6 * (1L << (6 * N - 5));
        index = L_add(index, L_shl(quant_3p_3N1(posA[0], posA[1], posA[2], n_1), 3*N - 2));
        index = L_add(index, quant_3p_3N1(posB[0], posB[1], posB[2], n_1));
        break;
    case 4:
        index = 4 * (1L << (6 * N - 5));
        index = L_add(index, L_shl(quant_4p_4N(posA, n_1), 2*N - 1));
        index = L_add(index, quant_2p_2N1(posB[0], posB[1], n_1));
        break;
    case 5:
        index = 2 * (1L << (6 * N - 5));
        index = L_add(index, L_shl(quant_5p_5N(posA, n_1), N));
        index = L_add(index, quant_1p_N1(posB[0], n_1));
        break;
    case 6:
        index = 0;
        index = L_add(index, L_shl(quant_5p_5N(posA, n_1), N));
        index = L_add(index, quant_1p_N1(posA[5], n_1));
        break;
    default:
        index = 0;
        break;
    }

    return (index);
}

void dec_6p_6N_2(Word32 index, Word16 N, Word16 offset, Word16 pos[])
{
    Word16 j, n_1, offsetA, offsetB, bit;
	Word32 idx;

    n_1 = N - 1;
    /* !!  N and n_1 are constants
       -> it doesn't need to be operated by Basic Operators */

    bit = (Word16)(index >> (6*N-5)) & 1;
    j = offset + (1 << n_1);

    offsetA = offset + (bit << n_1);
    offsetB = offset + ((1 - bit) << n_1);

    switch (L_shr(index, (Word16) (6 * N - 4)) & 3)
    {                                      /* (index >> ((6*N)-4)) & 3 */
    case 0:
        idx = index >> N;
        dec_5p_5N(idx, n_1, offsetA, pos);
        dec_1p_N1(index, n_1, offsetA, pos + 5);
        break;
    case 1:
        idx = index >> N;
        dec_5p_5N(idx, n_1, offsetA, pos);
        dec_1p_N1(index, n_1, offsetB, pos + 5);
        break;
    case 2:
        idx = index >> (2*N-1);
        dec_4p_4N(idx, n_1, offsetA, pos);
        dec_2p_2N1(index, n_1, offsetB, pos + 4);
        break;
    case 3:
        idx = index >> (3*N-2);
        dec_3p_3N1(idx, n_1, offset, pos);
        dec_3p_3N1(index, n_1, j, pos + 3);
        break;
    }

    return;
}

