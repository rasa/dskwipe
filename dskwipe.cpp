/*

$Id$

Copyright (c) 2005-2006 Ross Smith II (http://smithii.com). All rights reserved.

This program is free software; you can redistribute it and/or modify it
under the terms of version 2 of the GNU General Public License
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

*/

/*
todo/wishlist
handle bad sectors without failing
scramble the MFT/FAT tables first
catch ctrl-C to report where to start over
Implement HAVE_CRYPTOGRAPHIC using TrueCrypt's RNG
*/

#include <windows.h>

#include <stdio.h>
#include <time.h>		// time()
#include <process.h>	// _getpid()
#include <wincrypt.h>

#include "getopt.h"

#include "version.h"

#define APPNAME			VER_INTERNAL_NAME
#define APPVERSION		VER_STRING2
#define APPCOPYRIGHT	VER_LEGAL_COPYRIGHT

static char *progname = APPNAME;

#undef HAVE_CRYPTOGRAPHIC

//#define DUMMY_WRITE 1

#define BYTES_PER_ELEMENT (3)

#define SECTORS_PER_READ (64)

// \todo convert separate wipe arrays to one array

int dod_bytes[] = {0x00, 0xff, -1};

int dod_elements = sizeof(dod_bytes) / sizeof(dod_bytes[0]);

// source: BCWipe-1.6-5/bcwipe/wipe.h
int dod7_bytes[] = {0x35, 0xca, 0x97, 0x68, 0xac, 0x53, -1};

int dod7_elements = sizeof(dod7_bytes) / sizeof(dod7_bytes[0]);

// source: http://www.cs.auckland.ac.nz/~pgut001/pubs/secure_del.html
int gutmann_bytes[][BYTES_PER_ELEMENT] = {
	{-1,   -1,   -1}, // 1
	{-1,   -1,   -1}, // 2
	{-1,   -1,   -1}, // 3
	{-1,   -1,   -1}, // 4
	{0x55, 0x55, 0x55}, // 5
	{0xAA, 0xAA, 0xAA}, // 6
	{0x92, 0x49, 0x24}, // 7
	{0x49, 0x24, 0x92}, // 8
	{0x24, 0x92, 0x49}, // 9
	{0x00, 0x00, 0x00}, // 10
	{0x11, 0x11, 0x11}, // 11
	{0x22, 0x22, 0x22}, // 12
	{0x33, 0x33, 0x33}, // 13
	{0x44, 0x44, 0x44}, // 14
	{0x55, 0x55, 0x55}, // 15
	{0x66, 0x66, 0x66}, // 16
	{0x77, 0x77, 0x77}, // 17
	{0x88, 0x88, 0x88}, // 18
	{0x99, 0x99, 0x99}, // 19
	{0xAA, 0xAA, 0xAA}, // 20
	{0xBB, 0xBB, 0xBB}, // 21
	{0xCC, 0xCC, 0xCC}, // 22
	{0xDD, 0xDD, 0xDD}, // 23
	{0xEE, 0xEE, 0xEE}, // 24
	{0xFF, 0xFF, 0xFF}, // 25
	{0x92, 0x49, 0x24}, // 26
	{0x49, 0x24, 0x92}, // 27
	{0x24, 0x92, 0x49}, // 28
	{0x6D, 0xB6, 0xDB}, // 29
	{0xB6, 0xDB, 0x6D}, // 30
	{0xDB, 0x6D, 0xB6}, // 31
	{-1,   -1,   -1}, // 32
	{-1,   -1,   -1}, // 33
	{-1,   -1,   -1}, // 34
	{-1,   -1,   -1}  // 35
};

int gutmann_elements = sizeof(gutmann_bytes) / sizeof(gutmann_bytes[0]);

	         //          1         2         3         4         5         6         7
             // 12345678901234567890123456789012345678901234567890123456789012345678901234567890
#define HEADER "                     This      All      All      All                           \n" \
               "Pass No. of          Pass   Passes   Passes   Passes              Est.     %s/\n" \
               " No. Passes Byte Complete Complete  Elapsed  Remain.    Start   Finish   Second\n" \
               "---- ------ ---- -------- -------- -------- -------- -------- -------- --------\n"
             // 1234 123456 0xff 100.000% 100.000% 00:00:00 00:00:00 00:00:00 00:00:00 12345.67

#define FORMAT_STRING "%4d %6d %4s %7.3f%% %7.3f%%%9s%9s %8s %8s%9.2f\r"

static char *short_options = "12de:fgkln:p:qrs:vx:yz:D?"
#ifdef HAVE_CRYPTOGRAPHIC
	"3"
#endif
	;

static struct option long_options[] = {
  {"dod",		no_argument,		0, 'd'},
  {"dod3",		no_argument,		0, 'd'},
  {"dod7",		no_argument,		0, 'D'},
  {"end",		required_argument,	0, 'e'},
  {"exit",		required_argument,	0, 'x'},
  {"force",		no_argument,		0, 'f'},
  {"gutmann",	no_argument,		0, 'g'},
  {"help",		no_argument,		0, '?'},
  {"kilo",		no_argument,		0, 'k'},
  {"kilobyte",	no_argument,		0, 'k'},
  {"list",		no_argument,		0, 'l'},
  {"passes",	required_argument,	0, 'p'},
  {"pseudo",	no_argument,		0, '1'},
  {"quiet",		no_argument,		0, 'q'},
  {"read",		no_argument,		0, 'r'},
  {"refresh",	required_argument,	0, 'z'},
  {"sectors",	required_argument,	0, 'n'},
  {"start",		required_argument,	0, 's'},
  {"version",	no_argument,		0, 'v'},
  {"windows",	no_argument,		0, '2'},
  {"yes",		no_argument,		0, 'y'},

  {"customwipe",required_argument,	0, 'p'}, // gdisk compatible
  {"dodwipe",	no_argument,		0, 'd'}, // gdisk compatible
  {"sure",		no_argument,		0, 'y'}, // gdisk compatible
#ifdef HAVE_CRYPTOGRAPHIC
  {"crypto",	no_argument,		0, '3'},
#endif
/*
  {"poweroff",	no_argument,		0, 'P'},
  {"shutdown",	no_argument,		0, 'S'},
  {"hibernate",	no_argument,		0, 'H'},
  {"logoff",	no_argument,		0, 'L'},
  {"reboot",	no_argument,		0, 'R'},
  {"standby",	no_argument,		0, 'T'},
*/
  {NULL,		0,					0, 0}
};

void version() {
	printf(APPNAME " " APPVERSION " - " __DATE__ "\n");
	printf(APPCOPYRIGHT "\n");
}

typedef enum {
	EXIT_NONE,
	EXIT_POWEROFF,
	EXIT_SHUTDOWN,
	EXIT_HIBERNATE,
	EXIT_LOGOFF,
	EXIT_REBOOT,
	EXIT_STANDBY
} ExitMode;

char *exit_mode[] = {
	"none",
	"poweroff",
	"shutdown",
	"hibernate",
	"logoff",
	"reboot",
	"standby"
};

int exit_modes = sizeof(exit_mode) / sizeof(exit_mode[0]);

void _usage() {
	fprintf(stderr, "Usage: %s [options] device(s) [byte(s)]\n"
		" bytes can be one or more numbers between 0 to 255, use 0xNN for hexidecimal,\n"
		"  0NNN for octal, r for random bytes, default is 0\n"
		"\nOptions:\n"
		" -l | --list      List available devices and exit\n"
		" -p | --passes n  Wipe device n times (default is 1)\n"
		" -d | --dod       Wipe device using DoD 5220.22-M method (3 passes)\n"
		" -D | --dod7      Wipe device using DoD 5200.28-STD method (7 passes)\n"
		" -g | --gutmann   Wipe device using Gutmann method (35 passes)\n"
		" -1 | --pseudo    Use pseudo RNG (fast, not secure, this is the default)\n"
		" -2 | --windows   Use Windows RNG (slower, more secure)\n"
#ifdef HAVE_CRYPTOGRAPHIC
		" -3 | --crypto    Use cryptographically secure RNG (slowest, secure)\n"
#endif
		" -k | --kilobyte  Use 1024 for kilobyte (default is 1000)\n"
		" -y | --yes       Start processing without waiting for confirmation\n"
      //          1         2         3         4         5         6         7
      // 12345678901234567890123456789012345678901234567890123456789012345678901234567890

		" -x | --exit mode Exit Windows. mode can be: poweroff, shutdown, hibernate, \n"
		"                  logoff, reboot, or standby.\n"
/*
		" -P | --poweroff  Power off computer when finished\n"
		" -S | --shutdown  Shutdown computer when finished\n"
		" -H | --hibernate Hibernate computer when finished\n"
		" -L | --logoff    Log off computer when finished\n"
		" -R | --reboot    Reboot computer when finished\n"
		" -T | --standby   Standby computer when finished\n"
*/
		" -F | --force     Force poweroff/shutdown/logoff/reboot (WARNING: DATA LOSS!)\n"
		" -q | --quiet     Display less information (-qq = quieter, etc.)\n"
		" -z | --refresh n Refresh display every n seconds (default is 1)\n"
		" -n | --sectors n Write n sectors at once (1-65535, default is %d)\n"
		" -s | --start   n Start at relative sector n (default is 0)\n"
		" -e | --end     n End at relative sector n (default is last sector)\n"
		" -r | --read      Only read the data on the device (DOES NOT WIPE!)\n"
		" -v | --version   Show version and copyright information and quit\n"
		" -? | --help      Show this help message and quit (-?? = more help, etc.)\n",
		progname, SECTORS_PER_READ);
}

void examples() {
	fprintf(stderr,
		"\nExamples:\n"
		" %s -l                         & lists devices, and exit\n"
		" %s \\\\.\\PhysicalDrive1         & erase disk once using the byte 0\n"
		" %s \\Device\\Ramdisk 1          & erase disk once using the byte 1\n"
		" %s \\Device\\Ramdisk 0 255      & erase disk twice using bytes 0 then 255\n"
		" %s --dod \\Device\\Ramdisk      & erase disk using DoD 5220.22-M method\n"
		" %s \\Device\\Ramdisk 0 0xff r   & same as --dod (bytes 0, 255, weak random)\n"
		" %s -p 2 \\Device\\Ramdisk 0 1   & erase disk 4 times using bytes 0/1/0/1\n"
		" %s -p 2 --dod \\Device\\Ramdisk & erase disk twice using DoD method\n"
		" %s -1 \\Device\\Ramdisk r r     & erase disk twice using weak RNG\n"
		" %s -2 \\Device\\Ramdisk r r r r & erase disk four times using strong RNG\n",
		progname,
		progname,
		progname,
		progname,
		progname,
		progname,
		progname,
		progname,
		progname,
		progname);
}

void usage(int exit_code) {
	_usage();
	exit(exit_code);
}

typedef enum {
	WIPEMODE_NORMAL,
	WIPEMODE_DOD,
	WIPEMODE_DOD7,
	WIPEMODE_GUTMANN
} WipeMode;

char *wipe_methods[] = {
	"standard wiping method (1 pass per iteration)",
	"DoD 5220.22-M wiping method (3 passes per iteration)",
	"DoD 5200.28-STD wiping method (7 passes per iteration)",
	"Gutmann wiping method (35 passes per iteration)"
};

typedef enum {
	RANDOM_PSEUDO,
	RANDOM_WINDOWS,
#ifdef HAVE_CRYPTOGRAPHIC
	RANDOM_CRYPTOGRAPHIC
#endif
} RandomMode;

struct _opt {
	bool			list;
	unsigned int	passes;
	WipeMode		mode;
	bool			yes;
	RandomMode		random;
	ExitMode		restart;
	bool			force;
	unsigned int	quiet;
	unsigned int	sectors;
	ULONGLONG		start;
	ULONGLONG		end;
	bool			read;
	unsigned int	help;
	bool			kilobyte;
	unsigned int	refresh;
};

typedef struct _opt t_opt;

static t_opt opt = {
	false,				/* list */
	0,					/* passes */
	WIPEMODE_NORMAL,	/* normal, dod, dod7, gutmann */
	false,				/* yes */
	RANDOM_PSEUDO,		/* pseudo, windows, cryptographic */
	EXIT_NONE,			/* none, poweroff, shutdown, hibernate, logoff, reboot, standby */
	false,				/* force */
	0,					/* quiet */
	SECTORS_PER_READ,	/* sectors */
	0ui64,				/* start */
	0ui64,				/* end */
	false,				/* read */
	0,					/* help */
	false,				/* kilobyte */
	1,					/* refresh */
};

/* per http://www.scit.wlv.ac.uk/cgi-bin/mansec?3C+basename */
static char* basename(char* s) {
	char* rv;

	if (!s || !*s)
		return ".";

	rv = s + strlen(s) - 1;

	do {
		if (*rv == '/' || *rv == '\\')
			return rv + 1;
		--rv;
	} while (rv >= s);

	return s;
}

static void FatalError(char *str) {
	LPVOID lpMsgBuf;
	DWORD err = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL);
	fprintf(stderr, "\n%s: %s\n", str, lpMsgBuf);
	exit(err ? err : 1);
}

static BOOL add_seconds(SYSTEMTIME *st, DWORD seconds, SYSTEMTIME *rv) {
	FILETIME ft;
	SystemTimeToFileTime(st, &ft);

	ULARGE_INTEGER uli;
	uli.HighPart		= ft.dwHighDateTime;
	uli.LowPart			= ft.dwLowDateTime;
	uli.QuadPart		+= (seconds * 10000000ui64);

	ft.dwHighDateTime	= uli.HighPart;
	ft.dwLowDateTime	= uli.LowPart;

	return (BOOL) FileTimeToSystemTime(&ft, rv);
};

static char *systemtime_to_hhmmss(SYSTEMTIME *st, char *rv, int bufsiz) {
	_snprintf(rv, bufsiz, "%02d:%02d:%02d", st->wHour, st->wMinute, st->wSecond);

	return rv;
}

static char *seconds_to_hhmmss(DWORD seconds, char *rv, int bufsiz) {
	DWORD hours = seconds / 3600;
	seconds -= hours * 3600;

	DWORD minutes = seconds / 60;
	seconds -= minutes * 60;

	_snprintf(rv, bufsiz, "%02d:%02d:%02d", hours, minutes, seconds);

	return rv;
}

struct _stats {
	char		*device_name;
	DWORD		bytes_per_sector;
	ULONGLONG	tick_frequency;	/* ticks to seconds divisor */

	ULONGLONG	start_ticks;
	SYSTEMTIME	lpStartTime;
	char		start_time[20];
	ULONGLONG	wiping_ticks;

	ULONGLONG	all_start_ticks;
	ULONGLONG	all_wiping_ticks;
};

typedef struct _stats t_stats;

static ULONGLONG get_ticks(t_stats *stats) {
	typedef enum {STATE_UNINITIALIZED, STATE_USE_FREQUENCY, STATE_USE_TICKCOUNT} t_state;
	static t_state state = STATE_UNINITIALIZED;

	static DWORD last_ticks;
	static ULONGLONG overflow_ticks = 0;

	if (state == STATE_UNINITIALIZED) {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		if (frequency.QuadPart >= 1000) {
			state = STATE_USE_FREQUENCY;
			stats->tick_frequency = frequency.QuadPart;
		} else {
			state = STATE_USE_TICKCOUNT;
			stats->tick_frequency = 1000;
			last_ticks = GetTickCount();
		}
	}

	if (state == STATE_USE_FREQUENCY) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return now.QuadPart;
	}

	DWORD ticks = GetTickCount();
	if (ticks < last_ticks) {
		overflow_ticks += 0x100000000;
	}

	return overflow_ticks + (ULONGLONG) ticks;
}

static void print_stats(unsigned int pass, char *s_byte, ULONGLONG sector, t_stats *stats) {
	ULONGLONG starting_sector = opt.start;
	ULONGLONG ending_sector = opt.end;

	ULONGLONG done_sectors	= (ending_sector * ((ULONGLONG) pass - 1)) + sector;
	ULONGLONG total_sectors = ending_sector * opt.passes;

	double all_pct = (double) (LONGLONG) done_sectors / (double) (LONGLONG) total_sectors * 100.0;

	ULONGLONG remaining_ticks = 0;

	ULONGLONG elapsed_ticks = get_ticks(stats) - stats->start_ticks;

	if (done_sectors) {
		remaining_ticks = (total_sectors - done_sectors) * elapsed_ticks / done_sectors;
	}

	static double kilo = opt.kilobyte ? 1024 : 1000;

	double mb_sec = 0;

	if (stats->wiping_ticks) {
		ULONGLONG bytes = done_sectors * stats->bytes_per_sector;
		double megabytes = (double) (LONGLONG) bytes / (kilo * kilo);
		double seconds = (double) (LONGLONG) stats->wiping_ticks / (double) (LONGLONG) stats->tick_frequency;
//printf("\nsector=%20I64d done_sectors=%20I64d bytes=%20I64d megabytes=%20.10f seconds=%20.10f\n", sector, done_sectors, bytes, megabytes, seconds);
		if (seconds > 0) {
			mb_sec = megabytes / seconds;
		}
	}

	sector -= starting_sector;
	ending_sector -= starting_sector;

	double this_pct = (double) (LONGLONG) sector / (double) (LONGLONG) ending_sector * 100.0;

	if (sector >= ending_sector) {
		this_pct = 100.0;
		all_pct = (double) (pass) * 100.0 / (double) opt.passes;
		if (pass >= opt.passes) {
			this_pct = 100.0;
			all_pct = 100.0;
			remaining_ticks = 0;
		}
	}

	DWORD remaining_seconds = (DWORD) (remaining_ticks / stats->tick_frequency);

	char remaining_time[255];
	seconds_to_hhmmss(remaining_seconds, remaining_time, sizeof(remaining_time));

	DWORD elapsed_seconds = (DWORD) (elapsed_ticks / stats->tick_frequency);

	char elapsed_time[255];
	seconds_to_hhmmss(elapsed_seconds, elapsed_time, sizeof(elapsed_time));

	SYSTEMTIME lpEndTime;
	add_seconds(&stats->lpStartTime, elapsed_seconds + remaining_seconds, &lpEndTime);

	char finish_time[255];
	systemtime_to_hhmmss(&lpEndTime, finish_time, sizeof(finish_time));

	char buf[255];
	_snprintf(buf, sizeof(buf), "%.3f%% - %s - %s - %s", all_pct, remaining_time, stats->device_name, progname);
	SetConsoleTitle(buf);

	if (opt.quiet == 1) {
		_snprintf(buf, sizeof(buf), "%s - %.3f%% complete - %s remaining\r", stats->device_name, all_pct, remaining_time, progname);
		printf("%s\r", buf);
		fflush(stdout);
		return;
	}

	printf(FORMAT_STRING,
		pass,
		opt.passes,
		s_byte,
		this_pct,
		all_pct,
		elapsed_time,
		remaining_time,
		stats->start_time,
		finish_time,
		mb_sec);
	fflush(stdout);
}

// <adapted from truecrypt-4.2a-source-code/TrueCrypt/Common/Dlgcode.c>

static int FakeDosNameForDevice (char *lpszDiskFile, char *lpszDosDevice, char *lpszCFDevice, BOOL bNameOnly) {
	if (strncmp(lpszDiskFile, "\\\\", 2) == 0) {
		strcpy(lpszCFDevice, lpszDiskFile);
		return 1;
	}

	BOOL bDosLinkCreated = TRUE;
	_snprintf(lpszDosDevice, MAX_PATH, "dskwipe%lu", GetCurrentProcessId());

	if (bNameOnly == FALSE)
		bDosLinkCreated = DefineDosDevice (DDD_RAW_TARGET_PATH, lpszDosDevice, lpszDiskFile);

	if (bDosLinkCreated == FALSE) {
		return 1;
	} else {
		_snprintf(lpszCFDevice, MAX_PATH, "\\\\.\\%s", lpszDosDevice);
	}

	return 0;
}

static int RemoveFakeDosName (char *lpszDiskFile, char *lpszDosDevice) {
	BOOL bDosLinkRemoved = DefineDosDevice (DDD_RAW_TARGET_PATH | DDD_EXACT_MATCH_ON_REMOVE |
			DDD_REMOVE_DEFINITION, lpszDosDevice, lpszDiskFile);
	if (bDosLinkRemoved == FALSE) {
		return 1;
	}

	return 0;
}

static void GetSizeString (LONGLONG size, wchar_t *str) {
	static wchar_t *b, *kb, *mb, *gb, *tb, *pb;

	if (b == NULL) {
		if (opt.kilobyte) {
			kb = L"KiB";
			mb = L"MiB";
			gb = L"GiB";
			tb = L"TiB";
			pb = L"PiB";
		} else {
			kb = L"KB";
			mb = L"MB";
			gb = L"GB";
			tb = L"TB";
			pb = L"PB";
		}
		b = L"bytes";
	}

	DWORD kilo = opt.kilobyte ? 1024 : 1000;
	LONGLONG kiloI64 = kilo;
	double kilod = kilo;

	if (size > kiloI64 * kilo * kilo * kilo * kilo * 99)
		swprintf (str, L"%I64d %s", size/ kilo / kilo /kilo/kilo/kilo, pb);
	else if (size > kiloI64*kilo*kilo*kilo*kilo)
		swprintf (str, L"%.1f %s",(double)(size/kilod/kilo/kilo/kilo/kilo), pb);
	else if (size > kiloI64*kilo*kilo*kilo*99)
		swprintf (str, L"%I64d %s",size/kilo/kilo/kilo/kilo, tb);
	else if (size > kiloI64*kilo*kilo*kilo)
		swprintf (str, L"%.1f %s",(double)(size/kilod/kilo/kilo/kilo), tb);
	else if (size > kiloI64*kilo*kilo*99)
		swprintf (str, L"%I64d %s",size/kilo/kilo/kilo, gb);
	else if (size > kiloI64*kilo*kilo)
		swprintf (str, L"%.1f %s",(double)(size/kilod/kilo/kilo), gb);
	else if (size > kiloI64*kilo*99)
		swprintf (str, L"%I64d %s", size/kilo/kilo, mb);
	else if (size > kiloI64*kilo)
		swprintf (str, L"%.1f %s",(double)(size/kilod/kilo), mb);
	else if (size > kiloI64)
		swprintf (str, L"%I64d %s", size/kilo, kb);
	else
		swprintf (str, L"%I64d %s", size, b);
}

static void list_device(char *format_str, char *szTmp, int n) {
	int nDosLinkCreated;
	HANDLE dev;
	DWORD dwResult;
	BOOL bResult;
	PARTITION_INFORMATION diskInfo;
	DISK_GEOMETRY driveInfo;
	char szDosDevice[MAX_PATH], szCFDevice[MAX_PATH];
	static LONGLONG deviceSize = 0;
	wchar_t size[100] = {0}, partTypeStr[1024] = {0}, *partType = partTypeStr;

	BOOL drivePresent = FALSE;
	BOOL removable = FALSE;

	drivePresent = TRUE;

	nDosLinkCreated = FakeDosNameForDevice (szTmp, szDosDevice,
		szCFDevice, FALSE);

	dev = CreateFile (szCFDevice, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

	bResult = DeviceIoControl (dev, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0,
		&diskInfo, sizeof (diskInfo), &dwResult, NULL);

	// Test if device is removable
	if (/* n == 0 && */ DeviceIoControl (dev, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
		&driveInfo, sizeof (driveInfo), &dwResult, NULL))
		removable = driveInfo.MediaType == RemovableMedia;

	RemoveFakeDosName(szTmp, szDosDevice);
	CloseHandle(dev);

	if (!bResult)
		return;

	// System creates a virtual partition1 for some storage devices without
	// partition table. We try to detect this case by comparing sizes of
	// partition0 and partition1. If they match, no partition of the device
	// is displayed to the user to avoid confusion. Drive letter assigned by
	// system to partition1 is displayed as subitem of partition0

	if (n == 0) {
		deviceSize = diskInfo.PartitionLength.QuadPart;
	}

	if (n > 0 && diskInfo.PartitionLength.QuadPart == deviceSize) {
		return;
	}

	switch(diskInfo.PartitionType) {
		case PARTITION_ENTRY_UNUSED:	partType = L""; break;
		case PARTITION_XINT13_EXTENDED:
		case PARTITION_EXTENDED:		partType = L"Extended"; break;
		case PARTITION_HUGE:			wsprintfW (partTypeStr, L"%s (0x%02X)", L"Unformatted", diskInfo.PartitionType); partType = partTypeStr; break;
		case PARTITION_FAT_12:			partType = L"FAT12"; break;
		case PARTITION_FAT_16:			partType = L"FAT16"; break;
		case PARTITION_FAT32:
		case PARTITION_FAT32_XINT13:	partType = L"FAT32"; break;
		case 0x08:						partType = L"DELL (spanning)"; break;
		case 0x12:						partType = L"Config/diagnostics"; break;
		case 0x11:
		case 0x14:
		case 0x16:
		case 0x1b:
		case 0x1c:
		case 0x1e:						partType = L"Hidden FAT"; break;
		case PARTITION_IFS:				partType = L"NTFS"; break;
		case 0x17:						partType = L"Hidden NTFS"; break;
		case 0x3c:						partType = L"PMagic recovery"; break;
		case 0x3d:						partType = L"Hidden NetWare"; break;
		case 0x41:						partType = L"Linux/MINIX"; break;
		case 0x42:						partType = L"SFS/LDM/Linux Swap"; break;
		case 0x51:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:						partType = L"Novell"; break;
		case 0x55:						partType = L"EZ-Drive"; break;
		case PARTITION_OS2BOOTMGR:		partType = L"OS/2 BM"; break;
		case PARTITION_XENIX_1:
		case PARTITION_XENIX_2:			partType = L"Xenix"; break;
		case PARTITION_UNIX:			partType = L"UNIX"; break;
		case 0x74:						partType = L"Scramdisk"; break;
		case 0x78:						partType = L"XOSL FS"; break;
		case 0x80:
		case 0x81:						partType = L"MINIX"; break;
		case 0x82:						partType = L"Linux Swap"; break;
		case 0x43:
		case 0x83:						partType = L"Linux"; break;
		case 0xc2:
		case 0x93:						partType = L"Hidden Linux"; break;
		case 0x86:
		case 0x87:						partType = L"NTFS volume set"; break;
		case 0x9f:						partType = L"BSD/OS"; break;
		case 0xa0:
		case 0xa1:						partType = L"Hibernation"; break;
		case 0xa5:						partType = L"BSD"; break;
		case 0xa8:						partType = L"Mac OS-X"; break;
		case 0xa9:						partType = L"NetBSD"; break;
		case 0xab:						partType = L"Mac OS-X Boot"; break;
		case 0xb8:						partType = L"BSDI BSD/386 swap"; break;
		case 0xc3:						partType = L"Hidden Linux swap"; break;
		case 0xfb:						partType = L"VMware"; break;
		case 0xfc:						partType = L"VMware swap"; break;
		case 0xfd:						partType = L"Linux RAID"; break;
		case 0xfe:						partType = L"WinNT hidden"; break;
		default:						wsprintfW(partTypeStr, L"0x%02X", diskInfo.PartitionType); partType = partTypeStr; break;
	}

	GetSizeString(diskInfo.PartitionLength.QuadPart, size);
	char *s_type = removable ? "Removable" : "Fixed";
	printf(format_str, szTmp, size, s_type, partType);
}

// </adapted from truecrypt-4.2a-source-code/TrueCrypt/Common/Dlgcode.c>

void print_ticks(char *fmt, ULONGLONG ticks, ULONGLONG tick_frequency) {
	char wiping_time[255];
	DWORD seconds = (DWORD) (ticks / tick_frequency);
	seconds_to_hhmmss(seconds, wiping_time, sizeof(wiping_time));
	printf(fmt, wiping_time);
}

static void list_devices() {
	printf(
		"Device Name                         Size Type      Partition Type\n"
		"------------------------------ --------- --------- --------------------\n"
//       123456789012345678901234567890 123456789 123456789 12345678901234567890
//       \Device\Harddisk30\Partition03 1234.1 GB Removable SFS/LDM/Linux Swap
	);


	char *format_str = "%-30s %9S %-9s %-20S\n";

	char szTmp[MAX_PATH];
	int i;

	for (i = 0; i < 64; i++) {
		_snprintf(szTmp, sizeof(szTmp), "\\\\.\\PhysicalDrive%d", i);
		list_device(format_str, szTmp, 0);
	}

	for (i = 0; i < 64; i++) {
		for (int n = 0; n <= 32; n++) {
			_snprintf(szTmp, sizeof(szTmp), "\\Device\\Harddisk%d\\Partition%d", i, n);
			list_device(format_str, szTmp, n);
		}
	}

	for (i = 0; i < 8; i++) {
		_snprintf(szTmp, sizeof(szTmp), "\\Device\\Floppy%d", i);
		list_device(format_str, szTmp, 0);
	}

	list_device(format_str, "\\Device\\Ramdisk", 0);

	for (i = 0; i < 26; i++) {
		_snprintf(szTmp, sizeof(szTmp), "\\\\.\\%c:", 'A' + i);
		list_device(format_str, szTmp, 0);
	}
}

int wipe_device(char *device_name, int bytes, int *byte, t_stats *stats, HCRYPTPROV hProv, HCRYPTKEY hKey) {
	stats->start_ticks = get_ticks(stats);
	stats->wiping_ticks = 0;
	GetLocalTime(&stats->lpStartTime);
	systemtime_to_hhmmss(&stats->lpStartTime, stats->start_time, sizeof(stats->start_time));
	stats->start_ticks = get_ticks(stats);

	stats->device_name = device_name;

	char err[256];

	char szCFDevice[MAX_PATH];
	char szDosDevice[MAX_PATH];
	int nDosLinkCreated = FakeDosNameForDevice(device_name, szDosDevice, szCFDevice, FALSE);

	switch (opt.quiet) {
		case 0:
			printf("Device:          %s\n", device_name);
			break;
		case 1:
			printf("%s\r", device_name);
			break;
	}

	HANDLE hnd = CreateFile(
		szCFDevice,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hnd == INVALID_HANDLE_VALUE) {
		_snprintf(err, sizeof(err), "Cannot open '%s'", device_name);
		FatalError(err);
	}

	ULONGLONG last_sector = 0;

	DISK_GEOMETRY driveInfo;
	PARTITION_INFORMATION diskInfo;
	DWORD dwResult;
	BOOL bResult;

	bResult = DeviceIoControl(
			hnd,
			IOCTL_DISK_GET_DRIVE_GEOMETRY,
			NULL,
			0,
			&driveInfo,
			sizeof(driveInfo),
			&dwResult,
			NULL);

	if (!bResult) {
		_snprintf(err, sizeof(err), "Cannot query '%s'", device_name);
		FatalError(err);
	}

	last_sector = driveInfo.Cylinders.QuadPart * driveInfo.TracksPerCylinder * driveInfo.SectorsPerTrack;

	stats->bytes_per_sector = driveInfo.BytesPerSector;

	if (opt.quiet == 0) {
		ULONGLONG total_sectors = last_sector + 1;
		ULONGLONG total_bytes = total_sectors * stats->bytes_per_sector;

		wchar_t size[512];
		GetSizeString(total_bytes, size);

		printf("Cylinders:       %I64d\n", driveInfo.Cylinders.QuadPart);
		printf("Tracks/cylinder: %d\n", driveInfo.TracksPerCylinder);
		printf("Sectors/track:   %d\n", driveInfo.SectorsPerTrack);
		printf("Bytes/sector:    %d\n", driveInfo.BytesPerSector);
		printf("Total Sectors:   %I64d\n", total_sectors);
		printf("Total Bytes:     %I64d\n", total_bytes);
		printf("Size:            %S\n", size);
	}

	bResult = DeviceIoControl(
		hnd,
		IOCTL_DISK_GET_PARTITION_INFO,
		NULL,
		0,
		&diskInfo,
		sizeof(diskInfo),
		&dwResult,
		NULL);

	if (bResult) {
		last_sector = diskInfo.PartitionLength.QuadPart / stats->bytes_per_sector;
	}

	if (opt.end > last_sector) {
		_snprintf(err, sizeof(err), "Ending sector must be less than or equal to %I64d for %s", last_sector, device_name);
		FatalError(err);
	}

	if (opt.end == 0) {
		opt.end = last_sector;
	}

	if (opt.start > opt.end) {
		_snprintf(err, sizeof(err), "Ending sector must be greater than starting sector");
		FatalError(err);
	}

	if (opt.quiet < 2) {
		if (opt.start != 0 || opt.end != last_sector) {
			if (opt.quiet == 1) {
				printf("\n");
			}
			ULONGLONG bytes = (opt.end - opt.start) * stats->bytes_per_sector;
			wchar_t size[512];
			GetSizeString(bytes, size);
			printf("Processing sectors %I64d to %I64d (%I64d bytes) (%S)\n", opt.start, opt.end, bytes, size);
		}
	}

	if (opt.quiet < 1) {
		printf("\n");
	}

	unsigned int bytes_to_process = opt.sectors * stats->bytes_per_sector;

	unsigned char *sector_data = (unsigned char *) malloc(bytes_to_process + BYTES_PER_ELEMENT);

	if (opt.quiet == 0) {
		printf(HEADER, opt.kilobyte ? " MB" : "MiB");
	}

	for (unsigned int pass = 1; pass <= opt.passes; ++pass) {
		int byte_to_write;

		unsigned char chars[3];

		unsigned int n;
		int j;

		switch (opt.mode) {
			case WIPEMODE_NORMAL:
				if (bytes == 0) {
					byte_to_write = 0;
				} else {
					int n = (pass - 1) % bytes;
					byte_to_write = byte[n];
				}

				for (j = 0; j < BYTES_PER_ELEMENT; ++j) {
					chars[j] = (unsigned char) byte_to_write;
				}
				break;

			case WIPEMODE_DOD:
				n = (pass - 1) % dod_elements;
				byte_to_write = dod_bytes[n];
				for (j = 0; j < BYTES_PER_ELEMENT; ++j) {
					chars[j] = (unsigned char) byte_to_write;
				}
				break;

			case WIPEMODE_DOD7:
				n = (pass - 1) % dod7_elements;
				byte_to_write = dod7_bytes[n];
				for (j = 0; j < BYTES_PER_ELEMENT; ++j) {
					chars[j] = (unsigned char) byte_to_write;
				}
				break;

			case WIPEMODE_GUTMANN:
				n = (pass - 1) % gutmann_elements;
				byte_to_write = gutmann_bytes[n][0];
				for (j = 0; j < BYTES_PER_ELEMENT; ++j) {
					chars[j] = (unsigned char) gutmann_bytes[n][j];
				}
				break;
		}

		char s_byte[5];

		if (byte_to_write < 0) {
			switch (opt.random) {
				case RANDOM_PSEUDO:
					sprintf(s_byte, "prnd");
					break;
				case RANDOM_WINDOWS:
					sprintf(s_byte, "wrnd");
					break;
#ifdef HAVE_CRYPTOGRAPHIC
				case RANDOM_CRYPTOGRAPHIC:
					sprintf(s_byte, "crnd");
					break;
#endif
			}
		} else {
			sprintf(s_byte, "0x%02x", byte_to_write);
			for (unsigned int i = 0; i < stats->bytes_per_sector * opt.sectors / BYTES_PER_ELEMENT; i += BYTES_PER_ELEMENT) {
				for (int j = 0; j < BYTES_PER_ELEMENT; ++j) {
	 				sector_data[i + j] = chars[j];
				}
			}
		}

		ULONGLONG starting_byte = opt.start * stats->bytes_per_sector;

		LARGE_INTEGER li;

		li.QuadPart = starting_byte;

		SetLastError(0);
		DWORD dw = SetFilePointer(hnd, li.LowPart, &li.HighPart, FILE_BEGIN);

		if (GetLastError() != NO_ERROR) {
			_snprintf(err, sizeof(err), "Failed to seek to sector &I64d", opt.start);
			FatalError(err);
		}

		ULONGLONG last_ticks = get_ticks(stats);

		char *action = opt.read ? "read" : "write";

		unsigned long sectors_to_process = opt.sectors;

		for (ULONGLONG sector = opt.start; sector <= opt.end; sector += opt.sectors) {
			if (sector + sectors_to_process > opt.end) {
				sectors_to_process = (unsigned long) (opt.end - sector);
				bytes_to_process = sectors_to_process * stats->bytes_per_sector;
				if (bytes_to_process == 0) {
					break;
				}
			}

			if (!opt.read) {
				unsigned int i;

				if (byte_to_write < 0) {
					switch (opt.random) {
						case RANDOM_PSEUDO:
							for (i = 0; i < bytes_to_process; ++i) {
								sector_data[i] = (unsigned char) rand();
							}
							break;

						case RANDOM_WINDOWS:
							if (!CryptGenRandom(hProv, bytes_to_process, sector_data)) {
								_snprintf(err, sizeof(err), "CryptGenRandom error");
								FatalError(err);
							}
							break;

#ifdef HAVE_CRYPTOGRAPHIC
						case RANDOM_CRYPTOGRAPHIC:
#error "RANDOM_CRYPTOGRAPHIC has not been implemented yet"
							break;
#endif
					}
				}
			}

			DWORD dwBytes = 0;

			ULONGLONG before_ticks = get_ticks(stats);

			SetLastError(0);
			BOOL rv;
			if (opt.read) {
				rv = ReadFile(hnd, sector_data, bytes_to_process, &dwBytes, NULL);
			} else {
#ifdef DUMMY_WRITE
				rv = true;
				dwBytes = bytes_to_process;
#else
				rv = WriteFile(hnd, sector_data, bytes_to_process, &dwBytes, NULL);
#endif
			}

			ULONGLONG after_ticks = get_ticks(stats);
			stats->wiping_ticks += after_ticks - before_ticks;

			if (!rv || GetLastError()) {
				_snprintf(err, sizeof(err), "Failed to %s %d bytes at sectors %I64d-%I64d", action, bytes_to_process - dwBytes, sector, sector + sectors_to_process);
				FatalError(err);
			}

			if (opt.quiet < 2) {
				ULONGLONG seconds = (after_ticks - last_ticks) / stats->tick_frequency;

				if (seconds >= opt.refresh) {
					last_ticks = after_ticks;
					print_stats(pass, s_byte, sector, stats);
				}
			}
		}

		if (opt.quiet == 0) {
			print_stats(pass, s_byte, opt.end, stats);
		}

		if (opt.quiet < 2) {
			printf("\n");
			fflush(stdout);
		}
	}

	stats->all_wiping_ticks += stats->wiping_ticks;

	if (opt.quiet < 2) {
		printf("\n");
		print_ticks("Wiping time:  %s\n", stats->wiping_ticks, stats->tick_frequency);

		ULONGLONG elapsed_ticks = get_ticks(stats) - stats->start_ticks;
		print_ticks("Elapsed time: %s\n", elapsed_ticks, stats->tick_frequency);
		printf("\n");

//		printf("\n\nThe operation completed successfully!\n");
	}

	free(sector_data);

	CloseHandle(hnd);

	if (nDosLinkCreated == 0) {
		RemoveFakeDosName(device_name, szDosDevice);
	}

	return 0;
}

int main(int argc, char * argv[]) {
	int i;

	time_t t;
	time(&t);

	srand(t ^ _getpid());

	progname = basename(argv[0]);

	if (progname) {
		int len = strlen(progname);
		if (len > 4 && _stricmp(progname + len - 4, ".exe") == 0)
			progname[len - 4] = '\0';
	}

	opterr = 0;
	int option_index = 0;
	optind = 1;

#ifdef _DEBUG
	for (i = 0; i < argc; ++i) {
		printf("argv[%d]=%s\n", i, argv[i]);
	}
#endif
	while (true) {
		int i;
		if (optind < argc && argv[optind] && argv[optind][0] == '/')
			argv[optind][0] = '-';

		int c = getopt_long(argc, argv, short_options, long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 'l':
				opt.list = true;
				break;
			case 'p': /* -p | --passes n  Wipe device n times (default is 1) */
				opt.passes = atoi(optarg);
				if (opt.passes == 0 || opt.passes > 10000)
					usage(1);
				break;
			case 'd': /* -d | --dod       Wipe device using DoD 5220.22-M method (3 passes) */
				opt.mode = WIPEMODE_DOD;
				break;
			case 'D': /* -D | --dod7      Wipe device using DoD 5200.28-STD method (7 passes) */
				opt.mode = WIPEMODE_DOD7;
				break;
			case 'g': /* -g | --gutmann   Wipe device using Gutmann method (35 passes) */
				opt.mode = WIPEMODE_GUTMANN;
				break;
			case 'y':
				opt.yes = true;
				break;
			case '1':
				opt.random = RANDOM_PSEUDO;
				break;
			case '2':
				opt.random = RANDOM_WINDOWS;
				break;
#ifdef HAVE_CRYPTOGRAPHIC
			case '3':
				opt.random = RANDOM_CRYPTOGRAPHIC;
				break;
#endif
			case 'k':
				opt.kilobyte = true;
				break;
			case 'z':
				opt.refresh = atoi(optarg);
				break;
			case 'x':
				opt.restart = EXIT_NONE;
				for (i = 0; i < exit_modes; ++i) {
					if (strnicmp(exit_mode[i], optarg, strlen(optarg)) == 0) {
						opt.restart = (ExitMode) i;
						break;
					}
				}
				if (opt.restart == EXIT_NONE) {
					usage(1);
				}
/*
			case 'P':
				opt.restart = EXIT_POWEROFF;
				break;
			case 'S':
				opt.restart = EXIT_SHUTDOWN;
				break;
			case 'H':
				opt.restart = EXIT_HIBERNATE;
				break;
			case 'L':
				opt.restart = EXIT_LOGOFF;
				break;
			case 'R':
				opt.restart = EXIT_REBOOT;
				break;
			case 'T':
				opt.restart = EXIT_STANDBY;
				break;
*/
			case 'f':
				opt.force = true;
				break;
			case 'q': /* -q | --quiet     Display less information */
				++opt.quiet;
				break;
			case 'n': /* -n | --sectors n Write n sectors at once (default is %d) */
				opt.sectors = atoi(optarg);
				if (opt.sectors == 0)
					usage(1);
				if (opt.sectors >= 65536)
					usage(1);
				break;
			case 's': // -s | --start   n Start at sector n (default is first sector)
				opt.start = _atoi64(optarg);
				if (opt.start == (ULONGLONG) -1)
					usage(1);
				break;
			case 'e': // -e | --end     n End at sector n (default is last sector)
				opt.end = _atoi64(optarg);
				if (opt.end == 0)
					usage(1);
				break;
			case 'r': /* -r | --read      Only read the data on the device (DOES NOT WIPE!) */
				opt.read = true;
				break;
			case 'v': /* -v | --version   Show version and copyright information and quit */
				version();
				exit(0);
				break;
			case '?': /* -? | --help      Show this help message and quit */
				++opt.help;
				break;
			default:
				fprintf(stderr, "Invalid option: '%s'\n", optarg);
				usage(1);
				break;
		}
	}

	if (opt.help) {
		_usage();
		if (opt.help > 1) {
			examples();
		}
		exit(0);
	}

	if (opt.list) {
		list_devices();
		exit(0);
	}

	int devices = 0;
	int bytes = 0;

	for (i = optind; i < argc; ++i) {
#ifdef _DEBUG
		printf("argv[%d]=%s\n", i, argv[i]);
#endif
		if (strlen(argv[i]) == 2 && tolower(argv[i][0]) >= 'a' && tolower(argv[i][0]) <= 'z' && argv[i][1] == ':') {
			++devices;
			continue;
		}
		if (argv[i][0] == '\\') {
			++devices;
			continue;
		}
		++bytes;
	}

	if (devices == 0) {
		fprintf(stderr, "%s: No devices specified", progname);
		usage(1);
	}

	char **device	= (char **) malloc(sizeof(char *) * devices);
	int *byte		= NULL;

	if (bytes) {
		byte = (int *) malloc(sizeof(int) * bytes);
	}

	devices = 0;
	bytes = 0;
	for (i = optind; i < argc; ++i) {
		device[devices] = (char *) malloc((strlen(argv[i]) + 6) * sizeof(char));

		if (strlen(argv[i]) == 2 && tolower(argv[i][0]) >= 'a' && tolower(argv[i][0]) <= 'z' && argv[i][1] == ':') {
			sprintf(device[devices++], "\\\\.\\%c:", argv[i][0]);
			continue;
		}

		if (argv[i][0] == '\\') {
			strcpy(device[devices++], argv[i]);
			continue;
		}

		int byte_to_write = 0;
		while (1) {
			if (strnicmp(argv[i], "0x", 2) == 0) {
				if (sscanf(argv[i], "%x", &byte_to_write) == 0) {
					usage(1);
				}
				break;
			}

			if (argv[i][0] == '0') {
				if (sscanf(argv[i], "%o", &byte_to_write) == 0) {
					usage(1);
				}
				break;
			}

			if (toupper(argv[i][0]) == 'R') {
				byte_to_write = -1;
				break;
			}

			byte_to_write = atoi(argv[i]);

			if (byte_to_write > 0) {
				byte_to_write &= 0xff;
			}
			break;
		}
		byte[bytes++] = byte_to_write;
	}

	if (bytes == 0) {
		bytes = 1;
		byte = (int *) malloc(sizeof(int) * bytes);
		byte[0] = 0;
	}

	if (opt.passes == 0) {
		opt.passes = 1;
	}

	if (!opt.read && !opt.yes) {
		version();
		printf(
		  //          1         2         3         4         5         6         7
		  // 12345678901234567890123456789012345678901234567890123456789012345678901234567890
			"\n"
			"WARNING! You are about to permentently and irretrievably erase all data on the\n"
			"following device(s):\n\n");

		for (int i = 0; i < devices; ++i) {
			printf("%2d: %s\n", i + 1, device[i]);
		}

		printf("\nUsing %d iteration%s of the %s\n", opt.passes, opt.passes > 1 ? "s" : "", wipe_methods[opt.mode]);

		printf(
			"\n"
			"Once this process starts, the device%s will be unrecognizable by the Operating\n"
			"System, and will need to be reinitialized to again be usable.\n"
			"\n"
			"Are you sure you want to wipe %s (yes/no)? ",
			((devices > 1) ? "s" : ""),
			((devices > 1) ? "these devices" : "this device")
		);
		char buf[256];
		fgets(buf, sizeof(buf), stdin);
		if (strlen(buf) != 4 || strnicmp(buf, "yes", 3) != 0) {
			printf("Processing aborted");
			exit(0);
		}
	}

	switch (opt.mode) {
		case WIPEMODE_NORMAL:
			opt.passes *= bytes;
			break;
		case WIPEMODE_DOD:
			opt.passes *= dod_elements;
			break;
		case WIPEMODE_DOD7:
			opt.passes *= dod7_elements;
			break;
		case WIPEMODE_GUTMANN:
			opt.passes *= gutmann_elements;
			break;
		default:
			FatalError("Unimplemented mode");
	}

	if (opt.passes >= 10000) {
		usage(1);
	}

	HCRYPTPROV hProv = 0;
	HCRYPTKEY hKey = 0;

	if (opt.random == RANDOM_WINDOWS) {
		// Get a handle to the user default provider.
		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0)) {
			FatalError("CryptAcquireContext error");
		}

		// Create a random block cipher session key.
		if (!CryptGenKey(hProv, CALG_RC2, CRYPT_EXPORTABLE, &hKey)) {
			FatalError("CryptGenKey error");
		}

		// Set the cipher mode.
		DWORD dwMode = CRYPT_MODE_ECB;
		if (!CryptSetKeyParam(hKey, KP_MODE, (BYTE *) &dwMode, 0)) {
			FatalError("CryptSetKeyParam error");
		}

		BYTE pbData[16];

		// Generate a random initialization vector.
		if (!CryptGenRandom(hProv, 8, pbData)) {
			FatalError("CryptGenRandom error");
		}

		// Set the initialization vector.
		if (!CryptSetKeyParam(hKey, KP_IV, pbData, 0)) {
			FatalError("CryptSetKeyParam error");
		}
	}

	t_stats stats = {
		0,
		0,
		0,
		0,
		NULL
	};

	stats.all_start_ticks = get_ticks(&stats);

	for (i = 0; i < devices; ++i) {
		wipe_device(device[i], bytes, byte, &stats, hKey, hProv);
	}

	if (opt.quiet < 2 && devices > 1) {
		print_ticks("Total wiping time:  %s\n", stats.all_wiping_ticks, stats.tick_frequency);

		ULONGLONG elapsed_ticks = get_ticks(&stats) - stats.all_start_ticks;
		print_ticks("Total elapsed time: %s\n", elapsed_ticks, stats.tick_frequency);
	}

	for (i = 0; i < devices; ++i) {
		free(device[i]);
	}

	free(device);
	free(byte);

	if (opt.random == RANDOM_WINDOWS) {
		CryptDestroyKey(hKey);
		CryptReleaseContext(hProv, 0);
	}

	switch (opt.restart) {
		case EXIT_NONE:
			return 0;

		case EXIT_HIBERNATE:
			if (SetSystemPowerState(FALSE, opt.force) == 0) {
				FatalError("Unable to hibernate");
			}
			return 0;

		case EXIT_STANDBY:
			if (SetSystemPowerState(TRUE, opt.force) == 0) {
				FatalError("Unable to enter standby");
			}
			return 0;
	}

	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	ZeroMemory(&tkp, sizeof(tkp));

	// Get a token for this process.
	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		FatalError("Unable to get shutdown privileges");
	}

	// Get the LUID for the shutdown privilege.
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1;  // one privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process.
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	if (GetLastError() != ERROR_SUCCESS) {
		FatalError("Unable to get shutdown privileges");
	}

	DWORD options = 0;
	char *cmd;

	switch (opt.restart) {
		case EXIT_POWEROFF:
			cmd = "poweroff";
			options |= EWX_SHUTDOWN | EWX_POWEROFF;
			break;

		case EXIT_SHUTDOWN:
			cmd = "shutdown";
			options |= EWX_SHUTDOWN;
			break;

		case EXIT_LOGOFF:
			cmd = "logoff";
			options |= EWX_LOGOFF;
			break;

		case EXIT_REBOOT:
			cmd = "reboot";
			options |= EWX_REBOOT;
			break;
	}

	if (opt.force) {
		options |= EWX_FORCE;
	}

	if (!ExitWindowsEx(options,
		SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
		SHTDN_REASON_MINOR_OTHER |
		SHTDN_REASON_FLAG_PLANNED)) {
		char err[255];
		_snprintf(err, sizeof(err), "Unable to %s", cmd);
		FatalError(err);
	}

	return 0;
}
