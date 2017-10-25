/*
raw2iso/raw2iso.c

Copyright 2017 not-a-user

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifdef NDEBUG
#	error "assert() required for side-effects"
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#	include <fcntl.h>
#	include <io.h>
#	define SET_BINARY_MODE(fd) assert(-1 != _setmode(fd, _O_BINARY))
#else
#	define SET_BINARY_MODE(fd) assert(1)
#endif

#define RAW2ISO_VERSION "0.0.2"

static void usage(char const * argv0)
{
	fprintf(stderr,
"Convert raw optical disk images to iso files.\n"
"\n"
"Usage: %s <track-mode> [<sub-channel-mode>] < INPUT.BIN > OUTPUT.ISO\n"
"\n"
"<track-mode>: MODE1_RAW | MODE1 | MODE2_RAW | MODE2_FORM1 | "
	"MODE2_FORM2 | MODE2_FORM_MIX\n"
"<sub-channel-mode>: RW | RW_RAW\n"
"\n"
"(See cdrdao(1) -> TOC FILES -> Track Specification -> TRACK.)\n"
"\n"
"WARNING: Error correction/detection data is NOT checked (try\n"
"https://github.com/claunia/edccchk) and errors are NOT corrected!\n"
"\n"
"raw2iso %s - https://github.com/not-a-user/raw2iso\n",
	argv0, RAW2ISO_VERSION);
}

// https://en.wikipedia.org/wiki/CD-ROM#CD-ROM_format
// https://en.wikipedia.org/wiki/CD-ROM#CD-ROM_XA_extension

#define SECTOR_SIZE 2048

// default: MODE1_RAW
size_t sector_raw_size = 2352;
size_t sector_offset = 16;

static void parse_args(int const argc, char const * argv[])
{
	assert(argc > 0);

	if (argc == 1 || argc > 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	} else {
		char const * const track_mode = argv[1];
		if (! strcmp("MODE1_RAW", track_mode)) {
			; // default
		} else if (
			! strcmp("MODE1", track_mode) ||
			! strcmp("MODE2_FORM1", track_mode)
		) {
			sector_raw_size = 2048;
			sector_offset = 0;
		} else if (! strcmp("MODE2_RAW", track_mode)) {
			sector_offset = 24;
		} else if (! strcmp("MODE2_FORM2", track_mode)) {
			sector_raw_size = 2324;
			sector_offset = 0;
		} else if (! strcmp("MODE2_FORM_MIX", track_mode)) {
			sector_raw_size = 2336;
			sector_offset = 8;
		} else {
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
		if (argc == 3) {
			char const * const sub_channel_mode = argv[2];
			if (
				! strcmp("RW", sub_channel_mode) ||
				! strcmp("RW_RAW", sub_channel_mode)
			) {
				sector_raw_size += 96;
			} else {
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
		}
	}
}

int main(int const argc, char const * argv[])
{
	parse_args(argc, argv);
	assert(sector_raw_size >= SECTOR_SIZE + sector_offset);

	SET_BINARY_MODE(_fileno(stdin));
	SET_BINARY_MODE(_fileno(stdout));

	uint8_t sector[sector_raw_size];

	for(unsigned k = 0; ; k++) {
		size_t const got = fread(sector, 1, sector_raw_size, stdin);
		assert(! ferror(stdin));
		if (got == 0) {
			fprintf(stderr, "converted %u sectors\n", k);
			assert(feof(stdin));
			exit(EXIT_SUCCESS);
		} else if (got == sector_raw_size) {
			assert(fwrite(& sector[sector_offset], 1, SECTOR_SIZE,
				stdout) == SECTOR_SIZE);
			assert(! ferror(stdout));
		} else {
			fprintf(stderr, "input is corrupted\n");
			assert(feof(stdin));
			exit(EXIT_FAILURE);
		}
	}
}
