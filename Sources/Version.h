/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-2.0-or-later */
/*
 *  CAN Interface API, Version 3 (for PEAK-System PCAN Interfaces)
 *
 *  Copyright (c) 2005-2012 Uwe Vogt, UV Software, Friedrichshafen
 *  Copyright (c) 2013-2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This file is part of PCANBasic-Wrapper.
 *
 *  PCANBasic-Wrapper is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v2.0 (or any later version). You can
 *  choose between one of them if you use PCANBasic-Wrapper in whole or in part.
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
 *  PCANBasic-Wrapper IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF PCANBasic-Wrapper, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  (2) GNU General Public License v2.0 or later
 *
 *  PCANBasic-Wrapper is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  PCANBasic-Wrapper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with PCANBasic-Wrapper; if not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED
#include "build_no.h"
#ifdef _MSC_VER
#define VERSION_MAJOR    0
#define VERSION_MINOR    6
#define VERSION_PATCH    0
#else
#define VERSION_MAJOR    0
#define VERSION_MINOR    4
#define VERSION_PATCH    0
#endif
#define VERSION_BUILD    BUILD_NO
#if (VERSION_PATCH == 0)
#define VERSION_STRING   TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) " (" TOSTRING(BUILD_NO) ")"
#else
#define VERSION_STRING   TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH) " (" TOSTRING(BUILD_NO) ")"
#endif
#endif
