/*  -- $HeadURL$ --
 *
 *  project   :  CAN - Controller Area Network
 *
 *  purpose   :  CAN Interface API, Version 3 (PCAN-Basic)
 *
 *  copyright :  (C) 2010, UV Software, Friedrichshafen
 *               (C) 2014, 2017-2019, UV Software, Berlin
 *
 *  compiler  :  Microsoft Visual C/C++ Compiler (Version 19.16)
 *
 *  export    :  (see header file)
 *
 *  includes  :  can_api.h (can_defs.h), PCANBasic.h
 *
 *  author    :  Uwe Vogt, UV Software
 *
 *  e-mail    :  uwe.vogt@uv-software.de
 *
 *
 *  -----------  description  --------------------------------------------
 */
/** @file        can_api.h
 *
 *  @brief       CAN API V3 for PEAK PCAN-Basic Interfaces - API
 *
 *  @author      $Author$
 *
 *  @version     $Rev$
 *
 *  @addtogroup  can_api
 *  @{
 */
/*  -----------  version  ------------------------------------------------
 */

#include "can_vers.h"

#ifdef _MSC_VER
#define VERSION_MAJOR     3
#define VERSION_MINOR     3
#define VERSION_PATCH     0
#else
#define VERSION_MAJOR     0
#define VERSION_MINOR     2
#define VERSION_PATCH     0
#endif
#define VERSION_BUILD     BUILD_NO
#define VERSION_STRING    TOSTRING(VERSION_MAJOR)"." TOSTRING(VERSION_MINOR)"."TOSTRING(VERSION_PATCH)"-"TOSTRING(BUILD_NO)
#if defined(_WIN64)
#define PLATFORM    "x64"
#elif defined(_WIN32)
#define PLATFORM    "x86"
#elif defined(__linux__)
#define PLATFORM    "Linux"
#elif defined(__APPLE__)
#define PLATFORM    "macOS"
#elif defined(__MINGW32__)
#define PLATFORM    "MinGW"
#else
#error Unsupported architecture
#endif
#ifdef _DEBUG
    static char _id[] = "CAN API V3 for PEAK PCAN-Basic Interfaces, Version "VERSION_STRING" ("PLATFORM") _DEBUG";
#else
    static char _id[] = "CAN API V3 for PEAK PCAN-Basic Interfaces, Version "VERSION_STRING" ("PLATFORM")";
#endif

/*  -----------  includes  -----------------------------------------------
 */

#include "can_api.h"

#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <windows.h>

#include "PCANBasic.h"


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#ifndef PCAN_MAX_HANDLES
#define PCAN_MAX_HANDLES        (16)    // maximum number of open handles
#endif
#define INVALID_HANDLE          (-1)
#define IS_HANDLE_VALID(hnd)    ((0 <= (hnd)) && ((hnd) < PCAN_MAX_HANDLES))
#ifndef DLC2LEN
#define DLC2LEN(x)              dlc_table[x & 0xF]
#endif
#ifdef  CANAPI_CiA_BIT_TIMING
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
#ifndef QWORD
#define QWORD                   unsigned long long
#endif

/*  -----------  types  --------------------------------------------------
 */

typedef struct {
    QWORD tx;                           // number of transmitted CAN frames
    QWORD rx;                           // number of received CAN frames
    QWORD err;                          // number of receiced error frames
}   can_counter_t;

typedef struct {
    TPCANHandle board;                  // board hardware channel handle
    BYTE  brd_type;                     // board type (none PnP hardware)
    DWORD brd_port;                     // board parameter: I/O port address
    WORD  brd_irq;                      // board parameter: interrupt number
#if defined(_WIN32) || defined(_WIN64)
    HANDLE event;                       // event handle for blocking read
#endif
    can_mode_t mode;                    // operation mode of the CAN channel
    can_status_t status;                // 8-bit status register
    can_counter_t counters;             // statistical counters
}   can_interface_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static int pcan_error(TPCANStatus);     // PCAN specific errors
static int pcan_capability(WORD board, can_mode_t *capability);

static int bitrate2register(const can_bitrate_t *bitrate, TPCANBaudrate *btr0btr1);
static int register2bitrate(const TPCANBaudrate btr0btr1, can_bitrate_t *bitrate);
static int bitrate2string(const can_bitrate_t *bitrate, TPCANBitrateFD string, int brse);
static int string2bitrate(const TPCANBitrateFD string, can_bitrate_t *bitrate, int brse);

static int lib_parameter(int param, void *value, size_t nbytes);
static int drv_parameter(int handle, int param, void *value, size_t nbytes);

static int calc_speed(can_bitrate_t *bitrate, can_speed_t *speed, int modify);


/*  -----------  variables  ----------------------------------------------
 */

#ifdef _CANAPI_EXPORTS
#define ATTRIB  __declspec(dllexport)
#else
#define ATTRIB
#endif
ATTRIB can_board_t can_board[PCAN_BOARDS]=// list of CAN Interface boards:
{
    {PCAN_USB1,                           "PCAN-USB1"},
    {PCAN_USB2,                           "PCAN-USB2"},
    {PCAN_USB3,                           "PCAN-USB3"},
    {PCAN_USB4,                           "PCAN-USB4"},
    {PCAN_USB5,                           "PCAN-USB5"},
    {PCAN_USB6,                           "PCAN-USB6"},
    {PCAN_USB7,                           "PCAN-USB7"},
    {PCAN_USB8,                           "PCAN-USB8"},
    {PCAN_USB9,                           "PCAN-USB9"},
    {PCAN_USB10,                          "PCAN-USB10"},
    {PCAN_USB11,                          "PCAN-USB11"},
    {PCAN_USB12,                          "PCAN-USB12"},
    {PCAN_USB13,                          "PCAN-USB13"},
    {PCAN_USB14,                          "PCAN-USB14"},
    {PCAN_USB15,                          "PCAN-USB15"},
    {PCAN_USB16,                          "PCAN-USB16"}
};
static const BYTE dlc_table[16] = {     // DLC to length
    0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64
};
static can_interface_t can[PCAN_MAX_HANDLES]; // interface handles
static int init = 0;                    // initialization flag


/*  -----------  functions  ----------------------------------------------
 */

int can_test(int board, unsigned char mode, const void *param, int *result)
{
    TPCANStatus rc;                     // return value
    DWORD condition;                    // channel condition
    DWORD features;                     // channel features
    int used = 0;                       // own used channel
    int i;

    if(!init) {                         // when not init before:
        for(i = 0; i < PCAN_MAX_HANDLES; i++) {
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
    if((rc = CAN_GetValue((WORD)board, PCAN_CHANNEL_CONDITION,
                          (void*)&condition, sizeof(condition))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    for(i = 0; i < PCAN_MAX_HANDLES; i++) {
        if(can[i].board == board) {     //   me, myself and I!
            condition = PCAN_CHANNEL_OCCUPIED;
            used = 1;
            break;
        }
    }
    if(result) {                        // CAN board test:
        if((condition == PCAN_CHANNEL_AVAILABLE) || (condition == PCAN_CHANNEL_PCANVIEW))
            *result = CANBRD_PRESENT;     // CAN board present
        else if(condition == PCAN_CHANNEL_UNAVAILABLE)
            *result = CANBRD_NOT_PRESENT; // CAN board not present
        else if(condition == PCAN_CHANNEL_OCCUPIED)
            *result = CANBRD_OCCUPIED;    // CAN board present, but occupied
        else
            *result = CANBRD_NOT_TESTABLE;// guess borad is not testable
    }
    if(((condition == PCAN_CHANNEL_AVAILABLE) || (condition == PCAN_CHANNEL_PCANVIEW)) ||
       (/*(condition == PCAN_CHANNEL_OCCUPIED) ||*/ used)) {
        // FIXME: issue TC07_47_9w - returns PCAN_ERROR_INITIALIZE when channel used by another process
        if((rc = CAN_GetValue((WORD)board, PCAN_CHANNEL_FEATURES,
                              (void*)&features, sizeof(features))) != PCAN_ERROR_OK)
            return pcan_error(rc);
        if((mode & CANMODE_FDOE) && !(features & FEATURE_FD_CAPABLE))
            return CANERR_ILLPARA; // CAN FD operation requested, but not supported
        if((mode & CANMODE_BRSE) && !(mode & CANMODE_FDOE))
            return CANERR_ILLPARA; // bit-rate switching requested, but CAN FD not enabled
        /*if((mode & CANMODE_NISO)) {} // This can not be determined (FIXME) */
#if (0)
        /*if((mode & CANMODE_NXTD)) {} // PCAN_ACCEPTANCE_FILTER_29BIT available since version 4.2.0 */
        /*if((mode & CANMODE_NRTR)) {} // PCAN_ALLOW_RTR_FRAMES available since version 4.2.0 */
#else
        if((mode & CANMODE_NXTD)) return CANERR_ILLPARA; // acceptance filtering not supported!
        if((mode & CANMODE_NRTR)) return CANERR_ILLPARA; // suppressing RTR frames not supported!
#endif
        /*if((mode & CANMODE_ERR)) {}  // PCAN_ALLOW_ERROR_FRAMES available since version 4.2.0 */
        /*if((mode & CANMODE_MON)) {}  // PCAN_LISTEN_ONLY available since version 1.0.0 */
    }
    (void)param;
    return CANERR_NOERROR;
}

int can_init(int board, unsigned char mode, const void *param)
{
    TPCANStatus rc;                     // return value
    DWORD value;                        // parameter value
    BYTE  type = 0;                     // board type (none PnP hardware)
    DWORD port = 0;                     // board parameter: I/O port address
    WORD  irq = 0;                      // board parameter: interrupt number
    int i;

    if(!init) {                         // when not init before:
        for(i = 0; i < PCAN_MAX_HANDLES; i++) {
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
	for(i = 0; i < PCAN_MAX_HANDLES; i++) {
		if(can[i].board == board)       // channel already in use
			return CANERR_YETINIT;
	}
    for(i = 0; i < PCAN_MAX_HANDLES; i++) {
        if(can[i].board == PCAN_NONEBUS)// get an unused handle, if any
            break;
    }
    if(!IS_HANDLE_VALID(i))             // no free handle found
        return CANERR_HANDLE;

    /* to start the CAN controller initially in reset state, we have switch off
     * the receiver and the transmitter and then to call CAN_Initialize[FD]() */
    value = PCAN_PARAMETER_OFF;         // receiver off
    if((rc = CAN_SetValue((WORD)board, PCAN_RECEIVE_STATUS, (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    value = PCAN_PARAMETER_ON;          // transmitter off
    if((rc = CAN_SetValue((WORD)board, PCAN_LISTEN_ONLY, (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    if((mode & CANMODE_FDOE)) {         // CAN FD operation mode?
        if ((rc = CAN_InitializeFD((WORD)board, BIT_RATE_DEFAULT)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    else {                              // CAN 2.0 operation mode
        if(param) {
            type = ((struct _pcan_param*)param)->type;
            port = ((struct _pcan_param*)param)->port;
            irq = ((struct _pcan_param*)param)->irq;
        }
        if((rc = CAN_Initialize((WORD)board, BTR0BTR1_DEFAULT, type, port, irq)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    can[i].board = board;               // handle of the CAN channel
    if(param) {                         // non-plug'n'play devices:
        can[i].brd_type = ((struct _pcan_param*)param)->type;
        can[i].brd_port = ((struct _pcan_param*)param)->port;
        can[i].brd_irq  = ((struct _pcan_param*)param)->irq;
    }
    can[i].mode.byte = mode;            // store selected operation mode
    can[i].status.byte = CANSTAT_RESET; // CAN controller not started yet!

    return i;                           // return the handle
}

int can_exit(int handle)
{
    TPCANStatus rc;                     // return value
    int i;

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(handle != CANEXIT_ALL) {
        if(!IS_HANDLE_VALID(handle))   // must be a valid handle
            return CANERR_HANDLE;
        if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
            return CANERR_HANDLE;
        if(!can[handle].status.b.can_stopped) {// when running then go bus off
            /* note: here we should turn off the receiver and the transmitter,
             *       but after CAN_Uninitialize we are really bus off! */
            (void)CAN_Reset(can[handle].board);
        }
#if defined(_WIN32) || defined(_WIN64)
        if(can[handle].event != NULL) { // close event handle, if any
            if(!CloseHandle(can[handle].event))
                return CANERR_FATAL;
        }
#endif
        if((rc = CAN_Uninitialize(can[handle].board)) != PCAN_ERROR_OK)
            return pcan_error(rc);
		
        can[handle].status.byte |= CANSTAT_RESET;  // CAN controller in INIT state
        can[handle].board = PCAN_NONEBUS; // handle can be used again
    }
    else {
        for(i = 0; i < PCAN_MAX_HANDLES; i++) {
            if(can[i].board != PCAN_NONEBUS) // must be an opened handle
            {
                if (!can[i].status.b.can_stopped) {// when running then go bus off
                    /* note: here we should turn off the receiver and the transmitter,
                     *       but after CAN_Uninitialize we are really bus off! */
                    (void)CAN_Reset(can[i].board);
                }
#if defined(_WIN32) || defined(_WIN64)
                if(can[i].event != NULL) // close event handle, if any
                    (void)CloseHandle(can[i].event);
#endif
                (void)CAN_Uninitialize(can[i].board); // resistance is futile!
				
                can[i].status.byte |= CANSTAT_RESET;  // CAN controller in INIT state
                can[i].board = PCAN_NONEBUS; // handle can be used again
            }
        }
    }
    return CANERR_NOERROR;
}

int can_start(int handle, const can_bitrate_t *bitrate)
{
    TPCANBaudrate btr0btr1;             // btr0btr1 value
    char string[PCAN_BUF_SIZE];         // bit-rate string
    DWORD value;                        // parameter value
    //QWORD filter;                       // for 29-bit filter
    TPCANStatus rc;                     // return value

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;
    if(bitrate == NULL)                 // check for null-pointer
        return CANERR_NULLPTR;
    if(!can[handle].status.b.can_stopped)// must be stopped!
        return CANERR_ONLINE;

    if(bitrate->index <= 0) {           // btr0btr1 from index
        switch(bitrate->index) {
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
    else if(!can[handle].mode.b.fdoe) { // btr0btr1 for CAN 2.0
        if((rc = bitrate2register(bitrate, &btr0btr1)) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
    }
    else {                              // a string for CAN FD
        if((rc = bitrate2string(bitrate, string, can[handle].mode.b.brse)) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
    }
    /* note: to (re-)start the CAN controller, we have to reinitialize it */
#if defined(_WIN32) || defined(_WIN64)
    if(can[handle].event != NULL)       // close event handle, if any
        (void)CloseHandle(can[handle].event);
#endif
    if((rc = CAN_Reset(can[handle].board)) != PCAN_ERROR_OK)
        return pcan_error(rc);
    if((rc = CAN_Uninitialize(can[handle].board)) != PCAN_ERROR_OK)
        return pcan_error(rc);
    /* note: the receiver is automatically switched on by CAN_Uninitialize() */
    if(can[handle].mode.b.fdoe) {       // CAN FD operation mode?
        if((rc = CAN_InitializeFD(can[handle].board, string)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    else {                              // CAN 2.0 operation mode!
        if((rc = CAN_Initialize(can[handle].board, btr0btr1,
                                can[handle].brd_type, can[handle].brd_port,
                                can[handle].brd_irq)) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
#if defined(_WIN32) || defined(_WIN64)
    if((can[handle].event = CreateEvent( // create an event handle
            NULL,                       //   default security attributes
            FALSE,                      //   auto-reset event
            FALSE,                      //   initial state is nonsignaled
            TEXT("PCANBasic")           //   object name
            )) == NULL) {
        CAN_Uninitialize(can[handle].board);
        return CANERR_FATAL;
    }
    if((rc = CAN_SetValue(can[handle].board, PCAN_RECEIVE_EVENT,
                  (void*)&can[handle].event, sizeof(can[handle].event))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
#endif
    value = (can[handle].mode.b.mon) ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    if((rc = CAN_SetValue(can[handle].board, PCAN_LISTEN_ONLY,
                   (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
    value = (can[handle].mode.b.err) ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    if((rc = CAN_SetValue(can[handle].board, PCAN_ALLOW_ERROR_FRAMES,
                     (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
#if (0)
    value = (can[handle].mode.b.nrtr) ? PCAN_PARAMETER_OFF : PCAN_PARAMETER_ON;
    if((rc = CAN_SetValue(can[handle].board, PCAN_ALLOW_RTR_FRAMES, // TODO: fdoe?
                  (void*)&value, sizeof(value))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
    filter = (can[handle].mode.b.nxtd) ? 0x1FFFFFFF1FFFFFFFull : 0x000000001FFFFFFFull;
    if((rc = CAN_SetValue(can[handle].board, PCAN_ACCEPTANCE_FILTER_29BIT,
                          (void*)&filter, sizeof(filter))) != PCAN_ERROR_OK) {
        CAN_Uninitialize(can[handle].board);
        return pcan_error(rc);
    }
#endif
    can[handle].status.byte = 0x00;     // clear old status bits and counters
    can[handle].counters.tx = 0ull;
    can[handle].counters.rx = 0ull;
    can[handle].counters.err = 0ull;
    can[handle].status.b.can_stopped = 0;  // CAN controller started!

    return CANERR_NOERROR;
}

#if defined(_WIN32) || defined(_WIN64)
 int can_kill(int handle)
 {
    int i;

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(handle != CANKILL_ALL) {
        if(!IS_HANDLE_VALID(handle))    // must be a valid handle
            return CANERR_HANDLE;
        if((can[handle].board != PCAN_NONEBUS) &&
           (can[handle].event != NULL)) {
            SetEvent(can[handle].event);  // signal event oject
        }
    }
    else {
        for(i = 0; i < PCAN_MAX_HANDLES; i++) {
            if((can[i].board != PCAN_NONEBUS) &&
               (can[i].event != NULL))  {
                SetEvent(can[i].event); //   signal all event ojects
            }
        }
    }
    return CANERR_NOERROR;
 }
#endif

int can_reset(int handle)
{
    TPCANStatus rc;                     // return value
    DWORD value;                        // parameter value

    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if(can[handle].status.b.can_stopped) { // when running then go bus off
        /* note: we turn off the receiver and the transmitter to do that! */
        value = PCAN_PARAMETER_OFF;     //   receiver off
        if((rc = CAN_SetValue(can[handle].board, PCAN_RECEIVE_STATUS, (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
            return pcan_error(rc);
        value = PCAN_PARAMETER_ON;      //   transmitter off
        if((rc = CAN_SetValue(can[handle].board, PCAN_LISTEN_ONLY, (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
            return pcan_error(rc);
    }
    can[handle].status.b.can_stopped = 1; // CAN controller stopped!

    return CANERR_NOERROR;
}

int can_write(int handle, const can_msg_t *msg)
{
    TPCANMsg can_msg;                   // the message (CAN 2.0)
    TPCANMsgFD can_msg_fd;              // the message (CAN FD)
    TPCANStatus rc;                     // return value

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;
    if(msg == NULL)                     // check for null-pointer
        return CANERR_NULLPTR;
    if(can[handle].status.b.can_stopped) // must be running
        return CANERR_OFFLINE;

    if(!can[handle].mode.b.fdoe) {
        if(msg->dlc > CAN_MAX_LEN)      //   data length 0 .. 8
            return CANERR_ILLPARA;
        if(msg->ext)                    //   29-bit identifier
            can_msg.MSGTYPE = PCAN_MESSAGE_EXTENDED;
        else                            //   11-bit identifier
            can_msg.MSGTYPE = PCAN_MESSAGE_STANDARD;
        if(msg->rtr)                    //   request a message
            can_msg.MSGTYPE |= PCAN_MESSAGE_RTR;
        can_msg.ID = (DWORD)(msg->id);
        can_msg.LEN = (BYTE)(msg->dlc);
        memcpy(can_msg.DATA, msg->data, msg->dlc);

        rc = CAN_Write(can[handle].board, &can_msg);
    }
    else {
        if(msg->dlc > CANFD_MAX_DLC)    //   data length 0 .. 0Fh!
            return CANERR_ILLPARA;
        if(msg->ext)                    //   29-bit identifier
            can_msg_fd.MSGTYPE = PCAN_MESSAGE_EXTENDED;
        else                            //   11-bit identifier
            can_msg_fd.MSGTYPE = PCAN_MESSAGE_STANDARD;
        if(msg->rtr)                    //   request a message
            can_msg_fd.MSGTYPE |= PCAN_MESSAGE_RTR;
        if(msg->fdf)                    //   CAN FD format
            can_msg_fd.MSGTYPE |= PCAN_MESSAGE_FD;
        if(msg->brs && can[handle].mode.b.brse) //   bit-rate switching
            can_msg_fd.MSGTYPE |= PCAN_MESSAGE_BRS;
        can_msg_fd.ID = (DWORD)(msg->id);
        can_msg_fd.DLC = (BYTE)(msg->dlc);
        memcpy(can_msg_fd.DATA, msg->data, DLC2LEN(msg->dlc));

        rc = CAN_WriteFD(can[handle].board, &can_msg_fd);
    }
    if(rc != PCAN_ERROR_OK) {
        if((rc & PCAN_ERROR_QXMTFULL)) {//   transmit queue full?
            can[handle].status.b.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy
        }
        if((rc & PCAN_ERROR_XMTFULL)) { //   transmission pending?
            can[handle].status.b.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy
        }
        return pcan_error(rc);          //   PCAN specific error?
    }
    can[handle].status.b.transmitter_busy = 0; // message transmitted
    can[handle].counters.tx++;

    return CANERR_NOERROR;
}

int can_read(int handle, can_msg_t *msg, unsigned short timeout)
{
    TPCANMsg can_msg;                   // the message (CAN 2.0)
    TPCANTimestamp timestamp;           // time stamp (CAN 2.0)
    TPCANMsgFD can_msg_fd;              // the message (CAN FD)
    TPCANTimestampFD timestamp_fd;      // time stamp (CAN FD)
    unsigned long long msec;            // milliseconds
    TPCANStatus rc;                     // return value

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;
    if(msg == NULL)                     // check for null-pointer
        return CANERR_NULLPTR;
    if(can[handle].status.b.can_stopped) // must be running
        return CANERR_OFFLINE;

    if(!can[handle].mode.b.fdoe)
        rc = CAN_Read(can[handle].board, &can_msg, &timestamp);
    else
        rc = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
    if(rc == PCAN_ERROR_QRCVEMPTY) {
#if defined(_WIN32) || defined(_WIN64)
        if(timeout > 0) {
            switch(WaitForSingleObject(can[handle].event, (timeout != 65535) ? timeout : INFINITE)) {
            case WAIT_OBJECT_0:
                break;                  //   one or more messages received
            case WAIT_TIMEOUT:
                break;                  //   time-out, but look for old messages
            default:
                return CANERR_FATAL;    //   function failed!
            }
            if(!can[handle].mode.b.fdoe)
                rc = CAN_Read(can[handle].board, &can_msg, &timestamp);
            else
                rc = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
            if(rc == PCAN_ERROR_QRCVEMPTY) {
                can[handle].status.b.receiver_empty = 1;
                return CANERR_RX_EMPTY; //   receiver empty
            }
        }
        else {
            can[handle].status.b.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty
    }
#else
        can[handle].status.b.receiver_empty = 1;
        return CANERR_RX_EMPTY;         //   receiver empty
#endif
    }
    /*if(rc != PCAN_ERROR_OK) { // Is this a good idea? */
    if((rc & ~(PCAN_ERROR_ANYBUSERR |
               PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN |
               PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL))) {
        return pcan_error(rc);          //   something's wrong
    }
    if(!can[handle].mode.b.fdoe) {      // CAN 2.0 message:
        if((can_msg.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            can[handle].status.b.bus_off = (can_msg.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
            can[handle].status.b.bus_error = (can_msg.DATA[3] & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
            can[handle].status.b.warning_level = (can_msg.DATA[3] & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
            can[handle].status.b.message_lost |= (can_msg.DATA[3] & PCAN_ERROR_OVERRUN) != PCAN_ERROR_OK;
            can[handle].status.b.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty
        }
        if((can_msg.MSGTYPE & PCAN_MESSAGE_ERRFRAME))  {
            can[handle].status.b.receiver_empty = 1;
            can[handle].counters.err++;
            return CANERR_ERR_FRAME;    //   error frame received
        }
        msg->id = can_msg.ID;
        msg->ext = (can_msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
        msg->rtr = (can_msg.MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
        msg->fdf = 0;
        msg->brs = 0;
        msg->esi = 0;
        msg->dlc = can_msg.LEN;
        memcpy(msg->data, can_msg.DATA, CAN_MAX_LEN);
        msec = ((unsigned long long)timestamp.millis_overflow << 32) + (unsigned long long)timestamp.millis;
        msg->timestamp.sec = (long)(msec / 1000ull);
        msg->timestamp.usec = (((long)(msec % 1000ull)) * 1000L) + (long)timestamp.micros;
    }
    else {                              // CAN FD message:
        if((can_msg_fd.MSGTYPE & PCAN_MESSAGE_STATUS)) {
            can[handle].status.b.bus_off = (can_msg_fd.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
            can[handle].status.b.bus_error = (can_msg_fd.DATA[3] & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
            can[handle].status.b.warning_level = (can_msg_fd.DATA[3] & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
            can[handle].status.b.message_lost |= (can_msg_fd.DATA[3] & PCAN_ERROR_OVERRUN) != PCAN_ERROR_OK;
            can[handle].status.b.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty
        }
        if((can_msg_fd.MSGTYPE & PCAN_MESSAGE_ERRFRAME)) {
            can[handle].status.b.receiver_empty = 1;
            can[handle].counters.err++;
            return CANERR_ERR_FRAME;    //   error frame received
        }
        msg->id = can_msg_fd.ID;
        msg->ext = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_EXTENDED) ? 1 : 0;
        msg->rtr = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_RTR) ? 1 : 0;
        msg->fdf = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_FD) ? 1 : 0;
        msg->brs = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_BRS) ? 1 : 0;
        msg->esi = (can_msg_fd.MSGTYPE & PCAN_MESSAGE_ESI) ? 1 : 0;
        msg->dlc = can_msg_fd.DLC;
        memcpy(msg->data, can_msg_fd.DATA, CANFD_MAX_LEN);
        msg->timestamp.sec = (long)(timestamp_fd / 1000000ull);
        msg->timestamp.usec = (long)(timestamp_fd % 1000000ull);
    }
    can[handle].status.b.receiver_empty = 0; // message read
    can[handle].counters.rx++;

    return CANERR_NOERROR;
}

int can_status(int handle, unsigned char *status)
{
    TPCANStatus rc;                     // represents a status

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if(!can[handle].status.b.can_stopped) { // when running get bus status
        rc = CAN_GetStatus(can[handle].board);
        if((rc & ~(PCAN_ERROR_ANYBUSERR | 
                   PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN | 
                   PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)))
            return pcan_error(rc);
        can[handle].status.b.bus_off = (rc & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
        can[handle].status.b.bus_error = (rc & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
        can[handle].status.b.warning_level = (rc & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
        can[handle].status.b.message_lost |= (rc & (PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN)) != PCAN_ERROR_OK;
        can[handle].status.b.transmitter_busy |= (rc & (PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)) != PCAN_ERROR_OK;
    }
    if(status)                          // status-register
      *status = can[handle].status.byte;

    return CANERR_NOERROR;
}

int can_busload(int handle, unsigned char *load, unsigned char *status)
{
    float busload = 0.0;                // bus-load (in [percent])

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if(!can[handle].status.b.can_stopped) { // when running get bus load
        (void)busload; //  TODO: measure bus load
    }
    if(load)                            // bus-load (in [percent])
        *load = (unsigned char)busload;
     return can_status(handle, status); // status-register
}

int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed)
{
    TPCANBaudrate btr0btr1;             // btr0btr1 value
    char string[PCAN_BUF_SIZE];         // bit-rate string
    can_bitrate_t temporary;            // bit-rate settings
    int rc;                             // return value

    memset(&temporary, 0, sizeof(can_bitrate_t));

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    if(!can[handle].mode.b.fdoe) {      // CAN 2.0
        if((rc = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO,
                             (void*)&btr0btr1, sizeof(TPCANBaudrate))) != PCAN_ERROR_OK)
            return pcan_error(rc);
        if((rc = register2bitrate(btr0btr1, &temporary)) != CANERR_NOERROR)
            return rc;
    }
    else {                              // CAN FD
        if((rc = CAN_GetValue(can[handle].board, PCAN_BITRATE_INFO_FD,
                             (void*)string, PCAN_BUF_SIZE)) != PCAN_ERROR_OK)
            return pcan_error(rc);
        if((rc = string2bitrate(string, &temporary, can[handle].mode.b.brse)) != CANERR_NOERROR)
            return rc;
    }
    if(bitrate) {
        memcpy(bitrate, &temporary, sizeof(can_bitrate_t));
    }
    if(speed) {
        if((rc = calc_speed(&temporary, speed, 0)) != CANERR_NOERROR)
            return rc;
        speed->nominal.fdoe = can[handle].mode.b.fdoe;
        speed->data.brse = can[handle].mode.b.brse;
    }
    if(!can[handle].status.b.can_stopped)
        rc = CANERR_NOERROR;
    else
        rc = CANERR_OFFLINE;
    return rc;
}

int can_property(int handle, int param, void *value, int nbytes)
{
    if(!init || !IS_HANDLE_VALID(handle)) {
        return lib_parameter(param, value, (size_t)nbytes);
    }
    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return CANERR_HANDLE;

    return drv_parameter(handle, param, value, (size_t)nbytes);
}

char *can_hardware(int handle)
{
    static char hardware[256] = "";     // hardware version
    char  str[256], *ptr;               // info string
    DWORD dev = 0x0000UL;               // device number

    if(!init)                           // must be initialized
        return NULL;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return NULL;
    if(can[handle].board == PCAN_NONEBUS) // must be an opened handle
        return NULL;

    if(CAN_GetValue(can[handle].board, PCAN_CHANNEL_VERSION, (void*)str, 256) != PCAN_ERROR_OK)
        return NULL;
    if((ptr = strchr(str, '\n')) != NULL)
       *ptr = '\0';
    if((((can[handle].board & 0x00F0) >> 4) == PCAN_USB) ||
       (((can[handle].board & 0x0F00) >> 8) == PCAN_USB))
    {
        if(CAN_GetValue(can[handle].board, PCAN_DEVICE_NUMBER, (void*)&dev, 4) != PCAN_ERROR_OK)
            return NULL;
        snprintf(hardware, 256, "%s (Device %02lXh)", str, dev);
    }
    else
        strcpy(hardware, str);

    return (char*)hardware;             // hardware version
}

char *can_software(int handle)
{
    static char software[256] = "";     // software version
    char  str[256] = "PCAN-Basic API "; // info string

    if(!init)                           // must be initialized
        return NULL;
    (void)handle;                       // handle not needed here

    if(CAN_GetValue(PCAN_NONEBUS, PCAN_API_VERSION, (void*)&str[15], 256-15) != PCAN_ERROR_OK)
        return NULL;
    strcpy(software, str);

    return (char*)software;             // software version
}

/*  -----------  local functions  ----------------------------------------
 */

static int pcan_error(TPCANStatus status)
{
    if((status & PCAN_ERROR_XMTFULL)      == PCAN_ERROR_XMTFULL)       return CANERR_TX_BUSY;
    if((status & PCAN_ERROR_OVERRUN)      == PCAN_ERROR_OVERRUN)       return CANERR_MSG_LST;
    if((status & PCAN_ERROR_BUSOFF)       == PCAN_ERROR_BUSOFF)        return CANERR_BOFF;
    if((status & PCAN_ERROR_BUSPASSIVE)   == PCAN_ERROR_BUSPASSIVE)    return CANERR_EWRN;
    if((status & PCAN_ERROR_BUSHEAVY)     == PCAN_ERROR_BUSHEAVY)      return CANERR_BERR;
    if((status & PCAN_ERROR_BUSLIGHT)     == PCAN_ERROR_BUSLIGHT)      return CANERR_BERR;
    if((status & PCAN_ERROR_QRCVEMPTY)    == PCAN_ERROR_QRCVEMPTY)     return CANERR_RX_EMPTY;
    if((status & PCAN_ERROR_QOVERRUN)     == PCAN_ERROR_QOVERRUN)      return CANERR_MSG_LST;
    if((status & PCAN_ERROR_QXMTFULL)     == PCAN_ERROR_QXMTFULL)      return CANERR_TX_BUSY;
    if((status & PCAN_ERROR_REGTEST)      == PCAN_ERROR_REGTEST)       return PCAN_ERR_REGTEST;
    if((status & PCAN_ERROR_NODRIVER)     == PCAN_ERROR_NODRIVER)      return PCAN_ERR_NODRIVER;
    if((status & PCAN_ERROR_HWINUSE)      == PCAN_ERROR_HWINUSE)       return PCAN_ERR_HWINUSE;
    if((status & PCAN_ERROR_NETINUSE)     == PCAN_ERROR_NETINUSE)      return PCAN_ERR_NETINUSE;
    if((status & PCAN_ERROR_ILLHW)        == PCAN_ERROR_ILLHW)         return PCAN_ERR_ILLHW;
    if((status & PCAN_ERROR_ILLNET)       == PCAN_ERROR_ILLNET)        return PCAN_ERR_ILLNET;
    if((status & PCAN_ERROR_ILLCLIENT)    == PCAN_ERROR_ILLCLIENT)     return PCAN_ERR_ILLCLIENT;
    if((status & PCAN_ERROR_RESOURCE)     == PCAN_ERROR_RESOURCE)      return PCAN_ERR_RESOURCE;
    if((status & PCAN_ERROR_ILLPARAMTYPE) == PCAN_ERROR_ILLPARAMTYPE)  return PCAN_ERR_ILLPARAMTYPE;
    if((status & PCAN_ERROR_ILLPARAMVAL)  == PCAN_ERROR_ILLPARAMVAL)   return PCAN_ERR_ILLPARAMVAL;
    if((status & PCAN_ERROR_ILLDATA)      == PCAN_ERROR_ILLDATA)       return PCAN_ERR_ILLDATA;
    if((status & PCAN_ERROR_CAUTION)      == PCAN_ERROR_CAUTION)       return PCAN_ERR_CAUTION;
    if((status & PCAN_ERROR_INITIALIZE)   == PCAN_ERROR_INITIALIZE)    return CANERR_NOTINIT;
    if((status & PCAN_ERROR_ILLOPERATION) == PCAN_ERROR_ILLOPERATION)  return PCAN_ERR_ILLOPERATION;

    return PCAN_ERR_UNKNOWN;
}

static int pcan_capability(WORD board, can_mode_t *capability)
{
    TPCANStatus rc;                     // return value
    DWORD features;                     // channel features

    if((rc = CAN_GetValue((WORD)board, PCAN_CHANNEL_FEATURES,
                          (void*)&features, sizeof(features))) != PCAN_ERROR_OK)
        return pcan_error(rc);

    capability->b.fdoe = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->b.brse = (features & FEATURE_FD_CAPABLE) ? 1 : 0;
    capability->b.niso = 0; // This can not be determined (FIXME) 
    capability->b.shrd = 0; // This feature is not supported (TODO: clarify)
#if (0)
    capability->b.nxtd = 1; // PCAN_ACCEPTANCE_FILTER_29BIT available since version 4.2.0
    capability->b.nrtr = 1; // PCAN_ALLOW_RTR_FRAMES available since version 4.2.0
#else
    capability->b.nxtd = 0; // This feature is not supported (TODO: acceptance filtering)
    capability->b.nrtr = 0; // This feature is not supported (TODO: suppress RTR frames)
#endif
    capability->b.err = 1;  // PCAN_ALLOW_ERROR_FRAMES available since version 4.2.0
    capability->b.mon = 1;  // PCAN_LISTEN_ONLY available since version 1.0.0

    return CANERR_NOERROR;
}

static int index2bitrate(int index, can_bitrate_t *bitrate)
{
    TPCANBaudrate btr0btr1 = 0x0000u;

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
    return register2bitrate(btr0btr1, bitrate);
}

static int bitrate2register(const can_bitrate_t *bitrate, TPCANBaudrate *btr0btr1)
{
    if(bitrate->btr.frequency != (long)CANBTR_FREQ_SJA1000) // SJA1000 @ 8MHz
        return CANERR_BAUDRATE;
    if((bitrate->btr.nominal.brp < CANBTR_SJA1000_BRP_MIN) || (CANBTR_SJA1000_BRP_MAX < bitrate->btr.nominal.brp))
        return CANERR_BAUDRATE;
    if((bitrate->btr.nominal.tseg1 < CANBTR_SJA1000_TSEG1_MIN) || (CANBTR_SJA1000_TSEG1_MAX < bitrate->btr.nominal.tseg1))
        return CANERR_BAUDRATE;
    if((bitrate->btr.nominal.tseg2 < CANBTR_SJA1000_TSEG2_MIN) || (CANBTR_SJA1000_TSEG2_MAX < bitrate->btr.nominal.tseg2))
        return CANERR_BAUDRATE;
    if((bitrate->btr.nominal.sjw < CANBTR_SJA1000_SJW_MIN) || (CANBTR_SJA1000_SJW_MAX < bitrate->btr.nominal.sjw))
        return CANERR_BAUDRATE;
    if(/*(bitrate->btr.nominal.sam < CANBTR_SJA1000_SAM_MIN) ||*/ (CANBTR_SJA1000_SAM_MAX < bitrate->btr.nominal.sam))
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

static int register2bitrate(const TPCANBaudrate btr0btr1, can_bitrate_t *bitrate)
{
    bitrate->btr.frequency = (long)CANBTR_FREQ_SJA1000;     // SJA1000 @ 8MHz
    bitrate->btr.nominal.sjw = (unsigned short)((btr0btr1 & 0xC000u) >> 14) + 1u;
    bitrate->btr.nominal.brp = (unsigned short)((btr0btr1 & 0x3F00u) >> 8) + 1u;
    bitrate->btr.nominal.sam = (unsigned short)((btr0btr1 & 0x0080u) >> 7) + 0u;
    bitrate->btr.nominal.tseg2 = (unsigned short)((btr0btr1 & 0x0070u) >> 4) + 1u;
    bitrate->btr.nominal.tseg1 = (unsigned short)((btr0btr1 & 0x000Fu) >> 0) + 1u;
    bitrate->btr.data.brp = 0;
    bitrate->btr.data.tseg1 = 0;
    bitrate->btr.data.tseg2 = 0;
    bitrate->btr.data.sjw = 0;
    return CANERR_NOERROR;
}

static int bitrate2string(const can_bitrate_t *bitrate, TPCANBitrateFD string, int brse)
{
    if((bitrate->btr.nominal.brp < CANBTR_NOMINAL_BRP_MIN) || (CANBTR_NOMINAL_BRP_MAX < bitrate->btr.nominal.brp))
        return CANERR_BAUDRATE;
    if((bitrate->btr.nominal.tseg1 < CANBTR_NOMINAL_TSEG1_MIN) || (CANBTR_NOMINAL_TSEG1_MAX < bitrate->btr.nominal.tseg1))
        return CANERR_BAUDRATE;
    if((bitrate->btr.nominal.tseg2 < CANBTR_NOMINAL_TSEG2_MIN) || (CANBTR_NOMINAL_TSEG2_MAX < bitrate->btr.nominal.tseg2))
        return CANERR_BAUDRATE;
    if((bitrate->btr.nominal.sjw < CANBTR_NOMINAL_SJW_MIN) || (CANBTR_NOMINAL_SJW_MAX < bitrate->btr.nominal.sjw))
        return CANERR_BAUDRATE;
    if(!brse) {     // long frames only
        if(sprintf(string, "f_clock=%li,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u",
                            bitrate->btr.frequency,
                            bitrate->btr.nominal.brp,
                            bitrate->btr.nominal.tseg1,
                            bitrate->btr.nominal.tseg2,
                            bitrate->btr.nominal.sjw,
                            bitrate->btr.nominal.sam) < 0)
            return CANERR_BAUDRATE;
    }
    else {          // long and fast frames
        if((bitrate->btr.data.brp < CANBTR_DATA_BRP_MIN) || (CANBTR_DATA_BRP_MAX < bitrate->btr.data.brp))
            return CANERR_BAUDRATE;
        if((bitrate->btr.data.tseg1 < CANBTR_DATA_TSEG1_MIN) || (CANBTR_DATA_TSEG1_MAX < bitrate->btr.data.tseg1))
            return CANERR_BAUDRATE;
        if((bitrate->btr.data.tseg2 < CANBTR_DATA_TSEG2_MIN) || (CANBTR_DATA_TSEG2_MAX < bitrate->btr.data.tseg2))
            return CANERR_BAUDRATE;
        if((bitrate->btr.data.sjw < CANBTR_DATA_SJW_MIN) || (CANBTR_DATA_SJW_MAX < bitrate->btr.data.sjw))
            return CANERR_BAUDRATE;
        if(sprintf(string, "f_clock=%li,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u,"
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

static int string2bitrate(const TPCANBitrateFD string, can_bitrate_t *bitrate, int brse)
{
    long unsigned freq = 0;
    int unsigned nom_brp = 0, nom_tseg1 = 0, nom_tseg2 = 0, nom_sjw = 0/*, nom_sam = 0*/;
    int unsigned data_brp = 0, data_tseg1 = 0, data_tseg2 = 0, data_sjw = 0/*, data_ssp_offset = 0*/;

    // TODO: rework this!
    if(sscanf(string, "f_clock=%lu,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,"
                                   "data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                             &freq, &nom_brp, &nom_tseg1, &nom_tseg2, &nom_sjw,
                                   &data_brp, &data_tseg1, &data_tseg2, &data_sjw) != 9)
        return CANERR_BAUDRATE;
    bitrate->btr.frequency = (long)freq;
    bitrate->btr.nominal.brp = (unsigned short)nom_brp;
    bitrate->btr.nominal.tseg1 = (unsigned short)nom_tseg1;
    bitrate->btr.nominal.tseg2 = (unsigned short)nom_tseg2;
    bitrate->btr.nominal.sjw = (unsigned short)nom_sjw;
    if(brse) {
        bitrate->btr.data.brp = (unsigned short)data_brp;
        bitrate->btr.data.tseg1 = (unsigned short)data_tseg1;
        bitrate->btr.data.tseg2 = (unsigned short)data_tseg2;
        bitrate->btr.data.sjw = (unsigned short)data_sjw;
    }
    else {
        bitrate->btr.data.brp = (unsigned short)0;
        bitrate->btr.data.tseg1 = (unsigned short)0;
        bitrate->btr.data.tseg2 = (unsigned short)0;
        bitrate->btr.data.sjw = (unsigned short)0;
    }
    return CANERR_NOERROR;
}

/*  - - - - - -  CAN API V3 properties  - - - - - - - - - - - - - - - - -
    */
static int lib_parameter(int param, void *value, size_t nbytes)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter
    TPCANStatus sts;

    if(value == NULL)                   // check for null-pointer
        return CANERR_NULLPTR;

    /* CAN library properties */
    switch(param) {
    case CANPROP_GET_SPEC:              // version of the wrapper specification (USHORT)
        if(nbytes == sizeof(unsigned short)) {
            *(unsigned short*)value = (unsigned short)CAN_API_SPEC;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_VERSION:           // version number of the library (USHORT)
        if(nbytes == sizeof(unsigned short)) {
            *(unsigned short*)value = ((unsigned short)VERSION_MAJOR << 8)
                                    | ((unsigned short)VERSION_MINOR & 0xFu);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_PATCH_NO:          // patch number of the library (UCHAR)
        if(nbytes == sizeof(unsigned char)) {
            *(unsigned char*)value = (unsigned char)VERSION_PATCH;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BUILD_NO:          // build number of the library (ULONG)
        if(nbytes == sizeof(unsigned long)) {
            *(unsigned long*)value = (unsigned long)VERSION_BUILD;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_ID:        // library id of the library (int)
        if(nbytes == sizeof(int)) {
            *(int*)value = (int)PCAN_LIB_ID;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_VENDOR:    // vendor name of the library (char[256])
        if((nbytes > strlen(CAN_API_VENDOR)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, CAN_API_VENDOR);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_DLLNAME:   // file name of the library (char[256])
        if ((nbytes > strlen(PCAN_LIB_WRAPPER)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_WRAPPER);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BOARD_VENDOR:      // vendor name of the CAN interface (char[256])
        if((nbytes > strlen(PCAN_LIB_VENDOR)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_VENDOR);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BOARD_DLLNAME:     // file name of the CAN interface (char[256])
        if((nbytes > strlen(PCAN_LIB_BASIC)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, PCAN_LIB_BASIC);
            rc = CANERR_NOERROR;
        }
        break;
    default:
        if((CANPROP_GET_VENDOR_PROP <= param) &&  // get a vendor-specific property value (void*)
            (param < (CANPROP_GET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if((sts = CAN_GetValue(PCAN_NONEBUS, (BYTE)(param - CANPROP_GET_VENDOR_PROP),
                (void*)value, (DWORD)nbytes)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else if((CANPROP_SET_VENDOR_PROP <= param) &&  // set a vendor-specific property value (void*)
            (param < (CANPROP_SET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if((sts = CAN_SetValue(PCAN_NONEBUS, (BYTE)(param - CANPROP_SET_VENDOR_PROP),
                (void*)value, (DWORD)nbytes)) == PCAN_ERROR_OK)
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

static int drv_parameter(int handle, int param, void *value, size_t nbytes)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter
    can_bitrate_t bitrate;
    can_speed_t speed;
    can_mode_t mode;
    unsigned char status;
    unsigned char load;
    TPCANStatus sts;
    int i;

    assert(IS_HANDLE_VALID(handle));    // just to make sure

    if(value == NULL)                   // check for null-pointer
        return CANERR_NULLPTR;

    /* CAN interface properties */
    switch(param) {
    case CANPROP_GET_BOARD_TYPE:        // board type of the CAN interface (int)
        if(nbytes == sizeof(int)) {
            *(int*)value = (int)can[handle].board;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BOARD_NAME:        // board name of the CAN interface (char[256])
        for(i = 0; i < PCAN_BOARDS; i++) {
            if(can_board[i].type == (unsigned long)can[handle].board) {
                if((nbytes > strlen(can_board[i].name)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
                    strcpy((char*)value, can_board[i].name);
                    rc = CANERR_NOERROR;
                    break;
                }
            }
        }
        if((i == PCAN_BOARDS) || (rc = CANERR_NOERROR))
            rc = CANERR_FATAL;
        break;
    case CANPROP_GET_BOARD_PARAM:       // board parameter of the CAN interface (char[256])
        if(nbytes == sizeof(struct _pcan_param)) {
            ((struct _pcan_param*)value)->type = (unsigned char)can[handle].brd_type;
            ((struct _pcan_param*)value)->port = (unsigned long)can[handle].brd_port;
            ((struct _pcan_param*)value)->irq = (unsigned short)can[handle].brd_irq;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_OP_CAPABILITY:     // supported operation modes of the CAN controller (UCHAR)
        if((rc = pcan_capability(can[handle].board, &mode)) == CANERR_NOERROR) {
            if(nbytes == sizeof(unsigned char)) {
                *(unsigned char*)value = (unsigned char)mode.byte;
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_OP_MODE:           // active operation mode of the CAN controller (UCHAR)
        if (nbytes == sizeof(unsigned char)) {
            *(unsigned char*)value = (unsigned char)can[handle].mode.byte;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BITRATE:           // active bit-rate of the CAN controller (can_bitrate_t)
        if(((rc = can_bitrate(handle, &bitrate, NULL)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
            if(nbytes == sizeof(can_bitrate_t)) {
                memcpy(value, &bitrate, sizeof(can_bitrate_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_SPEED:             // active bus speed of the CAN controller (can_speed_t)
        if(((rc = can_bitrate(handle, NULL, &speed)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
            if(nbytes == sizeof(can_speed_t)) {
                memcpy(value, &speed, sizeof(can_speed_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_STATUS:            // current status register of the CAN controller (UCHAR)
        if((rc = can_status(handle, &status)) == CANERR_NOERROR) {
            if(nbytes == sizeof(unsigned char)) {
                *(unsigned char*)value = (unsigned char)status;
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (UCHAR)
        if((rc = can_busload(handle, &load, NULL)) == CANERR_NOERROR) {
            if(nbytes == sizeof(unsigned char)) {
                *(unsigned char*)value = (unsigned char)load;
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_TX_COUNTER:        // total number of sent messages (ULONGONG)
        if(nbytes == sizeof(unsigned long long)) {
            *(unsigned long long*)value = (unsigned long long)can[handle].counters.tx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_RX_COUNTER:        // total number of reveiced messages (ULONGONG)
        if(nbytes == sizeof(unsigned long long)) {
            *(unsigned long long*)value = (unsigned long long)can[handle].counters.rx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_ERR_COUNTER:       // total number of reveiced error frames (ULONGONG)
        if(nbytes == sizeof(unsigned long long)) {
            *(unsigned long long*)value = (unsigned long long)can[handle].counters.err;
            rc = CANERR_NOERROR;
        }
        break;
    default:
        if((CANPROP_GET_VENDOR_PROP <= param) &&  // get a vendor-specific property value (void*)
           (param < (CANPROP_GET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if((sts = CAN_GetValue(can[handle].board, (BYTE)(param - CANPROP_GET_VENDOR_PROP),
                (void*)value, (DWORD)nbytes)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else if((CANPROP_SET_VENDOR_PROP <= param) &&  // set a vendor-specific property value (void*)
                (param < (CANPROP_SET_VENDOR_PROP + CANPROP_VENDOR_PROP_RANGE))) {
            if((sts = CAN_SetValue(can[handle].board, (BYTE)(param - CANPROP_SET_VENDOR_PROP),
                (void*)value, (DWORD)nbytes)) == PCAN_ERROR_OK)
                rc = CANERR_NOERROR;
            else
                rc = pcan_error(sts);
        }
        else                            // or general library properties (see lib_parameter)
            rc = lib_parameter(param, value, nbytes);
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

    memset(&temporary, 0, sizeof(can_bitrate_t));

    if(bitrate->index <= 0) {
        if((rc = index2bitrate(bitrate->index, &temporary)) != CANERR_NOERROR)
            return rc;
        if(modify)                      // translate index to bit-rate
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
    if((temporary.btr.nominal.brp < CANBTR_NOMINAL_BRP_MIN) || (CANBTR_NOMINAL_BRP_MAX < temporary.btr.nominal.brp))
        return CANERR_BAUDRATE;
    if((temporary.btr.nominal.tseg1 < CANBTR_NOMINAL_TSEG1_MIN) || (CANBTR_NOMINAL_TSEG1_MAX < temporary.btr.nominal.tseg1))
        return CANERR_BAUDRATE;
    if((temporary.btr.nominal.tseg2 < CANBTR_NOMINAL_TSEG2_MIN) || (CANBTR_NOMINAL_TSEG2_MAX < temporary.btr.nominal.tseg2))
        return CANERR_BAUDRATE;
    if((temporary.btr.nominal.sjw < CANBTR_NOMINAL_SJW_MIN) || (CANBTR_NOMINAL_SJW_MAX < temporary.btr.nominal.sjw))
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
    if(speed->data.brse) {
        if((temporary.btr.data.brp < CANBTR_DATA_BRP_MIN) || (CANBTR_DATA_BRP_MAX < temporary.btr.data.brp))
            return CANERR_BAUDRATE;
        if((temporary.btr.data.tseg1 < CANBTR_DATA_TSEG1_MIN) || (CANBTR_DATA_TSEG1_MAX < temporary.btr.data.tseg1))
            return CANERR_BAUDRATE;
        if((temporary.btr.data.tseg2 < CANBTR_DATA_TSEG2_MIN) || (CANBTR_DATA_TSEG2_MAX < temporary.btr.data.tseg2))
            return CANERR_BAUDRATE;
        if((temporary.btr.data.sjw < CANBTR_DATA_SJW_MIN) || (CANBTR_DATA_SJW_MAX < temporary.btr.data.sjw))
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

char* can_version()
{
    return (char*)_id;
}
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
