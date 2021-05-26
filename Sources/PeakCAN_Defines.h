/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (PEAK PCAN Interface)
 *
 *  Copyright (C) 2005-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
 *  along with PCANBasic-Wrapper.  If not, see <http://www.gnu.org/licenses/>.
 */
 /** @addtogroup  can_api
  *  @{
  */
#ifndef CANAPI_PEAKCAN_H_INCLUDED
#define CANAPI_PEAKCAN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  ------------------------------------------------
 */

#include <stdint.h>                     /* C99 header for sized integer types */
#include <stdbool.h>                    /* C99 header for boolean type */


/*  -----------  options  ------------------------------------------------
 */

#define OPTION_PCAN_CiA_BIT_TIMING      /* CiA bit-timing (from CANopen spec.) */


/*  -----------  defines  ------------------------------------------------
 */

/** @name  CAN API Interfaces
 *  @brief PCAN-Basic channel no.
 *  @{ */
#define PCAN_USB1              0x51U    /**< PCAN-USB interface, channel 1 */
#define PCAN_USB2              0x52U    /**< PCAN-USB interface, channel 2 */
#define PCAN_USB3              0x53U    /**< PCAN-USB interface, channel 3 */
#define PCAN_USB4              0x54U    /**< PCAN-USB interface, channel 4 */
#define PCAN_USB5              0x55U    /**< PCAN-USB interface, channel 5 */
#define PCAN_USB6              0x56U    /**< PCAN-USB interface, channel 6 */
#define PCAN_USB7              0x57U    /**< PCAN-USB interface, channel 7 */
#define PCAN_USB8              0x58U    /**< PCAN-USB interface, channel 8 */
#define PCAN_USB9              0x509U   /**< PCAN-USB interface, channel 9 */
#define PCAN_USB10             0x50AU   /**< PCAN-USB interface, channel 10 */
#define PCAN_USB11             0x50BU   /**< PCAN-USB interface, channel 11 */
#define PCAN_USB12             0x50CU   /**< PCAN-USB interface, channel 12 */
#define PCAN_USB13             0x50DU   /**< PCAN-USB interface, channel 13 */
#define PCAN_USB14             0x50EU   /**< PCAN-USB interface, channel 14 */
#define PCAN_USB15             0x50FU   /**< PCAN-USB interface, channel 15 */
#define PCAN_USB16             0x510U   /**< PCAN-USB interface, channel 16 */
#define PCAN_BOARDS              16     /**< number of PCAN interface boards */
/** @} */

/** @name  CAN API Error Codes
 *  @brief PCAN-Basic specific error code
 *  @{ */
#define PCAN_ERR_REGTEST       (-201)   /**< test of the CAN controller hardware registers failed (no hardware found) */
#define PCAN_ERR_NODRIVER      (-202)   /**< driver not loaded */
#define PCAN_ERR_HWINUSE       (-203)   /**< hardware is in use by another Net */
#define PCAN_ERR_NETINUSE      (-204)   /**< a client is already connected to the Net */
#define PCAN_ERR_ILLHW         (-220)   /**< hardware handle is wrong */
#define PCAN_ERR_ILLNET        (-224)   /**< net handle is wrong */
#define PCAN_ERR_ILLCLIENT     (-228)   /**< client handle is wrong */
#define PCAN_ERR_RESOURCE      (-232)   /**< resource (FIFO, Client, timeout) cannot be created */
#define PCAN_ERR_ILLPARAMTYPE  (-264)   /**< invalid parameter */
#define PCAN_ERR_ILLPARAMVAL   (-265)   /**< invalid parameter value */
#define PCAN_ERR_ILLDATA       (-267)   /**< invalid data, function, or action */
#define PCAN_ERR_ILLOPERATION  (-268)   /**< invalid operation [Value was changed from 0x80000 to 0x8000000] */
#define PCAN_ERR_CAUTION       (-289)   /**< an operation was successfully carried out, however, irregularities were registered */
#define PCAN_ERR_ILLBOARD      (-290)   /**< unknown board type, or not supported */

#define PCAN_ERR_OFFSET        (-200)   /**< offset for PCANBasic-specific errors */
#define PCAN_ERR_UNKNOWN       (-299)   /**< unknown error */
/** @} */

/** @name  CAN API Property Value
 *  @brief PCAN-Pasic parameter to be read or written
 *  @{ */
#define PCAN_PROP_DEVICE_ID      0x01U  /**< Device identifier parameter */
#define PCAN_PROP_API_VERSION    0x05U  /**< PCAN-Basic API version parameter */
#define PCAN_PROP_CHANNEL_VERSION 0x06U /**< PCAN device channel version parameter */
#define PCAN_PROP_HARDWARE_NAME  0x0EU  /**< PCAN hardware name parameter */
//#define PCAN_PROP_SERIAL_NUMBER  0x??U  /**< Tbd. */
//#define PCAN_PROP_CLOCK_DOMAIN   0x??U  /**< Tbd. */
// TODO: define more or all parameters
// ...
#define PCAN_MAX_BUFFER_SIZE     256U   /**< max. buffer size for CAN_GetValue/CAN_SetValue */
/** @} */


/** @name  CAN API Library ID
 *  @brief Library ID and dynamic library names
 *  @{ */
#define PCAN_LIB_ID              400      /**< library ID (CAN/COP API V1 compatible) */
#if defined(_WIN32) || defined (_WIN64)
 #define PCAN_LIB_BASIC         "PCANBasic.dll"
 #define PCAN_LIB_WRAPPER       "u3canpcb.dll"
#elif defined(__APPLE__)
 #define PCAN_LIB_BASIC         "libPCBUSB.dylib"
 #define PCAN_LIB_WRAPPER       "libUVCANPCB.dylib"
#else
#error Platform not supported
#endif
/** @} */

/** @name  Miscellaneous
 *  @brief More or less useful stuff
 *  @{ */
#define PCAN_LIB_VENDOR         "PEAK-System Technik GmbH, Darmstadt"
#define PCAN_LIB_WEBSITE        "www.peak-system.com"
#define PCAN_LIB_HAZARD_NOTE    "Do not connect your CAN device to a real CAN network when using this program.\n" \
                                "This can damage your application."
/** @} */

/*  -----------  types  --------------------------------------------------
 */

/** @brief PCAN non-plug'n'play device parameters
  */
typedef struct can_pcan_param_t_ {      /* device parameters: */
    uint8_t  type;                      /**<  operation mode (non-plug'n'play devices) */
    uint32_t port;                      /**<  I/O port address (parallel device) */
    uint16_t irq;                       /**<  interrupt number (parallel device) */
} can_pcan_param_t;
#define _pcan_param  can_pcan_param_t_  /* for compatibility with CAN/COP API V1 */

#ifdef __cplusplus
}
#endif
#endif /* CANAPI_PEAKCAN_H_INCLUDED */
/** @}
 */
 /*  ----------------------------------------------------------------------
  *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
  *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
  *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
  */
