/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (for PEAK PCAN Interfaces)
 *
 *  Copyright (c) 2005-2010 Uwe Vogt, UV Software, Friedrichshafen
 *  Copyright (c) 2014-2022 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
#include "build_no.h"
#ifdef _MSC_VER
#define VERSION_MAJOR    0
#define VERSION_MINOR    4
#define VERSION_PATCH    2
#else
#define VERSION_MAJOR    0
#define VERSION_MINOR    2
#define VERSION_PATCH    2
#endif
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
static const char version[] = "CAN API V3 for PEAK PCAN Interfaces, Version " VERSION_STRING;


/*  -----------  includes  -----------------------------------------------
 */

#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include "can_defs.h"
#include "can_api.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include "PCANBasic.h"
#else
#include <unistd.h>
#include <sys/select.h>
#include "PCBUSB.h"
#endif


/*  -----------  options  ------------------------------------------------
 */
#if (OPTION_CANAPI_PCBUSB_DYLIB != 0)
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

#ifndef PCAN_MAX_HANDLES
#define PCAN_MAX_HANDLES        (16)    // maximum number of open handles
#endif
#define INVALID_HANDLE          (-1)
#define IS_HANDLE_VALID(hnd)    ((0 <= (hnd)) && ((hnd) < PCAN_MAX_HANDLES))
#ifndef DLC2LEN
#define DLC2LEN(x)              dlc_table[(x) & 0xF]
#endif
#ifdef  OPTION_PCAN_CiA_BIT_TIMING
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
#ifndef SYSERR_OFFSET
#define SYSERR_OFFSET           (-10000)
#endif

/*  -----------  types  --------------------------------------------------
 */

typedef struct {                        // frame conters:
    uint64_t tx;                        //   number of transmitted CAN frames
    uint64_t rx;                        //   number of received CAN frames
    uint64_t err;                       //   number of receiced error frames
}   can_counter_t;

typedef struct {                        // PCAN interface:
    TPCANHandle board;                  //   board hardware channel handle
    BYTE  brd_type;                     //   board type (none PnP hardware)
    DWORD brd_port;                     //   board parameter: I/O port address
    WORD  brd_irq;                      //   board parameter: interrupt number
#if defined(_WIN32) || defined(_WIN64)
    HANDLE event;                       //   event handle for blocking read
#endif
    can_mode_t mode;                    //   operation mode of the CAN channel
    can_status_t status;                //   8-bit status register
    can_counter_t counters;             //   statistical counters
}   can_interface_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static int pcan_error(TPCANStatus);     // PCAN specific errors
static TPCANStatus pcan_capability(TPCANHandle board, can_mode_t *capability);

static int map_index2bitrate(int index, can_bitrate_t* bitrate);
static int map_bitrate2register(const can_bitrate_t *bitrate, TPCANBaudrate *btr0btr1);
static int map_register2bitrate(const TPCANBaudrate btr0btr1, can_bitrate_t *bitrate);
static int map_bitrate2string(const can_bitrate_t *bitrate, TPCANBitrateFD string, int brse);
static int map_string2bitrate(const TPCANBitrateFD string, can_bitrate_t *bitrate, int brse);

static int lib_parameter(uint16_t param, void *value, size_t nbyte);
static int drv_parameter(int handle, uint16_t param, void *value, size_t nbyte);

static int calc_speed(can_bitrate_t *bitrate, can_speed_t *speed, int modify);


/*  -----------  variables  ----------------------------------------------
 */

EXPORT
can_board_t can_boards[PCAN_BOARDS+1]=// list of CAN Interface boards:
{
    {PCAN_USB1,                           "PCAN-USB1"},
    {PCAN_USB2,                           "PCAN-USB2"},
    {PCAN_USB3,                           "PCAN-USB3"},
    {PCAN_USB4,                           "PCAN-USB4"},
    {PCAN_USB5,                           "PCAN-USB5"},
    {PCAN_USB6,                           "PCAN-USB6"},
    {PCAN_USB7,                           "PCAN-USB7"},
    {PCAN_USB8,                           "PCAN-USB8"},
#ifndef __APPLE__
    {PCAN_USB9,                           "PCAN-USB9"},
    {PCAN_USB10,                          "PCAN-USB10"},
    {PCAN_USB11,                          "PCAN-USB11"},
    {PCAN_USB12,                          "PCAN-USB12"},
    {PCAN_USB13,                          "PCAN-USB13"},
    {PCAN_USB14,                          "PCAN-USB14"},
    {PCAN_USB15,                          "PCAN-USB15"},
    {PCAN_USB16,                          "PCAN-USB16"},
#endif
    {EOF, NULL}
};
static const uint8_t dlc_table[16] = {  // DLC to length
    0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64
};
static can_interface_t can[PCAN_MAX_HANDLES]; // interface handles
static int init =  0;  // initialization flag


/*  -----------  functions  ----------------------------------------------
 */
EXPORT
int can_test(int32_t board, uint8_t mode, const void *param, int *result)
{
    TPCANStatus rc;                     // return value
    DWORD condition;                    // channel condition
    can_mode_t capa;                    // channel capability
    int used = 0;                       // own used channel
    int i;

    if ((board < 0) || (65535 < board)) // PCAN handle is of type WORD!
        return pcan_error(PCAN_ERROR_ILLCLIENT);

    if (!init) {                        // when not init before:
        for (i = 0; i < PCAN_MAX_HANDLES; i++) {
            can[i].board = PCAN_NONEBUS;
            can[i].brd_type = 0;
            can[i].brd_port = 0;
            can[i].brd_irq = 0;
#if defined(_WIN32) || defined(_WIN64)
            can[i].event = NULL;
#endif
            can[i].mode.byte = CANMODE_DEFAULT;
            can[i].status.byte = CANSTAT_RESET;
            can[i].counters.tx = 0ull;
            can[i].counters.rx = 0ull;
            can[i].counters.err = 0ull;
        }
        init = 1;                       //   set initialization flag
    }
    if ((rc = CAN_GetValue((TPCANHandle)board, PCAN_CHANNEL_CONDITION,
                          (void*)&condition, sizeof(condition))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    for (i = 0; i < PCAN_MAX_HANDLES; i++) {
        if (can[i].board == (TPCANHandle)board) { // me, myself and I!
            condition = PCAN_CHANNEL_OCCUPIED;
            used = 1;
            break;
        }
    }
    if (result) {                        // CAN board test:
        if ((condition == PCAN_CHANNEL_AVAILABLE) || (condition == PCAN_CHANNEL_PCANVIEW))
            *result = CANBRD_PRESENT;     // CAN board present
        else if (condition == PCAN_CHANNEL_UNAVAILABLE)
            *result = CANBRD_NOT_PRESENT; // CAN board not present
        else if (condition == PCAN_CHANNEL_OCCUPIED)
            *result = CANBRD_OCCUPIED;    // CAN board present, but occupied
        else
            *result = CANBRD_NOT_TESTABLE;// guess borad is not testable
    }
    if (((condition == PCAN_CHANNEL_AVAILABLE) || (condition == PCAN_CHANNEL_PCANVIEW)) ||
       (/*(condition == PCAN_CHANNEL_OCCUPIED) ||*/ used)) {   // FIXME: issue TC07_47_9w - returns PCAN_ERROR_INITIALIZE when channel used by another process
        // get operation capability from CAN board
        if ((rc = pcan_capability((TPCANHandle)board, &capa)) != PCAN_ERROR_OK)
            return pcan_error(rc);
        // check given operation mode against the operation capability
        if ((mode & ~capa.byte) != 0)
            return CANERR_ILLPARA;
        if ((mode & CANMODE_BRSE) && !(mode & CANMODE_FDOE))
            return CANERR_ILLPARA;
    }
    (void)param;
    return CANERR_NOERROR;
}

EXPORT
int can_init(int32_t board, uint8_t mode, const void *param)
{
    TPCANStatus rc;                     // return value
    DWORD value;                        // parameter value
    can_mode_t capa;                    // board capability
    BYTE  type = 0;                     // board type (none PnP hardware)
    DWORD port = 0;                     // board parameter: I/O port address
    WORD  irq = 0;                      // board parameter: interrupt number
    int i;

    if ((board < 0) || (65535 < board)) // PCAN handle is of type WORD!
        return pcan_error(PCAN_ERROR_ILLCLIENT);

    if (!init) {                        // when not init before:
        for (i = 0; i < PCAN_MAX_HANDLES; i++) {
            can[i].board = PCAN_NONEBUS;
            can[i].brd_type = 0;
            can[i].brd_port = 0;
            can[i].brd_irq = 0;
#if defined(_WIN32) || defined(_WIN64)
            can[i].event = NULL;
#endif
            can[i].mode.byte = CANMODE_DEFAULT;
            can[i].status.byte = CANSTAT_RESET;
            can[i].counters.tx = 0ull;
            can[i].counters.rx = 0ull;
            can[i].counters.err = 0ull;
        }
        init = 1;                       //   set initialization flag
    }
    for (i = 0; i < PCAN_MAX_HANDLES; i++) {
        if (can[i].board == (TPCANHandle)board) // channel already in use
          return CANERR_YETINIT;
    }
    for (i = 0; i < PCAN_MAX_HANDLES; i++) {
        if (can[i].board == PCAN_NONEBUS)  // get an unused handle, if any
            break;
    }
    if (!IS_HANDLE_VALID(i))             // no free handle found
        return CANERR_HANDLE;

    /* get operation capabilit from channel check with given operation mode */
    if ((rc = pcan_capability((TPCANHandle)board, &capa)) != PCAN_ERROR_OK)
        return pcan_error(rc);
    if ((mode & ~capa.byte) != 0)
        return CANERR_ILLPARA;
    if ((mode & CANMODE_BRSE) && !(mode & CANMODE_FDOE))
        return CANERR_ILLPARA;
#if defined(_WIN32) || defined(_WIN64)
    /* one event handle per channel */
    if ((can[i].event = CreateEvent(    // create an event handle
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
    if ((rc = CAN_SetValue((TPCANHandle)board, PCAN_RECEIVE_STATUS,
                          (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    value = PCAN_PARAMETER_ON;          // transmitter OFF
    if ((rc = CAN_SetValue((TPCANHandle)board, PCAN_LISTEN_ONLY,
                          (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    if ((mode & CANMODE_FDOE)) {        // CAN FD operation mode?
        if ((rc = CAN_InitializeFD((TPCANHandle)board, BIT_RATE_DEFAULT)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    else {                              // CAN 2.0 operation mode
        if (param) {
            type =  (BYTE)((struct _pcan_param*)param)->type;
            port = (DWORD)((struct _pcan_param*)param)->port;
            irq  =  (WORD)((struct _pcan_param*)param)->irq;
        }
        if ((rc = CAN_Initialize((TPCANHandle)board, BTR0BTR1_DEFAULT, type, port, irq)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    can[i].board = (TPCANHandle)board;  // handle of the CAN channel
    if (param) {                        // non-plug'n'play devices:
        can[i].brd_type =  (BYTE)((struct _pcan_param*)param)->type;
        can[i].brd_port = (DWORD)((struct _pcan_param*)param)->port;
        can[i].brd_irq  =  (WORD)((struct _pcan_param*)param)->irq;
    }
    can[i].mode.byte = mode;            // store selected operation mode
    can[i].status.byte = CANSTAT_RESET; // CAN controller not started yet!

    return i;                           // return the handle
}

EXPORT
int can_exit(int handle)
{
    TPCANStatus rc;                     // return value
    int i;

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (handle != CANEXIT_ALL) {
        if (!IS_HANDLE_VALID(handle))   // must be a valid handle
            return CANERR_HANDLE;
        if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
            return CANERR_HANDLE;
        if (!can[handle].status.can_stopped) { // when running then go bus off
            /* note: here we should turn off the receiver and the transmitter,
             *       but after CAN_Uninitialize we are really (bus) OFF! */
            (void)CAN_Reset(can[handle].board);
        }
        if ((rc = CAN_Uninitialize(can[handle].board)) != PCAN_ERROR_OK)
            return pcan_error(rc);

        can[handle].status.byte |= CANSTAT_RESET;  // CAN controller in INIT state
        can[handle].board = PCAN_NONEBUS; // handle can be used again

#if defined(_WIN32) || defined(_WIN64)
        if (can[handle].event != NULL) {  // close event handle, if any
            if (!CloseHandle(can[handle].event))
                return SYSERR_OFFSET - (int)GetLastError();
        }
#endif
    }
    else {
        for (i = 0; i < PCAN_MAX_HANDLES; i++) {
            if (can[i].board != PCAN_NONEBUS) // must be an opened handle
            {
                if (!can[i].status.can_stopped) { // when running then go bus off
                    /* note: here we should turn off the receiver and the transmitter,
                     *       but after CAN_Uninitialize we are really bus off! */
                    (void)CAN_Reset(can[i].board);
                }
                (void)CAN_Uninitialize(can[i].board); // resistance is futile!

                can[i].status.byte |= CANSTAT_RESET;  // CAN controller in INIT state
                can[i].board = PCAN_NONEBUS; // handle can be used again

#if defined(_WIN32) || defined(_WIN64)
                if (can[i].event != NULL)    // close event handle, if any
                    (void)CloseHandle(can[i].event);
#endif
            }
        }
    }
    return CANERR_NOERROR;
}

EXPORT
int can_kill(int handle)
{
    int i;

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (handle != CANKILL_ALL) {
        if (!IS_HANDLE_VALID(handle))   // must be a valid handle
            return CANERR_HANDLE;
        if ((can[handle].board != PCAN_NONEBUS) &&
           (can[handle].event != NULL)) {
            SetEvent(can[handle].event);  // signal event oject
        }
    }
    else {
        for (i = 0; i < PCAN_MAX_HANDLES; i++) {
            if ((can[i].board != PCAN_NONEBUS) &&
               (can[i].event != NULL))  {
                SetEvent(can[i].event); //   signal all event ojects
            }
        }
    }
    return CANERR_NOERROR;
}

EXPORT
int can_start(int handle, const can_bitrate_t *bitrate)
{
    TPCANBaudrate btr0btr1 = 0x011CU;   // btr0btr1 value
    char string[PCAN_MAX_BUFFER_SIZE];  // bit-rate string
    DWORD value;                        // parameter value
    //UINT64 filter;                       // for 29-bit filter
    TPCANStatus rc;                     // return value

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;
    if (bitrate == NULL)                // check for null-pointer
        return CANERR_NULLPTR;
    if (!can[handle].status.can_stopped) // must be stopped!
        return CANERR_ONLINE;

    if (bitrate->index <= 0) {          // btr0btr1 from index
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
    }
    else if (!can[handle].mode.fdoe) {  // btr0btr1 for CAN 2.0
        if (map_bitrate2register(bitrate, &btr0btr1) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
    }
    else {                              // a string for CAN FD
        if (map_bitrate2string(bitrate, string, can[handle].mode.brse) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
    }
    /* note: to (re-)start the CAN controller, we have to reinitialize it */
    if ((rc = CAN_Reset(can[handle].board)) != PCAN_ERROR_OK)
        return pcan_error(rc);
    if ((rc = CAN_Uninitialize(can[handle].board)) != PCAN_ERROR_OK)
        return pcan_error(rc);
    /* note: the receiver is automatically switched ON by CAN_Uninitialize() */
    if (can[handle].mode.fdoe) {        // CAN FD operation mode?
        if ((rc = CAN_InitializeFD(can[handle].board, string)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    else {                              // CAN 2.0 operation mode!
        if ((rc = CAN_Initialize(can[handle].board, btr0btr1,
                                can[handle].brd_type, can[handle].brd_port,
                                can[handle].brd_irq)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
#if defined(_WIN32) || defined(_WIN64)
    if ((rc = CAN_SetValue(can[handle].board, PCAN_RECEIVE_EVENT,
                  (void*)&can[handle].event, sizeof(can[handle].event))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
#endif
    value = (can[handle].mode.mon) ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    if ((rc = CAN_SetValue(can[handle].board, PCAN_LISTEN_ONLY,
                   (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
    value = (can[handle].mode.err) ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    if ((rc = CAN_SetValue(can[handle].board, PCAN_ALLOW_ERROR_FRAMES,
                     (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
#if (0)
    value = (can[handle].mode.nrtr) ? PCAN_PARAMETER_OFF : PCAN_PARAMETER_ON;
    if ((rc = CAN_SetValue(can[handle].board, PCAN_ALLOW_RTR_FRAMES, // TODO: fdoe?
                  (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
    filter = (can[handle].mode.nxtd) ? 0x1FFFFFFF1FFFFFFFull : 0x000000001FFFFFFFull;
    if ((rc = CAN_SetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_29BIT,
                          (void*)&filter, sizeof(filter))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
#endif
    can[handle].status.byte = 0x00;     // clear old status bits and counters
    can[handle].counters.tx = 0ull;
    can[handle].counters.rx = 0ull;
    can[handle].counters.err = 0ull;
    can[handle].status.can_stopped = 0; // CAN controller started!

    return CANERR_NOERROR;
}

EXPORT
int can_reset(int handle)
{
    TPCANStatus rc;                     // return value
    DWORD value;                        // parameter value

    if (!init)                          // must be initialized!
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if (can[handle].status.can_stopped) { // when running then go bus off
        /* note: we turn off the receiver and the transmitter to do that! */
        value = PCAN_PARAMETER_OFF;     //   receiver off
        if ((rc = CAN_SetValue(can[handle].board, PCAN_RECEIVE_STATUS, (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
            return pcan_error(rc);
        value = PCAN_PARAMETER_ON;      //   transmitter off
        if ((rc = CAN_SetValue(can[handle].board, PCAN_LISTEN_ONLY, (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    can[handle].status.can_stopped = 1; // CAN controller stopped!

    return CANERR_NOERROR;
}

EXPORT
int can_write(int handle, const can_msg_t *msg, uint16_t timeout)
{
    TPCANMsg can_msg;                   // the message (CAN 2.0)
    TPCANMsgFD can_msg_fd;              // the message (CAN FD)
    TPCANStatus rc;                     // return value

    (void)timeout;                      // TODO: "blocking write"

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
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

        rc = CAN_Write(can[handle].board, &can_msg);
    }
    else {
        if (msg->dlc > CANFD_MAX_DLC)   //   data length 0 .. 0Fh!
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

        rc = CAN_WriteFD(can[handle].board, &can_msg_fd);
    }
    if (rc != PCAN_ERROR_OK) {
        if ((rc & PCAN_ERROR_QXMTFULL)) {//   transmit queue full?
            can[handle].status.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy
        }
        if ((rc & PCAN_ERROR_XMTFULL)) {//   transmission pending?
            can[handle].status.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy
        }
        return pcan_error(rc);          //   PCAN specific error?
    }
    can[handle].status.transmitter_busy = 0; // message transmitted
    can[handle].counters.tx++;

    return CANERR_NOERROR;
}

EXPORT
int can_read(int handle, can_msg_t *msg, uint16_t timeout)
{
    TPCANMsg can_msg;                   // the message (CAN 2.0)
    TPCANTimestamp timestamp;           // time stamp (CAN 2.0)
    TPCANMsgFD can_msg_fd;              // the message (CAN FD)
    TPCANTimestampFD timestamp_fd;      // time stamp (CAN FD)
    uint64_t msec;                      // milliseconds
    TPCANStatus rc;                     // return value

    memset(&can_msg, 0, sizeof(TPCANMsg));
    memset(&timestamp, 0, sizeof(TPCANTimestamp));
    memset(&can_msg_fd, 0, sizeof(TPCANMsgFD));
    memset(&timestamp_fd, 0, sizeof(TPCANTimestampFD));

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;
    if (msg == NULL)                    // check for null-pointer
        return CANERR_NULLPTR;
    if (can[handle].status.can_stopped) // must be running
        return CANERR_OFFLINE;

    if (!can[handle].mode.fdoe)
        rc = CAN_Read(can[handle].board, &can_msg, &timestamp);
    else
        rc = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
    if (rc == PCAN_ERROR_QRCVEMPTY) {
#if defined(_WIN32) || defined(_WIN64)
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
                rc = CAN_Read(can[handle].board, &can_msg, &timestamp);
            else
                rc = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
            if (rc == PCAN_ERROR_QRCVEMPTY) {
                can[handle].status.receiver_empty = 1;
                return CANERR_RX_EMPTY; //   receiver empty
            }
        }
        else {
            can[handle].status.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty
    }
#else
        can[handle].status.receiver_empty = 1;
        return CANERR_RX_EMPTY;         //   receiver empty
#endif
    }
    /*if (rc != PCAN_ERROR_OK) { // Is this a good idea? */
    if ((rc & ~(PCAN_ERROR_ANYBUSERR |
               PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN |
               PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL))) {
        return pcan_error(rc);          //   something's wrong
    }
    if (!can[handle].mode.fdoe) {       // CAN 2.0 message:
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            can[handle].status.bus_off = (can_msg.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
            can[handle].status.bus_error = (can_msg.DATA[3] & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
            can[handle].status.warning_level = (can_msg.DATA[3] & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
            can[handle].status.message_lost |= (can_msg.DATA[3] & PCAN_ERROR_OVERRUN) != PCAN_ERROR_OK;
            can[handle].status.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty
        }
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_ERRFRAME))  {
            can[handle].status.receiver_empty = 1;
            can[handle].counters.err++;
            return CANERR_ERR_FRAME;    //   error frame received
        }
        msg->id = (int32_t)can_msg.ID;
        msg->xtd = (can_msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
        msg->rtr = (can_msg.MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
        msg->fdf = 0;
        msg->brs = 0;
        msg->esi = 0;
        msg->dlc = (uint8_t)can_msg.LEN;
        memcpy(msg->data, can_msg.DATA, CAN_MAX_LEN);
        msec = ((uint64_t)timestamp.millis_overflow << 32) + (uint64_t)timestamp.millis;
        msg->timestamp.tv_sec = (time_t)(msec / 1000ull);
        msg->timestamp.tv_nsec = ((((long)(msec % 1000ull)) * 1000L) + (long)timestamp.micros) * (long)1000;
    }
    else {                              // CAN FD message:
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            can[handle].status.bus_off = (can_msg_fd.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
            can[handle].status.bus_error = (can_msg_fd.DATA[3] & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
            can[handle].status.warning_level = (can_msg_fd.DATA[3] & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
            can[handle].status.message_lost |= (can_msg_fd.DATA[3] & PCAN_ERROR_OVERRUN) != PCAN_ERROR_OK;
            can[handle].status.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty
        }
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_ERRFRAME)) {
            can[handle].status.receiver_empty = 1;
            can[handle].counters.err++;
            return CANERR_ERR_FRAME;    //   error frame received
        }
        msg->id = (int32_t)can_msg_fd.ID;
        msg->xtd = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
        msg->rtr = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
        msg->fdf = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_FD) ? 1 : 0;
        msg->brs = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_BRS) ? 1 : 0;
        msg->esi = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_ESI) ? 1 : 0;
        msg->dlc = (uint8_t)can_msg_fd.DLC;
        memcpy(msg->data, can_msg_fd.DATA, CANFD_MAX_LEN);
        msg->timestamp.tv_sec = (time_t)(timestamp_fd / 1000000ull);
        msg->timestamp.tv_nsec = (long)(timestamp_fd % 1000000ull) * (long)1000;
    }
    can[handle].status.receiver_empty = 0; // message read
    can[handle].counters.rx++;

    return CANERR_NOERROR;
}

EXPORT
int can_status(int handle, uint8_t *status)
{
    TPCANStatus rc;                     // represents a status

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if (!can[handle].status.can_stopped) { // when running get bus status
        rc = CAN_GetStatus(can[handle].board);
        if ((rc & ~(PCAN_ERROR_ANYBUSERR |
                   PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN |
                   PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)))
            return pcan_error(rc);
        can[handle].status.bus_off = (rc & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
        can[handle].status.bus_error = (rc & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
        can[handle].status.warning_level = (rc & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
        can[handle].status.message_lost |= (rc & (PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN)) != PCAN_ERROR_OK;
        can[handle].status.transmitter_busy |= (rc & (PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)) != PCAN_ERROR_OK;
    }
    if (status)                         // status-register
      *status = can[handle].status.byte;

    return CANERR_NOERROR;
}

EXPORT
int can_busload(int handle, uint8_t *load, uint8_t *status)
{
    float busload = 0.0;                // bus-load (in [percent])

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if (!can[handle].status.can_stopped) { // when running get bus load
        (void)busload; //  TODO: measure bus load
    }
    if (load)                           // bus-load (in [percent])
        *load = (uint8_t)busload;
     return can_status(handle, status); // status-register
}

EXPORT
int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed)
{
    TPCANBaudrate btr0btr1 = 0x011CU;   // btr0btr1 value
    char string[PCAN_MAX_BUFFER_SIZE];  // bit-rate string
    can_bitrate_t temporary;            // bit-rate settings
    int rc;                             // return value

    memset(&temporary, 0, sizeof(can_bitrate_t));

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if (!can[handle].mode.fdoe) {       // CAN 2.0
        if ((rc = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO,
                             (void*)&btr0btr1, sizeof(TPCANBaudrate))) != PCAN_ERROR_OK)
            return pcan_error(rc);
        if ((rc = map_register2bitrate(btr0btr1, &temporary)) != CANERR_NOERROR)
            return rc;
    }
    else {                              // CAN FD
        if ((rc = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO_FD,
                             (void*)string, PCAN_MAX_BUFFER_SIZE)) != PCAN_ERROR_OK)
            return pcan_error(rc);
        if ((rc = map_string2bitrate(string, &temporary, can[handle].mode.brse)) != CANERR_NOERROR)
            return rc;
    }
    if (bitrate) {
        memcpy(bitrate, &temporary, sizeof(can_bitrate_t));
    }
    if (speed) {
        if ((rc = calc_speed(&temporary, speed, 0)) != CANERR_NOERROR)
            return rc;
        speed->nominal.fdoe = can[handle].mode.fdoe;
        speed->data.brse = can[handle].mode.brse;
    }
    if (!can[handle].status.can_stopped)
        rc = CANERR_NOERROR;
    else
        rc = CANERR_OFFLINE;
    return rc;
}

EXPORT
int can_property(int handle, uint16_t param, void *value, uint32_t nbyte)
{
    if (!init || !IS_HANDLE_VALID(handle)) {
        return lib_parameter(param, value, (size_t)nbyte);
    }
    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    return drv_parameter(handle, param, value, (size_t)nbyte);
}

EXPORT
char *can_hardware(int handle)
{
    static char hardware[256] = "";     // hardware version
    char  str[256], *ptr;               // info string
    DWORD dev = 0x0000UL;               // device number

    if (!init)                          // must be initialized
        return NULL;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return NULL;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return NULL;

    if (CAN_GetValue(can[handle].board, PCAN_HARDWARE_NAME, (void*)str, 256) != PCAN_ERROR_OK)
        return NULL;
    if ((ptr = strchr(str, '\n')) != NULL)
       *ptr = '\0';
    if ((((can[handle].board & 0x00F0) >> 4) == PCAN_USB) ||
       (((can[handle].board & 0x0F00) >> 8) == PCAN_USB))
    {
        if (CAN_GetValue(can[handle].board, PCAN_DEVICE_NUMBER, (void*)&dev, 4) != PCAN_ERROR_OK)
            return NULL;
        snprintf(hardware, 256, "%s, Device-Id. %02lXh", str, dev);
    }
    else
        strcpy(hardware, str);

    return (char*)hardware;             // hardware version
}

EXPORT
char *can_firmware(int handle)
{
    static char firmware[256] = "";     // firmware version
    char  str[256], *ptr;               // info string
    char  ver[256];                     // version

    if (!init)                          // must be initialized
        return NULL;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return NULL;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return NULL;

    if (CAN_GetValue(can[handle].board, PCAN_HARDWARE_NAME, (void*)str, 256) != PCAN_ERROR_OK)
        return NULL;
    if ((ptr = strchr(str, '\n')) != NULL)
        *ptr = '\0';
    if (CAN_GetValue(can[handle].board, PCAN_FIRMWARE_VERSION, (void*)ver, 256) != PCAN_ERROR_OK)
        return NULL;
    snprintf(firmware, 256, "%s, Firmware %s", str, ver);

    return (char*)firmware;             // firmware version
}

/*  -----------  local functions  ----------------------------------------
 */

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
    if ((status & PCAN_ERROR_QOVERRUN)     == PCAN_ERROR_QOVERRUN)      return CANERR_MSG_LST;
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

static TPCANStatus pcan_capability(TPCANHandle board, can_mode_t *capability)
{
    TPCANStatus rc;                     // return value
    DWORD features;                     // channel features

    assert(capability);
    capability->byte = 0x00U;

    if ((rc = CAN_GetValue((TPCANHandle)board, PCAN_CHANNEL_FEATURES,
                          (void*)&features, sizeof(features))) != PCAN_ERROR_OK)
        return rc;

    capability->fdoe = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->brse = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->niso = 0; // This can not be determined (FIXME)
    capability->shrd = 0; // This feature is not supported (PCANBasic)
#if (0)
    capability->nxtd = 1; // PCAN_ACCEPTANCE_FILTER_29BIT available since version 4.2.0
    capability->nrtr = 1; // PCAN_ALLOW_RTR_FRAMES available since version 4.2.0
#else
    capability->nxtd = 0; // This feature is not supported (TODO: acceptance filtering)
    capability->nrtr = 0; // This feature is not supported (TODO: suppress RTR frames)
#endif
    capability->err = 1;  // PCAN_ALLOW_ERROR_FRAMES available since version 4.2.0
    capability->mon = 1;  // PCAN_LISTEN_ONLY available since version 1.0.0

    return PCAN_ERROR_OK;
}

static int map_index2bitrate(int index, can_bitrate_t *bitrate)
{
    TPCANBaudrate btr0btr1 = 0x0000u;

    assert(bitrate);

    switch (index) {
    case CANBTR_INDEX_1M: btr0btr1 = PCAN_BAUD_1M; break;
    case CANBTR_INDEX_800K: btr0btr1 = PCAN_BAUD_800K; break;
    case CANBTR_INDEX_500K: btr0btr1 = PCAN_BAUD_500K; break;
    case CANBTR_INDEX_250K: btr0btr1 = PCAN_BAUD_250K; break;
    case CANBTR_INDEX_125K: btr0btr1 = PCAN_BAUD_125K; break;
    case CANBTR_INDEX_100K: btr0btr1 = PCAN_BAUD_100K; break;
    case CANBTR_INDEX_50K: btr0btr1 = PCAN_BAUD_50K; break;
    case CANBTR_INDEX_20K: btr0btr1 = PCAN_BAUD_20K; break;
    case CANBTR_INDEX_10K: btr0btr1 = PCAN_BAUD_10K; break;
    /*default: return CANERR_BAUDRATE;  // take it easy! */
    }
    return map_register2bitrate(btr0btr1, bitrate);
}

static int map_bitrate2register(const can_bitrate_t *bitrate, TPCANBaudrate *btr0btr1)
{
    assert(bitrate);
    assert(btr0btr1);

    if (bitrate->btr.frequency != (int32_t)CANBTR_FREQ_SJA1000) // SJA1000 @ 8MHz
        return CANERR_BAUDRATE;
    if ((bitrate->btr.nominal.brp < CANBTR_SJA1000_BRP_MIN) || (CANBTR_SJA1000_BRP_MAX < bitrate->btr.nominal.brp))
        return CANERR_BAUDRATE;
    if ((bitrate->btr.nominal.tseg1 < CANBTR_SJA1000_TSEG1_MIN) || (CANBTR_SJA1000_TSEG1_MAX < bitrate->btr.nominal.tseg1))
        return CANERR_BAUDRATE;
    if ((bitrate->btr.nominal.tseg2 < CANBTR_SJA1000_TSEG2_MIN) || (CANBTR_SJA1000_TSEG2_MAX < bitrate->btr.nominal.tseg2))
        return CANERR_BAUDRATE;
    if ((bitrate->btr.nominal.sjw < CANBTR_SJA1000_SJW_MIN) || (CANBTR_SJA1000_SJW_MAX < bitrate->btr.nominal.sjw))
        return CANERR_BAUDRATE;
    if (/*(bitrate->btr.nominal.sam < CANBTR_SJA1000_SAM_MIN) ||*/ (CANBTR_SJA1000_SAM_MAX < bitrate->btr.nominal.sam))
        return CANERR_BAUDRATE;
    /* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ */
    /* |  SJW  |          BRP          |SAM|   TSEG2   |     TSEG1     | */
    /* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ */
    *btr0btr1 = ((((TPCANBaudrate)(bitrate->btr.nominal.sjw - 1u) & 0x0003u) << 14) | \
                 (((TPCANBaudrate)(bitrate->btr.nominal.brp - 1u) & 0x003Fu) << 8) | \
                 (((TPCANBaudrate)(bitrate->btr.nominal.sam - 0u) & 0x0001u) << 7) | \
                 (((TPCANBaudrate)(bitrate->btr.nominal.tseg2 - 1u) & 0x0007u) << 4) | \
                 (((TPCANBaudrate)(bitrate->btr.nominal.tseg1 - 1u) & 0x000Fu) << 0));
    return CANERR_NOERROR;
}

static int map_register2bitrate(const TPCANBaudrate btr0btr1, can_bitrate_t *bitrate)
{
    assert(bitrate);

    bitrate->btr.frequency = (int32_t)CANBTR_FREQ_SJA1000; // SJA1000 @ 8MHz
    bitrate->btr.nominal.sjw = (uint16_t)((btr0btr1 & 0xC000u) >> 14) + 1u;
    bitrate->btr.nominal.brp = (uint16_t)((btr0btr1 & 0x3F00u) >> 8) + 1u;
    bitrate->btr.nominal.sam = (uint16_t)((btr0btr1 & 0x0080u) >> 7) + 0u;
    bitrate->btr.nominal.tseg2 = (uint16_t)((btr0btr1 & 0x0070u) >> 4) + 1u;
    bitrate->btr.nominal.tseg1 = (uint16_t)((btr0btr1 & 0x000Fu) >> 0) + 1u;
    bitrate->btr.data.brp = 0;
    bitrate->btr.data.tseg1 = 0;
    bitrate->btr.data.tseg2 = 0;
    bitrate->btr.data.sjw = 0;
    return CANERR_NOERROR;
}

static int map_bitrate2string(const can_bitrate_t *bitrate, TPCANBitrateFD string, int brse)
{
    assert(bitrate);
    assert(string);

    if ((bitrate->btr.nominal.brp < CANBTR_NOMINAL_BRP_MIN) || (CANBTR_NOMINAL_BRP_MAX < bitrate->btr.nominal.brp))
        return CANERR_BAUDRATE;
    if ((bitrate->btr.nominal.tseg1 < CANBTR_NOMINAL_TSEG1_MIN) || (CANBTR_NOMINAL_TSEG1_MAX < bitrate->btr.nominal.tseg1))
        return CANERR_BAUDRATE;
    if ((bitrate->btr.nominal.tseg2 < CANBTR_NOMINAL_TSEG2_MIN) || (CANBTR_NOMINAL_TSEG2_MAX < bitrate->btr.nominal.tseg2))
        return CANERR_BAUDRATE;
    if ((bitrate->btr.nominal.sjw < CANBTR_NOMINAL_SJW_MIN) || (CANBTR_NOMINAL_SJW_MAX < bitrate->btr.nominal.sjw))
        return CANERR_BAUDRATE;
    if (!brse) {     // long frames only
        if (sprintf(string, "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u",
                            bitrate->btr.frequency,
                            bitrate->btr.nominal.brp,
                            bitrate->btr.nominal.tseg1,
                            bitrate->btr.nominal.tseg2,
                            bitrate->btr.nominal.sjw,
                            bitrate->btr.nominal.sam) < 0)
            return CANERR_BAUDRATE;
    }
    else {          // long and fast frames
        if ((bitrate->btr.data.brp < CANBTR_DATA_BRP_MIN) || (CANBTR_DATA_BRP_MAX < bitrate->btr.data.brp))
            return CANERR_BAUDRATE;
        if ((bitrate->btr.data.tseg1 < CANBTR_DATA_TSEG1_MIN) || (CANBTR_DATA_TSEG1_MAX < bitrate->btr.data.tseg1))
            return CANERR_BAUDRATE;
        if ((bitrate->btr.data.tseg2 < CANBTR_DATA_TSEG2_MIN) || (CANBTR_DATA_TSEG2_MAX < bitrate->btr.data.tseg2))
            return CANERR_BAUDRATE;
        if ((bitrate->btr.data.sjw < CANBTR_DATA_SJW_MIN) || (CANBTR_DATA_SJW_MAX < bitrate->btr.data.sjw))
            return CANERR_BAUDRATE;
        if (sprintf(string, "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u,"
                                      "data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                            bitrate->btr.frequency,
                            bitrate->btr.nominal.brp,
                            bitrate->btr.nominal.tseg1,
                            bitrate->btr.nominal.tseg2,
                            bitrate->btr.nominal.sjw,
                            bitrate->btr.nominal.sam,
                            bitrate->btr.data.brp,
                            bitrate->btr.data.tseg1,
                            bitrate->btr.data.tseg2,
                            bitrate->btr.data.sjw) < 0)
            return CANERR_BAUDRATE;
    }
    return CANERR_NOERROR;
}

static int map_string2bitrate(const TPCANBitrateFD string, can_bitrate_t *bitrate, int brse)
{
    long unsigned freq = 0;
    int unsigned nom_brp = 0, nom_tseg1 = 0, nom_tseg2 = 0, nom_sjw = 0/*, nom_sam = 0*/;
    int unsigned data_brp = 0, data_tseg1 = 0, data_tseg2 = 0, data_sjw = 0/*, data_ssp_offset = 0*/;

    assert(string);
    assert(bitrate);

    // TODO: rework this!
    if (sscanf(string, "f_clock=%lu,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,"
                                  "data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                            &freq, &nom_brp, &nom_tseg1, &nom_tseg2, &nom_sjw,
                                  &data_brp, &data_tseg1, &data_tseg2, &data_sjw) != 9)
        return CANERR_BAUDRATE;
    bitrate->btr.frequency = (int32_t)freq;
    bitrate->btr.nominal.brp = (uint16_t)nom_brp;
    bitrate->btr.nominal.tseg1 = (uint16_t)nom_tseg1;
    bitrate->btr.nominal.tseg2 = (uint16_t)nom_tseg2;
    bitrate->btr.nominal.sjw = (uint16_t)nom_sjw;
    if (brse) {
        bitrate->btr.data.brp = (uint16_t)data_brp;
        bitrate->btr.data.tseg1 = (uint16_t)data_tseg1;
        bitrate->btr.data.tseg2 = (uint16_t)data_tseg2;
        bitrate->btr.data.sjw = (uint16_t)data_sjw;
    }
    else {
        bitrate->btr.data.brp = (uint16_t)0;
        bitrate->btr.data.tseg1 = (uint16_t)0;
        bitrate->btr.data.tseg2 = (uint16_t)0;
        bitrate->btr.data.sjw = (uint16_t)0;
    }
    return CANERR_NOERROR;
}

/*  - - - - - -  CAN API V3 properties  - - - - - - - - - - - - - - - - -
 */
static int lib_parameter(uint16_t param, void *value, size_t nbyte)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter

    static int idx_board = EOF;         // actual index in the interface list
    TPCANStatus sts;                    // status or error code

    if (value == NULL) {                // check for null-pointer
        if ((param != CANPROP_SET_FIRST_CHANNEL) &&
           (param != CANPROP_SET_NEXT_CHANNEL))
            return CANERR_NULLPTR;
    }
    /* CAN library properties */
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
            *(int32_t*)value = (int32_t)PCAN_LIB_ID;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_VENDOR:    // vendor name of the library (char[256])
        if ((nbyte > strlen(CAN_API_VENDOR)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            strcpy((char*)value, CAN_API_VENDOR);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_DLLNAME:   // file name of the library (char[256])
        if ((nbyte > strlen(PCAN_LIB_WRAPPER)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_WRAPPER);
            rc = CANERR_NOERROR;
        }
        break;
	/* *** **
    case CANPROP_GET_DEVICE_VENDOR:     // vendor name of the CAN interface (char[256])
        if ((nbyte > strlen(PCAN_LIB_VENDOR)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_VENDOR);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_DLLNAME:    // file name of the CAN interface DLL (char[256])
        if ((nbyte > strlen(PCAN_LIB_BASIC)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_BASIC);
            rc = CANERR_NOERROR;
        }
        break;
	** *** */
    case CANPROP_SET_FIRST_CHANNEL:     // set index to the first entry in the interface list (NULL)
        idx_board = 0;
        rc = (can_boards[idx_board].type != EOF) ? CANERR_NOERROR : CANERR_RESOURCE;
        break;
    case CANPROP_SET_NEXT_CHANNEL:      // set index to the next entry in the interface list (NULL)
        if ((0 <= idx_board) && (idx_board < PCAN_BOARDS)) {
            if (can_boards[idx_board].type != EOF)
                idx_board++;
            rc = (can_boards[idx_board].type != EOF) ? CANERR_NOERROR : CANERR_RESOURCE;
        }
        else
            rc = CANERR_RESOURCE;
        break;
    case CANPROP_GET_CHANNEL_NO:        // get channel no. at actual index in the interface list (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            if ((0 <= idx_board) && (idx_board < PCAN_BOARDS) &&
                (can_boards[idx_board].type != EOF)) {
                *(int32_t*)value = (int32_t)can_boards[idx_board].type;
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_NAME:      // get channel name at actual index in the interface list (char[256])
        if ((0U < nbyte) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            if ((0 <= idx_board) && (idx_board < PCAN_BOARDS) &&
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, can_boards[idx_board].name, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_DLLNAME:   // get file name of the DLL at actual index in the interface list (char[256])
        if ((0U < nbyte) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            if ((0 <= idx_board) && (idx_board < PCAN_BOARDS) &&
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, PCAN_LIB_BASIC, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_VENDOR_ID: // get library id at actual index in the interface list (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            if ((0 <= idx_board) && (idx_board < PCAN_BOARDS) &&
                (can_boards[idx_board].type != EOF)) {
                *(int32_t*)value = (int32_t)PCAN_LIB_ID;
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_VENDOR_NAME: // get vendor name at actual index in the interface list (char[256])
        if ((0U < nbyte) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            if ((0 <= idx_board) && (idx_board < PCAN_BOARDS) &&
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, PCAN_LIB_VENDOR, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_DEVICE_TYPE:       // device type of the CAN interface (int32_t)
    case CANPROP_GET_DEVICE_NAME:       // device name of the CAN interface (char[256])
    case CANPROP_GET_DEVICE_VENDOR:     // vendor name of the CAN interface (char[256])
    case CANPROP_GET_DEVICE_DLLNAME:    // file name of the CAN interface DLL (char[256])
    case CANPROP_GET_OP_CAPABILITY:     // supported operation modes of the CAN controller (uint8_t)
    case CANPROP_GET_OP_MODE:           // active operation mode of the CAN controller (uint8_t)
    case CANPROP_GET_BITRATE:           // active bit-rate of the CAN controller (can_bitrate_t)
    case CANPROP_GET_SPEED:             // active bus speed of the CAN controller (can_speed_t)
    case CANPROP_GET_STATUS:            // current status register of the CAN controller (uint8_t)
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint8_t)
    case CANPROP_GET_NUM_CHANNELS:      // numbers of CAN channels on the CAN interface (uint8_t)
    case CANPROP_GET_CAN_CHANNEL:       // active CAN channel on the CAN interface (uint8_t)
    case CANPROP_GET_CAN_CLOCKS:        // supported CAN clocks (in [Hz]) (int32_t[64])
    case CANPROP_GET_TX_COUNTER:        // total number of sent messages (uint64_t)
    case CANPROP_GET_RX_COUNTER:        // total number of reveiced messages (uint64_t)
    case CANPROP_GET_ERR_COUNTER:       // total number of reveiced error frames (uint64_t)
    case CANPROP_GET_RCV_QUEUE_SIZE:    // maximum number of message the receive queue can hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_HIGH:    // maximum number of message the receive queue has hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_OVFL:    // overflow counter of the receive queue (uint64_t)
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
    can_bitrate_t bitrate;
    can_speed_t speed;
    can_mode_t mode;
    uint8_t status = 0U;
    uint8_t load = 0U;
    TPCANStatus sts;

    assert(IS_HANDLE_VALID(handle));    // just to make sure

    if (value == NULL) {                // check for null-pointer
        if ((param != CANPROP_SET_FIRST_CHANNEL) &&
           (param != CANPROP_SET_NEXT_CHANNEL))
        return CANERR_NULLPTR;
    }
    /* CAN interface properties */
    switch (param) {
    case CANPROP_GET_DEVICE_TYPE:       // device type of the CAN interface (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)can[handle].board;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_NAME:       // device name of the CAN interface (char[256])
        if (nbyte <= CANPROP_MAX_BUFFER_SIZE) {
            if ((sts = CAN_GetValue(can[handle].board, (BYTE)PCAN_HARDWARE_NAME,
                (void*)value, (DWORD)nbyte)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        break;
    case CANPROP_GET_DEVICE_VENDOR:     // vendor name of the CAN interface (char[256])
        if ((nbyte > strlen(PCAN_LIB_VENDOR)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_VENDOR);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_DLLNAME:    // file name of the CAN interface DLL (char[256])
        if ((nbyte > strlen(PCAN_LIB_BASIC)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_BASIC);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_PARAM:      // device parameter of the CAN interface (char[256])
        if (nbyte >= sizeof(struct _pcan_param)) {
            ((struct _pcan_param*)value)->type = (uint8_t)can[handle].brd_type;
            ((struct _pcan_param*)value)->port = (uint32_t)can[handle].brd_port;
            ((struct _pcan_param*)value)->irq = (uint16_t)can[handle].brd_irq;
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
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            if ((rc = can_busload(handle, &load, NULL)) == CANERR_NOERROR) {
                *(uint8_t*)value = (uint8_t)load;
                rc = CANERR_NOERROR;
            }
        }
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

/*  - - - - - -  Bus-speed calculator  - - - - - - - - - - - - - - - - - -
 */
static int calc_speed(can_bitrate_t *bitrate, can_speed_t *speed, int modify)
{
    can_bitrate_t temporary;            // bit-rate settings
    int rc;

    assert(bitrate);
    assert(speed);
    memset(&temporary, 0, sizeof(can_bitrate_t));

    if (bitrate->index <= 0) {
        if ((rc = map_index2bitrate(bitrate->index, &temporary)) != CANERR_NOERROR)
            return rc;
        if (modify)                     // translate index to bit-rate
            memcpy(bitrate, &temporary, sizeof(can_bitrate_t));

        speed->nominal.fdoe = 0;
        speed->data.brse = 0;
    }
    else {
        memcpy(&temporary, bitrate, sizeof(can_bitrate_t));

        speed->data.brse = temporary.btr.data.brp ? 1 : 0;
    }
    /* nominal bit-rate:
     *
     * (1) speed = freq / (brp * (1 + tseg1 +tseg2))
     *
     * (2) sp = (1 + tseg1) / (1 + tseg1 +tseg2)
     */
    if ((temporary.btr.nominal.brp < CANBTR_NOMINAL_BRP_MIN) || (CANBTR_NOMINAL_BRP_MAX < temporary.btr.nominal.brp))
        return CANERR_BAUDRATE;
    if ((temporary.btr.nominal.tseg1 < CANBTR_NOMINAL_TSEG1_MIN) || (CANBTR_NOMINAL_TSEG1_MAX < temporary.btr.nominal.tseg1))
        return CANERR_BAUDRATE;
    if ((temporary.btr.nominal.tseg2 < CANBTR_NOMINAL_TSEG2_MIN) || (CANBTR_NOMINAL_TSEG2_MAX < temporary.btr.nominal.tseg2))
        return CANERR_BAUDRATE;
    if ((temporary.btr.nominal.sjw < CANBTR_NOMINAL_SJW_MIN) || (CANBTR_NOMINAL_SJW_MAX < temporary.btr.nominal.sjw))
        return CANERR_BAUDRATE;
    speed->nominal.speed = (float)(temporary.btr.frequency)
                         / (float)(temporary.btr.nominal.brp * (1u + temporary.btr.nominal.tseg1 + temporary.btr.nominal.tseg2));
    speed->nominal.samplepoint = (float)(1u + temporary.btr.nominal.tseg1)
                               / (float)(1u + temporary.btr.nominal.tseg1 + temporary.btr.nominal.tseg2);

    /* data bit-rate (CAN FD only):
     *
     * (1) speed = freq / (brp * (1 + tseg1 +tseg2))
     *
     * (2) sp = (1 + tseg1) / (1 + tseg1 +tseg2)
     */
    if (speed->data.brse) {
        if ((temporary.btr.data.brp < CANBTR_DATA_BRP_MIN) || (CANBTR_DATA_BRP_MAX < temporary.btr.data.brp))
            return CANERR_BAUDRATE;
        if ((temporary.btr.data.tseg1 < CANBTR_DATA_TSEG1_MIN) || (CANBTR_DATA_TSEG1_MAX < temporary.btr.data.tseg1))
            return CANERR_BAUDRATE;
        if ((temporary.btr.data.tseg2 < CANBTR_DATA_TSEG2_MIN) || (CANBTR_DATA_TSEG2_MAX < temporary.btr.data.tseg2))
            return CANERR_BAUDRATE;
        if ((temporary.btr.data.sjw < CANBTR_DATA_SJW_MIN) || (CANBTR_DATA_SJW_MAX < temporary.btr.data.sjw))
            return CANERR_BAUDRATE;
        speed->data.speed = (float)(temporary.btr.frequency)
                          / (float)(temporary.btr.data.brp * (1u + temporary.btr.data.tseg1 + temporary.btr.data.tseg2));
        speed->data.samplepoint = (float)(1u + temporary.btr.data.tseg1)
                                / (float)(1u + temporary.btr.data.tseg1 + temporary.btr.data.tseg2);
    }
    else {
        speed->data.speed = 0.0;
        speed->data.samplepoint = 0.0;
    }
    return CANERR_NOERROR;
}

/*  -----------  revision control  ---------------------------------------
 */

EXPORT
char* can_version(void)
{
    return (char*)version;
}
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
