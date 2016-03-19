/***********************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   2004 Polycom, Inc.
**
**   All rights reserved.
**
***********************************************************************/
#include	"libg722_1.h"

#define REGION_POWER_STEPSIZE_DB 3.010299957
#define ABS_REGION_POWER_LEVELS  32
#define DIFF_REGION_POWER_LEVELS 24

#define DRP_DIFF_MIN -12
#define DRP_DIFF_MAX 11

#define MAX_NUM_BINS 16
#define MAX_VECTOR_INDICES 625
#define MAX_VECTOR_DIMENSION 5

extern Int16  differential_region_power_bits[MAX_NUMBER_OF_REGIONS][DIFF_REGION_POWER_LEVELS];
extern Uint16 differential_region_power_codes[MAX_NUMBER_OF_REGIONS][DIFF_REGION_POWER_LEVELS];
extern Int16  differential_region_power_decoder_tree[MAX_NUMBER_OF_REGIONS][DIFF_REGION_POWER_LEVELS-1][2];
extern Int16  mlt_quant_centroid[NUM_CATEGORIES][MAX_NUM_BINS];
extern Int16  expected_bits_table[NUM_CATEGORIES];
extern Int16  mlt_sqvh_bitcount_category_0[196];
extern Uint16 mlt_sqvh_code_category_0[196];
extern Int16  mlt_sqvh_bitcount_category_1[100];
extern Uint16 mlt_sqvh_code_category_1[100];
extern Int16  mlt_sqvh_bitcount_category_2[49];
extern Uint16 mlt_sqvh_code_category_2[49];
extern Int16  mlt_sqvh_bitcount_category_3[625];
extern Uint16 mlt_sqvh_code_category_3[625];
extern Int16  mlt_sqvh_bitcount_category_4[256];
extern Uint16 mlt_sqvh_code_category_4[256];
extern Int16  mlt_sqvh_bitcount_category_5[243];
extern Uint16 mlt_sqvh_code_category_5[243];
extern Int16  mlt_sqvh_bitcount_category_6[32];
extern Uint16 mlt_sqvh_code_category_6[32];
extern Int16  *table_of_bitcount_tables[NUM_CATEGORIES-1];
extern Uint16 *table_of_code_tables[NUM_CATEGORIES-1];
extern Int16  mlt_decoder_tree_category_0[180][2];
extern Int16  mlt_decoder_tree_category_1[93][2];
extern Int16  mlt_decoder_tree_category_2[47][2];
extern Int16  mlt_decoder_tree_category_3[519][2];
extern Int16  mlt_decoder_tree_category_4[208][2];
extern Int16  mlt_decoder_tree_category_5[191][2];
extern Int16  mlt_decoder_tree_category_6[31][2];
extern Int16  *table_of_decoder_tables[NUM_CATEGORIES-1];

