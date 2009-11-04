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
/* GNU Thales' database interface */

#ifndef _DB_H_
#define _DB_H_

#include "thales.h"
#include "log.h"

void db_connect();
int db_query(const char *fmt, ...);
char *db_escape(char *);
int db_checkserver(char *serv);
int db_getserver(char *serv);
int db_getservfromnick(char *nick);
int db_checknick(char *nick);
int db_getnick(char *nick);
int db_getnick_unsure(char *nick);
int db_insertid();
void db_chgnick(char *newnick, char *oldnick);
void db_addserver(char *server, int servid);
void db_delserver(char *server);
void db_addnick(char *nick, int nickid);
void db_delnick(char *nick);
int db_getchannel(char *chan);
int db_getchmode(char *chan, char *chmode);
int db_getchancreate(char *chan);
void db_removefromchans(int nickid);
void db_removenick(char *nick);
void db_checkemptychan(int chanid, char *chan);
void db_close();
void db_cleanserver();
void db_cleanuser();
int db_getlusers(int type);
#define LUSERS_USERS 1
#define LUSERS_USERSINV 2
#define LUSERS_OPERS 3
#define LUSERS_SERV 4
#define LUSERS_CHAN 5
#define LUSERS_USERSGLOB 6
#define LUSERS_USERSMAX 7

#ifndef TBL_PREFIX
#define TBL_PREFIX ""
#endif

	/* Tables names */
#define TBL_USER TBL_PREFIX"user"
#define TBL_CHAN TBL_PREFIX"chan"
#define TBL_ISON TBL_PREFIX"ison"
#define TBL_SERV TBL_PREFIX"server"
#define TBL_MAXV TBL_PREFIX"maxvalues"
#define TBL_TOPI TBL_PREFIX"topics"

#endif
