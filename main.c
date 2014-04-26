/******************************************************************************
 * main.c 
 *
 * fifot - FIFO sniffing utility
 *
 * Copyright (C) 2014 Matt Davis (enferex) <mattdavis9@gmail.com>
 *
 * main.c is part of fifot.
 * fifot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * fifot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fifot.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/


#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>


static void usage(const char *execname)
{
    //printf("Usage: %s [-f fifo] [-n secs] [-h]\n"
    printf("Usage: %s [-f fifo] [-h]\n"
           "  -f fifo | Path to fifo file to monitor\n"
           "  -d      | Display the contents of the fifo to stdout\n"
           "  -h      | This help message\n",
           execname);
    exit(EXIT_SUCCESS);
}


/* Signal handler */
unsigned n_lines = 0, n_bytes = 0;
static void handler(int signum)
{
    printf("fifot: Lines: %u, Bytes: %u\n", n_lines, n_bytes);
    exit(EXIT_SUCCESS);
}


/* If 'dump' is set, the contents of the fifo will be displayed to stdout */
static void monitor(FILE *in, FILE *out, time_t report_rate, _Bool dump)
{
    struct stat st;
    time_t last_mod; //now, last_report;
    char line[1024] = {0};
    int fd = fileno(in);

    printf("Collecting fifo stats...\n");
    for ( ;; )
    {
        /* Report statistics to stdout */
/*        
        now = time(NULL);
        if ((now - last_report) >= report_rate)
        {
            printf("fifot: Lines: %u Bytes %u\n", n_lines, n_bytes);
            fflush(stdout);
            last_report = now;
        }
*/

        /* New input! Read it before the other processes do!
         * Assumes line buffered (delimited by a newline)
         */
        n_bytes += strlen(fgets(line, sizeof(line), in));
        ++n_lines;

        /* Write it back, so that the other processes can read it */
        if (dump)
          printf(line);
        fprintf(out, line);
        fflush(out);
        fstat(fd, &st);
        last_mod = st.st_mtime;
        
        /* Don't read the line back that we just wrote! */
        fstat(fileno(in), &st);
        last_mod = st.st_mtime;
        while (st.st_mtime == last_mod)
          fstat(fileno(in), &st);
    }
}


int main(int argc, char **argv)
{
    int rate, opt;
    FILE *in, *out;
    _Bool dump_to_stdout;
    const char *fname;

    /* Args */
    rate = 1;
    fname = NULL;
    dump_to_stdout = false;
    while ((opt = getopt(argc, argv, "f:n:dh")) != -1)
    {
        switch (opt)
        {
            case 'f':
                fname = optarg;
                break;
/*
            case 'n':
                rate = atoi(optarg);
                break;
*/

            case 'd':
                dump_to_stdout = true;
                break;

            case 'h':
            default:
                usage(argv[0]);
                break;
        }
    }

    /* Sanity */
    if (rate < 1 || !fname)
      usage(argv[0]);

    /* Catch ctrl-c */
    signal(SIGINT, handler);

    /* Open handles */
    if (!(in = fopen(fname, "r")))
    {
        perror("Opening fifo (input)");
        exit(EXIT_FAILURE);
    }
    
    if (!(out = fopen(fname, "w")))
    {
        perror("Opening fifo (output)");
        exit(EXIT_FAILURE);
    }

    monitor(in, out, rate, dump_to_stdout);

    fclose(in);
    fclose(out);
    return 0;
}
