### Wrapper Library for PEAK PCAN&reg; Interfaces (Windows&reg;)

_Copyright &copy; 2005-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.de)_
_All rights reserved._

# Deployment

## Release Candidate

### Precondition

- **_All changes are made exclusively on a feature branch!_**

### Preparation

1. Update the CAN API V3 sources in `$(PROJROOT)\Sources\CANAPI` from SVN repo
   when required and commit them with commit comment:
  - `Update CAN API V3 sources to rev. `_nnn_ \
    `- `_list of major changes (optional)_
2. Update the PCANBasic DLL in `$(PROJROOT)\Sources\PCANBasic` from PEAK´s website
   when required and commit them with commit comment:
  - `Update PEAK´s PCANBasic DLL (version `_n_`.`_nn_`)` \
    `- `_list of major changes (optional)_
2. Check and update the version and date information in the following files:
  - `$(PROJROOT)\Sources\PeakCAN.h`
  - `$(PROJROOT)\Sources\PeakCAN.cpp`
  - `$(PROJROOT)\Sources\Wrapper\can_api.cpp`
  - `$(PROJROOT)\Libraries\CANAPI\Resource.rc`
  - `$(PROJROOT)\Libraries\PeakCAN\Resource.rc`
  - `$(PROJROOT)\Utilities\can_moni\Sources\main.cpp`
  - `$(PROJROOT)\Utilities\can_test\Sources\main.cpp`

### Procedure

1. Check the working directory for uncommitted changes.
  - _**There should not be any uncommitted change.**_
  - _If there are uncommitted changes then commit them or revert them._
2. Open the trial program with Visual Studio and run a code analysis.
  - _**There should not be any serious finding.**_
  - _If there are findings then fix them or create an issue in the repo._
3. Run `build_86.bat` and `build_64.bat` in the project root directory.
  - _**There should be absolute no compiler or linker error!**_
  - _If there are compiler or linker warnings then think twice._
4. Try out the trial program with different options.
  - _**There should be no crash, hangup, or any other error.**_
  - _If there is an error then fix it or create an issue in the repo._
5. Try out the utilities with different options.
  - _**There should be no crash, hangup, or any other error.**_
  - _If there is an error then fix it or create an issue in the repo._
6. Build and try out the examples (repair them when necessary);
  - `$(PROJROOT)\Examples\C++`
  - `$(PROJROOT)\Examples\Python`

### Pull Request

1. Update the `README.md` (e.g. development environment, supported devices, etc.).
2. Push the feature branch onto the remote repo.
3. Create a pull request and name it somehow like '**Release Candidate #**_n_'.
4. Review the changes and merge the feature branch into the default branch.

## Release Tag

### Preparation

1. Pull or clone the default branch on all development systems.
2. Double check all version numbers again (see above).
3. Run the batch file in the project root directory:
  - `C:\Users\haumea>cd C:\Projects\CAN\Drivers\PeakCAN`
  - `C:\Projects\CAN\Drivers\PeakCAN>build_86.bat`
  - `C:\Projects\CAN\Drivers\PeakCAN>build_64.bat`
  - `C:\Projects\CAN\Drivers\PeakCAN>install_86.bat`
  - `C:\Projects\CAN\Drivers\PeakCAN>install_64.bat`
4. Update and build the CAN API V3 Loader Library:
  - `C:\Users\haumea>cd C:\Projects\CAN\Library`
  - `C:\Projects\CAN\Library>build_86.bat`
  - `C:\Projects\CAN\Library>install_86.bat`
5. Update and build the CAN API V3 GoogleTest:
  - `C:\Users\haumea>cd C:\Projects\CAN\Library\Testing\Windows`
  - `C:\Projects\CAN\Library\Testing\Windows>build_86.bat`
6. Run the CAN API V3 GoogleTest with the CAN Leaf Pro device:
  - `C:\Projects\CAN\Library\Testing\Windows>Debug\can_testing --can_path=C:\Projects\CAN\API\json --can_dut1="Kvaser CAN Channel 0" --can_dut2="Kvaser CAN Channel 1"  --gtest_output=xml:TestReport_LeafPro.xml --gtest_filter=-SmokeTest.* --sunnyday_traffic=2048` [...]
  - _If there is any error then **stop** here or create an issue for each error in the repo._
  - Copy the test report into the binaries directory `$(PROJROOT)\Binaries`.
7. Run the CAN API V3 GoogleTest with the CAN Leaf Light device:
  - `C:\Projects\CAN\Library\Testing\Windows>Debug\can_testing --can_path=C:\Projects\CAN\API\json --can_dut1="Kvaser CAN Channel 1" --can_dut2="Kvaser CAN Channel 0"  --gtest_output=xml:TestReport_LeafLight.xml --gtest_filter=-SmokeTest.* --sunnyday_traffic=2048` [...]
  - _If there is any error then **stop** here or create an issue for each error in the repo._
  - Copy the test report into the binaries directory `$(PROJROOT)\Binaries`.
8. Pack the artifacts into a .zip-archive, e.g. `artifacts.zip`:
  - `$(PROJROOT)\Binaries\*.*`
  - `$(PROJROOT)\Includes\*.*`
  - `$(PROJROOT)\README.md`
  - `$(PROJROOT)\LICENSE`
9. Double check and update the [`README.md`](https://github.com/uv-software/PeakCAN-Wrapper/blob/main/README.md) on GitHub (or insert just a blank).

### Procedure

1. Click on `Draft a new release` in the [GitHub](https://github.com/uv-software/PeakCAN-Wrapper) repo.
2. Fill out all required fields:
  - Tag version: e.g `v0.2.1` (cf. semantic versioning)
  - Target: `main` (default branch)
  - Release title: e.g. `Release of June 3, 2021`
  - Change-log: list all major changes, e.g. from commit comments
  - Assets: drag and drop the artifacts archive (see above)
3. Click on `Publish release`.
4. That´s all folks!

### Announcement

1. Create a new post with the change-log in the `mac-can.github.io` repo.
2. Update the PCANBasic page in the `mac-can.github.io` repo.
3. Post the new release on
[Twitter](https://twitter.com/uv_software),
[LinkedIn](https://linkedin.com/in/uwe-vogt-software),
[Facebook](https://facebook.com/uvsoftware.berlin),
etc.
