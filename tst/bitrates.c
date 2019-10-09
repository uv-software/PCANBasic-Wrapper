/*  -- $HeadURL: https://svn.uv-software.net/projects/uv-software/CAN/I386/DRV/trunk/API/misc/bitrates.c $ --
 *
 *  project   :  CAN - Controller Area Network
 *
 *  purpose   :  CAN FD Bit-rates (Converter)
 *
 *  copyright :  (C) 2017-20xx, UV Software, Berlin
 *
 *  compiler  :  Microsoft Visual C/C++ Compiler
 *               Apple LLVM (clang) Compiler
 *               GNU C/C++ Compiler
 *
 *  export    :  (see header file)
 *
 *  includes  :  bitrates.h
 *
 *  author    :  Uwe Vogt, UV Software
 *
 *  e-mail    :  uwe.vogt@uv-software.de
 *
 *
 *  -----------  description  --------------------------------------------
 */
/** @file        bitrates.c
 *
 *  @brief       CAN FD Bit-rates (Converter)
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 592 $
 *
 *  @addtogroup  bit_rate
 *  @{
 */

/*  -----------  includes  -----------------------------------------------
 */

#include "bitrates.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*  -----------  defines  ------------------------------------------------
 */

#define BTR_FREQUENCY_MIN       BTR_FREQUENCY_SJA1000
#define BTR_FREQUENCY_MAX       (1000000000ul)
#define BTR_FREQUENCY_MHZ_MIN   (BTR_FREQUENCY_MIN / 1000000ul)
#define BTR_FREQUENCY_MHZ_MAX   (BTR_FREQUENCY_MAX / 1000000ul)

#define BTR_SJA1000_BRP_MIN     (1u)
#define BTR_SJA1000_BRP_MAX     (64u)
#define BTR_SJA1000_TSEG1_MIN   (1u)
#define BTR_SJA1000_TSEG1_MAX   (16u)
#define BTR_SJA1000_TSEG2_MIN   (1u)
#define BTR_SJA1000_TSEG2_MAX   (8u)
#define BTR_SJA1000_SJW_MIN     (1u)
#define BTR_SJA1000_SJW_MAX     (4u)
#define BTR_SJA1000_SAM_MIN     (0u)
#define BTR_SJA1000_SAM_MAX     (1u)
#define BTR_SJA1000_SP_MIN      (0.20)
#define BTR_SJA1000_SP_MAX      (0.95)

#define BTR_NOMINAL_BRP_MIN     (1u)
#define BTR_NOMINAL_BRP_MAX     (1024u)
#define BTR_NOMINAL_TSEG1_MIN   (1u)
#define BTR_NOMINAL_TSEG1_MAX   (256u)
#define BTR_NOMINAL_TSEG2_MIN   (1u)
#define BTR_NOMINAL_TSEG2_MAX   (128u)
#define BTR_NOMINAL_SJW_MIN     (1u)
#define BTR_NOMINAL_SJW_MAX     (128u)
#define BTR_NOMINAL_SAM_MIN     (1u)
#define BTR_NOMINAL_SAM_MAX     (8u)
#define BTR_NOMINAL_SP_MIN      (0.05)
#define BTR_NOMINAL_SP_MAX      (1.00)
#define BTR_NOMINAL_SPEED_MIN   (1000)
#define BTR_NOMINAL_SPEED_MAX   (2000000)

#define BTR_DATA_BRP_MIN        BTR_NOMINAL_BRP_MIN
#define BTR_DATA_BRP_MAX        BTR_NOMINAL_BRP_MAX
#define BTR_DATA_TSEG1_MIN      (1u)
#define BTR_DATA_TSEG1_MAX      (32u)
#define BTR_DATA_TSEG2_MIN      (1u)
#define BTR_DATA_TSEG2_MAX      (16u)
#define BTR_DATA_SJW_MIN        (1u)
#define BTR_DATA_SJW_MAX        (16u)
#define BTR_DATA_SP_MIN         (0.1)
#define BTR_DATA_SP_MAX         (1.0)
#define BTR_DATA_SPEED_MIN      (1000)
#define BTR_DATA_SPEED_MAX      (12000000)

/*  - - - - - -  helper macros  - - - - - - - - - - - - - - - - - - - - - -
 */
#define BTR_SJW(btr0btr1)       (((unsigned short)(btr0btr1) & 0xC000u) >> 14)
#define BTR_BRP(btr0btr1)       (((unsigned short)(btr0btr1) & 0x3F00u) >> 8)
#define BTR_SAM(btr0btr1)       (((unsigned short)(btr0btr1) & 0x0080u) >> 7)
#define BTR_TSEG2(btr0btr1)     (((unsigned short)(btr0btr1) & 0x0070u) >> 4)
#define BTR_TSEG1(btr0btr1)     (((unsigned short)(btr0btr1) & 0x000Fu) >> 0)
#define BTR_BTR0BTR1(sjw,brp,sam,tseg2,tseg1) \
                                ((((unsigned short)(sjw) & 0x0003) << 14)  | \
                                 (((unsigned short)(brp) & 0x003F) << 8)   | \
                                 (((unsigned short)(sam) & 0x0001) << 7)   | \
                                 (((unsigned short)(tseg2) & 0x0007) << 4) | \
                                 (((unsigned short)(tseg1) & 0x000F) << 0))
#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

/*  -----------  types  --------------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

static unsigned long calc_bit_rate(unsigned long brp, unsigned long tseg1, unsigned long tseg2, unsigned long frequency);
static float calc_sample_point(unsigned long tseg1, unsigned long tseg2);

static char *scan_key(char *str);
static char *scan_value(char *str);
static char *skip_blanks(char *str);


/*  -----------  variables  ----------------------------------------------
 */

static const unsigned short sja1000_btr0btr1[10] = {
    0x0014U,  //   1 MBit/s
    0x0016U,  // 800 kBit/s
    0x001CU,  // 500 kBit/s
    0x011CU,  // 250 kBit/s
    0x031CU,  // 125 kBit/s
    0x432FU,  // 100 kBit/s
    0x472FU,  //  50 kBit/s
    0x532FU,  //  20 kBit/s
    0x672FU,  //  10 kBit/s
    0x7F7FU   //   5 kBit/s
};

/*  -----------  functions  ----------------------------------------------
 */

int btr_index_to_bit_rate_sja1000(long index, unsigned short *btr0btr1)
{
	if (index < 0)
		index *= -1;
	if (index > 9)
		return 0;
	if (!btr0btr1)
		return 0;
	*btr0btr1 = sja1000_btr0btr1[index];
	return 1;
}

int btr_index_to_bit_timing_sja1000(long index, unsigned long *frequency, struct btr_bit_timing *btr_timing)
{
	unsigned short btr0btr1 = 0x0000;

	if (!btr_timing)
		return 0;
	if (!btr_index_to_bit_rate_sja1000(index, &btr0btr1))
		return 0;
	if (frequency)
		*frequency = BTR_FREQUENCY_SJA1000;
	btr_timing->brp = BTR_BRP(btr0btr1);
	btr_timing->tseg1 = BTR_TSEG1(btr0btr1);
	btr_timing->tseg2 = BTR_TSEG2(btr0btr1);
	btr_timing->sjw = BTR_SJW(btr0btr1);
	btr_timing->sam = BTR_SAM(btr0btr1);
	return 1;
}

unsigned long btr_calc_bit_rate_sja1000(unsigned short btr0btr1)
{
    return calc_bit_rate((unsigned long)BTR_BRP(btr0btr1), (unsigned long)BTR_TSEG1(btr0btr1),
                         (unsigned long)BTR_TSEG2(btr0btr1), BTR_FREQUENCY_SJA1000);
}

float btr_calc_sample_point_sja1000(unsigned short btr0btr1)
{
    return calc_sample_point((unsigned long)BTR_TSEG1(btr0btr1), (unsigned long)BTR_TSEG2(btr0btr1));
}

int btr_find_bit_timing_sja1000(unsigned long bit_rate, float sample_point, struct btr_bit_timing *bit_timing)
{
    unsigned long frequency = BTR_FREQUENCY_SJA1000;
    unsigned short btr0btr1, i;

    if(!bit_timing)
        return 0;

    if((sample_point < BTR_SJA1000_SP_MIN) || (BTR_SJA1000_SP_MAX < sample_point))
        return 0;

    for(i = BTR_SJA1000_BRP_MIN; i <= BTR_SJA1000_BRP_MAX; i++) {
        bit_timing->brp = i;
        bit_timing->tseg1 = (unsigned long)(((float)frequency * sample_point) / (float)(bit_rate * i)) - 1u;
        bit_timing->tseg2 = (unsigned long)(((float)(1u + bit_timing->tseg1) / sample_point) - (float)(2u + bit_timing->tseg1)) + 1u;
        bit_timing->sjw = 1u;
        bit_timing->sam = 0u;
        btr0btr1 = BTR_BTR0BTR1((bit_timing->sjw - 1u),
                                (bit_timing->brp - 1u),
                                (bit_timing->sam),
                                (bit_timing->tseg2 - 1u),
                                (bit_timing->tseg1 - 1u));
        if(bit_rate == btr_calc_bit_rate_sja1000(btr0btr1))
            return 1;
    }
    return 0;
}

unsigned long btr_calc_bit_rate_nominal(const struct btr_bit_timing *bit_timing, unsigned long frequency)
{
    if(!bit_timing)
        return 0;

    if(((BTR_NOMINAL_BRP_MIN <= bit_timing->brp) && (bit_timing->brp <= BTR_NOMINAL_BRP_MAX)) &&
       ((BTR_NOMINAL_TSEG1_MIN <= bit_timing->tseg1) && (bit_timing->tseg1 <= BTR_NOMINAL_TSEG1_MAX)) &&
       ((BTR_NOMINAL_TSEG2_MIN <= bit_timing->tseg2) && (bit_timing->tseg2 <= BTR_NOMINAL_TSEG2_MAX)) &&
#ifndef _BTR_FREQUENCY_PCANBasic
       ((BTR_FREQUENCY_MIN <= frequency) && (frequency <= BTR_FREQUENCY_MAX)))
#else
       ((BTR_FREQUENCY_80MHz == frequency) ||
        (BTR_FREQUENCY_60MHz == frequency) ||
        (BTR_FREQUENCY_40MHz == frequency) ||
        (BTR_FREQUENCY_30MHz == frequency) ||
        (BTR_FREQUENCY_24MHz == frequency) ||
        (BTR_FREQUENCY_20MHz == frequency)))
#endif
        return calc_bit_rate((bit_timing->brp - 1u), (bit_timing->tseg1 - 1u), (bit_timing->tseg2 - 1u), frequency);
    else
        return 0u;
}

float btr_calc_sample_point_nominal(const struct btr_bit_timing *bit_timing)
{
    if(!bit_timing)
        return 0.0;

    if(((BTR_NOMINAL_TSEG1_MIN <= bit_timing->tseg1) && (bit_timing->tseg1 <= BTR_NOMINAL_TSEG1_MAX)) &&
       ((BTR_NOMINAL_TSEG2_MIN <= bit_timing->tseg2) && (bit_timing->tseg2 <= BTR_NOMINAL_TSEG2_MAX)))
        return calc_sample_point((bit_timing->tseg1 - 1u), (bit_timing->tseg2 - 1u));
    else
        return 0.0;
}

int btr_find_bit_timing_nominal(unsigned long bit_rate, float sample_point, unsigned long frequency, struct btr_bit_timing *bit_timing, unsigned long options)
{
    unsigned short i;

    if(!bit_timing)
        return 0;

    if((sample_point < BTR_NOMINAL_SP_MIN) || (BTR_NOMINAL_SP_MAX < sample_point))
        return 0;
    if((frequency < BTR_FREQUENCY_MIN) || (BTR_FREQUENCY_MAX < frequency))
        return 0;

    if((options & BTR_OPTION_TQ_MASK) == BTR_OPTION_TQ_MANY) {
        for(i = BTR_NOMINAL_BRP_MAX; i >= BTR_NOMINAL_BRP_MIN; i--) {
            bit_timing->brp = i;
            bit_timing->tseg1 = (unsigned long)(((float)frequency * sample_point) / (float)(bit_rate * i)) - 1u;
            bit_timing->tseg2 = (unsigned long)(((float)(1u + bit_timing->tseg1) / sample_point) - (float)(2u + bit_timing->tseg1)) + 1u;
            bit_timing->sjw = BTR_NOMINAL_SJW_MIN;
            bit_timing->sam = BTR_NOMINAL_SAM_MIN;
            if(bit_rate == btr_calc_bit_rate_nominal(bit_timing, frequency))
                return 1;
        }
    }
    else {
        for(i = BTR_NOMINAL_BRP_MIN; i <= BTR_NOMINAL_BRP_MAX; i++) {
            bit_timing->brp = i;
            bit_timing->tseg1 = (unsigned long)(((float)frequency * sample_point) / (float)(bit_rate * i)) - 1u;
            bit_timing->tseg2 = (unsigned long)(((float)(1u + bit_timing->tseg1) / sample_point) - (float)(2u + bit_timing->tseg1)) + 1u;
            bit_timing->sjw = BTR_NOMINAL_SJW_MIN;
            bit_timing->sam = BTR_NOMINAL_SAM_MIN;
            if(bit_rate == btr_calc_bit_rate_nominal(bit_timing, frequency))
                return 1;
        }
    }
    return 0;
}

unsigned long btr_calc_bit_rate_data(const struct btr_bit_timing *bit_timing, unsigned long frequency)
{
    if(!bit_timing)
        return 0;

    if(((BTR_DATA_BRP_MIN <= bit_timing->brp) && (bit_timing->brp <= BTR_DATA_BRP_MAX)) &&
       ((BTR_DATA_TSEG1_MIN <= bit_timing->tseg1) && (bit_timing->tseg1 <= BTR_DATA_TSEG1_MAX)) &&
       ((BTR_DATA_TSEG2_MIN <= bit_timing->tseg2) && (bit_timing->tseg2 <= BTR_DATA_TSEG2_MAX)) &&
#ifndef _BTR_FREQUENCY_PCANBasic
       ((BTR_FREQUENCY_MIN <= frequency) && (frequency <= BTR_FREQUENCY_MAX)))
#else
       ((BTR_FREQUENCY_80MHz == frequency) ||
        (BTR_FREQUENCY_60MHz == frequency) ||
        (BTR_FREQUENCY_40MHz == frequency) ||
        (BTR_FREQUENCY_30MHz == frequency) ||
        (BTR_FREQUENCY_24MHz == frequency) ||
        (BTR_FREQUENCY_20MHz == frequency)))
#endif
        return calc_bit_rate((bit_timing->brp - 1u), (bit_timing->tseg1 - 1u), (bit_timing->tseg2 - 1u), frequency);
    else
        return 0u;
}

float btr_calc_sample_point_data(const struct btr_bit_timing *bit_timing)
{
    if(!bit_timing)
        return 0.0;

    if(((BTR_DATA_TSEG1_MIN <= bit_timing->tseg1) && (bit_timing->tseg1 <= BTR_DATA_TSEG1_MAX)) &&
       ((BTR_DATA_TSEG2_MIN <= bit_timing->tseg2) && (bit_timing->tseg2 <= BTR_DATA_TSEG2_MAX)))
        return calc_sample_point((bit_timing->tseg1 -1u), (bit_timing->tseg2 -1u));
    else
        return 0.0;
}

int btr_find_bit_timing_data(unsigned long bit_rate, float sample_point, unsigned long frequency, struct btr_bit_timing *bit_timing, unsigned long options)
{
    unsigned short i;

    if(!bit_timing)
        return 0;

    if((sample_point < BTR_DATA_SP_MIN) || (BTR_DATA_SP_MAX < sample_point))
        return 0;
    if((frequency < BTR_FREQUENCY_MIN) || (BTR_FREQUENCY_MAX < frequency))
        return 0;

    if((options & BTR_OPTION_TQ_MASK) == BTR_OPTION_TQ_MANY) {
        for(i = BTR_DATA_BRP_MAX; i >= BTR_DATA_BRP_MIN; i--) {
            bit_timing->brp = i;
            bit_timing->tseg1 = (unsigned long)(((float)frequency * sample_point) / (float)(bit_rate * i)) - 1u;
            bit_timing->tseg2 = (unsigned long)(((float)(1u + bit_timing->tseg1) / sample_point) - (float)(2u + bit_timing->tseg1)) + 1u;
            bit_timing->sjw = BTR_DATA_SJW_MIN;
            bit_timing->sam = 0u;
            if(bit_rate == btr_calc_bit_rate_data(bit_timing, frequency))
                return 1;
        }
    }
    else {
        for(i = BTR_DATA_BRP_MIN; i <= BTR_DATA_BRP_MAX; i++) {
            bit_timing->brp = i;
            bit_timing->tseg1 = (unsigned long)(((float)frequency * sample_point) / (float)(bit_rate * i)) - 1u;
            bit_timing->tseg2 = (unsigned long)(((float)(1u + bit_timing->tseg1) / sample_point) - (float)(2u + bit_timing->tseg1)) + 1u;
            bit_timing->sjw = BTR_DATA_SJW_MIN;
            bit_timing->sam = 0u;
            if(bit_rate == btr_calc_bit_rate_data(bit_timing, frequency))
                return 1;
        }
    }
    return 0;
}

int btr_string_to_bit_timing(const char *bit_rate, 
                             unsigned long *frequency, 
                             struct btr_bit_timing *btr_nominal, 
                             struct btr_bit_timing *btr_data)
{
    char str[256], *ptr;
    char *key, *value;
    unsigned long tmp = 0u;
    unsigned long nom_speed = 0;
    unsigned long data_speed = 0;
    float nom_sample_point = 0.0;
    float data_sample_point = 0.0;
    float tmp_float = 0.0;

    if(!bit_rate)
        return 0;
    if(!frequency)
        return 0;
    if(!btr_nominal)
        return 0;
    if(!btr_data)
        return 0;

    *frequency =  0u;
    memset(btr_nominal, 0, sizeof(struct btr_bit_timing));
    memset(btr_data, 0, sizeof(struct btr_bit_timing));

    if(strlen(bit_rate) > 255)
        return 0;
    strncpy(str, bit_rate, 256);
    ptr = str;

    while(*ptr != '\0') {
        tmp = 0;
        tmp_float = 0.0;
        // skip blanks and scan: <key> '='
        if(!(key = skip_blanks(ptr)))
            return 0;
        if(!(ptr = scan_key(key)))
            return 0;
        // skip blanks and scan: <value> [',']
        if(!(value = skip_blanks(ptr)))
            return 0;
        if(!(ptr = scan_value(value)))
            return 0;
        // evaluate <key> '=' <value> [',']
        if(strlen(value) == 0 || strlen(value) > 8)
            return 0;
        // convert <value> = [0-9]+ and less than '99999999'
        if(strchr(value, '.') == NULL)
            tmp = (unsigned long)strtol(value, NULL, 10);
        else
            tmp_float = (float)strtof(value, NULL) / 100.0f;

        // f_clock: (80000000, 60000000, 40000000, 30000000, 24000000, 20000000)
        if(!strcasecmp(key, "f_clock")) {
#ifndef _BTR_FREQUENCY_PCANBasic
            if((BTR_FREQUENCY_MIN <= tmp) && (tmp <= BTR_FREQUENCY_MAX))
                *frequency = (unsigned long)tmp;
            else
                return 0;
#else
            switch(tmp) {
                case 80000000ul: *frequency = 80000000ul; break;
                case 60000000ul: *frequency = 60000000ul; break;
                case 40000000ul: *frequency = 40000000ul; break;
                case 30000000ul: *frequency = 30000000ul; break;
                case 24000000ul: *frequency = 24000000ul; break;
                case 20000000ul: *frequency = 20000000ul; break;
                default: return 0;
            }
#endif
        }
        // f_clock_mhz: (80, 60, 40, 30, 24, 20)
        else if(!strcasecmp(key, "f_clock_mhz")) {
#ifndef _BTR_FREQUENCY_PCANBasic
            if((BTR_FREQUENCY_MHZ_MIN <= tmp) && (tmp <= BTR_FREQUENCY_MHZ_MAX))
                *frequency = (unsigned long)tmp * 1000000ul;
            else
                return 0;
#else
            switch(tmp) {
                case 80ul: *frequency = 80000000ul; break;
                case 60ul: *frequency = 60000000ul; break;
                case 40ul: *frequency = 40000000ul; break;
                case 30ul: *frequency = 30000000ul; break;
                case 24ul: *frequency = 24000000ul; break;
                case 20ul: *frequency = 20000000ul; break;
                default: return 0;
            }
#endif
        }
        // nom_brp: 1..1024
        else if(!strcasecmp(key, "nom_brp")) {
            if((BTR_NOMINAL_BRP_MIN <= tmp) && (tmp <= BTR_NOMINAL_BRP_MAX))
                btr_nominal->brp = (unsigned short)tmp;
            else
                return 0;
        }
        // nom_tseg1: 1..256
        else if(!strcasecmp(key, "nom_tseg1")) {
            if((BTR_NOMINAL_TSEG1_MIN <= tmp) && (tmp <= BTR_NOMINAL_TSEG1_MAX))
                btr_nominal->tseg1 = (unsigned short)tmp;
            else
                return 0;
        }
        // nom_tseg2: 1..128
        else if(!strcasecmp(key, "nom_tseg2")) {
            if((BTR_NOMINAL_TSEG2_MIN <= tmp) && (tmp <= BTR_NOMINAL_TSEG2_MAX))
                btr_nominal->tseg2 = (unsigned short)tmp;
            else
                return 0;
        }
        // nom_sjw: 1..128
        else if(!strcasecmp(key, "nom_sjw")) {
            if((BTR_NOMINAL_SJW_MIN <= tmp) && (tmp <= BTR_NOMINAL_SJW_MAX))
                btr_nominal->sjw = (unsigned short)tmp;
            else
                return 0;
        }
        // nom_sam: (none)
        else if(!strcasecmp(key, "nom_sam")) {
            if((BTR_NOMINAL_SAM_MIN <= tmp) && (tmp <= BTR_NOMINAL_SAM_MAX))
                btr_nominal->sam = (unsigned char)tmp;
            else
                return 0;
        }
        // data_brp: 1..1024
        else if(!strcasecmp(key, "data_brp")) {
            if((BTR_DATA_BRP_MIN <= tmp) && (tmp <= BTR_DATA_BRP_MAX))
                btr_data->brp = (unsigned short)tmp;
            else
                return 0;
        }
        // data_tseg1: 1..32
        else if(!strcasecmp(key, "data_tseg1")) {
            if((BTR_DATA_TSEG1_MIN <= tmp) && (tmp <= BTR_DATA_TSEG1_MAX))
                btr_data->tseg1 = (unsigned short)tmp;
            else
                return 0;
        }
        // data_tseg2: 1..16
        else if(!strcasecmp(key, "data_tseg2")) {
            if((BTR_DATA_TSEG2_MIN <= tmp) && (tmp <= BTR_DATA_TSEG2_MAX))
                btr_data->tseg2 = (unsigned short)tmp;
            else
                return 0;
        }
        // data_sjw: 1..16
        else if(!strcasecmp(key, "data_sjw")) {
            if((BTR_DATA_SJW_MIN <= tmp) && (tmp <= BTR_DATA_SJW_MAX))
                btr_data->sjw = (unsigned short)tmp;
            else
                return 0;
        }
        // data_ssp_offset: (none)
        else if(!strcasecmp(key, "data_ssp_offset")) {
            // not used
        }
        // nom_speed: 1000..2000000
        else if(!strcasecmp(key, "nom_speed")) {
            if(tmp < 1000ul)
                tmp *= 1000ul;
            if((BTR_NOMINAL_SPEED_MIN <= tmp) && (tmp <= BTR_NOMINAL_SPEED_MAX))
                nom_speed = (unsigned long)tmp;
            else
                return 0;
        }
        // nom_sp: 0.05..1.00
        else if(!strcasecmp(key, "nom_sp")) {
            if(tmp_float == 0.0)
                tmp_float = (float)tmp / 100.0f;
            if((BTR_NOMINAL_SP_MIN <= tmp_float) && (tmp_float <= BTR_NOMINAL_SP_MAX))
                nom_sample_point = tmp_float;
            else
                return 0;
        }
        // data_speed: 1000..12000000
        else if(!strcasecmp(key, "data_speed")) {
            if(tmp < 100ul)
                tmp *= 1000000ul;
            if(tmp < 1000ul)
                tmp *= 1000ul;
            if((BTR_DATA_SPEED_MIN <= tmp) && (tmp <= BTR_DATA_SPEED_MAX))
                data_speed = (unsigned long)tmp;
            else
                return 0;
        }
        // data_sp: 0.1..1.0
        else if(!strcasecmp(key, "data_sp")) {
            if(tmp_float == 0.0)
                tmp_float = (float)tmp / 100.0f;
            if((BTR_DATA_SP_MIN <= tmp_float) && (tmp_float <= BTR_DATA_SP_MAX))
                data_sample_point = tmp_float;
            else
                return 0;
        }
        else {
            // unknown key
            return 0;
        }
    }
    if(*frequency) {
        /* f_clock=<freq>,nom_speed=<nom_speed>,nom_sp=<nom_sample_point>[,data_speed=<data_speed>,data_sp=<data_sample_point>] */
        if((nom_speed && (nom_sample_point != 0.0)) &&
           !((btr_nominal->brp || btr_nominal->tseg1 || btr_nominal->tseg2 || btr_nominal->sjw || btr_nominal->sam) ||
             (btr_data->brp || btr_data->tseg1 || btr_data->tseg2 || btr_data->sjw))) {
            if(!btr_find_bit_timing_nominal(nom_speed, nom_sample_point, *frequency, btr_nominal, BTR_OPTION_TQ_FEW))
                return 0;

            if(data_speed && (data_sample_point != 0.0)) {
                if(!btr_find_bit_timing_data(data_speed, data_sample_point, *frequency, btr_data, BTR_OPTION_TQ_FEW))
                    return 0;
            }
            else if(data_speed || (data_sample_point != 0.0))
                return 0;
        }
        else if(nom_speed || (nom_sample_point != 0.0))
            return 0;

        /* f_clock=<freq>,nom_brp=<nom_brp>,nom_tseg1=<nom_tseg1>,nom_tseg2=<nom_tseg2>,nom_sjw=<nom_sjw>[,nom_sam=<nom_sam>]
                        [,data_brp=<data_brp>,data_tseg1=<data_tseg1>,data_tseg2=<data_tseg2>,data_sjw=<data_sjw>] */
        if((btr_nominal->brp && btr_nominal->tseg1 && btr_nominal->tseg2 && btr_nominal->sjw) &&
           ((btr_data->brp && btr_data->tseg1 && btr_data->tseg2 && btr_data->sjw) ||
            (!btr_data->brp && !btr_data->tseg1 && !btr_data->tseg2 && !btr_data->sjw)))
            return 1;
    }
    return 0;
}

/*  -----------  local functions  ---------------------------------------
 */

static unsigned long calc_bit_rate(unsigned long brp, unsigned long tseg1, unsigned long tseg2, unsigned long frequency)
{
    return frequency / ((1u + brp) * (3u + tseg1 + tseg2));
}

static float calc_sample_point(unsigned long tseg1, unsigned long tseg2)
{
    return (float)(2u + tseg1)/(float)(3u + tseg1 + tseg2);
}

static char *scan_key(char *str)
{
    char *ptr = str;

    while(('a' <= *ptr && *ptr <= 'z') || (*ptr == '_') || ('0' <= *ptr && *ptr <= '9') || ('A' <= *ptr && *ptr <= 'Z'))
        ptr++;
    while(*ptr == ' ')
        *ptr++ = '\0';
    if((*ptr != '='))
        return NULL;
    *ptr++ = '\0';
    return ptr;
}

static char *scan_value(char *str)
{
    char *ptr = str;
    int   dot = 0;

    while(('0' <= *ptr && *ptr <= '9') || (*ptr == '.')) {
        if((*ptr == '.') && (++dot > 1))
            return ptr;
        ptr++;
    }
    while(*ptr == ' ')
        *ptr++ = '\0';
    if((*ptr != ',') && (*ptr != '\0'))
        return NULL;
    if(*ptr != '\0')
        *ptr++ = '\0';
    return ptr;
}

static char *skip_blanks(char *str)
{
    char *ptr = str;

    while(*ptr == ' ')
        *ptr++ = '\0';
    return ptr;
}


/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
