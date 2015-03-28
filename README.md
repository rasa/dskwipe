# dskwipe [![Flattr this][flatter_png]][flatter]

Securely wipe disk media.

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

````batch
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

## Verify a Release

To verify a release, download the .zip, .sha256, and .asc files for the release 
(replacing dskwipe-1.1-win32.zip with the release you are verifying):

````
$ wget https://github.com/rasa/dskwipe/releases/download/v1.1/dskwipe-1.1-win32.zip{,.sha256,.asc}
````

Next, check that sha256sum reports "OK":
````
$ sha256sum -c dskwipe-1.1-win32.zip.sha256
dskwipe-1.1-win32.zip: OK
````

Lastly, check that GPG reports "Good signature":

````
$ gpg --keyserver hkps.pool.sks-keyservers.net --recv-key 0x105a5225b6ab4b22
$ gpg --verify dskwipe-1.1-win32.zip.asc dskwipe-1.1-win32.zip
gpg:                using RSA key 0xFF914F74B4BB6EF3
gpg: Good signature from "Ross Smith II <ross@smithii.com>" [ultimate]
...
````

## Contributing

To contribute to this project, please see [CONTRIBUTING.md](CONTRIBUTING.md).

## Bugs

To view existing bugs, or report a new bug, please see [issues](../../issues).

## Changelog

To view the version history for this project, please see [CHANGELOG.md](CHANGELOG.md).

## License

This project is [MIT licensed](LICENSE).

## Contact

This project was created and is maintained by [Ross Smith II][] [![endorse][endorse_png]][endorse]

Feedback, suggestions, and enhancements are welcome.

[Ross Smith II]: mailto:ross@smithii.com "ross@smithii.com"
[flatter]: https://flattr.com/submit/auto?user_id=rasa&url=https%3A%2F%2Fgithub.com%2Frasa%2Fdskwipe
[flatter_png]: http://button.flattr.com/flattr-badge-large.png "Flattr this"
[endorse]: https://coderwall.com/rasa
[endorse_png]: https://api.coderwall.com/rasa/endorsecount.png "endorse"

