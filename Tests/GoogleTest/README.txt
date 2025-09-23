Google Test Framework (GoogleTest) for CAN API V3 C++ Testing
=============================================================

GoogleTest (aka gtest) is a unit testing library for the C++ programming language,
released under the BSD 3-clause license and based on the xUnit architecture.

CAN API V3 is a wrapper specification to have a multi-vendor, cross-platform CAN API.
GoogleTest is used for testing of CAN API V3 C++ wrapper implementations.

Source Code Repositiory of GoogleTest
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
URL: https://github.com/google/googletest

Current Version used by CAN API V3 C++ Testing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
v1.17.0 (https://github.com/google/googletest/releases/tag/v1.17.0)

Installation and Usage of GoogleTest
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1. Clone the GoogleTest main branch or download the sources from a tag
   e.g. into 'D:\Projekte\Testing\googletest' ($(GTESTDIR))
2. Copy all header files from '$(GTESTDIR)\googletest\include\gtest'
   into '$(PROJROOT)\Tests\GoogleTest\include\gtest'
3. Build static libraries for amd64 (_WIN64) with CMake
   note: GoogleMock (aka gmock) does not need to be created for this project.
4. Copy all files from '$(GTESTDIR)\build\lib\Debug'
   into $(PROJROOT)\Tests\GoogleTest\Windows\lib\Debug'
*) Do not build for x86 (_WIN32) anymore!
 
Important Notes
~~~~~~~~~~~~~~~
- Since version 1.17.0 Googletest requires at least C++17 (with option /MDd)
- Since version 1.13.x Googletest requires at least C++14 (with option /MDt)

Last Updated
~~~~~~~~~~~~
September 22, 2025
