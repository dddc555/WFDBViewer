/* file: rdsamp.c	G. Moody	 23 June 1983
			Last revised:  11 December 2017

-------------------------------------------------------------------------------
rdsamp: Print an arbitrary number of samples from each signal
Copyright (C) 1983-2010 George B. Moody

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, see <http://www.gnu.org/licenses/>.

You may contact the author by e-mail (wfdb@physionet.org) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

*/

#include <stdio.h>
#include "wfdb.h"

#ifdef c_plusplus	/* true for some other C++ compilers */
#define wfdb_CPP
#define wfdb_PROTO
#endif

/* Specify C linkage for C++ compilers. */
#ifdef __cplusplus
extern "C" {
#endif

/* values for timeunits */
#define SECONDS     1
#define MINUTES     2
#define HOURS       3
#define TIMSTR      4
#define MSTIMSTR    5
#define SHORTTIMSTR 6
#define HHMMSS	    7
#define SAMPLES     8

#define WFDBXMLPROLOG  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
 "<?xml-stylesheet type=\"text/xsl\"" \
 " href=\"wfdb.xsl\"?>\n" \
 "<!DOCTYPE wfdbsampleset PUBLIC \"-//PhysioNet//DTD WFDB 1.0//EN\"" \
 " \"http://physionet.org/physiobank/database/XML/wfdb.dtd\">\n"

int convertWFDB2CSV(int argc, char *record, char *output);

char *myescapify(char *s);
#ifdef __cplusplus
}
#endif
