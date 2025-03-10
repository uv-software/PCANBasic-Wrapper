/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-2.0-or-later */
/*
 *  CAN Interface API, Version 3 (Message Formatter)
 *
 *  Copyright (c) 2019-2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v2.0 (or any later version). You can
 *  choose between one of them if you use CAN API V3 in whole or in part.
 *
 *  (1) BSD 2-Clause "Simplified" License
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  CAN API V3 IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF CAN API V3, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  (2) GNU General Public License v2.0 or later
 *
 *  CAN API V3 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CAN API V3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with CAN API V3; if not, see <https://www.gnu.org/licenses/>.
 */
/** @file        can_msg.c
 *
 *  @brief       CAN Message Formatter
 *
 *  @author      $Author: quaoar $
 *
 *  @version     $Rev: 1467 $
 *
 *  @addtogroup  can_msg
 *  @{
 */


/*  -----------  includes  -----------------------------------------------
 */

#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include "can_msg.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#include <ctype.h>
#include <time.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/time.h>
#else
#include <windows.h>
#endif

/*  -----------  defines  ------------------------------------------------
 */

#ifndef DLC2LEN
#define DLC2LEN(x)  dlc_table[((x) < 16U) ? (x) : 15U]
#endif
#ifndef LEN2DLC
#define LEN2DLC(x)  ((x) > 48U) ? 0xFU : \
                    ((x) > 32U) ? 0xEU : \
                    ((x) > 24U) ? 0xDU : \
                    ((x) > 20U) ? 0xCU : \
                    ((x) > 16U) ? 0xBU : \
                    ((x) > 12U) ? 0xAU : \
                    ((x) > 8U) ?  0x9U : (x)
#endif
#define IS_EOL(c)   (((c) == '\0') || ((c) == '\n') || ((c) == '\r'))
#define IS_HEX(c)   (isdigit(c) || (((c) >= 'A') && ((c) <= 'F')) || (((c) >= 'a') && ((c) <= 'f')))
#define IS_DIGIT(c) (isdigit(c))
#define IS_SPACE(c) (isspace(c))
#define CHR2DEC(c)  ((c) >= '0' && (c) <= '9' ? (c) - '0' : 0xFF)
#define CHR2HEX(c)  ((c) >= '0' && (c) <= '9' ? (c) - '0' : \
                     (c) >= 'a' && (c) <= 'f' ? (c) - 'a' + 10 : \
                     (c) >= 'A' && (c) <= 'F' ? (c) - 'A' + 10 : 0xFF)
#define CANFD_BRS   0x01 /* bit rate switch (second bitrate for payload data) */
#define CANFD_ESI   0x02 /* error state indicator of the transmitting node */
#define CANFD_FDF   0x04 /* mark CAN FD for dual use of struct canfd_frame */


/*  -----------  types  --------------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

static void format_time(char *string, const msg_message_t *message);
static void format_id(char *string, const msg_message_t *message);
static void format_dlc(char *string, const msg_message_t *message);
static void format_data(char *string, const msg_message_t *message, int ascii, int indent);
static void format_ascii(char *string, const msg_message_t *message);
static void format_data_byte(char *string, unsigned char data);
static void format_data_ascii(char *string, unsigned char data);
static void format_fill_byte(char *string);


/*  -----------  variables  ----------------------------------------------
 */

static struct {                         /* format option: */
    msg_fmt_timestamp_t  time_stamp;    /*   time-stamp {ZERO, ABS, REL} */
    msg_fmt_option_t     time_usec;     /*   time-stamp in usec {OFF, ON} */
    msg_fmt_time_t       time_format;   /*   time format {TIME, SEC, DJD} */
    msg_fmt_number_t     id;            /*   identifier {HEX, DEC, OCT, BIN} */
    msg_fmt_option_t     id_xtd;        /*   extended identifier {OFF, ON} */
    msg_fmt_number_t     dlc;           /*   DLC/length {HEX, DEC, OCT, BIN} */
    msg_fmt_canfd_t      dlc_format;    /*   CAN FD format {DLC, LENGTH} */
    int                  dlc_brackets;  /*   DLC in brackets {'\0', '(', '['} */
    msg_fmt_option_t     flags;         /*   message flags {ON, OFF} */
    msg_fmt_number_t     data;          /*   message data {HEX, DEC, OCT, BIN} */
    msg_fmt_option_t     ascii;         /*   data as ASCII {ON, OFF} */
    int                  ascii_subst;   /*   substitute for non-printables */
    msg_fmt_option_t     channel;       /*   message source {OFF, ON} */
    msg_fmt_option_t     counter;       /*   message counter {ON, OFF} */
    msg_fmt_separator_t  separator;     /*   separator {SPACES, TABS} */
    msg_fmt_wraparound_t wraparound;    /*   wraparound {NO, 8, 16, 32, 64} */
    msg_fmt_option_t     end_of_line;   /*   end-of-line character {ON, OFF} */
    char                 rx_prompt[6+1];/*   prompt for received messages */
    char                 tx_prompt[6+1];/*   prompt for sent messages */
}   msg_option = {
                        .time_stamp = MSG_FMT_TIMESTAMP_ZERO,
                        .time_usec = MSG_FMT_OPTION_OFF,
                        .time_format = MSG_FMT_TIME_SEC,
                        .id = MSG_FMT_NUMBER_HEX,
                        .id_xtd = MSG_FMT_OPTION_OFF,
                        .dlc = MSG_FMT_NUMBER_DEC,
                        .dlc_format = MSG_FMT_CANFD_LENGTH,
                        .dlc_brackets = '\0',
                        .flags = MSG_FMT_OPTION_ON,
                        .data = MSG_FMT_NUMBER_HEX,
                        .ascii = MSG_FMT_OPTION_ON,
                        .ascii_subst = '.',
                        .channel = MSG_FMT_OPTION_OFF,
                        .counter = MSG_FMT_OPTION_ON,
                        .separator = MSG_FMT_SEPARATOR_SPACES,
                        .wraparound = MSG_FMT_WRAPAROUND_NO,
                        .end_of_line = MSG_FMT_OPTION_OFF,
                        .rx_prompt = "",
                        .tx_prompt = ""
};
static msg_format_t msg_format = MSG_FORMAT_DEFAULT;
static char msg_string[MSG_STRING_LENGTH] = "";
static const unsigned char dlc_table[16] = {
    0U,1U,2U,3U,4U,5U,6U,7U,8U,12U,16U,20U,24U,32U,48U,64U
};


/*  -----------  functions  ----------------------------------------------
 */

char *msg_format_message(const msg_message_t *message, msg_direction_t direction,
                               msg_counter_t counter, msg_channel_t channel)
{
    char tmp_string[MSG_STRING_LENGTH];

    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* prompt (optional) */
        if (strlen(msg_option.tx_prompt) && (direction == MSG_TX_MESSAGE)) {
            strcat(msg_string, msg_option.tx_prompt);
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : " ");
        }
        else if (strlen(msg_option.rx_prompt)) { /* defaults to MSG_DIRECTION_RX_MSG */
            strcat(msg_string, msg_option.rx_prompt);
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : " ");
        }
        /* counter (optional) */
        if ((msg_option.counter != MSG_FMT_OPTION_OFF) && ((msg_option.separator == MSG_FMT_SEPARATOR_TABS))) {
            sprintf(tmp_string, "%" PRIu64 "\t", counter);
            strcat(msg_string, tmp_string);
        }
        else if (msg_option.counter != MSG_FMT_OPTION_OFF) { /* defaults to MSG_FMT_SEPARATOR_SPACES */
            sprintf(tmp_string, "%-7" PRIu64 "  ", counter);
            strcat(msg_string, tmp_string);
        }
        /* time-stamp (abs/rel/zero) (hhmmss/sec/DJD).(msec/usec) */
        format_time(tmp_string, message);
        strcat(msg_string, tmp_string);
        strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : "  ");

        /* channel (optional) */
        if ((msg_option.channel != MSG_FMT_OPTION_OFF) && (msg_option.separator == MSG_FMT_SEPARATOR_TABS)) {
            sprintf(tmp_string, "%i\t", channel);
            strcat(msg_string, tmp_string);
        }
        else if (msg_option.channel != MSG_FMT_OPTION_OFF) { /* defaults to MSG_FMT_SEPARATOR_SPACES */
            sprintf(tmp_string, "%-2i  ", channel);
            strcat(msg_string, tmp_string);
        }
        /* identifier (hex/dec/oct) */
        format_id(tmp_string, message);
        strcat(msg_string, tmp_string);
        strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : "  ");

        /* flags (optional) */
        if (msg_option.flags != MSG_FMT_OPTION_OFF) {
#if (OPTION_CAN_2_0_ONLY == 0)
            if (!message->sts) {
                strcat(msg_string, message->xtd ? "X" : "S");
                strcat(msg_string, message->fdf ? "F" : "-");
                strcat(msg_string, message->brs ? "B" : "-");
                strcat(msg_string, message->esi ? "E" : "-");
                strcat(msg_string, message->rtr ? "R" : "-");
            }
            else {
                strcat(msg_string, "Error");
            }
#else
            if (!message->sts) {
                strcat(msg_string, message->xtd ? "X" : "S");
                strcat(msg_string, message->rtr ? "R" : "-");
            }
            else {
                strcat(msg_string, "E!");
            }
#endif
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : " ");  /* only one space! */
        }
        /* dlc/length (hex/dec/oct) */
        format_dlc(tmp_string, message);
        strcat(msg_string, tmp_string);

        /* data (hex/dec/oct) plus ascii (optional) */
        if (message->dlc && !message->rtr) {
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : "  ");
            format_data(tmp_string, message, (msg_option.ascii == MSG_FMT_OPTION_OFF) ? 0 : 1, (int)strlen(msg_string));
            strcat(msg_string, tmp_string);
        }
        /* end-of-line (optional) */
        if (msg_option.end_of_line) {
            strcat(msg_string, "\n");
        }
    }
    return msg_string;
}

char *msg_format_time(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* time-stamp (abs/rel/zero) (hhmmss/sec/DJD).(msec/usec) */
        format_time(msg_string, message);
    }
    return msg_string;
}

char *msg_format_id(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* identifier (hex/dec/oct) */
        format_id(msg_string, message);
    }
    return msg_string;
}

char *msg_format_flags(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
#if (OPTION_CAN_2_0_ONLY == 0)
        if (!message->sts) {
            strcat(msg_string, message->xtd ? "X" : "S");
            strcat(msg_string, message->fdf ? "F" : "-");
            strcat(msg_string, message->brs ? "B" : "-");
            strcat(msg_string, message->esi ? "E" : "-");
            strcat(msg_string, message->rtr ? "R" : "-");
        }
        else {
            strcat(msg_string, "Error");
        }
#else
        if (!message->sts) {
            strcat(msg_string, message->xtd ? "X" : "S");
            strcat(msg_string, message->rtr ? "R" : "-");
        }
        else {
            strcat(msg_string, "E!");
        }
#endif
    }
    return msg_string;
}

char *msg_format_dlc(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* dlc/length (hex/dec/oct) */
        format_dlc(msg_string, message);
    }
    return msg_string;
}

char *msg_format_data(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* data (hex/dec/oct) */
        if (message->dlc) {
            format_data(msg_string, message, 0, 0);
        }
    }
    return msg_string;
}

char *msg_format_ascii(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* data (hex/dec/oct) */
        if (message->dlc) {
            format_ascii(msg_string, message);
        }
    }
    return msg_string;
}


/* message output format {DEFAULT, ...} */
int msg_set_format(msg_format_t format)
{
    int rc = 1;

    switch (format) {
    case MSG_FORMAT_DEFAULT:
        msg_format = format;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: time-stamp {ZERO, ABS, REL} */
int  msg_set_fmt_time_stamp(msg_fmt_timestamp_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_TIMESTAMP_ZERO:
    case MSG_FMT_TIMESTAMP_ABSOLUTE:
    case MSG_FMT_TIMESTAMP_RELATIVE:
        msg_option.time_stamp = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: time-stamp in usec {ON, OFF} */
int msg_set_fmt_time_usec(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.time_usec = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: time format {TIME, SEC, DJD} */
int msg_set_fmt_time_format(msg_fmt_time_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_TIME_HHMMSS:
    case MSG_FMT_TIME_SEC:
    case MSG_FMT_TIME_DJD:
        msg_option.time_format = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: identifier {HEX, DEC, OCT, BIN} */
int msg_set_fmt_id(msg_fmt_number_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_NUMBER_HEX:
    case MSG_FMT_NUMBER_DEC:
    case MSG_FMT_NUMBER_OCT:
        msg_option.id = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: extended identifier {ON, OFF} */
int msg_set_fmt_id_xtd(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.id_xtd = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: DLC/length {HEX, DEC, OCT, BIN} */
int msg_set_fmt_dlc(msg_fmt_number_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_NUMBER_HEX:
    case MSG_FMT_NUMBER_DEC:
    case MSG_FMT_NUMBER_OCT:
        msg_option.dlc = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: CAN FD format {DLC, LENGTH} */
int msg_set_fmt_dlc_format(msg_fmt_canfd_t option)
{
    int rc = 1;

    switch (option) {
    case  MSG_FMT_CANFD_DLC:
    case  MSG_FMT_CANFD_LENGTH:
        msg_option.dlc_format = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: DLC in brackets {'\0', '(', '['} */
int msg_set_fmt_dlc_brackets(int option)
{
    int rc = 1;

    switch (option) {
    case '\0':
    case '(':
    case '[':
        msg_option.dlc_brackets = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: message flags {ON, OFF} */
int msg_set_fmt_flags(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.flags = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: message data {HEX, DEC, OCT, BIN} */
int msg_set_fmt_data(msg_fmt_number_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_NUMBER_HEX:
    case MSG_FMT_NUMBER_DEC:
    case MSG_FMT_NUMBER_OCT:
        msg_option.data = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: data as ASCII {ON, OFF} */
int msg_set_fmt_ascii(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.ascii = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: substitute for non-printables */
int msg_set_fmt_ascii_subst(int option)
{
    int rc = 1;

    if (isprint(option))
        msg_option.ascii_subst = option;
    else
        rc = 0;
    return rc;
}

/* formatter option: message source {ON, OFF} */
int msg_set_fmt_channel(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.channel = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: message counter {ON, OFF} */
int msg_set_fmt_counter(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.counter = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: separator {SPACES, TABS} */
int msg_set_fmt_separator(msg_fmt_separator_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_SEPARATOR_SPACES:
    case MSG_FMT_SEPARATOR_TABS:
        msg_option.separator = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: wraparound {NO, 8, 16, 32, 64} */
int msg_set_fmt_wraparound(msg_fmt_wraparound_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_WRAPAROUND_NO:
    case MSG_FMT_WRAPAROUND_8:
    case MSG_FMT_WRAPAROUND_10:
    case MSG_FMT_WRAPAROUND_16:
    case MSG_FMT_WRAPAROUND_32:
    case MSG_FMT_WRAPAROUND_64:
        msg_option.wraparound = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: end-of-line character {ON, OFF} */
int msg_set_fmt_eol(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.end_of_line = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: prompt for received messages */
int msg_set_fmt_rx_prompt(const char *option)
{
    int rc = 1;

    if (strlen(option) <= 6)
        strcpy(msg_option.rx_prompt, option);
    else
        rc = 0;
    return rc;
}

/* formatter option: prompt for sent messages */
int msg_set_fmt_tx_prompt(const char *option)
{
    int rc = 1;

    if (strlen(option) <= 6)
        strcpy(msg_option.tx_prompt, option);
    else
        rc = 0;
    return rc;
}

/* parser: SocketCAN ASCII format (see 'can_utils/cansend') */
int msg_parse(const char *str, msg_message_t *msg, uint32_t *cnt, uint64_t *cyc, int *inc) {
    unsigned long can_id = 0x000U;
    const char *ptr = str;
    char *endptr;
    
    memset(msg, 0, sizeof(msg_message_t));
    *cnt = 1;
    *cyc = 0;
    *inc = 0;

    /* sanity check */
    if (!str || !msg || !cnt || !cyc || !inc) {
        return -1;
    }

    /* parse CAN identifier */
    can_id = strtoul(ptr, &endptr, 16);
    if ((endptr == ptr) || (((endptr - ptr) != 3) && ((endptr - ptr) != 8))) {
        return -1;
    }
    if ((endptr - ptr) == 8) {
        msg->xtd = 1;
    }
    if ((uint32_t)can_id > (uint32_t)(msg->xtd ? CAN_MAX_XTD_ID : CAN_MAX_STD_ID)) {
        return -1;
    }
    msg->id = (uint32_t)can_id;
    ptr = endptr;

    /* parse frame type */
    if (*ptr != '#') {
        return -1;
    }
    ptr++;

    /* parse CAN FD flags if present */
    if (*ptr == '#') {
        ptr++;
        if (!IS_HEX(*ptr)) {
            return -1;
        }
        /* map CAN FD flags */
        char flags = CHR2HEX(*ptr);
        if ((flags & ~(CANFD_BRS | CANFD_ESI)) != CANFD_FDF) {
            return -1;
        }
        msg->fdf = (flags & CANFD_FDF) ? 1 : 0;
        msg->brs = (flags & CANFD_BRS) ? 1 : 0;
        msg->esi = (flags & CANFD_ESI) ? 1 : 0;
        ptr++;
        /* not sure if this is also in linux/can_utils */
        if (*ptr == '.') ptr++;
    }

    /* parse data or remote frame */
    if (*ptr == 'R') {
        msg->rtr = 1;
        ptr++;
        /* parse optional DLC */
        if (IS_DIGIT(*ptr)) {
            msg->dlc = (uint8_t)CHR2DEC(*ptr);
            if (msg->dlc > CAN_MAX_DLC) return -1;
            ptr++;
        }
    } else {
        uint8_t len = 0;
        /* parse data (max. 8 bytes resp. 64 bytes for CAN FD) */
        while (!IS_EOL(*ptr) && IS_HEX(*ptr)) {
            if (len >= (msg->fdf ? CANFD_MAX_LEN : CAN_MAX_LEN)) return -1;
            if (!IS_HEX(*(ptr + 1))) return -1; // Ensure there are two hex digits
            char hex_byte[3] = { *ptr, *(ptr + 1), '\0' }; // Create a string with two hex digits
            msg->data[len++] = (uint8_t)strtol(hex_byte, NULL, 16);
            ptr += 2; // Move to the next pair of hex digits
            if (*ptr == '.') ptr++; // Skip the '.' separator
        }
        if (*ptr == '.') return -1;
        /* convert data length to DLC */
        msg->dlc = (uint8_t)LEN2DLC(len);
    }

    /* parse optional DLC ('9' .. 'F') */
    if (!msg->fdf && (*ptr == '_')) {
        ptr++;
        if (!IS_HEX(*ptr)) return -1;
        if (CHR2DEC(*ptr) <= CAN_MAX_DLC) return -1;
        /* ignore extra DLC */
        ptr++;
    }

    /* parse optional transmission options */
    if ((*ptr == 'x') || (*ptr == 'X') || (*ptr == '*')) {
        ptr++;
        /* number of messages to send */
        if (!IS_DIGIT(*ptr)) return -1;
        long count = strtoul(ptr, &endptr, 10);
        if ((endptr == ptr) || (count <= 0) || (count > UINT32_MAX)) return -1;
        *cnt = (uint32_t)count;
        ptr = endptr;
        /* cycle time (in [ms]) */
        if ((*ptr == 'C') || (*ptr == 'c')) {
            ptr++;
            if (!IS_DIGIT(*ptr)) return -1;
            long msec = strtoul(ptr, &endptr, 10);
            if ((endptr == ptr) || (msec < 0L) || (msec > 60000L)) return -1;
            *cyc = (uint64_t)(msec * 1000);
            ptr = endptr;
        }
        /* cycle time (in [us]) */
        else if ((*ptr == 'U') || (*ptr == 'u')) {
            ptr++;
            if (!IS_DIGIT(*ptr)) return -1;
            long usec = strtoul(ptr, &endptr, 10);
            if ((endptr == ptr) || (usec < 0L) || (usec > 60000000L)) return -1;
            *cyc = (uint64_t)usec;
            ptr = endptr;
        }
        /* upcounting data */
        if (*ptr == '+') {
            if (*(ptr + 1) == '+') {
                *inc = 1;
                ptr += 2;
            } else {
                return -1;
            }
        }
        /* downcounting data */ 
        else if (*ptr == '-') {
            if (*(ptr + 1) == '-') {
                *inc = -1;
                ptr += 2;
            } else {
                return -1;
            }
        }
    }
    /* check end of string */
    if (!IS_EOL(*ptr) && !IS_SPACE(*ptr)) {
        return -1;
    }
    return 0;
}

/*  -----------  local functions  ----------------------------------------
 */

static void format_time(char *string, const msg_message_t *message)
{
    static msg_timestamp_t laststamp = { 0, 0 };
    static int first = 1;

    struct timespec difftime;
    struct tm tm; time_t t;
    char   timestring[25];
    double djd;

    assert(string);
    assert(message);

    switch (msg_option.time_stamp) {
    case MSG_FMT_TIMESTAMP_RELATIVE:
    case MSG_FMT_TIMESTAMP_ZERO:
        if (first) { /* first time-stamp received */
            first = 0;
            laststamp.tv_sec = message->timestamp.tv_sec;
            laststamp.tv_nsec = message->timestamp.tv_nsec;
        }
        difftime.tv_sec = message->timestamp.tv_sec - laststamp.tv_sec;
        difftime.tv_nsec = message->timestamp.tv_nsec - laststamp.tv_nsec;
        if (difftime.tv_nsec < 0) {
            difftime.tv_sec -= 1;
            difftime.tv_nsec += 1000000000;
        }
        if (difftime.tv_sec < 0) { /* FIXME: why shouldn't it be negative? */
            difftime.tv_sec = 0;
            difftime.tv_nsec = 0;
        }
        if (msg_option.time_stamp == MSG_FMT_TIMESTAMP_RELATIVE) { /* update for delta calculation */
            laststamp.tv_sec = message->timestamp.tv_sec;
            laststamp.tv_nsec = message->timestamp.tv_nsec;
        }
        t = (time_t)difftime.tv_sec;
        tm = *gmtime(&t);
        break;
    case MSG_FMT_TIMESTAMP_ABSOLUTE:
    default:
        difftime.tv_sec = message->timestamp.tv_sec;
        difftime.tv_nsec = message->timestamp.tv_nsec;
        t = (time_t)message->timestamp.tv_sec;
        tm = *localtime(&t);
        break;
    }
    switch (msg_option.time_format) {
    case MSG_FMT_TIME_HHMMSS:
        strftime(timestring, 24, "%H:%M:%S", &tm); // TODO: tm > 24h (?)
        if (msg_option.time_usec)
            sprintf(string, "%s.%06li", timestring, (long)difftime.tv_nsec / 1000L);
        else/* resolution is 0.1 milliseconds! */
            sprintf(string, "%s.%04li", timestring, (long)difftime.tv_nsec / 100000L);
        break;
    case MSG_FMT_TIME_DJD:
        if (!msg_option.time_usec)  /* round to milliseconds resolution */
            difftime.tv_nsec = ((difftime.tv_nsec + 500000L) / 1000000L) * 1000000L;
        djd = (double)difftime.tv_sec / (double)86400;
        djd += (double)difftime.tv_nsec / (double)86400000000000;
        if (msg_option.time_usec)
            sprintf(string, "%1.12lf", djd);
        else
            sprintf(string, "%1.9lf", djd);
        break;
    case MSG_FMT_TIME_SEC:
    default:
        if (msg_option.time_usec)
            sprintf(string, "%3li.%06li", (long)difftime.tv_sec, (long)difftime.tv_nsec / 1000L);
        else/* resolution is 0.1 milliseconds! */
            sprintf(string, "%3li.%04li", (long)difftime.tv_sec, (long)difftime.tv_nsec / 100000L);
        break;
    }
}

static void format_id(char *string, const msg_message_t *message)
{
    assert(string);
    assert(message);

    string[0] = '\0';
    switch (msg_option.id) {
    case MSG_FMT_NUMBER_DEC:
        if (!msg_option.id_xtd)
            sprintf(string, "%-4" PRIu32, message->id);
        else
            sprintf(string, "%-9" PRIu32, message->id);
        break;
    case MSG_FMT_NUMBER_OCT:
        if (!msg_option.id_xtd)
            sprintf(string, "%04" PRIo32, message->id);
        else
            sprintf(string, "%010" PRIo32, message->id);
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        if (!msg_option.id_xtd)
            sprintf(string, "%03" PRIX32, message->id);
        else
            sprintf(string, "%08" PRIX32, message->id);
        break;
    }
}

static void format_dlc(char *string, const msg_message_t *message)
{
    assert(string);
    assert(message);

    unsigned char length = (msg_option.dlc_format == MSG_FMT_CANFD_DLC) ? message->dlc : DLC2LEN(message->dlc);
    char pre = '\0', post = '\0';
    int blank = 0;

    string[0] = '\0';
    switch (msg_option.dlc_brackets) {
    case '(': pre = '('; post = ')'; break;
    case '[': pre = '['; post = ']'; break;
    default: break;
    }
    switch (msg_option.dlc) {
    case MSG_FMT_NUMBER_DEC:
        if (pre && post)
            sprintf(string, "%c%u%c", pre, length, post);
        else
            sprintf(string, "%u", length);
        blank = length >= 10 ? 0 : 1;
        break;
    case MSG_FMT_NUMBER_OCT:
        if (pre && post)
            sprintf(string, "%c%02o%c", pre, length, post);
        else
            sprintf(string, "%02o", length);
        blank = length >= 64 ? 0 : 1;
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        if (pre && post)
            sprintf(string, "%c%X%c", pre, length, post);
        else
            sprintf(string, "%X", length);
        break;
    }
#if (OPTION_CAN_2_0_ONLY == 0)
    if (message->fdf && blank)
        strcat(string, " ");
#else
    (void)blank;  /* to avoid compiler warnings */
#endif
}

static void format_data(char *string, const msg_message_t *message, int ascii, int indent)
{
    assert(string);
    assert(message);

    int length = DLC2LEN(message->dlc);
    int i, j, col, wraparound;
    char datastring[8];

    string[0] = '\0';
#if (OPTION_CAN_2_0_ONLY == 0)
    if (msg_option.wraparound == MSG_FMT_WRAPAROUND_NO)
        wraparound = message->fdf ? (int)MSG_FMT_WRAPAROUND_64 : (int)MSG_FMT_WRAPAROUND_8;
    else
        wraparound = (int)msg_option.wraparound;
#else
    wraparound = (int)MSG_FMT_WRAPAROUND_8;
#endif
    for (i = 0, j = 0, col = 0; i < length; i++) {
        format_data_byte(datastring, message->data[i]);
        strcat(string, datastring);
        if ((i + 1) < length) {
            if ((col + 1) == wraparound) {
                if (ascii) {
                    strcat(string, msg_option.separator == MSG_FMT_SEPARATOR_TABS ? "\t" : "  ");
                    for (col = 0; col < (int)msg_option.wraparound; j++, col++) {
                        format_data_ascii(datastring, message->data[j]);
                        strcat(string, datastring);
                    }
                }
                strcat(string, "\n");
                if (msg_option.separator != MSG_FMT_SEPARATOR_TABS) {
                    for (col = 0; col < indent; col++)
                        strcat(string, " ");
                }
                else
                    strcat(string, "\t");
                col = 0;
            }
            else {
                strcat(string, " ");
                col++;
            }
        }
        else
            col++;
    }
    if (ascii) {
        if ((col < wraparound) && (i != 0)) {
            strcat(string, " ");
            for (; col < wraparound; col++) {
                format_fill_byte(datastring);
                strcat(string, datastring);
                if ((col + 1) != wraparound)
                    strcat(string, " ");
            }
        }
        strcat(string, msg_option.separator == MSG_FMT_SEPARATOR_TABS ? "\t" : "  ");
        for (; j < length; j++) {
            format_data_ascii(datastring, message->data[j]);
            strcat(string, datastring);
        }
    }
}

static void format_ascii(char *string, const msg_message_t *message)
{
    assert(string);
    assert(message);

    int length = DLC2LEN(message->dlc);
    int i, col, wraparound;
    char datastring[8];

    string[0] = '\0';
#if (OPTION_CAN_2_0_ONLY == 0)
    if (msg_option.wraparound == MSG_FMT_WRAPAROUND_NO)
        wraparound = message->fdf ? (int)MSG_FMT_WRAPAROUND_64 : (int)MSG_FMT_WRAPAROUND_8;
    else
        wraparound = (int)msg_option.wraparound;
#else
    wraparound = (int)MSG_FMT_WRAPAROUND_8;
#endif
    for (i = 0, col = 0; i < length; i++) {
        format_data_ascii(datastring, message->data[i]);
        strcat(string, datastring);
        if ((i + 1) < length) {
            if ((col + 1) == wraparound) {
                strcat(string, "\n");
                col = 0;
            }
            else {
                strcat(string, " ");
                col++;
            }
        }
    }
}

static void format_data_byte(char *string, unsigned char data)
{
    assert(string);

    switch (msg_option.data) {
    case MSG_FMT_NUMBER_DEC:
        sprintf(string, "%-3u", data);
        break;
    case MSG_FMT_NUMBER_OCT:
        sprintf(string, "%03o", data);
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        sprintf(string, "%02X", data);
        break;
    }
}

static void format_fill_byte(char *string)
{
    assert(string);

    switch (msg_option.data) {
    case MSG_FMT_NUMBER_DEC:
        sprintf(string, "   ");
        break;
    case MSG_FMT_NUMBER_OCT:
        sprintf(string, "   ");
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        sprintf(string, "  ");
        break;
    }
}

static void format_data_ascii(char *string, unsigned char data)
{
    assert(string);

    sprintf(string, "%c", isprint((int)data) ? (char)data : (char)msg_option.ascii_subst);
}

/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
