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

#ifndef NAME_MAX
# define NAME_MAX 255
#endif

#ifndef BUFSIZ
# define BUFSIZ 256
#else
# if BUFSIZ < 256
#  define BUFSIZ 256
# endif
#endif

/* Length of an array: */
#define lenof(a)	(sizeof(a) / sizeof(*(a)))

/* Telling compilers about printf()-like functions: */
#ifdef __GNUC__
# define FORMAT(type,fmt,start) __attribute__((format(type,fmt,start)))
#else
# define FORMAT(type,fmt,start)
#endif

#define BUFSIZE 1024
#define NET_BUFSIZE 65536
/*************************************************************************/
