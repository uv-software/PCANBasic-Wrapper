//  SPDX-License-Identifier: GPL-2.0-or-later
//
//  CAN Monitor for PEAK-System PCAN Interfaces (CAN API V3)
//
//  Copyright (c) 2007,2012-2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, see <https://www.gnu.org/licenses/>.
//
#ifndef DRIVER_H_INCLUDED
#define DRIVER_H_INCLUDED

#include "PeakCAN.h"

#if (OPTION_CAN_2_0_ONLY != 0)
#error Compilation with legacy CAN 2.0 frame format!
#else
#define CAN_FD_SUPPORTED    1  // don't touch that dial
#define CAN_TRACE_SUPPORTED 0  // write trace file (1=PCAN)
#endif
#if !defined(__APPLE__)
#define MONITOR_INTERFACE  "PEAK-System PCAN Interfaces"
#else
#define MONITOR_INTERFACE  "PEAK-System PCAN USB Interfaces"
#endif
#define MONITOR_COPYRIGHT  "2007,2012-2025 by Uwe Vogt, UV Software, Berlin"
#if defined(_WIN32) || defined(_WIN64)
#define MONITOR_PLATFORM   "Windows"
#elif defined(__linux__)
#define MONITOR_PLATFORM   "Linux"
#elif defined(__APPLE__)
#define MONITOR_PLATFORM   "Darwin"
#endif
#define MONITOR_ALIASNAME  "PCB:"

#if (OPTION_PCAN_BIT_TIMING == 1)
#define BITRATE_1M(x)    PEAKCAN_BR_1M(x)
#define BITRATE_800K(x)  PEAKCAN_BR_800K(x)
#define BITRATE_500K(x)  PEAKCAN_BR_500K(x)
#define BITRATE_250K(x)  PEAKCAN_BR_250K(x)
#define BITRATE_125K(x)  PEAKCAN_BR_125K(x)
#define BITRATE_100K(x)  PEAKCAN_BR_100K(x)
#define BITRATE_50K(x)   PEAKCAN_BR_50K(x)
#define BITRATE_20K(x)   PEAKCAN_BR_20K(x)
#define BITRATE_10K(x)   PEAKCAN_BR_10K(x)
#define BITRATE_5K(x)    PEAKCAN_BR_5K(x)
#else
#define BITRATE_1M(x)    DEFAULT_CAN_BR_1M(x)
#define BITRATE_800K(x)  DEFAULT_CAN_BR_800K(x)
#define BITRATE_500K(x)  DEFAULT_CAN_BR_500K(x)
#define BITRATE_250K(x)  DEFAULT_CAN_BR_250K(x)
#define BITRATE_125K(x)  DEFAULT_CAN_BR_125K(x)
#define BITRATE_100K(x)  DEFAULT_CAN_BR_100K(x)
#define BITRATE_50K(x)   DEFAULT_CAN_BR_50K(x)
#define BITRATE_20K(x)   DEFAULT_CAN_BR_20K(x)
#define BITRATE_10K(x)   DEFAULT_CAN_BR_10K(x)
#define BITRATE_5K(x)    DEFAULT_CAN_BR_5K(x)
#endif
#if (CAN_FD_SUPPORTED != 0)
#if (OPTION_PCAN_BIT_TIMING == 1)
#define BITRATE_FD_1M(x)      PEAKCAN_FD_BR_1M(x)
#define BITRATE_FD_500K(x)    PEAKCAN_FD_BR_500K(x)
#define BITRATE_FD_250K(x)    PEAKCAN_FD_BR_250K(x)
#define BITRATE_FD_125K(x)    PEAKCAN_FD_BR_125K(x)
#define BITRATE_FD_1M8M(x)    PEAKCAN_FD_BR_1M8M(x)
#define BITRATE_FD_500K4M(x)  PEAKCAN_FD_BR_500K4M(x)
#define BITRATE_FD_250K2M(x)  PEAKCAN_FD_BR_250K2M(x)
#define BITRATE_FD_125K1M(x)  PEAKCAN_FD_BR_125K1M(x)
#else
#define BITRATE_FD_1M(x)      DEFAULT_CAN_FD_BR_1M(x)
#define BITRATE_FD_500K(x)    DEFAULT_CAN_FD_BR_500K(x)
#define BITRATE_FD_250K(x)    DEFAULT_CAN_FD_BR_250K(x)
#define BITRATE_FD_125K(x)    DEFAULT_CAN_FD_BR_125K(x)
#define BITRATE_FD_1M8M(x)    DEFAULT_CAN_FD_BR_1M8M(x)
#define BITRATE_FD_500K4M(x)  DEFAULT_CAN_FD_BR_500K4M(x)
#define BITRATE_FD_250K2M(x)  DEFAULT_CAN_FD_BR_250K2M(x)
#define BITRATE_FD_125K1M(x)  DEFAULT_CAN_FD_BR_125K1M(x)
#endif
#endif

typedef CPeakCAN  CCanDriver;

#endif // DRIVER_H_INCLUDED
