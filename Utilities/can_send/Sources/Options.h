//  SPDX-License-Identifier: GPL-2.0-or-later
//
//  CAN Sender for generic Interfaces (CAN API V3)
//
//  Copyright (c) 2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, see <https://www.gnu.org/licenses/>.
//
#ifndef CAN_SEND_OPTIONS_H_INCLUDED
#define CAN_SEND_OPTIONS_H_INCLUDED

#include "Driver.h"
#include "Version.h"

#include <stdio.h>
#include <stdint.h>

#define CAN_SEND_APPLICATION "CAN Sender for " SENDER_INTERFACE ", Version " VERSION_STRING
#define CAN_SEND_COPYRIGHT   "Copyright (c) " SENDER_COPYRIGHT
#if !defined(_WIN32) && !defined(_WIN64)
#define CAN_SEND_WARRANTY    "This program comes with ABSOLUTELY NO WARRANTY!\n\n" \
                             "This is free software, and you are welcome to redistribute it\n" \
                             "under certain conditions; type `" CAN_SEND_PROGRAM " --version' for details.";
#else
#define CAN_SEND_WARRANTY    "This program comes with ABSOLUTELY NO WARRANTY!\n\n" \
                             "This is free software, and you are welcome to redistribute it\n" \
                             "under certain conditions; type '" CAN_SEND_PROGRAM " /VERSION' for details.";
#endif
#define CAN_SEND_LICENSE     "This program is free software; you can redistribute it and/or modify\n" \
                             "it under the terms of the GNU General Public License as published by\n" \
                             "the Free Software Foundation; either version 2 of the License, or\n" \
                             "(at your option) any later version.\n\n" \
                             "This program is distributed in the hope that it will be useful,\n" \
                             "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
                             "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
                             "GNU General Public License for more details.\n\n" \
                             "You should have received a copy of the GNU General Public License along\n" \
                             "with this program; if not, see <https://www.gnu.org/licenses/>."
#define CAN_SEND_PROGRAM     "can_send"

struct SOptions {
    // attributes
    char* m_szBasename;
    char* m_szInterface;
#if (OPTION_CANAPI_LIBRARY != 0)
    char* m_szSearchPath;
#else
    char* m_szJsonFilename;
#endif
#if (SERIAL_CAN_SUPPORTED != 0)
    uint8_t m_u8Protocol;
#endif
    CANAPI_OpMode_t m_OpMode;
    CANAPI_Bitrate_t m_Bitrate;
    CANAPI_BusSpeed_t m_BusSpeed;
    bool m_bHasDataPhase;
    bool m_bHasNoSamp;
    struct {
        uint32_t m_u32Code;
        uint32_t m_u32Mask;
    } m_StdFilter, m_XtdFilter;
#if (CAN_TRACE_SUPPORTED != 0)
    enum ETraceMode {
        eTraceOff,
        eTraceBinary,
        eTraceLogger,
        eTraceVendor
    } m_eTraceMode;
#endif
    bool m_fListBitrates;
    bool m_fListBoards;
    bool m_fTestBoards;
    bool m_fVerbose;
    bool m_fExit;
    // initializer
    SOptions();
    // operations
    int ScanCommanline(int argc, const char* argv[], FILE* err = stderr, FILE* out = stdout);
    void ShowGreetings(FILE* stream);
    void ShowFarewell(FILE* stream);
    void ShowVersion(FILE* stream);
    void ShowHelp(FILE* stream);
    void ShowUsage(FILE* stream, bool args = false);
};

#endif  // CAN_SEND_OPTIONS_H_INCLUDED
