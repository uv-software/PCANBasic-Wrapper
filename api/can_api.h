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
 *  export    :  int can_test(int board, unsigned char mode, const void *param, int *result);
 *               int can_init(int board, unsigned char mode, const void *param);
 *               int can_exit(int handle);
 *               int can_start(int handle, const can_bitrate_t *bitrate);
 *               int can_reset(int handle);
 *               int can_write(int handle, const can_msg_t *msg);
 *               int can_read(int handle, can_msg_t *msg, unsigned short timeout);
 *               int can_status(int handle, unsigned char *status);
 *               int can_busload(int handle, unsigned char *load, unsigned char *status);
 *               int can_bitrate(int handle, can_bitrate_t *bitrate, unsigned char *status);
 *               int can_interface(int handle, int *board, unsigned char *mode, void *param);
 *               char *can_hardware(int handle);
 *               char *can_software(int handle);
 *               int can_library(unsigned short *version, unsigned char *revision, unsigned long *build);
 *               char *can_version();
 *
 *  includes  :  can_defs.h
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
 *               For PEAK PCAN-Basic Interfaces (V4.3.4.246):
 *               - PCAN-USB Interface (channel 1 - 16)
 *               - PCAN-USB FD Interface (channel 1 - 16)
 *  @note        Up to 16 handles are supported by the API.
 *
 *  @author      $Author$
 *
 *  @version     $Rev$
 *
 *  @defgroup    can_api CAN Interface API, Version 3
 *  @{
 */
#ifndef __CAN_API_H
#define __CAN_API_H

/*  -----------  options  ------------------------------------------------
 */
#ifdef _CANAPI_EXPORTS
    #define CANAPI  __declspec(dllexport)
#else
  #ifndef _CANAPI_EXTERN
    #define CANAPI  __declspec(dllimport)
  #else
    #define CANAPI  extern
  #endif
#endif

/*  -----------  includes  -----------------------------------------------
 */

#include "can_defs.h"                   /* CAN definitions and options */


/*  -----------  defines  ------------------------------------------------
 */

/** @name  CAN Identifier
 *  @brief CAN Identifier range
 *  @{ */
#define CAN_MAX_STD_ID  (0x7FF)
#define CAN_MAX_XTD_ID  (0x1FFFFFFF)
/** @} */

/** @name  CAN Data Length
 *  @brief CAN payload length and DLC definition
 *  @{ */
#define CAN_MAX_DLC (8)
#define CAN_MAX_LEN (8)
/** @} */

/** @name  CAN FD Data Length
 *  @brief CAN FD payload length and DLC definition
 *  @{ */
#define CANFD_MAX_DLC (15)
#define CANFD_MAX_LEN (64)
/** @} */

/** @name  CAN Baud-rate Indexes
 *  @brief CAN baud-rate indexes defined by CiA
 *  @note  They must be passed with a minus sign to can_start()
 *  @{ */
#define CANBDR_1000                  0  /**< baud rate: 1000 kBit/s */
#define CANBDR_800                   1  /**< baud rate:  800 kBit/s */
#define CANBDR_500                   2  /**< baud rate:  500 kBit/s */
#define CANBDR_250                   3  /**< baud rate:  250 kBit/s */
#define CANBDR_125                   4  /**< baud rate:  125 kBit/s */
#define CANBDR_100                   5  /**< baud rate:  100 kBit/s */
#define CANBDR_50                    6  /**< baud rate:   50 kBit/s */
#define CANBDR_20                    7  /**< baud rate:   20 kBit/s */
#define CANBDR_10                    8  /**< baud rate:   10 kBit/s */
/** @} */

/** @name  CAN Mode Flags
 *  @brief Flags to control the operation mode
 *  @{ */
#define CANMODE_FDOE              0x80  /**< CAN FD operation enable */
#define CANMODE_BRSE              0x40  /**< bit-rate switch enable */
#define CANMODE_NISO              0x10  /**< Non-ISO CAN FD enable */
#define CANMODE_TXP               0x08  /**< transmit pause enable/disable */
#define CANMODE_DAR               0x02  /**< disable automatic retransmission */
#define CANMODE_MON               0x01  /**< monitor mode enable/disable */
#define CANMODE_DEFAULT           0x00  /**< CAN 2.0 operation mode */
/** @} */

/** @name  CAN Error Codes
 *  @brief General CAN error codes (negative)
 *  @note  Codes less or equal than -100 are for vendor-specific error codes
 *         and codes less or equal than -10000 are for OS-specific error codes
 *         (add 10000 to get the reported OS error code, e.g. errno).
 *  @{ */
#define CANERR_NOERROR               0  /**< no error! */
#define CANERR_BOFF                 -1  /**< CAN - busoff status */
#define CANERR_EWRN                 -2  /**< CAN - error warning status */
#define CANERR_BERR                 -3  /**< CAN - bus error */
#define CANERR_OFFLINE              -9  /**< CAN - not started */
#define CANERR_ONLINE               -8  /**< CAN - already started */
#define CANERR_MSG_LST              -10 /**< CAN - message lost */
#define CANERR_LEC_STUFF            -11 /**< LEC - stuff error */
#define CANERR_LEC_FORM             -12 /**< LEC - form error */
#define CANERR_LEC_ACK              -13 /**< LEC - acknowledge error */
#define CANERR_LEC_BIT1             -14 /**< LEC - recessive bit error */
#define CANERR_LEC_BIT0             -15 /**< LEC - dominant bit error */
#define CANERR_LEC_CRC              -16 /**< LEC - checksum error */
#define CANERR_TX_BUSY              -20 /**< USR - transmitter busy */
#define CANERR_RX_EMPTY             -30 /**< USR - receiver empty */
#define CANERR_TIMEOUT              -50 /**< USR - time-out */
#define CANERR_BAUDRATE             -91 /**< USR - illegal baudrate */
#define CANERR_HANDLE               -92 /**< USR - illegal handle */
#define CANERR_ILLPARA              -93 /**< USR - illegal parameter */
#define CANERR_NULLPTR              -94 /**< USR - null-pointer assignment */
#define CANERR_NOTINIT              -95 /**< USR - not initialized */
#define CANERR_YETINIT              -96 /**< USR - already initialized */
#define CANERR_NOTSUPP              -98 /**< USR - not supported */
#define CANERR_FATAL                -99 /**< USR - other errors */
/** @} */

/** @name  CAN Status Codes
 *  @brief CAN status from CAN controller
 *  @{ */
#define CANSTAT_RESET             0x80  /**< CAN status: controller stopped */
#define CANSTAT_BOFF              0x40  /**< CAN status: busoff status */
#define CANSTAT_EWRN              0x20  /**< CAN status: error warning level */
#define CANSTAT_BERR              0x10  /**< CAN status: bus error (LEC) */
#define CANSTAT_TX_BUSY           0x08  /**< CAN status: transmitter busy */
#define CANSTAT_RX_EMPTY          0x04  /**< CAN status: receiver empty */
#define CANSTAT_MSG_LST           0x02  /**< CAN status: message lost */
#define CANSTAT_QUE_OVR           0x01  /**< CAN status: event-queue overrun */
/** @} */

/** @name  Board Test Codes
 *  @brief Results of the board test
 *  @{ */
#define CANBRD_NOT_PRESENT         (-1) /**< CAN board not present */
#define CANBRD_PRESENT              (0) /**< CAN board present */
#define CANBRD_NOT_AVAILABLE       (+1) /**< CAN board present, but occupied */
/** @} */

/** @name  Blocking Read
 *  @brief Control of blocking read
 *  @{ */
#define CANREAD_INFINITE         65535u /**< infinite timeout (blocking read) */
#define CANKILL_ALL                (-1) /**< to signal all CAN interfaces */
/** @} */

/** @name  Legacy Stuff
 *  @brief For compatibility
 *  @{ */
#define can_transmit(hnd, msg)      can_write(hnd, msg)
#define can_receive(hnd, msg)       can_read(hnd, msg, 0u)
#define _CAN_STATE                  _CAN_STATUS
#define _can_state_t                _can_status_t
#define  can_state_t                 can_status_t
/** @} */

/*  -----------  types  --------------------------------------------------
 */

#ifndef _CAN_BOARD
#define _CAN_BOARD
 /** @brief      CAN Interface board:
  */
 typedef struct _can_board_t
 {
   unsigned long type;                  /**< board type */
   char* name;                          /**< board name */
 } can_board_t;
#endif
#ifndef _CAN_STATUS
#define _CAN_STATUS
 /** @brief      CAN Status-register:
  */
 typedef union _can_status_t
 {
   unsigned char byte;                  /**< byte access */
   struct {                             /*   bit access: */
     unsigned char queue_overrun : 1;   /**<   Event-queue overrun */
     unsigned char message_lost : 1;    /**<   Message lost */
     unsigned char receiver_empty : 1;  /**<   Receiver empty */
     unsigned char transmitter_busy : 1;/**<   Transmitter busy */
     unsigned char bus_error : 1;       /**<   Bus error (LEC) */
     unsigned char warning_level : 1;   /**<   Error warning status */
     unsigned char bus_off : 1;         /**<   Busoff status */
     unsigned char can_stopped : 1;     /**<   CAN controller stopped */
   } b;
 } can_status_t;
#endif
/** @brief      CAN Bit-rate (nominal and data):
 */
typedef union _can_bitrate_t
{
    long index;                         /**< index to predefined bit-rate (<= 0) */
    struct {                            /*   bit-timing register: */
        long frequency;                 /**<   clock domain (frequency in [Hz]) */
        struct {                        /*     nominal bus speed: */
            unsigned short brp;         /**<     bit-rate prescaler */
            unsigned short tseg1;       /**<     TSEG1 segment */
            unsigned short tseg2;       /**<     TSEG2 segment */
            unsigned short sjw;         /**<     synchronization jump width */
            unsigned char  sam;         /**<     number of samples (SJA1000) */
        }   nominal;                    /**<   nominal bus speed */
#ifndef CAN_20_ONLY
        struct {                        /*     data bus speed: */
            unsigned short brp;         /**<     bit-rate prescaler */
            unsigned short tseg1;       /**<     TSEG1 segment */
            unsigned short tseg2;       /**<     TSEG2 segment */
            unsigned short sjw;         /**<     synchronization jump width */
        }   data;                       /**<   data bus speed */
#endif
    }   btr;                            /**< bit-timing register */
}   can_bitrate_t;
/** @brief      CAN Message (with Time-stamp):
 */
typedef struct _can_msg_t {
    unsigned long id;                   /**< CAN identifier */
    int rtr;                            /**< flag: RTR frame */
    int ext;                            /**< flag: extended format */
#ifndef CAN_20_ONLY
    int fdf;                            /**< flag: CAN FD format */
    int brs;                            /**< flag: bit-rate switching */
    int esi;                            /**< flag: error state indicator */
    unsigned char dlc;                  /**< data length code (0 .. 15) */
    unsigned char data[CANFD_MAX_LEN];  /**< payload (CAN FD:  0 .. 64) */
#else
    unsigned char dlc;                  /**< data length code (0 .. 8) */
    unsigned char data[CAN_MAX_LEN];    /**< payload (CAN 2.0: 0 .. 8) */
#endif
    struct _timestamp {                 /*   time-stamp: */
        long sec;                       /**<   seconds */
        long usec;                      /**<   microseconds */
    } timestamp;
}   can_msg_t;

/*  -----------  variables  ----------------------------------------------
 */

#ifndef _CANAPI_EXPORTS
 CANAPI can_board_t can_board[];        /**< list of CAN interface boards */
#endif


/*  -----------  prototypes  ---------------------------------------------
 */

/** @brief       tests if the CAN interface (hardware and driver) given by
 *               the argument 'board' is present.
 *
 *  @param[in]   board   - type of the CAN interface board.
 *  @param[in]   mode    - operation mode to be checked.
 *  @param[in]   param   - pointer to board-specific parameters.
 *  @param[out]  result  - result of the board test:
 *                             < 0 - board is not present;
 *                             = 0 - board is present;
 *                             > 0 - board is present, but in use.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_test(int board, unsigned char mode, const void *param, int *result);


/** @brief       initializes the CAN interface (hardware and driver) by loading
 *               and starting the appropriate DLL for the specified CAN board
 *               given by the argument 'board'. 
 *               The operation status of the CAN interface is set to 'stopped';
 *               no communication is possible in this state.
 *
 *  @param[in]   board   - type of the CAN interface board.
 *  @param[in]   mode    - operation mode of the CAN controller.
 *  @param[in]   param   - pointer to board-specific parameters.
 *
 *  @returns     handle of the CAN interface if successful,
 *               or a negative value on error.
 */
CANAPI int can_init(int board, unsigned char mode, const void *param);


/** @brief       stops any operation of the CAN interface and sets the operation
 *               status to 'offline'.
 *
 *  @note        The handle is invalid after this operation and may be assigned
 *               to a different CAN interface board by a call to can_init().
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_exit(int handle);


/** @brief       initializes the bit-timing register of the CAN interface with
 *               the parameters of the bit-timing table selected by the baudrate
 *               index and sets the operation status to 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[in]   bitrate - bit-rate as btr register or baud rate index.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_start(int handle, const can_bitrate_t *bitrate);


/** @brief       stops any operation of the CAN interface and sets the operation
 *               status to 'stopped'; no communication is possible in this state.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_reset(int handle);


/** @brief       transmits a message over the CAN bus. The CAN interface must be
 *               in operation status 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[in]   msg     - pointer to the message to send.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_write(int handle, const can_msg_t *msg);


/** @brief       read one message from the message queue of the CAN interface, if
 *               any message was received. The CAN interface must be in operation
 *               status 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[out]  msg     - pointer to a message buffer.
 *  @param[in]   timeout - time to wait for the reception of a message:
 *                              0 means the function returns immediately,
 *                              65535 means blocking read, and any other
 *                              value means the time to wait im milliseconds.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_read(int handle, can_msg_t *msg, unsigned short timeout);


/** @brief       signals a waiting event object of the CAN interface. This is
 *               used to terminat a blocking read operation (e.g. by means of
 *               a Ctrl-C handler or similar).
 *
 *  @remark      The PCAN-Basic DLL uses an event object to realize a blocking
 *               read by a call to WaitForSingleObject, but this event object
 *               is not terminated by Ctrl-C (SIGINT).
 *
 *  @note        SIGINT is not supported for any Win32 application. [MSVC Docs]
 *
 *  @param[in]   handle  - handle of the CAN interface, or (-1) for all.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
#if defined (_WIN32) || defined(_WIN64)
 CANAPI int can_kill(int handle);
#endif


/** @brief       retrieves the status register of the CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[out]  status  - 8-bit status register.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_status(int handle, unsigned char *status);


/** @brief       retrieves the bus-load (in percent) of the CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[out]  load    - bus-load in [percent].
 *  @param[out]  status  - 8-bit status register.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_busload(int handle, unsigned char *load, unsigned char *status);


/** @brief       retrieves the bit-rate setting of the CAN interface. The
 *               CAN interface must be in operation status 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[out]  bitrate - bit-rate setting.
 *  @param[out]  status  - 8-bit status register.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_bitrate(int handle, can_bitrate_t *bitrate, unsigned char *status);


/** @brief       retrieves operation information of the CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[out]  board   - type of the CAN interface board.
 *  @param[out]  mode    - operation mode of the CAN controller.
 *  @param[out]  param   - pointer to board-specific parameters.
 *
 *  @returns     0 if successful, or a negative value on error.
 */
CANAPI int can_interface(int handle, int *board, unsigned char *mode, void *param);


/** @brief       retrieves the hardware version of the CAN interface
 *               as a zero-terminated string.
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char *can_hardware(int handle);


/** @brief       retrieves the firmware version of the CAN interface
 *               as a zero-terminated string.
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char *can_software(int handle);


/** @brief      retrieves the library number (ID) of the CAN interface.
 *
 *  @param[out] version  - version number (high byte = major, low byte = minor).
 *  @param[out] revision - revision number (e.g. for service releases).
 *  @param[out] build    - build number (taken from svn or git).
 *
 *  @returns    library number if successful, or a negative value on error.
 */
CANAPI int can_library(unsigned short *version, unsigned char *revision, unsigned long *build);


/** @brief       retrieves version information of the CAN API V300
 *               as a zero-terminated string.
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char* can_version();


#endif /* __CAN_API_H */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
