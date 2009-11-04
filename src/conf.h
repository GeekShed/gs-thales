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
#ifndef _CONF_H_
#define _CONF_H_

#include "thales.h"
#include "log.h"

/* Configuration directives. */
#define THALES_CONF "thales.conf"
#define MAXPARAMS	4

typedef struct
{
	char *name;
	struct
	{
		int type;					  /* PARAM_* below */
		int flags;					  /* Same */
		void *ptr;					  /* Pointer to where to store the value */
	}
	params[MAXPARAMS];
}
Directive;

#define PARAM_NONE	0
#define PARAM_INT	1
#define PARAM_POSINT	2			  /* Positive integer only */
#define PARAM_PORT	3			  /* 1..65535 only */
#define PARAM_STRING	4
#define PARAM_TIME	5
#define PARAM_STRING_ARRAY 6	  /* Array of string */
#define PARAM_SET	-1				  /* Not a real parameter; just set the
										   * *    given integer variable to 1 */
#define PARAM_DEPRECATED -2	  /* Set for deprecated directives; `ptr'
										   * *    is a function pointer to call */

/* Flags: */
#define PARAM_OPTIONAL	0x01
#define PARAM_FULLONLY	0x02	  /* Directive only allowed if !STREAMLINED */
#define PARAM_RELOAD    0x04	  /* Directive is reloadable */

/* Print an error message to the log (and the console, if open). */
void error(int linenum, char *message, ...);

/* Parse a configuration line.  Return 1 on success; otherwise, print an
 * appropriate error message and return 0.  Destroys the buffer by side
 * effect.
 */

int parse(char *buf, int linenum, int reload);

/* Read the entire configuration file.  If an error occurs while reading
 * the file or a required directive is not found, print and log an
 * appropriate error message and return 0; otherwise, return 1.
 *
 * If reload is 1, will reload the configuration file.
 *		--lara
 *
 */

int read_config(void);

#endif /* _CONF_H_ */
