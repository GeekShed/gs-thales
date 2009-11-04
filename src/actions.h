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

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

void do_checknbchansmax();
void do_checknbusersmax();
void do_checknbservsmax();

void do_server(char *server, char *comment, char *linkedto);
void do_squit(char *server);
void do_nick_new(int ac, char **av);
void do_nick_chg(char *newnick, char *oldnick);
void do_part(char *chan, char *nick);
void do_partall(char *nick);
void do_join(char *chan, char *nick);
void do_quit(char *nick);
void do_sjoin(char *chan, char *users, char **modes, int nbmodes);
void do_umode(char *nick, char *modes);
void do_cmode(char *chan, char **modes, int nbmodes);
#ifdef IRCD_HYBRID
void do_topic(char *author, char *chan, char *topic);
#else
void do_topic(char *chan, char *author, char *time, char *topic);
#endif /* IRCD_HYBRID */
void do_away_set(char *nick, char *msg);
void do_away_unset(char *nick);
void do_lusers(char *nick);
#if defined(IRCD_UNREAL)||defined(IRCD_ULTIMATE)||defined(IRCD_ULTI28)
void do_defhost(char *user, char *msg);
#endif
#if defined(IRCD_UNREAL)||defined(IRCD_ULTI28)
void do_defname(char *user, char *msg);
void do_defident(char *user, char *msg);
void do_sdesc(char *user, char *msg);
#endif
#if defined(IRCD_UNREAL)
void do_swhois(char * user, char * msg);
#endif
#ifdef IRCD_HYBRID
void do_realhost(char *user, char *host);
#endif

/* tburst stuff -iwes */
#ifdef IRCD_HYBRID
void do_tburst(char *chan, char *time, char *author, char *topic);
#endif /* IRCD_HYBRID */

/* available modes for each ircd */
#if defined(IRCD_BAHAMUT)
#define USERMODES "abcdefghiknorswyAR"
#define CHANMODES "beIovciklmnprstLMRO"
#elif defined(IRCD_IRCDRU)
#define USERMODES "abcdefghiknorswyAR"
#define CHANMODES "beIBXEovciklmnw7prstLMRO"
#elif defined(IRCD_UNREAL)
#define USERMODES "adghiopqrtvwxzABCDGHNRSTVW"
#define CHANMODES "beIovahqcfijklmnprstuyzACDGHKLMNOQRSTV" /* ChanModes +I and +H are not used anymore in 3.2 beta19 */
#elif defined(IRCD_ULTIMATE)
#define USERMODES "acdefghijkmnoprstwxyACDFJOPSTWZ"
#define CHANMODES "beIovahceiklmnprstKNOR"
#elif defined(IRCD_ULTI28)
#define USERMODES "abcdfghikmnoprswxzABCFGMNOPRSTWXZ"
#define CHANMODES "befhiklmnoprstvxAIKLORS"
#elif defined(IRCD_HYBRID)
#define USERMODES "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define CHANMODES "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

#endif

#endif
