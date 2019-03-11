/*  -- $HeadURL: https://svn.uv-software.net/projects/uv-software/CAN/I386/DRV/trunk/API/misc/printmsg.c $ --
 *
 *  project   :  CAN - Controller Area Network
 *
 *  purpose   :  Print CAN Messages (Monitor)
 *
 *  copyright :  (C) 2019, UV Software, Berlin
 *
 *  compiler  :  Microsoft Visual C/C++ Compiler
 *               Apple LLVM version (clang)
 *
 *  export    :  (see header file)
 *
 *  includes  :  printmsg.h
 *
 *  author    :  Uwe Vogt, UV Software
 *
 *  e-mail    :  uwe.vogt@uv-software.de
 *
 *
 *  -----------  description  --------------------------------------------
 */
/** @file        printmsg.c
 *
 *  @brief       Print CAN Messages (Monitor)
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 576 $
 *
 *  @addtogroup  print_msg
 *  @{
 */

/*  -----------  includes  -----------------------------------------------
 */

#include "printmsg.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <windows.h>
#endif

/*  -----------  options  ------------------------------------------------
 */

#ifndef MSG_PRINT_USEC
#define MSG_PRINT_USEC  0
#endif

/*  -----------  defines  ------------------------------------------------
 */


/*  -----------  types  --------------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  functions  ----------------------------------------------
 */

void msg_print_id(FILE *stream, unsigned long id, int rtr, int ext, unsigned char dlc, int mode)
{
    switch (mode) {
    case MSG_MODE_DEC:
        fprintf(stream, "%-4lu ", id);
        break;
    case MSG_MODE_OCT:
        fprintf(stream, "\\%04lo ", id);
        break;
    case MSG_MODE_HEX:
    default:
        fprintf(stream, "%03lX ", id);
        break;
    }
    fprintf(stream, "%c%c [%u]\t",
        ext ? 'X' : ' ',
        rtr ? 'R' : ' ',
        dlc);
}

void msg_print_id_fd(FILE *stream, unsigned long id, int rtr, int ext, int fdf, int brs, int esi, int len, int mode)
{
    switch (mode) {
    case MSG_MODE_DEC:
        fprintf(stream, "%-4lu ", id);
        break;
    case MSG_MODE_OCT:
        fprintf(stream, "\\%04lo ", id);
        break;
    case MSG_MODE_HEX:
    default:
        fprintf(stream, "%03lX ", id);
        break;
    }
    fprintf(stream, "%c%c%c%c%c [%u]\t",
        ext ? 'X' : ' ',
        rtr ? 'R' : ' ',
        fdf ? 'F' : ' ',
        brs ? 'B' : ' ',
        esi ? 'E' : ' ',
        len);
}

void msg_print_data(FILE *stream, unsigned char data, int last, int mode)
{
    switch (mode) {
    case MSG_MODE_DEC:
        if (!last)
            fprintf(stream, "%-3u ", data);
        else
            fprintf(stream, "%-3u", data);
        break;
    case MSG_MODE_OCT:
        if (!last)
            fprintf(stream, "\\%03o ", data);
        else
            fprintf(stream, "\\%03o", data);
        break;
    case MSG_MODE_HEX:
    default:
        if (!last)
            fprintf(stream, "%02X ", data);
        else
            fprintf(stream, "%02X", data);
        break;
    }
}

void msg_print_space(FILE *stream, int last, int mode)
{
    switch (mode) {
    case MSG_MODE_DEC:
        if (!last)
            fprintf(stream, "    ");
        else
            fprintf(stream, "   ");
        break;
    case MSG_MODE_OCT:
        if (!last)
            fprintf(stream, "     ");
        else
            fprintf(stream, "    ");
        break;
    case MSG_MODE_HEX:
    default:
        if (!last)
            fprintf(stream, "   ");
        else
            fprintf(stream, "  ");
        break;
    }
}

void msg_print_ascii(FILE *stream, unsigned char data, int mode)
{
    if (mode == MSG_ASCII_ON) {
        if (isprint(data))
            fprintf(stream, "%c", data);
        else
            fprintf(stream, "?");
    }
}
void msg_print_time(FILE *stream, struct msg_timestamp *timestamp, int mode)
{
    static struct msg_timestamp laststamp = { 0, 0 };
    struct timeval difftime;
    struct tm tm; time_t t;  char timestring[25];

    switch (mode) {
    case MSG_TIME_REL:
    case MSG_TIME_ZERO:
        if (laststamp.tv_sec == 0)  /* first init */
            laststamp = *timestamp;
        difftime.tv_sec = timestamp->tv_sec - laststamp.tv_sec;
        difftime.tv_usec = timestamp->tv_usec - laststamp.tv_usec;
        if (difftime.tv_usec < 0) {
            difftime.tv_sec -= 1;
            difftime.tv_usec += 1000000;
        }
        if (difftime.tv_sec < 0) {
            difftime.tv_sec = 0;
            difftime.tv_usec = 0;
        }
        t = (time_t)difftime.tv_sec;
        tm = *gmtime(&t);
        strftime(timestring, 24, "%H:%M:%S", &tm);
#if (MSG_PRINT_USEC == 0)
        fprintf(stream, "%s.%04li\t", timestring, difftime.tv_usec / 100L);
#else
        fprintf(stream, "%s.%06li\t", timestring, difftime.tv_usec);
#endif
        if (mode == MSG_TIME_REL)
            laststamp = *timestamp; /* update for delta calculation */
        break;
    case MSG_TIME_ABS:
    default:
        if (timestamp->tv_sec < 0)
            timestamp->tv_sec = 0;
        if (timestamp->tv_usec < 0)
            timestamp->tv_usec = 0;

        t = (time_t)timestamp->tv_sec;
        tm = *localtime(&t);
        strftime(timestring, 24, "%H:%M:%S", &tm);
#if (MSG_PRINT_USEC == 0)
        fprintf(stream, "%s.%04li\t", timestring, timestamp->tv_usec / 100L);
#else
        fprintf(stream, "%s.%04li\t", timestring, timestamp->tv_usec);
#endif
        break;
    }
}


/*  -----------  local functions  ----------------------------------------
 */


/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
 