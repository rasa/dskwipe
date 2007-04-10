dskwipe 0.3 - Apr 10 2007
Copyright (c) 2006-2007 Ross Smith II (http://smithii.com) All Rights Reserved

------------------------------------------------------------------------------

Usage: dskwipe [options] device(s) [byte(s)]
 bytes can be one or more numbers between 0 to 255, use 0xNN for hexidecimal,
  0NNN for octal, r for random bytes, default is 0

Options:
 -l | --list      List available devices and exit
 -p | --passes n  Wipe device n times (default is 1)
 -d | --dod       Wipe device using DoD 5220.22-M method (3 passes)
 -D | --dod7      Wipe device using DoD 5200.28-STD method (7 passes)
 -g | --gutmann   Wipe device using Gutmann method (35 passes)
 -1 | --pseudo    Use pseudo RNG (fast, not secure, this is the default)
 -2 | --windows   Use Windows RNG (slower, more secure)
 -k | --kilobyte  Use 1024 for kilobyte (default is 1000)
 -y | --yes       Start processing without waiting for confirmation
 -x | --exit mode Exit Windows. mode can be: poweroff, shutdown, hibernate,
                  logoff, reboot, or standby.
 -F | --force     Force poweroff/shutdown/logoff/reboot (WARNING: DATA LOSS!)
 -q | --quiet     Display less information (-qq = quieter, etc.)
 -z | --refresh n Refresh display every n seconds (default is 1)
 -n | --sectors n Write n sectors at once (1-65535, default is 64)
 -s | --start   n Start at relative sector n (default is 0)
 -e | --end     n End at relative sector n (default is last sector)
 -r | --read      Only read the data on the device (DOES NOT WIPE!)
 -v | --version   Show version and copyright information and quit
 -? | --help      Show this help message and quit (-?? = more help, etc.)

Examples:
 dskwipe -l                         & lists devices, and exit
 dskwipe \\.\PhysicalDrive1         & erase disk once using the byte 0
 dskwipe \Device\Ramdisk 1          & erase disk once using the byte 1
 dskwipe \Device\Ramdisk 0 255      & erase disk twice using bytes 0 then 255
 dskwipe --dod \Device\Ramdisk      & erase disk using DoD 5220.22-M method
 dskwipe \Device\Ramdisk 0 0xff r   & same as --dod (bytes 0, 255, weak random)
 dskwipe -p 2 \Device\Ramdisk 0 1   & erase disk 4 times using bytes 0/1/0/1
 dskwipe -p 2 --dod \Device\Ramdisk & erase disk twice using DoD method
 dskwipe -1 \Device\Ramdisk r r     & erase disk twice using weak RNG
 dskwipe -2 \Device\Ramdisk r r r r & erase disk four times using strong RNG
 
Here are some device names that have worked for me:

\\.\PhysicalDrive0
\\.\c:
\device\harddisk0\partition0
\device\harddisk0\partition1
\device\floppy0
\device\ramdisk

------------------------------------------------------------------------------

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

$Id$
