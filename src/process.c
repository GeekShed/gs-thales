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
#include "process.h"
#include "messages.h"
#include "memory.h"
#include "log.h"
#include "misc.h"

extern Message messages[];
extern int verbose;
extern char inbuf[BUFSIZE];

/*************************************************************************/
/*************************************************************************/

/* split_buf:  Split a buffer into arguments and store the arguments in an
 *             argument vector pointed to by argv (which will be malloc'd
 *             as necessary); return the argument count.  If colon_special
 *             is non-zero, then treat a parameter with a leading ':' as
 *             the last parameter of the line, per the IRC RFC.  Destroys
 *             the buffer by side effect.
 */

int split_buf(char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;

	*argv = (char **) scalloc(sizeof(char *) * argvsize, 1);
	argc = 0;
	while (*buf)
	{
		if (argc == argvsize)
		{
			argvsize += 8;
			*argv = (char **) srealloc(*argv, sizeof(char *) * argvsize);
		}
		if (*buf == ':')
		{
			(*argv)[argc++] = buf + 1;
			buf = "";
		}
		else
		{
			s = strpbrk(buf, " ");
			if (s)
			{
				*s++ = 0;
				while (*s == ' ')
					s++;
			}
			else
			{
				s = buf + strlen(buf);
			}
			(*argv)[argc++] = buf;
			buf = s;
		}
	}
	return argc;
}

/*************************************************************************/

/* process:  Main processing routine.  Takes the string in inbuf (global
 *           variable) and does something appropriate with it. */

void process()
{
	char source[64];
	char cmd[64];
	char buf[512];					  /* Longest legal IRC command line */
	char *s;
	int ac;							  /* Parameters for the command */
	char **av;
	Message *m;


	/* If debugging, log the buffer. */
	if (verbose)
		mylog("<IRC : %s", inbuf);

	/* First make a copy of the buffer so we have the original in case we
	 * crash - in that case, we want to know what we crashed on. */
	strscpy(buf, inbuf, sizeof(buf));

	/* Split the buffer into pieces. */
	if (*buf == ':')
	{
		s = strpbrk(buf, " ");
		if (!s)
			return;
		*s = 0;
		while (isspace(*++s))
			;
		strscpy(source, buf + 1, sizeof(source));
		memmove(buf, s, strlen(s) + 1);
	}
	else
	{
		*source = 0;
	}
	if (!*buf)
		return;
	s = strpbrk(buf, " ");
	if (s)
	{
		*s = 0;
		while (isspace(*++s))
			;
	}
	else
		s = buf + strlen(buf);
	strscpy(cmd, buf, sizeof(cmd));
	ac = split_buf(s, &av, 1);

	/* Do something with the message. */
	m = find_message(cmd);
	if (m)
	{
		if (m->func)
			m->func(source, ac, av);
	}
	else
	{
		mylog("IRC : unknown message from server (%s)", inbuf);
	}

	/* Free argument list we created */
	free(av);
}

/*************************************************************************/
