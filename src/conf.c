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

/* Configuration file handling. */

#include "conf.h"
#include "misc.h"
#include "compat.h"

extern int debug;


/* configuration variables: */
char *RemoteServer;
int RemotePort;
char *RemotePassword;
char *LocalHost;
int LocalPort;

char *ServerName;
char *ServerDesc;
char *PIDFilename;
char *LogFilename;

char *MysqlServer;
char *MysqlDatabase;
char *MysqlUser;
char *MysqlPassword;

int UserCacheTime;
int UserCleanFreq;

int ServerCacheTime;
int ServerCleanFreq;

int HidePasswords;

Directive directives[] = {
	{"RemoteServer", {{PARAM_STRING, 0, &RemoteServer},
							{PARAM_PORT, 0, &RemotePort},
							{PARAM_STRING, 0, &RemotePassword}}},
	{"LocalAddress", {{PARAM_STRING, 0, &LocalHost},
							{PARAM_PORT, PARAM_OPTIONAL, &LocalPort}}},
	{"ServerDesc", {{PARAM_STRING, 0, &ServerDesc}}},
	{"ServerName", {{PARAM_STRING, 0, &ServerName}}},
	{"MysqlServer", {{PARAM_STRING, 0, &MysqlServer}}},
	{"MysqlDatabase", {{PARAM_STRING, 0, &MysqlDatabase}}},
	{"MysqlUser", {{PARAM_STRING, 0, &MysqlUser}}},
	{"MysqlPassword", {{PARAM_STRING, 0, &MysqlPassword}}},
	{"PIDFile", {{PARAM_STRING, 0, &PIDFilename}}},
	{"LogFile", {{PARAM_STRING, 0, &LogFilename}}},
	{"UserCacheTime", {{PARAM_POSINT, 0, &UserCacheTime}}},
	{"ServerCacheTime", {{PARAM_POSINT, 0, &ServerCacheTime}}},
	{"UserCleanFreq", {{PARAM_POSINT, 0, &UserCleanFreq}}},
	{"ServerCleanFreq", {{PARAM_POSINT, 0, &ServerCleanFreq}}},
	{"HidePasswords", {{PARAM_SET, 0, &HidePasswords}}},
};

/*************************************************************************/

/* Print an error message to the log (and the console, if open). */

void error(int linenum, char *message, ...)
{
	char buf[4096];
	va_list args;

	va_start(args, message);
	vsnprintf(buf, sizeof(buf), message, args);
	if (linenum)
		mylog("%s:%d: %s", THALES_CONF, linenum, buf);
	else
		mylog("%s: %s", THALES_CONF, buf);
	if (!debug && isatty(2))
	{
		if (linenum)
			fprintf(stderr, "%s:%d: %s\n", THALES_CONF, linenum, buf);
		else
			fprintf(stderr, "%s: %s\n", THALES_CONF, buf);
	}
}

/*************************************************************************/

/* Parse a configuration line.  Return 1 on success; otherwise, print an
 * appropriate error message and return 0.  Destroys the buffer by side
 * effect.
 */

int parse(char *buf, int linenum, int reload)
{
	char *s, *t, *dir;
	int i, n, optind, val;
	int retval = 1;
	int ac = 0;
	char *av[MAXPARAMS];

	dir = strtok(buf, " \t\r\n");
	s = strtok(NULL, "");
	if (s)
	{
		while (isspace(*s))
			s++;
		while (*s)
		{
			if (ac >= MAXPARAMS)
			{
				error(linenum, "Warning: too many parameters (%d max)",
						MAXPARAMS);
				break;
			}
			t = s;
			if (*s == '"')
			{
				t++;
				s++;
				while (*s && *s != '"')
				{
					if (*s == '\\' && s[1] != 0)
						s++;
					s++;
				}
				if (!*s)
					error(linenum,
							"Warning: unterminated double-quoted string");
				else
					*s++ = 0;
			}
			else
			{
				s += strcspn(s, " \t\r\n");
				if (*s)
					*s++ = 0;
			}
			av[ac++] = t;
			while (isspace(*s))
				s++;
		}
	}

	if (!dir)
		return 1;

	for (n = 0; n < lenof(directives); n++)
	{
		Directive *d = &directives[n];
		if (stricmp(dir, d->name) != 0)
			continue;
		optind = 0;
		for (i = 0; i < MAXPARAMS && d->params[i].type != PARAM_NONE; i++)
		{
			if (reload && !(d->params[i].flags & PARAM_RELOAD))
				continue;

			if (d->params[i].type == PARAM_SET)
			{
				*(int *) d->params[i].ptr = 1;
				continue;
			}
#ifdef STREAMLINED
			if (d->params[i].flags & PARAM_FULLONLY)
			{
				error(linenum,
						"Directive `%s' not available in STREAMLINED mode",
						d->name);
				break;
			}
#endif

			if (d->params[i].type == PARAM_DEPRECATED)
			{
				void (*func) (void);
				error(linenum, "Deprecated directive `%s' used", d->name);
				func = (void (*)(void)) (d->params[i].ptr);
				func();				  /* For clarity */
				continue;
			}
			if (optind >= ac)
			{
				if (!(d->params[i].flags & PARAM_OPTIONAL))
				{
					error(linenum, "Not enough parameters for `%s'", d->name);
					retval = 0;
				}
				break;
			}
			switch (d->params[i].type)
			{
			case PARAM_INT:
				val = strtol(av[optind++], &s, 0);
				if (*s)
				{
					error(linenum, "%s: Expected an integer for parameter %d",
							d->name, optind);
					retval = 0;
					break;
				}
				*(int *) d->params[i].ptr = val;
				break;
			case PARAM_POSINT:
				val = strtol(av[optind++], &s, 0);
				if (*s || val < 0)
				{
					error(linenum,
							"%s: Expected a positive integer for parameter %d",
							d->name, optind);
					retval = 0;
					break;
				}
				*(int *) d->params[i].ptr = val;
				break;
			case PARAM_PORT:
				val = strtol(av[optind++], &s, 0);
				if (*s)
				{
					error(linenum,
							"%s: Expected a port number for parameter %d",
							d->name, optind);
					retval = 0;
					break;
				}
				if (val < 1 || val > 65535)
				{
					error(linenum,
							"Port numbers must be in the range 1..65535");
					retval = 0;
					break;
				}
				*(int *) d->params[i].ptr = val;
				break;
			case PARAM_STRING:
/*	      if (reload && *(char **)d->params[i].ptr)
	      	free(*(char **)d->params[i].ptr); */
				*(char **) d->params[i].ptr = strdup(av[optind++]);
				if (!d->params[i].ptr)
				{
					error(linenum, "%s: Out of memory", d->name);
					return 0;
				}
				break;
			case PARAM_TIME:
				val = dotime(av[optind++]);
				if (val < 0)
				{
					error(linenum,
							"%s: Expected a time value for parameter %d",
							d->name, optind);
					retval = 0;
					break;
				}
				*(int *) d->params[i].ptr = val;
				break;
			default:
				error(linenum, "%s: Unknown type %d for param %d",
						d->name, d->params[i].type, i + 1);
				return 0;			  /* don't bother continuing--something's bizarre */
			}
		}
		break;						  /* because we found a match */
	}

	if (n == lenof(directives))
	{
		error(linenum, "Unknown directive `%s'", dir);
		return 1;					  /* don't cause abort */
	}

	return retval;
}

/*************************************************************************/

#define CHECK(v) do {			\
    if (!v) {				\
	error(0, #v " missing");	\
	retval = 0;			\
    }					\
} while (0)

#define CHEK2(v,n) do {			\
    if (!v) {				\
	error(0, #n " missing");	\
	retval = 0;			\
    }					\
} while (0)

/* Read the entire configuration file.  If an error occurs while reading
 * the file or a required directive is not found, print and log an
 * appropriate error message and return 0; otherwise, return 1.
 *
 * If reload is 1, will reload the configuration file.
 *		--lara
 *
 */

int read_config(void)
{
	FILE *config;
	int linenum = 0, retval = 1;
	char buf[1024];
	config = fopen(THALES_CONF, "r");
	if (!config)
	{
		mylog_perror("Can't open " THALES_CONF);
		if (!debug && isatty(2))
			perror("Can't open " THALES_CONF);
		else
			mylog("Can't open %s", THALES_CONF);
		return 0;
	}
	while (fgets(buf, sizeof(buf), config))
	{
		linenum++;
		if (*buf == '#' || *buf == '\r' || *buf == '\n')
			continue;
		if (!parse(buf, linenum, 0))
			retval = 0;
	}
	fclose(config);

	CHECK(RemoteServer);
	CHECK(ServerName);
	CHECK(ServerDesc);
	CHEK2(PIDFilename, PIDFile);
	return retval;
}
