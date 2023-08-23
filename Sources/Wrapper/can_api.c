/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (for PEAK PCAN Interfaces)
 *
 *  Copyright (c) 2005-2010 Uwe Vogt, UV Software, Friedrichshafen
 *  Copyright (c) 2014-2023 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
#define VERSION_PATCH    99
#else
#define VERSION_MAJOR    0
#define VERSION_MINOR    2
#define VERSION_PATCH    4
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
static const char version[] = "CAN API V3 for Peak-System PCAN Interfaces, Version " VERSION_STRING;


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
#include "can_btr.h"

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
#ifndef SYSERR_OFFSET
#define SYSERR_OFFSET           (-10000)
#endif

/*  -----------  types  --------------------------------------------------
 */

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
#endif
    can_mode_t mode;                    //   operation mode of the CAN channel
    can_status_t status;                //   8-bit status register
    can_error_t error;                  //   error code capture
    can_counter_t counters;             //   statistical counters
}   can_interface_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static int pcan_error(TPCANStatus);     // PCAN specific errors
static int pcan_compatibility(void);    // PCAN compatibility check
static TPCANStatus pcan_capability(TPCANHandle board, can_mode_t *capability);
static int lib_parameter(uint16_t param, void *value, size_t nbyte);
static int drv_parameter(int handle, uint16_t param, void *value, size_t nbyte);


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
            can[i].brd_type = 0u;
            can[i].brd_port = 0u;
            can[i].brd_irq = 0u;
#if defined(_WIN32) || defined(_WIN64)
            can[i].event = NULL;
#endif
            can[i].mode.byte = CANMODE_DEFAULT;
            can[i].status.byte = CANSTAT_RESET;
            can[i].error.lec = 0x00u;
            can[i].error.rx_err = 0u;
            can[i].error.tx_err = 0u;
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
    for (i = 0; i < PCAN_MAX_HANDLES; i++) {  // any open handle?
        if (can[i].board != PCAN_NONEBUS)
            break;
    }
    if (i == PCAN_MAX_HANDLES) {        // if no open handle then
        init = 0;                       //   clear initialization flag
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
            can[i].brd_type = 0u;
            can[i].brd_port = 0u;
            can[i].brd_irq = 0u;
#if defined(_WIN32) || defined(_WIN64)
            can[i].event = NULL;
#endif
            can[i].mode.byte = CANMODE_DEFAULT;
            can[i].status.byte = CANSTAT_RESET;
            can[i].error.lec = 0x00u;
            can[i].error.rx_err = 0u;
            can[i].error.tx_err = 0u;
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

    /* check for minimum required library version */
    if ((rc = pcan_compatibility()) != PCAN_ERROR_OK)
        return rc;
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
    for (i = 0; i < PCAN_MAX_HANDLES; i++) {  // any open handle?
        if (can[i].board != PCAN_NONEBUS)
            break;
    }
    if (i == PCAN_MAX_HANDLES) {        // if no open handle then
        init = 0;                       //   clear initialization flag
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
        if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
            return CANERR_HANDLE;
        if (can[handle].event != NULL)
            SetEvent(can[handle].event);  // signal event oject
    }
    else {
        for (i = 0; i < PCAN_MAX_HANDLES; i++) {
            if ((can[i].board != PCAN_NONEBUS) // must be an opened handle
            &&  (can[i].event != NULL)) {
                SetEvent(can[i].event); //   signal all event ojects
            }
        }
    }
    return CANERR_NOERROR;
}

EXPORT
int can_start(int handle, const can_bitrate_t *bitrate)
{
    uint16_t btr0btr1 = BTR0BTR1_DEFAULT;  // btr0btr1 value
    char string[PCAN_MAX_BUFFER_SIZE];  // bit-rate string
    DWORD value;                        // parameter value
    //UINT64 filter;                       // for 29-bit filter
    TPCANStatus rc;                     // return value

    strcpy(string, "");                 // empty string

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
        if (btr_check_bitrate(bitrate, can[handle].mode.fdoe, can[handle].mode.brse) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
        if (btr_bitrate2string(bitrate, can[handle].mode.brse, false, string, PCAN_MAX_BUFFER_SIZE) != CANERR_NOERROR)
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
    can[handle].status.byte = 0x00u;    // clear old status, errors and counters
    can[handle].error.lec = 0x00u;
    can[handle].error.rx_err = 0u;
    can[handle].error.tx_err = 0u;
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

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if (!can[handle].status.can_stopped) { // when running then go bus off
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
    memset(msg, 0, sizeof(can_msg_t));  // invalidate the message
    msg->id = 0xFFFFFFFFU;
    msg->sts = 1;

repeat:
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
    if ((rc & ~(PCAN_ERROR_ANYBUSERR | PCAN_ERROR_QOVERRUN))) {
        can[handle].status.receiver_empty = 1;
        return pcan_error(rc);          // something went wrong
    }
    else if ((rc & ~PCAN_ERROR_ANYBUSERR) == PCAN_ERROR_QOVERRUN) {
        can[handle].status.queue_overrun = 1;
        /* note: queue has overrun, but we have a message */
    }
    if (!can[handle].mode.fdoe) {       // CAN 2.0 message:
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) && can[handle].mode.nxtd)
            goto repeat;                //   refuse extended frames
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_RTR) && can[handle].mode.nrtr)
            goto repeat;                //   refuse remote frames
        if ((can_msg.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            /* update status register from status frame */
            can[handle].status.bus_off = ((can_msg.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK) ? 1 : 0;
            can[handle].status.warning_level = ((can_msg.DATA[3] & PCAN_ERROR_BUSHEAVY) != PCAN_ERROR_OK) ? 1 : 0;
            /* refuse status message if suppressed by user */
            if (!can[handle].mode.err)
                goto repeat;
            /* status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors) */
            msg->id = (int32_t)0;
            msg->xtd = 0;
            msg->rtr = 0;
            msg->fdf = 0;
            msg->brs = 0;
            msg->esi = 0;
            msg->sts = 1;
            msg->dlc = 4u;
            msg->data[0] = can[handle].status.byte;
            msg->data[1] = can[handle].error.lec;
            msg->data[2] = can[handle].error.rx_err;
            msg->data[4] = can[handle].error.tx_err;
            /* update error counter */
            can[handle].counters.err++;
        }
        else if ((can_msg.MSGTYPE & PCAN_MESSAGE_ERRFRAME))  {
            /* update status register from error frame */
            can[handle].error.lec = (uint8_t)can_msg.ID;  // TODO: 82527 encoding
            can[handle].error.rx_err = can_msg.DATA[2];
            can[handle].error.tx_err = can_msg.DATA[3];
            can[handle].status.bus_error = can[handle].error.lec ? 1 : 0;
            /* refuse status message if suppressed by user */
            if (!can[handle].mode.err)
                goto repeat;
            /* status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors) */
            msg->id = (int32_t)0;
            msg->xtd = 0;
            msg->rtr = 0;
            msg->fdf = 0;
            msg->brs = 0;
            msg->esi = 0;
            msg->sts = 1;
            msg->dlc = 4u;
            msg->data[0] = can[handle].status.byte;
            msg->data[1] = can[handle].error.lec;
            msg->data[2] = can[handle].error.rx_err;
            msg->data[4] = can[handle].error.tx_err;
            /* update error counter */
            can[handle].counters.err++;
        }
        else {
            /* decode PEAK CAN 2.0 message */
            msg->id = (int32_t)can_msg.ID;
            msg->xtd = (can_msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
            msg->rtr = (can_msg.MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
            msg->fdf = 0;
            msg->brs = 0;
            msg->esi = 0;
            msg->sts = 0;
            msg->dlc = (uint8_t)can_msg.LEN;
            memcpy(msg->data, can_msg.DATA, CAN_MAX_LEN);
            /* update message counter */
            can[handle].counters.rx++;
        }
        /* time-stamp in nanoseconds since start of Windows */
        msec = ((uint64_t)timestamp.millis_overflow << 32) + (uint64_t)timestamp.millis;
        msg->timestamp.tv_sec = (time_t)(msec / 1000ull);
        msg->timestamp.tv_nsec = ((((long)(msec % 1000ull)) * 1000L) + (long)timestamp.micros) * (long)1000;
    }
    else {                              // CAN FD message:
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_EXTENDED) && can[handle].mode.nxtd)
            goto repeat;                //   refuse extended frames
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_RTR) && can[handle].mode.nrtr)
            goto repeat;                //   refuse remote frames (n/a w/ fdoe)
        if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            /* update status register from status frame */
            can[handle].status.bus_off = ((can_msg_fd.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK) ? 1 : 0;
            can[handle].status.warning_level = ((can_msg_fd.DATA[3] & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK) ? 1 : 0;
            /* refuse status message if suppressed by user */
            if (!can[handle].mode.err)
                goto repeat;
            /* status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors) */
            msg->id = (int32_t)0;
            msg->xtd = 0;
            msg->rtr = 0;
            msg->fdf = 0;
            msg->brs = 0;
            msg->esi = 0;
            msg->sts = 1;
            msg->dlc = 4u;
            msg->data[0] = can[handle].status.byte;
            msg->data[1] = can[handle].error.lec;
            msg->data[2] = can[handle].error.rx_err;
            msg->data[4] = can[handle].error.tx_err;
            /* update error counter */
            can[handle].counters.err++;
        }
        else if ((can_msg_fd.MSGTYPE & PCAN_MESSAGE_ERRFRAME)) {
            /* update status register from error frame */
            can[handle].error.lec = (uint8_t)can_msg_fd.ID;  // TODO: 82527 encoding
            can[handle].error.rx_err = can_msg_fd.DATA[2];
            can[handle].error.tx_err = can_msg_fd.DATA[3];
            can[handle].status.bus_error = can[handle].error.lec ? 1 : 0;
            /* refuse status message if suppressed by user */
            if (!can[handle].mode.err)
                goto repeat;
            /* status message: ID=000h, DLC=4 (status, lec, rx errors, tx errors) */
            msg->id = (int32_t)0;
            msg->xtd = 0;
            msg->rtr = 0;
            msg->fdf = 0;
            msg->brs = 0;
            msg->esi = 0;
            msg->sts = 1;
            msg->dlc = 4u;
            msg->data[0] = can[handle].status.byte;
            msg->data[1] = can[handle].error.lec;
            msg->data[2] = can[handle].error.rx_err;
            msg->data[4] = can[handle].error.tx_err;
            /* update error counter */
            can[handle].counters.err++;
        }
        else {
            /* decode PEAK CAN FD message */
            msg->id = (int32_t)can_msg_fd.ID;
            msg->xtd = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
            msg->rtr = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
            msg->fdf = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_FD) ? 1 : 0;
            msg->brs = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_BRS) ? 1 : 0;
            msg->esi = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_ESI) ? 1 : 0;
            msg->sts = 0;
            msg->dlc = (uint8_t)can_msg_fd.DLC;
            memcpy(msg->data, can_msg_fd.DATA, CANFD_MAX_LEN);
            /* update message counter */
            can[handle].counters.rx++;
        }
        /* time-stamp in nanoseconds since start of Windows */
        msg->timestamp.tv_sec = (time_t)(timestamp_fd / 1000000ull);
        msg->timestamp.tv_nsec = (long)(timestamp_fd % 1000000ull) * (long)1000;
    }
    can[handle].status.receiver_empty = 0; // message read

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
        can[handle].status.bus_off = ((rc & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.bus_error = can[handle].error.lec ? 1 : 0;  // last eror code from error code capture (ECC)
        can[handle].status.warning_level = ((rc & (PCAN_ERROR_BUSWARNING/*PCAN_ERROR_BUSHEAVY*/)) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.transmitter_busy |= ((rc & (PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.message_lost |= ((rc & PCAN_ERROR_OVERRUN) != PCAN_ERROR_OK) ? 1 : 0;
        can[handle].status.queue_overrun |= ((rc & PCAN_ERROR_QOVERRUN) != PCAN_ERROR_OK) ? 1 : 0;
    }
    if (status)                         // parameter 'status' is optional
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
    uint16_t btr0btr1 = BTR0BTR1_DEFAULT;  // btr0btr1 value
    char string[PCAN_MAX_BUFFER_SIZE];  // bit-rate string
    can_bitrate_t temporary;            // bit-rate settings
    bool data = false,sam = false;      // no further usage
    int rc;                             // return value

    memset(&temporary, 0, sizeof(can_bitrate_t));

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if (!can[handle].mode.fdoe) {       // CAN 2.0: read BTR0BTR1 register
        if ((rc = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO,
                             (void*)&btr0btr1, sizeof(TPCANBaudrate))) != PCAN_ERROR_OK)
            return pcan_error(rc);
        if ((rc = btr_sja10002bitrate(btr0btr1, &temporary)) != CANERR_NOERROR)
            return rc;
    }
    else {                              // CAN FD: read PCAN bit-rate string
        if ((rc = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO_FD,
                             (void*)string, PCAN_MAX_BUFFER_SIZE)) != PCAN_ERROR_OK)
            return pcan_error(rc);
        if ((rc = btr_string2bitrate(string, &temporary, &data, &sam)) != CANERR_NOERROR)
            return rc;
    }
    if (bitrate) {                      // parameter 'bitrate' is optional
        memcpy(bitrate, &temporary, sizeof(can_bitrate_t));
    }
    if (speed) {                        // parameter 'speed' is optional
        if ((rc = btr_bitrate2speed(&temporary, speed)) != CANERR_NOERROR)
            return rc;
    }
    if (!can[handle].status.can_stopped)// result not guaranteed if not started
        rc = CANERR_NOERROR;
    else
        rc = CANERR_OFFLINE;
    return rc;
}

EXPORT
int can_property(int handle, uint16_t param, void *value, uint32_t nbyte)
{
    if (!init || !IS_HANDLE_VALID(handle)) {
        // note: library properties can be queried w/o a handle
        return lib_parameter(param, value, (size_t)nbyte);
    }
    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;
    // note: device properties must be queried with a valid handle
    return drv_parameter(handle, param, value, (size_t)nbyte);
}

EXPORT
char *can_hardware(int handle)
{
    static char hardware[256] = "";     // hardware version
    char  str[256] = "", *ptr;          // info string
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
    char  str[256] = "", *ptr;          // info string
    char  ver[256] = "";                // version

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
    TPCANStatus sts;                    // channel status
    unsigned int major = 0, minor = 0;  // channel version
    char api[256] = "0.0.0.0";          // PCANBasic version

    /* get library version (as a string) */
    if ((sts = CAN_GetValue(PCAN_NONEBUS, PCAN_API_VERSION, (void*)api, 256)) != PCAN_ERROR_OK)
        return pcan_error(sts);
    /* extract major and minor revision */
    if (sscanf(api, "%u.%u", &major, &minor) != 2)
        return CANERR_FATAL;
    /* check for minimal required version */
    if ((major != PCAN_LIB_MIN_MAJOR) || (minor < PCAN_LIB_MIN_MINOR))
        return CANERR_LIBRARY;

    return CANERR_NOERROR;
}

static TPCANStatus pcan_capability(TPCANHandle board, can_mode_t *capability)
{
    TPCANStatus sts;                    // channel status
    DWORD features;                     // channel features

    assert(capability);
    capability->byte = 0x00U;

    if ((sts = CAN_GetValue((TPCANHandle)board, PCAN_CHANNEL_FEATURES,
                            (void*)&features, sizeof(features))) != PCAN_ERROR_OK)
        return sts;

    capability->fdoe = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->brse = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->niso = 0; // This can not be determined (FIXME)
    capability->shrd = 0; // This feature is not supported (PCANBasic)
#if (0)
    capability->nxtd = 1; // PCAN_ACCEPTANCE_FILTER_29BIT available since version 4.2.0
    capability->nrtr = 1; // PCAN_ALLOW_RTR_FRAMES available since version 4.2.0
#else
    capability->nxtd = 1; // Suppress XTD frames (software solution)
    capability->nrtr = 1; // Suppress RTR frames (software solution)
#endif
    capability->err = 1;  // PCAN_ALLOW_ERROR_FRAMES available since version 4.2.0
    capability->mon = 1;  // PCAN_LISTEN_ONLY available since version 1.0.0

    return PCAN_ERROR_OK;
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
            *(int32_t*)value = (int32_t)PCAN_BOARD_TYPE(can[handle].board);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_NAME:       // device name of the CAN interface (char[256])
        if ((nbyte >= MAX_LENGTH_HARDWARE_NAME) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
            if ((sts = CAN_GetValue(can[handle].board, (BYTE)PCAN_HARDWARE_NAME,
                (void*)value, (DWORD)MAX_LENGTH_HARDWARE_NAME)) == PCAN_ERROR_OK)
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
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint16_t)
        if (nbyte >= sizeof(uint8_t)) {
            if ((rc = can_busload(handle, &load, NULL)) == CANERR_NOERROR) {
                if (nbyte > sizeof(uint8_t))
                    *(uint16_t*)value = (uint16_t)load * 100U;  // 0..10000 ==> 0.00%..100.00%
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
