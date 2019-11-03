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
 *  libraries :  PCANBasic.lib
 *
 *  includes  :  can_api.h (can_defs.h), misc\printmsg.h
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
#include "misc\printmsg.h"

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
#define _WAITABLE_TIMER


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

#define BITRATE_DEFAULT(x)  do{ x.btr.frequency=80000000; x.btr.nominal.brp=20; x.btr.nominal.tseg1= 12; x.btr.nominal.tseg2= 3; x.btr.nominal.sjw= 1; x.btr.data.brp=4; x.btr.data.tseg1= 7; x.btr.data.tseg2=2; x.btr.data.sjw=1; } while(0)
#define BITRATE_125K1M(x)   do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1=255; x.btr.nominal.tseg2=64; x.btr.nominal.sjw=64; x.btr.data.brp=2; x.btr.data.tseg1=31; x.btr.data.tseg2=8; x.btr.data.sjw=8; } while(0)
#define BITRATE_250K2M(x)   do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1=127; x.btr.nominal.tseg2=32; x.btr.nominal.sjw=32; x.btr.data.brp=2; x.btr.data.tseg1=15; x.btr.data.tseg2=4; x.btr.data.sjw=4; } while(0)
#define BITRATE_500K4M(x)   do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1= 63; x.btr.nominal.tseg2=16; x.btr.nominal.sjw=16; x.btr.data.brp=2; x.btr.data.tseg1= 7; x.btr.data.tseg2=2; x.btr.data.sjw=2; } while(0)
#define BITRATE_1M8M(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1= 31; x.btr.nominal.tseg2= 8; x.btr.nominal.sjw= 8; x.btr.data.brp=2; x.btr.data.tseg1= 3; x.btr.data.tseg2=1; x.btr.data.sjw=1; } while(0)
#define BITRATE_125K(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1=255; x.btr.nominal.tseg2=64; x.btr.nominal.sjw=64; } while(0)
#define BITRATE_250K(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1=127; x.btr.nominal.tseg2=32; x.btr.nominal.sjw=32; } while(0)
#define BITRATE_500K(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1= 63; x.btr.nominal.tseg2=16; x.btr.nominal.sjw=16; } while(0)
#define BITRATE_1M(x)       do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1= 31; x.btr.nominal.tseg2= 8; x.btr.nominal.sjw= 8; } while(0)

#define BR_CiA_125K2M(x)    do{ x.btr.frequency=80000000; x.btr.nominal.brp= 4; x.btr.nominal.tseg1=127; x.btr.nominal.tseg2=32; x.btr.nominal.sjw=32; x.btr.data.brp=4; x.btr.data.tseg1= 6; x.btr.data.tseg2=3; x.btr.data.sjw=3; } while(0)
#define BR_CiA_250K2M(x)    do{ x.btr.frequency=80000000; x.btr.nominal.brp= 4; x.btr.nominal.tseg1= 63; x.btr.nominal.tseg2=16; x.btr.nominal.sjw=16; x.btr.data.brp=4; x.btr.data.tseg1= 6; x.btr.data.tseg2=3; x.btr.data.sjw=3; } while(0)
#define BR_CiA_500K2M(x)    do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1= 63; x.btr.nominal.tseg2=16; x.btr.nominal.sjw=16; x.btr.data.brp=2; x.btr.data.tseg1=14; x.btr.data.tseg2=5; x.btr.data.sjw=5; } while(0)
#define BR_CiA_1M5M(x)      do{ x.btr.frequency=80000000; x.btr.nominal.brp= 2; x.btr.nominal.tseg1= 31; x.btr.nominal.tseg2= 8; x.btr.nominal.sjw =8; x.btr.data.brp=2; x.btr.data.tseg1= 5; x.btr.data.tseg2=2; x.btr.data.sjw=2; } while(0)


 /*  -----------  types  --------------------------------------------------
 */


 /*  -----------  prototypes  ---------------------------------------------
 */

static int transmit(int handle, int frames, DWORD delay);
static int receive(int handle);
static int transmit_fd(int handle, int frames, DWORD delay);
static int receive_fd(int handle);
static void verbose(const can_bitrate_t *bitrate, const can_speed_t *speed);

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
    can_speed_t speed;
    can_status_t status;
    char *device, *firmware, *software;

    unsigned char  uchar;
    unsigned short ushort;
    unsigned long  ulong;
    char string[CANPROP_BUF_MAX_SIZE];
    
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
        if(!strcmp(argv[i], "BR:125K1M")) BITRATE_125K1M(bitrate);
        if(!strcmp(argv[i], "BR:250K2M")) BITRATE_250K2M(bitrate);
        if(!strcmp(argv[i], "BR:500K4M")) BITRATE_500K4M(bitrate);
        if(!strcmp(argv[i], "BR:1M8M")) BITRATE_1M8M(bitrate);
        if(!strcmp(argv[i], "BR:125K")) BITRATE_125K(bitrate);
        if(!strcmp(argv[i], "BR:250K")) BITRATE_250K(bitrate);
        if(!strcmp(argv[i], "BR:500K")) BITRATE_500K(bitrate);
        if(!strcmp(argv[i], "BR:1M")) BITRATE_1M(bitrate);
        if(!strcmp(argv[i], "BR:CiA125K2M")) BR_CiA_125K2M(bitrate);
        if(!strcmp(argv[i], "BR:CiA250K2M")) BR_CiA_250K2M(bitrate);
        if(!strcmp(argv[i], "BR:CiA5002M")) BR_CiA_500K2M(bitrate);
        if(!strcmp(argv[i], "BR:CiA1M5M")) BR_CiA_1M5M(bitrate);
        /* additional operation modes */
        //if(!strcmp(argv[i], "SHARED")) op_mode |= CANMODE_SHRD;
        if(!strcmp(argv[i], "MONITOR")) op_mode |= CANMODE_MON;
        if(!strcmp(argv[i], "ERR:ON")) op_mode |= CANMODE_ERR;
        if(!strcmp(argv[i], "XTD:OFF")) op_mode |= CANMODE_NXTD;
        if(!strcmp(argv[i], "RTR:OFF")) op_mode |= CANMODE_NRTR;
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
        if((rc = can_property(CANAPI_HANDLE, CANPROP_GET_SPEC, (void*)&ushort, sizeof(ushort))) == CANERR_NOERROR)
            fprintf(stdout, "Property: CANPROP_GET_SPEC=%03xh\n", ushort);
        else
            printf("+++ error(%i): can_property(CANPROP_GET_SPEC) failed\n", rc);
        if((rc = can_property(CANAPI_HANDLE, CANPROP_GET_VERSION, (void*)&ushort, sizeof(ushort))) == CANERR_NOERROR)
            fprintf(stdout, "Property: CANPROP_GET_VERSION=%03xh\n", ushort);
        else
            printf("+++ error(%i): can_property(CANPROP_GET_VERSION) failed\n", rc);
        if((rc = can_property(CANAPI_HANDLE, CANPROP_GET_REVISION, (void*)&uchar, sizeof(uchar))) == CANERR_NOERROR)
            fprintf(stdout, "Property: CANPROP_GET_REVISION=%xh\n", uchar);
        else
            printf("+++ error(%i): can_property(CANPROP_GET_REVISION) failed\n", rc);
        if((rc = can_property(CANAPI_HANDLE, CANPROP_GET_BUILD_NO, (void*)&ulong, sizeof(ulong))) == CANERR_NOERROR)
            fprintf(stdout, "Property: CANPROP_GET_BUILD_NO=%lxh\n", ulong);
        else
            printf("+++ error(%i): can_property(CANPROP_GET_BUILD_NO) failed\n", rc);
        if((rc = can_property(CANAPI_HANDLE, CANPROP_GET_LIBRARY_ID, (void*)&i, sizeof(i))) == CANERR_NOERROR)
            fprintf(stdout, "Property: CANPROP_GET_LIBRARY_ID=(%d)\n", i);
        else
            printf("+++ error(%i): can_property(CANPROP_GET_LIBRARY_ID) failed\n", rc);
        if((rc = can_property(CANAPI_HANDLE, CANPROP_GET_LIBRARY_DLL, (void*)string, CANPROP_BUF_MAX_SIZE)) == CANERR_NOERROR)
            fprintf(stdout, "Property: CANPROP_GET_LIBRARY_DLL=%s\n", string);
        else
            printf("+++ error(%i): can_property(CANPROP_GET_LIBRARY_DLL) failed\n", rc);
        if((rc = can_property(CANAPI_HANDLE, CANPROP_GET_VENDOR_NAME, (void*)string, CANPROP_BUF_MAX_SIZE)) == CANERR_NOERROR)
            fprintf(stdout, "Property: CANPROP_GET_VENDOR_NAME=%s\n", string);
        else
            printf("+++ error(%i): can_property(CANPROP_GET_VENDOR_NAME) failed\n", rc);
        for(i = 0; i < PCAN_BOARDS; i++) {
            if(channel == can_board[i].type) {
                fprintf(stdout, "Hardware: %s (0x%lx)\n", can_board[i].name, can_board[i].type);
            }
        }
    }
    /* initialization */
    if((handle = can_init(channel, op_mode, NULL)) < 0) {
        printf("+++ error(%i): can_init failed\n", handle);
        goto end;
    }
    if((rc = can_status(handle, &status.byte)) != CANERR_NOERROR) {
        printf("+++ error(%i): can_status failed\n", rc);
        goto end;
    }
	if(option_info) {
        // TODO: ...
    }
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
    if((rc = can_status(handle, &status.byte)) != CANERR_NOERROR) {
        printf("+++ error(%i): can_status failed\n", rc);
        goto end;
    }
    if((rc = can_bitrate(handle, &bitrate, &speed)) != CANERR_NOERROR) {
        printf("+++ error(%i): can_bitrate failed\n", rc);
        goto end;
    }
	if(option_info) {
        verbose(&bitrate, &speed);
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
    if((rc = can_exit(CANEXIT_ALL)) != CANERR_NOERROR) {
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

#ifndef _WAITABLE_TIMER
        start_timer(delay);
#endif
repeat:
        if((rc = can_write(handle, &message)) != CANERR_NOERROR) {
            if(rc == CANERR_TX_BUSY && running)
                goto repeat;
            printf("+++ error(%i): can_write failed\n", rc);
            if(option_stop)
                return -1;
        }
#ifndef _WAITABLE_TIMER
        while(!is_timeout()) {
            if(!running) {
                printf("%i\n", frames);
                return i;
            }
        }
#else
        usleep(delay);
#endif
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

#ifndef _WAITABLE_TIMER
        start_timer(delay);
#endif
repeat_fd:
        if((rc = can_write(handle, &message)) != CANERR_NOERROR) {
            if(rc == CANERR_TX_BUSY && running)
                goto repeat_fd;
            printf("+++ error(%i): can_write failed\n", rc);
            if(option_stop)
                return -1;
        }
#ifndef _WAITABLE_TIMER
        while(!is_timeout()) {
            if(!running) {
                printf("%i\n", frames);
                return i;
            }
        }
#else
        usleep(delay);
#endif
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

static void verbose(const can_bitrate_t *bitrate, const can_speed_t *speed)
{
    //if(bitrate->btr.frequency > 0) {
        fprintf(stdout, "Baudrate: %.0fkbps@%.1f%%",
            speed->nominal.speed / 1000., speed->nominal.samplepoint * 100.);
        if(speed->data.brse)
            fprintf(stdout, ":%.0fkbps@%.1f%%",
                speed->data.speed / 1000., speed->data.samplepoint * 100.);
        fprintf(stdout, " (f_clock=%lu,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u",
            bitrate->btr.frequency,
            bitrate->btr.nominal.brp,
            bitrate->btr.nominal.tseg1,
            bitrate->btr.nominal.tseg2,
            bitrate->btr.nominal.sjw);
        if(speed->data.brse)
            fprintf(stdout, ",data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                bitrate->btr.data.brp,
                bitrate->btr.data.tseg1,
                bitrate->btr.data.tseg2,
                bitrate->btr.data.sjw);
        fprintf(stdout, ")\n");
    //}
    //else {
    //    fprintf(stdout, "Baudrate: %sbps (CiA index %li)\n",
    //        bitrate->index == CANBDR_1000 ? "1000000" :
    //        bitrate->index == -CANBDR_800 ? "800000" :
    //        bitrate->index == -CANBDR_500 ? "500000" :
    //        bitrate->index == -CANBDR_250 ? "250000" :
    //        bitrate->index == -CANBDR_125 ? "125000" :
    //        bitrate->index == -CANBDR_100 ? "100000" :
    //        bitrate->index == -CANBDR_50 ? "50000" :
    //        bitrate->index == -CANBDR_20 ? "20000" :
    //        bitrate->index == -CANBDR_10 ? "10000" : "?", -bitrate->index);
    //}
}

static void sigterm(int signo)
{
    //printf("%s: got signal %d\n", __FILE__, signo);
    running = 0;
    (void)signo;
#ifdef _WIN32
    (void)can_kill(CANKILL_ALL);
#endif
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
