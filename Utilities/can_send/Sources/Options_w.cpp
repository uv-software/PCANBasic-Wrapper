//  SPDX-License-Identifier: GPL-2.0-or-later
//
//  CAN Sender for generic Interfaces (CAN API V3)
//
//  Copyright (c) 2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
#include "Options.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
extern "C" {
#include "dosopt.h"
}
#if defined(_WIN64)
#define PLATFORM  "x64"
#elif defined(_WIN32)
#define PLATFORM  "x86"
#else
#error Platform not supported
#endif
#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
#define USE_BASENAME  0

#define DEFAULT_OP_MODE   CANMODE_DEFAULT
#define DEFAULT_BAUDRATE  CANBTR_INDEX_250K

#define BAUDRATE_STR      0
#define BAUDRATE_CHR      1
#define BITRATE_STR       2
#define BITRATE_CHR       3
#define VERBOSE_STR       4
#define VERBOSE_CHR       5
#define OP_MODE_STR       6
#define OP_MODE_CHR       7
#define OP_RTR_STR        8
#define OP_XTD_STR        9
#define OP_ERR_STR        10
#define OP_ERRFRMS_STR    11
#define OP_MON_STR        12
#define OP_MONITOR_STR    13
#define OP_LSTNONLY_STR   14
#define OP_SHARED_STR     15
#define OP_SHARED_CHR     16
#define STD_CODE_STR      17
#define STD_MASK_CHR      18
#define XTD_CODE_STR      19
#define XTD_MASK_CHR      20
#define TRACEFILE_STR     21
#define TRACEFILE_CHR     22
#define LISTBITRATES_STR  23
#define LISTBOARDS_STR    24
#define LISTBOARDS_CHR    25
#define TESTBOARDS_STR    26
#define TESTBOARDS_CHR    27
#define PROTOCOL_STR      28
#define PROTOCOL_CHR      29
#define JSON_STR          30
#define JSON_CHR          31
#define HELP              32
#define QUESTION_MARK     33
#define ABOUT             34
#define CHARACTER_MJU     35
#define VERSION           36
#define MAX_OPTIONS       37

static char* option[MAX_OPTIONS] = {
    (char*)"BAUDRATE", (char*)"bd",
    (char*)"BITRATE", (char*)"br",
    (char*)"VERBOSE", (char*)"v",
    (char*)"MODE", (char*)"m",
    (char*)"RTR",
    (char*)"XTD",
    (char*)"ERR", (char*)"ERROR-FRAMES",
    (char*)"MON", (char*)"MONITOR", (char*)"LISTEN-ONLY",
    (char*)"SHARED", (char*)"shrd",
    (char*)"CODE", (char*)"MASK",
    (char*)"XTD-CODE", (char*)"XTD-MASK",
    (char*)"TRACE", (char*)"trc",
    (char*)"LIST-BITRATES",
    (char*)"LIST-BOARDS", (char*)"list",
    (char*)"TEST-BOARDS", (char*)"test",
    (char*)"PROTOCOL", (char*)"pr",
#if (OPTION_CANAPI_LIBRARY != 0)
    (char*)"PATH", (char*)"p",
#else
    (char*)"JSON-FILE", (char*)"json",
#endif
    (char*)"HELP", (char*)"?",
    (char*)"ABOUT", (char*)"\xB5",
    (char*)"VERSION"
};
static const char* c_szApplication = CAN_SEND_APPLICATION;
static const char* c_szCopyright = CAN_SEND_COPYRIGHT;
static const char* c_szWarranty = CAN_SEND_WARRANTY;
static const char* c_szLicense = CAN_SEND_LICENSE;
static const char* c_szBasename = CAN_SEND_PROGRAM;
static const char* c_szInterface = "(unknown)";

#if (USE_BASENAME != 0)
static char* basename(char* path);
#endif

SOptions::SOptions() {
    // to have default bus speed from bit-timing index
    (void)CCanDriver::MapIndex2Bitrate(DEFAULT_BAUDRATE, m_Bitrate);
    (void)CCanDriver::MapBitrate2Speed(m_Bitrate, m_BusSpeed);
    // initialization
    m_szBasename = (char*)c_szBasename;
    m_szInterface = (char*)c_szInterface;
#if (OPTION_CANAPI_LIBRARY != 0)
    m_szSearchPath = (char*)NULL;
#else
    m_szJsonFilename = (char*)NULL;
#endif
#if (SERIAL_CAN_SUPPORTED != 0)
    m_u8Protocol = CANSIO_LAWICEL;
#endif
    m_OpMode.byte = DEFAULT_OP_MODE;
    m_Bitrate.index = DEFAULT_BAUDRATE;
    m_bHasDataPhase = false;
    m_bHasNoSamp = false;
    m_StdFilter.m_u32Code = CANACC_CODE_11BIT;
    m_StdFilter.m_u32Mask = CANACC_MASK_11BIT;
    m_XtdFilter.m_u32Code = CANACC_CODE_29BIT;
    m_XtdFilter.m_u32Mask = CANACC_MASK_29BIT;
#if (CAN_TRACE_SUPPORTED != 0)
    m_eTraceMode = SOptions::eTraceOff;
#endif
    m_fListBitrates = false;
    m_fListBoards = false;
    m_fTestBoards = false;
    m_fVerbose = false;
    m_fExit = false;
}

int SOptions::ScanCommanline(int argc, const char* argv[], FILE* err, FILE* out) {
    int optind;
    char* optarg;
    int64_t intarg;

    int argInterface = 0;
    int optBitrate = 0;
    int optVerbose = 0;
    int optMode = 0;
    int optShared = 0;
    int optListenOnly = 0;
    int optErrorFrames = 0;
    int optExtendedFrames = 0;
    int optRemoteFrames = 0;
    int optStdCode = 0;
    int optStdMask = 0;
    int optXtdCode = 0;
    int optXtdMask = 0;
#if (CAN_TRACE_SUPPORTED != 0)
    int optTraceMode = 0;
#endif
    int optListBitrates = 0;
    int optListBoards = 0;
    int optTestBoards = 0;
#if (SERIAL_CAN_SUPPORTED != 0)
    int optProtocol = 0;
#endif
#if (OPTION_CANAPI_LIBRARY != 0)
    int optPath = 0;
#else
    int optJson = 0;
#endif
    // (0) sanity check
    if ((argc <= 0) || (argv == NULL))
        return (-1);
    if ((err == NULL) || (out == NULL))
        return (-1);
    // (1) get basename from command-line
#if (USE_BASENAME != 0)
    m_szBasename = basename((char*)argv[0]);
#endif
    // (2) scan command-line for options
    while ((optind = getOption(argc, (char**)argv, MAX_OPTIONS, option)) != EOF) {
        switch (optind) {
        /* option '--baudrate=<baudrate>' (-b) */
        case BAUDRATE_STR:
        case BAUDRATE_CHR:
            if ((optBitrate++)) {
                fprintf(err, "%s: duplicated option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            switch (intarg) {
            case 0: case 1000: case 1000000: m_Bitrate.index = (int32_t)CANBTR_INDEX_1M; break;
            case 1: case 800:  case 800000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_800K; break;
            case 2: case 500:  case 500000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_500K; break;
            case 3: case 250:  case 250000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_250K; break;
            case 4: case 125:  case 125000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_125K; break;
            case 5: case 100:  case 100000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_100K; break;
            case 6: case 50:   case 50000:   m_Bitrate.index = (int32_t)CANBTR_INDEX_50K; break;
            case 7: case 20:   case 20000:   m_Bitrate.index = (int32_t)CANBTR_INDEX_20K; break;
            case 8: case 10:   case 10000:   m_Bitrate.index = (int32_t)CANBTR_INDEX_10K; break;
            default:                         m_Bitrate.index = (int32_t)-intarg; break;
            }
            CANAPI_Bitrate_t bitrate;  // in order not to overwrite the index
            if (CCanDriver::MapIndex2Bitrate(m_Bitrate.index, bitrate) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(bitrate, m_BusSpeed) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--bitrate=<bit-rate>' as string */
        case BITRATE_STR:
        case BITRATE_CHR:
            if ((optBitrate++)) {
                fprintf(err, "%s: duplicated option /BITRATE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /BITRATE\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapString2Bitrate(optarg, m_Bitrate, m_bHasDataPhase, m_bHasNoSamp) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option /BITRATE\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(m_Bitrate, m_BusSpeed) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option /BITRATE\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--verbose' (-v) */
        case VERBOSE_STR:
        case VERBOSE_CHR:
            if (optVerbose) {
                fprintf(err, "%s: duplicated option /VERBOSE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /VERBOSE\n", m_szBasename);
                return 1;
            }
            m_fVerbose = true;
            break;
#if (OPTION_CANAPI_LIBRARY != 0)
        /* option '--path=<pathname>' (-p) */
        case JSON_STR:
        case JSON_CHR:
            if ((optPath++)) {
                fprintf(err, "%s: duplicated option /PATH\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /PATH\n", m_szBasename);
                return 1;
            }
            m_szSearchPath = optarg;
            break;
#endif
#if (SERIAL_CAN_SUPPORTED != 0)
        /* option '--protocol=(Lawicel|CANable)' (-z) */
        case PROTOCOL_STR:
        case PROTOCOL_CHR:
            if ((optProtocol++)) {
                fprintf(err, "%s: duplicated option /PROTOCOL\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /PROTOCOL\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "LAWICEL") || !strcasecmp(optarg, "default") || !strcasecmp(optarg, "SLCAN"))
                m_u8Protocol = CANSIO_LAWICEL;
            else if (!strcasecmp(optarg, "CANABLE"))
                m_u8Protocol = CANSIO_CANABLE;
            else {
                fprintf(err, "%s: illegal argument for option /PROTOCOL\n", m_szBasename);
                return 1;
            }
            break;
#endif
        /* option '--mode=(2.0|FDF[+BRS])' (-m) */
        case OP_MODE_STR:
        case OP_MODE_CHR:
            if ((optMode++)) {
                fprintf(err, "%s: duplicated option /MODE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /MODE\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "DEFAULT") || !strcasecmp(optarg, "CLASSIC") || !strcasecmp(optarg, "CLASSICAL") ||
                !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                m_OpMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
            else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                m_OpMode.byte |= CANMODE_FDOE;
            else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FDF+BRS") || !strcasecmp(optarg, "FD+BRS"))
                m_OpMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
            else {
                fprintf(err, "%s: illegal argument for option /MODE\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--shared' */
        case OP_SHARED_STR:
        case OP_SHARED_CHR:
            if ((optShared++)) {
                fprintf(err, "%s: duplicated option /SHARED\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /SHARED\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_SHRD;
            break;
        /* option '--listen-only' */
        case OP_MON_STR:
        case OP_MONITOR_STR:
            if ((optListenOnly++)) {
                fprintf(err, "%s: duplicated option /MON\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /MON\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte |= CANMODE_MON;
            else if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte &= ~CANMODE_MON;
            else {
                fprintf(err, "%s: illegal argument for option /MON\n", m_szBasename);
                return 1;
            }
            break;
        case OP_LSTNONLY_STR:
            if ((optListenOnly++)) {
                fprintf(err, "%s: duplicated option /LISTEN-ONLY\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /LISTEN-ONLY\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_MON;
            break;
        /* option '--error-frames' */
        case OP_ERR_STR:
            if ((optErrorFrames++)) {
                fprintf(err, "%s: duplicated option /ERR\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /ERR\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte |= CANMODE_ERR;
            else if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte &= ~CANMODE_ERR;
            else {
                fprintf(err, "%s: illegal argument for option /ERR\n", m_szBasename);
                return 1;
            }
            break;
        case OP_ERRFRMS_STR:
            if ((optErrorFrames++)) {
                fprintf(err, "%s: duplicated option /ERROR-FRAMES\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /ERROR-FRAMES\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_ERR;
            break;
        /* option '--no-extended-frames' */
        case OP_XTD_STR:
            if ((optExtendedFrames++)) {
                fprintf(err, "%s: duplicated option /XTD\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /XTD\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte |= CANMODE_NXTD;
            else if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte &= ~CANMODE_NXTD;
            else {
                fprintf(err, "%s: illegal argument for option /XTD\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--no-remote-frames' */
        case OP_RTR_STR:
            if ((optRemoteFrames++)) {
                fprintf(err, "%s: duplicated option /RTR\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /RTR\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte |= CANMODE_NRTR;
            else if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte &= ~CANMODE_NRTR;
            else {
                fprintf(err, "%s: illegal argument for option /RTR\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--code=<11-bit-code>' */
        case STD_CODE_STR:
            if ((optStdCode++)) {
                fprintf(err, "%s: duplicated option /CODE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /CODE\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /CODE\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_STD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /CODE\n", m_szBasename);
                return 1;
            }
            m_StdFilter.m_u32Code = (uint32_t)intarg;
            break;
        /* option '--mask=<11-bit-mask>' */
        case STD_MASK_CHR:
            if ((optStdMask++)) {
                fprintf(err, "%s: duplicated option /MASK\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /MASK\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /MASK\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_STD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /MASK\n", m_szBasename);
                return 1;
            }
            m_StdFilter.m_u32Mask = (uint32_t)intarg;
            break;
        /* option '--xtd-code=<29-bit-code>' */
        case XTD_CODE_STR:
            if ((optXtdCode++)) {
                fprintf(err, "%s: duplicated option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_XTD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            m_XtdFilter.m_u32Code = (uint32_t)intarg;
            break;
        /* option '--xtd-mask=<29-bit-mask>' */
        case XTD_MASK_CHR:
            if ((optXtdMask++)) {
                fprintf(err, "%s: duplicated option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_XTD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            m_XtdFilter.m_u32Mask = (uint32_t)intarg;
            break;
        /* option '--trace=(ON|OFF)' (-y) */
#if (CAN_TRACE_SUPPORTED != 0)
        case TRACEFILE_STR:
        case TRACEFILE_CHR:
            if (optTraceMode++) {
                fprintf(err, "%s: duplicated option /TRACE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /TRACE\n", m_szBasename);
                return 1;
            }
#if (CAN_TRACE_SUPPORTED == 1)
            if (!strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "NO") || !strcasecmp(optarg, "n") || !strcasecmp(optarg, "0"))
                m_eTraceMode = SOptions::eTraceOff;
            else if (!strcasecmp(optarg, "ON") || !strcasecmp(optarg, "YES") || !strcasecmp(optarg, "y") || !strcasecmp(optarg, "1"))
                m_eTraceMode = SOptions::eTraceVendor;
#else
            if (!strcasecmp(optarg, "BIN") || !strcasecmp(optarg, "BINARY") || !strcasecmp(optarg, "default"))
                m_eTraceMode = SOptions::eTraceBinary;
            else if (!strcasecmp(optarg, "CSV") || !strcasecmp(optarg, "logger") || !strcasecmp(optarg, "log"))
                m_eTraceMode = SOptions::eTraceLogger;
            else if (!strcasecmp(optarg, "TRC") || !strcasecmp(optarg, "vendor"))
                m_eTraceMode = SOptions::eTraceVendor;
#endif
            else {
                fprintf(err, "%s: illegal argument for option /TRACE\n", m_szBasename);
                return 1;
            }
            break;
#endif
        /* option '--list-bitrates[=(2.0|FDF[+BRS])]' */
        case LISTBITRATES_STR:
            if ((optListBitrates++)) {
                fprintf(err, "%s: duplicated option /LIST-BITRATES\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                if ((optMode++)) {
                    fprintf(err, "%s: option /MODE already set\n", m_szBasename);
                    return 1;
                }
                if (!strcasecmp(optarg, "DEFAULT") || !strcasecmp(optarg, "CLASSIC") || !strcasecmp(optarg, "CLASSICAL") ||
                    !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                    m_OpMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
                else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                    m_OpMode.byte |= CANMODE_FDOE;
                else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FDF+BRS") || !strcasecmp(optarg, "FD+BRS"))
                    m_OpMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
                else {
                    fprintf(err, "%s: illegal argument for option /LIST-BITRATES\n", m_szBasename);
                    return 1;
                }
            }
            m_fListBitrates = true;
            m_fExit = true;
            break;
        /* option '--list-boards' (-L) */
        case LISTBOARDS_STR:
        case LISTBOARDS_CHR:
            if ((optListBoards++)) {
                fprintf(err, "%s: duplicated option /LIST-BOARDS\n", m_szBasename);
                return 1;
            }
#if (OPTION_CANAPI_LIBRARY != 0)
            if ((optarg = getOptionParameter()) != NULL) {
                if ((optPath++)) {
                    fprintf(err, "%s: option /PATH already set\n", m_szBasename);
                    return 1;
                }
                m_szSearchPath = optarg;  // option '--list-boards=<pathname>' (-L)
            }
#endif
            m_fListBoards = true;
            m_fExit = true;
            break;
        /* option '--test-boards' (-T) */
        case TESTBOARDS_STR:
        case TESTBOARDS_CHR:
            if ((optTestBoards++)) {
                fprintf(err, "%s: duplicated option /TEST-BOARDS\n", m_szBasename);
                return 1;
            }
#if (OPTION_CANAPI_LIBRARY != 0)
            if ((optarg = getOptionParameter()) != NULL) {
                if ((optPath++)) {
                    fprintf(err, "%s: option /PATH already set\n", m_szBasename);
                    return 1;
                }
                m_szSearchPath = optarg;  // option '--test-boards=<pathname>' (-L)
            }
#endif
            m_fTestBoards = true;
            m_fExit = true;
            break;
#if (OPTION_CANAPI_LIBRARY == 0)
        /* option '--json=<filename>' (-j) */
        case JSON_STR:
        case JSON_CHR:
            if ((optJson++)) {
                fprintf(err, "%s: duplicated option /JSON-FILE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /JSON-FILE\n", m_szBasename);
                return 1;
            }
            m_szJsonFilename = optarg;
            m_fExit = true;
            break;
#endif
        /* option '--help' (-h) */
        case HELP:
        case QUESTION_MARK:
            ShowHelp(out);;
            return 1;
        case ABOUT:
        case VERSION:
        case CHARACTER_MJU:
            ShowVersion(out);;
            return 1;
        default:
            ShowUsage(out);;
            return 1;
        }
    }
    // (3) scan command-line for argument <interface>
    for (int i = 1; i < argc; i++) {
        if (!isOption(argc, (char**)argv, MAX_OPTIONS, option, i)) {
            if ((argInterface++)) {
                fprintf(err, "%s: too many arguments\n", m_szBasename);
                return 1;
            }
            m_szInterface = (char*)argv[i];
        }
    }
    // - check if one and only one <interface> is given
    if (!argInterface && !m_fExit) {
        fprintf(err, "%s: no interface given\n", m_szBasename);
        return 1;
    }
    // (4) check for illegal combinations
#if (CAN_FD_SUPPORTED != 0)
    /* - check bit-timing index (n/a for CAN FD) */
    if (m_OpMode.fdoe && (m_Bitrate.btr.frequency <= CANBTR_INDEX_1M) && !m_fExit) {
        fprintf(err, "%s: illegal combination of options /MODE and /BAUDRATE\n", m_szBasename);
        return 1;
    }
#endif
    return 0;
}

void SOptions::ShowGreetings(FILE* stream) {
    if (!stream)
        return;
    fprintf(stream, "%s\n%s\n\n%s\n\n", c_szApplication, c_szCopyright, c_szWarranty);
}

void SOptions::ShowFarewell(FILE* stream) {
    if (!stream)
        return;
    fprintf(stream, "%s\n", c_szCopyright);
}

void SOptions::ShowVersion(FILE* stream) {
    if (!stream)
        return;
    fprintf(stream, "%s\n%s\n\n%s\n\n", c_szApplication, c_szCopyright, c_szLicense);
    fprintf(stream, "Written by Uwe Vogt, UV Software, Berlin <https://www.uv-software.com/>\n");
}

void SOptions::ShowUsage(FILE* stream, bool args) {
    if(!stream)
        return;
    fprintf(stream, "Usage: %s <interface> [<option>...]\n", m_szBasename);
    fprintf(stream, "Options:\n");
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, "  /Path:<pathname>                    search path for JSON configuration files\n");
#endif
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /Mode:(2.0|FDf[+BRS])               CAN operation mode: CAN CC or CAN FD mode\n");
#else
    fprintf(stream, "  /Mode:2.0                           CAN operation mode: CAN CC\n");
#endif
    fprintf(stream, "  /SHARED                             shared CAN controller access (if supported)\n");
    fprintf(stream, "  /MONitor:(No|Yes) | /LISTEN-ONLY    monitor mode (listen-only mode)\n");
    fprintf(stream, "  /ERR:(No|Yes) | /ERROR-FRAMES       allow reception of error frames\n");
    fprintf(stream, "  /RTR:(Yes|No)                       allow remote frames (RTR frames)\n");
    fprintf(stream, "  /XTD:(Yes|No)                       allow extended frames (29-bit identifier)\n");
    fprintf(stream, "  /CODE:<id>                          acceptance code for 11-bit IDs (default=0x%03lx)\n", CANACC_CODE_11BIT);
    fprintf(stream, "  /MASK:<id>                          acceptance mask for 11-bit IDs (default=0x%03lx)\n", CANACC_MASK_11BIT);
    fprintf(stream, "  /XTD-CODE:<id>                      acceptance code for 29-bit IDs (default=0x%08lx)\n", CANACC_CODE_29BIT);
    fprintf(stream, "  /XTD-MASK:<id>                      acceptance mask for 29-bit IDs (default=0x%08lx)\n", CANACC_MASK_29BIT);
    fprintf(stream, "  /BauDrate:<baudrate>                CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "  /BitRate:<bitrate>                  CAN bit-rate settings (as key/value list)\n");
    fprintf(stream, "  /Verbose                            show detailed bit-rate settings\n");
#if (CAN_TRACE_SUPPORTED != 0)
#if (CAN_TRACE_SUPPORTED == 1)
    fprintf(stream, "  /TRaCe:(ON|OFF)                     write a trace file (default=OFF)\n");
#else
    fprintf(stream, "  /TRaCe:(BIN|CSV|TRC)                write a trace file (default=OFF)\n");
#endif
#endif
#if (SERIAL_CAN_SUPPORTED != 0)
    fprintf(stream, "  /PRotocol:(Lawicel|CANable)         select SLCAN protocol (default=Lawicel)\n");
#endif
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /LIST-BITRATES[:(2.0|FDf[+BRS])]    list standard bit-rate settings and exit\n");
#else
    fprintf(stream, "  /LIST-BITRATES[:2.0]                list standard bit-rate settings and exit\n");
#endif
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, "  /LIST-boards[:<pathname>]           list all supported CAN interfaces and exit\n");
    fprintf(stream, "  /TEST-boards[:<pathname>]           list all available CAN interfaces and exit\n");
#else
    fprintf(stream, "  /LIST-BOARDS | /LIST                list all supported CAN interfaces and exit\n");
    fprintf(stream, "  /TEST-BOARDS | /TEST                list all available CAN interfaces and exit\n");
    fprintf(stream, "  /JSON-file:<filename>               write configuration into JSON file and exit\n");
#endif
    fprintf(stream, "  /HELP | /?                          display this help screen and exit\n");
    fprintf(stream, "  /VERSION                            show version information and exit\n");
    if (args) {
        fprintf(stream, "Arguments:\n");
        fprintf(stream, "  <id>           CAN identifier (11-bit)\n");
        fprintf(stream, "  <interface>    CAN interface board (list all with /LIST)\n");
        fprintf(stream, "  <baudrate>     CAN baud rate index (default=3):\n");
        fprintf(stream, "                 0 = 1000 kbps\n");
        fprintf(stream, "                 1 = 800 kbps\n");
        fprintf(stream, "                 2 = 500 kbps\n");
        fprintf(stream, "                 3 = 250 kbps\n");
        fprintf(stream, "                 4 = 125 kbps\n");
        fprintf(stream, "                 5 = 100 kbps\n");
        fprintf(stream, "                 6 = 50 kbps\n");
        fprintf(stream, "                 7 = 20 kbps\n");
        fprintf(stream, "                 8 = 10 kbps\n");
        fprintf(stream, "  <bitrate>      comma-separated key/value list:\n");
        fprintf(stream, "                 f_clock=<value>      frequency in Hz or\n");
        fprintf(stream, "                 f_clock_mhz=<value>  frequency in MHz\n");
        fprintf(stream, "                 nom_brp=<value>      bit-rate prescaler (nominal)\n");
        fprintf(stream, "                 nom_tseg1=<value>    time segment 1 (nominal)\n");
        fprintf(stream, "                 nom_tseg2=<value>    time segment 2 (nominal)\n");
        fprintf(stream, "                 nom_sjw=<value>      sync. jump width (nominal)\n");
        fprintf(stream, "                 nom_sam=<value>      sampling (only SJA1000)\n");
        fprintf(stream, "                 data_brp=<value>     bit-rate prescaler (FD data)\n");
        fprintf(stream, "                 data_tseg1=<value>   time segment 1 (FD data)\n");
        fprintf(stream, "                 data_tseg2=<value>   time segment 2 (FD data)\n");
        fprintf(stream, "                 data_sjw=<value>     sync. jump width (FD data).\n");
    }
    fprintf(stream, "Syntax:\n");
    fprintf(stream, " <can_frame>:\n");
    fprintf(stream, "  <can_id>#{data}                     for CAN CC data frames\n");
    fprintf(stream, "  <can_id>#R{len}                     for CAN CC remote frames\n");
    //fprintf(stream, "  <can_id>#{data}_{dlc}               for CAN CC data frames with 9..F DLC\n");
    //fprintf(stream, "  <can_id>#R{len}_{dlc}               for CAN CC remote frames with 9..F DLC\n");
    fprintf(stream, "  <can_id>##<flags>{data}             for CAN FD data frames (up to 64 bytes)\n");
    fprintf(stream, " <can_id>:\n");
    fprintf(stream, "  3  ASCII hex. characters for Standard frame format (SFF) or\n");
    fprintf(stream, "  8  ASCII hex. characters for eXtended frame format (EFF)\n");
    fprintf(stream, " {data}:\n");
    fprintf(stream, "  0 .. 8   ASCII hex. values in CAN CC mode (optionally separated by '.') or\n");
    fprintf(stream, "  0 .. 64  ASCII hex. values in CAN FD mode (optionally separated by '.')\n");
    fprintf(stream, " {len}:\n");
    fprintf(stream, "  an optional 0 .. 8 value as RTR frames can contain a valid DLC field\n");
    //fprintf(stream, " _{dlc}:\n");
    //fprintf(stream, "  an optional 9..F data length code value when payload length is 8\n");
    fprintf(stream, " <flags>:\n");
    fprintf(stream, "  one ASCII hex. character which defines CAN FD flags:\n");
    fprintf(stream, "  4 = FDF                             for CAN FD frame format\n");
    fprintf(stream, "  5 = FDF and BRS                     for CAN FD with Bit Rate Switch\n");
    fprintf(stream, "  6 = FDF and ESI                     for CAN FD with Error State Indicator\n");
    fprintf(stream, "  7 = FDF, BRS and ESI                all together now\n");
    fprintf(stream, "Hazard note:\n");
    fprintf(stream, "  If you connect your CAN device to a real CAN network when using this program,\n");
    fprintf(stream, "  you might damage your application.\n");
}

void SOptions::ShowHelp(FILE* stream) {
    ShowGreetings(stream);
    ShowUsage(stream);
}

#if (USE_BASENAME != 0)
/* see man basename(3) */
static char* basename(char* path) {
    static char exe[] = "agimus.exe";
    char* ptr = NULL;
    if (path)
        ptr = strrchr(path, '\\');
    if (ptr)
        ptr++;
    return (ptr ? ptr : exe);
}
#endif
