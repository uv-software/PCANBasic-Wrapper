/*  -- $HeadURL: https://svn.uv-software.net/projects/uv-software/CAN/I386/DRV/trunk/API/misc/bitrates.h $ --
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
 *  export    :  int btr_string_to_bit_timing(const char *bit_rate,
 *                                            unsigned long *frequency, 
 *                                            struct btr_bit_timing *btr_nominal, 
 *                                            struct btr_bit_timing *btr_data);
 *
 *  includes  :  (none)
 *
 *  author    :  Uwe Vogt, UV Software
 *
 *  e-mail    :  uwe.vogt@uv-software.de
 *
 *
 *  -----------  description  --------------------------------------------
 */
/** @file        bitrates.h
 *
 *  @brief       CAN FD Bit-rates (Converter)
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 592 $
 *
 *  @defgroup    bit_rate CAN FD Bit-rates (Converter)
 *  @{
 */
#ifndef BITRATES_H_INCLUDED
#define BITRATES_H_INCLUDED

/*  -----------  includes  -----------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#define BTR_FREQUENCY_80MHz     (80000000u)
#define BTR_FREQUENCY_60MHz     (60000000u)
#define BTR_FREQUENCY_40MHz     (40000000u)
#define BTR_FREQUENCY_30MHz     (30000000u)
#define BTR_FREQUENCY_24MHz     (24000000u)
#define BTR_FREQUENCY_20MHz     (20000000u)

#define BTR_OPTION_TQ_MASK      (0x00000001u)
#define BTR_OPTION_TQ_MANY      (0x00000001u)
#define BTR_OPTION_TQ_FEW       (0x00000000u)

#define BTR_FREQUENCY_SJA1000   (8000000u)

#define BTR_INDEX_1M            (0)
#define BTR_INDEX_800K          (-1)
#define BTR_INDEX_500K          (-2)
#define BTR_INDEX_250K          (-3)
#define BTR_INDEX_125K          (-4)
#define BTR_INDEX_100K          (-5)
#define BTR_INDEX_50K           (-6)
#define BTR_INDEX_20K           (-7)
#define BTR_INDEX_10K           (-8)
#define BTR_INDEX_5K            (-9)


/*  -----------  types  --------------------------------------------------
 */

/** bit-timing register
 */
struct btr_bit_timing
{
    unsigned long brp;      /**< prescaler: 1..1024 (m_can)          / 1..64 (sja1000) */
    unsigned long tseg1;    /**< tseg1: 1..256 (slow) / 1..32 (fast) / 1..16 (sja1000) */
    unsigned long tseg2;    /**< tseg2: 1..128 (slow) / 1..16 (fast) / 1..8  (sja1000) */
    unsigned long sjw;      /**< swj:   1..128 (slow) / 1..16 (fast) / 1..4  (sja1000) */
    unsigned long sam;      /**< sam:   0..1 (sja1000 only) */
};


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

/** @brief       TBD
 */
int btr_string_to_bit_timing(const char *bit_rate,
	                         unsigned long *frequency,
	                         struct btr_bit_timing *btr_nominal,
	                         struct btr_bit_timing *btr_data);


/** @brief       TBD
 */
unsigned long btr_calc_bit_rate_nominal(const struct btr_bit_timing *bit_timing, 
                                        unsigned long frequency);

/** @brief       TBD
 */
float btr_calc_sample_point_nominal(const struct btr_bit_timing *bit_timing);

/** @brief       TBD
 */
int btr_find_bit_timing_nominal(unsigned long bit_rate, float sample_point, 
                                unsigned long frequency, 
                                struct btr_bit_timing *bit_timing, 
                                unsigned long options);

/** @brief       TBD
 */
unsigned long btr_calc_bit_rate_data(const struct btr_bit_timing *bit_timing, 
                                     unsigned long frequency);

/** @brief       TBD
 */
float btr_calc_sample_point_data(const struct btr_bit_timing *bit_timing);

/** @brief       TBD
 */
int btr_find_bit_timing_data(unsigned long bit_rate, 
                             float sample_point, 
                             unsigned long frequency, 
                             struct btr_bit_timing *bit_timing, 
                             unsigned long options);


/** @brief       TBD
 */
int btr_index_to_bit_rate_sja1000(long index, unsigned short *btr0btr1);

/** @brief       TBD
 */
int btr_index_to_bit_timing_sja1000(long index, 
	                                unsigned long *frequency,
	                                struct btr_bit_timing *btr_timing);

/** @brief       TBD
 */
unsigned long btr_calc_bit_rate_sja1000(unsigned short btr0btr1);
 
/** @brief       TBD
 */
float btr_calc_sample_point_sja1000(unsigned short btr0btr1);

/** @brief       TBD
 */
int btr_find_bit_timing_sja1000(unsigned long bit_rate,
	                            float sample_point,
	                            struct btr_bit_timing *bit_timing);

#endif /* BITRATES_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
