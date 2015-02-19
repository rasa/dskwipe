# dskwipe 

Securely wipe disk media

## Usage

````
dskwipe [options] device(s) [byte(s)]
 bytes can be one or more numbers between 0 to 255, use 0xNN for hexidecimal,
  0NNN for octal, r for random bytes, default is 0

Options:

 -l | --list      List available devices and exit
 -p | --passes n  Wipe device n times (default is 1)
 -d | --dod       Wipe device using US DoD 5220.22-M method (3 passes)
 -E | --doe       Wipe device using US DoE method (3 passes)
 -D | --dod7      Wipe device using US DoD 5200.28-STD method (7 passes)
 -S | --schneier  Wipe device using Bruce Schneier's method (7 passes)
 -b | --bci       Wipe device using German BCI/VSITR method (7 passes)
 -g | --gutmann   Wipe device using Peter Gutmann's method (35 passes)
 -1 | --pseudo    Use pseudo RNG (fast, not secure, this is the default)
 -2 | --windows   Use Windows RNG (slower, more secure)
 -k | --kilobyte  Use 1024 for kilobyte (default is 1000)
 -y | --yes       Start processing without waiting for confirmation
 -x | --exit mode Exit Windows. mode can be: poweroff, shutdown, hibernate,
                  logoff, reboot, or standby.
 -f | --force     Force poweroff/shutdown/logoff/reboot (WARNING: DATA LOSS!)
 -q | --quiet     Display less information (-qq = quieter, etc.)
 -z | --refresh n Refresh display every n seconds (default is 1)
 -n | --sectors n Write n sectors at once (1-65535, default is 64)
 -s | --start   n Start at relative sector n (default is 0)
 -e | --end     n End at relative sector n (default is last sector)
 -r | --read      Only read the data on the device (DOES NOT WIPE!)
 -i | --ignore    Ignore certain read/write errors
 -v | --version   Show version and copyright information and quit
 -? | --help      Show this help message and quit (-?? = more help, etc.)
````

## Examples

````
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
```` 
Here are some device names that have worked for me:

````
\\.\PhysicalDrive0
\\.\c:
\device\harddisk0\partition0
\device\harddisk0\partition1
\device\floppy0
\device\ramdisk
````

## Contributing

To contribute to this project, please see [CONTRIBUTING.md](CONTRIBUTING.md).

## Bugs

To view existing bugs, or report a new bug, please see the [issues](/issues) page for this project.

## License

This project is [MIT licensed](LICENSE).

## Changelog

Please see [CHANGELOG.md](CHANGELOG.md) for the version history for this project.

## Contact

This project was originally developed by [Ross Smith II](mailto:ross@smithii.com).
Any enhancements and suggestions are welcome.
