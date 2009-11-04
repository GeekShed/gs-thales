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
#include "send.h"
#include "sockutil.h"
#include "log.h"

extern int servsock;
extern int verbose;
extern char *ServerName;

/*************************************************************************/

/* Send a command to the server.  The two forms here are like
 * printf()/vprintf() and friends. */

void send_cmd(const char *source, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsend_cmd(source, fmt, args);
	va_end(args);
}

void vsend_cmd(const char *source, const char *fmt, va_list args)
{
	char buf[BUFSIZE];

	vsnprintf(buf, sizeof(buf), fmt, args);
	if (source)
	{
		sockprintf(servsock, ":%s %s\r\n", source, buf);
		if (verbose)
			mylog(">IRC : %s %s", source, buf);
	}
	else
	{
		sockprintf(servsock, "%s\r\n", buf);
		if (verbose)
			mylog(">IRC : %s", buf);
	}
}

/*************************************************************************/

/* Send out a WALLOPS (a GLOBOPS on ircd.dal). */

void wallops(const char *source, const char *fmt, ...)
{
	va_list args;
	char buf[BUFSIZE];

	va_start(args, fmt);
	snprintf(buf, sizeof(buf), "GLOBOPS :%s", fmt);
	vsend_cmd(source ? source : ServerName, buf, args);
}
