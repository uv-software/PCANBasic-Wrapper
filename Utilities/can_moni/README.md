__CAN Monitor for Peak-System PCAN Interfaces, Version 0.5.0__ \
Copyright &copy; 2007,2012-2024 by Uwe Vogt, UV Software, Berlin

```
Usage: can_moni <interface> [<option>...]
Options:
  /Time:(ZERO|ABS|REL)                absolute or relative time (default=0)
  /Id:(HEX|DEC|OCT)                   display mode of CAN-IDs (default=HEX)
  /Data:(HEX|DEC|OCT)                 display mode of data bytes (default=HEX)
  /Ascii:(ON|OFF)                     display data bytes in ASCII (default=ON)
  /Wraparound:(No|8|10|16|32|64)      wraparound after n data bytes (default=NO)
  /eXclude:[~]<id-list>               exclude CAN-IDs: <id-list> = <id>[-<id>]{,<id>[-<id>]}
  /CODE:<id>                          acceptance code for 11-bit IDs (default=0x000)
  /MASK:<id>                          acceptance mask for 11-bit IDs (default=0x000)
  /XTD-CODE:<id>                      acceptance code for 29-bit IDs (default=0x00000000)
  /XTD-MASK:<id>                      acceptance mask for 29-bit IDs (default=0x00000000)
  /Mode:(2.0|FDf[+BRS])               CAN operation mode: CAN 2.0 or CAN FD mode
  /SHARED                             shared CAN controller access (if supported)
  /MONitor:(No|Yes) | /LISTEN-ONLY    monitor mode (listen-only mode)
  /ERR:(No|Yes) | /ERROR-FRAMES       allow reception of error frames
  /RTR:(Yes|No)                       allow remote frames (RTR frames)
  /XTD:(Yes|No)                       allow extended frames (29-bit identifier)
  /BauDrate:<baudrate>                CAN bit-timing in kbps (default=250), or
  /BitRate:<bitrate>                  CAN bit-rate settings (as key/value list)
  /Verbose                            show detailed bit-rate settings
  /LIST-BITRATES[:(2.0|FDf[+BRS])]    list standard bit-rate settings and exit
  /LIST-BOARDS | /LIST                list all supported CAN interfaces and exit
  /TEST-BOARDS | /TEST                list all available CAN interfaces and exit
  /JSON-file:<filename>               write configuration into JSON file and exit
  /HELP | /?                          display this help screen and exit
  /VERSION                            show version information and exit
Arguments:
  <id>           CAN identifier (11-bit)
  <interface>    CAN interface board (list all with /LIST)
  <baudrate>     CAN baud rate index (default=3):
                 0 = 1000 kbps
                 1 = 800 kbps
                 2 = 500 kbps
                 3 = 250 kbps
                 4 = 125 kbps
                 5 = 100 kbps
                 6 = 50 kbps
                 7 = 20 kbps
                 8 = 10 kbps
  <bitrate>      comma-separated key/value list:
                 f_clock=<value>      frequency in Hz or
                 f_clock_mhz=<value>  frequency in MHz
                 nom_brp=<value>      bit-rate prescaler (nominal)
                 nom_tseg1=<value>    time segment 1 (nominal)
                 nom_tseg2=<value>    time segment 2 (nominal)
                 nom_sjw=<value>      sync. jump width (nominal)
                 nom_sam=<value>      sampling (only SJA1000)
                 data_brp=<value>     bit-rate prescaler (FD data)
                 data_tseg1=<value>   time segment 1 (FD data)
                 data_tseg2=<value>   time segment 2 (FD data)
                 data_sjw=<value>     sync. jump width (FD data).
Hazard note:
  If you connect your CAN device to a real CAN network when using this program,
  you might damage your application.
```

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
