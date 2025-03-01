@echo off
echo /*  -- Do not commit this file -- > .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  CAN Interface API, Version 3 (for PEAK-System PCAN Interfaces) >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  Copyright (c) 2005-2025 Uwe Vogt, UV Software, Berlin (info@uv-software.com) >> .\Sources\build_no.h
echo  *  All rights reserved. >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  This file is part of PCANBasic-Wrapper. >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  PCANBasic-Wrapper is dual-licensed under the BSD 2-Clause "Simplified" License >> .\Sources\build_no.h
echo  *  and under the GNU General Public License v2.0 (or any later version). You can >> .\Sources\build_no.h
echo  *  choose between one of them if you use PCANBasic-Wrapper in whole or in part. >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  (1) BSD 2-Clause "Simplified" License >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  Redistribution and use in source and binary forms, with or without >> .\Sources\build_no.h
echo  *  modification, are permitted provided that the following conditions are met: >> .\Sources\build_no.h
echo  *  1. Redistributions of source code must retain the above copyright notice, this >> .\Sources\build_no.h
echo  *     list of conditions and the following disclaimer. >> .\Sources\build_no.h
echo  *  2. Redistributions in binary form must reproduce the above copyright notice, >> .\Sources\build_no.h
echo  *     this list of conditions and the following disclaimer in the documentation >> .\Sources\build_no.h
echo  *     and/or other materials provided with the distribution. >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  PCANBasic-Wrapper IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" >> .\Sources\build_no.h
echo  *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE >> .\Sources\build_no.h
echo  *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE >> .\Sources\build_no.h
echo  *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE >> .\Sources\build_no.h
echo  *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL >> .\Sources\build_no.h
echo  *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR >> .\Sources\build_no.h
echo  *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER >> .\Sources\build_no.h
echo  *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, >> .\Sources\build_no.h
echo  *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE >> .\Sources\build_no.h
echo  *  OF PCANBasic-Wrapper, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  (2) GNU General Public License v2.0 or later >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  PCANBasic-Wrapper is free software; you can redistribute it and/or modify >> .\Sources\build_no.h
echo  *  it under the terms of the GNU General Public License as published by >> .\Sources\build_no.h
echo  *  the Free Software Foundation; either version 2 of the License, or >> .\Sources\build_no.h
echo  *  (at your option) any later version. >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  PCANBasic-Wrapper is distributed in the hope that it will be useful, >> .\Sources\build_no.h
echo  *  but WITHOUT ANY WARRANTY; without even the implied warranty of >> .\Sources\build_no.h
echo  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the >> .\Sources\build_no.h
echo  *  GNU General Public License for more details. >> .\Sources\build_no.h
echo  * >> .\Sources\build_no.h
echo  *  You should have received a copy of the GNU General Public License along >> .\Sources\build_no.h
echo  *  with PCANBasic-Wrapper; if not, see "https://www.gnu.org/licenses/". >> .\Sources\build_no.h
echo  */ >> .\Sources\build_no.h
echo #ifndef BUILD_NO_H_INCLUDED >> .\Sources\build_no.h
echo #define BUILD_NO_H_INCLUDED >> .\Sources\build_no.h
git log -1 >nul 2>nul
if %errorlevel% == 0 (
    git log -1 --pretty=format:"#define BUILD_NO 0x%%h" > build_no.txt
    type build_no.txt >> .\Sources\build_no.h
    erase /Q build_no.txt >nul 2>nul
    echo  /* git hash */ >> .\Sources\build_no.h
) else (
    echo #define BUILD_NO 0xDEADC0DE >> .\Sources\build_no.h
)
echo #define STRINGIFY(X) #X >> .\Sources\build_no.h
echo #define TOSTRING(X) STRINGIFY(X) >> .\Sources\build_no.h
echo #define SVN_REV_INT (BUILD_NO) >> .\Sources\build_no.h
echo #define SVN_REV_STR TOSTRING(BUILD_NO) >> .\Sources\build_no.h
echo #endif >> .\Sources\build_no.h
