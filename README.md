<!--
raw2iso/README.md

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
-->

raw2iso - Convert raw optical disk images to `.iso` files
=========================================================

Compiling
---------

The tool is a single `.c` file, and should compile and run in all
reasonable environments that come with a C99 compiler and standard
library. If it does not, please file an issue.

Windows binaries can be found in the
[releases](https://github.com/not-a-user/raw2iso/releases) section.

### Compiler command line example

~~~
cc -o raw2iso raw2iso.c 
~~~

### Compile directly from GitHub

~~~
wget -O - https://raw.githubusercontent.com/not-a-user/raw2iso/master/raw2iso.c | cc -x c -o raw2iso -
~~~

Usage
-----

~~~
raw2iso <track-mode> [<sub-channel-mode>] < INPUT.BIN > OUTPUT.ISO
~~~

-   `<track-mode>`: `MODE1_RAW` | `MODE1` | `MODE2_RAW` |
    `MODE2_FORM1` | `MODE2_FORM2` | `MODE2_FORM_MIX`
-   `<sub-channel-mode>`: `RW` | `RW_RAW`

**Warning**: Error correction/detection data is **not** checked (try
[edccchk](https://github.com/claunia/edccchk)) and errors are **not**
corrected! (Finding this type of errors in an image is unlikely, because
[CIRC](https://en.wikipedia.org/wiki/Cross-interleaved_Reed%E2%80%93Solomon_coding)
detects/corrects errors when reading from the disk.)

How to determine the image type (track mode and sub-channel mode)
-----------------------------------------------------------------

Common extensions for raw image files are `.raw`, `.bin`, `.img`,
`.ima`, and others.

### If you have an accompanying `.toc` file

The track mode and sub-channel mode can be found in the `.toc` file. See
[cdrdao(1)](https://linux.die.net/man/1/cdrdao) -> TOC FILES ->
Track Specification -> TRACK.

**Note:** `MODE1` and `MODE2_FORM1` images without sub-channel data need
no conversion. These images are `.iso` files.

### If you have an accompanying `.cue` file

-	`MODE1/2048`: No conversion is necessary. The image is an `.iso`
    file.
-   `MODE1/2352`: Use `MODE1_RAW`.
-   `MODE2/2336`: Use `MODE2_FORM_MIX`.
-   `MODE2/2352`: Use `MODE2_RAW`.

### By looking at the file size

If the file size is a multiple of the sector size

-   2048 bytes: The image might be an `.iso` file.
-   2324 bytes: Try `MODE2_FORM2`.
-   2336 bytes: Try `MODE2_FORM_MIX`.
-   2352 bytes: Try `MODE1_RAW` or `MODE2_RAW`.

If sub-channel data is present, the sector size is increased by 96
bytes:

-   2144 bytes: Try `MODE1 RW`.
-   2420 bytes: Try `MODE2_FORM2 RW`.
-   2432 bytes: Try `MODE2_FORM_MIX RW`.
-   2448 bytes: Try `MODE1_RAW RW` or `MODE2_RAW RW`.

Example - Create an `.iso` using `cdrdao`
-----------------------------------------

### Motivation

The standard method

~~~
dd if=/dev/cdrom of=image.iso bs=2048
~~~

can fail for various reasons.

### Step by step

Get `cdrdao`. (Windows users can get pre-compiled binaries
[here](https://www.student.tugraz.at/thomas.plank/).)

Then make a raw image of the disk:

~~~
cdrdao read-cd --read-raw --datafile image.bin image.toc
~~~

Look for the first `TRACK` line in the `image.toc` file to find out
which track mode to use. Then run `raw2iso` with the correct mode. The
following line automates these steps:

~~~
sh -c "raw2iso $(grep '^TRACK ' image.toc | head -1 | cut -d ' ' -f 2-) < image.bin > image.iso"
~~~
