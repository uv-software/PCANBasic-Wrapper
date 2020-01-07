@echo off
echo /*  --  Do not commit this file!  -- > can_vers.h
echo  * >> can_vers.h
echo  *  project   :  CAN - Controller Area Network >> can_vers.h
echo  * >> can_vers.h
echo  *  purpose   :  CAN Interface API, Version 3 (PCAN-Basic) >> can_vers.h
echo  * >> can_vers.h
echo  *  copyright :  (C) 2018-2020, UV Software, Berlin >> can_vers.h
echo  * >> can_vers.h
echo  *  compiler  :  Microsoft Visual C/C++ Compiler (Version 19.16) >> can_vers.h
echo  * >> can_vers.h
echo  *  export    :  BUILD_NO >> can_vers.h
echo  * >> can_vers.h
echo  *  includes  :  (none) >> can_vers.h
echo  * >> can_vers.h
echo  *  author    :  Uwe Vogt, UV Software >> can_vers.h
echo  * >> can_vers.h
echo  *  e-mail    :  uwe.vogt@uv-software.de >> can_vers.h
echo  * >> can_vers.h
echo  */ >> can_vers.h
echo #ifndef CAN_VERS_H_INCLUDED >> can_vers.h
echo #define CAN_VERS_H_INCLUDED >> can_vers.h
echo #define BUILD_NO \>> can_vers.h
svn info --show-item last-changed-revision can_api.c >> can_vers.h
echo #define STRINGIFY(X) #X >> can_vers.h
echo #define TOSTRING(X) STRINGIFY(X) >> can_vers.h
echo #define SVN_REV_INT (BUILD_NO) >> can_vers.h
echo #define SVN_REV_STR TOSTRING(BUILD_NO) >> can_vers.h
echo #endif >> can_vers.h
echo /*  ---------------------------------------------------------------------- >> can_vers.h
echo  *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany >> can_vers.h
echo  *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903 >> can_vers.h
echo  *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/ >> can_vers.h
echo  */ >> can_vers.h
