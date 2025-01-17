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
v1.15.2 (https://github.com/google/googletest/releases/tag/v1.15.2)

Installation and Usage of GoogleTest
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1. Clone the GoogleTest main branch or download the sources from a tag
   e.g. into 'C:\Projekte\gtest'
2. Copy all header files from 'C:\Projekte\gtest\googletest\include\gtest'
   into '$(PROJROOT)\Tests\GoogleTest\include\gtest'
3. Build static libraries for x64 and x86 with CMake
   note: GoogleMock (aka gmock) does not need to be created for this project.
4. Copy all files from 'C:\Projekte\gtest\googletest\out\build\x64-Debug\lib'
   into $(PROJROOT)\Tests\GoogleTest\build\x64-Debug\lib'
   and all files from 'C:\Projekte\gtest\googletest\out\build\x86-Debug\lib'
   into $(PROJROOT)\Tests\GoogleTest\build\x86-Debug\lib'
 
Important Notes
~~~~~~~~~~~~~~~
- By default the GoogleTest libraries are build with option /MDt
- Since version 1.13.x Googletest requires at least C++14

Last Updated
~~~~~~~~~~~~
January 17, 2025
