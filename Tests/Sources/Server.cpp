//  SPDX-License-Identifier: BSD-2-Clause OR GPL-2.0-or-later
//
//  CAN Interface API, Version 3 (Testing)
//
//  Copyright (c) 2004-2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//  All rights reserved.
//
//  This file is part of CAN API V3.
//
//  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License
//  and under the GNU General Public License v2.0 (or any later version).
//  You can choose between one of them if you use this file.
//
//  (1) BSD 2-Clause "Simplified" License
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  CAN API V3 IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF CAN API V3, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  (2) GNU General Public License v2.0 or later
//
//  CAN API V3 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  CAN API V3 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with CAN API V3; if not, see <https://www.gnu.org/licenses/>.
//
#include "pch.h"
#include "Server.h"
#include "crc_j1850.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#include <pthread.h>
#else
#include <windows.h>
#endif
#include <assert.h>

#if (OPTION_CANTCP_ENABLED != 0)
CCanServer g_CanServer = CCanServer();  // global access to CAN server
#endif

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex for critical sections
static CCanDevice *canDevice = NULL;  // to be used in the C callback function

static int EventHandler(const void *data, size_t size, void *param) {
    CANTCP_Message_t* ipc_msg = (CANTCP_Message_t*)data;
    CANAPI_Message_t can_msg = CANAPI_Message_t();
    int retVal = CCanApi::FatalError;
    (void)param;

    /* sanity check */
    if (!data) {
        return CCanApi::NullPointer;
    }
    if (size != sizeof(CANTCP_Message_t)) {
        return CCanApi::IllegalParameter;
    }
    if (crc_j1850_calc(data, sizeof(CANTCP_Message_t) - 1U, NULL) != ipc_msg->checksum) {
        return (int)(-80);  // TODO: define error code
    }
    if (ipc_msg->ctrlchar != CANTCP_ETX_CHAR) {
        return (int)(-81);  // TODO: define error code
    }
    /* convert the message from network to host byte order */
    CANTCP_MSG_NTOH(*ipc_msg);
    /* transmit the message on the CAN bus */
    can_msg.id = ipc_msg->id;
    can_msg.xtd = (ipc_msg->flags & CANTCP_XTD_MASK) ? 1 : 0;
    can_msg.rtr = (ipc_msg->flags & CANTCP_RTR_MASK) ? 1 : 0;
    can_msg.fdf = (ipc_msg->flags & CANTCP_FDF_MASK) ? 1 : 0;
    can_msg.brs = (ipc_msg->flags & CANTCP_BRS_MASK) ? 1 : 0;
    can_msg.esi = (ipc_msg->flags & CANTCP_ESI_MASK) ? 1 : 0;
    can_msg.sts = (ipc_msg->flags & CANTCP_STS_MASK) ? 1 : 0;
    can_msg.dlc = CCanApi::Len2Dlc(ipc_msg->length);
    for (int i = 0; (i < CANFD_MAX_LEN) && (i < CANTCP_MAX_LEN); i++) {
        can_msg.data[i] = ipc_msg->data[i];
    }
    /* make it so! */
    assert(pthread_mutex_lock(&mutex) == 0);
    if (canDevice) {
        do {
            retVal = canDevice->WriteMessage(can_msg);
            if (retVal == CCanApi::TransmitterBusy)
                PCBUSB_QXMT_DELAY();
        } while (retVal == CCanApi::TransmitterBusy);
    } else {
        retVal = CCanApi::NotSupported;
    }
    assert(pthread_mutex_unlock(&mutex) == 0);
    /* who cares about? */
    return retVal;
}

CCanServer::CCanServer() : CCanTcpServer() {
    assert(pthread_mutex_init(&mutex, NULL) == 0);
    canDevice = NULL;
}

CCanServer::~CCanServer() {
    if (IsRunning()) {
        Stop();
    }
    assert(pthread_mutex_destroy(&mutex) == 0);
}

bool CCanServer::AttachDevice(CCanDevice *device) {
    assert(pthread_mutex_lock(&mutex) == 0);
    canDevice = device;
    assert(pthread_mutex_unlock(&mutex) == 0);
    return true;
}

bool CCanServer::DetachDevice() {
    assert(pthread_mutex_lock(&mutex) == 0);
    canDevice = NULL;
    assert(pthread_mutex_unlock(&mutex) == 0);
    return true;
}

CANAPI_Return_t CCanServer::StartServer(const char *service) {
    if (!SetCallback(EventHandler)) return CCanApi::AlreadyInitialized;
    return Start(service);
}

CANAPI_Return_t CCanServer::StopServer() {
    CANAPI_Return_t retVal = Stop();
    SetCallback(NULL);
    DetachDevice();
    return retVal;
}

void CCanServer::ShowServerPort(const char* prefix) {
    if (IsRunning()) {
        if (prefix)
            std::cout << prefix << ' ';
        std::cout << "RocketCAN server listening on port " << GetService();
        std::cout << " with data size " << GetFrameSize() << " 🚀" << std::endl;
    }
}

// $Id: Server.cpp 1486 2025-03-02 15:50:07Z quaoar $  Copyright (c) UV Software, Berlin.
