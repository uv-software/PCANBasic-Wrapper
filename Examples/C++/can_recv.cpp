//
//  can_recv.cpp
//  PCANBasic-Wrapper
//  Receive some CAN messages using the C++ API (PeakCAN.h)
//  Library: PeakCAN.dll, libPeakCAN.dylib, libpeakcan.so
//
#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include <iostream>
#include <signal.h>
#include <errno.h>

#include "PeakCAN.h"

#ifndef CHANNEL
#define CHANNEL  0x51
#endif
#ifndef BAUDRATE
#define BAUDRATE  CANBTR_INDEX_250K
#endif

static void sigterm(int signo);
static volatile int running = 1;

static CPeakCAN myDriver = CPeakCAN();

int main(/*int argc, const char * argv[]*/) {
    CANAPI_OpMode_t opMode = {};
    opMode.byte = CANMODE_DEFAULT;
    CANAPI_Bitrate_t bitrate = {};
    bitrate.index = BAUDRATE;
    CANAPI_Message_t message = {};
    CANAPI_Return_t retVal = 0;
    int frames = 0;

    std::cout << CPeakCAN::GetVersion() << std::endl;
    if((signal(SIGINT, sigterm) == SIG_ERR) ||
#if !defined(_WIN32) && !defined(_WIN64)
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    if ((retVal = myDriver.InitializeChannel(CHANNEL, opMode)) != CCanApi::NoError) {
        std::cerr << "+++ error: interface could not be initialized" << std::endl;
        return retVal;
    }
    if ((retVal = myDriver.StartController(bitrate)) != CCanApi::NoError) {
        std::cerr << "+++ error: interface could not be started" << std::endl;
        goto teardown;
    }
    std::cout << "Press ^C to abort." << std::endl;
    while (running) {
        if ((retVal = myDriver.ReadMessage(message, CANREAD_INFINITE)) == CCanApi::NoError) {
            fprintf(stdout, "%i\t", frames++);
            fprintf(stdout, "%7li.%04li\t", (long)message.timestamp.tv_sec, message.timestamp.tv_nsec / 100000);
            fprintf(stdout, "%03X\t", message.id);
            if (!message.sts) {
                fputc(message.xtd ? 'X' : 'S', stdout);
                fputc(message.fdf ? 'F' : '-', stdout);
                fputc(message.brs ? 'B' : '-', stdout);
                fputc(message.esi ? 'E' : '-', stdout);
                fputc(message.rtr ? 'R' : '-', stdout);
            } else {
                fprintf(stdout, "Error");
            }
            printf(" [%u]", CCanApi::Dlc2Len(message.dlc));
            for (uint8_t i = 0; i < CCanApi::Dlc2Len(message.dlc); i++)
                fprintf(stdout, " %02X", message.data[i]);
            if (message.sts)
                fprintf(stdout, " <<< status frame");
            fprintf(stdout, "\n");
        }
        else if (retVal != CCanApi::ReceiverEmpty) {
            fprintf(stderr, "+++ error: read message returned %i", retVal);
            running = 0;
        }
    }
    std::cout << std::endl;
teardown:
    if ((retVal = myDriver.TeardownChannel()) != CCanApi::NoError)
        std::cerr << "+++ error: interface could not be shutdown" << std::endl;
    std::cout << "Cheers!" << std::endl;
    return retVal;
}

static void sigterm(int signo) {
    //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
    myDriver.SignalChannel();
    running = 0;
    (void)signo;
}
