//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  CAN Tester for PEAK PCAN Interfaces
//
//  Copyright (c) 2008-2010,2014-2022 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "build_no.h"
#define VERSION_MAJOR    0
#define VERSION_MINOR    4
#define VERSION_PATCH    2
#define VERSION_BUILD    BUILD_NO
#define VERSION_STRING   TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH) " (" TOSTRING(BUILD_NO) ")"
#if defined(_WIN64)
#define PLATFORM        "x64"
#elif defined(_WIN32)
#define PLATFORM        "x86"
#elif defined(__linux__)
#define PLATFORM        "Linux"
#elif defined(__APPLE__)
#define PLATFORM        "macOS"
#else
#error Unsupported architecture
#endif
static const char APPLICATION[] = "CAN Tester for PEAK PCAN Interfaces, Version " VERSION_STRING;
static const char COPYRIGHT[]   = "Copyright (c) 2008-2010,2014-2022 by Uwe Vogt, UV Software, Berlin";
static const char WARRANTY[]    = "This program comes with ABSOLUTELY NO WARRANTY!\n\n" \
                                  "This is free software, and you are welcome to redistribute it\n" \
                                  "under certain conditions; type `/ABOUT' for details.";
static const char LICENSE[]     = "This program is free software: you can redistribute it and/or modify\n" \
                                  "it under the terms of the GNU General Public License as published by\n" \
                                  "the Free Software Foundation, either version 3 of the License, or\n" \
                                  "(at your option) any later version.\n\n" \
                                  "This program is distributed in the hope that it will be useful,\n" \
                                  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
                                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
                                  "GNU General Public License for more details.\n\n" \
                                  "You should have received a copy of the GNU General Public License\n" \
                                  "along with this program.  If not, see <http://www.gnu.org/licenses/>.";
#define basename(x)  "can_test" // FIXME: Where is my `basename' function?

#include "PeakCAN.h"
#include "Timer.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <inttypes.h>

#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#define RxMODE  (0)
#define TxMODE  (1)
#define TxFRAMES  (2)
#define TxRANDOM  (3)

extern "C" {
#include "dosopt.h"
}
#define BAUDRATE_STR    0
#define BAUDRATE_CHR    1
#define BITRATE_STR     2
#define BITRATE_CHR     3
#define VERBOSE_STR     4
#define VERBOSE_CHR     5
#define OP_MODE_STR     6
#define OP_MODE_CHR     7
#define OP_RTR_STR      8
#define OP_XTD_STR      9
#define OP_ERR_STR      10
#define OP_ERRFRMS_STR  11
#define OP_MON_STR      12
#define OP_MONITOR_STR  13
#define OP_LSTNONLY_STR 14
#define SHARED_STR      15
#define SHARED_CHR      16
#define RECEIVE_STR     17
#define RECEIVE_CHR     18
#define NUMBER_STR      19
#define NUMBER_CHR      20
#define STOP_STR        21
#define STOP_CHR        22
#define TRANSMIT_STR    23
#define TRANSMIT_CHR    24
#define FRAMES_STR      25
#define FRAMES_CHR      26
#define RANDOM_STR      27
#define RANDOM_CHR      28
#define CYCLE_STR       29
#define CYCLE_CHR       30
#define USEC_STR        31
#define USEC_CHR        32
#define DLC_STR         33
#define DLC_CHR         34
#define DLC_LEN         35
#define CAN_STR         36
#define CAN_CHR         37
#define CAN_ID          38
#define COB_ID          39
#define LISTBOARDS_STR  40
#define LISTBOARDS_CHR  41
#define TESTBOARDS_STR  42
#define TESTBOARDS_CHR  43
#define HELP            44
#define QUESTION_MARK   45
#define ABOUT           46
#define CHARACTER_MJU   47
#define MAX_OPTIONS     48

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
    (char*)"RECEIVE", (char*)"rx",
    (char*)"NUMBER", (char*)"n",
    (char*)"STOP", (char*)"s",
    (char*)"TRANSMIT", (char*)"tx",
    (char*)"FRAMES", (char*)"fr",
    (char*)"RANDOM", (char*)"rand",
    (char*)"CYCLE", (char*)"c",
    (char*)"USEC", (char*)"u",
    (char*)"DLC", (char*)"d", (char*)"DATA",
    (char*)"CAN-ID", (char*)"id", (char*)"i", (char*)"COP-ID",
    (char*)"LIST-BOARDS", (char*)"list",
    (char*)"TEST-BOARDS", (char*)"test",
    (char*)"HELP", (char*)"?",
    (char*)"ABOUT", (char*)"�"
};

class CCanDriver : public CPeakCAN {
public:
    uint64_t ReceiverTest(bool checkCounter = false, uint64_t expectedNumber = 0U, bool stopOnError = false);
    uint64_t TransmitterTest(time_t duration, CANAPI_OpMode_t opMode, uint32_t id = 0x100U, uint8_t dlc = 0U, uint32_t delay = 0U, uint64_t offset = 0U);
    uint64_t TransmitterTest(uint64_t count, CANAPI_OpMode_t opMode, bool random = false, uint32_t id = 0x100U, uint8_t dlc = 0U, uint32_t delay = 0U, uint64_t offset = 0U);
public:
    static int ListCanDevices(const char *vendor = NULL);
    static int TestCanDevices(CANAPI_OpMode_t opMode, const char *vendor = NULL);
    // list of CAN interface vendors
    static const struct TCanVendor {
        int32_t id;
        char *name;
    } m_CanVendors[];
    // list of CAN interface devices
    static const struct TCanDevice {
        int32_t library;
        int32_t adapter;
        char *name;
    } m_CanDevices[];
};
const CCanDriver::TCanVendor CCanDriver::m_CanVendors[] = {
    {PEAKCAN_LIBRARY_ID, (char *)"PEAK" },
    {EOF, NULL}
};
const CCanDriver::TCanDevice CCanDriver::m_CanDevices[] = {
    {PEAKCAN_LIBRARY_ID, PCAN_USB1, (char *)"PCAN-USB1" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB2, (char *)"PCAN-USB2" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB3, (char *)"PCAN-USB3" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB4, (char *)"PCAN-USB4" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB5, (char *)"PCAN-USB5" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB6, (char *)"PCAN-USB6" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB7, (char *)"PCAN-USB7" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB8, (char *)"PCAN-USB8" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB9, (char *)"PCAN-USB9" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB10, (char *)"PCAN-USB10" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB11, (char *)"PCAN-USB11" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB12, (char *)"PCAN-USB12" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB13, (char *)"PCAN-USB13" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB14, (char *)"PCAN-USB14" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB15, (char *)"PCAN-USB15" },
    {PEAKCAN_LIBRARY_ID, PCAN_USB16, (char *)"PCAN-USB16" },
    {EOF, EOF, NULL}
};

static void sigterm(int signo);
static void usage(FILE *stream, const char *program);
static void version(FILE *stream, const char *program);

static const char *prompt[4] = {"-\b", "/\b", "|\b", "\\\b"};
static volatile int running = 1;

static CCanDriver canDriver = CCanDriver();

// TODO: this code could be made more C++ alike
int main(int argc, const char * argv[]) {
    int i;
    int optind;
    char *optarg;

    int channel = 0, hw = 0;
    int op = 0, rf = 0, xf = 0, ef = 0, lo = 0, sh = 0;
    int baudrate = CANBDR_250; int bd = 0;
    int mode = RxMODE, m = 0;
    int verbose = 0;
    time_t txtime = 0;
    int txframes = 0;
    int id = 0x100; int c = 0;
    int dlc = 8; int d = 0;
    int delay = 0; int t = 0;
    int number = 0; int n = 0;
    int stop_on_error = 0;
    int num_boards = 0;
    int show_version = 0;
    char *device, *firmware, *software;

    CANAPI_Bitrate_t bitrate = {};
    bitrate.index = CANBTR_INDEX_250K;
    CANAPI_OpMode_t opMode = {};
    opMode.byte = CANMODE_DEFAULT;
    CANAPI_Return_t retVal = 0;

    /* default bit-timing */
    CANAPI_BusSpeed_t speed = {};
    (void)CCanDriver::MapIndex2Bitrate(bitrate.index, bitrate);
    (void)CCanDriver::MapBitrate2Speed(bitrate, speed);
    (void)op;

    /* signal handler */
    if ((signal(SIGINT, sigterm) == SIG_ERR) ||
#if !defined(_WIN32) && !defined(_WIN64)
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    /* scan command-line */
    while ((optind = getOption(argc, (char**)argv, MAX_OPTIONS, option)) != EOF) {
        switch (optind) {
        case BAUDRATE_STR:
        case BAUDRATE_CHR:
            if ((bd++)) {
                fprintf(stderr, "%s: duplicated option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &baudrate) != 1) {
                fprintf(stderr, "%s: illegal argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            switch (baudrate) {
            case 1000: case 1000000: bitrate.index = CANBTR_INDEX_1M; break;
            case 800:  case 800000:  bitrate.index = CANBTR_INDEX_800K; break;
            case 500:  case 500000:  bitrate.index = CANBTR_INDEX_500K; break;
            case 250:  case 250000:  bitrate.index = CANBTR_INDEX_250K; break;
            case 125:  case 125000:  bitrate.index = CANBTR_INDEX_125K; break;
            case 100:  case 100000:  bitrate.index = CANBTR_INDEX_100K; break;
            case 50:   case 50000:   bitrate.index = CANBTR_INDEX_50K; break;
            case 20:   case 20000:   bitrate.index = CANBTR_INDEX_20K; break;
            case 10:   case 10000:   bitrate.index = CANBTR_INDEX_10K; break;
            default:                 bitrate.index = -baudrate; break;
            }
            if (CCanDriver::MapIndex2Bitrate(bitrate.index, bitrate) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            break;
        case BITRATE_STR:
        case BITRATE_CHR:
            if ((bd++)) {
                fprintf(stderr, "%s: duplicated option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            if (CCanDriver::MapString2Bitrate(optarg, bitrate) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            break;
        case VERBOSE_STR:
        case VERBOSE_CHR:
            if (verbose) {
                fprintf(stderr, "%s: duplicated option /VERBOSE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /VERBOSE\n", basename(argv[0]));
                return 1;
            }
            verbose = 1;
            break;
        case OP_MODE_STR:
        case OP_MODE_CHR:
            if ((op++)) {
                fprintf(stderr, "%s: duplicated option /MODE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /MODE\n", basename(argv[0]));
                return 1;
            }
            if (!_strcmpi(optarg, "DEFAULT") || !_strcmpi(optarg, "CLASSIC") ||
                !_strcmpi(optarg, "CAN20") || !_strcmpi(optarg, "CAN2.0") || !_strcmpi(optarg, "2.0"))
                opMode.byte |= CANMODE_DEFAULT;
            else if (!_strcmpi(optarg, "CANFD") || !_strcmpi(optarg, "FD") || !_strcmpi(optarg, "FDF"))
                opMode.byte |= CANMODE_FDOE;
            else if (!_strcmpi(optarg, "CANFD+BRS") || !_strcmpi(optarg, "FDF+BRS") || !_strcmpi(optarg, "FD+BRS"))
                opMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
            else {
                fprintf(stderr, "%s: illegal argument for option /MODE\n", basename(argv[0]));
                return 1;
            }
            break;
        case OP_RTR_STR:
            if ((rf++)) {
                fprintf(stderr, "%s: duplicated option /RTR\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /RTR\n", basename(argv[0]));
                return 1;
            }
            if (!_strcmpi(optarg, "NO") || !_strcmpi(optarg, "N") || !_strcmpi(optarg, "OFF") || !_strcmpi(optarg, "0"))
                opMode.byte |= CANMODE_NRTR;
            else if (!_strcmpi(optarg, "YES") || !_strcmpi(optarg, "Y") || !_strcmpi(optarg, "ON") || !_strcmpi(optarg, "1"))
                opMode.byte &= ~CANMODE_NRTR;
            else {
                fprintf(stderr, "%s: illegal argument for option /RTR\n", basename(argv[0]));
                return 1;
            }
            break;
        case OP_XTD_STR:
            if ((xf++)) {
                fprintf(stderr, "%s: duplicated option /XTD\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /XTD\n", basename(argv[0]));
                return 1;
            }
            if (!_strcmpi(optarg, "NO") || !_strcmpi(optarg, "N") || !_strcmpi(optarg, "OFF") || !_strcmpi(optarg, "0"))
                opMode.byte |= CANMODE_NXTD;
            else if (!_strcmpi(optarg, "YES") || !_strcmpi(optarg, "Y") || !_strcmpi(optarg, "ON") || !_strcmpi(optarg, "1"))
                opMode.byte &= ~CANMODE_NXTD;
            else {
                fprintf(stderr, "%s: illegal argument for option /XTD\n", basename(argv[0]));
                return 1;
            }
            break;
        case OP_ERR_STR:
            if ((ef++)) {
                fprintf(stderr, "%s: duplicated option /ERR\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /ERR\n", basename(argv[0]));
                return 1;
            }
            if (!_strcmpi(optarg, "YES") || !_strcmpi(optarg, "Y") || !_strcmpi(optarg, "ON") || !_strcmpi(optarg, "1"))
                opMode.byte |= CANMODE_ERR;
            else if (!_strcmpi(optarg, "NO") || !_strcmpi(optarg, "N") || !_strcmpi(optarg, "OFF") || !_strcmpi(optarg, "0"))
                opMode.byte &= ~CANMODE_ERR;
            else {
                fprintf(stderr, "%s: illegal argument for option /ERR\n", basename(argv[0]));
                return 1;
            }
            break;
        case OP_ERRFRMS_STR:
            if ((ef++)) {
                fprintf(stderr, "%s: duplicated option /ERROR-FRAMES\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /ERROR-FRAMES\n", basename(argv[0]));
                return 1;
            }
            opMode.byte |= CANMODE_ERR;
            break;
        case OP_MON_STR:
        case OP_MONITOR_STR:
            if ((lo++)) {
                fprintf(stderr, "%s: duplicated option /MON\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /MON\n", basename(argv[0]));
                return 1;
            }
            if (!_strcmpi(optarg, "YES") || !_strcmpi(optarg, "Y") || !_strcmpi(optarg, "ON") || !_strcmpi(optarg, "1"))
                opMode.byte |= CANMODE_MON;
            else if (!_strcmpi(optarg, "NO") || !_strcmpi(optarg, "N") || !_strcmpi(optarg, "OFF") || !_strcmpi(optarg, "0"))
                opMode.byte &= ~CANMODE_MON;
            else {
                fprintf(stderr, "%s: illegal argument for option /MON\n", basename(argv[0]));
                return 1;
            }
            break;
        case OP_LSTNONLY_STR:
            if ((lo++)) {
                fprintf(stderr, "%s: duplicated option /LISTEN-ONLY\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /LISTEN-ONLY\n", basename(argv[0]));
                return 1;
            }
            opMode.byte |= CANMODE_MON;
            break;
        case SHARED_STR:
        case SHARED_CHR:
            if ((sh++)) {
                fprintf(stderr, "%s: duplicated option /SHARED\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /SHARED\n", basename(argv[0]));
                return 1;
            }
            opMode.byte |= CANMODE_SHRD;
            break;
        case RECEIVE_STR:
        case RECEIVE_CHR:
            if ((m++)) {
                fprintf(stderr, "%s: duplicated option /RECEIVE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /RECEIVE\n", basename(argv[0]));
                return 1;
            }
            mode = RxMODE;
            break;
        case NUMBER_STR:
        case NUMBER_CHR:
            if (n++) {
                fprintf(stderr, "%s: duplicated option /NUMBER\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /NUMBER\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &number) != 1) {
                fprintf(stderr, "%s: illegal argument for option /NUMBER\n", basename(argv[0]));
                return 1;
            }
            if (number < 0) {
                fprintf(stderr, "%s: illegal argument for option /NUMBER\n", basename(argv[0]));
                return 1;
            }
            break;
        case STOP_STR:
        case STOP_CHR:
            if (stop_on_error) {
                fprintf(stderr, "%s: duplicated option /STOP\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /STOP\n", basename(argv[0]));
                return 1;
            }
            stop_on_error = 1;
            break;
        case TRANSMIT_STR:
        case TRANSMIT_CHR:
            if ((m++)) {
                fprintf(stderr, "%s: duplicated option /TRANSMIT\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /TRANSMIT\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &txtime) != 1) {
                fprintf(stderr, "%s: illegal argument for option /TRANSMIT\n", basename(argv[0]));
                return 1;
            }
            if (txtime < 0) {
                fprintf(stderr, "%s: illegal argument for option /TRANSMIT\n", basename(argv[0]));
                return 1;
            }
            mode = TxMODE;
            break;
        case FRAMES_STR:
        case FRAMES_CHR:
            if ((m++)) {
                fprintf(stderr, "%s: duplicated option /FRAMES\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /FRAMES\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &txframes) != 1) {
                fprintf(stderr, "%s: illegal argument for option /FRAMES\n", basename(argv[0]));
                return 1;
            }
            if (txframes < 0) {
                fprintf(stderr, "%s: illegal argument for option /FRAMES\n", basename(argv[0]));
                return 1;
            }
            mode = TxFRAMES;
            break;
        case RANDOM_STR:
        case RANDOM_CHR:
            if ((m++)) {
                fprintf(stderr, "%s: duplicated option /RANDOM\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /RANDOM\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &txframes) != 1) {
                fprintf(stderr, "%s: illegal argument for option /RANDOM\n", basename(argv[0]));
                return 1;
            }
            if (txframes < 0) {
                fprintf(stderr, "%s: illegal argument for option /RANDOM\n", basename(argv[0]));
                return 1;
            }
            if (!d) /* let the tester generate messages of arbitrary length */
                dlc = 0;
            mode = TxRANDOM;
            break;
        case CYCLE_STR:
        case CYCLE_CHR:
            if ((t++)) {
                fprintf(stderr, "%s: duplicated option /CYCLE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /CYCLE\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &delay) != 1) {
                fprintf(stderr, "%s: illegal argument for option /CYCLE\n", basename(argv[0]));
                return 1;
            }
            if ((delay < 0) || (delay > (LONG_MAX / 1000l))) {
                fprintf(stderr, "%s: illegal argument for option /CYCLE\n", basename(argv[0]));
                return 1;
            }
            delay *= 1000l;
            break;
        case USEC_STR:
        case USEC_CHR:
            if ((t++)) {
                fprintf(stderr, "%s: duplicated option /USEC\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /USEC\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &delay) != 1) {
                fprintf(stderr, "%s: illegal argument for option /USEC\n", basename(argv[0]));
                return 1;
            }
            if (delay < 0) {
                fprintf(stderr, "%s: illegal argument for option /USEC\n", basename(argv[0]));
                return 1;
            }
            break;
        case DLC_STR:
        case DLC_CHR:
        case DLC_LEN:
            if ((d++)) {
                fprintf(stderr, "%s: duplicated option /DLC\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /DLC\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &dlc) != 1) {
                fprintf(stderr, "%s: illegal argument for option /DLC\n", basename(argv[0]));
                return 1;
            }
            if ((dlc < 0) || (CANFD_MAX_LEN < dlc)) {
                fprintf(stderr, "%s: illegal argument for option /DLC\n", basename(argv[0]));
                return 1;
            }
            break;
        case CAN_STR:
        case CAN_CHR:
        case CAN_ID:
        case COB_ID:
            if ((c++)) {
                fprintf(stderr, "%s: duplicated option /CAN-ID\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /CAN-ID\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &id) != 1) {
                fprintf(stderr, "%s: illegal argument for option /CAN-ID\n", basename(argv[0]));
                return 1;
            }
            if ((id < 0x000) || (CAN_MAX_XTD_ID < id)) { // TODO: to be checked with --mode=NXTD
                fprintf(stderr, "%s: illegal argument for option /CAN-ID\n", basename(argv[0]));
                return 1;
            }
            break;
        case LISTBOARDS_STR:
        case LISTBOARDS_CHR:
            fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
            /* list all supported interfaces */
            num_boards = CCanDriver::ListCanDevices(getOptionParameter());
            fprintf(stdout, "Number of supported CAN interfaces=%i\n", num_boards);
            return (num_boards >= 0) ? 0 : 1;
        case TESTBOARDS_STR:
        case TESTBOARDS_CHR:
            fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
            /* list all available interfaces */
            num_boards = CCanDriver::TestCanDevices(opMode, getOptionParameter());
            fprintf(stdout, "Number of present CAN interfaces=%i\n", num_boards);
            return (num_boards >= 0) ? 0 : 1;
        case HELP:
        case QUESTION_MARK:
            usage(stdout, basename(argv[0]));
            return 0;
        case ABOUT:
        case CHARACTER_MJU:
            version(stdout, basename(argv[0]));
            return 0;
        default:
            usage(stderr, basename(argv[0]));
            return 1;
        }
    }
    /* - check if one and only one <interface> is given */
    for (i = 1; i < argc; i++) {
        if (!isOption(argc, (char**)argv, MAX_OPTIONS, option, i)) {
            if ((hw++)) {
                fprintf(stderr, "%s: too many arguments\n", basename(argv[0]));
                return 1;
            }
            for (channel = 0; CCanDriver::m_CanDevices[channel].adapter != EOF; channel++) {
                if (!_stricmp(argv[i], CCanDriver::m_CanDevices[channel].name))
                    break;
            }
            if (CCanDriver::m_CanDevices[channel].adapter == EOF) {
                fprintf(stderr, "%s: illegal argument\n", basename(argv[0]));
                return 1;
            }
        }
    }
    if (!hw || ((CCanDriver::m_CanDevices[channel].adapter == EOF))) {
        fprintf(stderr, "%s: not enough arguments\n", basename(argv[0]));
        return 1;
    }
    /* - check data length length and make CAN FD DLC (0x0..0xF) */
    if (!opMode.fdoe && (dlc > CAN_MAX_LEN)) {
        fprintf(stderr, "%s: illegal combination of options /MODE and /DLC\n", basename(argv[0]));
        return 1;
    }
    else {
        if (dlc > 48) dlc = 0xF;
        else if (dlc > 32) dlc = 0xE;
        else if (dlc > 24) dlc = 0xD;
        else if (dlc > 20) dlc = 0xC;
        else if (dlc > 16) dlc = 0xB;
        else if (dlc > 12) dlc = 0xA;
        else if (dlc > 8) dlc = 0x9;
    }
    /* - check bit-timing index (n/a for CAN FD) */
    if (opMode.fdoe && (bitrate.btr.frequency <= 0)) {
        fprintf(stderr, "%s: illegal combination of options /MODE and /BAUDRATE\n", basename(argv[0]));
        return 1;
    }
    /* - check operation mode flags */
    if ((mode != RxMODE) && opMode.mon) {
        fprintf(stderr, "%s: illegal option /MON:YES alias /LISTEN-ONLY for transmitter test\n", basename(argv[0]));
        return 1;
    }
    if ((mode != RxMODE) && opMode.err) {
        fprintf(stderr, "%s: illegal option /ERR:YES alias /ERROR-FRAMES for transmitter test\n", basename(argv[0]));
        return 1;
    }
    if ((mode != RxMODE) && opMode.nxtd) {
        fprintf(stderr, "%s: illegal option /XTD:NO for transmitter test\n", basename(argv[0]));
        return 1;
    }
    if ((mode != RxMODE) && opMode.nrtr) {
        fprintf(stderr, "%s: illegal option /RTR:NO for transmitter test\n", basename(argv[0]));
        return 1;
    }
    /* CAN Tester for PEAK PCAN interfaces */
    fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);

    /* - show operation mode and bit-rate settings */
    if (verbose) {
        fprintf(stdout, "Op.-mode=%s", (opMode.byte & CANMODE_FDOE) ? "CANFD" : "CAN2.0");
        if ((opMode.byte & CANMODE_BRSE)) fprintf(stdout, "+BRS");
        if ((opMode.byte & CANMODE_NISO)) fprintf(stdout, "+NISO");
        if ((opMode.byte & CANMODE_SHRD)) fprintf(stdout, "+SHRD");
        if ((opMode.byte & CANMODE_NXTD)) fprintf(stdout, "+NXTD");
        if ((opMode.byte & CANMODE_NRTR)) fprintf(stdout, "+NRTR");
        if ((opMode.byte & CANMODE_ERR)) fprintf(stdout, "+ERR");
        if ((opMode.byte & CANMODE_MON)) fprintf(stdout, "+MON");
        fprintf(stdout, " (op_mode=%02Xh)\n", opMode.byte);
        if (bitrate.btr.frequency > 0) {
            fprintf(stdout, "Bit-rate=%.0fkbps@%.1f%%",
                speed.nominal.speed / 1000.,
                speed.nominal.samplepoint * 100.);
            if (speed.data.brse)
                fprintf(stdout, ":%.0fkbps@%.1f%%",
                    speed.data.speed / 1000.,
                    speed.data.samplepoint * 100.);
            fprintf(stdout, " (f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u",
                bitrate.btr.frequency,
                bitrate.btr.nominal.brp,
                bitrate.btr.nominal.tseg1,
                bitrate.btr.nominal.tseg2,
                bitrate.btr.nominal.sjw);
            if (speed.data.brse)
                fprintf(stdout, ",data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                    bitrate.btr.data.brp,
                    bitrate.btr.data.tseg1,
                    bitrate.btr.data.tseg2,
                    bitrate.btr.data.sjw);
            else
                fprintf(stdout, ",nom_sam=%u", bitrate.btr.nominal.sam);
            fprintf(stdout, ")\n\n");
        }
        else {
            fprintf(stdout, "Baudrate=%.0fkbps@%.1f%% (index %i)\n\n",
                             speed.nominal.speed / 1000.,
                             speed.nominal.samplepoint * 100., -bitrate.index);
        }
    }
    /* - initialize interface */
    fprintf(stdout, "Hardware=%s...", CCanDriver::m_CanDevices[channel].name);
    fflush (stdout);
    retVal = canDriver.InitializeChannel(CCanDriver::m_CanDevices[channel].adapter, opMode);
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be initialized (%i)\n", retVal);
        if (retVal == CCanApi::NotSupported)
            fprintf(stderr, " - possibly CAN operating mode %02Xh not supported", opMode.byte);
        fputc('\n', stderr);
        goto finalize;
    }
    fprintf(stdout, "OK!\n");
    /* - start communication */
    if (bitrate.btr.frequency > 0) {
        fprintf(stdout, "Bit-rate=%.0fkbps",
            speed.nominal.speed / 1000.);
        if (speed.data.brse)
            fprintf(stdout, ":%.0fkbps",
                speed.data.speed / 1000.);
        fprintf(stdout, "...");
    }
    else {
        fprintf(stdout, "Baudrate=%skbps...",
            bitrate.index == CANBTR_INDEX_1M   ? "1000" :
            bitrate.index == CANBTR_INDEX_800K ? "800" :
            bitrate.index == CANBTR_INDEX_500K ? "500" :
            bitrate.index == CANBTR_INDEX_250K ? "250" :
            bitrate.index == CANBTR_INDEX_125K ? "125" :
            bitrate.index == CANBTR_INDEX_100K ? "100" :
            bitrate.index == CANBTR_INDEX_50K  ? "50" :
            bitrate.index == CANBTR_INDEX_20K  ? "20" :
            bitrate.index == CANBTR_INDEX_10K  ? "10" : "?");
    }
    fflush(stdout);
    retVal = canDriver.StartController(bitrate);
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be started (%i)\n", retVal);
        goto teardown;
    }
    fprintf(stdout, "OK!\n");
    /* - do your job well: */
    switch (mode) {
    case TxMODE:    /* transmitter test (duration) */
        (void)canDriver.TransmitterTest((time_t)txtime, opMode, (uint32_t)id, (uint8_t)dlc, (uint32_t)delay, (uint64_t)number);
        break;
    case TxFRAMES:  /* transmitter test (frames) */
        (void)canDriver.TransmitterTest((uint64_t)txframes, opMode, false, (uint32_t)id, (uint8_t)dlc, (uint32_t)delay, (uint64_t)number);
        break;
    case TxRANDOM:  /* transmitter test (random) */
        (void)canDriver.TransmitterTest((uint64_t)txframes, opMode, true, (uint32_t)id, (uint8_t)dlc, (uint32_t)delay, (uint64_t)number);
        break;
    default:        /* receiver test (abort with Ctrl+C) */
        (void)canDriver.ReceiverTest((bool)n, (uint64_t)number, (bool)stop_on_error);
        break;
    }
    /* - show interface information */
    if ((device = canDriver.GetHardwareVersion()) != NULL)
        fprintf(stdout, "Hardware: %s\n", device);
    if ((firmware = canDriver.GetFirmwareVersion()) != NULL)
        fprintf(stdout, "Firmware: %s\n", firmware);
    if ((software = CCanDriver::GetVersion()) != NULL)
        fprintf(stdout, "Software: %s\n", software);
teardown:
    /* - teardown the interface*/
    retVal = canDriver.TeardownChannel();
    if (retVal != CCanApi::NoError) {
        fprintf(stderr, "+++ error: CAN Controller could not be reset (%i)\n", retVal);
        goto finalize;
    }
finalize:
    /* So long and farewell! */
    fprintf(stdout, "%s\n", COPYRIGHT);
    return retVal;
}

int CCanDriver::ListCanDevices(const char *vendor) {
    int32_t library = EOF; int n = 0;

    if (vendor != NULL) {
        /* search library ID in the vendor list */
        for (int32_t i = 0; CCanDriver::m_CanVendors[i].id != EOF; i++) {
            if (!strcmp(vendor, CCanDriver::m_CanVendors[i].name)) {
                library = CCanDriver::m_CanVendors[i].id;
                break;
            }
        }
        fprintf(stdout, "Suppored hardware from \"%s\":\n", vendor);
    }
    else
        fprintf(stdout, "Suppored hardware:\n");
    for (int32_t i = 0; CCanDriver::m_CanDevices[i].library != EOF; i++) {
        /* list all boards or from a specific vendor */
        if ((vendor == NULL) || (library == CCanDriver::m_CanDevices[i].library) ||
            !strcmp(vendor, "*")) { // TODO: pattern matching
            fprintf(stdout, "\"%s\" ", CCanDriver::m_CanDevices[i].name);
            /* search vendor name in the vendor list */
            for (int32_t j = 0; CCanDriver::m_CanVendors[j].id != EOF; j++) {
                if (CCanDriver::m_CanDevices[i].library == CCanDriver::m_CanVendors[j].id) {
                    fprintf(stdout, "(VendorName=\"%s\", LibraryId=%" PRIi32 ", AdapterId=%" PRIi32 ")",
                        CCanDriver::m_CanVendors[j].name, CCanDriver::m_CanDevices[i].library, CCanDriver::m_CanDevices[i].adapter);
                    break;
                }
            }
            fprintf(stdout, "\n");
            n++;
        }
    }
    return n;
}

int CCanDriver::TestCanDevices(CANAPI_OpMode_t opMode, const char *vendor) {
    int32_t library = EOF; int n = 0;

    if (vendor != NULL) {
        /* search library ID in the vendor list */
        for (int32_t i = 0; CCanDriver::m_CanVendors[i].id != EOF; i++) {
            if (!strcmp(vendor, CCanDriver::m_CanVendors[i].name)) {
                library = CCanDriver::m_CanVendors[i].id;
                break;
            }
        }
    }
    for (int32_t i = 0; CCanDriver::m_CanDevices[i].library != EOF; i++) {
        /* test all boards or from a specific vendor */
        if ((vendor == NULL) || (library == CCanDriver::m_CanDevices[i].library) ||
            !strcmp(vendor, "*")) { // TODO: pattern matching
            fprintf(stdout, "Hardware=%s...", CCanDriver::m_CanDevices[i].name);
            fflush(stdout);
            EChannelState state;
            CANAPI_Return_t retVal = CCanDriver::ProbeChannel(CCanDriver::m_CanDevices[i].adapter, opMode, state);
            if ((retVal == CCanApi::NoError) || (retVal == CCanApi::IllegalParameter)) {
                CTimer::Delay(333U * CTimer::MSEC);  // to fake probing a hardware
                switch (state) {
                    case CCanApi::ChannelOccupied: fprintf(stdout, "occupied\n"); n++; break;
                    case CCanApi::ChannelAvailable: fprintf(stdout, "available\n"); n++; break;
                    case CCanApi::ChannelNotAvailable: fprintf(stdout, "not available\n"); break;
                    default: fprintf(stdout, "not testable\n"); break;
                }
                if (retVal == CCanApi::IllegalParameter)
                    fprintf(stderr, "+++ warning: CAN operation mode not supported (%02x)\n", opMode.byte);
            } else
                fprintf(stdout, "FAILED!\n");
        }
    }
    return n;
}

uint64_t CCanDriver::TransmitterTest(time_t duration, CANAPI_OpMode_t opMode, uint32_t id, uint8_t dlc, uint32_t delay, uint64_t offset) {
    CANAPI_Message_t message;
    CANAPI_Return_t retVal;

    time_t start = time(NULL);
    uint64_t frames = 0;
    uint64_t errors = 0;
    uint64_t calls = 0;

    fprintf(stderr, "\nPress ^C to abort.\n");
    message.id  = id;
    message.xtd = 0;
    message.rtr = 0;
    message.fdf = opMode.fdoe;
    message.brs = opMode.brse;
    message.dlc = dlc;
    fprintf(stdout, "\nTransmitting message(s)...");
    fflush (stdout);
    while (time(NULL) < (start + duration)) {
        message.data[0] = (uint8_t)((frames + offset) >> 0);
        message.data[1] = (uint8_t)((frames + offset) >> 8);
        message.data[2] = (uint8_t)((frames + offset) >> 16);
        message.data[3] = (uint8_t)((frames + offset) >> 24);
        message.data[4] = (uint8_t)((frames + offset) >> 32);
        message.data[5] = (uint8_t)((frames + offset) >> 40);
        message.data[6] = (uint8_t)((frames + offset) >> 48);
        message.data[7] = (uint8_t)((frames + offset) >> 56);
        memset(&message.data[8], 0, CANFD_MAX_LEN - 8);
        /* transmit message (repeat when busy) */
retry_tx_test:
        calls++;
        retVal = WriteMessage(message);
        if (retVal == CCanApi::NoError)
            fprintf(stderr, "%s", prompt[(frames++ % 4)]);
        else if ((retVal == CCanApi::TransmitterBusy) && running)
            goto retry_tx_test;
        else
            errors++;
        /* pause between two messages, as you please */
        CTimer::Delay(delay * CTimer::USEC);
        if (!running) {
            fprintf(stderr, "\b");
            fprintf(stdout, "STOP!\n\n");
            fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
            fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
            fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
            fprintf(stdout, "Time=%llisec\n\n", time(NULL) - start);
            return frames;
        }
    }
    fprintf(stderr, "\b");
    fprintf(stdout, "OK!\n\n");
    fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
    fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
    fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
    fprintf(stdout, "Time=%llisec\n\n", time(NULL) - start);

    CTimer::Delay(1U * CTimer::SEC);  /* afterburner */
    return frames;
}

uint64_t CCanDriver::TransmitterTest(uint64_t count, CANAPI_OpMode_t opMode, bool random, uint32_t id, uint8_t dlc, uint32_t delay, uint64_t offset) {
    CANAPI_Message_t message;
    CANAPI_Return_t retVal;

    time_t start = time(NULL);
    uint64_t frames = 0;
    uint64_t errors = 0;
    uint64_t calls = 0;

    srand((unsigned int)time(NULL));

    fprintf(stderr, "\nPress ^C to abort.\n");
    message.id  = id;
    message.xtd = 0;
    message.rtr = 0;
    message.fdf = opMode.fdoe;
    message.brs = opMode.brse;
    message.dlc = dlc;
    fprintf(stdout, "\nTransmitting message(s)...");
    fflush (stdout);
    while (frames < count) {
        message.data[0] = (uint8_t)((frames + offset) >> 0);
        message.data[1] = (uint8_t)((frames + offset) >> 8);
        message.data[2] = (uint8_t)((frames + offset) >> 16);
        message.data[3] = (uint8_t)((frames + offset) >> 24);
        message.data[4] = (uint8_t)((frames + offset) >> 32);
        message.data[5] = (uint8_t)((frames + offset) >> 40);
        message.data[6] = (uint8_t)((frames + offset) >> 48);
        message.data[7] = (uint8_t)((frames + offset) >> 56);
        memset(&message.data[8], 0, CANFD_MAX_LEN - 8);
        if (random)
            message.dlc = dlc + (uint8_t)(rand() % ((CANFD_MAX_DLC - dlc) + 1));
        /* transmit message (repeat when busy) */
retry_tx_test:
        calls++;
        retVal = WriteMessage(message);
        if (retVal == CCanApi::NoError)
            fprintf(stderr, "%s", prompt[(frames++ % 4)]);
        else if ((retVal == CCanApi::TransmitterBusy) && running)
            goto retry_tx_test;
        else
            errors++;
        /* pause between two messages, as you please */
        if (random)
            CTimer::Delay(CTimer::USEC * (delay + (uint32_t)(rand() % 54945)));
        else
            CTimer::Delay(CTimer::USEC * delay);
        if (!running) {
            fprintf(stderr, "\b");
            fprintf(stdout, "STOP!\n\n");
            fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
            fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
            fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
            fprintf(stdout, "Time=%llisec\n\n", time(NULL) - start);
            return frames;
        }
    }
    fprintf(stderr, "\b");
    fprintf(stdout, "OK!\n\n");
    fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
    fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
    fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
    fprintf(stdout, "Time=%llisec\n\n", time(NULL) - start);

    CTimer::Delay(1U * CTimer::SEC);  /* afterburner */
    return frames;}

uint64_t CCanDriver::ReceiverTest(bool checkCounter, uint64_t expectedNumber, bool stopOnError) {
    CANAPI_Message_t message;
    CANAPI_Status_t status;
    CANAPI_Return_t retVal;

    time_t start = time(NULL);
    uint64_t frames = 0U;
    uint64_t errors = 0U;
    uint64_t calls = 0U;
    uint64_t data;

    fprintf(stderr, "\nPress ^C to abort.\n");
    fprintf(stdout, "\nReceiving message(s)...");
    fflush (stdout);
    for (;;) {
        retVal = ReadMessage(message);
        if (retVal == CCanApi::NoError) {
            fprintf(stderr, "%s", prompt[(frames++ % 4)]);
            // checking PCBUSB issue #198 (aka. MACCAN-2)
            if (checkCounter) {
                data = 0;
                if (message.dlc > 0)
                    data |= (uint64_t)message.data[0] << 0;
                if (message.dlc > 1)
                    data |= (uint64_t)message.data[1] << 8;
                if (message.dlc > 2)
                    data |= (uint64_t)message.data[2] << 16;
                if (message.dlc > 3)
                    data |= (uint64_t)message.data[3] << 24;
                if (message.dlc > 4)
                    data |= (uint64_t)message.data[4] << 32;
                if (message.dlc > 5)
                    data |= (uint64_t)message.data[5] << 40;
                if (message.dlc > 6)
                    data |= (uint64_t)message.data[6] << 48;
                if (message.dlc > 7)
                    data |= (uint64_t)message.data[7] << 56;
                if (data != expectedNumber) {
                    fprintf(stderr, "\b");
                    fprintf(stdout, "ISSUE#198!\n");
                    fprintf(stderr, "+++ data inconsistent: %" PRIu64 " received / %" PRIu64 " expected\n", data, expectedNumber);
                    retVal = GetStatus(status);
                    if ((retVal == CCanApi::NoError) && ((status.byte & ~CANSTAT_RESET) != 0x00U)) {
                        fprintf(stderr, "    status register:%s%s%s%s%s%s (%02X)\n",
                            (status.bus_off) ? " BO" : "",
                            (status.warning_level) ? " WL" : "",
                            (status.bus_error) ? " BE" : "",
                            (status.transmitter_busy) ? " TP" : "",
                            (status.message_lost) ? " ML" : "",
                            (status.queue_overrun) ? " QUE" : "", status.byte);
                    }
                    if (stopOnError) {
                        fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
                        fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
                        fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
                        fprintf(stdout, "Time=%llisec\n\n", time(NULL) - start);
                        return frames;
                    }
                    else {
                        fprintf(stderr, "Receiving message(s)... ");
                        expectedNumber = data;
                    }
                }
                expectedNumber++;  // depending on DLC received data may wrap around while number is counting up!
            }
        } else if (retVal != CCanApi::ReceiverEmpty)
            errors++;
        calls++;
        if (!running) {
            fprintf(stderr, "\b");
            fprintf(stdout, "OK!\n\n");
            fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
            fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
            fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
            fprintf(stdout, "Time=%llisec\n\n", time(NULL) - start);
            return frames;
        }
    }
}

/** @brief       signal handler to catch Ctrl+C.
 *
 *  @param[in]   signo - signal number (SIGINT, SIGHUP, SIGTERM)
 */
static void sigterm(int signo)
{
    //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
    (void)canDriver.SignalChannel();
    running = 0;
    (void)signo;
}

/** @brief       shows a help screen with all command-line options.
 *
 *  @param[in]   stream  - output stream (e.g. stdout)
 *  @param[in]   program - base name of the program
 */
static void usage(FILE *stream, const char *program)
{
    fprintf(stream, "Usage:\n");
    fprintf(stream, "  %-8s <interface>  [/RECEIVE | /RX]\n", program);
    fprintf(stream, "  %-8s              [/Number=<number> [/Stop]]\n", "");
    fprintf(stream, "  %-8s              [/RTR=(Yes|No)] [/XTD=(Yes|No)]\n", "");
    fprintf(stream, "  %-8s              [/ERR=(No|Yes) | /ERROR-FRAMES]\n", "");
    fprintf(stream, "  %-8s              [/MONitor=(No|Yes) | /LISTEN-ONLY]\n", "");
    fprintf(stream, "  %-8s              [/Mode=(2.0|FDf[+BRS])] [/SHARED] [/Verbose]\n", "");
    fprintf(stream, "  %-8s              [/BauDrate=<baudrate> | /BitRate=<bitrate>]\n", "");
    fprintf(stream, "  %-8s <interface>  (/TRANSMIT=<time> | /TX=<time> |\n", program);
    fprintf(stream, "  %-8s               /FRames=<frames> | /RANDom=<frames>)\n", "");
    fprintf(stream, "  %-8s              [/Cycle=<msec> | /Usec=<usec>] [/can-Id=<can-id>]\n", "");
    fprintf(stream, "  %-8s              [/Dlc=<length>] [/Number=<number>]\n", "");
    fprintf(stream, "  %-8s              [/Mode=(2.0|FDf[+BRS])] [/SHARED] [/Verbose]\n", "");
    fprintf(stream, "  %-8s              [/BauDrate=<baudrate> | /BitRate=<bitrate>]\n", "");
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, "  %-8s (/TEST-BOARDS[=<vendor>] | /TEST[=<vendor>])\n", program);
    fprintf(stream, "  %-8s (/LIST-BOARDS[=<vendor>] | /LIST[=<vendor>])\n", program);
#else
    fprintf(stream, "  %-8s (/TEST-BOARDS | /TEST)\n", program);
    fprintf(stream, "  %-8s (/LIST-BOARDS | /LIST)\n", program);
#endif
    fprintf(stream, "  %-8s (/HELP | /?)\n", program);
    fprintf(stream, "  %-8s (/ABOUT | /�)\n", program);
    fprintf(stream, "Options:\n");
    fprintf(stream, "  <frames>    Send this number of messages (frames) or\n");
    fprintf(stream, "  <time>      send messages for the given time in seconds\n");
    fprintf(stream, "  <msec>      Cycle time in milliseconds (default=0) or \n");
    fprintf(stream, "  <usec>      cycle time in microseconds (default=0)\n");
    fprintf(stream, "  <can-id>    Send with given identifier (default=100h)\n");
    fprintf(stream, "  <length>    Send data of given length (default=8)\n");
    fprintf(stream, "  <number>    Set first up-counting number (default=0)\n");
    fprintf(stream, "  <interface> CAN interface board (list all with /LIST)\n");
    fprintf(stream, "  <baudrate>  CAN baud rate index (default=3):\n");
    fprintf(stream, "              0 = 1000 kbps\n");
    fprintf(stream, "              1 = 800 kbps\n");
    fprintf(stream, "              2 = 500 kbps\n");
    fprintf(stream, "              3 = 250 kbps\n");
    fprintf(stream, "              4 = 125 kbps\n");
    fprintf(stream, "              5 = 100 kbps\n");
    fprintf(stream, "              6 = 50 kbps\n");
    fprintf(stream, "              7 = 20 kbps\n");
    fprintf(stream, "              8 = 10 kbps\n");
    fprintf(stream, "  <bitrate>   Comma-separated <key>=<value>-list:\n");
    fprintf(stream, "              f_clock=<value>      Frequency in Hz or\n");
    fprintf(stream, "              f_clock_mhz=<value>  Frequency in MHz\n");
    fprintf(stream, "              nom_brp=<value>      Bit-rate prescaler (nominal)\n");
    fprintf(stream, "              nom_tseg1=<value>    Time segment 1 (nominal)\n");
    fprintf(stream, "              nom_tseg2=<value>    Time segment 2 (nominal)\n");
    fprintf(stream, "              nom_sjw=<value>      Sync. jump width (nominal)\n");
    fprintf(stream, "              nom_sam=<value>      Sampling (only SJA1000)\n");
    fprintf(stream, "              data_brp=<value>     Bit-rate prescaler (FD data)\n");
    fprintf(stream, "              data_tseg1=<value>   Time segment 1 (FD data)\n");
    fprintf(stream, "              data_tseg2=<value>   Time segment 2 (FD data)\n");
    fprintf(stream, "              data_sjw=<value>     Sync. jump width (FD data).\n");
    fprintf(stream, "Hazard note:\n");
    fprintf(stream, "  If you connect your CAN device to a real CAN network when using this program,\n");
    fprintf(stream, "  you might damage your application.\n");
}

/** @brief       shows version information of the program.
 *
 *  @param[in]   stream  - output stream (e.g. stdout)
 *  @param[in]   program - base name of the program
 */
static void version(FILE *stream, const char *program)
{
    fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, LICENSE);
    (void)program;
    fprintf(stream, "Written by Uwe Vogt, UV Software, Berlin <https://uv-software.com/>\n");
}
