/*  -- $HeadURL$ --
 *
 *  project   :  CAN - Controller Area Network
 *
 *  purpose   :  CAN Interface API, Version 3 (PCAN-Basic)
 *
 *  copyright :  (C) 2010, UV Software, Friedrichshafen
 *               (C) 2014, 2017-2019, UV Software, Berlin
 *
 *  compiler  :  Microsoft Visual C/C++ Compiler (Version 19.15)
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

#define VERSION "3.3.dev"
#if defined(_WIN64)
#define PLATFORM    "x64"
#elif defined(_WIN32)
#define PLATFORM    "x86"
#else
#error Unsupported architecture
#endif
#include "can_vers.h"
#ifdef _DEBUG
    static char _id[] = "CAN API V3 for PEAK PCAN-Basic Interfaces, Version "VERSION"."SVN_REV_STR" ("PLATFORM") _DEBUG";
#else
    static char _id[] = "CAN API V3 for PEAK PCAN-Basic Interfaces, Version "VERSION"."SVN_REV_STR" ("PLATFORM")";
#endif

/*  -----------  includes  -------------------------------------------------
 */

#include "can_api.h"                    // Interface prototypes

#include <stdio.h>                      // Standard I/O routines
#include <string.h>                     // String manipulation functions
#include <windows.h>                    // Master include file for Windows
#include <signal.h>

#include "PCANBasic.h"                  // PEAK PCAN-Basic device(s)


/*  -----------  options  ------------------------------------------------
 */

#define _BLOCKING_READ                  // blocking read via wait event


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

/*  -----------  types  --------------------------------------------------
 */

typedef union {
    BYTE byte;                          // byte access
    struct {                            // bit access:
        BYTE mon : 1;                   //   monitor mode enable/disable
        BYTE : 3;                       //   (reserved)
        BYTE niso : 1;                  //   Non-ISO CAN FD enable
        BYTE : 1;                       //   (reserved)
        BYTE brse : 1;                  //   bit-rate switch enable
        BYTE fdoe : 1;                  //   CAN FD operation enable
    }   b;
}   can_mode_t;

typedef struct {
    TPCANHandle board;                  // board hardware channel handle
    BYTE  brd_type;                     // board type (none PnP hardware)
    DWORD brd_port;                     // board parameter: I/O port address
    WORD  brd_irq;                      // board parameter: interrupt number
    int   reset;                        // re-initialization flag (a helper)
#ifdef _BLOCKING_READ
    HANDLE event;                       // event handle for blocking read
    int    killed;                      // flag to signal Ctrl+C (SIGINT)
#endif
    can_mode_t mode;                    // operation mode of the CAN channel
    can_status_t status;                // 8-bit status register
}   can_interface_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static int pcan_error(TPCANStatus);     // PCAN specific errors
#ifdef _BLOCKING_READ
 static void pcan_signal(int signo);    // Ctrl-C handler
#endif

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
static can_interface_t can[PCAN_MAX_HANDLES];   // interface handles 
static void (*sigint)(int) = SIG_DFL;   // previous Ctrl-C handler
static char hardware[256] = "";         // hardware version of the CAN interface board
static char software[256] = "";         // software version of the CAN interface driver
static int  init = 0;                   // initialization flag


/*  -----------  functions  ----------------------------------------------
 */

int can_test(int board, unsigned char mode, const void *param, int *result)
{
    TPCANStatus rc;                     // return value
    DWORD cond;                         // channel condition
    int i;

    if(!init) {                         // when not init before:
        for(i = 0; i < PCAN_MAX_HANDLES; i++) {
            can[i].board = PCAN_NONEBUS;
            can[i].brd_type = 0;
            can[i].brd_port = 0;
            can[i].brd_irq = 0;
#ifdef _BLOCKING_READ
            /* to terminate blocking read on Ctrl-C */
            if ((sigint = signal(SIGINT, pcan_signal)) == SIG_ERR)
                return CANERR_FATAL;
#endif
        }
        init = 1;                       //   set initialization flag
    }
    if((rc = CAN_GetValue((WORD)board, PCAN_CHANNEL_CONDITION, (void*)&cond, sizeof(cond))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    if(result) {
        if((cond == PCAN_CHANNEL_AVAILABLE) || (cond == PCAN_CHANNEL_PCANVIEW))
            *result = CANBRD_PRESENT;
        if(cond == PCAN_CHANNEL_UNAVAILABLE)
            *result = CANBRD_NOT_PRESENT;
        else if(cond == PCAN_CHANNEL_OCCUPIED)
            *result = CANBRD_NOT_AVAILABLE;
        else
            return PCAN_ERR_UNKNOWN;
    }
    (void)mode;
    (void)param;
    return CANERR_NOERROR;
}

int can_init(int board, unsigned char mode, const void *param)
{
    TPCANStatus rc;                     // return value
    DWORD cond;                         // channel condition
    int i;

    if(!init) {                         // when not init before:
        for(i = 0; i < PCAN_MAX_HANDLES; i++) {
            can[i].board = PCAN_NONEBUS;
            can[i].brd_type = 0;
            can[i].brd_port = 0;
            can[i].brd_irq = 0;
        }
#ifdef _BLOCKING_READ
        /* to terminate blocking read on Ctrl-C */
        if ((sigint = signal(SIGINT, pcan_signal)) == SIG_ERR)
            return CANERR_FATAL;
#endif
        init = 1;                       // set initialization flag
    }
    for(i = 0; i < PCAN_MAX_HANDLES; i++) {
        if(can[i].board == PCAN_NONEBUS)
            break;
    }
    if(!IS_HANDLE_VALID(i))             // no free handle found
        return CANERR_HANDLE;

    if((rc = CAN_GetValue((WORD)board, PCAN_CHANNEL_CONDITION, (void*)&cond, sizeof(cond))) != PCAN_ERROR_OK)
        return pcan_error(rc);
    if(cond == PCAN_CHANNEL_UNAVAILABLE)
        return PCAN_ERR_ILLBOARD;
    else if(cond == PCAN_CHANNEL_OCCUPIED)
        return CANERR_YETINIT;
    else if((cond != PCAN_CHANNEL_AVAILABLE) && (cond != PCAN_CHANNEL_PCANVIEW))
        return PCAN_ERR_UNKNOWN;

    can[i].board = board;               // handle of the CAN channel
    if(param) {                         // non-plug'n'play devices:
        can[i].brd_type = ((struct _pcan_param*)param)->type;
        can[i].brd_port = ((struct _pcan_param*)param)->port;
        can[i].brd_irq  = ((struct _pcan_param*)param)->irq;
    }
    can[i].reset = 0;                   // clear re-initialization flag
    can[i].mode.byte = mode;            // store selected operation mode
    can[i].status.byte = CANSTAT_RESET; // CAN controller not started yet!

    return CANERR_NOERROR;
}

int can_exit(int handle)
{
    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;

    /*if(!can[handle].status.b.can_stopped) // release the CAN interface!*/
    {
        CAN_Uninitialize(can[handle].board);
#ifdef _BLOCKING_READ
        CloseHandle(can[handle].event);
#endif
    }
    can[handle].status.byte |= CANSTAT_RESET;// CAN controller in INIT state
    can[handle].board = PCAN_NONEBUS;   // handle can be used again

    return CANERR_NOERROR;
}

int can_start(int handle, const can_bitrate_t *bitrate)
{
    TPCANBaudrate baudrate;             // btr0btr1 value
    char string[256];                   // bit-rate string
    DWORD value;                        // parameter value
    TPCANStatus rc;                     // return value

    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;
    if(bitrate == NULL)                 // null-pointer assignment!
        return CANERR_NULLPTR;
    if(!can[handle].status.b.can_stopped)// must be stopped!
        return CANERR_ONLINE;

    if(bitrate->index <= 0) {           // btr0btr1 from index
        switch(bitrate->index * (-1)) {
        case CANBDR_1000: baudrate = PCAN_BAUD_1M; break;
        case CANBDR_800: baudrate = PCAN_BAUD_800K; break;
        case CANBDR_500: baudrate = PCAN_BAUD_500K; break;
        case CANBDR_250: baudrate = PCAN_BAUD_250K; break;
        case CANBDR_125: baudrate = PCAN_BAUD_125K; break;
        case CANBDR_100: baudrate = PCAN_BAUD_100K; break;
        case CANBDR_50: baudrate = PCAN_BAUD_50K; break;
        case CANBDR_20: baudrate = PCAN_BAUD_20K; break;
        case CANBDR_10: baudrate = PCAN_BAUD_10K; break;
        default: return CANERR_BAUDRATE;
        }
    }
    else if(!can[handle].mode.b.fdoe) { // btr0btr1 for CAN 2.0
        if(bitrate->btr.frequency != 8000000) return CANERR_BAUDRATE;   // SJA1000 @ 8MHz
        if((bitrate->btr.nominal.brp < 1) || (64 < bitrate->btr.nominal.brp)) return CANERR_BAUDRATE;
        if((bitrate->btr.nominal.tseg1 < 1) || (16 < bitrate->btr.nominal.tseg1)) return CANERR_BAUDRATE;
        if((bitrate->btr.nominal.tseg2 < 1) || (8 < bitrate->btr.nominal.tseg2)) return CANERR_BAUDRATE;
        if((bitrate->btr.nominal.sjw < 1) || (4 < bitrate->btr.nominal.sjw)) return CANERR_BAUDRATE;
        if((bitrate->btr.nominal.sam > 1)) return CANERR_BAUDRATE;
        /* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ */
        /* |  SJW  |          BRP          |SAM|   TSEG2   |     TSEG1     | */
        /* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ */
        baudrate = ((((TPCANBaudrate)(bitrate->btr.nominal.sjw - 1) & 0x0003) << 14)  | \
                    (((TPCANBaudrate)(bitrate->btr.nominal.brp - 1) & 0x003F) << 8)   | \
                    (((TPCANBaudrate)(bitrate->btr.nominal.sam - 1) & 0x0001) << 7)   | \
                    (((TPCANBaudrate)(bitrate->btr.nominal.tseg2 - 1) & 0x0007) << 4) | \
                    (((TPCANBaudrate)(bitrate->btr.nominal.tseg1 - 1) & 0x000F) << 0));
    }
    else {                              // a string for CAN FD
        if(can[handle].mode.b.brse)     //   long and fast frames
            sprintf_s(string, 256, "f_clock=%li,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u,"
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
                      bitrate->btr.data.sjw);
        else                            //   long frames or CAN 2.0
            sprintf_s(string, 256, "f_clock=%li,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u",
                      bitrate->btr.frequency,
                      bitrate->btr.nominal.brp,
                      bitrate->btr.nominal.tseg1,
                      bitrate->btr.nominal.tseg2,
                      bitrate->btr.nominal.sjw,
                      bitrate->btr.nominal.sam);
    }
    if(!can[handle].reset) {            // initialize CAN controller
        if(can[handle].mode.b.mon) {    //   set listen-only mode?
            value = PCAN_PARAMETER_ON;
            if((rc = CAN_SetValue(can[handle].board, PCAN_LISTEN_ONLY,
                                  (void*)&value, sizeof(value))) != PCAN_ERROR_OK)
                return pcan_error(rc);
        }
        if(can[handle].mode.b.fdoe) {   //   CAN FD operation mode?
            if((rc = CAN_InitializeFD(can[handle].board, string)) != PCAN_ERROR_OK)
                return pcan_error(rc);
        }
        else {                          //   CAN 2.0 operation mode
            if((rc = CAN_Initialize(can[handle].board, baudrate,
                                    can[handle].brd_type, can[handle].brd_port, 
                                    can[handle].brd_irq)) != PCAN_ERROR_OK)
                return pcan_error(rc);
        }
#ifdef _BLOCKING_READ
        if((can[handle].event = CreateEvent( // create an event handle
                NULL,                   //   default security attributes
                FALSE,                  //   auto-reset event
                FALSE,                  //   initial state is nonsignaled
                TEXT("PCANBasic")       //   object name
                )) == NULL)
            return CANERR_FATAL;
        if((rc = CAN_SetValue(can[handle].board, PCAN_RECEIVE_EVENT, 
                    (void*)&can[handle].event, sizeof(can[handle].event))) != PCAN_ERROR_OK) {
            CAN_Uninitialize(can[handle].board);
            return pcan_error(rc);
        }
        can[handle].killed = 0;
#endif
    }
    can[handle].status.byte = 0x00;     // clear old status bits
    can[handle].status.b.can_stopped = 0;// CAN controller started!

    return CANERR_NOERROR;
}

int can_reset(int handle)
{
    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;

    if(can[handle].status.b.can_stopped) {  // CAN started, then reset
        can[handle].reset = (CAN_Reset(can[handle].board) == PCAN_ERROR_OK);
    }
    can[handle].status.b.can_stopped = 1;   // CAN controller stopped!

    return CANERR_NOERROR;
}

int can_write(int handle, const can_msg_t *msg)
{
    TPCANMsg can_msg;                   // the message (CAN 2.0)
    TPCANMsgFD can_msg_fd;              // the message (CAN FD)
    TPCANStatus rc;                     // return value

    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;
    if(msg == NULL)                     // null-pointer assignment!
        return CANERR_NULLPTR;
    if(can[handle].status.b.can_stopped)// must be running!
        return CANERR_OFFLINE;

    if(!can[handle].mode.b.fdoe) {
        if(msg->dlc > CAN_MAX_LEN)      //   data length 0 .. 8!
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
        if(msg->dlc > CANFD_MAX_LEN)    //   data length 0 .. 64!
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
            return CANERR_TX_BUSY;      //     transmitter busy!
        }
        else if((rc & PCAN_ERROR_XMTFULL)){//transmission pending?
            can[handle].status.b.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy!
        }
        return pcan_error(rc);          //   PCAN specific error?
    }
    can[handle].status.b.transmitter_busy = 0;  // message transmitted!

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

    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;
    if(msg == NULL)                     // null-pointer assignment!
        return CANERR_NULLPTR;
    if(can[handle].status.b.can_stopped)// must be running!
        return CANERR_OFFLINE;

    if(!can[handle].mode.b.fdoe)        // read message queue
        rc = CAN_Read(can[handle].board, &can_msg, &timestamp);
    else
        rc = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
    if(rc == PCAN_ERROR_QRCVEMPTY) {
#ifdef _BLOCKING_READ
        if (!can[handle].killed) {
            switch (WaitForSingleObject(can[handle].event, (timeout != 65535) ? timeout : INFINITE)) {
            case WAIT_OBJECT_0:
                break;                  //   one or more messages received, or got signal SIGINT
            case WAIT_TIMEOUT:
                break;                  //   time-out, but look for old messages
            default:
                return CANERR_FATAL;    //   function failed!
            }
         }
#endif
        if (!can[handle].mode.b.fdoe)   // read message queue
            rc = CAN_Read(can[handle].board, &can_msg, &timestamp);
        else
            rc = CAN_ReadFD(can[handle].board, &can_msg_fd, &timestamp_fd);
        if (rc == PCAN_ERROR_QRCVEMPTY) {
            can[handle].status.b.receiver_empty = 1;
            return CANERR_RX_EMPTY;     //   receiver empty!
        }
    }
    if(rc != PCAN_ERROR_OK) {
        return pcan_error(rc);          //   something´s wrong!
    }
    if((can_msg.MSGTYPE == PCAN_MESSAGE_STATUS) || 
       (can_msg_fd.MSGTYPE == PCAN_MESSAGE_STATUS)) {
        can[handle].status.b.bus_off = (can_msg.DATA[3] & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
        can[handle].status.b.bus_error = (can_msg.DATA[3] & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
        can[handle].status.b.warning_level = (can_msg.DATA[3] & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
        can[handle].status.b.message_lost |= (can_msg.DATA[3] & PCAN_ERROR_OVERRUN) != PCAN_ERROR_OK;
        return CANERR_RX_EMPTY;         //   receiver empty!
    }
    if(!can[handle].mode.b.fdoe) {      // CAN 2.0 message:
        msg->id = can_msg.ID;
        msg->ext = can_msg.MSGTYPE & PCAN_MESSAGE_EXTENDED;
        msg->rtr = can_msg.MSGTYPE & PCAN_MESSAGE_RTR;
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
        msg->id = can_msg_fd.ID;
        msg->ext = can_msg_fd.MSGTYPE & PCAN_MESSAGE_EXTENDED;
        msg->rtr = can_msg_fd.MSGTYPE & PCAN_MESSAGE_RTR;
        msg->fdf = can_msg_fd.MSGTYPE & PCAN_MESSAGE_FD;
        msg->brs = can_msg_fd.MSGTYPE & PCAN_MESSAGE_BRS;
        msg->esi = can_msg_fd.MSGTYPE & PCAN_MESSAGE_ESI;
        msg->dlc = can_msg_fd.DLC;
        memcpy(msg->data, can_msg_fd.DATA, CANFD_MAX_LEN);
        msg->timestamp.sec = (long)(timestamp_fd / 1000ull);
        msg->timestamp.usec = (long)(timestamp_fd % 1000ull);
    }
    can[handle].status.b.receiver_empty = 0;        // message read!

    return CANERR_NOERROR;
}

int can_status(int handle, unsigned char *status)
{
    TPCANStatus rc;                     // return value

    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;

    if(!can[handle].status.b.can_stopped)   {   // must be running:
        if((rc = CAN_GetStatus(can[handle].board)) > 255)
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
    TPCANStatus rc;                     // return value

    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;

    if(!can[handle].status.b.can_stopped)   {   // must be running:
        if((rc = CAN_GetStatus(can[handle].board)) > 255)
            return pcan_error(rc);
        can[handle].status.b.bus_off = (rc & PCAN_ERROR_BUSOFF) != PCAN_ERROR_OK;
        can[handle].status.b.bus_error = (rc & PCAN_ERROR_BUSPASSIVE) != PCAN_ERROR_OK;
        can[handle].status.b.warning_level = (rc & PCAN_ERROR_BUSWARNING) != PCAN_ERROR_OK;
        can[handle].status.b.message_lost |= (rc & (PCAN_ERROR_OVERRUN | PCAN_ERROR_QOVERRUN)) != PCAN_ERROR_OK;
        can[handle].status.b.transmitter_busy |= (rc & (PCAN_ERROR_XMTFULL | PCAN_ERROR_QXMTFULL)) != PCAN_ERROR_OK;
    }
    if(status)                          // status-register
      *status = can[handle].status.byte;
    if(load)                            // bus-load
      *load = 255;

    return CANERR_NOERROR;
}

int can_interface(int handle, int *board, unsigned char *mode, void *param)
{

    if(!init)                           // must be initialized!
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return CANERR_HANDLE;
    if(board == NULL || mode == NULL)   // null-pointer assignment!
        return CANERR_NULLPTR;

    *board = can[handle].board;         // handle of the CAN channel
    *mode  = can[handle].mode.byte;     // current opperation mode
    if(param) {                         // non-plug'n'play devices:
        ((struct _pcan_param*)param)->type = can[handle].brd_type;
        ((struct _pcan_param*)param)->port = can[handle].brd_port;
        ((struct _pcan_param*)param)->irq  = can[handle].brd_irq;
    }
    return CANERR_NOERROR;
}

char *can_hardware(int handle)
{
    char  str[256], *ptr;               // info string
    DWORD dev = 0x0000UL;               // device number

    if(!init)                           // must be initialized!
        return NULL;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle!
        return NULL;

    if(CAN_GetValue(can[handle].board, PCAN_CHANNEL_VERSION, (void*)str, 256) != PCAN_ERROR_OK)
        return NULL;
    if((ptr = strchr(str, '\n')) != NULL)
       *ptr = '\0';
    if((((can[handle].board & 0x00F0) >> 8) == PCAN_USB) ||
       (((can[handle].board & 0x0F00) >> 16) == PCAN_USB))
    {
        if(CAN_GetValue(can[handle].board, PCAN_DEVICE_NUMBER, (void*)&dev, 4) != PCAN_ERROR_OK)
            return NULL;
        _snprintf_s(hardware, 256, 256, "%s (Device %02lXh)", str, dev);
    }
    else
        strcpy_s(hardware, 256, str);

    return (char*)hardware;     // hardware version
}

char *can_software(int handle)
{
    char  str[256] = "PCAN-Basic API "; // info string

    if(!init)                           // must be initialized!
        return NULL;
    (void)handle;                       // handle not needed here

    if(CAN_GetValue(PCAN_NONEBUS, PCAN_API_VERSION, (void*)&str[15], 256-15) != PCAN_ERROR_OK)
        return NULL;
    strcpy_s(software, 256, str);

    return (char*)software;             // software version
}

int can_library(int *library)
{
    //if(!init)                         // must be initialized!
    //  return CANERR_NOTINIT;
    if(library == NULL)                 // null-pointer assignment!
        return CANERR_NULLPTR;

    *library = PCAN_LIB_ID;             // library ID

    return CANERR_NOERROR;
}

/*  -----------  local functions  ----------------------------------------
 */

static int pcan_error(TPCANStatus status)
{
    if((status & PCAN_ERROR_REGTEST)      == PCAN_ERROR_REGTEST)        return PCAN_ERR_REGTEST;
    if((status & PCAN_ERROR_NODRIVER)     == PCAN_ERROR_NODRIVER)       return PCAN_ERR_NODRIVER;
    if((status & PCAN_ERROR_ILLHANDLE)    == PCAN_ERROR_HWINUSE)        return PCAN_ERR_HWINUSE;
    if((status & PCAN_ERROR_ILLHANDLE)    == PCAN_ERROR_NETINUSE)       return PCAN_ERR_NETINUSE;
    if((status & PCAN_ERROR_ILLHANDLE)    == PCAN_ERROR_ILLCLIENT)      return PCAN_ERR_ILLCLIENT;
    if((status & PCAN_ERROR_ILLHANDLE)    == PCAN_ERROR_ILLNET)         return PCAN_ERR_ILLNET;
    if((status & PCAN_ERROR_ILLHANDLE)    == PCAN_ERROR_ILLHW)          return PCAN_ERR_ILLHW;
    if((status & PCAN_ERROR_RESOURCE)     == PCAN_ERROR_RESOURCE)       return PCAN_ERR_RESOURCE;
    if((status & PCAN_ERROR_ILLPARAMTYPE) == PCAN_ERROR_ILLPARAMTYPE)   return PCAN_ERR_ILLPARAMTYPE;
    if((status & PCAN_ERROR_ILLDATA)      == PCAN_ERROR_ILLDATA)        return PCAN_ERR_ILLDATA;
    if((status & PCAN_ERROR_ILLOPERATION) == PCAN_ERROR_ILLOPERATION)   return PCAN_ERR_ILLOPERATION;
    if((status & PCAN_ERROR_CAUTION)      == PCAN_ERROR_CAUTION)        return PCAN_ERR_CAUTION;
    if((status & PCAN_ERROR_INITIALIZE)   == PCAN_ERROR_INITIALIZE)     return CANERR_NOTINIT;
    if((status & 0xFFFFFF00))
        return PCAN_ERR_UNKNOWN;
    else
        return CANERR_NOERROR;
}

#ifdef _BLOCKING_READ
 static void pcan_signal(int signo)
 {
    int i;

    //printf("%s: got signal %d\n", __FILE__, signo);
    for (i = 0; i < PCAN_MAX_HANDLES; i++) {
        if (can[i].board != PCAN_NONEBUS) {
            SetEvent(can[i].event);
            can[i].killed = 1;
        }
    }
    if ((signo == SIGINT) && (sigint != NULL))
        sigint(signo);
 }
#endif

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
