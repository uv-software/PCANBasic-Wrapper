__CAN Monitor for PEAK PCAN Interfaces, Version 0.4.3__ \
Copyright &copy; 2007,2017-2022 by Uwe Vogt, UV Software, Berlin

```
Usage:
  can_moni <interface>  [/Time=(ZERO|ABS|REL)]
                        [/Id=(HEX|DEC|OCT)]
                        [/Data=(HEX|DEC|OCT)]
                        [/Ascii=(ON|OFF)]
                        [/Wraparound=(No|8|10|16|32|64)]
                        [/eXclude=[~]<id>[-<id>]{,<id>[-<id>]}]
                        [/RTR=(Yes|No)] [/XTD=(Yes|No)]
                        [/ERR=(No|Yes) | /ERROR-FRAMES]
                        [/MONitor=(No|Yes) | /LISTEN-ONLY]
                        [/Mode=(2.0|FDf[+BRS])] [/SHARED] [/Verbose]
                        [/BauDrate=<baudrate> | /BitRate=<bitrate>]
  can_moni (/TEST-BOARDS | /TEST)
  can_moni (/LIST-BOARDS | /LIST)
  can_moni (/HELP  | /?)
  can_moni (/ABOUT | /µ)
Options:
  <id>        CAN identifier (11-bit)
  <interface> CAN interface board (list all with /LIST)
  <baudrate>  CAN baud rate index (default=3):
              0 = 1000 kbps
              1 = 800 kbps
              2 = 500 kbps
              3 = 250 kbps
              4 = 125 kbps
              5 = 100 kbps
              6 = 50 kbps
              7 = 20 kbps
              8 = 10 kbps
  <bitrate>   Comma-separated <key>=<value>-list:
              f_clock=<value>      Frequency in Hz or
              f_clock_mhz=<value>  Frequency in MHz
              nom_brp=<value>      Bit-rate prescaler (nominal)
              nom_tseg1=<value>    Time segment 1 (nominal)
              nom_tseg2=<value>    Time segment 2 (nominal)
              nom_sjw=<value>      Sync. jump width (nominal)
              nom_sam=<value>      Sampling (only SJA1000)
              data_brp=<value>     Bit-rate prescaler (FD data)
              data_tseg1=<value>   Time segment 1 (FD data)
              data_tseg2=<value>   Time segment 2 (FD data)
              data_sjw=<value>     Sync. jump width (FD data).
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
along with this program.  If not, see <http://www.gnu.org/licenses/>.
