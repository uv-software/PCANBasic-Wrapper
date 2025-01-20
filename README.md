### CAN API V3 Wrapper Library for PEAK-System PCAN&reg; Interfaces (Windows&reg;)

_Copyright &copy; 2005-2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com)_

![MSBuild](https://github.com/uv-software/PCANBasic-Wrapper/actions/workflows/msbuild.yml/badge.svg)

# CAN API V3 for PCAN Interfaces

CAN API V3 is a wrapper specification to have a uniform CAN Interface API for various CAN interfaces from different vendors running under multiple operating systems.

## PCANBasic-Wrapper

This repo contains the source code for a CAN API V3 compatible wrapper library under Windows for PCAN Interfaces from PEAK-System Technik GmbH.
The wrapper library is build upon Peak´s PCANBasic DLL.

### CAN Interface API, Version 3

```C++
/// \name   PeakCAN API
/// \brief  CAN API V3 Wrapper for PEAK-System PCAN Interfaces
/// \note   See CCanApi for a description of the overridden methods
/// \{
class CPeakCAN : public CCanApi {
public:
    // constructor / destructor
    CPeakCAN();
    ~CPeakCAN();

    // CCanApi overrides
    static bool GetFirstChannel(SChannelInfo &info, void *param = NULL);
    static bool GetNextChannel(SChannelInfo &info, void *param = NULL);

    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, EChannelState &state);

    CANAPI_Return_t InitializeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param = NULL);
    CANAPI_Return_t TeardownChannel();
    CANAPI_Return_t SignalChannel();

    CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate);
    CANAPI_Return_t ResetController();

    CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U);
    CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANWAIT_INFINITE);

    CANAPI_Return_t GetStatus(CANAPI_Status_t &status);
    CANAPI_Return_t GetBusLoad(uint8_t &load);

    CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed);

    CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbyte);
    CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbyte);

    CANAPI_Return_t SetFilter11Bit(uint32_t code, uint32_t mask);
    CANAPI_Return_t GetFilter11Bit(uint32_t &code, uint32_t &mask);
    CANAPI_Return_t SetFilter29Bit(uint32_t code, uint32_t mask);
    CANAPI_Return_t GetFilter29Bit(uint32_t &code, uint32_t &mask);
    CANAPI_Return_t ResetFilters();

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)

    static CANAPI_Return_t MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length);
    static CANAPI_Return_t MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed);

    static uint8_t Dlc2Len(uint8_t dlc) { return CCanApi::Dlc2Len(dlc); }
    static uint8_t Len2Dlc(uint8_t len) { return CCanApi::Len2Dlc(len); }
};
/// \}
```
See header file `PeakCAN.h` for a description of the provided methods.

## Build Targets

Important note: _To build any of the following build targets run the script_ `build_no.bat` _to generate a pseudo build number._
```
C:\Users\haumea>cd C:\Projects\CAN\Drivers\PeakCAN
C:\Projects\CAN\Drivers\PeakCAN>build_no.bat
```
Repeat this step after each `git commit`, `git pull`, `git clone`, etc.

To build all 32-bit targets (x86) run the script `x86_build.bat`.
```
C:\Users\haumea>cd C:\Projects\CAN\Drivers\PeakCAN
C:\Projects\CAN\Drivers\PeakCAN>x86_build.bat
```

To build all 64-bit targets (x64) run the script `x64_build.bat`.
```
C:\Users\haumea>cd C:\Projects\CAN\Drivers\PeakCAN
C:\Projects\CAN\Drivers\PeakCAN>x64_build.bat
```
(The version number of the libraries can be adapted by editing the `.rc` files in the corresponding subfolders. Don´t forget to set the version number also in the header file `Version.h`.)

#### uvPeakCAN (DLL)

___uvPeakCAN___ is a dynamic link library with a CAN API V3 compatible application programming interface for use in __C++__ applications.
See header file `PeakCAN.h` for a description of all class members.

#### u3canpcb (DLL)

___u3canpcb___ is a dynamic link library with a CAN API V3 compatible application programming interface for use in __C__ applications.
See header file `can_api.h` for a description of all API functions.

#### can_moni (CLI)

`can_moni` is a command line tool to view incoming CAN messages.
I hate this messing around with binary masks for identifier filtering.
So I wrote this little program to have an exclude list for single identifiers or identifier ranges (see program option `/EXCLUDE` or just `/X`). Precede the list with a `~` and you get an include list.

Type `can_moni /?` to display all program options.

#### can_test (CLI)

`can_test` is a command line tool to test CAN communication.
Originally developed for electronic environmental tests on an embedded Linux system with SocketCAN, I´m using it for many years as a traffic generator for CAN stress-tests.

Type `can_test /?` to display all program options.

### Target Platform

- Windows 10 & 11 (x64 operating systems)

### Development Environment

- Microsoft Visual Studio Community 2022 (Version 17.12.4)

### Required PCANBasic DLL

- Version 4.5 or later _(Latest is Greatest!)_

### Tested CAN Hardware

- PCAN-USB - single channel, CAN 2.0 (Peak´s item no.: IPEH-002021, IPEH-002021)
- PCAN-USB FD - single channel, CAN 2.0 and CAN FD (Peak´s item no.: IPEH-004022)
- PCAN-USB Pro FD - dual channel, CAN 2.0 and CAN FD (Peak´s item no.: IPEH-004061)

## Known Bugs and Caveats

For a list of known bugs and caveats see tab [Issues](https://github.com/uv-software/PCANBasic-Wrapper/issues) in the GitHub repo.

## This and That

### PCANBasic DLL for Windows&reg;

The PCANBasic DLL can be downloaded from [PEAK-System](https://www.peak-system.com/) website. \
Please note the copyright and license agreements.

### Wrapper Library for macOS&reg; and Linux&reg;

A version for macOS and Linux can be downloaded from / cloned at [GitHub](https://github.com/mac-can/PCBUSB-Wrapper).

### CAN API V3 Reference

A generic documentation of the CAN API V3 application programming interface can be found [here](https://uv-software.github.io/CANAPI-Docs/#/).

### Dual-License

Except where otherwise noted, this work is dual-licensed under the terms of the BSD 2-Clause "Simplified" License
and under the terms of the GNU General Public License v2.0 (or any later version).
You can choose between one of them if you use these portions of this work in whole or in part.

### Trademarks

Windows is a registered trademark of Microsoft Corporation in the United States and/or other countries. \
PCAN is a registered trademark of PEAK-System Technik GmbH, Darmstadt, Germany. \
Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries. \
Linux is a registered trademark of Linus Torvalds. \
All other company, product and service names mentioned herein may be trademarks, registered trademarks, or service marks of their respective owners.

### Hazard Note

_If you connect your CAN device to a real CAN network when using this library, you might damage your application._

### Contact

E-Mail: mailto://info@uv-software.com \
Internet: https://www.uv-software.com
