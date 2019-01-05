/*	-- $HeadURL$ --
 *
 *	project   :  CAN - Controller Area Network
 *
 *	purpose   :  CAN FD Bit-rates (Converter)
 *
 *	copyright :  (C) 2017, UV Software, Berlin
 *
 *	compiler  :  Microsoft Visual C/C++ Compiler (VS 2017 v141)
 *	             Apple LLVM version 8.1.0 (clang-802.0.42)
 *
 *	export    :  int btr_string_to_bit_timing(const char *bit_rate,
 *	                                          unsigned long *frequency, 
 *	                                          struct btr_bit_timing *btr_nominal, 
 *	                                          struct btr_bit_timing *btr_data);
 *
 *	includes  :  (none)
 *
 *	author    :  Uwe Vogt, UV Software
 *
 *	e-mail    :  uwe.vogt@uv-software.de
 *
 *
 *	-----------  description  --------------------------------------------
 */
/**	@file        bitrates.h
 *
 *	@brief       CAN FD Bit-rates (Converter)
 *
 *	@author      $Author$
 *
 *	@version     $Rev$
 *
 *	@defgroup    bit_rate CAN FD Bit-rates (Converter)
 *	@{
 */
#ifndef BITRATES_H_INCLUDED
#define BITRATES_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  -----------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#define BTR_FREQUENCY_80MHz		(80000000u)
#define BTR_FREQUENCY_60MHz		(60000000u)
#define BTR_FREQUENCY_40MHz		(40000000u)
#define BTR_FREQUENCY_30MHz		(30000000u)
#define BTR_FREQUENCY_24MHz		(24000000u)
#define BTR_FREQUENCY_20MHz		(20000000u)

#define BTR_FREQUENCY_SJA1000	(8000000u)

#define BTR_OPTION_TQ_MASK		(0x00000001u)
#define BTR_OPTION_TQ_MANY		(0x00000001u)
#define BTR_OPTION_TQ_FEW		(0x00000000u)


/*  -----------  types  --------------------------------------------------
 */

/** bit-timing register
 */
struct btr_bit_timing
{
	unsigned long brp;		/**< prescaler: 1..1024 (m_can)          / 1..64 (sja1000) */
	unsigned long tseg1;	/**< tseg1: 1..256 (slow) / 1..32 (fast) / 1..16 (sja1000) */
	unsigned long tseg2;	/**< tseg2: 1..128 (slow) / 1..16 (fast) / 1..8  (sja1000) */
	unsigned long sjw;		/**< swj:   1..128 (slow) / 1..16 (fast) / 1..4  (sja1000) */
	unsigned long sam;		/**< sam:   0..1 (sja1000 only) */
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
unsigned long btr_calc_bit_rate_sja1000(unsigned short btr0btr1);
/** @brief       TBD
 */
float btr_calc_sample_point_sja1000(unsigned short btr0btr1);
/** @brief       TBD
 */
int btr_find_bit_timing_sja1000(unsigned long bit_rate, 
								float sample_point, 
								struct btr_bit_timing *bit_timing);

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


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* BITRATES_H_INCLUDED */
/**	@}
 */
/*	----------------------------------------------------------------------
 *	Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *	Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *	E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
