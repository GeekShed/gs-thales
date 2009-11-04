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

#ifndef _LOG_H_
#define _LOG_H_

#include "thales.h"

/* open the log file */
int open_log(void);

/* close the log file */
void close_log(void);

/* Log stuff to the log file with a datestamp. errno preserved. */
void mylog(const char *fmt, ...);

/* Like log(), but tack a ": " and a system error message (as returned by strerror()) onto the end.
*/
void mylog_perror(const char *fmt, ...);

/* fatal error, log then go down */
void fatal(const char *fmt, ...);

/* same, but do it like perror() */
void fatal_perror(const char *fmt, ...);

#endif /* _LOG_H_ */
