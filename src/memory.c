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
#include "memory.h"
#include "log.h"

/*************************************************************************/
/*************************************************************************/

/* smalloc, scalloc, srealloc, sstrdup:
 *	Versions of the memory allocation functions which will cause the
 *	program to terminate with an "Out of memory" error if the memory
 *	cannot be allocated.  (Hence, the return value from these functions
 *	is never NULL.)
 */

void *smalloc(long size)
{
	void *buf;

	if (!size)
	{
		mylog("smalloc: Illegal attempt to allocate 0 bytes");
		size = 1;
	}
	buf = malloc(size);
	if (!buf)
#if !defined(USE_THREADS) || !defined(LINUX20)
		raise(SIGUSR1);
#else
		abort();
#endif
	return buf;
}

void *scalloc(long elsize, long els)
{
	void *buf;

	if (!elsize || !els)
	{
		mylog("scalloc: Illegal attempt to allocate 0 bytes");
		elsize = els = 1;
	}
	buf = calloc(elsize, els);
	if (!buf)
#if !defined(USE_THREADS) || !defined(LINUX20)
		raise(SIGUSR1);
#else
		abort();
#endif
	return buf;
}

void *srealloc(void *oldptr, long newsize)
{
	void *buf;

	if (!newsize)
	{
		mylog("srealloc: Illegal attempt to allocate 0 bytes");
		newsize = 1;
	}
	buf = realloc(oldptr, newsize);
	if (!buf)
#if !defined(USE_THREADS) || !defined(LINUX20)
		raise(SIGUSR1);
#else
		abort();
#endif
	return buf;
}

char *sstrdup(const char *s)
{
	char *t = strdup(s);
	if (!t)
#if !defined(USE_THREADS) || !defined(LINUX20)
		raise(SIGUSR1);
#else
		abort();
#endif
	return t;
}

/*************************************************************************/
/*************************************************************************/

/* In the future: malloc() replacements that tell us if we're leaking and
 * maybe do sanity checks too... */

/*************************************************************************/
