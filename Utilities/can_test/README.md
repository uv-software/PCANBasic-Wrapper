__CAN Tester for PEAK PCAN Interfaces, Version 0.4.3__ \
Copyright &copy; 2008-2010,2014-2022 by Uwe Vogt, UV Software, Berlin

```
Usage:
  can_test <interface>  [/RECEIVE | /RX]
                        [/Number=<number> [/Stop]]
                        [/RTR=(Yes|No)] [/XTD=(Yes|No)]
                        [/ERR=(No|Yes) | /ERROR-FRAMES]
                        [/MONitor=(No|Yes) | /LISTEN-ONLY]
                        [/Mode=(2.0|FDf[+BRS])] [/SHARED] [/Verbose]
                        [/BauDrate=<baudrate> | /BitRate=<bitrate>]
  can_test <interface>  (/TRANSMIT=<time> | /TX=<time> |
                         /FRames=<frames> | /RANDom=<frames>)
                        [/Cycle=<msec> | /Usec=<usec>] [/can-Id=<can-id>]
                        [/Dlc=<length>] [/Number=<number>]
                        [/Mode=(2.0|FDf[+BRS])] [/SHARED] [/Verbose]
                        [/BauDrate=<baudrate> | /BitRate=<bitrate>]
  can_test (/TEST-BOARDS | /TEST)
  can_test (/LIST-BOARDS | /LIST)
  can_test (/HELP | /?)
  can_test (/ABOUT | /µ)
Options:
  <frames>    Send this number of messages (frames) or
  <time>      send messages for the given time in seconds
  <msec>      Cycle time in milliseconds (default=0) or
  <usec>      cycle time in microseconds (default=0)
  <can-id>    Send with given identifier (default=100h)
  <length>    Send data of given length (default=8)
  <number>    Set first up-counting number (default=0)
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
