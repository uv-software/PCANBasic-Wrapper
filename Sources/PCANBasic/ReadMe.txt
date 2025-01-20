===============================================================================
ReadMe.txt

PCAN-Basic V4.10.0.964
Copyright (c) 2024 PEAK-System Technik GmbH Darmstadt, Germany
All rights reserved.
===============================================================================

Maximize the Notepad Window to read this file more easily.


Contents:
---------
  * Introduction
  * System Requirements
  * Rights to use these files
  * Contents of this directory
  * Installation of PCAN hardware
  * How to contact PEAK-System Technik GmbH
  * LIFE SUPPORT APPLIANCES


Introduction
------------
The PCAN system of the company PEAK-System Technik GmbH consists of a
collection of Windows Device Drivers. These allow the Real-time connection of
Windows applications to all CAN busses that are physically connected to the PC
via a PCAN hardware.

PCAN-Basic is a simple programming interface to the PCAN system. Via one
Interface DLL it is possible to connect own applications to the Device drivers
and the PCAN hardware, to communicate with the CAN busses.

The provided drivers, the PCAN-Basic API, and the PCAN-View CAN bus Monitor
software are the feature-reduced versions of the larger software packages
PCAN-Developer or PCAN-Explorer. These can be aquired separately.


Rights to use these files
-------------------------
Please read the End User License Agreement of the company PEAK-System Technik GmbH at:
www.peak-system.com/quick/eula

PEAK-System Technik GmbH grants the right to the customer to use the files in
this software package as long as this is done in connection with original
hardware by PEAK-System or OEM hardware coming from PEAK-System. It is NOT
allowed to use any of these files (even not parts) with third-party hardware.

If you are not sure whether you have acquired an appropriate license with the
used hardware, please contact our technical support team (support@peak-system.com).


System Requirements
-------------------
- Operating systems: Windows 11 (x64/ARM64), Windows 10 (x64)


Contents of this directory
--------------------------
ReadMe.txt
    This text file.

LiesMich.txt
    This text file in German language.

PCANBasic_enu.chm
    The PCAN-Basic documentation in English.

PCANBasic_deu.chm
    The PCAN-Basic documentation in German.

PCAN-Parameter_Documentation.pdf
    Additional documentation about PCAN-Basic Get/Set parameters in English.

\Include
    - The Header files for different programming languages and development
      environments.

\x86
    - Contains the 32-bit (x86) interface DLL and an x86 demo application (exe).

    \BB_LIB
        x86 LIB file for C++ Builder.

    \VC_LIB
        x86 LIB file for Visual C/C++.

\x64
    - Contains the 64-bit (x64) interface DLL and an x64 demo application (exe).

    \VC_LIB
        x64 LIB file for Visual C/C++.

\ARM64
    - Contains the ARM64 interface DLL and an ARM64 demo application (exe).

    \VC_LIB
        ARM64 LIB file for Visual C/C++

\Samples
    - Contains example files that demonstrate the use of the PCAN-Basic API in
      different programming languages and development environments.
    
\NuGet
    - Contains the PCAN-Basic.NET assembly NuGet package.


Installation of PCAN hardware
-----------------------------
For information on installing PCAN hardware, please refer to the corresponding
hardware user manual. These manuals are accessible online at: 
www.peak-system.com/quick/Documentation.

The target system requires to have the PCANBasic.dll library installed in order 
to be able to run applications that use this API. There are two ways to get this 
library installed on a system:
   1. Via "Device Driver Setup" (recommended): The same application used for device
      driver installation can also install this library. It can be downloaded using
      the following link: www.peak-system.com/quick/DrvSetup.
   2. Copying the library manually: Copy the library files contained in this package 
      to the target system as follow:

      For x64 Windows systems:
      \x86\PCANBasic.dll --> Windows\SysWOW64
      \x64\PCANBasic.dll --> Windows\System32
      
      For ARM64 Windows systems:
      \x86\PCANBasic.dll   --> Windows\SysWOW64
      \x64\PCANBasic.dll   --> Application folder (x64 applications)
      \ARM64\PCANBasic.dll --> Application folder (ARM64 applications)
      
      
Installation of the NuGet Package 
---------------------------------
Performing an online or offline installation of the NuGet package for Microsoft
Visual Studio or Visual Studio Code is described in the documentation of the 
PCAN-Basic.NET Assembly at https://docs.peak-system.com/API/PCAN-Basic.Net/, 
in chapter "Introduction\How-To's and Q&A's".


How to contact PEAK-System Technik GmbH
---------------------------------------
If you have any questions concerning the installation of PCAN hardware, or
require information about other PEAK CAN products, then please contact us:

PEAK-System Technik GmbH
Darmstadt, Germany

Tel. +49 6151 / 8173-20
FAX  +49 6151 / 8173-29

support@peak-system.com
http://www.peak-system.com


LIFE SUPPORT APPLIANCES
-----------------------
These products are not designed for use in life support appliances, devices,
or systems where malfunction of these products can reasonably be expected to
result in personal injury. PEAK-System customers using or selling these
products for use in such applications do so at their own risk and agree to
fully indemnify PEAK-System for any damages resulting from such improper use
or sale.
