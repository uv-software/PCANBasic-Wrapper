@echo off
echo /*  --  Do not commit this file!  -- > build_no.h
echo  * >> build_no.h
echo  *  project   :  CAN - Controller Area Network >> build_no.h
echo  * >> build_no.h
echo  *  purpose   :  CAN Interface API, Version 3 (PCAN-Basic) >> build_no.h
echo  * >> build_no.h
echo  *  copyright :  (C) 2018-2021, UV Software, Berlin >> build_no.h
echo  * >> build_no.h
echo  *  compiler  :  Microsoft Visual C/C++ Compiler (Version 19.16) >> build_no.h
echo  * >> build_no.h
echo  *  export    :  BUILD_NO >> build_no.h
echo  * >> build_no.h
echo  *  includes  :  (none) >> build_no.h
echo  * >> build_no.h
echo  *  author    :  Uwe Vogt, UV Software >> build_no.h
echo  * >> build_no.h
echo  *  e-mail    :  uwe.vogt@uv-software.de >> build_no.h
echo  * >> build_no.h
echo  */ >> build_no.h
echo #ifndef BUILD_NO_H_INCLUDED >> build_no.h
echo #define BUILD_NO_H_INCLUDED >> build_no.h
rem FIXME: get pseudo build number from git hash
echo #define BUILD_NO 0x0000000 >> build_no.h
echo #define STRINGIFY(X) #X >> build_no.h
echo #define TOSTRING(X) STRINGIFY(X) >> build_no.h
echo #define SVN_REV_INT (BUILD_NO) >> build_no.h
echo #define SVN_REV_STR TOSTRING(BUILD_NO) >> build_no.h
echo #endif >> build_no.h
echo /*  ---------------------------------------------------------------------- >> build_no.h
echo  *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany >> build_no.h
echo  *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903 >> build_no.h
echo  *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/ >> build_no.h
echo  */ >> build_no.h
