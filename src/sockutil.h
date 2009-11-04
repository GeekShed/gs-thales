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
#ifndef _SOCKUTIL_H_
#define _SOCKUTIL_H_

int total_read, total_written;
int read_buffer_len(void);
int write_buffer_len(void);

int sgetc(int s);
char *sgets(char *buf, int len, int s);
char *sgets2(char *buf, int len, int s);
int sread(int s, char *buf, int len);
int sputs(char *str, int s);
int sockprintf(int s, char *fmt, ...);
int conn(const char *host, int port, const char *lhost, int lport);
void disconn(int s);


#endif
