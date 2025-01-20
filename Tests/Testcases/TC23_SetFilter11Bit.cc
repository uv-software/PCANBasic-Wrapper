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

//  @note: This test suite tests the following methods:
//  @      - CCanApi::SetFilter11Bit()  [TC23.*]
//  @      - CCanApi::GetFilter11Bit()  [TC24.*]
//  @
#ifndef FEATURE_FILTERING
#define FEATURE_FILTERING  FEATURE_UNSUPPORTED
#ifdef _MSC_VER
#pragma message ( "FEATURE_FILTERING not set, default = FEATURE_UNSUPPORTED" )
#else
#warning FEATURE_FILTERING not set, default = FEATURE_UNSUPPORTED
#endif
#endif
#if (FEATURE_FILTERING != FEATURE_UNSUPPORTED)

class SetFilter11Bit : public testing::Test {
    virtual void SetUp() {}
    virtual void TearDown() {}
protected:
    void CheckFilter11Bit(CCanDevice& dut1, uint32_t accCode, uint32_t accMask) {
        CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
        CANAPI_Message_t trmMsg = {};
        CANAPI_Message_t rcvMsg = {};
        CANAPI_Return_t retVal;
        // CAN message
        trmMsg.id = 0U;
        trmMsg.xtd = 0;
        trmMsg.rtr = 0;
        trmMsg.sts = 0;
#if (OPTION_CAN_2_0_ONLY != 0)
        trmMsg.dlc = 0U; // CAN_MAX_DLC;
        memset(trmMsg.data, 0, CAN_MAX_LEN);
#else
        trmMsg.fdf = g_Options.GetOpMode(DUT1).fdoe ? 1 : 0;
        trmMsg.brs = g_Options.GetOpMode(DUT1).brse ? 1 : 0;
        trmMsg.esi = 0;
        trmMsg.dlc = 0U; // g_Options.GetOpMode(DUT1).fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
        memset(trmMsg.data, 0, CANFD_MAX_LEN);
#endif
        // initialize DUT2 with configured settings
        retVal = dut2.InitializeChannel();
        ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut2.InitializeChannel() failed with error code " << retVal;
        // start DUT2 with configured bit-rate settings
        retVal = dut2.StartController();
        EXPECT_EQ(CCanApi::NoError, retVal);
        // DUT2 send 2048 standard messages
        int n = 0, frames = 2048;
        CProgress progress = CProgress(frames);
        for (int i = 0; i < frames; i++) {
            trmMsg.id = (uint32_t)i;
            // send one message (retry if busy)
            do {
                retVal = dut2.WriteMessage(trmMsg, DEVICE_SEND_TIMEOUT);
                if (retVal == CCanApi::TransmitterBusy)
                    PCBUSB_QXMT_DELAY();
            } while (retVal == CCanApi::TransmitterBusy);
            // on error abort
            if (retVal != CCanApi::NoError) {
                EXPECT_EQ(retVal, CCanApi::NoError);
                n = 0;
                break;
            }
            PCBUSB_QXMT_DELAY();  // TODO: why here?
            // update number of transmitted CAN messages
            progress.Update((int)(i + 1), (int)0);
            // update number of accepted CAN messages
            if (dut1.IsCanIdAccepted(trmMsg.id, accCode, accMask)) {
                n++;
            }
        }
        // DUT1 process all messages with acceptance filter
        uint64_t timeout = (((uint64_t)dut1.TransmissionTime(dut1.GetBitrate(), (n + DEVICE_LOOP_EXTRA))
                         *   (uint64_t)DEVICE_LOOP_FACTOR) / (uint64_t)DEVICE_LOOP_DIVISOR);  // bit-rate dependent timeout
        CTimer timer = CTimer(timeout);
        int j = 0;
        while ((j < n) && !timer.Timeout()) {
            // read one message from receiver's queue, if any
            retVal = dut1.ReadMessage(rcvMsg, DEVICE_READ_TIMEOUT);
            if (retVal == CCanApi::NoError) {
                // ignore status messages/error frames
                if (!rcvMsg.sts) {
                    // update number of received CAN messages
                    progress.Update((int)frames, (int)(j + 1));
                    // check if received message is accepted by acceptance filter
                    if (!dut1.IsCanIdAccepted(rcvMsg.id, accCode, accMask)) {
                        EXPECT_TRUE(dut1.IsCanIdAccepted(rcvMsg.id, accCode, accMask));
                        break;
                    }
                    j++;
                }
            }
            // on error abort (if receiver is empty, continue)
            else if (retVal != CCanApi::ReceiverEmpty) {
                EXPECT_EQ(retVal, CCanApi::ReceiverEmpty);
                break;
            }
        }
        progress.Clear();
        // tear down DUT2
        retVal = dut2.TeardownChannel();
        EXPECT_EQ(CCanApi::NoError, retVal);
    }
};

// @gtest TC23/TC24.0: Set CAN acceptance filter for 11-bit identifier (sunnyday scenario)
//
// @expected: CANERR_NOERROR
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(SunnydayScenario, GTEST_SUNNYDAY)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CCanApi::EChannelState state;
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    uint32_t codeSet, maskSet;
    CANAPI_Return_t retVal;
    // @pre:
    // @- probe if DUT1 is present and not occupied
    retVal = dut1.ProbeChannel(state);
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.ProbeChannel() failed with error code " << retVal;
    ASSERT_EQ(CCanApi::ChannelAvailable, state) << "[  ERROR!  ] " << g_Options.GetDeviceName(DUT1) << " is not available";
    // @- probe if DUT2 is present and not occupied
    retVal = dut2.ProbeChannel(state);
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut2.ProbeChannel() failed with error code " << retVal;
    ASSERT_EQ(CCanApi::ChannelAvailable, state) << "[  ERROR!  ] " << g_Options.GetDeviceName(DUT2) << " is not available";
    // @- check if different channels have been selected
    ASSERT_TRUE((g_Options.GetChannelNo(DUT1) != g_Options.GetChannelNo(DUT2)) || \
        (g_Options.GetLibraryId(DUT1) != g_Options.GetLibraryId(DUT2))) << "[  ERROR!  ] same channel selected twice";
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @test:
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @- set 11-bit filter (code 0x000 and mask 0x500)
    codeSet = 0x00000000U; maskSet = 0x00000500U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x500)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
#if (TC23_X_ISSUE_PCBUSB_FILTER_CODE != WORKAROUND_ENABLED)
    EXPECT_EQ(codeSet, codeGet);
#else
    // @  issue(PCBUSB): code is bit-wise ANDed with mask (Linux)
    EXPECT_EQ(codeSet & maskSet, codeGet);
#endif
    EXPECT_EQ(maskGet, maskGet);
    // @- reset acceptance filter
    // @  note: SJA100 has only one filter for 11-bit and 29-bit identifier!
    retVal = dut1.ResetFilters();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @post:
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @end.
}

// @gtest TC23/TC24.1: Set CAN acceptance filter for 11-bit identifier with invalid interface handle(s)
//
// @note: checking channel handles is not possible with the C++ API!

// @gtest TC24.2: Give a NULL pointer as argument for parameter 'code and 'mask'
//
// @note: passing pointers for 'code and 'mask' is not possible with the C++ API!

// @gtest TC23/TC24.3: Set CAN acceptance filter for 11-bit identifier if CAN channel is not initialized
//
// @expected: CANERR_NOTINIT
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(IfChannelNotInitialized, GTEST_ENABLED)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    uint32_t codeSet, maskSet;
    CANAPI_Return_t retVal;
 
    // @test:
    // @- try to get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NotInitialized, retVal);
    // @- try to set 11-bit filter (code 0x000 and mask 0x500)
    codeSet = 0x00000000U; maskSet = 0x00000500U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::NotInitialized, retVal);
    // @post:
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @end.
}

// @gtest TC23/TC24.4: Set CAN acceptance filter for 11-bit identifier if CAN controller is not started
//
// @expected: CANERR_NOERROR
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(IfControllerNotStarted, GTEST_ENABLED)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    uint32_t codeSet, maskSet;
    CANAPI_Return_t retVal;
    // @pre:
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @test:
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @- set 11-bit filter (code 0x000 and mask 0x500)
    codeSet = 0x00000000U; maskSet = 0x00000500U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x500)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
#if (TC23_X_ISSUE_PCBUSB_FILTER_CODE != WORKAROUND_ENABLED)
    EXPECT_EQ(codeSet, codeGet);
#else
    // @  issue(PCBUSB): code is bit-wise ANDed with mask (Linux)
    EXPECT_EQ(codeSet & maskSet, codeGet);
#endif
    EXPECT_EQ(maskGet, maskGet);
    // @- reset acceptance filter
    // @  note: SJA100 has only one filter for 11-bit and 29-bit identifier!
    retVal = dut1.ResetFilters();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @post:
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @end.
}

// @gtest TC23/TC24.5: Set CAN acceptance filter for 11-bit identifier if CAN controller is started
//
// @expected: CANERR_ONLINE
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(IfControllerStarted, GTEST_ENABLED)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    uint32_t codeSet, maskSet;
    CANAPI_Return_t retVal;

    // @pre:
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @test:
    // @- try to set 11-bit filter (code 0x000 and mask 0x500)
    codeSet = 0x00000000U; maskSet = 0x00000500U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::ControllerOnline, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @post:
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @end.
}

// @gtest TC23/TC24.6: Set CAN acceptance filter for 11-bit identifier if CAN controller was previously stopped
//
// @expected: CANERR_NOERROR
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(IfControllerStopped, GTEST_ENABLED)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    uint32_t codeSet, maskSet;
    CANAPI_Return_t retVal;
    // @pre:
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @test:
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @- set 11-bit filter (code 0x000 and mask 0x500)
    codeSet = 0x00000000U; maskSet = 0x00000500U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x500)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
#if (TC23_X_ISSUE_PCBUSB_FILTER_CODE != WORKAROUND_ENABLED)
    EXPECT_EQ(codeSet, codeGet);
#else
    // @  issue(PCBUSB): code is bit-wise ANDed with mask (Linux)
    EXPECT_EQ(codeSet & maskSet, codeGet);
#endif
    EXPECT_EQ(maskGet, maskGet);
    // @- reset acceptance filter
    // @  note: SJA100 has only one filter for 11-bit and 29-bit identifier!
    retVal = dut1.ResetFilters();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @post:
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    /*int32_t*/ frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @end.
}

// @gtest TC23/TC24.7: Set CAN acceptance filter for 11-bit identifier if CAN channel was previously torn down
//
// @expected: CANERR_NOTINIT
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(IfChannelTornDown, GTEST_ENABLED)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    uint32_t codeSet, maskSet;
    CANAPI_Return_t retVal;

    // @pre:
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @test:
    // @- try to get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NotInitialized, retVal);
    // @- try to set 11-bit filter (code 0x000 and mask 0x500)
    codeSet = 0x00000000U; maskSet = 0x00000500U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::NotInitialized, retVal);
    // @end.
}

// @gtest TC23.8: Set CAN acceptance filter for 11-bit identifier with valid values
//
// @expected: CANERR_NOERROR
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(WithValidValues, GTEST_ENABLED)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    const uint32_t codeSet[4] = { 0x000U, 0x0F0U, 0x70FU, 0x7FFU };
    const uint32_t maskSet[4] = { 0x000U, 0x0F0U, 0x70FU, 0x7FFU };
    CANAPI_Return_t retVal;
    // @
    // @note: This test takes quite a long time
    if (g_Options.RunQuick())
        GTEST_SKIP() << "This test takes quite a long time!";
    // @pre:
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @test:
    CCounter counter = CCounter();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            counter.Increment();
            // @-- set 11-bit filter
            retVal = dut1.SetFilter11Bit(codeSet[i], maskSet[j]);
            EXPECT_EQ(CCanApi::NoError, retVal);
            // @-- get 11-bit filter
            codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
            retVal = dut1.GetFilter11Bit(codeGet, maskGet);
            EXPECT_EQ(CCanApi::NoError, retVal);
#if (TC23_X_ISSUE_PCBUSB_FILTER_CODE != WORKAROUND_ENABLED)
            EXPECT_EQ(codeSet[i], codeGet);
#else
            // @   issue(PCBUSB): code is bit-wise ANDed with mask (Linux)
            EXPECT_EQ(codeSet[i] & maskSet[j], codeGet);
#endif
            EXPECT_EQ(maskSet[j], maskGet);
            // @-- start DUT1 with configured bit-rate settings
            retVal = dut1.StartController();
            EXPECT_EQ(CCanApi::NoError, retVal);
            // @-- check acceptance filter on DUT1 (DUT2 sends frames with 11-bit identifier)
            counter.Clear();
            CheckFilter11Bit(dut1, codeSet[i], maskSet[j]);
            // @-- stop/reset DUT1
            retVal = dut1.ResetController();
            EXPECT_EQ(CCanApi::NoError, retVal);
        }
    }
    counter.Reset();
    // @post:
    // @- reset acceptance filter
    // @  note: SJA100 has only one filter for 11-bit and 29-bit identifier!
    retVal = dut1.ResetFilters();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @end.
}

// @gtest TC23.9: Set CAN acceptance filter for 11-bit identifier with invalid values
//
// @expected: CANERR_ILLPARAM
//
TEST_F(SetFilter11Bit, GTEST_TESTCASE(WithInvalidValues, GTEST_ENABLED)) {
    CCanDevice dut1 = CCanDevice(TEST_DEVICE(DUT1));
    CCanDevice dut2 = CCanDevice(TEST_DEVICE(DUT2));
    CANAPI_Status_t status = {};
    uint32_t codeGet, maskGet;
    uint32_t codeSet, maskSet;
    CANAPI_Return_t retVal;
    // @pre:
    // @- initialize DUT1 with configured settings
    retVal = dut1.InitializeChannel();
    ASSERT_EQ(CCanApi::NoError, retVal) << "[  ERROR!  ] dut1.InitializeChannel() failed with error code " << retVal;
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @test:
    // @- try to set 11-bit filter with invalid value (code 0x800 and mask 0x800)
    codeSet = 0x00000800U; maskSet = 0x00000800U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::IllegalParameter, retVal);
    // @- try to set 11-bit filter with invalid value (code 0x000 and mask 0xFFFF)
    codeSet = 0x00000000U; maskSet = 0x0000FFFFU;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::IllegalParameter, retVal);
    // @- try to set 11-bit filter with invalid value (code 0x1FFFFFFF and mask 0x000)
    codeSet = 0x1FFFFFFFU; maskSet = 0x00000000U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::IllegalParameter, retVal);
    // @- try to set 11-bit filter with invalid value (code 0xFFFFFFFF and mask 0xE0000000)
    codeSet = 0xFFFFFFFFU; maskSet = 0xE0000000U;
    retVal = dut1.SetFilter11Bit(codeSet, maskSet);
    EXPECT_EQ(CCanApi::IllegalParameter, retVal);
    // @post:
    // @- get 11-bit filter (code 0x000 and mask 0x000)
    codeGet = 0xFFFFFFFFU; maskGet = 0xFFFFFFFFU;
    retVal = dut1.GetFilter11Bit(codeGet, maskGet);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_EQ(CANACC_CODE_11BIT, codeGet);
    EXPECT_EQ(CANACC_MASK_11BIT, maskGet);
    // @- start DUT1 with configured bit-rate settings
    retVal = dut1.StartController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- send some frames to DUT2 and receive some frames from DUT2
    int32_t frames = g_Options.GetNumberOfTestFrames();
    EXPECT_EQ(frames, dut1.SendSomeFrames(dut2, frames));
    EXPECT_EQ(frames, dut1.ReceiveSomeFrames(dut2, frames));
    // @- get status of DUT1 and check to be in RUNNING state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_FALSE(status.can_stopped);
    // @- stop/reset DUT1
    retVal = dut1.ResetController();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @- get status of DUT1 and check to be in INIT state
    retVal = dut1.GetStatus(status);
    EXPECT_EQ(CCanApi::NoError, retVal);
    EXPECT_TRUE(status.can_stopped);
    // @- tear down DUT1
    retVal = dut1.TeardownChannel();
    EXPECT_EQ(CCanApi::NoError, retVal);
    // @end.
}

#endif // FEATURE_FILTERING != FEATURE_UNSUPPORTED

//  $Id: TC23_SetFilter11Bit.cc 1411 2025-01-17 18:59:07Z quaoar $  Copyright (c) UV Software, Berlin.
