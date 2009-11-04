/*  GNU Thales - IRC to Relational Database Gateway
 *  Copyright (C) 2002 Lucas Nussbaum <lucas@lucas-nussbaum.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "thales.h"
#include "log.h"
#include "misc.h"
#include "send.h"

extern char *LogFilename;
extern char inbuf[];

extern int debug;
extern int verbose;

extern int servsock;

FILE *logfile = NULL;

/* Open the log file.  Return -1 if the log file could not be opened, else
 * return 0. */

int open_log(void)
{
	if (logfile)
		return 0;					  /* logfile already opened */
	logfile = fopen(LogFilename, "a");
	return logfile ? 0 : -1;
}

/* Close the log file. */

void close_log(void)
{
	if (!logfile)
		return;
	fclose(logfile);
	logfile = NULL;
}

/* Log stuff to the log file with a datestamp. errno preserved. */
void mylog(const char *fmt, ...)
{
	va_list args;
	time_t t;
	struct tm tm;
	char buf[256];
	int errno_save = errno;

	va_start(args, fmt);
	time(&t);
	tm = *localtime(&t);
#if HAVE_GETTIMEOFDAY
	if (verbose)
	{
		/* if verbose is on, we log the microsecond to make a more precise analysis */
		char *s;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S", &tm);
		s = buf + strlen(buf);
		s += snprintf(s, sizeof(buf) - (s - buf), ".%06ld", tv.tv_usec);
		strftime(s, sizeof(buf) - (s - buf) - 1, " %Y] ", &tm);
	}
	else
	{
#endif
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S %Y] ", &tm);
#if HAVE_GETTIMEOFDAY
	}
#endif
	if (logfile)
	{
		fputs(buf, logfile);
		vfprintf(logfile, fmt, args);
		fputc('\n', logfile);
		fflush(logfile);
	}
	if (debug)
	{									  /* log it to stderr */
		fputs(buf, stderr);
		vfprintf(stderr, fmt, args);
		fputc('\n', stderr);
	}
	errno = errno_save;
}

/* Like mylog(), but tack a ": " and a system error message (as returned by
 * strerror()) onto the end.
 */

void mylog_perror(const char *fmt, ...)
{
	va_list args;
	time_t t;
	struct tm tm;
	char buf[256];
	int errno_save = errno;
	va_start(args, fmt);
	time(&t);
	tm = *localtime(&t);
#if HAVE_GETTIMEOFDAY
	if (verbose)
	{
		char *s;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S", &tm);
		s = buf + strlen(buf);
		s += snprintf(s, sizeof(buf) - (s - buf), ".%06ld", tv.tv_usec);
		strftime(s, sizeof(buf) - (s - buf) - 1, " %Y] ", &tm);
	}
	else
	{
#endif
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S %Y] ", &tm);
#if HAVE_GETTIMEOFDAY
	}
#endif
	if (logfile)
	{
		fputs(buf, logfile);
		vfprintf(logfile, fmt, args);
		fprintf(logfile, ": %s\n", strerror(errno_save));
		fflush(logfile);
	}
	if (debug)
	{
		fputs(buf, stderr);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, ": %s\n", strerror(errno_save));
	}
	errno = errno_save;
}

/*************************************************************************/

/* We've hit something we can't recover from.  Let people know what
 * happened, then go down.
 */

void fatal(const char *fmt, ...)
{
	va_list args;
	time_t t;
	struct tm tm;
	char buf[256], buf2[4096];

	mylog("IRC Context: %s", inbuf);

	va_start(args, fmt);
	time(&t);
	tm = *localtime(&t);
#if HAVE_GETTIMEOFDAY
	if (verbose)
	{
		char *s;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S", &tm);
		s = buf + strlen(buf);
		s += snprintf(s, sizeof(buf) - (s - buf), ".%06ld", tv.tv_usec);
		strftime(s, sizeof(buf) - (s - buf) - 1, " %Y] ", &tm);
	}
	else
	{
#endif
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S %Y] ", &tm);
#if HAVE_GETTIMEOFDAY
	}
#endif
	vsnprintf(buf2, sizeof(buf2), fmt, args);
	if (logfile)
		fprintf(logfile, "%sFATAL: %s\n", buf, buf2);
	if (debug)
		fprintf(stderr, "%sFATAL: %s\n", buf, buf2);
	if (servsock >= 0)
		wallops(NULL, "FATAL ERROR! %s (IRC Context: %s)", buf2, inbuf);
	exit(1);
}


/* Same thing, but do it like perror(). */

void fatal_perror(const char *fmt, ...)
{
	va_list args;
	time_t t;
	struct tm tm;
	char buf[256], buf2[4096];
	int errno_save = errno;

	mylog("IRC context : %s", inbuf);

	va_start(args, fmt);
	time(&t);
	tm = *localtime(&t);
#if HAVE_GETTIMEOFDAY
	if (verbose)
	{
		char *s;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S", &tm);
		s = buf + strlen(buf);
		s += snprintf(s, sizeof(buf) - (s - buf), ".%06ld", tv.tv_usec);
		strftime(s, sizeof(buf) - (s - buf) - 1, " %Y] ", &tm);
	}
	else
	{
#endif
		strftime(buf, sizeof(buf) - 1, "[%b %d %H:%M:%S %Y] ", &tm);
#if HAVE_GETTIMEOFDAY
	}
#endif
	vsnprintf(buf2, sizeof(buf2), fmt, args);
	if (logfile)
		fprintf(logfile, "%sFATAL: %s: %s\n", buf, buf2,
				  strerror(errno_save));
	if (debug)
		fprintf(stderr, "%sFATAL: %s: %s\n", buf, buf2,
				  strerror(errno_save));
	if (servsock >= 0)
		wallops(NULL, "FATAL ERROR!  %s: %s (IRC context : %s)", buf2,
				  strerror(errno_save), inbuf);
	exit(1);
}

/*************************************************************************/
