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
 *               int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed);
 *               int can_property(int handle, int param, void *value, int nbytes);
 *               char *can_hardware(int handle);
 *               char *can_software(int handle);
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
#ifndef CAN_API_H_INCLUDED
#define CAN_API_H_INCLUDED

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

/** @name  CAN API Revision Number
 *  @brief Revision number of the CAN API wrapper specification
 */
#define CAN_API_SPEC            0x0300u /**< CAN API revision number! */
/** @} */

/** @name  CAN Identifier
 *  @brief CAN Identifier range
 *  @{ */
#define CAN_MAX_STD_ID          (0x7FF) /**< highest 11-bit identifier */
#define CAN_MAX_XTD_ID     (0x1FFFFFFF) /**< highest 29-bit identifier */
/** @} */

/** @name  CAN Data Length
 *  @brief CAN payload length and DLC definition
 *  @{ */
#define CAN_MAX_DLC                 (8) /**< max. data lenth code (CAN 2.0) */
#define CAN_MAX_LEN                 (8) /**< max. payload length (CAN 2.0) */
/** @} */

/** @name  CAN FD Data Length
 *  @brief CAN FD payload length and DLC definition
 *  @{ */
#define CANFD_MAX_DLC             (15) /**< max. data lenth code (CAN FD) */
#define CANFD_MAX_LEN             (64) /**< max. payload length (CAN FD) */
/** @} */

/** @name  CAN Baud Rate Indexes (for compatibility)
 *  @brief CAN baud rate indexes defined by CiA (for CANopen)
 *  @note  They must be passed with a minus sign to can_start()
 *  @{ */
#define CANBDR_1000                  0  /**< baud rate: 1000 kbit/s */
#define CANBDR_800                   1  /**< baud rate:  800 kbit/s */
#define CANBDR_500                   2  /**< baud rate:  500 kbit/s */
#define CANBDR_250                   3  /**< baud rate:  250 kbit/s */
#define CANBDR_125                   4  /**< baud rate:  125 kbit/s */
#define CANBDR_100                   5  /**< baud rate:  100 kbit/s */
#define CANBDR_50                    6  /**< baud rate:   50 kbit/s */
#define CANBDR_20                    7  /**< baud rate:   20 kbit/s */
#define CANBDR_10                    8  /**< baud rate:   10 kbit/s */
/** @} */

/** @name  CAN 2.0 Predefined Bit-rates
 *  @brief Indexes to predefined bit-rates (CAN 2.0 only!)
 *  @{ */
#define CANBTR_INDEX_1M            (0l) /**< bit-rate: 1000 kbit/s */
#define CANBTR_INDEX_800K         (-1l) /**< bit-rate:  800 kbit/s */
#define CANBTR_INDEX_500K         (-2l) /**< bit-rate:  500 kbit/s */
#define CANBTR_INDEX_250K         (-3l) /**< bit-rate:  250 kbit/s */
#define CANBTR_INDEX_125K         (-4l) /**< bit-rate:  125 kbit/s */
#define CANBTR_INDEX_100K         (-5l) /**< bit-rate:  100 kbit/s */
#define CANBTR_INDEX_50K          (-6l) /**< bit-rate:   50 kbit/s */
#define CANBTR_INDEX_20K          (-7l) /**< bit-rate:   20 kbit/s */
#define CANBTR_INDEX_10K          (-8l) /**< bit-rate:   10 kbit/s */
/** @} */

/** @name  CAN Controller Frequencies
 *  @brief Frequencies for calculation of the bit-rate
 *  @note  Usable frequencies depend on the microcontroller used
 *  @{ */
#define CANBTR_FREQ_80MHz     80000000l /**< frequency: 80 MHz */
#define CANBTR_FREQ_60MHz     60000000l /**< frequency: 60 MHz */
#define CANBTR_FREQ_40MHz     40000000l /**< frequency: 40 MHz */
#define CANBTR_FREQ_30MHz     30000000l /**< frequency: 30 MHz */
#define CANBTR_FREQ_24MHz     24000000l /**< frequency: 24 MHz */
#define CANBTR_FREQ_20MHz     20000000l /**< frequency: 20 MHz */
#define CANBTR_FREQ_SJA1000    8000000l /**< frequency:  8 MHz */
/** @} */

/** @name  CAN 2.0 and CAN FD Nominal Bit-rate Settings
 *  @brief Limits for nominal bit-rate settings
 *  @{ */
#define CANBTR_NOMINAL_BRP_MIN      1u  /**< min. bit-timing prescaler */
#define CANBTR_NOMINAL_BRP_MAX   1024u  /**< max. bit-timing prescaler */
#define CANBTR_NOMINAL_TSEG1_MIN    1u  /**< min. time segment 1 (before SP) */
#define CANBTR_NOMINAL_TSEG1_MAX  256u  /**< max. time segment 1 (before SP) */
#define CANBTR_NOMINAL_TSEG2_MIN    1u  /**< min. time segment 2 (after SP) */
#define CANBTR_NOMINAL_TSEG2_MAX  128u  /**< max. time segment 2 (after SP) */
#define CANBTR_NOMINAL_SJW_MIN      1u  /**< min. syncronization jump width */
#define CANBTR_NOMINAL_SJW_MAX    128u  /**< max. syncronization jump width */
/** @} */

/** @name  CAN FD Data Bit-rate Settings
 *  @brief Limits for data bit-rate settings
 *  @{ */
#define CANBTR_DATA_BRP_MIN         1u  /**< min. baud rate prescaler */
#define CANBTR_DATA_BRP_MAX      1024u  /**< max. baud rate prescaler */
#define CANBTR_DATA_TSEG1_MIN       1u  /**< min. time segment 1 (before SP) */
#define CANBTR_DATA_TSEG1_MAX      32u  /**< max. time segment 1 (before SP) */
#define CANBTR_DATA_TSEG2_MIN       1u  /**< min. time segment 2 (after SP) */
#define CANBTR_DATA_TSEG2_MAX      16u  /**< max. time segment 2 (after SP) */
#define CANBTR_DATA_SJW_MIN         1u  /**< min. syncronization jump width */
#define CANBTR_DATA_SJW_MAX        16u  /**< max. syncronization jump width */
/** @} */

/** @name  SJA1000 Bit-rate Settings (CAN 2.0 only)
 *  @brief Limits for bit-rate settings of the SJA1000 CAN controller
 *  @{ */
#define CANBTR_SJA1000_BRP_MIN      1u  /**< min. baud rate prescaler */
#define CANBTR_SJA1000_BRP_MAX     64u  /**< max. baud rate prescaler */
#define CANBTR_SJA1000_TSEG1_MIN    1u  /**< min. time segment 1 (before SP) */
#define CANBTR_SJA1000_TSEG1_MAX   16u  /**< max. time segment 1 (before SP) */
#define CANBTR_SJA1000_TSEG2_MIN    1u  /**< min. time segment 2 (after SP) */
#define CANBTR_SJA1000_TSEG2_MAX    8u  /**< max. time segment 2 (after SP) */
#define CANBTR_SJA1000_SJW_MIN      1u  /**< min. syncronization jump width */
#define CANBTR_SJA1000_SJW_MAX      4u  /**< max. syncronization jump width */
#define CANBTR_SJA1000_SAM_MIN      0u  /**< single: the bus is sampled once */
#define CANBTR_SJA1000_SAM_MAX      1u  /**< triple: the bus is sampled three times */
/** @} */

/** @name  CAN Mode Flags
 *  @brief Flags to control the operation mode
 *  @{ */
#define CANMODE_FDOE              0x80u /**< CAN FD operation enable/disable */
#define CANMODE_BRSE              0x40u /**< bit-rate switch enable/disable */
#define CANMODE_NISO              0x20u /**< Non-ISO CAN FD enable/disable */
#define CANMODE_SHRD              0x10u /**< shared access enable/disable */
#define CANMODE_NXTD              0x08u /**< extended format disable/enable */
#define CANMODE_NRTR              0x04u /**< remote frames disable/enable */
#define CANMODE_ERR               0x02u /**< error frames enable/disable */
#define CANMODE_MON               0x01u /**< monitor mode enable/disable */
#define CANMODE_DEFAULT           0x00u /**< CAN 2.0 operation mode */
/** @} */

/** @name  CAN Error Codes
 *  @brief General CAN error codes (negative)
 *  @note  Codes less or equal than -100 are for vendor-specific error codes
 *         and codes less or equal than -10000 are for OS-specific error codes
 *         (add 10000 to get the reported OS error code, e.g. errno).
 *  @{ */
#define CANERR_NOERROR              (0) /**< no error! */
#define CANERR_BOFF                (-1) /**< CAN - busoff status */
#define CANERR_EWRN                (-2) /**< CAN - error warning status */
#define CANERR_BERR                (-3) /**< CAN - bus error */
#define CANERR_OFFLINE             (-9) /**< CAN - not started */
#define CANERR_ONLINE              (-8) /**< CAN - already started */
#define CANERR_MSG_LST            (-10) /**< CAN - message lost */
#define CANERR_LEC_STUFF          (-11) /**< LEC - stuff error */
#define CANERR_LEC_FORM           (-12) /**< LEC - form error */
#define CANERR_LEC_ACK            (-13) /**< LEC - acknowledge error */
#define CANERR_LEC_BIT1           (-14) /**< LEC - recessive bit error */
#define CANERR_LEC_BIT0           (-15) /**< LEC - dominant bit error */
#define CANERR_LEC_CRC            (-16) /**< LEC - checksum error */
#define CANERR_TX_BUSY            (-20) /**< USR - transmitter busy */
#define CANERR_RX_EMPTY           (-30) /**< USR - receiver empty */
#define CANERR_ERR_FRAME          (-40) /**< USR - error frame */
#define CANERR_TIMEOUT            (-50) /**< USR - time-out */
#define CANERR_BAUDRATE           (-91) /**< USR - illegal baudrate */
#define CANERR_HANDLE             (-92) /**< USR - illegal handle */
#define CANERR_ILLPARA            (-93) /**< USR - illegal parameter */
#define CANERR_NULLPTR            (-94) /**< USR - null-pointer assignment */
#define CANERR_NOTINIT            (-95) /**< USR - not initialized */
#define CANERR_YETINIT            (-96) /**< USR - already initialized */
#define CANERR_LIBRARY            (-97) /**< USR - illegal library */
#define CANERR_NOTSUPP            (-98) /**< USR - not supported */
#define CANERR_FATAL              (-99) /**< USR - other errors */
/** @} */

/** @name  CAN Status Codes
 *  @brief CAN status from CAN controller
 *  @{ */
#define CANSTAT_RESET             0x80u /**< CAN status: controller stopped */
#define CANSTAT_BOFF              0x40u /**< CAN status: busoff status */
#define CANSTAT_EWRN              0x20u /**< CAN status: error warning level */
#define CANSTAT_BERR              0x10u /**< CAN status: bus error (LEC) */
#define CANSTAT_TX_BUSY           0x08u /**< CAN status: transmitter busy */
#define CANSTAT_RX_EMPTY          0x04u /**< CAN status: receiver empty */
#define CANSTAT_MSG_LST           0x02u /**< CAN status: message lost */
#define CANSTAT_QUE_OVR           0x01u /**< CAN status: event-queue overrun */
/** @} */

/** @name  Board Test Codes
 *  @brief Results of the board test
 *  @{ */
#define CANBRD_NOT_PRESENT         (-1) /**< CAN board not present */
#define CANBRD_PRESENT              (0) /**< CAN board present */
#define CANBRD_OCCUPIED            (+1) /**< CAN board present, but occupied */
#define CANBRD_NOT_TESTABLE        (-2) /**< CAN board not testable (e.g. legacy API) */
/** @} */

/** @name  Blocking Read
 *  @brief Control of blocking read
 *  @{ */
#define CANREAD_INFINITE         65535u /**< infinite time-out (blocking read) */
#define CANKILL_ALL                (-1) /**< to signal all waiting event objects */
/** @} */

/** @name  Property IDs
 *  @brief Properties which can be read or written  
 *  @{ */
#define CANPROP_GET_SPEC             0  /**< version of the wrapper specification (USHORT) */
#define CANPROP_GET_VERSION          1  /**< version number of the library (USHORT) */
#define CANPROP_GET_REVISION         2  /**< revision number of the library (UCHAR) */
#define CANPROP_GET_BUILD_NO         3  /**< build number of the library (ULONG) */
#define CANPROP_GET_LIBRARY_ID       4  /**< library id of the library (int) */
#define CANPROP_GET_LIBRARY_VENDOR   5  /**< vendor name of the library (char[256]) */
#define CANPROP_GET_LIBRARY_DLLNAME  6  /**< file name of the library DLL (char[256]) */
#define CANPROP_GET_BOARD_TYPE      10  /**< board type of the CAN interface (int) */
#define CANPROP_GET_BOARD_NAME      11  /**< board name of the CAN interface (char[256]) */
#define CANPROP_GET_BOARD_VENDOR    12  /**< vendor name of the CAN interface (char[256]) */
#define CANPROP_GET_BOARD_DLLNAME   13  /**< file name of the CAN interface DLL(char[256]) */
#define CANPROP_GET_BOARD_PARAM     14  /**< board parameter of the CAN interface (char[256]) */
#define CANPROP_GET_OP_CAPABILITY   15  /**< supported operation modes of the CAN controller (UCHAR) */
#define CANPROP_GET_OP_MODE         16  /**< active operation mode of the CAN controller (UCHAR) */
#define CANPROP_GET_BITRATE         17  /**< active bit-rate of the CAN controller (can_bitrate_t) */
#define CANPROP_GET_SPEED           18  /**< active bus speed of the CAN controller (can_speed_t) */
#define CANPROP_GET_STATUS          19  /**< current status register of the CAN controller (UCHAR) */
#define CANPROP_GET_BUSLOAD         20  /**< current bus load of the CAN controller (UCHAR) */
#define CANPROP_GET_TX_COUNTER      24  /**< total number of sent messages (ULONGONG) */
#define CANPROP_GET_RX_COUNTER      25  /**< total number of reveiced messages (ULONGONG) */
#define CANPROP_GET_ERR_COUNTER     26  /**< total number of reveiced error frames (ULONGONG) */
/* - -  access to vendor-specific properties  - - - - - - - - - - - - - */
#define CANPROP_GET_VENDOR_PROP    256  /**< get a vendor-specific property value (void*) */
#define CANPROP_SET_VENDOR_PROP    512  /**< set a vendor-specific property value (void*) */
#define CANPROP_VENDOR_PROP_RANGE  256  /**< range for vendor-specific property values */
#define CANPROP_BUFFER_SIZE        256  /**< max. buffer size for property values */
/** @} */

/** @name  Legacy Stuff
 *  @brief For compatibility reasons
 *  @{ */
#define can_transmit(hnd, msg)          can_write(hnd, msg)
#define can_receive(hnd, msg)           can_read(hnd, msg, 0u)
#define _CAN_STATE                      _CAN_STATUS
#define _can_state_t                    _can_status_t
#define  can_state_t                     can_status_t
#define _timestamp                      _can_timestamp_t
/** @} */

/** @name  Aliases
 *  @brief Alternative names
 *  @{ */
typedef int                             can_handle_t;
typedef unsigned long                   can_board_type_t;
typedef unsigned char                   can_status_reg_t;
typedef unsigned char                   can_op_mode_t;
#define CANAPI_HANDLE                   (can_handle_t)(-1)
#define CANBRD_AVAILABLE                CANBRD_PRESENT    
#define CANBRD_UNAVAILABLE              CANBRD_NOT_PRESENT
#define CANBRD_INTESTABLE               CANBRD_NOT_TESTABLE
#define CANEXIT_ALL                     CANKILL_ALL
#define CAN_MAX_EXT_ID                  CAN_MAX_XTD_ID
/** @} */

/*  -----------  types  --------------------------------------------------
 */

#ifndef _CAN_BOARD
#define _CAN_BOARD
 /** @brief      CAN Interface Board:
  */
 typedef struct _can_board_t {
   unsigned long type;                  /**< board type */
   char* name;                          /**< board name */
 } can_board_t;
#endif
#ifndef _CAN_STATUS
#define _CAN_STATUS
 /** @brief      CAN Status-register:
  */
 typedef union _can_status_t {
   unsigned char byte;                  /**< byte access */
   struct {                             /*   bit access: */
     unsigned char queue_overrun : 1;   /**<   event-queue overrun */
     unsigned char message_lost : 1;    /**<   message lost */
     unsigned char receiver_empty : 1;  /**<   receiver empty */
     unsigned char transmitter_busy : 1;/**<   transmitter busy */
     unsigned char bus_error : 1;       /**<   bus error (LEC) */
     unsigned char warning_level : 1;   /**<   error warning status */
     unsigned char bus_off : 1;         /**<   bus off status */
     unsigned char can_stopped : 1;     /**<   CAN controller stopped */
   } b;
 } can_status_t;
#endif
/** @brief      CAN Operation Mode:
 */
 typedef union _can_mode_t {
  unsigned char byte;                   /**< byte access */
  struct {                              /*   bit access: */
    unsigned char mon : 1;              /**<   monitor mode enabled */
    unsigned char err : 1;              /**<   error frames enabled */
    unsigned char nrtr : 1;             /**<   remote frames disabled */
    unsigned char nxtd : 1;             /**<   extended format disabled */
    unsigned char shrd : 1;             /**<   shared access enabled */
    unsigned char niso : 1;             /**<   Non-ISO CAN FD enabled */
    unsigned char brse : 1;             /**<   bit-rate switch enabled */
    unsigned char fdoe : 1;             /**<   CAN FD operation enabled */
  } b;
} can_mode_t;

/** @brief      CAN Bit-rate Settings (nominal and data):
 */
typedef union _can_bitrate_t {
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

/** @brief      CAN Transmission Rate (nominal and data):
 */
typedef struct _can_speed_t {
    struct {                            /*   nominal bus speed: */
#ifndef CAN_20_ONLY
        int fdoe;                       /**<   CAN FD operation enabled */
        float speed;                    /**<   bus speed in [Bit/s] */
        float samplepoint;              /**<   sample point in [percent] */
#endif
    }   nominal;                        /**< nominal bus speed */
#ifndef CAN_20_ONLY
    struct {                            /*   data bus speed: */
        int brse;                       /**<   bit-rate switch enabled */
        float speed;                    /**<   bus speed in [Bit/s] */
        float samplepoint;              /**<   sample point in [percent] */
    }   data;                           /**< data bus speed */
#endif
}   can_speed_t;

/** @brief      CAN Time-stamp:
 */
typedef struct _can_timestamp_t {
    long sec;                           /**<   seconds */
    long usec;                          /**<   microseconds */
}   can_timestamp_t;

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
    can_timestamp_t timestamp;          /**< time-stamp { sec, usec } */
}   can_msg_t;

/*  -----------  variables  ----------------------------------------------
 */

#ifndef _CANAPI_EXPORTS
 CANAPI can_board_t can_board[];        /**< list of CAN interface boards */
#endif


/*  -----------  prototypes  ---------------------------------------------
 */

/** @brief       tests if the CAN interface (hardware and driver) given by
 *               the argument 'board' is present, and if the requested
 *               operation mode is supported by the CAN controller board.
 *
 *  @note        When a requested operation mode is not supported by the
 *               CAN controller, error CANERR_ILLPARA will be returned.
 *
 *  @param[in]   board   - type of the CAN controller board
 *  @param[in]   mode    - operation mode to be checked
 *  @param[in]   param   - pointer to board-specific parameters
 *  @param[out]  result  - result of the board test:
 *                             < 0 - board is not present,
 *                             = 0 - board is present,
 *                             > 0 - board is present, but in use
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_ILLPARA   - illegal parameter value
 *  @retval      CANERR_NOTSUPP   - function not supported
 *  @retval      others           - vendor-specific
 */
CANAPI int can_test(int board, unsigned char mode, const void *param, int *result);


/** @brief       initializes the CAN interface (hardware and driver) by loading
 *               and starting the appropriate DLL for the specified CAN controller
 *               board given by the argument 'board'. 
 *               The operation state of the CAN controller is set to 'stopped';
 *               no communication is possible in this state.
 *
 *  @param[in]   board   - type of the CAN controller board
 *  @param[in]   mode    - operation mode of the CAN controller
 *  @param[in]   param   - pointer to board-specific parameters
 *
 *  @returns     handle of the CAN interface if successful, 
 *               or a negative value on error.
 *
 *  @retval      CANERR_YETINIT   - interface already in use
 *  @retval      CANERR_HANDLE    - no free handle found
 *  @retval      others           - vendor-specific
 */
CANAPI int can_init(int board, unsigned char mode, const void *param);


/** @brief       stops any operation of the CAN interface and sets the operation
 *               state of the CAN controller to 'offline'.
 *
 *  @note        The handle is invalid after this operation and could be assigned
 *               to a different CAN controller board in a multy-board application.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      others           - vendor-specific
 */
CANAPI int can_exit(int handle);


/** @brief       initializes the operation mode and the bit-rate settings of the
 *               CAN interface and sets the operation state of the CAN controller
 *               to 'running'.
 *
 *  @note        All statistical counters (tx/rx/err) will be reset by this.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[in]   bitrate - bit-rate as btr register or baud rate index
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_BAUDRATE  - illegal bit-rate settings
 *  @retval      CANERR_ONLINE    - interface already started
 *  @retval      others           - vendor-specific
 */
CANAPI int can_start(int handle, const can_bitrate_t *bitrate);


/** @brief       stops any operation of the CAN interface and sets the operation
 *               state of the CAN controller to 'stopped'; no communication is
 *               possible in this state.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_OFFLINE   - interface already stopped
 *  @retval      others           - vendor-specific
 */
CANAPI int can_reset(int handle);


/** @brief       transmits a message over the CAN bus. The CAN controller must be
 *               in operation state 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[in]   msg     - pointer to the message to send
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_ILLPARA   - illegal data length code
 *  @retval      CANERR_OFFLINE   - interface not started
 *  @retval      CANERR_TX_BUSY   - transmitter busy
 *  @retval      others           - vendor-specific
 */
CANAPI int can_write(int handle, const can_msg_t *msg);


/** @brief       read one message from the message queue of the CAN interface, if
 *               any message was received. The CAN controller must be in operation
 *               state 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[out]  msg     - pointer to a message buffer
 *  @param[in]   timeout - time to wait for the reception of a message:
 *                              0 means the function returns immediately,
 *                              65535 means blocking read, and any other
 *                              value means the time to wait im milliseconds
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_OFFLINE   - interface not started
 *  @retval      CANERR_RX_EMPTY  - message queue empty
 *  @retval      CANERR_ERR_FRAME - error frame received
 *  @retval      others           - vendor-specific
 */
CANAPI int can_read(int handle, can_msg_t *msg, unsigned short timeout);


#if defined (_WIN32) || defined(_WIN64)
/** @brief       signals a waiting event object of the CAN interface. This can
 *               be used to terminate a blocking read operation in progress
 *               (e.g. by means of a Ctrl-C handler or similar).
 *
 *  @remark      This driver DLL uses an event object to realize blocking
 *               read by a call to WaitForSingleObject, but this event object
 *               is not terminated by Ctrl-C (SIGINT).
 *
 *  @note        SIGINT is not supported for any Win32 application. [MSVC Docs]
 *
 *  @param[in]   handle  - handle of the CAN interface, or (-1) to signal all
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NOTSUPP   - function not supported
 *  @retval      others           - vendor-specific
 */
CANAPI int can_kill(int handle);
#endif


/** @brief       retrieves the status register of the CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[out]  status  - 8-bit status register.
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      others           - vendor-specific
 */
CANAPI int can_status(int handle, unsigned char *status);


/** @brief       retrieves the bus-load (in percent) of the CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[out]  load    - bus-load in [percent]
 *  @param[out]  status  - 8-bit status register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      others           - vendor-specific
 */
CANAPI int can_busload(int handle, unsigned char *load, unsigned char *status);


/** @brief       retrieves the bit-rate setting of the CAN interface. The
 *               CAN controller must be in operation state 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[out]  bitrate - bit-rate setting
 *  @param[out]  speed   - transmission rate
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_OFFLINE   - interface not started
 *  @retval      CANERR_BAUDRATE  - invalid bit-rate settings
 *  @retval      CANERR_NOTSUPP   - function not supported
 *  @retval      others           - vendor-specific
 */
CANAPI int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed);


/** @brief       retrieves or modifies a property value of the CAN interface.
 *
 *  @note        To read or to write a property value of the CAN interface DLL,
 *               -1 can be given as handle.
 *
 *  @param[in]   handle   - handle of the CAN interface, or (-1)
 *  @param[in]   param    - property id to be read or to be written
 *  @param[out]  value    - pointer to a buffer for the value to be read
 *  @param[in]   value    - pointer to a buffer with the value to be written
 *  @param[in]   nbytes   - size of the given buffer in bytes
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - interface not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_ILLPARA   - illegal parameter, value or nbytes
 *  @retval      CANERR_...       - tbd.
 *  @retval      CANERR_NOTSUPP   - property or function not supported
 *  @retval      others           - vendor-specific
 */
CANAPI int can_property(int handle, int param, void *value, int nbytes);


/** @brief       retrieves the hardware version of the CAN controller
 *               board as a zero-terminated string.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char *can_hardware(int handle);


/** @brief       retrieves the firmware version of the CAN controller
 *               board as a zero-terminated string.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char *can_software(int handle);


/** @brief       retrieves version information of the CAN interface DLL
 *               as a zero-terminated string.
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char* can_version();


#endif /* CAN_API_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
