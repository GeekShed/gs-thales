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
/* Thales' database interface */

#include "db.h"
#include "misc.h"
#include "actions.h"
#ifdef HASHLISTSUPPORT
#include "hashlist.h"
#endif

extern char *MysqlServer;
extern char *MysqlDatabase;
extern char *MysqlUser;
extern char *MysqlPassword;

extern unsigned int nbusers_max;
extern unsigned int nbusers;
extern unsigned int nbchans_max;
extern unsigned int nbchans;
extern unsigned int nbservs_max;
extern unsigned int nbservs;

extern int verbose;

MYSQL *myptr;

#ifdef HASHLISTSUPPORT
hashlist hashnicks;
hashlist hashchans;
hashlist hashservs;
#endif

/* last time we cleaned the server table */
static int ServerLastClean = -1;
extern int ServerCleanFreq;
extern int ServerCacheTime;

/* last time we cleaned the user table */
static int UserLastClean = -1;
extern int UserCleanFreq;
extern int UserCacheTime;


void db_connect()
{
	MYSQL_RES *resptr;
	if (myptr)
		mysql_close(myptr);
	/* connecting to MySQL */
	myptr = mysql_init(NULL);
	if (!mysql_real_connect
		 (myptr, MysqlServer, MysqlUser, MysqlPassword, MysqlDatabase, 0,
		  NULL, 0))
		fatal("Failed to connect to database : %s\n", mysql_error(myptr));

	/* Cleaning up the database */
	db_query("DELETE FROM " TBL_CHAN);
	db_query("DELETE FROM " TBL_ISON);
	db_query("DELETE FROM " TBL_SERV);
	db_query("DELETE FROM " TBL_USER);

	db_query("SELECT val FROM " TBL_MAXV " WHERE type=\'users\'");
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		nbusers_max = atoi(*mysql_fetch_row(resptr));
	else
		db_query("INSERT INTO " TBL_MAXV " VALUES ('users', '0', NOW())");
	mysql_free_result(resptr);

	db_query("SELECT val FROM " TBL_MAXV " WHERE type=\'channels\'");
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		nbchans_max = atoi(*mysql_fetch_row(resptr));
	else
		db_query("INSERT INTO " TBL_MAXV " VALUES ('channels', '0', NOW())");
	mysql_free_result(resptr);

	db_query("SELECT val FROM " TBL_MAXV " WHERE type=\'servers\'");
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		nbservs_max = atoi(*mysql_fetch_row(resptr));
	else
		db_query("INSERT INTO " TBL_MAXV " VALUES ('servers', '0', NOW())");
	mysql_free_result(resptr);
#ifdef HASHLISTSUPPORT
	/* init the hash lists */
	hashnicks = newhashlist();
	hashchans = newhashlist();
	hashservs = newhashlist();
#endif
}

/* escape the string */
char *db_escape(char *ch)
{
	char *buf = NULL;
	int size;
	if (ch)
	{
		size = (strlen(ch)) * 2 + 1;
		buf = (char *) malloc(sizeof(char) * size);
		if (*ch)
			mysql_real_escape_string(myptr, buf, ch, strlen(ch));
		else
			strcpy(buf, "");
	}
	return buf;
}

/* send an SQL query to the database */
int db_query(const char *fmt, ...)
{
	va_list args;
	char buf[2048];				  /* should be enough for long queries (don't underestimate :o)) */
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	if (verbose)
		mylog(">VERBOSE SQL : %s", buf);
	if (mysql_real_query(myptr, buf, strlen(buf))) {
		fatal("Query failed: (MySQL Said: %s) (Query: %s)", mysql_error(myptr), buf);
		mylog(">BAD SQL : %s", buf);
	}
	return 0;
}

/* serv should be db_escape'd before call */
/* -1 if server not found, servid else */
int db_checkserver(char *serv)
{
	int servid = -1;
	MYSQL_RES *resptr;
	db_query("SELECT servid FROM " TBL_SERV " WHERE server=\"%s\"", serv);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		servid = atoi(*mysql_fetch_row(resptr));
	mysql_free_result(resptr);
	return servid;
}

/* serv should be db_escape'd before call */
int db_getserver(char *serv)
{
#ifdef HASHLISTSUPPORT
	strtolwr(serv);
	return hash_find(hashservs, serv, KEYOTHER);
#else
	MYSQL_RES *resptr;
	int res = 0;

	db_query("SELECT servid FROM " TBL_SERV " WHERE server=\'%s\'", serv);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		res = atoi(*mysql_fetch_row(resptr));
	mysql_free_result(resptr);
	return res;
#endif
}

/* changes a nick in the hashlist */
void db_chgnick(char *newnick, char *oldnick)
{
#ifdef HASHLISTSUPPORT
	/* people often change from NICK-sthing to NICK-sthing */
	strtolwr(newnick);
	strtolwr(oldnick);
	hash_update(hashnicks, newnick, oldnick, KEYOTHER);
#endif
}

/* remove a server from the hashlist */
void db_delserver(char *server)
{
#ifdef HASHLISTSUPPORT
	strtolwr(server);
	hash_del(hashservs, server, KEYOTHER);
#endif
}

/* add a server to the hashlist */
void db_addserver(char *server, int servid)
{
#ifdef HASHLISTSUPPORT
	strtolwr(server);
	hash_add(hashservs, server, servid, KEYOTHER);
#endif
}

/* add a nick to the hashlist */
void db_addnick(char *nick, int nickid)
{
#ifdef HASHLISTSUPPORT
	strtolwr(nick);
	hash_add(hashnicks, nick, nickid, KEYOTHER);
#endif
}

/* add a nick to the hashlist */
void db_delnick(char *nick)
{
#ifdef HASHLISTSUPPORT
	strtolwr(nick);
	hash_del(hashnicks, nick, KEYOTHER);
#endif
}

/* nick should be db_escape'd before call */
/* -1 if nick not found, nickid else */
int db_checknick(char *nick)
{
#ifdef HASHLISTSUPPORT
	int res = 0;
	char *nicklow = strdup(nick);
	strtolwr(nicklow);
	res = hash_find_unsure(hashnicks, nicklow, KEYOTHER);
	free(nicklow);
	return res;
#else
	int nickid = -1;
	MYSQL_RES *resptr;
	db_query("SELECT nickid FROM " TBL_USER " WHERE nick=\"%s\"", nick);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		nickid = atoi(*mysql_fetch_row(resptr));
	mysql_free_result(resptr);
	return nickid;
#endif
}

/* nick should be db_escape'd before call */
int db_getnick_unsure(char *nick)
{
#ifdef HASHLISTSUPPORT
	int res = 0;
	char *nicklow = strdup(nick);
	strtolwr(nicklow);
	res = hash_find_unsure(hashnicks, nicklow, KEYOTHER);
	free(nicklow);
	return res;
#else
	MYSQL_RES *resptr;
	int res = 0;

	db_query("SELECT nickid FROM " TBL_USER " WHERE nick=\'%s\'", nick);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		res = atoi(*mysql_fetch_row(resptr));
	else
		res = -1;
	mysql_free_result(resptr);
	return res;
#endif
}


/* nick should be db_escape'd before call */
int db_getnick(char *nick)
{
#ifdef HASHLISTSUPPORT
	int res = 0;
	char *nicklow = strdup(nick);
	strtolwr(nicklow);
	res = hash_find(hashnicks, nicklow, KEYOTHER);
	free(nicklow);
	return res;
#else
	MYSQL_RES *resptr;
	int res = 0;

	db_query("SELECT nickid FROM " TBL_USER " WHERE nick=\'%s\'", nick);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		res = atoi(*mysql_fetch_row(resptr));
	else
		fatal("nickname not found !");
	mysql_free_result(resptr);
	return res;
#endif
}

int db_getservfromnick(char *nick)
{
	MYSQL_RES *resptr;
	int res = 0;
	db_query("SELECT servid FROM " TBL_USER " WHERE nick=\'%s\'", nick);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		res = atoi(*mysql_fetch_row(resptr));
	else
		fatal("nickname not found !");
	mysql_free_result(resptr);
	return res;
}

int db_insertid()
{
	return mysql_insert_id(myptr);
}

void db_removenick(char *nick)
{
	int nickid = db_getnick(nick);
	//TODO
	if (nickid == 0)
		fatal("nickid 0");
	db_removefromchans(nickid);
	if (UserCacheTime)
		db_query("UPDATE " TBL_USER
					" SET online=\"N\", lastquit=NOW(), servid=NULL WHERE nickid=\"%d\"",
					nickid);
	else
		db_query("DELETE FROM " TBL_USER " WHERE nickid=\'%d\'", nickid);
}


void db_removefromchans(int nickid)
{
	MYSQL_RES *resptr;
	char **res;
	db_query("SELECT " TBL_ISON ".chanid, channel FROM " TBL_ISON ", "
				TBL_CHAN " WHERE nickid=\'%d\' AND " TBL_CHAN ".chanid = "
				TBL_ISON ".chanid", nickid);
	resptr = mysql_store_result(myptr);
	db_query("DELETE FROM " TBL_ISON " WHERE nickid=\'%d\'", nickid);
	while ((res = mysql_fetch_row(resptr)))
	{
		char *chan = db_escape(res[1]);
		db_checkemptychan(atoi(res[0]), chan);
		free(chan);
	}
	mysql_free_result(resptr);
}

void db_checkemptychan(int chanid, char *chan)
{
	MYSQL_RES *resptr2;
	db_query("SELECT chanid FROM " TBL_ISON " WHERE chanid=\'%d\'", chanid);
	resptr2 = mysql_store_result(myptr);
	if (!mysql_num_rows(resptr2))
	{
		db_query("DELETE FROM " TBL_CHAN " WHERE chanid=\'%d\'", chanid);
#ifdef HASHLISTSUPPORT
		hash_del(hashchans, chan, KEYCHAN);
#endif
		nbchans--;
	}
	mysql_free_result(resptr2);
}

int db_getlusers(int type)
{
	MYSQL_RES *resptr;
	int retcode;
	switch (type)
	{
	case LUSERS_USERS:
		db_query("SELECT COUNT(*) FROM " TBL_USER " WHERE mode_li=\'N\'");
		break;
	case LUSERS_USERSINV:
		db_query("SELECT COUNT(*) FROM " TBL_USER " WHERE mode_li=\'Y\'");
		break;
	case LUSERS_OPERS:
		db_query("SELECT COUNT(*) FROM " TBL_USER " WHERE mode_lo=\'Y\'");
		break;
	case LUSERS_CHAN:
		db_query("SELECT COUNT(*) FROM " TBL_CHAN);
		break;
	case LUSERS_SERV:
		db_query("SELECT COUNT(*) FROM " TBL_SERV);
		break;
	case LUSERS_USERSGLOB:
		db_query("SELECT COUNT(*) FROM " TBL_USER);
		break;
	case LUSERS_USERSMAX:
		db_query("SELECT val FROM " TBL_MAXV " WHERE type='users'");
		break;
	}
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		retcode = atoi(*mysql_fetch_row(resptr));
	else
		retcode = -1;
	mysql_free_result(resptr);
	return retcode;
}

/* chan should be db_escape'd before call */
int db_getchannel(char *chan)
{
#ifdef HASHLISTSUPPORT
	strtolwr(chan);
	return hash_find(hashchans, chan, KEYCHAN);
#else
	int res = 0;

	MYSQL_RES *resptr;
	strtolwr(chan);
	db_query("SELECT chanid FROM " TBL_CHAN " WHERE channel=\'%s\'", chan);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		res = atoi(*mysql_fetch_row(resptr));
	else
		fatal("channel not found !");
	mysql_free_result(resptr);
	return res;
#endif
}

int db_getchmode(char *chan, char *chmode) {
        MYSQL_RES *resptr;
        strtolwr(chan);

	char *res = NULL;

        db_query("SELECT mode_%s FROM " TBL_CHAN " WHERE channel=\'%s\'", chmode, chan);
        resptr = mysql_store_result(myptr);
        if (mysql_num_rows(resptr))
                res = *mysql_fetch_row(resptr);
        else
                fatal("channel not found !");
        mysql_free_result(resptr);

	if (!strcmp(res, "N")) {
		return 0;
	}
	else {
		return 1;
	}
}

/* chan should be db_escape'd before call */
/* chan is created if not exists */
int db_getchancreate(char *chan)
{
	int res = -1;

#ifdef HASHLISTSUPPORT
	strtolwr(chan);
	res = hash_find_unsure(hashchans, chan, KEYCHAN);
#else
	MYSQL_RES *resptr;
	strtolwr(chan);
	db_query("SELECT chanid FROM " TBL_CHAN " WHERE channel=\'%s\'", chan);
	resptr = mysql_store_result(myptr);
	if (mysql_num_rows(resptr))
		res = atoi(*mysql_fetch_row(resptr));
	mysql_free_result(resptr);
#endif
	if (res == -1)

	{
		db_query("INSERT INTO " TBL_CHAN " (channel) VALUES (\'%s\')", chan);
		res = db_insertid();
#ifdef HASHLISTSUPPORT
		hash_add(hashchans, chan, res, KEYCHAN);
#endif
		nbchans++;
		do_checknbchansmax();
	}
	return res;
}

/* close connection to DBMS */
void db_close()
{
	if (myptr)
		mysql_close(myptr);
	myptr = NULL;
}

/* cleanup the server table, removing old entries */
void db_cleanserver()
{
	MYSQL_RES *resptr;
	int curtime = time(NULL);
	char **res;
	if (ServerLastClean == -1)
		ServerLastClean = curtime;
	if (curtime > (ServerLastClean + ServerCleanFreq))
	{
		ServerLastClean = curtime;
		db_query("SELECT server FROM " TBL_SERV
					" WHERE online=\"N\" AND lastsplit<FROM_UNIXTIME(\"%d\")",
					curtime - ServerCacheTime);
		resptr = mysql_store_result(myptr);
		if (mysql_num_rows(resptr))
		{
			while ((res = mysql_fetch_row(resptr)))
			{
				char *server = db_escape(res[0]);
				db_delserver(server);
				free(server);
			}
			mysql_free_result(resptr);
			db_query("DELETE FROM " TBL_SERV
						" WHERE online=\"N\" AND lastsplit<FROM_UNIXTIME(\"%d\")",
						curtime - ServerCacheTime);
		}
	}
}

/* cleanup the user table, removing old entries */
void db_cleanuser()
{
	MYSQL_RES *resptr;
	int curtime = time(NULL);
	char **res;
	if (UserLastClean == -1)
		UserLastClean = curtime;
	if (curtime > (UserLastClean + UserCleanFreq))
	{
		UserLastClean = curtime;
		db_query("SELECT nick FROM " TBL_USER
					" WHERE online=\"N\" AND lastquit<FROM_UNIXTIME(\"%d\")",
					curtime - UserCacheTime);

		resptr = mysql_store_result(myptr);
		if (mysql_num_rows(resptr))
		{
			while ((res = mysql_fetch_row(resptr)))
			{
				char *nick = db_escape(res[0]);
				db_delnick(nick);
				free(nick);
			}
			mysql_free_result(resptr);
			db_query("DELETE FROM " TBL_USER
						" WHERE online=\"N\" AND lastquit<FROM_UNIXTIME(\"%d\")",
						curtime - UserCacheTime);
		}
	}
}
