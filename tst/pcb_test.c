/*  -- $HeadURL$ --
 *
 *  project   :  CAN - Controller Area Network
 *
 *  purpose   :  CAN API V3 Tester (PCAN-Basic)
 *
 *  copyright :  (C) 2005-2010, UV Software, Friedrichshafen
 *               (C) 2014,2017-2019, UV Software, Berlin
 *
 *  compiler  :  Microsoft Visual C/C++ Compiler (Version 19.16)
 *
 *  syntax    :  <program> [<option>...] <file>...
 *               Options:
 *                 -h, --help     display this help and exit
 *                     --version  show version information and exit
 *
 *  libraries :  (none)
 *
 *  includes  :  can_api.h (can_defs.h), dosopt.h, bitrates.h
 *
 *  author    :  Uwe Vogt, UV Software
 *
 *  e-mail    :  uwe.vogt@uv-software.de
 *
 *
 *  -----------  description  --------------------------------------------
 *
 *  <description>
 */

//static const char* __copyright__ = "Copyright (C) 2005-2019 by UV Software, Berlin";
//static const char* __version__   = "0.x";
//static const char* __revision__  = "$Rev$";

/*  -----------  includes  -----------------------------------------------
 */
#include "can_api.h"
#include "bitrates.h"
#include "printmsg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#include <libgen.h>
#include <getopt.h>
#include <pthread.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#else
#include <windows.h>
#ifndef QWORD
#define QWORD unsigned long long
#endif
#endif
#include <inttypes.h>


/*  -----------  options  ------------------------------------------------
 */

#if !defined(__uvs_license) && !defined(__gpl_license)
    #define  __uvs_license
#endif
//#define _WAITABLE_TIMER


/*  -----------  defines  ------------------------------------------------
 */

#define PROMPT_NORMAL       (0)
#define PROMPT_OVERRUN      (1)
#define PROMPT_ISSUE198     (2)

#define TIME_IO_POLLING     (0)
#define TIME_IO_BLOCKING    (65535)

#define OPTION_MODE_CAN_20  (0)
#define OPTION_MODE_CAN_FD  (1)

#define OPTION_TIME_ZERO    (0)
#define OPTION_TIME_ABS     (1)
#define OPTION_TIME_REL     (2)

#define OPTION_IO_POLLING   (0)
#define OPTION_IO_BLOCKING  (1)

#define OPTION_NO           (0)
#define OPTION_YES          (1)

//#define STOP_FRAMES       64//+16

#define DLC2DLEN(dlc)       dtab[(dlc) & 0xFu]

#define BITRATE_DEFAULT     "f_clock_mhz=80,nom_brp=20,nom_tseg1=12,nom_tseg2=3,nom_sjw=1,data_brp=4,data_tseg1=7,data_tseg2=2,data_sjw=1"
#define BITRATE_125K1M      "f_clock_mhz=80,nom_brp=2,nom_tseg1=255,nom_tseg2=64,nom_sjw=64,data_brp=2,data_tseg1=31,data_tseg2=8,data_sjw=8"
#define BITRATE_250K2M      "f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4"
#define BITRATE_500K4M      "f_clock_mhz=80,nom_brp=2,nom_tseg1=63,nom_tseg2=16,nom_sjw=16,data_brp=2,data_tseg1=7,data_tseg2=2,data_sjw=2"
#define BITRATE_1M8M        "f_clock_mhz=80,nom_brp=2,nom_tseg1=31,nom_tseg2=8,nom_sjw=8,data_brp=2,data_tseg1=3,data_tseg2=1,data_sjw=1"
#define BITRATE_125K        "f_clock_mhz=80,nom_brp=2,nom_tseg1=255,nom_tseg2=64,nom_sjw=64"
#define BITRATE_250K        "f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32"
#define BITRATE_500K        "f_clock_mhz=80,nom_brp=2,nom_tseg1=63,nom_tseg2=16,nom_sjw=16"
#define BITRATE_1M          "f_clock_mhz=80,nom_brp=2,nom_tseg1=31,nom_tseg2=8,nom_sjw=8"

#define BR_CiA_125K2M       "f_clock_mhz=80,nom_brp=4,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=4,data_tseg1=6,data_tseg2=3,data_sjw=3"
#define BR_CiA_250K2M       "f_clock_mhz=80,nom_brp=4,nom_tseg1=63,nom_tseg2=16,nom_sjw=16,data_brp=4,data_tseg1=6,data_tseg2=3,data_sjw=3"
#define BR_CiA_500K2M       "f_clock_mhz=80,nom_brp=2,nom_tseg1=63,nom_tseg2=16,nom_sjw=16,data_brp=2,data_tseg1=14,data_tseg2=5,data_sjw=5"
#define BR_CiA_1M5M         "f_clock_mhz=80,nom_brp=2,nom_tseg1=31,nom_tseg2=8,nom_sjw=8,data_brp=2,data_tseg1=5,data_tseg2=2,data_sjw=2"



 /*  -----------  types  --------------------------------------------------
 */


 /*  -----------  prototypes  ---------------------------------------------
 */

static int transmit(int handle, int frames, DWORD delay);
static int receive(int handle);
static int transmit_fd(int handle, int frames, DWORD delay);
static int receive_fd(int handle);
static int convert(const char *string, can_bitrate_t *bitrate);
static void verbose(BYTE op_mode, const can_bitrate_t *bitrate);

#ifndef _WAITABLE_TIMER
 static int start_timer(DWORD timeout);
 static int is_timeout(void);
#else
 static void usleep(QWORD usec);
#endif

static void sigterm(int signo);
//static void usage(FILE *stream, char *program);
//static void version(FILE *stream, char *program);


/*  -----------  variables  ----------------------------------------------
 */

static int option_io = OPTION_IO_BLOCKING;
static int option_time = OPTION_TIME_ZERO;
static int option_test = OPTION_NO;
static int option_info = OPTION_NO;
static int option_stat = OPTION_NO;
static int option_stop = OPTION_NO;
static int option_echo = OPTION_YES;
static int option_check = OPTION_NO;
#if (0)
static int option_trace = OPTION_NO;
static int option_log = OPTION_NO;
#endif
static int option_transmit = 0;
static int option_mode = OPTION_MODE_CAN_20;
static int option_fdf = OPTION_NO;
static int option_brs = OPTION_NO;
#if (STOP_FRAMES != 0)
static int stop_frames = 0;
#endif
static const BYTE dtab[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

static int running = 1;


/*  -----------  functions  ----------------------------------------------
 */

int main(int argc, char *argv[])
/*
 * function : main function of the application.
 *
 * parameter: argc - number of command line arguments.
 *            argv - command line arguments as string vector.
 *
 * result   : 0    - no error occurred.
 */
{
    int handle = -1;
    int rc = -1;
    int opt, i;

    int channel = PCAN_USB1;
    BYTE op_mode = CANMODE_DEFAULT;
    DWORD delay = 0;
    can_bitrate_t bitrate = { -CANBDR_250 };
    char *device, *firmware, *software;

    //struct option long_options[] = {
    //  {"help", no_argument, 0, 'h'},
    //  {"version", no_argument, 0, 'v'},
    //  {0, 0, 0, 0}
    //};

    if((signal(SIGINT, sigterm) == SIG_ERR) ||
#ifndef _WIN32
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    //while((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
    //  switch(opt) {
    //      case 'v':
    //          version(stdout, basename(argv[0]));
    //          return 0;
    //      case 'h':
    //          usage(stdout, basename(argv[0]));
    //          return 0;
    //      default:
    //          usage(stderr, basename(argv[0]));
    //          return 1;
    //  }
    //}
    for(i = 1; i < argc; i++) {
        /* PCAN-USB channel */
        if(!strcmp(argv[i], "PCAN-USB1")) channel = PCAN_USB1;
        if(!strcmp(argv[i], "PCAN-USB2")) channel = PCAN_USB2;
        if(!strcmp(argv[i], "PCAN-USB3")) channel = PCAN_USB3;
        if(!strcmp(argv[i], "PCAN-USB4")) channel = PCAN_USB4;
        if(!strcmp(argv[i], "PCAN-USB5")) channel = PCAN_USB5;
        if(!strcmp(argv[i], "PCAN-USB6")) channel = PCAN_USB6;
        if(!strcmp(argv[i], "PCAN-USB7")) channel = PCAN_USB7;
        if(!strcmp(argv[i], "PCAN-USB8")) channel = PCAN_USB8;
#if !defined(__APPLE__)
        if(!strcmp(argv[i], "PCAN-USB9")) channel = PCAN_USB9;
        if(!strcmp(argv[i], "PCAN-USB10")) channel = PCAN_USB10;
        if(!strcmp(argv[i], "PCAN-USB11")) channel = PCAN_USB11;
        if(!strcmp(argv[i], "PCAN-USB12")) channel = PCAN_USB12;
        if(!strcmp(argv[i], "PCAN-USB13")) channel = PCAN_USB13;
        if(!strcmp(argv[i], "PCAN-USB14")) channel = PCAN_USB14;
        if(!strcmp(argv[i], "PCAN-USB15")) channel = PCAN_USB15;
        if(!strcmp(argv[i], "PCAN-USB16")) channel = PCAN_USB16;
#endif
        /* baud rate (CAN 2.0) */
        if(!strcmp(argv[i], "BD:0") || !strcmp(argv[i], "BD:1000")) bitrate.index = -CANBDR_1000;
        if(!strcmp(argv[i], "BD:1") || !strcmp(argv[i], "BD:800")) bitrate.index = -CANBDR_800;
        if(!strcmp(argv[i], "BD:2") || !strcmp(argv[i], "BD:500")) bitrate.index = -CANBDR_500;
        if(!strcmp(argv[i], "BD:3") || !strcmp(argv[i], "BD:250")) bitrate.index = -CANBDR_250;
        if(!strcmp(argv[i], "BD:4") || !strcmp(argv[i], "BD:125")) bitrate.index = -CANBDR_125;
        if(!strcmp(argv[i], "BD:5") || !strcmp(argv[i], "BD:100")) bitrate.index = -CANBDR_100;
        if(!strcmp(argv[i], "BD:6") || !strcmp(argv[i], "BD:50")) bitrate.index = -CANBDR_50;
        if(!strcmp(argv[i], "BD:7") || !strcmp(argv[i], "BD:20")) bitrate.index = -CANBDR_20;
        if(!strcmp(argv[i], "BD:8") || !strcmp(argv[i], "BD:10")) bitrate.index = -CANBDR_10;
        /* asynchronous IO */
        if(!strcmp(argv[i], "POLLING")) option_io = OPTION_IO_POLLING;
        if(!strcmp(argv[i], "BLOCKING")) option_io = OPTION_IO_BLOCKING;
        /* test all channels: not present / available / occupied */
        if(!strcmp(argv[i], "TEST")) option_test = OPTION_YES;
        /* query some informations: hw, sw, etc. */
        if(!strcmp(argv[i], "INFO")) option_info = OPTION_YES;
        /* query some statistical data */
        if(!strcmp(argv[i], "STAT")) option_stat = OPTION_YES;
        /* stop on error */
        if(!strcmp(argv[i], "STOP")) option_stop = OPTION_YES;
        if(!strcmp(argv[i], "IGNORE")) option_check = OPTION_NO;
        if(!strcmp(argv[i], "CHECK")) option_check = OPTION_YES;
        /* echo ON/OFF */
        if(!strcmp(argv[i], "SILENT")) option_echo = OPTION_NO;
        /* time-stamps */
        if(!strcmp(argv[i], "ZERO")) option_time = OPTION_TIME_ZERO;
        if(!strcmp(argv[i], "ABS") || !strcmp(argv[i], "ABSOLUTE")) option_time = OPTION_TIME_ABS;
        if(!strcmp(argv[i], "REL") || !strcmp(argv[i], "RELATIVE")) option_time = OPTION_TIME_REL;
#if (0)
        /* logging and debugging */
        if(!strcmp(argv[i], "TRACE")) option_trace = OPTION_YES;
        if(!strcmp(argv[i], "LOG")) option_log = OPTION_YES;
#endif
        /* transmit messages */
        if(!strncmp(argv[i], "C:", 2) && sscanf(argv[i], "C:%i", &opt) == 1) delay = (DWORD)opt * 1000;
        if(!strncmp(argv[i], "U:", 2) && sscanf(argv[i], "U:%i", &opt) == 1) delay = (DWORD)opt;
        if(sscanf(argv[i], "%i", &opt) == 1 && opt > 0) option_transmit = opt;
        /* CAN FD operation */
        if(!strcmp(argv[i], "CANFD") || !strcmp(argv[i], "FD")) { option_mode = OPTION_MODE_CAN_FD; op_mode = CANMODE_FDOE; }
        if(!strcmp(argv[i], "FDF")) { option_fdf = OPTION_YES; option_mode = OPTION_MODE_CAN_FD; op_mode = CANMODE_FDOE; }
        if(!strcmp(argv[i], "BRS")) { option_brs = OPTION_YES; option_fdf = OPTION_YES; option_mode = OPTION_MODE_CAN_FD; op_mode = CANMODE_FDOE | CANMODE_BRSE; }
        /* bit rate (CAN FD) */
        if(!strcmp(argv[i], "BR:125K1M")) convert(BITRATE_125K1M, &bitrate);
        if(!strcmp(argv[i], "BR:250K2M")) convert(BITRATE_250K2M, &bitrate);
        if(!strcmp(argv[i], "BR:500K4M")) convert(BITRATE_500K4M, &bitrate);
        if(!strcmp(argv[i], "BR:1M8M")) convert(BITRATE_1M8M, &bitrate);
        if(!strcmp(argv[i], "BR:125K")) convert(BITRATE_125K, &bitrate);
        if(!strcmp(argv[i], "BR:250K")) convert(BITRATE_250K, &bitrate);
        if(!strcmp(argv[i], "BR:500K")) convert(BITRATE_500K, &bitrate);
        if(!strcmp(argv[i], "BR:1M")) convert(BITRATE_1M, &bitrate);
        if(!strcmp(argv[i], "BR:CiA125K2M")) convert(BR_CiA_125K2M, &bitrate);
        if(!strcmp(argv[i], "BR:CiA250K2M")) convert(BR_CiA_250K2M, &bitrate);
        if(!strcmp(argv[i], "BR:CiA5002M")) convert(BR_CiA_500K2M, &bitrate);
        if(!strcmp(argv[i], "BR:CiA1M5M")) convert(BR_CiA_1M5M, &bitrate);
    }
    fprintf(stdout, "can_test: "__DATE__" "__TIME__" (MSC_VER=%u)\n", _MSC_VER);
    /* channel tester */
    if(option_test) {
        for(i = 0; i < PCAN_BOARDS; i++) {
            if((rc = can_test(can_board[i].type, op_mode, NULL, &opt)) == CANERR_NOERROR)
                printf("Channel 0x%02lx: %s\n", can_board[i].type, opt == CANBRD_OCCUPIED ? "occuptied" : opt == CANBRD_PRESENT ? "available" : "unavailable");
            else if(rc == CANERR_ILLPARA)
                printf("Channel 0x%02lx: incompatible\n", can_board[i].type);
            else
                printf("Channel 0x%02lx: can_test failed (%i)\n", can_board[i].type, rc);
        }
    }
    /* offline informations */
    if(option_info) {
        if((software = can_version()) != NULL)
            fprintf(stdout, "Software: %s\n", software);
        for(i = 0; i < PCAN_BOARDS; i++) {
            if(channel == can_board[i].type) {
                fprintf(stdout, "Hardware: %s (0x%lx)\n", can_board[i].name, can_board[i].type);
            }
        }
        verbose(op_mode, &bitrate);
    }
    /* initialization */
    if((rc = can_init(channel, op_mode, NULL)) < 0) {
        printf("+++ error(%i): can_init failed\n", rc);
        goto end;
    }
    handle = rc;
    /* channel status */
    if(option_test) {
        if((rc = can_test(channel, op_mode, NULL, &opt)) == CANERR_NOERROR)
            printf("Channel 0x%02lx: %s\n", channel, opt == CANBRD_OCCUPIED ? "now occuptied" : opt == CANBRD_PRESENT ? "available" : "unavailable");
        else if(rc == CANERR_ILLPARA)
            printf("Channel 0x%02lx: incompatible\n", channel);
        else
            printf("Channel 0x%02lx: can_test failed (%i)\n", channel, rc);
    }
    /* start communication */
    if((rc = can_start(handle, &bitrate)) != CANERR_NOERROR) {
        printf("+++ error(%i): can_start failed\n", rc);
        goto end;
    }
    /* transmit messages */
    if(option_transmit > 0) {
        if(option_mode == OPTION_MODE_CAN_20) {
            if(transmit(handle, option_transmit, delay) < 0)
                goto end;
        }
        else {
            if(transmit_fd(handle, option_transmit, delay) < 0)
                goto end;
        }
    }
    /* reception loop */
    if(option_mode == OPTION_MODE_CAN_20) {
        if(receive(handle) < 0)
            goto end;
    }
    else {
        if(receive_fd(handle) < 0)
            goto end;
    }
    /* online information */
    if(option_info) {
        if((device = can_hardware(handle)) != NULL)
            fprintf(stdout, "Hardware: %s\n", device);
        if((firmware = can_software(handle)) != NULL)
            fprintf(stdout, "Firmware: %s\n", firmware);
    }
end:
    printf("Teardown.."); fflush(stdout);
    if((rc = can_exit(handle)) != CANERR_NOERROR) {
        printf("FAILED\n");
        printf("+++ error(%i): can_exit failed\n", rc);
        return 1;
    }
    printf("Bye!\n");
    return 0;
}

static int transmit(int handle, int frames, DWORD delay)
{
    can_msg_t message;
    int rc = -1, i;

    printf("Transmit CAN 2.0 messages");
    memset(&message, 0, sizeof(can_msg_t));

    for(i = 0; i < frames; i++) {
        message.id = (DWORD)i % 0x800UL;
        message.dlc = (BYTE)8;
        message.data[0] = (BYTE)(((QWORD)i & 0x00000000000000FF) >> 0);
        message.data[1] = (BYTE)(((QWORD)i & 0x000000000000FF00) >> 8);
        message.data[2] = (BYTE)(((QWORD)i & 0x0000000000FF0000) >> 16);
        message.data[3] = (BYTE)(((QWORD)i & 0x00000000FF000000) >> 24);
        message.data[4] = (BYTE)(((QWORD)i & 0x000000FF00000000) >> 32);
        message.data[5] = (BYTE)(((QWORD)i & 0x0000FF0000000000) >> 40);
        message.data[6] = (BYTE)(((QWORD)i & 0x00FF000000000000) >> 48);
        message.data[7] = (BYTE)(((QWORD)i & 0xFF00000000000000) >> 56);

        start_timer(delay);
repeat:
        if((rc = can_write(handle, &message)) != CANERR_NOERROR) {
            if(rc == CANERR_TX_BUSY && running)
                goto repeat;
            printf("+++ error(%i): can_write failed\n", rc);
            if(option_stop)
                return -1;
        }
        while(!is_timeout()) {
            if(!running) {
                printf("%i\n", frames);
                return i;
            }
        }
        if(!(i % 2048)) {
            fprintf(stdout, ".");
            fflush(stdout);
        }
        if(!running) {
            printf("%i\n", frames);
            return i;
        }
    }
    printf("%i\n", frames);
    return i;
}

static int transmit_fd(int handle, int frames, DWORD delay)
{
    can_msg_t message;
    int rc = -1, i; BYTE j;

    printf("Transmit CAN FD messages");
    memset(&message, 0, sizeof(can_msg_t));
    if(option_brs)
        message.brs = 1;
    if(option_fdf)
        message.fdf = 1;
    for(i = 0, j = 0; i < frames; i++, j++) {
        message.id = (DWORD)i % 0x800UL;
        if(option_fdf)
            message.dlc = (BYTE)(8 + (j % 8)) & 0xF;
        else
            message.dlc = (BYTE)(8);
        message.data[0] = (BYTE)(((QWORD)i & 0x00000000000000FF) >> 0);
        message.data[1] = (BYTE)(((QWORD)i & 0x000000000000FF00) >> 8);
        message.data[2] = (BYTE)(((QWORD)i & 0x0000000000FF0000) >> 16);
        message.data[3] = (BYTE)(((QWORD)i & 0x00000000FF000000) >> 24);
        message.data[4] = (BYTE)(((QWORD)i & 0x000000FF00000000) >> 32);
        message.data[5] = (BYTE)(((QWORD)i & 0x0000FF0000000000) >> 40);
        message.data[6] = (BYTE)(((QWORD)i & 0x00FF000000000000) >> 48);
        message.data[7] = (BYTE)(((QWORD)i & 0xFF00000000000000) >> 56);

        start_timer(delay);
repeat_fd:
        if((rc = can_write(handle, &message)) != CANERR_NOERROR) {
            if(rc == CANERR_TX_BUSY && running)
                goto repeat_fd;
            printf("+++ error(%i): can_write failed\n", rc);
            if(option_stop)
                return -1;
        }
        while(!is_timeout()) {
            if(!running) {
                printf("%i\n", frames);
                return i;
            }
        }
        if(!(i % 2048)) {
            fprintf(stdout, ".");
            fflush(stdout);
        }
        if(!running) {
            printf("%i\n", frames);
            return i;
        }
    }
    printf("%i\n", frames);
    return i;
}

static int receive(int handle)
{
    can_msg_t message;
    int rc = -1, i;

    QWORD received = 0;
    QWORD expected = 0;
    QWORD frames = 0;
    QWORD errors = 0;

    char symbol[] = ">!?";
    int prompt = 0;

    printf("Receiving CAN 2.0 messages (%s)", (option_io == OPTION_IO_BLOCKING) ? "blocking" : "polling");
    if(option_echo)
        printf(":\n");
    else
        fflush(stdout);
    while(running) {
        if((rc = can_read(handle, &message, (option_io == OPTION_IO_BLOCKING) ? TIME_IO_BLOCKING : TIME_IO_POLLING)) == CANERR_NOERROR) {
            if(option_echo) {
                printf("%c %"PRIu64"\t", symbol[prompt], frames++);
                msg_print_time(stdout, (struct msg_timestamp*)&message.timestamp, option_time);  // an evil cast!
                msg_print_id(stdout, message.id, message.ext, message.rtr, message.dlc, MSG_MODE_HEX);
                for(i = 0; i < message.dlc; i++)
                    msg_print_data(stdout, message.data[i], ((i+1) == message.dlc), MSG_MODE_HEX);
                printf("\n");
            }
            else {
                if(!(frames++ % 2048)) {
                    fprintf(stdout, ".");
                    fflush(stdout);
                }
            }
            received = 0;
            if(message.dlc > 0) received |= (QWORD)message.data[0] << 0;
            if(message.dlc > 1) received |= (QWORD)message.data[1] << 8;
            if(message.dlc > 2) received |= (QWORD)message.data[2] << 16;
            if(message.dlc > 3) received |= (QWORD)message.data[3] << 24;
            if(message.dlc > 4) received |= (QWORD)message.data[4] << 32;
            if(message.dlc > 5) received |= (QWORD)message.data[5] << 40;
            if(message.dlc > 6) received |= (QWORD)message.data[6] << 48;
            if(message.dlc > 7) received |= (QWORD)message.data[7] << 56;
            if(received != expected) {
                if(option_check != OPTION_NO) {
                    printf("+++ error(X): received data is not equal to expected data (%"PRIu64" : %"PRIu64")\n", received, expected);
                    if(expected > received) {
                        printf("              issue #198: old messages on pipe #3 (offset -%"PRIu64")\a\n", expected - received);
#if (STOP_FRAMES == 0)
                        if(option_stop)
                            return -1;
#endif
                        prompt = PROMPT_ISSUE198;
                    }
                    else {
                        prompt = prompt != PROMPT_ISSUE198 ? PROMPT_OVERRUN : PROMPT_ISSUE198;
                    }
                }
            }
#if (STOP_FRAMES != 0)
            if((prompt == PROMPT_ISSUE198) && (option_stop) && (++stop_frames >= STOP_FRAMES))
                return -1;
#endif
            expected = received + 1;
        }
        else if(rc != CANERR_RX_EMPTY) {
            printf("+++ error(%i): can_read failed\n", rc);
            errors++;
        }
    }
    if(!option_echo) {
        fprintf(stdout, "%"PRIu64"\n", frames);
        fflush(stdout);
    }
    return 0;
}

static int receive_fd(int handle)
{
    can_msg_t message;
    int rc = -1, i;

    QWORD received = 0;
    QWORD expected = 0;
    QWORD frames = 0;
    QWORD errors = 0;

    char symbol[] = ">!?";
    int prompt = 0;

    printf("Receiving CAN FD messages (%s)", (option_io == OPTION_IO_BLOCKING) ? "blocking" : "polling");
    if(option_echo)
        printf(":\n");
    else
        fflush(stdout);
    while(running) {
        if((rc = can_read(handle, &message, (option_io == OPTION_IO_BLOCKING) ? TIME_IO_BLOCKING : TIME_IO_POLLING)) == CANERR_NOERROR) {
            if(option_echo) {
                printf("%c %"PRIu64"\t", symbol[prompt], frames++);
                msg_print_time(stdout, (struct msg_timestamp*)&message.timestamp, option_time);  // an evil cast!
                msg_print_id_fd(stdout, message.id, message.ext, message.rtr, message.fdf, message.brs, message.esi, DLC2DLEN(message.dlc), MSG_MODE_HEX);
                for(i = 0; i < DLC2DLEN(message.dlc); i++)
                    msg_print_data(stdout, message.data[i], ((i + 1) == DLC2DLEN(message.dlc)), MSG_MODE_HEX);
                printf("\n");
            }
            else {
                if(!(frames++ % 2048)) {
                    fprintf(stdout, ".");
                    fflush(stdout);
                }
            }
            received = 0;
            if(message.dlc > 0) received |= (QWORD)message.data[0] << 0;
            if(message.dlc > 1) received |= (QWORD)message.data[1] << 8;
            if(message.dlc > 2) received |= (QWORD)message.data[2] << 16;
            if(message.dlc > 3) received |= (QWORD)message.data[3] << 24;
            if(message.dlc > 4) received |= (QWORD)message.data[4] << 32;
            if(message.dlc > 5) received |= (QWORD)message.data[5] << 40;
            if(message.dlc > 6) received |= (QWORD)message.data[6] << 48;
            if(message.dlc > 7) received |= (QWORD)message.data[7] << 56;
            if(received != expected) {
                if(option_check != OPTION_NO) {
                    printf("+++ error(X): received data is not equal to expected data (%"PRIu64" : %"PRIu64")\n", received, expected);
                    if(expected > received) {
                        printf("              issue #198: old messages on pipe #3 (offset -%"PRIu64")\a\n", expected - received);
#if (STOP_FRAMES == 0)
                        if(option_stop)
                            return -1;
#endif
                        prompt = PROMPT_ISSUE198;
                    }
                    else {
                        prompt = prompt != PROMPT_ISSUE198 ? PROMPT_OVERRUN : PROMPT_ISSUE198;
                    }
                }
            }
#if (STOP_FRAMES != 0)
            if((prompt == PROMPT_ISSUE198) && (option_stop) && (++stop_frames >= STOP_FRAMES))
                return -1;
#endif
            expected = received + 1;
        }
        else if(rc != CANERR_RX_EMPTY) {
            printf("+++ error(%i): can_read failed\n", rc);
            errors++;
        }
    }
    if(!option_echo) {
        fprintf(stdout, "%"PRIu64"\n", frames);
        fflush(stdout);
    }
    return 0;
}

static int convert(const char *string, can_bitrate_t *bitrate)
{
    unsigned long freq = 0; struct btr_bit_timing slow, fast;

    if(!btr_string_to_bit_timing(string, &freq, &slow, &fast)) {
        fprintf(stderr, "+++ error: illegal argument in option /BITRATE!\n\n");
        return 0;
    }
    bitrate->btr.frequency = (long)freq;
    bitrate->btr.nominal.brp = (unsigned short)slow.brp;
    bitrate->btr.nominal.tseg1 = (unsigned short)slow.tseg1;
    bitrate->btr.nominal.tseg2 = (unsigned short)slow.tseg2;
    bitrate->btr.nominal.sjw = (unsigned short)slow.sjw;
    bitrate->btr.nominal.sam = (unsigned char)slow.sam;
    bitrate->btr.data.brp = (unsigned short)fast.brp;
    bitrate->btr.data.tseg1 = (unsigned short)fast.tseg1;
    bitrate->btr.data.tseg2 = (unsigned short)fast.tseg2;
    bitrate->btr.data.sjw = (unsigned short)fast.sjw;
    return 1;
}

static void verbose(BYTE op_mode, const can_bitrate_t *bitrate)
{
    unsigned long freq; struct btr_bit_timing slow, fast;

    freq = bitrate->btr.frequency;
    slow.brp = bitrate->btr.nominal.brp;
    slow.tseg1 = bitrate->btr.nominal.tseg1;
    slow.tseg2 = bitrate->btr.nominal.tseg2;
    slow.sjw = bitrate->btr.nominal.sjw;
    fast.brp = bitrate->btr.data.brp;
    fast.tseg1 = bitrate->btr.data.tseg1;
    fast.tseg2 = bitrate->btr.data.tseg2;
    fast.sjw = bitrate->btr.data.sjw;


    if(bitrate->btr.frequency > 0) {
        fprintf(stdout, "Baudrate: %lubps@%.2f%%",
            btr_calc_bit_rate_nominal(&slow, freq),
            btr_calc_sample_point_nominal(&slow) * 100.);
        if((op_mode & (CANMODE_FDOE | CANMODE_BRSE)) == (CANMODE_FDOE | CANMODE_BRSE))
            fprintf(stdout, ":%lubps@%.2f%%",
                btr_calc_bit_rate_data(&fast, freq),
                btr_calc_sample_point_data(&fast) * 100.);
        fprintf(stdout, " (f_clock=%lu,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u",
            bitrate->btr.frequency,
            bitrate->btr.nominal.brp,
            bitrate->btr.nominal.tseg1,
            bitrate->btr.nominal.tseg2,
            bitrate->btr.nominal.sjw);
        if((op_mode & (CANMODE_FDOE | CANMODE_BRSE)) == (CANMODE_FDOE | CANMODE_BRSE))
            fprintf(stdout, ",data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                bitrate->btr.data.brp,
                bitrate->btr.data.tseg1,
                bitrate->btr.data.tseg2,
                bitrate->btr.data.sjw);
        fprintf(stdout, ")\n");
    }
    else {
        fprintf(stdout, "Baudrate: %sbps (CiA index %li)\n",
            bitrate->index == CANBDR_1000 ? "1000000" :
            bitrate->index == -CANBDR_800 ? "800000" :
            bitrate->index == -CANBDR_500 ? "500000" :
            bitrate->index == -CANBDR_250 ? "250000" :
            bitrate->index == -CANBDR_125 ? "125000" :
            bitrate->index == -CANBDR_100 ? "100000" :
            bitrate->index == -CANBDR_50 ? "50000" :
            bitrate->index == -CANBDR_20 ? "20000" :
            bitrate->index == -CANBDR_10 ? "10000" : "?", -bitrate->index);
    }
}

static void sigterm(int signo)
{
    //printf("%s: got signal %d\n", __FILE__, signo);
    running = 0;
    (void)signo;
	(void)can_kill(CANKILL_ALL);
}

#ifndef _WAITABLE_TIMER
 static LONGLONG  llUntilStop = 0;      // counter value for time-out

 static int start_timer(DWORD timeout)
 {
    LARGE_INTEGER largeCounter;         // high-resolution performance counter
    LARGE_INTEGER largeFrequency;       // frequency in counts per second

    // retrieve the frequency of the high-resolution performance counter
    if (!QueryPerformanceFrequency(&largeFrequency))
        return CANERR_FATAL;
    // retrieve the current value of the high-resolution performance counter
    if (!QueryPerformanceCounter(&largeCounter))
        return CANERR_FATAL;
    // calculate the counter value for the desired time-out
    llUntilStop = largeCounter.QuadPart + ((largeFrequency.QuadPart * (LONGLONG)timeout)
                                                                    / (LONGLONG)1000000);
    return 0;
 }

 static int is_timeout(void)
 {
    LARGE_INTEGER largeCounter;         // high-resolution performance counter

    // retrieve the current value of the high-resolution performance counter
    if (!QueryPerformanceCounter(&largeCounter))
        return FALSE;
    // a time-out occurred, if the counter overruns the time-out value
    if (largeCounter.QuadPart < llUntilStop)
        return FALSE;
    else
        return TRUE;
 }
#else
 static void usleep(QWORD usec)
 {
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * (LONGLONG)usec); // Convert to 100 nanosecond interval, negative value indicates relative time
    if (usec >= 100) {
        timer = CreateWaitableTimer(NULL, TRUE, NULL);
        SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
 }
#endif

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
