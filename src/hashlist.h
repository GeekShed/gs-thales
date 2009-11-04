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

#ifndef _HASHLIST_H_
#define _HASHLIST_H_

#define KEYOTHER 1
#define KEYCHAN 2
struct item
{
	char *str;
	int n;
	struct item *next;
};
typedef struct item **hashlist;

hashlist newhashlist();
void hash_add(hashlist h, char *str, int n, int keytype);
void hash_del(hashlist h, char *str, int keytype);
int hash_find_unsure(hashlist h, char *str, int keytype);
int hash_find(hashlist h, char *str, int keytype);
int hash_findel(hashlist h, char *str, int keytype);
void hash_update(hashlist h, char *newinfo, char *str, int keytype);

#endif
