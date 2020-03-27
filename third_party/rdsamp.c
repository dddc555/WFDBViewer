
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
#include "rdsamp.h"

char *pname;

int convertWFDB2CSV(int argc, char *record, char *output)
{
    char *argv[10];
    char *search = NULL;
    char *invalid, *snfmt, *tfmt, *tnfmt, *tufmt, *vfmt, speriod[16], tustr[16];
    int cflag = 1, highres = 0, i, isiglist, nsig, nosig = 0, pflag = 1, s,
    *sig = NULL, timeunits = SECONDS, vflag = 0, xflag = 0;
    WFDB_Frequency freq;
    WFDB_Sample *v;
    WFDB_Siginfo *si;
    WFDB_Time from = 0L, maxl = 0L, to = 0L;

    FILE *fp = freopen(output, "w", stdout);
    if(fp == NULL) return  -1;
//    void help();

//    pname = prog_name(argv[0]);
//    for (i = 1 ; i < argc; i++) {
//    if (*argv[i] == '-') switch (*(argv[i]+1)) {
//      case 'c':	/* output in CSV format */
//        cflag = 1;
//        break;
//      case 'f':	/* starting time */
//        if (++i >= argc) {
////		(void)fprintf(stderr, "%s: time must follow -f\n", pname);
//        exit(1);
//        }
//        from = i;
//        break;
//      case 'h':	/* help requested */
////        help();
//        exit(0);
//        break;
//      case 'H':	/* select high-resolution mode */
//        highres = 1;
//        break;
//      case 'l':	/* maximum length of output follows */
//        if (++i >= argc) {
//        (void)fprintf(stderr, "%s: max output length must follow -l\n",
//                  pname);
//        exit(1);
//        }
//        maxl = i;
//        break;
//      case 'P':	/* output in high-precision physical units */
//        ++pflag;	/* (fall through to case 'p') */
//      case 'p':	/* output in physical units specified */
//        ++pflag;
//        if (*(argv[i]+2) == 'd') timeunits = TIMSTR;
//        else if (*(argv[i]+2) == 'e') timeunits = HHMMSS;
//        else if (*(argv[i]+2) == 'h') timeunits = HOURS;
//        else if (*(argv[i]+2) == 'm') timeunits = MINUTES;
//        else if (*(argv[i]+2) == 'S') timeunits = SAMPLES;
//        else timeunits = SECONDS;
//        break;
//      case 'r':	/* record name */
//        if (++i >= argc) {
//        (void)fprintf(stderr, "%s: record name must follow -r\n",
//                  pname);
//        exit(1);
//        }
//        record = argv[i];
//        break;
//      case 's':	/* signal list follows */
//        isiglist = i+1; /* index of first argument containing a signal # */
//        while (i+1 < argc && *argv[i+1] != '-') {
//        i++;
//        nosig++;	/* number of elements in signal list */
//        }
//        if (nosig == 0) {
//        (void)fprintf(stderr, "%s: signal list must follow -s\n",
//            pname);
//        exit(1);
//        }
//        break;
//      case 'S':	/* search for valid sample of specified signal */
//        if (++i >= argc) {
//        (void)fprintf(stderr,
//                  "%s: signal name or number must follow -S\n",
//                  pname);
//        exit(1);
//        }
//        search = argv[i];
//        break;
//      case 't':	/* end time */
//        if (++i >= argc) {
//        (void)fprintf(stderr, "%s: time must follow -t\n",pname);
//        exit(1);
//        }
//        to = i;
//        break;
//      case 'v':	/* verbose output -- include column headings */
//        vflag = 1;
//        break;
//      case 'X':	/* output in WFDB-XML format */
//        xflag = cflag = vflag = 1; /* format is CSV inside an XML wrapper */
//        break;
//      default:
//        (void)fprintf(stderr, "%s: unrecognized option %s\n", pname,
//              argv[i]);
//        exit(1);
//    }
//    else {
//        (void)fprintf(stderr, "%s: unrecognized argument %s\n", pname,
//              argv[i]);
//        exit(1);
//    }
//    }
    if (record == NULL) {
//    help();
    exit(1);
    }
    if ((nsig = isigopen(record, NULL, 0)) <= 0) {
        wfdb_error("Exit %d\n",  nsig);
        exit(2);
    }
    wfdb_error("nsig = %d\n",  nsig);
    if ((v = (WFDB_Sample*)malloc(nsig * sizeof(WFDB_Sample))) == NULL ||
    (si = (WFDB_Siginfo*)malloc(nsig * sizeof(WFDB_Siginfo))) == NULL) {
        (void)fprintf(stderr, "%s: insufficient memory\n", pname);
        exit(2);
    }
    if ((nsig = isigopen(record, si, nsig)) <= 0){
        wfdb_error("Exit2 %d\n",  nsig);
//        exit(2);
        return -1;
    }

    for (i = 0; i < nsig; i++)
    if (si[i].gain == 0.0) si[i].gain = WFDB_DEFGAIN;
    if (highres)
        setgvmode(WFDB_HIGHRES);
    freq = sampfreq(NULL);
    if (from > 0L && (from = strtim(argv[from])) < 0L)
    from = -from;
    if (isigsettime(from) < 0)
    exit(2);
    if (to > 0L && (to = strtim(argv[to])) < 0L)
    to = -to;
    if (nosig) {		/* print samples only from specified signals */
    if ((sig = (int *)malloc((unsigned)nosig*sizeof(int))) == NULL) {
        (void)fprintf(stderr, "%s: insufficient memory\n", pname);
        exit(2);
    }
    for (i = 0; i < nosig; i++) {
        if ((s = findsig(argv[isiglist+i])) < 0) {
        (void)fprintf(stderr, "%s: can't read signal '%s'\n", pname,
                  argv[isiglist+i]);
        exit(2);
        }
        sig[i] = s;
    }
    nsig = nosig;
    }
    else {			/* print samples from all signals */
    if ((sig = (int *)malloc((unsigned)nsig*sizeof(int))) == NULL) {
        (void)fprintf(stderr, "%s: insufficient memory\n", pname);
        exit(2);
    }
    for (i = 0; i < nsig; i++)
        sig[i] = i;
    }

    /* Reset 'from' if a search was requested. */
    if (search &&
    ((s = findsig(search)) < 0 || (from = tnextvec(s, from)) < 0)) {
    (void)fprintf(stderr, "%s: can't read signal '%s'\n", pname, search);
    exit(2);
    }

    /* Reset 'to' if a duration limit was specified. */
    if (maxl > 0L && (maxl = strtim(argv[maxl])) < 0L)
    maxl = -maxl;
    if (maxl && (to == 0L || to > from + maxl))
    to = from + maxl;

    /* Adjust timeunits if starting time or date is undefined. */
    if (timeunits == TIMSTR) {
    char *p = timstr(0L);
    if (*p != '[') timeunits = HHMMSS;
    else if (strlen(p) < 16) timeunits = SHORTTIMSTR;
    else if (freq > 1.0) timeunits = MSTIMSTR;
    }
    if (timeunits == HOURS) freq *= 3600.;
    else if (timeunits == MINUTES) freq *= 60.;

    /* Set formats for output. */
    if (cflag) {  /* CSV output selected */
    snfmt = ",'%s'";
    if (pflag) { 	/* output in physical units */
        switch (timeunits) {
          case SAMPLES:     tnfmt = "'sample interval'";
                        sprintf(tustr, "'%g sec'", 1./freq);
                tufmt = tustr; break;
          case SHORTTIMSTR: tnfmt = "'Time'";
                        tufmt = "'hh:mm:ss.mmm'"; break;
          case TIMSTR:      tnfmt = "'Time and date'";
                        tufmt = "'hh:mm:ss dd/mm/yyyy'"; break;
          case MSTIMSTR:    tnfmt = "'Time and date'";
                        tufmt = "'hh:mm:ss.mmm dd/mm/yyyy'"; break;
          case HHMMSS:      tnfmt = "'Elapsed time'";
                        tufmt = "'hh:mm:ss.mmm'"; break;
          case HOURS:       tnfmt = "'Elapsed time'";
                        tufmt = "'hours'"; break;
          case MINUTES:     tnfmt = "'Elapsed time'";
                        tufmt = "'minutes'"; break;
          default:
          case SECONDS:     tnfmt = "'Elapsed time'";
                        tufmt = "'seconds'"; break;
        }
        invalid = ",-";
        if (pflag > 1)	/* output in high-precision physical units */
        vfmt = "%.8lf";
        else
        vfmt = "%.3lf";
    }
    else {	/* output in raw units */
        tnfmt = "'sample #'";
        tfmt = "%ld";
        vfmt = "%d";
    }
    }
    else {	/* output in tab-separated columns selected */
    if (pflag) {	/* output in physical units */
        switch (timeunits) {
          case SAMPLES:     tnfmt = "sample interval";
                        sprintf(speriod, "(%g", 1./freq);
                                speriod[10] = '\0';
                        sprintf(tustr, "%10s sec)", speriod);
                tufmt = tustr; break;
          case SHORTTIMSTR: tnfmt = "     Time";
                        tufmt = "(hh:mm:ss.mmm)"; break;
          case TIMSTR:	tnfmt = "   Time      Date    ";
                        tufmt = "(hh:mm:ss dd/mm/yyyy)"; break;
          case MSTIMSTR:    tnfmt = "      Time      Date    ";
                        tufmt = "(hh:mm:ss.mmm dd/mm/yyyy)"; break;
          case HHMMSS:      tnfmt = "   Elapsed time";
                        tufmt = "   hh:mm:ss.mmm"; break;
          case HOURS:       tnfmt = "   Elapsed time";
                        tufmt = "        (hours)"; break;
          case MINUTES:     tnfmt = "   Elapsed time";
                        tufmt = "      (minutes)"; break;
          default:
          case SECONDS:     tnfmt = "   Elapsed time";
                        tufmt = "      (seconds)"; break;
        }
        if (pflag > 1) {	/* output in high-precision physical units */
        snfmt = "\t%15s";
        invalid = "\t              -";
        vfmt = "\t%15.8lf";
        }
        else {
        snfmt = "\t%7s";
        invalid = "\t      -";
        vfmt = "\t%7.3lf";
        }
    }
    else {	/* output in raw units */
        snfmt = "\t%7s";
        tnfmt = "       sample #";
        tfmt = "%15ld";
        vfmt = "\t%7d";
    }
    }

    /* Print WFDB-XML prolog if '-x' option selected. */
    if (xflag) {
    printf(WFDBXMLPROLOG);
    printf("<wfdbsampleset>\n"
           "<samplingfrequency>%g</samplingfrequency>\n"
           "<signals>%d</signals>\n<description>",
           freq, nsig);
    }
    /* Print column headers if '-v' option selected. */
    if (vflag) {
    char *p, *t;
    int j, l;

    (void)printf("%s", tnfmt);

    for (i = 0; i < nsig; i++) {
        /* Check if a default signal description was provided by looking
           for the string ", signal " in the desc field.  If so, replace it
           with a shorter string. */
        p = si[sig[i]].desc;
        if (strstr(p, ", signal ")) {
        char *t;
        if (t = malloc(10*sizeof(char))) {
            (void)sprintf(t, "sig %d", sig[i]);
            p = t;
        }
        }
        if (cflag == 0) {
        l = strlen(p);
        if (pflag > 1) {
            if (l > 15) p += l - 15;
        }
        else {
            if (l > 7) p+= l - 7;
        }
        }
        else
        p = myescapify(p);
        (void)printf(snfmt, p);
    }
    if (xflag) (void)printf("</description>");
    (void)printf("\n");
    }

    /* Print data in physical units if '-p' option selected. */
    if (pflag) {
        char *p;


        /* Print units as a second line of column headers if '-v' selected. */
        if (vflag) {
            char s[12];

            if (xflag) (void)printf("<units>");
            (void)printf("%s", tufmt);

            for (i = 0; i < nsig; i++) {
                p = si[sig[i]].units;
                if (p == NULL) p = "mV";
                if (cflag == 0) {
                    char ustring[16];
                    int len;

                    len = strlen(p);
                    if (pflag > 1) { if (len > 13) len = 13; }
                    else if (len > 5) len = 5;
                    ustring[0] = '(';
                    strncpy(ustring+1, p, len);
                    ustring[len+1] = '\0';
                    (void)printf(pflag > 1 ? "\t%14s)" : "\t%6s)", ustring);
                }
                else {
                    p = myescapify(p);
                    (void)printf(",'%s'", p);
                }
            }
            if (xflag) (void)printf("</units>");
            (void)printf("\n");
        }
        if(nsig == 1)
            (void)printf("ECG (mv)\n");
        if(nsig == 3)
            (void)printf("X,Y,Z\n");


        if (xflag) (void)printf("<samplevectors>\n");
        while ((to == 0L || from < to) && getvec(v) >= 0) {
            if (cflag == 0) {
                switch (timeunits) {
                case TIMSTR:   (void)printf("%s", timstr(-from)); break;
                case SHORTTIMSTR:
                case MSTIMSTR: (void)printf("%s", mstimstr(-from)); break;
                case HHMMSS:   (void)printf("%15s", from == 0L ?
                                                "0:00.000" : mstimstr(from)); break;
                case SAMPLES:  (void)printf("%15ld", from); break;
                default:
                case SECONDS:  (void)printf("%15.3lf",(double)from/freq); break;
                case MINUTES:  (void)printf("%15.5lf",(double)from/freq); break;
                case HOURS:    (void)printf("%15.7lf",(double)from/freq); break;
                }
            }
            else {
//                switch (timeunits) {
//                case TIMSTR:
//                    for (p = timstr(-from); *p == ' '; p++)
//                        ;
//                    (void)printf("'%s'", p); break;
//                case SHORTTIMSTR:
//                case MSTIMSTR:
//                    for (p = mstimstr(-from); *p == ' '; p++)
//                        ;
//                    (void)printf("'%s'", p); break;
//                case HHMMSS:
//                    if (from == 0L) printf("'0:00.000'");
//                    else {
//                        for (p = mstimstr(from); *p == ' '; p++)
//                            ;
//                        (void)printf("'%s'", p); break;
//                    }
//                    break;
//                case SAMPLES:  (void)printf("%ld", from); break;
//                default:
//                case SECONDS:  (void)printf("%.3lf",(double)from/freq); break;
//                case MINUTES:  (void)printf("%.5lf",(double)from/freq); break;
//                case HOURS:    (void)printf("%.7lf",(double)from/freq); break;
//                }
            }

            from++;
            for (i = 0; i < nsig; i++) {
                if (v[sig[i]] != WFDB_INVALID_SAMPLE){
                    (void)printf(vfmt, ((double)v[sig[i]] - si[sig[i]].baseline)/si[sig[i]].gain);
                    if(i < nsig - 1) (void)printf(",");

                }
                else
                    (void)printf("%s", invalid);
            }
            (void)printf("\n");
        }
    }

    else {	/* output in raw units */
        if (xflag) (void)printf("<samplevectors>\n");
        while ((to == 0L || from < to) && getvec(v) >= 0) {
            (void)printf(tfmt, from++);
            for (i = 0; i < nsig; i++)
                (void)printf(vfmt, v[sig[i]]);
            (void)printf("\n");
        }
    }

    if (xflag)		/* print trailer if WFDB-XML output was selected */
        printf("</samplevectors>\n</wfdbsampleset>\n");
    fclose(fp);
    //    exit(0);
}


int readWFDB(char *record, double *output, int *outSize)
{
    *outSize = 9;
    output = (double *)malloc(sizeof(double) * (9));
    int i;
    for(i = 0;i<outSize;i++){
        output[i] = (double)i;
    }
    return 1;
    char *argv[10];
    char *search = NULL;
    char *invalid, *snfmt, *tfmt, *tnfmt, *tufmt, *vfmt, speriod[16], tustr[16];
    int cflag = 1, highres = 0,  isiglist, nsig, nosig = 0, pflag = 1, s,
    *sig = NULL, timeunits = SECONDS, vflag = 0, xflag = 0;
    WFDB_Frequency freq;
    WFDB_Sample *v;
    WFDB_Siginfo *si;
    WFDB_Time from = 0L, maxl = 0L, to = 0L;

    if (record == NULL) {
        exit(1);
    }
    if ((nsig = isigopen(record, NULL, 0)) <= 0) {
        wfdb_error("Exit %d\n",  nsig);
        exit(2);
    }
    wfdb_error("nsig = %d\n",  nsig);
    if ((v = (WFDB_Sample*)malloc(nsig * sizeof(WFDB_Sample))) == NULL ||
    (si = (WFDB_Siginfo*)malloc(nsig * sizeof(WFDB_Siginfo))) == NULL) {
        (void)fprintf(stderr, "%s: insufficient memory\n", pname);
        exit(2);
    }
    if ((nsig = isigopen(record, si, nsig)) <= 0){
        wfdb_error("Exit2 %d\n",  nsig);
//        exit(2);
        return -1;
    }

    for (i = 0; i < nsig; i++)
    if (si[i].gain == 0.0) si[i].gain = WFDB_DEFGAIN;
    if (highres)
        setgvmode(WFDB_HIGHRES);
    freq = sampfreq(NULL);
    if (from > 0L && (from = strtim(argv[from])) < 0L)
    from = -from;
    if (isigsettime(from) < 0)
    exit(2);
    if (to > 0L && (to = strtim(argv[to])) < 0L)
    to = -to;
    if (nosig) {		/* print samples only from specified signals */
    if ((sig = (int *)malloc((unsigned)nosig*sizeof(int))) == NULL) {
        (void)fprintf(stderr, "%s: insufficient memory\n", pname);
        exit(2);
    }
    for (i = 0; i < nosig; i++) {
        if ((s = findsig(argv[isiglist+i])) < 0) {
        (void)fprintf(stderr, "%s: can't read signal '%s'\n", pname,
                  argv[isiglist+i]);
        exit(2);
        }
        sig[i] = s;
    }
    nsig = nosig;
    }
    else {			/* print samples from all signals */
    if ((sig = (int *)malloc((unsigned)nsig*sizeof(int))) == NULL) {
        (void)fprintf(stderr, "%s: insufficient memory\n", pname);
        exit(2);
    }
    for (i = 0; i < nsig; i++)
        sig[i] = i;
    }

    /* Reset 'from' if a search was requested. */
    if (search &&
    ((s = findsig(search)) < 0 || (from = tnextvec(s, from)) < 0)) {
    (void)fprintf(stderr, "%s: can't read signal '%s'\n", pname, search);
    exit(2);
    }

    /* Reset 'to' if a duration limit was specified. */
    if (maxl > 0L && (maxl = strtim(argv[maxl])) < 0L)
    maxl = -maxl;
    if (maxl && (to == 0L || to > from + maxl))
    to = from + maxl;

    /* Adjust timeunits if starting time or date is undefined. */
    if (timeunits == TIMSTR) {
    char *p = timstr(0L);
    if (*p != '[') timeunits = HHMMSS;
    else if (strlen(p) < 16) timeunits = SHORTTIMSTR;
    else if (freq > 1.0) timeunits = MSTIMSTR;
    }
    if (timeunits == HOURS) freq *= 3600.;
    else if (timeunits == MINUTES) freq *= 60.;

    /* Set formats for output. */
    if (cflag) {  /* CSV output selected */
    snfmt = ",'%s'";
    if (pflag) { 	/* output in physical units */
        switch (timeunits) {
          case SAMPLES:     tnfmt = "'sample interval'";
                        sprintf(tustr, "'%g sec'", 1./freq);
                tufmt = tustr; break;
          case SHORTTIMSTR: tnfmt = "'Time'";
                        tufmt = "'hh:mm:ss.mmm'"; break;
          case TIMSTR:      tnfmt = "'Time and date'";
                        tufmt = "'hh:mm:ss dd/mm/yyyy'"; break;
          case MSTIMSTR:    tnfmt = "'Time and date'";
                        tufmt = "'hh:mm:ss.mmm dd/mm/yyyy'"; break;
          case HHMMSS:      tnfmt = "'Elapsed time'";
                        tufmt = "'hh:mm:ss.mmm'"; break;
          case HOURS:       tnfmt = "'Elapsed time'";
                        tufmt = "'hours'"; break;
          case MINUTES:     tnfmt = "'Elapsed time'";
                        tufmt = "'minutes'"; break;
          default:
          case SECONDS:     tnfmt = "'Elapsed time'";
                        tufmt = "'seconds'"; break;
        }
        invalid = ",-";
        if (pflag > 1)	/* output in high-precision physical units */
        vfmt = "%.8lf";
        else
        vfmt = "%.3lf";
    }
    else {	/* output in raw units */
        tnfmt = "'sample #'";
        tfmt = "%ld";
        vfmt = "%d";
    }
    }
    else {	/* output in tab-separated columns selected */
    if (pflag) {	/* output in physical units */
        switch (timeunits) {
          case SAMPLES:     tnfmt = "sample interval";
                        sprintf(speriod, "(%g", 1./freq);
                                speriod[10] = '\0';
                        sprintf(tustr, "%10s sec)", speriod);
                tufmt = tustr; break;
          case SHORTTIMSTR: tnfmt = "     Time";
                        tufmt = "(hh:mm:ss.mmm)"; break;
          case TIMSTR:	tnfmt = "   Time      Date    ";
                        tufmt = "(hh:mm:ss dd/mm/yyyy)"; break;
          case MSTIMSTR:    tnfmt = "      Time      Date    ";
                        tufmt = "(hh:mm:ss.mmm dd/mm/yyyy)"; break;
          case HHMMSS:      tnfmt = "   Elapsed time";
                        tufmt = "   hh:mm:ss.mmm"; break;
          case HOURS:       tnfmt = "   Elapsed time";
                        tufmt = "        (hours)"; break;
          case MINUTES:     tnfmt = "   Elapsed time";
                        tufmt = "      (minutes)"; break;
          default:
          case SECONDS:     tnfmt = "   Elapsed time";
                        tufmt = "      (seconds)"; break;
        }
        if (pflag > 1) {	/* output in high-precision physical units */
        snfmt = "\t%15s";
        invalid = "\t              -";
        vfmt = "\t%15.8lf";
        }
        else {
        snfmt = "\t%7s";
        invalid = "\t      -";
        vfmt = "\t%7.3lf";
        }
    }
    else {	/* output in raw units */
        snfmt = "\t%7s";
        tnfmt = "       sample #";
        tfmt = "%15ld";
        vfmt = "\t%7d";
    }
    }

    /* Print column headers if '-v' option selected. */
    if (vflag) {
    char *p, *t;
    int j, l;

    (void)printf("%s", tnfmt);

    for (i = 0; i < nsig; i++) {
        /* Check if a default signal description was provided by looking
           for the string ", signal " in the desc field.  If so, replace it
           with a shorter string. */
        p = si[sig[i]].desc;
        if (strstr(p, ", signal ")) {
        char *t;
        if (t = malloc(10*sizeof(char))) {
            (void)sprintf(t, "sig %d", sig[i]);
            p = t;
        }
        }
        if (cflag == 0) {
        l = strlen(p);
        if (pflag > 1) {
            if (l > 15) p += l - 15;
        }
        else {
            if (l > 7) p+= l - 7;
        }
        }
        else
        p = myescapify(p);
        (void)printf(snfmt, p);
    }

    (void)printf("\n");
    }

    /* Print data in physical units if '-p' option selected. */
    if (pflag) {
        char *p;

        /* Print units as a second line of column headers if '-v' selected. */
        if (vflag) {
            char s[12];
            (void)printf("%s", tufmt);

            for (i = 0; i < nsig; i++) {
                p = si[sig[i]].units;
                if (p == NULL) p = "mV";
                if (cflag == 0) {
                    char ustring[16];
                    int len;

                    len = strlen(p);
                    if (pflag > 1) { if (len > 13) len = 13; }
                    else if (len > 5) len = 5;
                    ustring[0] = '(';
                    strncpy(ustring+1, p, len);
                    ustring[len+1] = '\0';
                    (void)printf(pflag > 1 ? "\t%14s)" : "\t%6s)", ustring);
                }
                else {
                    p = myescapify(p);
                    (void)printf(",'%s'", p);
                }
            }

            (void)printf("\n");
        }
        if(nsig == 1)
            (void)printf("ECG (mv)\n");
        if(nsig == 3)
            (void)printf("X,Y,Z\n");



        while ((to == 0L || from < to) && getvec(v) >= 0) {
            if (cflag == 0) {
                switch (timeunits) {
                case TIMSTR:   (void)printf("%s", timstr(-from)); break;
                case SHORTTIMSTR:
                case MSTIMSTR: (void)printf("%s", mstimstr(-from)); break;
                case HHMMSS:   (void)printf("%15s", from == 0L ?
                                                "0:00.000" : mstimstr(from)); break;
                case SAMPLES:  (void)printf("%15ld", from); break;
                default:
                case SECONDS:  (void)printf("%15.3lf",(double)from/freq); break;
                case MINUTES:  (void)printf("%15.5lf",(double)from/freq); break;
                case HOURS:    (void)printf("%15.7lf",(double)from/freq); break;
                }
            }

            from++;
            for (i = 0; i < nsig; i++) {
                if (v[sig[i]] != WFDB_INVALID_SAMPLE){
                    (void)printf(vfmt, ((double)v[sig[i]] - si[sig[i]].baseline)/si[sig[i]].gain);
                    if(i < nsig - 1) (void)printf(",");

                }
                else (void)printf("%s", invalid);
            }
            (void)printf("\n");
        }
    }
}

char *myescapify(char *s)
{
    char *p = s, *q = s, *r;
    int c = 0;

    while (*p) {
    if (*p == '\'' || *p == '\\')
        c++;
    p++;
    }
    if (c > 0 && (p = r = (char *)calloc(p-s+c, sizeof(char))) != NULL) {
    while (*q) {
        if (*q == '\'' || *q == '\\')
        *q++ = '\\';
        *p++ = *q;
    }
    q = r;
    }
    return (q);
}
