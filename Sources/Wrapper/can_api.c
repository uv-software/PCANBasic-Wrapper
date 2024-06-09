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
/** @addtogroup  can_api
 *  @{
 */
#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#if defined(_WIN64)
#define PLATFORM  "x64"
#elif defined(_WIN32)
#define PLATFORM  "x86"
#else
#error Platform not supported
#endif

/*  -----------  includes  -----------------------------------------------
 */
#include "can_defs.h"
#include "can_api.h"
#include "can_btr.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include "PCANBasic.h"
#else
#include <unistd.h>
#include <sys/select.h>
#if defined(__APPLE__)
#include "PCBUSB.h"
#else
#include "PCANBasic.h"
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*  -----------  options  ------------------------------------------------
 */
#if (OPTION_CAN_2_0_ONLY != 0)
#error Compilation with legacy CAN 2.0 frame format!
#endif
#if (OPTION_CANAPI_PCBUSB_DYLIB != 0) || (OPTION_CANAPI_PCANBASIC_SO != 0)
__attribute__((constructor))
static void _initializer() {
    // default initializer
}
__attribute__((destructor))
static void _finalizer() {
    // default finalizer
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif

/*  -----------  defines  ------------------------------------------------
 */
#ifndef CAN_MAX_HANDLES
#define CAN_MAX_HANDLES         (16)    // maximum number of open handles
#endif
#define INVALID_HANDLE          (-1)
#define IS_HANDLE_VALID(hnd)    ((0 <= (hnd)) && ((hnd) < CAN_MAX_HANDLES))
#define IS_HANDLE_OPENED(hnd)   (can[(hnd)].board != PCAN_NONEBUS)
#define IS_CHANNEL_VALID(ch)    ((0 <= (ch)) && ((ch) <= 0xFFFF))
#ifndef DLC2LEN
#define DLC2LEN(x)              dlc_table[((x) < 16) ? (x) : 15]
#endif
#if (OPTION_PCAN_BIT_TIMING == OPTION_DISABLED)
#undef  PCAN_BAUD_100K
#define PCAN_BAUD_100K          0x441Cu
#undef  PCAN_BAUD_50K
#define PCAN_BAUD_50K           0x491Cu
#undef  PCAN_BAUD_20K
#define PCAN_BAUD_20K           0x581Cu
#undef  PCAN_BAUD_10K
#define PCAN_BAUD_10K           0x711Cu
#endif
#define BTR0BTR1_DEFAULT        PCAN_BAUD_250K
#define BIT_RATE_DEFAULT        "f_clock_mhz=80,nom_brp=20,nom_tseg1=12,nom_tseg2=3,nom_sjw=1," \
                                              "data_brp=4,data_tseg1=7,data_tseg2=2,data_sjw=1"
#define FILTER_STD_CODE         (uint32_t)(0x000)
#define FILTER_STD_MASK         (uint32_t)(0x000)
#define FILTER_XTD_CODE         (uint32_t)(0x00000000)
#define FILTER_XTD_MASK         (uint32_t)(0x00000000)
#define FILTER_STD_XOR_MASK     (uint64_t)(0x00000000000007FF)
#define FILTER_XTD_XOR_MASK     (uint64_t)(0x000000001FFFFFFF)
#define FILTER_STD_VALID_MASK   (uint64_t)(0x000007FF000007FF)
#define FILTER_XTD_VALID_MASK   (uint64_t)(0x1FFFFFFF1FFFFFFF)
#define FILTER_RESET_VALUE      (uint64_t)(0x0000000000000000)
#ifndef SYSERR_OFFSET
#define SYSERR_OFFSET           (-10000)
#endif
#define LIB_ID                  PCAN_LIB_ID
#define LIB_DLLNAME             PCAN_LIB_WRAPPER
#define DEV_VENDOR              PCAN_LIB_VENDOR
#define DEV_DLLNAME             PCAN_LIB_BASIC
#define NUM_CHANNELS            PCAN_BOARDS

/*  -----------  types  --------------------------------------------------
 */
typedef enum {                          // filtering mode:
    FILTER_OFF = 0,                     //   no filtering
    FILTER_STD = 1,                     //   11-bit identifier
    FILTER_XTD = 2                      //   29-bit identifier
}   filtering_t;

typedef struct {                        // message filtering:
    filtering_t mode;                   //   filtering mode
    uint64_t mask;                      //   acceptance mask
}   can_filter_t;

typedef struct {                        // frame counters:
    uint64_t tx;                        //   number of transmitted CAN frames
    uint64_t rx;                        //   number of received CAN frames
    uint64_t err;                       //   number of receiced error frames
}   can_counter_t;

typedef struct {                        // error code capture:
    uint8_t lec;                        //   last error code
    uint8_t rx_err;                     //   receive error counter
    uint8_t tx_err;                     //   transmit error counter
}   can_error_t;

typedef struct {                        // PCAN interface:
    TPCANHandle board;                  //   board hardware channel handle
    BYTE  brd_type;                     //   board type (none PnP hardware)
    DWORD brd_port;                     //   board parameter: I/O port address
    WORD  brd_irq;                      //   board parameter: interrupt number
#if defined(_WIN32) || defined(_WIN64)
    HANDLE event;                       //   event handle for blocking read
#else
    int   fdes;                         //   file descriptor for blocking read
#endif
    can_mode_t mode;                    //   operation mode of the CAN channel
    can_filter_t filter;                //   message filtering settings
    can_status_t status;                //   8-bit status register
    can_error_t error;                  //   error code capture
    can_counter_t counters;             //   statistical counters
}   can_interface_t;

/*  -----------  prototypes  ---------------------------------------------
 */
static void var_init(void);             // initialize all variables
static int all_closed(void);            // check if all handles closed

static int exit_channel(int handle);    // teardown a single channel
static int kill_channel(int handle);    // signal a single channel

static void can_message(const TPCANMsg *pcan_msg, can_message_t *msg);
static void can_message_fd(const TPCANMsgFD *pcan_msg, can_message_t *msg);
static void can_message_sts(can_status_t status, can_error_t error, can_message_t *msg);
static void can_timestamp(TPCANTimestamp timestamp, can_message_t *msg);
static void can_timestamp_fd(TPCANTimestampFD timestamp, can_message_t *msg);

static int pcan_error(TPCANStatus);     // PCAN specific errors
static int pcan_compatibility(void);    // PCAN compatibility check

static TPCANStatus pcan_capability(TPCANHandle board, can_mode_t *capability);
static TPCANStatus pcan_get_filter(int handle, uint64_t *filter, filtering_t mode);
static TPCANStatus pcan_set_filter(int handle, uint64_t filter, filtering_t mode);
static TPCANStatus pcan_reset_filter(int handle);

static int lib_parameter(uint16_t param, void *value, size_t nbyte);
static int drv_parameter(int handle, uint16_t param, void *value, size_t nbyte);

/*  -----------  variables  ----------------------------------------------
 */
#if !defined(__APPLE__)
static const char version[] = "CAN API V3 for PEAK-System PCAN Interfaces, Version " VERSION_STRING;
#else
static const char version[] = "CAN API V3 for PEAK-System PCAN USB Interfaces, Version " VERSION_STRING;
#endif

EXPORT
can_board_t can_boards[NUM_CHANNELS+1] = {  // list of supported CAN channels
    {PCAN_USB1, "PCAN-USB1"},
    {PCAN_USB2, "PCAN-USB2"},
    {PCAN_USB3, "PCAN-USB3"},
    {PCAN_USB4, "PCAN-USB4"},
    {PCAN_USB5, "PCAN-USB5"},
    {PCAN_USB6, "PCAN-USB6"},
    {PCAN_USB7, "PCAN-USB7"},
    {PCAN_USB8, "PCAN-USB8"},
#ifndef __APPLE__
    {PCAN_USB9,  "PCAN-USB9"},
    {PCAN_USB10, "PCAN-USB10"},
    {PCAN_USB11, "PCAN-USB11"},
    {PCAN_USB12, "PCAN-USB12"},
    {PCAN_USB13, "PCAN-USB13"},
    {PCAN_USB14, "PCAN-USB14"},
    {PCAN_USB15, "PCAN-USB15"},
    {PCAN_USB16, "PCAN-USB16"},
#else
    {EOF, NULL},
    {EOF, NULL},
    {EOF, NULL},
    {EOF, NULL},
    {EOF, NULL},
    {EOF, NULL},
    {EOF, NULL},
    {EOF, NULL},
#endif
    {EOF, NULL}
};
static const uint8_t dlc_table[16] = {  // DLC to length
    0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64
};
static can_interface_t can[CAN_MAX_HANDLES];  // interface handles
static int init = 0;                    // initialization flag

/*  -----------  functions  ----------------------------------------------
 */
EXPORT
int can_test(int32_t board, uint8_t mode, const void *param, int *result)
{
    TPCANStatus sts;                    // represents a status
    DWORD condition;                    // channel condition
    can_mode_t capa;                    // channel capability
    int used = 0;                       // own used channel
    int i;                              // loop variable

    if (!IS_CHANNEL_VALID(board)) {     // PCAN handle is of type WORD!
        return pcan_error(PCAN_ERROR_ILLCLIENT);
    }
    if (!init) {                        // if not initialized:
        var_init();                     //   initialize all variables
        init = 1;                       //   set initialization flag
    }
    // get channel condition to check for availability
    if ((sts = CAN_GetValue((TPCANHandle)board, PCAN_CHANNEL_CONDITION,
                           (void*)&condition, sizeof(condition))) != PCAN_ERROR_OK)
        return pcan_error(sts);
    for (i = 0; i < CAN_MAX_HANDLES; i++) {
        if (can[i].board == (TPCANHandle)board) { // me, myself and I!
            condition = PCAN_CHANNEL_OCCUPIED;
            used = 1;
            break;
        }
    }
    // check if the CAN channel is available
    if (result) {
        if ((condition == PCAN_CHANNEL_AVAILABLE) || (condition == PCAN_CHANNEL_PCANVIEW))
            *result = CANBRD_PRESENT;      // CAN channel present
        else if (condition == PCAN_CHANNEL_UNAVAILABLE)
            *result = CANBRD_NOT_PRESENT;  // CAN channel not present
        else if (condition == PCAN_CHANNEL_OCCUPIED)
            *result = CANBRD_OCCUPIED;     // CAN channel present, but occupied
        else
            *result = CANBRD_NOT_TESTABLE; // guess the channel is not testable
    }
    // check given operation mode against the operation capability
    if (((condition == PCAN_CHANNEL_AVAILABLE) || (condition == PCAN_CHANNEL_PCANVIEW)) ||
       (/*(condition == PCAN_CHANNEL_OCCUPIED) ||*/ used)) {   // FIXME: issue TC07_47_9w - returns PCAN_ERROR_INITIALIZE if channel used by another process
        // get operation capability from CAN board
        if ((sts = pcan_capability((TPCANHandle)board, &capa)) != PCAN_ERROR_OK)
            return pcan_error(sts);
        // check given operation mode against the operation capability
        if ((mode & ~capa.byte) != 0)
            return CANERR_ILLPARA;
        if (!(mode & CANMODE_FDOE) && ((mode & CANMODE_BRSE) || (mode & CANMODE_NISO)))
            return CANERR_ILLPARA;
    }
    // when the music is over, turn out the lights
#if (1)
    if (all_closed()) {                 // if no open handle then
        init = 0;                       //   clear initialization flag
    }
#endif
    (void)param;
    return CANERR_NOERROR;
}

EXPORT
int can_init(int32_t board, uint8_t mode, const void *param)
{
    TPCANStatus sts;                    // represents a status
    DWORD value;                        // parameter value
    can_mode_t capa;                    // board capability
    BYTE  type = 0;                     // board type (none PnP hardware)
    DWORD port = 0;                     // board parameter: I/O port address
    WORD  irq = 0;                      // board parameter: interrupt number
    int handle;                         // handle index
    int rc;                             // return value

    if (!IS_CHANNEL_VALID(board)) {     // PCAN handle is of type WORD!
        return pcan_error(PCAN_ERROR_ILLCLIENT);
    }
    if (!init) {                        // if not initialized:
        var_init();                     //   initialize all variables
        init = 1;                       //   set initialization flag
    }
    for (handle = 0; handle < CAN_MAX_HANDLES; handle++) {
        if (can[handle].board == (TPCANHandle)board) // channel already in use
            return CANERR_YETINIT;
    }
    for (handle = 0; handle < CAN_MAX_HANDLES; handle++) {
        if (can[handle].board == PCAN_NONEBUS)  // get an unused handle, if any
            break;
    }
    if (!IS_HANDLE_VALID(handle)) {     // no free handle found
        return CANERR_NOTINIT;
    }
    // check for minimum required library version
    if ((rc = pcan_compatibility()) != PCAN_ERROR_OK)
        return rc;
    // get operation capability from channel and check with given operation mode
    if ((sts = pcan_capability((TPCANHandle)board, &capa)) != PCAN_ERROR_OK)
        return pcan_error(sts);
    if ((mode & ~capa.byte) != 0)
        return CANERR_ILLPARA;
    if (!(mode & CANMODE_FDOE) && ((mode & CANMODE_BRSE) || (mode & CANMODE_NISO)))
        return CANERR_ILLPARA;
#if defined(_WIN32) || defined(_WIN64)
    /* one event handle per channel */
    if ((can[handle].event = CreateEvent( // create an event handle
        NULL,                           //   default security attributes
        FALSE,                          //   auto-reset event
        FALSE,                          //   initial state is nonsignaled
        TEXT("PCANBasic")               //   object name
       )) == NULL) {
        return SYSERR_OFFSET - (int)GetLastError();
    }
#endif
    /* to start the CAN controller initially in reset state, we have switch OFF
     * the receiver and the transmitter and then to call CAN_Initialize[FD]() */
    value = PCAN_PARAMETER_OFF;         // receiver OFF
    if ((sts = CAN_SetValue((TPCANHandle)board, PCAN_RECEIVE_STATUS,
                           (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
        return pcan_error(sts);
    value = PCAN_PARAMETER_ON;          // transmitter OFF
    if ((sts = CAN_SetValue((TPCANHandle)board, PCAN_LISTEN_ONLY,
                           (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        return pcan_error(sts);
    }
    // initialize the CAN controller
    if ((mode & CANMODE_FDOE)) {        // CAN FD operation mode?
        if ((sts = CAN_InitializeFD((TPCANHandle)board, BIT_RATE_DEFAULT)) != PCAN_ERROR_OK)
            return pcan_error(sts);
    }
    else {                              // CAN 2.0 operation mode
        if (param) {                    //   parameter for non-plug'n'play devices
            type =  (BYTE)((struct _pcan_param*)param)->type;
            port = (DWORD)((struct _pcan_param*)param)->port;
            irq  =  (WORD)((struct _pcan_param*)param)->irq;
        }
        if ((sts = CAN_Initialize((TPCANHandle)board, BTR0BTR1_DEFAULT, type, port, irq)) != PCAN_ERROR_OK)
            return pcan_error(sts);
    }
    // store the handle and the operation mode
    can[handle].board = (TPCANHandle)board;  // handle of the CAN channel
    if (param) {                        // non-plug'n'play devices:
        can[handle].brd_type =  (BYTE)((struct _pcan_param*)param)->type;
        can[handle].brd_port = (DWORD)((struct _pcan_param*)param)->port;
        can[handle].brd_irq  =  (WORD)((struct _pcan_param*)param)->irq;
    }
    can[handle].mode.byte = mode;       // store selected operation mode
    can[handle].status.byte = CANSTAT_RESET; // CAN controller not started yet
    return handle;                      // return the handle
}

static int exit_channel(int handle)
{
    TPCANStatus sts;                    // represents a status

    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (!can[handle].status.can_stopped) { // if running then go bus off
        /* note: here we should turn off the receiver and the transmitter,
         *       but after CAN_Uninitialize we are really (bus) OFF! */
        (void)CAN_Reset(can[handle].board);
    }
    if ((sts = CAN_Uninitialize(can[handle].board)) != PCAN_ERROR_OK)
        return pcan_error(sts);

    can[handle].status.byte |= CANSTAT_RESET;  // CAN controller in INIT state
    can[handle].board = PCAN_NONEBUS; // handle can be used again
#if defined(_WIN32) || defined(_WIN64)
    if (can[handle].event != NULL) {  // close event handle, if any
        if (!CloseHandle(can[handle].event))
            return SYSERR_OFFSET - (int)GetLastError();
    }
#endif
    return CANERR_NOERROR;
}

EXPORT
int can_exit(int handle)
{
    int rc;                             // return value
    int i;                              // loop variable

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (handle != CANEXIT_ALL) {        // close a single handle
        if ((rc = exit_channel(handle)) != CANERR_NOERROR)
            return rc;
    }
    else {
        for (i = 0; i < CAN_MAX_HANDLES; i++) {
            (void)exit_channel(i);      // close all open handles
        }
    }
    // when the music is over, turn out the lights
    if (all_closed()) {                 // if no open handle then
        init = 0;                       //   clear initialization flag
    }
    return CANERR_NOERROR;
}

static int kill_channel(int handle)
{
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
#if defined(_WIN32) || defined(_WIN64)
    if (can[handle].event != NULL)
        if (!SetEvent(can[handle].event))  // signal event object
            return SYSERR_OFFSET - (int)GetLastError();
#endif
    return CANERR_NOERROR;
}

EXPORT
int can_kill(int handle)
{
    int rc;                             // return value
    int i;                              // loop variable

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (handle != CANKILL_ALL) {        // signal a single handle
        if ((rc = kill_channel(handle)) != CANERR_NOERROR)
            return rc;
    }
    else {
        for (i = 0; i < CAN_MAX_HANDLES; i++) {
            (void)kill_channel(i);      // signal all open handles
        }
    }
    return CANERR_NOERROR;
}

EXPORT
int can_start(int handle, const can_bitrate_t *bitrate)
{
    TPCANStatus sts;                    // represents a status
    uint16_t btr0btr1 = BTR0BTR1_DEFAULT;  // btr0btr1 value
    char string[PCAN_MAX_BUFFER_SIZE];  // bit-rate string
    DWORD value;                        // parameter value

    strcpy(string, "");                 // empty string

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (bitrate == NULL)                // check for null-pointer
        return CANERR_NULLPTR;
    if (!can[handle].status.can_stopped) // must be stopped
        return CANERR_ONLINE;

    // convert CAN API bit-rate to PCANBasic bit-rate
    if (bitrate->index <= 0) {          // btr0btr1 value from index:
        switch (bitrate->index) {
            case CANBTR_INDEX_1M: btr0btr1 = PCAN_BAUD_1M; break;
            case CANBTR_INDEX_800K: btr0btr1 = PCAN_BAUD_800K; break;
            case CANBTR_INDEX_500K: btr0btr1 = PCAN_BAUD_500K; break;
            case CANBTR_INDEX_250K: btr0btr1 = PCAN_BAUD_250K; break;
            case CANBTR_INDEX_125K: btr0btr1 = PCAN_BAUD_125K; break;
            case CANBTR_INDEX_100K: btr0btr1 = PCAN_BAUD_100K; break;
            case CANBTR_INDEX_50K: btr0btr1 = PCAN_BAUD_50K; break;
            case CANBTR_INDEX_20K: btr0btr1 = PCAN_BAUD_20K; break;
            case CANBTR_INDEX_10K: btr0btr1 = PCAN_BAUD_10K; break;
            default: return CANERR_BAUDRATE;
        }
        if (can[handle].mode.fdoe)      // note: btr0btr1 not allowed in CAN FD
            return CANERR_BAUDRATE;
    }
    else if (!can[handle].mode.fdoe) {  // a btr0btr1 value for CAN 2.0:
        /* note: clock and ranges are checkes by the converter */
        if (btr_bitrate2sja1000(bitrate, &btr0btr1) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
    }
    else {                              // a bit-rate string for CAN FD:
        switch (bitrate->btr.frequency) {
            case BTR_FREQ_80MHz: break;
            case BTR_FREQ_60MHz: break;
            case BTR_FREQ_40MHz: break;
            case BTR_FREQ_30MHz: break;
            case BTR_FREQ_24MHz: break;
            case BTR_FREQ_20MHz: break;
            default: return CANERR_BAUDRATE;
        }
        if (btr_check_bitrate(bitrate, can[handle].mode.fdoe,
                              can[handle].mode.brse) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
        if (btr_bitrate2string(bitrate, can[handle].mode.brse, false,
                               string, PCAN_MAX_BUFFER_SIZE) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
    }
    // start the CAN controller
    /* note: to (re-)start the CAN controller, we have to reinitialize it */
    if ((sts = CAN_Reset(can[handle].board)) != PCAN_ERROR_OK)
        return pcan_error(sts);
    if ((sts = CAN_Uninitialize(can[handle].board)) != PCAN_ERROR_OK)
        return pcan_error(sts);
    /* note: the receiver is automatically switched ON by CAN_Initialize[FD]() */
    if (can[handle].mode.fdoe) {        // CAN FD operation mode?
        if ((sts = CAN_InitializeFD(can[handle].board, string)) != PCAN_ERROR_OK)
            return pcan_error(sts);
    }
    else {                              // CAN 2.0 operation mode!
        if ((sts = CAN_Initialize(can[handle].board, btr0btr1,
                                  can[handle].brd_type, can[handle].brd_port,
                                  can[handle].brd_irq)) != PCAN_ERROR_OK)
            return pcan_error(sts);
    }
#if defined(_WIN32) || defined(_WIN64)
    // set event handle for blocking read
    // TODO: check if we have to create a new event handle
    if ((sts = CAN_SetValue(can[handle].board, PCAN_RECEIVE_EVENT,
                           (void*)&can[handle].event,
                            sizeof(can[handle].event))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(sts);
    }
#endif
    // set listen-only mode if selected
    value = (can[handle].mode.mon) ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    if ((sts = CAN_SetValue(can[handle].board, PCAN_LISTEN_ONLY,
                           (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(sts);
    }
    // enable error frame reception if selected
    value = (can[handle].mode.err) ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    if ((sts = CAN_SetValue(can[handle].board, PCAN_ALLOW_ERROR_FRAMES,
                           (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(sts);
    }
    // set acceptance filter as selected
    switch(can[handle].filter.mode) {
        case FILTER_STD:                // 11-bit identifier
            if ((sts = CAN_SetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_11BIT,
                                   (void*)&can[handle].filter.mask,
                                    sizeof(UINT64))) != PCAN_ERROR_OK) {
                CAN_Uninitialize(can[handle].board);
                return pcan_error(sts);
            }
            break;
        case FILTER_XTD:                // 29-bit identifier
            if ((sts = CAN_SetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_29BIT,
                                   (void*)&can[handle].filter.mask,
                                    sizeof(UINT64))) != PCAN_ERROR_OK) {
                CAN_Uninitialize(can[handle].board);
                return pcan_error(sts);
            }
            break;
        default:                        // no filtering
            value = PCAN_FILTER_OPEN;
            if ((sts = CAN_SetValue(can[handle].board, PCAN_MESSAGE_FILTER,
                                   (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
                CAN_Uninitialize(can[handle].board);
                return pcan_error(sts);
            }
            break;
    }
    // clear old status, errors and counters
    can[handle].status.byte = 0x00u;
    can[handle].error.lec = 0x00u;
    can[handle].error.rx_err = 0u;
    can[handle].error.tx_err = 0u;
    can[handle].counters.tx = 0ull;
    can[handle].counters.rx = 0ull;
    can[handle].counters.err = 0ull;
    // CAN controller started!
    can[handle].status.can_stopped = 0;
    return CANERR_NOERROR;
}

EXPORT
int can_reset(int handle)
{
    TPCANStatus sts;                    // represents a status
    DWORD value;                        // parameter value

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (can[handle].status.can_stopped) // must be running
#if (OPTION_CANAPI_RETVALS == OPTION_DISABLED)
        return CANERR_OFFLINE;
#else
        // note: can_reset shall return CANERR_NOERROR even if
        //       the CAN controller has not been started
        return CANERR_NOERROR;
#endif
    // stop the CAN controller (INIT state)
    /* note: we turn off the receiver and the transmitter to do that! */
    value = PCAN_PARAMETER_OFF;         //   receiver off
    if ((sts = CAN_SetValue(can[handle].board, PCAN_RECEIVE_STATUS,
                           (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
        return pcan_error(sts);
    value = PCAN_PARAMETER_ON;          //   transmitter off
    if ((sts = CAN_SetValue(can[handle].board, PCAN_LISTEN_ONLY,
                           (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
        return pcan_error(sts);
    // CAN controller stopped!
    can[handle].status.can_stopped = 1;
    return CANERR_NOERROR;
}

EXPORT
int can_write(int handle, const can_message_t *msg, uint16_t timeout)
{
    TPCANStatus sts;                    // represents a status
    TPCANMsg can_msg;                   // the message (CAN 2.0)
    TPCANMsgFD can_msg_fd;              // the message (CAN FD)
    (void)timeout;

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (msg == NULL)                    // check for null-pointer
        return CANERR_NULLPTR;
    if (can[handle].status.can_stopped) // must be running
        return CANERR_OFFLINE;

    if (msg->id > (uint32_t)(msg->xtd ? CAN_MAX_XTD_ID : CAN_MAX_STD_ID))
        return CANERR_ILLPARA;          // invalid identifier
    if (msg->xtd && can[handle].mode.nxtd)
        return CANERR_ILLPARA;          // suppress extended frames
    if (msg->rtr && can[handle].mode.nrtr)
        return CANERR_ILLPARA;          // suppress remote frames
    if (msg->fdf && !can[handle].mode.fdoe)
        return CANERR_ILLPARA;          // long frames only with CAN FD
    if (msg->brs && !can[handle].mode.brse)
        return CANERR_ILLPARA;          // fast frames only with CAN FD
    if (msg->brs && !msg->fdf)
        return CANERR_ILLPARA;          // bit-rate switching only with CAN FD
    if (msg->sts)
        return CANERR_ILLPARA;          // error frames cannot be sent

    if (!can[handle].mode.fdoe) {
        if (msg->dlc > CAN_MAX_LEN)     //   data length 0 .. 8
            return CANERR_ILLPARA;
        if (msg->xtd)                   //   29-bit identifier
            can_msg.MSGTYPE = PCAN_MESSAGE_EXTENDED;
        else                            //   11-bit identifier
            can_msg.MSGTYPE = PCAN_MESSAGE_STANDARD;
        if (msg->rtr)                   //   request a message
            can_msg.MSGTYPE |= PCAN_MESSAGE_RTR;
        can_msg.ID = (DWORD)(msg->id);
        can_msg.LEN = (BYTE)(msg->dlc);
        memcpy(can_msg.DATA, msg->data, msg->dlc);
        // CAN 2.0: transmit the message
        sts = CAN_Write(can[handle].board, &can_msg);
    }
    else {
        if (msg->dlc > CANFD_MAX_DLC)   //   data length 0 .. 0Fh
            return CANERR_ILLPARA;
        if (msg->xtd)                   //   29-bit identifier
            can_msg_fd.MSGTYPE = PCAN_MESSAGE_EXTENDED;
        else                            //   11-bit identifier
            can_msg_fd.MSGTYPE = PCAN_MESSAGE_STANDARD;
        if (msg->rtr)                   //   request a message
            can_msg_fd.MSGTYPE |= PCAN_MESSAGE_RTR;
        if (msg->fdf)                   //   CAN FD format
            can_msg_fd.MSGTYPE |= PCAN_MESSAGE_FD;
        if (msg->brs && can[handle].mode.brse) //   bit-rate switching
            can_msg_fd.MSGTYPE |= PCAN_MESSAGE_BRS;
        can_msg_fd.ID = (DWORD)(msg->id);
        can_msg_fd.DLC = (BYTE)(msg->dlc);
        memcpy(can_msg_fd.DATA, msg->data, DLC2LEN(msg->dlc));
        // CAN FD: transmit the message
        sts = CAN_WriteFD(can[handle].board, &can_msg_fd);
    }
    // check for errors
    if (sts != PCAN_ERROR_OK) {
        if ((sts & PCAN_ERROR_QXMTFULL)) {  // transmit queue full?
            can[handle].status.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy
        }
        if ((sts & PCAN_ERROR_XMTFULL)) {  // transmission pending?
            can[handle].status.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy
        }
        return pcan_error(sts);         //   PCAN specific error
    }
    // message transmitted: increment transmit counter
    can[handle].status.transmitter_busy = 0;
    can[handle].counters.tx++;
    return CANERR_NOERROR;
}

EXPORT
int can_read(int handle, can_message_t *msg, uint16_t timeout)
{
    TPCANStatus sts;                    // represents a status
    TPCANMsg can_msg;                   // the message (CAN 2.0)
    TPCANTimestamp timestamp;           // time stamp (CAN 2.0)
    TPCANMsgFD can_msg_fd;              // the message (CAN FD)
    TPCANTimestampFD timestamp_fd;      // time stamp (CAN FD)

    memset(&can_msg, 0, sizeof(TPCANMsg));
    memset(&timestamp, 0, sizeof(TPCANTimestamp));
    memset(&can_msg_fd, 0, sizeof(TPCANMsgFD));
    memset(&timestamp_fd, 0, sizeof(TPCANTimestampFD));

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (msg == NULL)                    // check for null-pointer
        return CANERR_NULLPTR;
    if (can[handle].status.can_stopped) // must be running
        return CANERR_OFFLINE;

    memset(msg, 0, sizeof(can_message_t));
    msg->id = 0xFFFFFFFFu;
    msg->sts = 1;
repeat:
    // try to read a message
    if (!can[handle].mode.fdoe)
        sts = CAN_Read(can[handle].board, &can_msg, &timestamp);
    else
        sts = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
    if (sts == PCAN_ERROR_QRCVEMPTY) {
#if defined(_WIN32) || defined(_WIN64)
        // blocking read or polling
        if (timeout > 0) {
            switch (WaitForSingleObject(can[handle].event,
                                      (timeout != CANREAD_INFINITE) ? (DWORD)timeout : INFINITE)) {
            case WAIT_OBJECT_0:
                break;                  //   one or more messages received
            case WAIT_TIMEOUT:
                break;                  //   time-out, but look for old messages
            default:
                return CANERR_FATAL;    //   function failed!
            }
            if (!can[handle].mode.fdoe)
                sts = CAN_Read(can[handle].board, &can_msg, &timestamp);
            else
                sts = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
            if (sts == PCAN_ERROR_QRCVEMPTY) {
                can[handle].status.receiver_empty = 1;
                return CANERR_RX_EMPTY; //   receiver empty
            }
        }
        else {
            can[handle].status.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty
        }
#endif
    }
    // check for errors
    if ((sts & PCAN_ERROR_OVERRUN)) {
        can[handle].status.message_lost = 1;
        /* note: at least one message got lost, but we have a message */
    }
    if ((sts & PCAN_ERROR_QOVERRUN)) {
        can[handle].status.queue_overrun = 1;
        /* note: queue has overrun, but we have a message */
    }
    if ((sts & PCAN_ERROR_QRCVEMPTY)) {  // receice queue empty?
        can[handle].status.receiver_empty = 1;
        if ((sts & 0xFF00u))  // TODO: explain this
            return pcan_error(sts);      //   something went wrong
        else
            return CANERR_RX_EMPTY;     //   receiver empty!
    }
    // convert PCAN message to CAN API message
    // TODO: move this into separate functions if possible
    if (!can[handle].mode.fdoe) {       // CAN 2.0 message:
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) && can[handle].mode.nxtd)
            goto repeat;                //   refuse extended frames
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_RTR) && can[handle].mode.nrtr)
            goto repeat;                //   refuse remote frames
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            // update status register from status frame
            can[handle].status.bus_off = ((can_msg.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK) ? 1 : 0;
            can[handle].status.warning_level = ((can_msg.DATA[3] & PCAN_ERROR_BUSHEAVY) != PCAN_ERROR_OK) ? 1 : 0;
            // refuse status message if suppressed by user
            if (!can[handle].mode.err)
                goto repeat;
            // status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors)
            can_message_sts(can[handle].status, can[handle].error, msg);
            can[handle].counters.err++;
        }
        else if ((can_msg.MSGTYPE & PCAN_MESSAGE_ERRFRAME))  {
            // update error and status register from error frame
            can[handle].error.lec = (uint8_t)can_msg.ID;
            can[handle].error.rx_err = can_msg.DATA[2];
            can[handle].error.tx_err = can_msg.DATA[3];
            can[handle].status.bus_error = can[handle].error.lec ? 1 : 0;
            // refuse status message if suppressed by user
            if (!can[handle].mode.err)
                goto repeat;
            // status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors)
            can_message_sts(can[handle].status, can[handle].error, msg);
            can[handle].counters.err++;
        }
        else {
            // decode PEAK CAN 2.0 message and increment receive counter
            can_message(&can_msg, msg);
            can[handle].counters.rx++;
        }
        // time-stamp in nanoseconds since start of Windows
        can_timestamp(timestamp, msg);
    }
    else {                              // CAN FD message:
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_EXTENDED) && can[handle].mode.nxtd)
            goto repeat;                //   refuse extended frames
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_RTR) && can[handle].mode.nrtr)
            goto repeat;                //   refuse remote frames (n/a w/ fdoe)
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            // update status register from status frame
            can[handle].status.bus_off = ((can_msg_fd.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK) ? 1 : 0;
            can[handle].status.warning_level = ((can_msg_fd.DATA[3] & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK) ? 1 : 0;
            // refuse status message if suppressed by user
            if (!can[handle].mode.err)
                goto repeat;
            // status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors)
            can_message_sts(can[handle].status, can[handle].error, msg);
            can[handle].counters.err++;
        }
        else if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_ERRFRAME)) {
            // update error and status register from error frame
            can[handle].error.lec = (uint8_t)can_msg_fd.ID;
            can[handle].error.rx_err = can_msg_fd.DATA[2];
            can[handle].error.tx_err = can_msg_fd.DATA[3];
            can[handle].status.bus_error = can[handle].error.lec ? 1 : 0;
            // refuse status message if suppressed by user
            if (!can[handle].mode.err)
                goto repeat;
            // status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors)
            can_message_sts(can[handle].status, can[handle].error, msg);
            can[handle].counters.err++;
        }
        else {
            // decode PEAK CAN FD message and increment receive counter
            can_message_fd(&can_msg_fd, msg);
            can[handle].counters.rx++;
        }
        // time-stamp in nanoseconds since start of Windows
        can_timestamp_fd(timestamp_fd, msg);
    }
    // one message read from receive queue
    can[handle].status.receiver_empty = 0;
    return CANERR_NOERROR;
}

EXPORT
int can_status(int handle, uint8_t *status)
{
    TPCANStatus sts;                    // represents a status

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;

    // TODO: check if running condition is required
    if (!can[handle].status.can_stopped) { // if running get bus status
        // get status from device
        sts = CAN_GetStatus(can[handle].board);
        if ((sts & ~(PCAN_ERROR_ANYBUSERR |
                   PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN |
                   PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)))
            return pcan_error(sts);
        // update status-register (some are latched)
        can[handle].status.bus_off = ((sts & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.bus_error = can[handle].error.lec ? 1 : 0;  // last eror code from error code capture (ECC)
        can[handle].status.warning_level = ((sts & (PCAN_ERROR_BUSWARNING/*PCAN_ERROR_BUSHEAVY*/)) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.transmitter_busy |= ((sts & (PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.message_lost |= ((sts & PCAN_ERROR_OVERRUN) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.queue_overrun |= ((sts & PCAN_ERROR_QOVERRUN) != PCAN_ERROR_OK) ? 1 : 0;
    }
    if (status)                         // status-register
        *status = can[handle].status.byte;
    return CANERR_NOERROR;
}

EXPORT
int can_busload(int handle, uint8_t *load, uint8_t *status)
{
    int rc = CANERR_FATAL;              // return value
    float busLoad = 0.0;                // bus-load (in [percent])

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;

    if (!can[handle].status.can_stopped) { // if running get bus load
        (void)busLoad; //  TODO: measure bus load
    }
    if (load)                           // bus-load (in [percent])
        *load = (uint8_t)busLoad;
    // get status-register from device
    rc = can_status(handle, status);
#if (OPTION_CANAPI_RETVALS == OPTION_DISABLED)
    if (rc == CANERR_NOERROR)
        rc = !can[handle].status.can_stopped ? CANERR_NOERROR : CANERR_OFFLINE;
#else
    // note: can_busload shall return CANERR_NOERROR if
    //       the CAN controller has not been started
#endif
    return rc;
}

EXPORT
int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed)
{
    int rc = CANERR_FATAL;              // return value
    can_bitrate_t tmpBitrate;           // bit-rate settings
    can_speed_t tmpSpeed;               // transmission speed
    bool data = false, sam = false;     // no further usage

    TPCANStatus sts;                    // represents a status
    uint16_t btr0btr1 = BTR0BTR1_DEFAULT;  // btr0btr1 value
    char string[PCAN_MAX_BUFFER_SIZE];  // bit-rate string

    memset(&tmpBitrate, 0, sizeof(can_bitrate_t));
    memset(&tmpSpeed, 0, sizeof(can_speed_t));

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;

    // get bit-rate settings from device
    if (!can[handle].mode.fdoe) {       // CAN 2.0: read BTR0BTR1 register
        if ((sts = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO,
                               (void*)&btr0btr1, sizeof(TPCANBaudrate))) != PCAN_ERROR_OK)
            return pcan_error(sts);
        if ((rc = btr_sja10002bitrate(btr0btr1, &tmpBitrate)) == CANERR_NOERROR)
            rc = btr_bitrate2speed(&tmpBitrate, &tmpSpeed);
    }
    else {                              // CAN FD: read PCAN bit-rate string
        if ((sts = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO_FD,
                               (void*)string, PCAN_MAX_BUFFER_SIZE)) != PCAN_ERROR_OK)
            return pcan_error(sts);
        if ((rc = btr_string2bitrate(string, &tmpBitrate, &data, &sam)) == CANERR_NOERROR)
            rc = btr_bitrate2speed(&tmpBitrate, &tmpSpeed);
    }
    /* note: 'bitrate' as well as 'speed' are optional */
    if (bitrate)
        memcpy(bitrate, &tmpBitrate, sizeof(can_bitrate_t));
    if (speed)
        memcpy(speed, &tmpSpeed, sizeof(can_speed_t));
#if (OPTION_CANAPI_RETVALS == OPTION_DISABLED)
    if (rc == CANERR_NOERROR)
        rc = !can[handle].status.can_stopped ? CANERR_NOERROR : CANERR_OFFLINE;
#else
    // note: can_bitrate shall return CANERR_NOERROR if
    //       the CAN controller has not been started
#endif
    return rc;
}

EXPORT
int can_property(int handle, uint16_t param, void *value, uint32_t nbyte)
{
    if (!init || !IS_HANDLE_VALID(handle)) {
        // note: library properties can be queried w/o a handle
        return lib_parameter(param, value, (size_t)nbyte);
    }
    // note: library is initialized and handle is valid

    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    // note: device properties must be queried with a valid handle
    return drv_parameter(handle, param, value, (size_t)nbyte);
}

EXPORT
char *can_hardware(int handle)
{
    static char hardware[CANPROP_MAX_BUFFER_SIZE+1] = "";
    char str[MAX_LENGTH_HARDWARE_NAME] = "", *ptr = NULL;
    DWORD dev = 0x0000ul;               // device number

    if (!init)                          // must be initialized
        return NULL;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return NULL;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return NULL;

    // return hardware version (zero-terminated string)
    if (CAN_GetValue(can[handle].board, PCAN_HARDWARE_NAME, (void*)str, MAX_LENGTH_HARDWARE_NAME) != PCAN_ERROR_OK)
        return NULL;
    if ((ptr = strchr(str, '\n')) != NULL)
       *ptr = '\0';
    if (PCAN_BOARD_TYPE(can[handle].board) == PCAN_USB) {
        // get device id. from device (USB only!)
        if (CAN_GetValue(can[handle].board, PCAN_DEVICE_ID, (void*)&dev, sizeof(DWORD)) != PCAN_ERROR_OK)
            return NULL;
        snprintf(hardware, CANPROP_MAX_BUFFER_SIZE, "%s, Device-Id. %02Xh", str, dev);
    }
    else {
        strncpy(hardware, str, CANPROP_MAX_BUFFER_SIZE);
    }
    hardware[CANPROP_MAX_BUFFER_SIZE] = '\0';
    return (char*)hardware;
}

EXPORT
char *can_firmware(int handle)
{
    static char firmware[CANPROP_MAX_BUFFER_SIZE+1] = "";
    char str[MAX_LENGTH_HARDWARE_NAME] = "", *ptr = NULL;
    char ver[MAX_LENGTH_VERSION_STRING] = "";

    if (!init)                          // must be initialized
        return NULL;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return NULL;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return NULL;

    if (CAN_GetValue(can[handle].board, PCAN_HARDWARE_NAME, (void*)str, MAX_LENGTH_HARDWARE_NAME) != PCAN_ERROR_OK)
        return NULL;
    if ((ptr = strchr(str, '\n')) != NULL)
        *ptr = '\0';
    if (CAN_GetValue(can[handle].board, PCAN_FIRMWARE_VERSION, (void*)ver, MAX_LENGTH_VERSION_STRING) != PCAN_ERROR_OK)
        return NULL;
    strncpy(firmware, str, CANPROP_MAX_BUFFER_SIZE);
    firmware[CANPROP_MAX_BUFFER_SIZE] = '\0';
    strncat(firmware, ", Firmware ", CANPROP_MAX_BUFFER_SIZE);
    firmware[CANPROP_MAX_BUFFER_SIZE] = '\0';
    strncat(firmware, ver, CANPROP_MAX_BUFFER_SIZE);
    firmware[CANPROP_MAX_BUFFER_SIZE] = '\0';
    return (char*)firmware;
}

/*  -----------  local functions  ----------------------------------------
 */
static void var_init(void)
{
    int i;

    for (i = 0; i < CAN_MAX_HANDLES; i++) {
        memset(&can[i], 0, sizeof(can_interface_t));
        can[i].board = PCAN_NONEBUS;
        can[i].brd_type = 0u;
        can[i].brd_port = 0u;
        can[i].brd_irq = 0u;
#if defined(_WIN32) || defined(_WIN64)
        can[i].event = NULL;
#else
        can[i].fdes = -1;
#endif
        can[i].mode.byte = CANMODE_DEFAULT;
        can[i].status.byte = CANSTAT_RESET;
        can[i].filter.mode = FILTER_OFF;
        can[i].error.lec = 0x00u;
        can[i].error.rx_err = 0u;
        can[i].error.tx_err = 0u;
        can[i].counters.tx = 0ull;
        can[i].counters.rx = 0ull;
        can[i].counters.err = 0ull;
    }
}

static int all_closed(void)
{
    int handle;

    if (!init)
        return 1;
    for (handle = 0; handle < CAN_MAX_HANDLES; handle++) {
        if (IS_HANDLE_OPENED(handle))
            return 0;
    }
    return 1;
}

static void can_message(const TPCANMsg *pcan_msg, can_message_t *msg)
{
    assert(msg);
    assert(pcan_msg);
    msg->id = (int32_t)pcan_msg->ID;
    msg->xtd = (pcan_msg->MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
    msg->rtr = (pcan_msg->MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
    msg->fdf = 0;
    msg->brs = 0;
    msg->esi = 0;
    msg->sts = 0;
    msg->dlc = (uint8_t)pcan_msg->LEN;
    memcpy(msg->data, pcan_msg->DATA, CAN_MAX_LEN);
}

static void can_message_fd(const TPCANMsgFD *pcan_msg, can_message_t *msg)
{
    assert(msg);
    assert(pcan_msg);
    msg->id = (int32_t)pcan_msg->ID;
    msg->xtd = (pcan_msg->MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
    msg->rtr = (pcan_msg->MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
    msg->fdf = (pcan_msg->MSGTYPE & PCAN_MESSAGE_FD) ? 1 : 0;
    msg->brs = (pcan_msg->MSGTYPE & PCAN_MESSAGE_BRS) ? 1 : 0;
    msg->esi = 0;  // FIXME: ESI?
    msg->sts = 0;
    msg->dlc = (uint8_t)pcan_msg->DLC;
    memcpy(msg->data, pcan_msg->DATA, CANFD_MAX_LEN);
}

static void can_message_sts(can_status_t status, can_error_t error, can_message_t *msg)
{
    assert(msg);
    msg->id = (int32_t)0;
    msg->xtd = 0;
    msg->rtr = 0;
    msg->fdf = 0;
    msg->brs = 0;
    msg->esi = 0;
    msg->sts = 1;
    msg->dlc = (uint8_t)4;
    msg->data[0] = (uint8_t)status.byte;
    msg->data[1] = (uint8_t)error.lec;
    msg->data[2] = (uint8_t)error.rx_err;
    msg->data[4] = (uint8_t)error.tx_err;
}

static void can_timestamp(TPCANTimestamp timestamp, can_message_t *msg)
{
    assert(msg);
    uint64_t msec = ((uint64_t)timestamp.millis_overflow << 32) + (uint64_t)timestamp.millis;
    msg->timestamp.tv_sec = (time_t)(msec / 1000ull);
    msg->timestamp.tv_nsec = ((((long)(msec % 1000ull)) * 1000L) + (long)timestamp.micros) * (long)1000;
}

static void can_timestamp_fd(TPCANTimestampFD timestamp, can_message_t *msg)
{
    assert(msg);
    msg->timestamp.tv_sec = (time_t)(timestamp / 1000000ull);
    msg->timestamp.tv_nsec = (long)(timestamp % 1000000ull) * (long)1000;
}

#define PCAN_ERROR_MASK  (PCAN_ERROR_REGTEST | PCAN_ERROR_NODRIVER | PCAN_ERROR_HWINUSE | PCAN_ERROR_NETINUSE | \
                          PCAN_ERROR_ILLHW | PCAN_ERROR_ILLHW | PCAN_ERROR_ILLCLIENT)

static int pcan_error(TPCANStatus status)
{
    if ((status & PCAN_ERROR_XMTFULL)      == PCAN_ERROR_XMTFULL)       return CANERR_TX_BUSY;
    if ((status & PCAN_ERROR_OVERRUN)      == PCAN_ERROR_OVERRUN)       return CANERR_MSG_LST;
    if ((status & PCAN_ERROR_BUSOFF)       == PCAN_ERROR_BUSOFF)        return CANERR_BOFF;
    if ((status & PCAN_ERROR_BUSPASSIVE)   == PCAN_ERROR_BUSPASSIVE)    return CANERR_EWRN;
    if ((status & PCAN_ERROR_BUSHEAVY)     == PCAN_ERROR_BUSHEAVY)      return CANERR_BERR;
    if ((status & PCAN_ERROR_BUSLIGHT)     == PCAN_ERROR_BUSLIGHT)      return CANERR_BERR;
    if ((status & PCAN_ERROR_QRCVEMPTY)    == PCAN_ERROR_QRCVEMPTY)     return CANERR_RX_EMPTY;
    if ((status & PCAN_ERROR_QOVERRUN)     == PCAN_ERROR_QOVERRUN)      return CANERR_QUE_OVR;
    if ((status & PCAN_ERROR_QXMTFULL)     == PCAN_ERROR_QXMTFULL)      return CANERR_TX_BUSY;
    if ((status & PCAN_ERROR_MASK)         == PCAN_ERROR_REGTEST)       return PCAN_ERR_REGTEST;
    if ((status & PCAN_ERROR_MASK)         == PCAN_ERROR_NODRIVER)      return PCAN_ERR_NODRIVER;
    if ((status & PCAN_ERROR_MASK)         == PCAN_ERROR_HWINUSE)       return PCAN_ERR_HWINUSE;
    if ((status & PCAN_ERROR_MASK)         == PCAN_ERROR_NETINUSE)      return PCAN_ERR_NETINUSE;
    if ((status & PCAN_ERROR_MASK)         == PCAN_ERROR_ILLHW)         return PCAN_ERR_ILLHW;
    if ((status & PCAN_ERROR_MASK)         == PCAN_ERROR_ILLNET)        return PCAN_ERR_ILLNET;
    if ((status & PCAN_ERROR_MASK)         == PCAN_ERROR_ILLCLIENT)     return PCAN_ERR_ILLCLIENT;
    if ((status & PCAN_ERROR_RESOURCE)     == PCAN_ERROR_RESOURCE)      return PCAN_ERR_RESOURCE;
    if ((status & PCAN_ERROR_ILLPARAMTYPE) == PCAN_ERROR_ILLPARAMTYPE)  return PCAN_ERR_ILLPARAMTYPE;
    if ((status & PCAN_ERROR_ILLPARAMVAL)  == PCAN_ERROR_ILLPARAMVAL)   return PCAN_ERR_ILLPARAMVAL;
    if ((status & PCAN_ERROR_ILLDATA)      == PCAN_ERROR_ILLDATA)       return PCAN_ERR_ILLDATA;
    if ((status & PCAN_ERROR_CAUTION)      == PCAN_ERROR_CAUTION)       return PCAN_ERR_CAUTION;
    if ((status & PCAN_ERROR_INITIALIZE)   == PCAN_ERROR_INITIALIZE)    return CANERR_NOTINIT;
    if ((status & PCAN_ERROR_ILLOPERATION) == PCAN_ERROR_ILLOPERATION)  return PCAN_ERR_ILLOPERATION;

    return PCAN_ERR_UNKNOWN;
}

static int pcan_compatibility(void) {
    TPCANStatus sts;                    // represents a status
    unsigned int major = 0, minor = 0;  // channel version
    char api[CANPROP_MAX_BUFFER_SIZE+1] = "0.0.0.0";

    // get library version (as a string)
    if ((sts = CAN_GetValue(PCAN_NONEBUS, PCAN_API_VERSION, (void*)api, CANPROP_MAX_BUFFER_SIZE)) != PCAN_ERROR_OK)
        return pcan_error(sts);
    // to be on the safe side, append a zero-terminator
    api[CANPROP_MAX_BUFFER_SIZE] = '\0';
    // extract major and minor revision
    if (sscanf(api, "%u.%u", &major, &minor) != 2)
        return CANERR_FATAL;
    // check for minimal required version
#if (PCAN_LIB_MIN_MINOR != 0)
    if ((major != PCAN_LIB_MIN_MAJOR) || (minor < PCAN_LIB_MIN_MINOR))
#else
    if (major != PCAN_LIB_MIN_MAJOR)
#endif
        return CANERR_LIBRARY;
    else
        return CANERR_NOERROR;
}

static TPCANStatus pcan_capability(TPCANHandle board, can_mode_t *capability)
{
    TPCANStatus sts;                    // represents a status
    DWORD features;                     // channel features

    assert(capability);                 // just to make sure
    capability->byte = 0x00u;

    // get channel features from device
    if ((sts = CAN_GetValue((TPCANHandle)board, PCAN_CHANNEL_FEATURES,
                            (void*)&features, sizeof(features))) != PCAN_ERROR_OK)
        return sts;
    // determine the channel capabilities
    capability->fdoe = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->brse = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->niso = 0; // this can not be determined (FIXME)
    capability->shrd = 0; // this feature is not supported (PCANBasic)
#if (0)
    capability->nxtd = 1; // PCAN_ACCEPTANCE_FILTER_29BIT available since version 4.2.0
    capability->nrtr = 1; // PCAN_ALLOW_RTR_FRAMES available since version 4.2.0
#else
    capability->nxtd = 1; // suppress XTD frames (software solution)
    capability->nrtr = 1; // suppress RTR frames (software solution)
#endif
    capability->err = 1;  // PCAN_ALLOW_ERROR_FRAMES available since version 4.2.0
    capability->mon = 1;  // PCAN_LISTEN_ONLY available since version 1.0.0

    return PCAN_ERROR_OK;
}

static TPCANStatus pcan_get_filter(int handle, uint64_t *filter, filtering_t mode)
{
    TPCANStatus sts;                    // represents a status

    assert(IS_HANDLE_VALID(handle));    // just to make sure
    assert(filter);

    // get the filter value from device
    switch (mode) {
        case FILTER_STD:                // 11-bit identifier
            if (can[handle].filter.mode != FILTER_XTD) {
                if ((sts = CAN_GetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_11BIT,
                                       (void*)filter, sizeof(UINT64))) == PCAN_ERROR_OK) {
                    *filter ^= FILTER_STD_XOR_MASK;  // SJA100 has inverted masks bits!
                }
            }
            else {
                // note: there is only one filter for both modes
                *filter = FILTER_RESET_VALUE;
                sts = PCAN_ERROR_OK;
            }
            break;
        case FILTER_XTD:                // 29-bit identifier
            if (can[handle].filter.mode != FILTER_STD) {
                if ((sts = CAN_GetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_29BIT,
                                       (void*)filter, sizeof(UINT64))) == PCAN_ERROR_OK) {
                    *filter ^= FILTER_XTD_XOR_MASK;   // SJA100 has inverted masks bits!
                }
            }
            else {
                // note: there is only one filter for both modes
                *filter = FILTER_RESET_VALUE;
                sts = PCAN_ERROR_OK;
            }
            break;
        default:                        // should not happen
            *filter = FILTER_RESET_VALUE;
            sts = PCAN_ERROR_CAUTION;
            break;
    }
    return sts;
}

static TPCANStatus pcan_set_filter(int handle, uint64_t filter, filtering_t mode)
{
    TPCANStatus sts;                    // represents a status
    UINT64 value = 0x0ull;              // PCAN filter value

    assert(IS_HANDLE_VALID(handle));    // just to make sure

    // set the filter value to device
    switch (mode) {
        case FILTER_STD:                // 11-bit identifier
            value = (filter ^ FILTER_STD_XOR_MASK);   // SJA100 has inverted masks bits!
            if ((sts = CAN_SetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_11BIT,
                                   (void*)&value, sizeof(value))) == PCAN_ERROR_OK) {
                can[handle].filter.mode = FILTER_STD;
                can[handle].filter.mask = (uint64_t)value;
            }
            break;
        case FILTER_XTD:                // 29-bit identifier
            value = (filter ^ FILTER_XTD_XOR_MASK);   // SJA100 has inverted masks bits!
            if ((sts = CAN_SetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_29BIT,
                                   (void*)&value, sizeof(value))) == PCAN_ERROR_OK) {
                can[handle].filter.mode = FILTER_XTD;
                can[handle].filter.mask = (uint64_t)value;
            }
            break;
        default:                        // no filtering
            sts = pcan_reset_filter(handle);
            break;
    }
    return sts;
}

static TPCANStatus pcan_reset_filter(int handle)
{
    TPCANStatus sts;                    // represents a status
    uint8_t filter = PCAN_FILTER_OPEN;  // filter mode (accept all)

    assert(IS_HANDLE_VALID(handle));    // just to make sure

    // reset the filter value to device
    if ((sts = CAN_SetValue(can[handle].board, (BYTE)PCAN_MESSAGE_FILTER,
        (void*)&filter, (DWORD)sizeof(uint8_t))) == PCAN_ERROR_OK) {
        can[handle].filter.mode = FILTER_OFF;
    }
    return sts;
}

/*  - - - - - -  CAN API V3 properties  - - - - - - - - - - - - - - - - -
 */
static int lib_parameter(uint16_t param, void *value, size_t nbyte)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter
    static int idx_board = EOF;         // actual index in the interface list
    TPCANStatus sts;                    // represents a status

    if (value == NULL) {                // check for null-pointer
        if ((param != CANPROP_SET_FIRST_CHANNEL) &&
            (param != CANPROP_SET_NEXT_CHANNEL) &&
            (param != CANPROP_SET_FILTER_RESET))
            return CANERR_NULLPTR;
    }
    // query or modify a CAN library property
    switch (param) {
    case CANPROP_GET_SPEC:              // version of the wrapper specification (uint16_t)
        if (nbyte >= sizeof(uint16_t)) {
            *(uint16_t*)value = (uint16_t)CAN_API_SPEC;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_VERSION:           // version number of the library (uint16_t)
        if (nbyte >= sizeof(uint16_t)) {
            *(uint16_t*)value = ((uint16_t)VERSION_MAJOR << 8)
                              | ((uint16_t)VERSION_MINOR & 0xFu);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_PATCH_NO:          // patch number of the library (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)VERSION_PATCH;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BUILD_NO:          // build number of the library (uint32_t)
        if (nbyte >= sizeof(uint32_t)) {
            *(uint32_t*)value = (uint32_t)VERSION_BUILD;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_ID:        // library id of the library (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)LIB_ID;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_VENDOR:    // vendor name of the library (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, CAN_API_VENDOR, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_DLLNAME:   // file name of the library (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, LIB_DLLNAME, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_VENDOR:     // vendor name of the CAN interface (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_VENDOR, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_DLLNAME:    // file name of the CAN interface DLL (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_DLLNAME, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_SET_FIRST_CHANNEL:     // set index to the first entry in the interface list (NULL)
        idx_board = 0;
        rc = (can_boards[idx_board].type != EOF) ? CANERR_NOERROR : CANERR_RESOURCE;
        break;
    case CANPROP_SET_NEXT_CHANNEL:      // set index to the next entry in the interface list (NULL)
        if ((0 <= idx_board) && (idx_board < NUM_CHANNELS)) {
            if (can_boards[idx_board].type != EOF)
                idx_board++;
            rc = (can_boards[idx_board].type != EOF) ? CANERR_NOERROR : CANERR_RESOURCE;
        }
        else
            rc = CANERR_RESOURCE;
        break;
    case CANPROP_GET_CHANNEL_NO:        // get channel no. at actual index in the interface list (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            if ((0 <= idx_board) && (idx_board < NUM_CHANNELS) &&
                (can_boards[idx_board].type != EOF)) {
                *(int32_t*)value = (int32_t)can_boards[idx_board].type;
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_NAME:      // get channel name at actual index in the interface list (char[])
        if (nbyte >= 1u) {
            if ((0 <= idx_board) && (idx_board < NUM_CHANNELS) &&
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, can_boards[idx_board].name, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_DLLNAME:   // get file name of the DLL at actual index in the interface list (char[])
        if (nbyte >= 1u) {
            if ((0 <= idx_board) && (idx_board < NUM_CHANNELS) &&
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, DEV_DLLNAME, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_VENDOR_ID: // get library id at actual index in the interface list (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            if ((0 <= idx_board) && (idx_board < NUM_CHANNELS) &&
                (can_boards[idx_board].type != EOF)) {
                *(int32_t*)value = (int32_t)LIB_ID;
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_VENDOR_NAME: // get vendor name at actual index in the interface list (char[])
        if (nbyte >= 1u) {
            if ((0 <= idx_board) && (idx_board < NUM_CHANNELS) &&
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, DEV_VENDOR, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_DEVICE_TYPE:       // device type of the CAN interface (int32_t)
    case CANPROP_GET_DEVICE_NAME:       // device name of the CAN interface (char[])
    case CANPROP_GET_OP_CAPABILITY:     // supported operation modes of the CAN controller (uint8_t)
    case CANPROP_GET_OP_MODE:           // active operation mode of the CAN controller (uint8_t)
    case CANPROP_GET_BITRATE:           // active bit-rate of the CAN controller (can_bitrate_t)
    case CANPROP_GET_SPEED:             // active bus speed of the CAN controller (can_speed_t)
    case CANPROP_GET_STATUS:            // current status register of the CAN controller (uint8_t)
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint16_t)
    case CANPROP_GET_NUM_CHANNELS:      // numbers of CAN channels on the CAN interface (uint8_t)
    case CANPROP_GET_CAN_CHANNEL:       // active CAN channel on the CAN interface (uint8_t)
    case CANPROP_GET_CAN_CLOCK:         // frequency of the CAN controller clock in [Hz] (int32_t)
    case CANPROP_GET_TX_COUNTER:        // total number of sent messages (uint64_t)
    case CANPROP_GET_RX_COUNTER:        // total number of reveiced messages (uint64_t)
    case CANPROP_GET_ERR_COUNTER:       // total number of reveiced error frames (uint64_t)
    case CANPROP_GET_RCV_QUEUE_SIZE:    // maximum number of message the receive queue can hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_HIGH:    // maximum number of message the receive queue has hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_OVFL:    // overflow counter of the receive queue (uint64_t)
    case CANPROP_GET_FILTER_11BIT:      // acceptance filter code and mask for 11-bit identifier (uint64_t)
    case CANPROP_GET_FILTER_29BIT:      // acceptance filter code and mask for 29-bit identifier (uint64_t)
    case CANPROP_SET_FILTER_11BIT:      // set value for acceptance filter code and mask for 11-bit identifier (uint64_t)
    case CANPROP_SET_FILTER_29BIT:      // set value for acceptance filter code and mask for 29-bit identifier (uint64_t)
    case CANPROP_SET_FILTER_RESET:      // reset acceptance filter code and mask to default values (NULL)
        // note: a device parameter requires a valid handle.
        if (!init)
            rc = CANERR_NOTINIT;
        else
            rc = CANERR_HANDLE;
        break;
    default:
        if ((CANPROP_GET_VENDOR_PROP <= param) &&  // get a vendor-specific property value (void*)
            (param < (CANPROP_GET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if ((sts = CAN_GetValue(PCAN_NONEBUS, (BYTE)(param - CANPROP_GET_VENDOR_PROP),
                (void*)value, (DWORD)nbyte)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else if ((CANPROP_SET_VENDOR_PROP <= param) &&  // set a vendor-specific property value (void*)
            (param < (CANPROP_SET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if ((sts = CAN_SetValue(PCAN_NONEBUS, (BYTE)(param - CANPROP_SET_VENDOR_PROP),
                (void*)value, (DWORD)nbyte)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else                            // or what?
            rc = CANERR_NOTSUPP;
        break;
    }
    return rc;
}

static int drv_parameter(int handle, uint16_t param, void *value, size_t nbyte)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter
    can_bitrate_t bitrate;              // bit-rate settings
    can_speed_t speed;                  // current bus speed
    can_mode_t mode;                    // current operation mode
    uint8_t status = 0u;                // status register
    uint8_t load = 0u;                  // bus load
    uint64_t filter = 0ull;             // acceptance filter
    char str[MAX_LENGTH_HARDWARE_NAME+1];  // device name
    TPCANStatus sts;                    // represents a status

    assert(IS_HANDLE_VALID(handle));    // just to make sure

    if (value == NULL) {                // check for null-pointer
        if ((param != CANPROP_SET_FIRST_CHANNEL) &&
            (param != CANPROP_SET_NEXT_CHANNEL) &&
            (param != CANPROP_SET_FILTER_RESET))
            return CANERR_NULLPTR;
    }
    // query or modify a CAN interface property
    switch (param) {
    case CANPROP_GET_DEVICE_TYPE:       // device type of the CAN interface (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)PCAN_BOARD_TYPE(can[handle].board);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_NAME:       // device name of the CAN interface (char[])
        if (nbyte >= 1u) {
            if ((sts = CAN_GetValue(can[handle].board, (BYTE)PCAN_HARDWARE_NAME,
                (void*)str, (DWORD)MAX_LENGTH_HARDWARE_NAME)) == PCAN_ERROR_OK) {
                str[MAX_LENGTH_HARDWARE_NAME] = '\0';
                strncpy((char*)value, str, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            } else
                rc = pcan_error(sts);
        }
        break;
    case CANPROP_GET_DEVICE_VENDOR:     // vendor name of the CAN interface (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_VENDOR, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_DLLNAME:    // file name of the CAN interface DLL (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_DLLNAME, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_PARAM:      // device parameter of the CAN interface (can_pcan_param_t)
        if (nbyte >= sizeof(can_pcan_param_t)) {
            ((can_pcan_param_t*)value)->type = (uint8_t)can[handle].brd_type;
            ((can_pcan_param_t*)value)->port = (uint32_t)can[handle].brd_port;
            ((can_pcan_param_t*)value)->irq = (uint16_t)can[handle].brd_irq;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_OP_CAPABILITY:     // supported operation modes of the CAN controller (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            if ((sts = pcan_capability(can[handle].board, &mode)) == PCAN_ERROR_OK) {
                *(uint8_t*)value = (uint8_t)mode.byte;
                rc = CANERR_NOERROR;
            } else
                rc = pcan_error(sts);
        }
        break;
    case CANPROP_GET_OP_MODE:           // active operation mode of the CAN controller (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)can[handle].mode.byte;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BITRATE:           // active bit-rate of the CAN controller (can_bitrate_t)
        if (nbyte >= sizeof(can_bitrate_t)) {
            if (((rc = can_bitrate(handle, &bitrate, NULL)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
                memcpy(value, &bitrate, sizeof(can_bitrate_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_SPEED:             // active bus speed of the CAN controller (can_speed_t)
        if (nbyte >= sizeof(can_speed_t)) {
            if (((rc = can_bitrate(handle, NULL, &speed)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
                memcpy(value, &speed, sizeof(can_speed_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_STATUS:            // current status register of the CAN controller (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            if ((rc = can_status(handle, &status)) == CANERR_NOERROR) {
                *(uint8_t*)value = (uint8_t)status;
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint16_t)
        if (nbyte >= sizeof(uint8_t)) {
            if (((rc = can_busload(handle, &load, NULL)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
                if (nbyte > sizeof(uint8_t))
                    *(uint16_t*)value = (uint16_t)load * 100u;  // 0..10000 ==> 0.00%..100.00%
                else
                    *(uint8_t*)value = (uint8_t)load;           // 0..100% (note: legacy resolution)
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_NUM_CHANNELS:      // numbers of CAN channels on the CAN interface (uint8_t)
        // TODO: insert coin here
        rc = CANERR_NOTSUPP;
        break;
    case CANPROP_GET_CAN_CHANNEL:       // active CAN channel on the CAN interface (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            if ((sts = CAN_GetValue(can[handle].board, (BYTE)PCAN_CONTROLLER_NUMBER,
                (void*)value, (DWORD)nbyte)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        break;
    case CANPROP_GET_CAN_CLOCK:         // frequency of the CAN controller clock in [Hz] (int32_t)
        // TODO: insert coin here
        rc = CANERR_NOTSUPP;
        break;
    case CANPROP_GET_TX_COUNTER:        // total number of sent messages (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.tx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_RX_COUNTER:        // total number of reveiced messages (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.rx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_ERR_COUNTER:       // total number of reveiced error frames (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.err;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_RCV_QUEUE_SIZE:    // maximum number of message the receive queue can hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_HIGH:    // maximum number of message the receive queue has hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_OVFL:    // overflow counter of the receive queue (uint64_t)
        // note: cannot be determined
        rc = CANERR_NOTSUPP;
        break;
    case CANPROP_GET_FILTER_11BIT:      // acceptance filter code and mask for 11-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            if ((sts = pcan_get_filter(handle, (uint64_t*)value, FILTER_STD)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        break;
    case CANPROP_GET_FILTER_29BIT:      // acceptance filter code and mask for 29-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            if ((sts = pcan_get_filter(handle, (uint64_t*)value, FILTER_XTD)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        break;
    case CANPROP_SET_FILTER_11BIT:      // set value for acceptance filter code and mask for 11-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            if (!(*(uint64_t*)value & ~FILTER_STD_VALID_MASK)) {
                // note: code and mask must not exceed 11-bit identifier
                if (can[handle].status.can_stopped) {
                    // note: set filter only if the CAN controller is in INIT mode
                    if ((sts = pcan_set_filter(handle, *(uint64_t*)value, FILTER_STD)) == PCAN_ERROR_OK)
                        rc = CANERR_NOERROR;
                    else
                        rc = pcan_error(sts);
                }
                else
                    rc = CANERR_ONLINE;
            }
            else
                rc = CANERR_ILLPARA;
        }
        break;
    case CANPROP_SET_FILTER_29BIT:      // set value for acceptance filter code and mask for 29-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            if (!(*(uint64_t*)value & ~FILTER_XTD_VALID_MASK) && !can[handle].mode.nxtd) {
                // note: code and mask must not exceed 29-bit identifier and
                //       extended frame format mode must not be suppressed
                if (can[handle].status.can_stopped) {
                    // note: set filter only if the CAN controller is in INIT mode
                    if ((sts = pcan_set_filter(handle, *(uint64_t*)value, FILTER_XTD)) == PCAN_ERROR_OK)
                        rc = CANERR_NOERROR;
                    else
                        rc = pcan_error(sts);
                }
                else
                    rc = CANERR_ONLINE;
            }
            else
                rc = CANERR_ILLPARA;
        }
        break;
    case CANPROP_SET_FILTER_RESET:      // reset acceptance filter code and mask to default values (NULL)
        if (can[handle].status.can_stopped) {
            // note: reset filter only if the CAN controller is in INIT mode
            if ((sts = pcan_reset_filter(handle)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else
            rc = CANERR_ONLINE;
        break;
    default:
        if ((CANPROP_GET_VENDOR_PROP <= param) &&  // get a vendor-specific property value (void*)
           (param < (CANPROP_GET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if ((sts = CAN_GetValue(can[handle].board, (BYTE)(param - CANPROP_GET_VENDOR_PROP),
                (void*)value, (DWORD)nbyte)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else if ((CANPROP_SET_VENDOR_PROP <= param) &&  // set a vendor-specific property value (void*)
                (param < (CANPROP_SET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if ((sts = CAN_SetValue(can[handle].board, (BYTE)(param - CANPROP_SET_VENDOR_PROP),
                (void*)value, (DWORD)nbyte)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else                            // or general library properties (see lib_parameter)
            rc = lib_parameter(param, value, nbyte);
        break;
    }
    return rc;
}

/*  -----------  revision control  ---------------------------------------
 */
EXPORT
char *can_version(void)
{
    return (char*)version;
}
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de, Homepage: https://www.uv-software.de/
 */
