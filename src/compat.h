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
#ifndef _COMPAT_H_
#define _COMPAT_H_

#if !HAVE_STRICMP && !HAVE_STRCASECMP
int stricmp(const char *s1, const char *s2);
int strnicmp(const char *s1, const char *s2, size_t len);
#endif
#if !HAVE_STRDUP
char *strdup(const char *s);
#endif
#if !HAVE_STRSPN
size_t strspn(const char *s, const char *accept);
#endif
#if !HAVE_STRERROR
char *strerror(int errnum);
#endif

#endif
