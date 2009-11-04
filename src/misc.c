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
#include "misc.h"

/*************************************************************************/

/* strscpy:  Copy at most len-1 characters from a string to a buffer, and
 *           add a null terminator after the last character copied.
 */

char *strscpy(char *d, const char *s, size_t len)
{
	char *d_orig = d;

	if (!len)
		return d;
	while (--len && (*d++ = *s++))
		;
	*d = 0;
	return d_orig;
}

/*************************************************************************/

/* stristr:  Search case-insensitively for string s2 within string s1,
 *           returning the first occurrence of s2 or NULL if s2 was not
 *           found.
 */

char *stristr(char *s1, char *s2)
{
	register char *s = s1, *d = s2;

	while (*s1)
	{
		if (tolower(*s1) == tolower(*d))
		{
			s1++;
			d++;
			if (*d == 0)
				return s;
		}
		else
		{
			s = ++s1;
			d = s2;
		}
	}
	return NULL;
}

/*************************************************************************/

/* strnrepl:  Replace occurrences of `old' with `new' in string `s'.  Stop
 *            replacing if a replacement would cause the string to exceed
 *            `size' bytes (including the null terminator).  Return the
 *            string.
 */

char *strnrepl(char *s, int size, const char *old, const char *new)
{
	char *ptr = s;
	int left = strlen(s);
	int avail = size - (left + 1);
	int oldlen = strlen(old);
	int newlen = strlen(new);
	int diff = newlen - oldlen;

	while (left >= oldlen)
	{
		if (strncmp(ptr, old, oldlen) != 0)
		{
			left--;
			ptr++;
			continue;
		}
		if (diff > avail)
			break;
		if (diff != 0)
			memmove(ptr + oldlen + diff, ptr + oldlen, left + 1);
		strncpy(ptr, new, newlen);
		ptr += newlen;
		left -= oldlen;
	}
	return s;
}

/*************************************************************************/
/*************************************************************************/

/* merge_args:  Take an argument count and argument vector and merge them
 *              into a single string in which each argument is separated by
 *              a space.
 */

char *merge_args(int argc, char **argv)
{
	int i;
	static char s[4096];
	char *t;

	t = s;
	for (i = 0; i < argc; i++)
		t += snprintf(t, sizeof(s) - (t - s), "%s%s", *argv++,
						  (i < argc - 1) ? " " : "");
	return s;
}

/*************************************************************************/

/* dotime:  Return the number of seconds corresponding to the given time
 *          string.  If the given string does not represent a valid time,
 *          return -1.
 *
 *          A time string is either a plain integer (representing a number
 *          of seconds), or an integer followed by one of these characters:
 *          "s" (seconds), "m" (minutes), "h" (hours), or "d" (days).
 */

int dotime(const char *s)
{
	int amount;

	amount = strtol(s, (char **) &s, 10);
	if (*s)
	{
		switch (*s)
		{
		case 's':
			return amount;
		case 'm':
			return amount * 60;
		case 'h':
			return amount * 3600;
		case 'd':
			return amount * 86400;
		default:
			return -1;
		}
	}
	else
	{
		return amount;
	}
}

void strtolwr(char *ch)
{
	while (*ch)
	{
		*ch = tolower(*ch);
		ch++;
	}
}
