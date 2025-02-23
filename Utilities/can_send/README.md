__CAN Sender for PEAK-System PCAN Interfaces, Version 0.4__ \
Copyright &copy; 2025 by Uwe Vogt, UV Software, Berlin

```
Usage: can_send <interface> [<option>...]
Options:
 -m, --mode=(2.0|FDF[+BRS])           CAN operation mode: CAN 2.0 or CAN FD mode
     --shared                         shared CAN controller access (if supported)
     --listen-only                    monitor mode (listen-only mode)
     --error-frames                   allow reception of error frames
     --no-remote-frames               suppress remote frames (RTR frames)
     --no-extended-frames             suppress extended frames (29-bit identifier)
     --code=<id>                      acceptance code for 11-bit IDs (default=0x000)
     --mask=<id>                      acceptance mask for 11-bit IDs (default=0x000)
     --xtd-code=<id>                  acceptance code for 29-bit IDs (default=0x00000000)
     --xtd-mask=<id>                  acceptance mask for 29-bit IDs (default=0x00000000)
 -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or
     --bitrate=<bit-rate>             CAN bit-rate settings (as key/value list)
 -v, --verbose                        show detailed bit-rate settings
     --list-bitrates[=<mode>]         list standard bit-rate settings and exit
 -L, --list-boards                    list all supported CAN interfaces and exit
 -T, --test-boards                    list all available CAN interfaces and exit
 -J, --json=<filename>                write configuration into JSON file and exit
 -h, --help                           display this help screen and exit
     --version                        show version information and exit
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
                 data_sjw=<value>     sync. jump width (FD data)
Syntax:
 <can_frame>:
  <can_id>#{data}                     for CAN CC data frames
  <can_id>#R{len}                     for CAN CC remote frames
  <can_id>##<flags>{data}             for CAN FD data frames (up to 64 bytes)
 <can_id>:
  3  ASCII hex-chars (0 .. F) for Standard frame format (SFF) or
  8  ASCII hex-chars (0 .. F) for eXtended frame format (EFF)
 {data}:
  0 .. 8   ASCII hex-values in CAN CC mode (optionally separated by '.') or
  0 .. 64  ASCII hex-values in CAN FD mode (optionally separated by '.')
 {len}:
  an optional 0 .. 8 value as RTR frames can contain a valid DLC field
 <flags>:
  one ASCII hex-char (0 .. F) which defines CAN FD flags:
    4 = FDF                           for CAN FD frame format
    5 = FDF and BRS                   for CAN FD with Bit Rate Switch
    6 = FDF and ESI                   for CAN FD with Error State Indicator
    7 = FDF, BRS and ESI              all together now
Hazard note:
  If you connect your CAN device to a real CAN network when using this program,
  you might damage your application.
```

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, see <https://www.gnu.org/licenses/>.
