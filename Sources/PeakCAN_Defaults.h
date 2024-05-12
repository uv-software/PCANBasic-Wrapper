/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (for PEAK-System PCAN Interfaces)
 *
 *  Copyright (c) 2005-2012 Uwe Vogt, UV Software, Friedrichshafen
 *  Copyright (c) 2013-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.de.com)
 *  All rights reserved.
 *
 *  This file is part of PCANBasic-Wrapper.
 *
 *  PCANBasic-Wrapper is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v3.0 (or any later version). You can
 *  choose between one of them if you use PCANBasic-Wrapper in whole or in part.
 *
 *  BSD 2-Clause "Simplified" License:
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  PCANBasic-Wrapper IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF PCANBasic-Wrapper, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  PCANBasic-Wrapper is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PCANBasic-Wrapper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PCANBasic-Wrapper.  If not, see <https://www.gnu.org/licenses/>.
 */
/** @addtogroup  can_btr
 *  @{
 */
#ifndef CANBTR_PEAKCAN_H_INCLUDED
#define CANBTR_PEAKCAN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  ------------------------------------------------
 */

#include "CANBTR_Defaults.h"  /* CAN API default bit-rate defines */


/*  -----------  options  ------------------------------------------------
 */

/** @name  Compiler Switches
 *  @brief Options for conditional compilation.
 *  @{ */
/** @note  Set define OPTION_PCAN_BIT_TIMING to a non-zero value to compile
 *         with non CiA bit-timing (e.g. in the build environment).
 */
#ifndef OPTION_DISABLED
#define OPTION_DISABLED  0  /**< if a define is not defined, it is automatically set to 0 */
#endif
#ifndef OPTION_PCAN_BIT_TIMING
#define OPTION_PCAN_BIT_TIMING OPTION_DISABLED
#endif
#if (OPTION_PCAN_BIT_TIMING != OPTION_DISABLED)
#ifdef _MSC_VER
#pragma message ( "Compilation with non CiA bit-timming!" )
#else
#warning Compilation with non CiA bit-timming!
#endif
#endif
/** @} */

/*  -----------  defines  ------------------------------------------------
 */

/** @name  PEAK CAN 2.0 Clock
 *  @brief Frequency of the SJA1000 CAN controller.
 *  @{ */
#define PCAN_CAN_CLOCK  8000000  /**< 8 MHz */
/** @} */

/** @name  PEAK Bit-timing Values
 *  @brief Predefined BTR0BTR1 register values (not compatible to CiA CANopen DS-301 spec.).
 *  @{ */
#define PCAN_1M    0x0014U  /**< 1000 kbps (SP=75.0%, SJW=1) */
#define PCAN_800K  0x0016U  /**<  800 kbps (SP=80.0%, SJW=1) */
#define PCAN_500K  0x001CU  /**<  500 kbps (SP=87.5%, SJW=1) */
#define PCAN_250K  0x011CU  /**<  250 kbps (SP=87.5%, SJW=1) */
#define PCAN_125K  0x031CU  /**<  125 kbps (SP=87.5%, SJW=1) */
#define PCAN_100K  0x432FU  /**<  100 kbps (SP=85.0%, SJW=2) */
#define PCAN_50K   0x472FU  /**<   50 kbps (SP=85.0%, SJW=2) */
#define PCAN_20K   0x532FU  /**<   20 kbps (SP=85.0%, SJW=2) */
#define PCAN_10K   0x672FU  /**<   10 kbps (SP=85.0%, SJW=2) */
#define PCAN_5K    0x7F7FU  /**<    5 kbps (SP=68.0%, SJW=2) */
/** @} */

/** @name  PEAK Bit-rate Settings
 *  @brief Predefined BTR0BTR1 register values as CAN API V3 bit-rate settings.
 *  @{ */
#define PCAN_BR_1M(x)    do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=1; x.btr.nominal.tseg1=5; x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_800K(x)  do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=1; x.btr.nominal.tseg1=7; x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_500K(x)  do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=1; x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_250K(x)  do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=2; x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_125K(x)  do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=4; x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_100K(x)  do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=4; x.btr.nominal.tseg1=16;x.btr.nominal.tseg2=3;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_50K(x)   do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=8; x.btr.nominal.tseg1=16;x.btr.nominal.tseg2=3;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_20K(x)   do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=20;x.btr.nominal.tseg1=16;x.btr.nominal.tseg2=3;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_10K(x)   do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=40;x.btr.nominal.tseg1=16;x.btr.nominal.tseg2=3;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define PCAN_BR_5K(x)    do {x.btr.frequency=PCAN_CAN_CLOCK;x.btr.nominal.brp=64;x.btr.nominal.tseg1=16;x.btr.nominal.tseg2=8;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
/** @} */

/** @name  PEAK CAN Clocks
 *  @brief Default value for CAN 2.0 and CAN FD operation modes.
 *  @{ */
#define PEAKCAN_CLOCK  PCAN_CAN_CLOCK  /**< PEAK CAN 2.0 clock (8 MHz) */
#define PEAKCAN_FD_CLOCK     80000000  /**< PEAK CAN FD clock (80 MHz) */
#define PEAKCAN_CLOCK_80MHz  80000000  /**< 80 MHz CAN clock frequency */
#define PEAKCAN_CLOCK_60MHz  60000000  /**< 60 MHz CAN clock frequency */
#define PEAKCAN_CLOCK_40MHz  40000000  /**< 40 MHz CAN clock frequency */
#define PEAKCAN_CLOCK_30MHz  30000000  /**< 30 MHz CAN clock frequency */
#define PEAKCAN_CLOCK_24MHz  24000000  /**< 24 MHz CAN clock frequency */
#define PEAKCAN_CLOCK_20MHz  20000000  /**< 20 MHz CAN clock frequency */
/** @} */

/** @name  PEAK CAN 2.0 Bit-rate Settings
 *  @brief Default values (not compatible to CiA CANopen DS-301 spec.).
 *  @{ */
#define PEAKCAN_BR_1M(x)    PCAN_BR_1M(x)
#define PEAKCAN_BR_800K(x)  PCAN_BR_800K(x)
#define PEAKCAN_BR_500K(x)  PCAN_BR_500K(x)
#define PEAKCAN_BR_250K(x)  PCAN_BR_250K(x)
#define PEAKCAN_BR_125K(x)  PCAN_BR_125K(x)
#define PEAKCAN_BR_100K(x)  PCAN_BR_100K(x)
#define PEAKCAN_BR_50K(x)   PCAN_BR_50K(x)
#define PEAKCAN_BR_20K(x)   PCAN_BR_20K(x)
#define PEAKCAN_BR_10K(x)   PCAN_BR_10K(x)
#define PEAKCAN_BR_5K(x)    PCAN_BR_5K(x)
/** @} */

#if (OPTION_CAN_2_0_ONLY == 0)
/** @name  PEAK CAN FD Bit-rate Settings w/o Bit-rate Switching
 *  @brief Default values for long frames only (0 to 64 bytes).
 *  @{ */
#define PEAKCAN_FD_BR_1M(x)      do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=31; x.btr.nominal.tseg2=8; x.btr.nominal.sjw=8; } while(0)
#define PEAKCAN_FD_BR_500K(x)    do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=63; x.btr.nominal.tseg2=16;x.btr.nominal.sjw=16;} while(0)
#define PEAKCAN_FD_BR_250K(x)    do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=127;x.btr.nominal.tseg2=32;x.btr.nominal.sjw=32;} while(0)
#define PEAKCAN_FD_BR_125K(x)    do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=255;x.btr.nominal.tseg2=64;x.btr.nominal.sjw=64;} while(0)
/** @} */

/** @name  PEAK CAN FD Bit-rate Settings with Bit-rate Switching
 *  @brief Default values for long and fast frames only (up to 8 Mbps).
 *  @{ */
#define PEAKCAN_FD_BR_1M8M(x)    do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=31; x.btr.nominal.tseg2=8; x.btr.nominal.sjw=8;  x.btr.data.brp=2; x.btr.data.tseg1=3;  x.btr.data.tseg2=1; x.btr.data.sjw=1; } while(0)
#define PEAKCAN_FD_BR_500K4M(x)  do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=63; x.btr.nominal.tseg2=16;x.btr.nominal.sjw=16; x.btr.data.brp=2; x.btr.data.tseg1=7;  x.btr.data.tseg2=2; x.btr.data.sjw=2; } while(0)
#define PEAKCAN_FD_BR_250K2M(x)  do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=127;x.btr.nominal.tseg2=32;x.btr.nominal.sjw=32; x.btr.data.brp=2; x.btr.data.tseg1=15; x.btr.data.tseg2=4; x.btr.data.sjw=4; } while(0)
#define PEAKCAN_FD_BR_125K1M(x)  do {x.btr.frequency=PEAKCAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=255;x.btr.nominal.tseg2=64;x.btr.nominal.sjw=64; x.btr.data.brp=2; x.btr.data.tseg1=31; x.btr.data.tseg2=8; x.btr.data.sjw=8; } while(0)
/** @} */
#endif
#ifdef __cplusplus
}
#endif
#endif /* CANBTR_PEAKCAN_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de, Homepage: https://www.uv-software.de/
 */
