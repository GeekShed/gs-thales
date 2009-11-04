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
#include "messages.h"
#include "send.h"
#include "misc.h"
#include "compat.h"
#include "memory.h"
#include "log.h"
#include "actions.h"
#include "sockutil.h"

extern char *ServerName;
extern char *RemoteServer;
extern time_t start_time;

extern unsigned int nbusers;
extern unsigned int nbusers_max;
extern unsigned int nbchans;
extern unsigned int nbchans_max;

/* List of messages is at the bottom of the file. */

/* PING - handled */
static void m_ping(char *source, int ac, char **av)
{
	if (ac < 1)
		return;
	send_cmd(ServerName, "PONG %s %s", ac > 1 ? av[1] : ServerName, av[0]);
}

/* SERVER - forwarded */
static void m_server(char *source, int ac, char **av)
{
	if (*source)
	{
		do_server(av[0], av[2], source);
	}
	else
	{
#ifdef IRCD_UNREAL
		do_server(av[0], strchr(av[2], ' ') + 1, ServerName);
#else
		do_server(av[0], av[2], ServerName);
#endif
	}
}

/* SQUIT - forwarded */
static void m_squit(char *source, int ac, char **av)
{
	do_squit(av[0]);
}

/* NICK - forwarded */
static void m_nick(char *source, int ac, char **av)
{
	if (*source)
		do_nick_chg(av[0], source);
	else
		do_nick_new(ac, av);
}

/* KICK - forwarded */
static void m_kick(char *source, int ac, char **av)
{
	do_part(av[0], av[1]);		  /* kicks are handled like parts */
}

/* PART - forwarded */
static void m_part(char *source, int ac, char **av)
{
	do_part(av[0], source);

}

/* ERROR - handled */
static void m_error(char *source, int ac, char **av)
{
	fatal(av[0][0] == ':' ? av[0] + 1 : av[0]);
}

/* JOIN - forwarded */
static void m_join(char *source, int ac, char **av)
{
#ifdef IRCD_ULTI28
	char *pos, *oldpos;
	/* LAME ultimate 2.8 with multiple channels in the same JOIN ... */
	oldpos = av[0];
	while ((pos = strchr(oldpos, ',')) != NULL)
	{
		*pos = '\0';
		do_join(oldpos, source);
		oldpos = pos + 1;
	}
	do_join(oldpos, source);
#else
	if (!strcmp(av[0], "0"))
		do_partall(source);
	else
		do_join(av[0], source);
#endif
}

/* QUIT - forwarded */
static void m_quit(char *source, int ac, char **av)
{
	do_quit(source);
}

/* SJOIN - forwarded */
static void m_sjoin(char *source, int ac, char **av)
{

#if defined(IRCD_BAHAMUT)||defined(IRCD_ULTIMATE)||defined(IRCD_IRCDRU)
	if (ac == 2)
		do_join(av[1], source);	  /* like a join */
	else
#endif
		do_sjoin(av[1], av[ac - 1], &av[2], (ac > 3) ? ac - 3 : 0);
}

/* SVSMODE - forwarded */
static void m_svsmode(char *source, int ac, char **av)
{
	if (*av[0] != '#')
#if defined(IRCD_BAHAMUT)||defined(IRCD_ULTIMATE)||defined(IRCD_IRCDRU)||defined(IRCD_HYBRID)
		do_umode(av[0], av[2]);
#elif defined(IRCD_UNREAL)||defined(IRCD_ULTI28)
		do_umode(av[0], av[1]);
#endif
	else
		do_cmode(av[0], &av[1], ac - 2);
}

/* MODE - forwarded */
static void m_mode(char *source, int ac, char **av)
{
	if (*av[0] != '#')
		do_umode(av[0], av[1]);
	else
		do_cmode(av[0], &av[1], ac - 1);
}

/* TOPIC - forwarded */
static void m_topic(char *source, int ac, char **av)
{
#ifdef IRCD_HYBRID
	do_topic(source, av[0], av[1]);
#else
	do_topic(av[0], av[1], av[2], av[3]);
#endif /* IRCD_HYBRID */
}

/* TBURST - forwarded -iwes */
#ifdef IRCD_HYBRID
static void m_tburst(char *source, int ac, char **av)
{
	do_tburst(av[1], av[2], av[3], av[4]);
}
#endif /* IRCD_HYBRID */

/* KILL - forwarded */
static void m_kill(char *source, int ac, char **av)
{
	do_quit(av[0]);
}

/* AWAY - forwarded */
static void m_away(char *source, int ac, char **av)
{
	if (!ac)
		do_away_unset(source);
	else
		do_away_set(source, av[0]);
}

/* VERSION - handled */
static void m_version(char *source, int ac, char **av)
{
	if (source)
		send_cmd(ServerName, "351 %s GNU Thales %s %s",
					source, VERSION, ServerName);
}

/* STATS - handled */
static void m_stats(char *source, int ac, char **av)
{
	if (ac < 1)
		return;
	switch (*av[0])
	{
	case 'l':
		send_cmd(NULL,
					"211 %s Server SendBuf SentBytes SentMsgs RecvBuf RecvBytes RecvMsgs ConnTime",
					source);
		send_cmd(NULL, "211 %s %s %d %d %d %d %d %d %ld", source,
					RemoteServer, write_buffer_len(), total_written, -1,
					read_buffer_len(), total_read, -1, time(NULL) - start_time);
		send_cmd(NULL, "219 %s l :End of /STATS report.", source);
		break;
	case 'u':
		{
			int uptime;
			uptime = time(NULL) - start_time;
			send_cmd(NULL, "242 %s :GNU Thales up %d day%s, %02d:%02d:%02d",
						source, uptime / 86400, (uptime / 86400 == 1) ? "" : "s",
						(uptime / 3600) % 24, (uptime / 60) % 60, uptime % 60);
			send_cmd(NULL, "219 %s u :End of /STATS report.", source);
			break;
		}
	default:
		send_cmd(NULL, "219 %s %c :End of /STATS report.", source, *av[0]);
		break;
	}
}

/* LUSERS - forwarded */
static void m_lusers(char *source, int ac, char **av)
{
	do_lusers(source);
}

/* MOTD - handled */
static void m_motd(char *source, int ac, char **av)
{
	send_cmd(ServerName, "375 %s :- %s Message of the Day", source,
				ServerName);
	send_cmd(ServerName, "372 %s :- _____ _           _", source);
	send_cmd(ServerName, "372 %s :-|_   _| |__   __ _| | ___  ___", source);
	send_cmd(ServerName, "372 %s :-  | | | '_ \\ / _` | |/ _ \\/ __|",
				source);
	send_cmd(ServerName, "372 %s :-  | | | | | | (_| | |  __/\\__ \\",
				source);
	send_cmd(ServerName, "372 %s :-  |_| |_| |_|\\__,_|_|\\___||___/",
				source);
	send_cmd(ServerName, "372 %s :-           v. %s", source, VERSION);
	send_cmd(ServerName, "372 %s :-    (c)2002-2004 Lucas Nussbaum", source);
	send_cmd(ServerName, "372 %s :-", source);
	send_cmd(ServerName, "372 %s :-GNU Thales is an IRC to MySQL gateway.",
				source);
	send_cmd(ServerName, "372 %s :-   More info is available on", source);
	send_cmd(ServerName, "372 %s :- http://www.gnu.org/software/thales/",
				source);
	send_cmd(ServerName, "376 %s :End of /MOTD command.", source);
}

/* ADMIN - handled */
static void m_admin(char *source, int ac, char **av)
{
	send_cmd(ServerName, "256 %s :Administrative info about %s", source,
				ServerName);
	send_cmd(ServerName, "257 %s :thales-%s : IRC to MySQL Gateway", source,
				VERSION);
	send_cmd(ServerName,
				"258 %s :Written by Lucas Nussbaum <lucas@lucas-nussbaum.net>",
				source);
	send_cmd(ServerName,
				"259 %s :See http://www.gnu.org/software/thales/ for more info.",
				source);
}

/* TIME - handled */
static char *months[] = {
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December"
};

static char *weekdays[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

static void m_time(char *source, int ac, char **av)
{
	static char buf[80], plus;
	struct tm *lt, *gm;
	struct tm gmbuf;
	int minswest;
	time_t clock;

	time(&clock);
	gm = gmtime(&clock);
	memcpy((char *) &gmbuf, (char *) gm, sizeof(gmbuf));
	gm = &gmbuf;
	lt = localtime(&clock);

	if (lt->tm_yday == gm->tm_yday)
		minswest =
			(gm->tm_hour - lt->tm_hour) * 60 + (gm->tm_min - lt->tm_min);
	else if (lt->tm_yday > gm->tm_yday)
		minswest = (gm->tm_hour - (lt->tm_hour + 24)) * 60;
	else
		minswest = ((gm->tm_hour + 24) - lt->tm_hour) * 60;

	plus = (minswest > 0) ? '-' : '+';
	if (minswest < 0)
		minswest = -minswest;

	(void) snprintf(buf, sizeof(buf),
						 "%s %s %d %04d -- %02d:%02d %c%02d:%02d",
						 weekdays[lt->tm_wday], months[lt->tm_mon], lt->tm_mday,
						 lt->tm_year + 1900, lt->tm_hour, lt->tm_min, plus,
						 minswest / 60, minswest % 60);

	send_cmd(ServerName, "391 %s %s :%s", source, ServerName, buf);
}


/*
 * IRCD specific commands
 */

#if defined(IRCD_UNREAL)||defined(IRCD_ULTI28)

/* CHGHOST - forwarded */
static void m_chghost(char *source, int ac, char **av)
{
	do_defhost(av[0], av[1]);
}

/* CHGNAME - forwarded */
static void m_chgname(char *source, int ac, char **av)
{
	do_defname(av[0], av[1]);
}

/* CHGIDENT - forwarded */
static void m_chgident(char *source, int ac, char **av)
{
	do_defname(av[0], av[1]);
}

/* SETNAME - forwarded */
static void m_setname(char *source, int ac, char **av)
{
	do_defname(source, av[0]);
}

/* SETIDENT - forwarded */
static void m_setident(char *source, int ac, char **av)
{
	do_defident(source, av[0]);
}

#endif

#ifdef IRCD_UNREAL

/* SDESC - forwarded */
static void m_sdesc(char *source, int ac, char **av)
{
	do_sdesc(source, av[0]);
}

/* SWHOIS - forwarded */
static void m_swhois(char *source, int ac, char **av)
{
	do_swhois(av[0], av[1]);
}


#endif

#if defined(IRCD_UNREAL)||defined(IRCD_ULTI28)
/* SETHOST - forwarded */
static void m_sethost(char *source, int ac, char **av)
{
	do_defhost(source, av[0]);
}
#endif

#if defined(IRCD_ULTIMATE)

/* SETHOST - forwarded */
static void m_sethost(char *source, int ac, char **av)
{
	do_defhost(av[0], av[1]);
}

#endif

#ifdef IRCD_HYBRID

/* REALHOST - forwarded */
static void m_realhost(char *source, int ac, char **av)
{
	mylog("NBPARAMS : %d", ac);
	if (ac > 1)
		do_realhost(av[0], av[1]);
	else
		do_realhost(av[0], "");
}
#endif

/*************************************************************************/
Message messages[] = {

	{"PASS", NULL}
	,
	{"PING", m_ping}
	,
	{"SERVER", m_server}
	,
	{"VERSION", m_version}
	,
	{"SQUIT", m_squit}
	,
	{"NICK", m_nick}
	,
	{"KILL", m_kill}
	,
	{"SJOIN", m_sjoin}
	,
	{"KICK", m_kick}
	,
	{"SVSMODE", m_svsmode}
	,
	{"PART", m_part}
	,
	{"MODE", m_mode}
	,
	{"TOPIC", m_topic}
	,
#ifdef IRCD_HYBRID
	{"TBURST", m_tburst}
	,
#endif /* IRCD_HYBRID */
	{"QUIT", m_quit}
	,
	{"STATS", m_stats}
	,
	{"JOIN", m_join}
	,
	{"AWAY", m_away}
	,
	{"ERROR", m_error}
	,
	{"SQLINE", NULL}
	,
	{"NOTICE", NULL}
	,
	{"GNOTICE", NULL}
	,
	{"SZLINE", NULL}
	,
	{"CAPAB", NULL}
	,
	{"SVINFO", NULL}
	,
	{"SGLINE", NULL}
	,
	{"UNSGLINE", NULL}
	,
	{"UNSZLINE", NULL}
	,
	{"AKILL", NULL}
	,
	{"GLOBOPS", NULL}
	,
	{"WALLOPS", NULL}
	,
	{"SVSKILL", NULL}
	,
	{"RAKILL", NULL}
	,
	{"CHATOPS", NULL}
	,
	{"LUSERS", m_lusers}
	,
	{"ADMIN", m_admin}
	,
	{"MOTD", m_motd}
	,
	{"TIME", m_time}
	,

#ifdef IRCD_ULTI28
	{"VCTRL", NULL}
	,
	{"PROTOCTL", NULL}
	,
	{"SNETINFO", NULL}
	,
	{"CHGHOST", m_chghost}
	,
	{"CHGIDENT", m_chgident}
	,
	{"CHGNAME", m_chgname}
	,
	{"SETHOST", m_sethost}
	,
	{"SETIDENT", m_setident}
	,
	{"SETNAME", m_setname}
	,
	{"FAILOPS", NULL}
	,
	{"GCONNECT", NULL}
	,
	{"NETINFO", NULL}
	,
	{"ADG", NULL}
	,
	{"NETG", NULL}
	,
	{"NETGLOBAL", NULL}
	,
	{"GNOTICE", NULL}
	,
	{"GLINE", NULL}
	,
	{"GOPER", NULL}
	,
	{"GNOTICE", NULL}
	,
	{"GOPER", NULL}
	,
	{"GCLIENT", NULL}
	,
#endif
#ifdef IRCD_ULTIMATE
	{"VCTRL", NULL}
	,
	{"GCONNECT", NULL}
	,
	{"NETGLOBAL", NULL}
	,
	{"GNOTICE", NULL}
	,
	{"GOPER", NULL}
	,
	{"GNOTICE", NULL}
	,
	{"GOPER", NULL}
	,
	{"NETCTRL", NULL}
	,
	{"EOBURST", NULL}
	,
	{"NETINFO", NULL}
	,
	{"SETHOST", m_sethost}
	,
	{"SMODE", m_mode}
	,
#endif
#ifdef IRCD_UNREAL
	{"HELP", NULL}
	,
	{"PROTOCTL", NULL}
	,
	{"TKL", NULL}
	,
	{"SMO", NULL}
	,
	{"SDESC", m_sdesc}
	,
	{"SVS2MODE", m_svsmode}
	,
	{"GOPER", NULL}
	,
	{"NETINFO", NULL}
	,
	{"CHGHOST", m_chghost}
	,
	{"CHGIDENT", m_chgident}
	,
	{"CHGNAME", m_chgname}
	,
	{"SETHOST", m_sethost}
	,
	{"SETIDENT", m_setident}
	,
	{"SETNAME", m_setname}
	,
	{"SWHOIS", m_swhois}
	,
	{"EOS", NULL}
	,
#endif
#ifdef IRCD_HYBRID
	{"REALHOST", m_realhost}
	,
#endif
	{NULL}
};

/*************************************************************************/

Message *find_message(const char *name)
{
	Message *m;
	for (m = messages; m->name; m++)
		if (stricmp(name, m->name) == 0)
			return m;
	return NULL;
}

/*************************************************************************/
