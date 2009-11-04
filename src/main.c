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
/* GNU Thales - main file */

#include "thales.h"
#include "conf.h"
#include "log.h"
#include "send.h"
#include "sockutil.h"
#include "compat.h"
#include "memory.h"
#include "process.h"
#include "db.h"

/* Global variables */

int debug = 0;						  /* -d , debug mode = don't fork + log to stderr */

int verbose = 0;					  /* -v, verbose output */

/* Socket for talking to server */
int servsock = -1;
char *quitmsg;

char inbuf[BUFSIZE];

/* At what time were we started? */
time_t start_time;

extern char *PIDFilename;
extern char *RemoteServer;
extern int RemotePort;
extern char *LocalHost;
extern int LocalPort;
extern char *RemotePassword;
extern char *ServerName;
extern char *ServerDesc;

/* display usage */
void usage()
{
	printf("\n   Syntax : thales\n");
	printf("         -v : enable verbose mode\n");
	printf("         -d : enable debugging mode\n");
	printf("         -V : display version and exit\n");
	exit(1);
}

/* Create our PID file and write the PID to it. */
void write_pidfile(void)
{
	FILE *pidfile;

	pidfile = fopen(PIDFilename, "w");
	if (pidfile)
	{
		fprintf(pidfile, "%d\n", (int) getpid());
		fclose(pidfile);
	}
	else
	{
		mylog_perror("Warning: cannot write to PID file %s", PIDFilename);
	}
}

/* Main routine */
int main(int argc, char **argv)
{
	char ch;
	int i;
	char *ch1, *ch2;
	/* record start time */
	start_time = time(NULL);
	/* Parse command line options */
	while ((ch = getopt(argc, argv, "dvV")) != -1)
	{
		switch (ch)
		{
		case 'd':
			debug = 1;
			verbose = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'V':
			printf("GNU Thales v.%s\n", VERSION);
			exit(0);
			break;
		default:
			usage();
		}
	}

	/* Read configuration file; exit if there are problems. */
	if (!read_config())
		exit(-1);

	/* Open logfile, and complain if we didn't. */
	if (open_log() < 0)
	{
		fprintf(stderr, "Warning: unable to open log file : %s\n",
				  strerror(errno));
		exit(1);
	}

	/* Detach ourselves if requested. */
	if (!debug)
	{
		if ((i = fork()) < 0)
		{
			perror("fork()");
			return -1;
		}
		else if (i != 0)
		{
			exit(0);
		}
		if (isatty(0) && isatty(1) && isatty(2))
		{
			close(0);
			close(1);
			close(2);
		}
	}
	write_pidfile();

	/* Announce ourselves to the logfile. */
	mylog("GNU Thales %s starting up with %s support%s", VERSION,
#if defined(IRCD_UNREAL)
		 "unreal",
#elif defined(IRCD_HYBRID)
		 "hybrid",
#elif defined (IRCD_BAHAMUT)
		 "bahamut",
#elif defined (IRCD_IRCDRU)
		 "ircdru",
#elif defined (IRCD_ULTIMATE)
		 "ultimate30",
#elif defined (IRCD_ULTI28)
		 "ultimate28",
#else
		 "unknown (!)",
#endif
		 verbose ? (debug ? " (options: debug)" : " (options: verbose") :
		 "");
	/* I should init databases here */
	db_connect();
	ch1 = db_escape(ServerName);
	ch2 = db_escape(ServerDesc);
	db_query("INSERT INTO " TBL_SERV
				" (server, comment, connecttime) values('%s', '%s', NOW())",
				ch1, ch2);
	db_addserver(ch1, db_insertid());
	free(ch1);
	free(ch2);
	/* connect to RemoteServer */
	servsock = conn(RemoteServer, RemotePort, LocalHost, LocalPort);
	if (servsock < 0)
	{
		fatal_perror("Can't connect to server");
		exit(-1);
	}
#if defined(IRCD_UNREAL)
	send_cmd(NULL, "PROTOCTL NICKv2 SJOIN SJOIN2 SJ3");
#endif
#if defined(IRCD_HYBRID)
	send_cmd(NULL, "PASS %s :TS", RemotePassword);
	send_cmd(NULL, "CAPAB :HOPS TBURST");
#elif defined (IRCD_BAHAMUT)
	send_cmd(NULL, "PASS %s :TS", RemotePassword);
	send_cmd(NULL, "CAPAB TS3 SSJOIN NICKIP");
#elif defined (IRCD_IRCDRU)
	send_cmd(NULL, "PASS %s :TS", RemotePassword);
	send_cmd(NULL, "CAPAB TS3 SSJOIN 8BNCI NICKIP");

#elif defined (IRCD_ULTIMATE)
	send_cmd(NULL, "PASS %s :TS", RemotePassword);
	send_cmd(NULL, "CAPAB TS5 SSJ5 NICKIP");
#elif defined(IRCD_ULTI28)		  /* special case to prevent future errors */
	send_cmd(NULL, "PASS %s", RemotePassword);
#else
	send_cmd(NULL, "PASS %s", RemotePassword);
#endif

#if defined(IRCD_IRCDRU)
	send_cmd(NULL, "SERVER %s 1 8 :%s", ServerName, ServerDesc);
#else
	send_cmd(NULL, "SERVER %s 1 :%s", ServerName, ServerDesc);
#endif

#if defined(IRCD_BAHAMUT)||defined(IRCD_IRCDRU)
	send_cmd(NULL, "SVINFO 3 1 0 :%ld", time(NULL));
#elif defined(IRCD_ULTIMATE)
	send_cmd(NULL, "SVINFO 5 3 0 :%ld", time(NULL));
#endif
#if defined(IRCD_UNREAL)
   send_cmd(NULL, ":%s EOS", ServerName);
#endif
	sgets2(inbuf, sizeof(inbuf), servsock);
	if (strnicmp(inbuf, "ERROR", 5) == 0)
	{
		/* Close server socket first to stop wallops, since the other
		 * server doesn't want to listen to us anyway */
		disconn(servsock);
		servsock = -1;
		fatal("Remote server returned: %s", inbuf);
	}
	/* We have a line left over from earlier, process it now */
	process();
	while (1)
	{
		i = (int) (long) sgets2(inbuf, sizeof(inbuf), servsock);
		if (i > 0)
		{
			process();
		}
		else if (i == 0)
		{
			int errno_save = errno;
			quitmsg = (char *) scalloc(BUFSIZE, 1);
			if (quitmsg)
			{
				snprintf(quitmsg, BUFSIZE, "Read error from server: %s",
							strerror(errno_save));
			}
			else
			{
				quitmsg = "Read error from server";
			}
			send_cmd(ServerName, "SQUIT %s :%s", ServerName, quitmsg);
			disconn(servsock);
			close_log();
			return 0;
		}
	}
}
