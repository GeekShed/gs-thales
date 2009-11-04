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
#include "actions.h"
#include "db.h"
#include "send.h"
#include "misc.h"
#include "compat.h"
#include "memory.h"
#include "log.h"
#include "sockutil.h"

extern char *ServerName;
extern char *RemoteServer;
extern time_t start_time;
extern char inbuf[];
extern int ServerCacheTime;
extern int UserCacheTime;
extern int HidePasswords;

/*************************************************************************/
/*
 * Max values calculation
 */

unsigned int nbusers = 0;
unsigned int nbusers_max = 0;
unsigned int nbchans = 0;
unsigned int nbchans_max = 0;
unsigned int nbservs = 0;
unsigned int nbservs_max = 0;
/* keep track of logged umodes et cmodes, init to 0 by default */
int log_umode[256];
int log_cmode[256];

/* check if nbchans > nbchans_max */
void do_checknbchansmax()
{
	if (nbchans > nbchans_max)
	{
		nbchans_max = nbchans;
		db_query
			("UPDATE " TBL_MAXV
			 " SET val=\'%d\', time=NOW() WHERE type='channels'",
			 nbchans_max);
	}
}

/* check if nbusers > nbusers_max */
void do_checknbusersmax()
{
	if (nbusers > nbusers_max)
	{
		nbusers_max = nbusers;
		db_query
			("UPDATE " TBL_MAXV
			 " SET val=\'%d\', time=NOW() WHERE type='users'", nbusers_max);
	}
}

/* check if nbservs > nbservs_max */
void do_checknbservsmax()
{
	if (nbservs > nbservs_max)
	{
		nbservs_max = nbservs;
		db_query
			("UPDATE " TBL_MAXV
			 " SET val=\'%d\', time=NOW() WHERE type='servers'", nbservs_max);
	}
}

/*************************************************************************/
/*
 * general purpose functions
 */

#if !(defined(IRCD_UNREAL)||defined(IRCD_HYBRID)||defined(IRCD_ULTI28))
/* converts an IP numeric to a char * */
static char *do_ipdecode(char *ipaddr)
{
	static char buf[16];			  /* enough for an IPv4 address. TODO IPv6 ? */
	unsigned int ip = (unsigned int) strtoul(ipaddr, (char **) NULL, 10);
	snprintf(buf, 16, "%u.%u.%u.%u", (unsigned int) ip >> 24,
				(unsigned int) (ip & 0xff0000) >> 16,
				(unsigned int) (ip & 0xff00) >> 8, (unsigned int) ip & 0xff);
	return buf;
}
#endif

/* adds modes to a given chanid */
static void do_chanmodes(int chanid, char **av, int ac)
{
	/* the ircd parses the mode before forwarding it to thales, so there's no buffer overrun
	 * possibility here.   - lucas
	 */
	int atleastone = 0;
	char db[1000];					  /* should be enough for long queries */
	char tmp[14] = "mode_XX=\"X\", ";
	char *modes = av[0];
	int argptr = 1;
	if (*modes == '0')
		return;
	strcpy(db, "UPDATE " TBL_CHAN " SET ");
	while (*modes)
	{
		switch (*modes)
		{
		case '+':
			tmp[9] = 'Y';
			break;
		case '-':
			tmp[9] = 'N';
			break;
		default:
			if (!strchr(CHANMODES, *modes))
			{
				if (!log_cmode[(int) *modes])
				{
					mylog("unknown mode : chanmode %c (in %s)", *modes, inbuf);
					log_cmode[(int) *modes] = TRUE;
				}
			}
#ifdef IRCD_ULTI28
			else if (*modes == 'b' || *modes == 'e' || *modes == 'f')	/* ignore them */
				argptr++;
#else
			else if (*modes == 'b' || *modes == 'e' || *modes == 'I')	/* ignore them */
				argptr++;
#endif
#ifdef IRCD_IRCDRU
			else if (*modes == 'B' || *modes == 'X' || *modes == 'E')	/* ignore them too */
				argptr++;
#endif

#if defined(IRCD_BAHAMUT)||defined(IRCD_IRCDRU)
			else if (*modes == 'o' || *modes == 'v')
#elif defined(IRCD_HYBRID)||defined(IRCD_ULTI28)
			else if (*modes == 'o' || *modes == 'v' || *modes == 'h')
#elif defined(IRCD_ULTIMATE)
			else if (*modes == 'o' || *modes == 'v'
						|| *modes == 'a' || *modes == 'h')
#elif defined(IRCD_UNREAL)
			else if (*modes == 'o' || *modes == 'v'
						|| *modes == 'a' || *modes == 'h' || *modes == 'q')
#endif
			{
				char *user;
				user = db_escape(av[argptr++]);
				db_query
					("UPDATE " TBL_ISON
					 " SET mode_l%c=\"%c\" WHERE chanid=\"%d\" AND nickid=\"%d\"",
					 *modes, tmp[9], chanid, db_getnick(user));
				free(user);
			}
			else
			{
				atleastone = 1;
				tmp[5] = ((*modes >= 'a') ? 'l' : 'u');
				tmp[6] = tolower(*modes);
				strcat(db, tmp);
				if (*modes == 'k')
				{
					if (tmp[9] == 'Y')
					{
						char *key = db_escape(av[argptr++]);
						if (HidePasswords)
						{
							strcat(db, "mode_lk_data=\"HIDDEN\", ");
						}
						else
						{
							strcat(db, "mode_lk_data=\"");
							strcat(db, key);
							strcat(db, "\", ");
						}
						free(key);
					}
					else
					{
						strcat(db, "mode_lk_data=\"\", ");
						argptr++;	  /* mode -k needs a parameter */
					}
				}
				else if (*modes == 'l')
				{
					if (tmp[9] == 'Y')
					{
						strcat(db, "mode_ll_data=\"");
						strcat(db, av[argptr++]);
						strcat(db, "\", ");
					}
					else
					{
						strcat(db, "mode_ll_data=\"\", ");
					}
				}
#if defined(IRCD_UNREAL)||defined(IRCD_ULTI28)
				else if (*modes == 'L')
				{
					if (tmp[9] == 'Y')
					{
						char *ch = db_escape(av[argptr++]);
						strcat(db, "mode_ul_data=\"");
						strcat(db, ch);
						strcat(db, "\", ");
						free(ch);
					}
					else
					{
						strcat(db, "mode_ul_data=\"\", ");
					}
				}
#endif
#ifdef IRCD_UNREAL
				else if (*modes == 'f')
				{
					if (tmp[9] == 'Y')
					{
						strcat(db, "mode_lf_data=\"");
						strcat(db, av[argptr++]);
						strcat(db, "\", ");
					}
					else
					{
						strcat(db, "mode_lf_data=\"\", ");
					}
				}
				else if (*modes == 'j')
				{
					if (tmp[9] == 'Y')
					{
						strcat(db, "mode_lj_data=\"");
						strcat(db, av[argptr++]);
						strcat(db, "\", ");
					}
					else
					{
						strcat(db, "mode_lj_data=\"\", ");
					}
				}
#endif
			}
			break;
		}
		modes++;
	}
#ifndef NOMODES
	if (atleastone)
	{
		sprintf(&db[strlen(db) - 2], " WHERE chanid=\'%d\'", chanid);
		db_query(db);
	}
#endif /* !NOMODES */
}

/* add one or more users to a chanid */
static void do_addusers(int chanid, char *users)
{
	int op, voice, halfop, owner, protect;
	char *nextusers;
	int nickid;
	while (users && (*users))
	{
		/* Neostats is broken and sends an additional space. */
		if (*users == ' ')
			users++;
#if defined(IRCD_UNREAL)
		/* Unreal uses SJOIN to send bans and exempts. Just ignore them. */
		if ((*users == '&') || (*users == '\"'))
		{
			nextusers = strchr(users, ' ');
			if (nextusers)
				*nextusers = '\0';
			users = nextusers;
			if (users)
				users++;
			continue;
		}
#endif
		op = 0;
		halfop = 0;
		voice = 0;
		owner = 0;
		protect = 0;
#if defined(IRCD_ULTIMATE)||defined(IRCD_ULTI28)
#if defined(IRCD_ULTIMATE)
		if ((*users == '!') || (*users == '¤'))
		{
			users++;
			owner = 1;
		}
#endif
		if (*users == '*')
		{
			users++;
			owner = 1;
		}
		if (*users == '%')
		{
			users++;
			halfop = 1;
		}
#endif
		if (*users == '@')
		{
			users++;
			op = 1;
		}
		if (*users == '+')
		{
			users++;
			voice = 1;
		}
#if defined(IRCD_UNREAL)||defined(IRCD_HYBRID)
		if (*users == '%')
		{
			users++;
			halfop = 1;
		}
#endif
#if defined(IRCD_UNREAL)
		if (*users == '*')
		{
			users++;
			owner = 1;
		}
		if (*users == '~')
		{
			users++;
			protect = 1;
		}
#endif
		nextusers = strchr(users, ' ');
		if (nextusers)
			*nextusers = '\0';
		users = db_escape(users);
		nickid = db_checknick(users);
		if (nickid != -1)
		{
#if defined(IRCD_BAHAMUT)||defined(IRCD_IRCDRU)
			db_query("INSERT INTO " TBL_ISON
						" (nickid, chanid, mode_lo, mode_lv) VALUES (\'%d\', \'%d\',\'%s\',\'%s\')",
						nickid, chanid, (op ? "Y" : "N"), (voice ? "Y" : "N"));
#elif defined(IRCD_UNREAL)
			db_query("INSERT IGNORE INTO " TBL_ISON
						" (nickid, chanid, mode_lo, mode_lv, mode_lq, mode_lh, mode_la) VALUES (\'%d\', \'%d\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\')",
						nickid, chanid, (op ? "Y" : "N"), (voice ? "Y" : "N"),
						(owner ? "Y" : "N"), (halfop ? "Y" : "N"),
						(protect ? "Y" : "N"));
#elif defined(IRCD_HYBRID)
			db_query("INSERT INTO " TBL_ISON
						" (nickid, chanid, mode_lo, mode_lv, mode_lh) VALUES (\'%d\', \'%d\',\'%s\',\'%s\', \'%s\')",
						nickid, chanid, (op ? "Y" : "N"), (voice ? "Y" : "N"),
						(halfop ? "Y" : "N"));
#elif defined(IRCD_ULTI28)
			db_query("INSERT INTO " TBL_ISON
						" (nickid, chanid, mode_lo, mode_lv, mode_lh) VALUES (\'%d\', \'%d\',\'%s\',\'%s\',\'%s\')",
						nickid, chanid, (op ? "Y" : "N"), (voice ? "Y" : "N"),
						(halfop ? "Y" : "N"));
#elif defined(IRCD_ULTIMATE)
			db_query("INSERT INTO " TBL_ISON
						" (nickid, chanid, mode_lo, mode_lv, mode_lh, mode_la) VALUES (\'%d\', \'%d\',\'%s\',\'%s\',\'%s\',\'%s\')",
						nickid, chanid, (op ? "Y" : "N"), (voice ? "Y" : "N"),
						(halfop ? "Y" : "N"), (owner ? "Y" : "N"));
#endif
		}
		else
		{
			mylog("received join of non-existing user %s on channel ID %d",
					users, chanid);
		}
		free(users);
		users = nextusers;
		if (users)
			users++;
	}
}

/* add modes to a nickid */
static void do_usermodes(int nickid, char *modes)
{
#ifndef NOMODES
	int atleastone = 0;
	char db[1000];					  /*should be enough for long queries */
	char tmp[14] = "mode_XX=\"X\", ";
	strcpy(db, "UPDATE " TBL_USER " SET ");
	while (*modes)
	{
		switch (*modes)
		{
		case '+':
			tmp[9] = 'Y';
			break;
		case '-':
			tmp[9] = 'N';
			break;
		default:
			if (!strchr(USERMODES, *modes))
			{
				if (!log_umode[(int) *modes])
				{
					mylog("unknown mode : usermode %c (in %s)", *modes, inbuf);
					log_umode[(int) *modes] = TRUE;
				}
			}
			else
			{
				atleastone = 1;
				tmp[5] = ((*modes >= 'a') ? 'l' : 'u');
#if defined(IRCD_UNREAL)||defined(IRCD_ULTIMATE)
				if (tmp[9] == 'N' && *modes == 'x')	/* lame Unreal ... */
					db_query
						("UPDATE " TBL_USER
						 " SET hiddenhostname='*' WHERE nickid='%d'", nickid);
#endif
				tmp[6] = tolower(*modes);
				strcat(db, tmp);
			}
			break;
		}
		modes++;
	}
	if (atleastone)
	{
		sprintf(&db[strlen(db) - 2], " WHERE nickid=\'%d\'", nickid);
		db_query(db);
	}
#endif /* !NOMODES */
}

/*************************************************************************/
/*
 * IRC messages handlers
 */

/* SERVER */
void do_server(char *server, char *comment, char *linkedto)
{
	int servid;
	int add = 1;
	server = db_escape(server);
	comment = db_escape(comment);
	linkedto = db_escape(linkedto);
	if (ServerCacheTime && ((servid = db_checkserver(server)) != -1))
	{
		db_query("UPDATE " TBL_SERV
					" SET server=\"%s\", comment=\"%s\", linkedto=\"%d\", connecttime=NOW(), online=\"Y\" WHERE servid=\"%d\"",
					server, comment, db_getserver(linkedto), servid);
		db_addserver(server, servid);
		add = 0;
	}
	if (add)
	{
		db_query
			("INSERT INTO " TBL_SERV
			 " (server, comment, linkedto, connecttime) VALUES(\'%s\','\%s\',\'%d\', NOW())",
			 server, comment, db_getserver(linkedto));
		db_addserver(server, db_insertid());
	}
	if (ServerCacheTime)
		db_cleanserver();
	free(server);
	free(comment);
	free(linkedto);
	nbservs++;
	do_checknbservsmax();
}

/* SQUIT */
void do_squit(char *server)
{
	server = db_escape(server);
	if (ServerCacheTime)
	{
		db_query("UPDATE " TBL_SERV
					" SET online=\"N\", lastsplit=NOW(),linkedto=NULL WHERE servid=\"%d\"",
					db_getserver(server));
		db_cleanserver();
	}
	else
	{
		db_query("DELETE FROM " TBL_SERV " WHERE server=\'%s\'", server);
		db_delserver(server);
	}
	free(server);
	nbservs--;
}

/* NICK (new nick) */
void do_nick_new(int ac, char **av)
{
	char *nick, *realname, *hostname, *username, *serv;
#ifndef IRCD_ULTI28
	char *hiddenhost;
#endif
#if !(defined(IRCD_UNREAL)||defined(IRCD_HYBRID)||defined(IRCD_ULTI28))
	char *ipaddr;
#endif
	unsigned int connecttime;
	int servid;
	int add = 1;
	int nickid;

	nick = db_escape(av[0]);
	connecttime = atoi(av[2]);
#if defined(IRCD_BAHAMUT)||defined(IRCD_ULTIMATE)||defined(IRCD_IRCDRU)||defined(IRCD_HYBRID)
	username = db_escape(av[4]);
	hostname = db_escape(av[5]);
	serv = db_escape(av[6]);
#if defined(IRCD_HYBRID)
	hiddenhost = "";
	realname = db_escape(av[7]);
#else
	hiddenhost = "";
	ipaddr = do_ipdecode(av[8]);
	realname = db_escape(av[9]);
#endif
#elif defined(IRCD_UNREAL)||defined(IRCD_ULTI28)
	username = db_escape(av[3]);
	hostname = db_escape(av[4]);
	serv = db_escape(av[5]);
#ifdef IRCD_UNREAL
	hiddenhost = db_escape(av[8]);
	realname = db_escape(av[9]);
#else
	realname = db_escape(av[7]);
#endif
#endif
	servid = db_getserver(serv);

	if (UserCacheTime && ((nickid = db_checknick(nick)) != -1))
	{
#if defined(IRCD_BAHAMUT)||defined(IRCD_IRCDRU)	/* with nickip */
		db_query
			("UPDATE " TBL_USER
			 " SET nick=\'%s\', realname=\'%s\', hostname=\'%s\', ipaddr=\'%s\', username=\'%s\', connecttime=FROM_UNIXTIME(\'%d\'), servid=\'%d\',lastquit=NULL, online=\'Y\', away=\'N\', awaymsg=\'\' WHERE nickid=\'%d\'",
			 nick, realname, hostname, ipaddr, username, connecttime, servid,
			 nickid);
#elif defined(IRCD_ULTIMATE)	  /* with nickip & hiddenhost */
		db_query
			("UPDATE " TBL_USER
			 " SET nick=\'%s\', realname=\'%s\', hostname=\'%s\', hiddenhostname=\"%s\", ipaddr=\'%s\', username=\'%s\', connecttime=FROM_UNIXTIME(\'%d\'), servid=\'%d\', lastquit=NULL, online=\'Y\', away=\'N\', awaymsg=\'\' WHERE nickid=\'%d\'",
			 nick, realname, hostname, hiddenhost, ipaddr, username,
			 connecttime, servid, nickid);
#elif defined(IRCD_UNREAL)||defined(IRCD_HYBRID)	/* with hiddenhost */
		db_query
			("UPDATE " TBL_USER
			 " SET nick=\'%s\', realname=\'%s\', hostname=\'%s\', hiddenhostname=\"%s\", username=\'%s\', connecttime=FROM_UNIXTIME(\'%d\'), servid=\'%d\', lastquit=NULL, online=\'Y\', away=\'N\', awaymsg=\'\' WHERE nickid=\'%d\'",
			 nick, realname, hostname, hiddenhost, username, connecttime,
			 servid, nickid);
#elif defined(IRCD_ULTI28)		  /* without nickip */
		db_query
			("UPDATE " TBL_USER
			 " SET nick=\'%s\', realname=\'%s\', hostname=\'%s\', username=\'%s\', connecttime=FROM_UNIXTIME(\'%d\'), servid=\'%d\',lastquit=NULL, online=\'Y\', away=\'N\', awaymsg=\'\' WHERE nickid=\'%d\'",
			 nick, realname, hostname, username, connecttime, servid, nickid);
#endif
		add = 0;
	}
	if (add)
	{
#if defined(IRCD_BAHAMUT)||defined(IRCD_IRCDRU)	/* with nickip */
		db_query
			("INSERT INTO " TBL_USER
			 " (nick, realname, hostname, ipaddr, username, connecttime, servid) VALUES(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',FROM_UNIXTIME(\'%d\'),\'%d\')",
			 nick, realname, hostname, ipaddr, username, connecttime, servid);
#elif defined(IRCD_ULTIMATE)	  /* with nickip & hiddenhost */
		db_query
			("INSERT INTO " TBL_USER
			 " (nick, realname, hostname, hiddenhostname, ipaddr, username, connecttime, servid) VALUES(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',FROM_UNIXTIME(\'%d\'),\'%d\')",
			 nick, realname, hostname, hiddenhost, ipaddr, username,
			 connecttime, servid);
#elif defined(IRCD_UNREAL)||defined(IRCD_HYBRID)	/* with hiddenhost */
		db_query
			("INSERT INTO " TBL_USER
			 " (nick, realname, hostname, hiddenhostname, username, connecttime, servid) VALUES(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',FROM_UNIXTIME(\'%d\'),\'%d\')",
			 nick, realname, hostname, hiddenhost, username, connecttime,
			 servid);
#elif defined(IRCD_ULTI28)		  /* without nickip */
		db_query
			("INSERT INTO " TBL_USER
			 " (nick, realname, hostname, username, connecttime, servid) VALUES(\'%s\',\'%s\',\'%s\',\'%s\',FROM_UNIXTIME(\'%d\'),\'%d\')",
			 nick, realname, hostname, username, connecttime, servid);
#endif
		db_addnick(nick, db_insertid());
	}
	if (UserCacheTime)
		db_cleanuser();
	free(nick);
	free(username);
	free(hostname);
	free(serv);
#if defined(IRCD_UNREAL)
	free(hiddenhost);
#endif
	free(realname);
#if defined(IRCD_BAHAMUT)||defined(IRCD_ULTIMATE)||defined(IRCD_IRCDRU)||defined(IRCD_HYBRID)
	do_usermodes(db_insertid(), av[3]);
#elif defined(IRCD_UNREAL)
	do_usermodes(db_insertid(), av[7]);
#endif
	nbusers++;
	do_checknbusersmax();
}

/* NICK (nick change) */
void do_nick_chg(char *newnick, char *oldnick)
{
	int nickid;

	newnick = db_escape(newnick);
	oldnick = db_escape(oldnick);
	/* the target nickname might already exist if caching is enabled */
	if (UserCacheTime && (strcasecmp(newnick, oldnick))
		 && ((nickid = db_checknick(newnick)) != -1))
	{
		/* In this case, we don't keep a record of the old nick. It would be :
		 * - technically difficult, because we'd have to make a copy of the record
		 * - dangerous, because it would provide an easy way to fill up the DB */
		db_delnick(newnick);
		db_query("DELETE from " TBL_USER " WHERE nickid=\'%d\'", nickid);
	}
	/* we update the nickname */
	db_query("UPDATE " TBL_USER " SET nick=\'%s\' WHERE nick=\'%s\'",
				newnick, oldnick);
	db_chgnick(newnick, oldnick);
	free(newnick);
	free(oldnick);
}

/* PART */
void do_part(char *chan, char *nick)
{
	chan = db_escape(chan);
	nick = db_escape(nick);
	db_query("DELETE FROM " TBL_ISON
				" WHERE nickid=\'%d\' AND chanid=\'%d\'", db_getnick(nick),
				db_getchannel(chan));
	db_checkemptychan(db_getchannel(chan), chan);
	free(chan);
	free(nick);
}

/* JOIN - join 0 */
void do_partall(char *nick)
{
	nick = db_escape(nick);
	db_removefromchans(db_getnick(nick));
	free(nick);
}

/* JOIN */
void do_join(char *chan, char *nick)
{
	chan = db_escape(chan);
	/* we don't escape nick because do_addusers does. */
	do_addusers(db_getchancreate(chan), nick);
	free(chan);
}

/* QUIT */
void do_quit(char *nick)
{
	nick = db_escape(nick);
	db_removenick(nick);			  /* sql */
	if (UserCacheTime)
	{
		db_cleanuser();
	}
	else
	{
		db_delnick(nick);			  /* hash */
	}
	free(nick);
	nbusers--;
}

/* SJOIN */
void do_sjoin(char *chan, char *users, char **modes, int nbmodes)
{
	chan = db_escape(chan);
	do_addusers(db_getchancreate(chan), users);
	if (nbmodes)
		do_chanmodes(db_getchannel(chan), modes, nbmodes);
	free(chan);
}

/* [SVS]MODE - user */
void do_umode(char *nick, char *modes)
{
	nick = db_escape(nick);
	do_usermodes(db_getnick(nick), modes);
	free(nick);
}

/* [SVS]MODE - channel */
void do_cmode(char *chan, char **modes, int nbmodes)
{
	chan = db_escape(chan);
	do_chanmodes(db_getchannel(chan), modes, nbmodes);
	free(chan);
}

/* TOPIC */
/* hybrid7 patches -iwes */
#ifdef IRCD_HYBRID
void do_topic(char *author, char *chan, char *topic)
{
	chan = db_escape(chan);
	author = db_escape(author);
	topic = db_escape(topic);
	db_query
		("UPDATE " TBL_CHAN
		 " SET topic=\'%s\', topicauthor=\'%s\', topictime=NOW() WHERE chanid=\'%d\'",
		 topic ? topic : "", author, db_getchannel(chan));
	free(chan);
	free(author);
	if (topic)
		free(topic);
}
#else	/* regular topic */
void do_topic(char *chan, char *author, char *time, char *topic)
{
	chan = db_escape(chan);
	author = db_escape(author);
	topic = db_escape(topic);
	db_query
		("UPDATE " TBL_CHAN
		 " SET topic=\'%s\', topicauthor=\'%s\', topictime=FROM_UNIXTIME(\'%s\') WHERE chanid=\'%d\'",
		 topic ? topic : "", author, time, db_getchannel(chan));
   
	if (db_getchmode(chan, "ls") == 0 && db_getchmode(chan, "lp") == 0) {
		db_query
                	("INSERT IGNORE INTO " TBL_TOPI
			 " (channel, topic, author, timestamp) VALUES (\'%s\', \'%s\', \'%s\', \'%s\')",
        	         chan, topic ? topic : "", author, time);
		db_query
			("DELETE FROM " TBL_TOPI
			 " WHERE channel=\'%s\' AND topicid NOT IN (SELECT * FROM (SELECT topicid FROM topics WHERE channel=\'%s\' ORDER BY timestamp DESC LIMIT 10) AS abc)",
			 chan, chan);
	}

	free(chan);
	free(author);
	if (topic)
		free(topic);
}
#endif /* IRCD_HYBRID */

/* TBURST -iwes */
#ifdef IRCD_HYBRID
void do_tburst(char *chan, char *time, char *author, char *topic)
{
	chan = db_escape(chan);
	author = db_escape(author);
	topic = db_escape(topic);
	db_query
		("UPDATE " TBL_CHAN
		 " SET topic=\'%s\', topicauthor=\'%s\', topictime=FROM_UNIXTIME(\'%s\') WHERE chanid=\'%d\'",
		 topic ? topic : "", author, time, db_getchannel(chan));
	free(chan);
	free(author);
	if (topic)
		free(topic);
}
#endif /* IRCD_HYBRID */


/* AWAY set */
void do_away_set(char *nick, char *msg)
{
	nick = db_escape(nick);
	msg = db_escape(msg);
	db_query("UPDATE " TBL_USER
				" SET away=\'Y\', awaymsg=\'%s\' WHERE nickid=\'%d\'", msg,
				db_getnick(nick));
	free(nick);
	free(msg);
}

/* AWAY unset */
void do_away_unset(char *nick)
{
	nick = db_escape(nick);
	db_query("UPDATE " TBL_USER
				" SET away=\'N\', awaymsg=\'\' WHERE nickid=\'%d\'",
				db_getnick(nick));
	free(nick);
}

/* LUSERS queries */
void do_lusers(char *nick)
{
	send_cmd(ServerName,
				"251 %s :There are %d users and %d invisible on %d servers",
				nick, db_getlusers(LUSERS_USERS),
				db_getlusers(LUSERS_USERSINV), db_getlusers(LUSERS_SERV));
	send_cmd(ServerName, "252 %s :%d :IRC Operators online", nick,
				db_getlusers(LUSERS_OPERS));
	send_cmd(ServerName, "254 %s :%d :channels formed", nick, nbchans,
				db_getlusers(LUSERS_CHAN));
	send_cmd(ServerName, "255 %s :I have 0 clients and 1 servers", nick);
	send_cmd(ServerName, "265 %s :Current local users: 0 Max: 0", nick);
	send_cmd(ServerName, "266 %s :Current global users: %d Max: %d", nick,
				db_getlusers(LUSERS_USERSGLOB), db_getlusers(LUSERS_USERSMAX));
}

#if defined(IRCD_UNREAL)||defined(IRCD_ULTIMATE)||defined(IRCD_ULTI28)
/* (CHG|SET)HOST */
void do_defhost(char *user, char *msg)
{
	user = db_escape(user);
	msg = db_escape(msg);
	db_query
		("UPDATE " TBL_USER
		 " SET mode_lx=\'Y\', mode_lt=\'Y\', hiddenhostname=\'%s\' WHERE nickid=\'%d\'",
		 msg, db_getnick(user));
	free(msg);
	free(user);
}

#endif

#if defined(IRCD_UNREAL)||defined(IRCD_ULTI28)

/* (CHG|SET)NAME */
void do_defname(char *user, char *msg)
{
	user = db_escape(user);
	msg = db_escape(msg);
	db_query
		("UPDATE " TBL_USER " SET realname=\'%s\' WHERE nickid=\'%d\'",
		 msg, db_getnick(user));
	free(msg);
	free(user);
}

/* (CHG|SET)IDENT */
void do_defident(char *user, char *msg)
{
	user = db_escape(user);
	msg = db_escape(msg);
	db_query
		("UPDATE " TBL_USER " SET username=\'%s\' WHERE nickid=\'%d\'",
		 msg, db_getnick(user));
	free(msg);
	free(user);
}
#endif

#ifdef IRCD_UNREAL

/* SDESC */
void do_sdesc(char *user, char *msg)
{
	user = db_escape(user);
	msg = db_escape(msg);
	db_query("UPDATE " TBL_SERV " SET comment=\'%s\' WHERE servid=\'%d\'",
				msg, db_getservfromnick(user));
	free(user);
	free(msg);
}

#define SWHOISBUGFREQ 20

void do_swhois(char *user, char *msg)
{
	int nickid;
	static int count = 0;
	user = db_escape(user);
	msg = db_escape(msg);
	nickid = db_getnick_unsure(user);
	if (nickid == -1)
	{
		if (count % SWHOISBUGFREQ == 0)
			wallops(NULL, "ERROR! Received SWHOIS for %s, but %s is not"
					  " registered yet! Please contact lucas@lucas-nussbaum.net"
					  " if you see this. I will only report this once every %d times"
					  " to avoid flooding you.", user, user, SWHOISBUGFREQ);
		count++;
		mylog
			("Received SWHOIS for unregistered user %s. This is an ircd bug. "
			 "Contact lucas@lucas-nussbaum.net if you see this on your"
			 " network.", user);
	}
	else
		db_query
			("UPDATE " TBL_USER " SET swhois=\'%s\' WHERE nickid=\'%d\'",
			 msg, nickid);
	free(user);
	free(msg);
}

#endif

#ifdef IRCD_HYBRID
/* REALHOST */
void do_realhost(char *user, char *host)
{
	user = db_escape(user);
	host = db_escape(host);
	db_query("UPDATE " TBL_USER
				" SET hiddenhostname=\'%s\' WHERE nickid=\'%d\'", host,
				db_getnick(user));
	free(user);
	free(host);
}

#endif
