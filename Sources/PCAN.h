//
//  CAN Interface API, Version 3 (for PEAK PCAN-Basic Interfaces)
//
//  Copyright (C) 2010-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This file is part of PCANBasic-Wrapper.
//
//  PCANBasic-Wrapper is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  PCANBasic-Wrapper is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with PCANBasic-Wrapper.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef PCAN_H_INCLUDED
#define PCAN_H_INCLUDED

#include "CANAPI.h"

/// \name   PCAN
/// \brief  PCAN dynamic library
/// \{
#define PCAN_LIBRARY_ID  CANLIB_PCANBASIC
#define PCAN_LIBRARY_NAME  CANDLL_PCANBASIC
#define PCAN_LIBRARY_VENDOR  "UV Software, Berlin"
#define PCAN_LIBRARY_LICENSE  "GNU Lesser General Public License, Version 3"
#define PCAN_LIBRARY_COPYRIGHT  "Copyright (C) 2010-2021  Uwe Vogt, UV Software, Berlin"
#define PCAN_LIBRARY_HAZARD_NOTE  "If you connect your CAN device to a real CAN network when using this library,\n" \
                                  "you might damage your application."
/// \}


/// \name   PCAN API
/// \brief  CAN API V3 driver for PEAK PCAN-Basic interfaces
/// \note   See CCANAPI for a description of the overridden methods
/// \{
class CPCAN : public CCANAPI {
private:
    CANAPI_OpMode_t m_OpMode;  ///< CAN operation mode
    CANAPI_Bitrate_t m_Bitrate;  ///< CAN bitrate settings
    struct {
        uint64_t u64TxMessages;  ///< number of transmitted CAN messages
        uint64_t u64RxMessages;  ///< number of received CAN messages
        uint64_t u64ErrorFrames;  ///< number of received status messages
    } m_Counter;
    // opaque data type
    struct SPCAN;  ///< C++ forward declaration
    SPCAN *m_pPCAN;  ///< PCANBasic interface
public:
    // constructor / destructor
    CPCAN();
    ~CPCAN();
    // CPCAN-specific error codes (CAN API V3 extension)
    enum EErrorCodes {
        // note: range 0...-99 is reserved by CAN API V3
        GeneralError = VendorSpecific, ///< mapped PCAN-Basic error codes
        RegisterTestFailed     = -201, ///< PCAN_ERROR_REGTEST: test of the CAN controller hardware registers failed (no hardware found)
        DriverNotLoaded        = -202, ///< PCAN_ERROR_NODRIVER: driver not loaded
        HardwareAlreadyInUse   = -203, ///< PCAN_ERROR_HWINUSE: hardware is in use by another Net
        ClientAlreadyConnected = -204, ///< PCAN_ERROR_NETINUSE: a client is already connected to the Net
        HardwareHandleIsWrong  = -220, ///< PCAN_ERROR_ILLHW: hardware handle is wrong
        NetworkHandleIsWrong   = -224, ///< PCAN_ERROR_ILLNET: net handle is wrong
        ClientHandleIsWrong    = -228, ///< PCAN_ERROR_ILLCLIENT: client handle is wrong
        ResourceNotCreated     = -232, ///< PCAN_ERROR_RESOURCE: resource (FIFO, Client, timeout) cannot be created
        InvalidParameterType   = -264, ///< PCAN_ERROR_ILLPARAMTYPE: invalid parameter
        InvalidParameterValue  = -265, ///< PCAN_ERROR_ILLPARAMVAL: invalid parameter value
        InvalidData            = -267, ///< PCAN_ERROR_ILLDATA: invalid data, function, or action
        InvalidOperation       = -268, ///< PCAN_ERROR_ILLOPERATION: invalid operation
        Caution                = -289, ///< PCAN_ERROR_CAUTION: an operation was successfully carried out, however, irregularities were registered
        UnkownError            = -299  ///< PCAN_ERROR_UNKNOWN: unknown error
    };
    // CCANAPI overrides
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, EChannelState &state);

    CANAPI_Return_t InitializeChannel(int32_t channel, can_mode_t opMode, const void *param = NULL);
    CANAPI_Return_t TeardownChannel();
    CANAPI_Return_t SignalChannel();

    CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate);
    CANAPI_Return_t ResetController();

    CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U);
    CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANREAD_INFINITE);

    CANAPI_Return_t GetStatus(CANAPI_Status_t &status);
    CANAPI_Return_t GetBusLoad(uint8_t &load);

    CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed);

    CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbytes);
    CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbytes);

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)

    static CANAPI_Return_t MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length);
    static CANAPI_Return_t MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed);
private:
    CANAPI_Return_t MapBitrate2Sja1000(CANAPI_Bitrate_t bitrate, uint16_t &btr0btr1);
    CANAPI_Return_t MapSja10002Bitrate(uint16_t btr0btr1, CANAPI_Bitrate_t &bitrate);
public:
    static uint8_t DLc2Len(uint8_t dlc);
    static uint8_t Len2Dlc(uint8_t len);
};
/// \}

/// \name   PCAN Property IDs
/// \brief  Properties that can be read (or written)
/// \{
#define PCAN_PROPERTY_CANAPI              (CANPROP_GET_SPEC)
#define PCAN_PROPERTY_VERSION             (CANPROP_GET_VERSION)
#define PCAN_PROPERTY_PATCH_NO            (CANPROP_GET_PATCH_NO)
#define PCAN_PROPERTY_BUILD_NO            (CANPROP_GET_BUILD_NO)
#define PCAN_PROPERTY_LIBRARY_ID          (CANPROP_GET_LIBRARY_ID)
#define PCAN_PROPERTY_LIBRARY_NAME        (CANPROP_GET_LIBRARY_DLLNAME)
#define PCAN_PROPERTY_LIBRARY_VENDOR      (CANPROP_GET_LIBRARY_VENDOR)
#define PCAN_PROPERTY_DEVICE_TYPE         (CANPROP_GET_DEVICE_TYPE)
#define PCAN_PROPERTY_DEVICE_NAME         (CANPROP_GET_DEVICE_NAME)
#define PCAN_PROPERTY_DEVICE_VENDOR       (CANPROP_GET_DEVICE_VENDOR)
#define PCAN_PROPERTY_DEVICE_DLLNAME      (CANPROP_GET_DEVICE_DLLNAME)
#define PCAN_PROPERTY_OP_CAPABILITY       (CANPROP_GET_OP_CAPABILITY)
#define PCAN_PROPERTY_OP_MODE             (CANPROP_GET_OP_MODE)
#define PCAN_PROPERTY_BITRATE             (CANPROP_GET_BITRATE)
#define PCAN_PROPERTY_SPEED               (CANPROP_GET_SPEED)
#define PCAN_PROPERTY_STATUS              (CANPROP_GET_STATUS)
#define PCAN_PROPERTY_BUSLOAD             (CANPROP_GET_BUSLOAD)
#define PCAN_PROPERTY_TX_COUNTER          (CANPROP_GET_TX_COUNTER)
#define PCAN_PROPERTY_RX_COUNTER          (CANPROP_GET_RX_COUNTER)
#define PCAN_PROPERTY_ERR_COUNTER         (CANPROP_GET_ERR_COUNTER)
#define PCAN_PROPERTY_DEVICE_ID           (CANPROP_GET_VENDOR_PROP + 0x01U)
#define PCAN_PROPERTY_API_VERSION         (CANPROP_GET_VENDOR_PROP + 0x05U)
#define PCAN_PROPERTY_CHANNEL_VERSION     (CANPROP_GET_VENDOR_PROP + 0x06U)
#define PCAN_PROPERTY_HARDWARE_NAME       (CANPROP_GET_VENDOR_PROP + 0x0EU)
//#define PCAN_PROPERTY_BOOTLOADER_VERSION  (CANPROP_GET_VENDOR_PROP + 0x??U)
//#define PCAN_PROPERTY_SERIAL_NUMBER       (CANPROP_GET_VENDOR_PROP + 0x??U)
//#define PCAN_PROPERTY_VID_PID             (CANPROP_GET_VENDOR_PROP + 0x??U)
//#define PCAN_PROPERTY_CLOCK_DOMAIN        (CANPROP_GET_VENDOR_PROP + 0x??U)
// TODO: insert coin here
/// \}
#endif // PCAN_H_INCLUDED
