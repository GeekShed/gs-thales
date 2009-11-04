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

/*************************************************************************/


#if !HAVE_STRICMP && !HAVE_STRCASECMP

/* stricmp, strnicmp:  Case-insensitive versions of strcmp() and
 *                     strncmp().
 */

int stricmp(const char *s1, const char *s2)
{
	register int c;

	while ((c = tolower(*s1)) == tolower(*s2))
	{
		if (c == 0)
			return 0;
		s1++;
		s2++;
	}
	if (c < tolower(*s2))
		return -1;
	return 1;
}

int strnicmp(const char *s1, const char *s2, size_t len)
{
	register int c;

	if (!len)
		return 0;
	while ((c = tolower(*s1)) == tolower(*s2) && len > 0)
	{
		if (c == 0 || --len == 0)
			return 0;
		s1++;
		s2++;
	}
	if (c < tolower(*s2))
		return -1;
	return 1;
}
#endif

/*************************************************************************/

#if !HAVE_STRDUP
char *strdup(const char *s)
{
	char *new = calloc(strlen(s) + 1, 1);
	if (new)
		strcpy(new, s);
	return new;
}
#endif

/*************************************************************************/

#if !HAVE_STRSPN
size_t strspn(const char *s, const char *accept)
{
	size_t i = 0;

	while (*s && strchr(accept, *s))
		++i, ++s;
	return i;
}
#endif

/*************************************************************************/

#if !HAVE_STRERROR
# if HAVE_SYS_ERRLIST
extern char *sys_errlist[];
# endif

char *strerror(int errnum)
{
# if HAVE_SYS_ERRLIST
	return sys_errlist[errnum];
# else
	static char buf[20];
	snprintf(buf, sizeof(buf), "Error %d", errnum);
	return buf;
# endif
}
#endif
