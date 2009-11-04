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

#include "hashlist.h"
#include "log.h"

extern int verbose;

/* creates a new hashlist */
hashlist newhashlist()
{
	hashlist h = (hashlist) malloc(sizeof(struct item *) * 256);
	memset(h, 0, sizeof(struct item *) * 256);
	return h;
}

/* returns an hashcode for the given string */
#define HASHSHIFT 5
int hash(char *s, int keytype)
{
	int h = 0;
	if (verbose)
		/* dont use the # if chan */
		if (keytype == KEYCHAN)
			s++;
	while (*s)
	{
		/* imported from ispell and modified */
		h = (h << HASHSHIFT)
			| ((h >> (8 - HASHSHIFT)) & ((1 << HASHSHIFT) - 1));
		h ^= *s++;
	}
	return h & ((1 << 8) - 1);
}

/* adds an element to the hashlist */
void hash_add(hashlist h, char *str, int n, int keytype)
{
	struct item *pio;
	struct item *pin;
	int key = hash(str, keytype);
	if (verbose)
		mylog("HASH : hash_add %d, %s, %d, %d", (int) h, str, n, keytype);
	pio = h[key];
	pin = (struct item *) malloc(sizeof(struct item));
	pin->str = strdup(str);
	pin->n = n;
	pin->next = pio;
	h[key] = pin;
}

/* deletes an element from the hashlist */
void hash_del(hashlist h, char *str, int keytype)
{
	struct item *pi, **piold;
	if (verbose)
		mylog("HASH : hash_del %d, %s, %d", (int) h, str, keytype);
	for (piold = &h[hash(str, keytype)], pi = *piold; pi; pi = pi->next)
	{
		if (!strcmp(str, pi->str))
		{
			*piold = pi->next;
			free(pi->str);
			free(pi);
			if (hash_find_unsure(h, str, keytype) != -1)
				fatal("deleted %s but still present !", str);
			return;
		}
		piold = &(pi->next);
	}
	fatal("hashlist element %s not found !", str);
}

/* finds an element in the hashlist - crash if not found */
int hash_find(hashlist h, char *str, int keytype)
{
	int res;
	if (verbose)
		mylog("HASH : hash_find %d, %s, %d", (int) h, str, keytype);
	res = hash_find_unsure(h, str, keytype);
	if (res != -1)
		return res;
	fatal("hashlist element %s not found !", str);
	return 0;						  /* prevents compiler warnings */
}

/* finds an element in the hashlist - returns -1 if not found */
int hash_find_unsure(hashlist h, char *str, int keytype)
{
	struct item *pi;
	if (verbose)
		mylog("HASH : hash_find_unsure %d, %s, %d", (int) h, str, keytype);
	for (pi = h[hash(str, keytype)]; pi; pi = pi->next)
		if (!strcmp(str, pi->str))
			return pi->n;
	return -1;
}

/* finds an element in the hashlist, then remove it */
/* Findel, Luxembourg - where I worked when I wrote this :) */
int hash_findel(hashlist h, char *str, int keytype)
{
	int n;
	struct item *pi, **piold;
	if (verbose)
		mylog("HASH : hash_findel %d, %s, %d", (int) h, str, keytype);

	for (piold = &h[hash(str, keytype)], pi = *piold; pi; pi = pi->next)
	{
		if (!strcmp(str, pi->str))
		{
			n = pi->n;
			*piold = pi->next;
			free(pi->str);
			free(pi);
			return n;
		}
		piold = &(pi->next);
	}
	fatal("hashlist element %s not found !", str);
	return 0;						  /* to prevent compiler warnings */
}

/* update an item */
void hash_update(hashlist h, char *newinfo, char *str, int keytype)
{
	int key;
	int key2;
	if (verbose)
		mylog("HASH : hash_update %d, %s, %s, %d", (int) h, newinfo, str,
			 keytype);
	if ((key = hash(str, keytype)) == (key2 = hash(newinfo, keytype)))
	{
		struct item *pi;

		for (pi = h[key]; pi; pi = pi->next)
			if (!strcmp(str, pi->str))
			{
				free(pi->str);
				pi->str = strdup(newinfo);
				return;
			}
		fatal("hashlist element %s not found !", str);
	}
	else
	{
		/* find and delete the old, create the new */
		int n;
		struct item *pi, **piold;
		struct item *pio;

		for (piold = &h[key], pi = *piold; pi; pi = pi->next)
		{
			if (!strcmp(str, pi->str))
			{
				/* save and delete the old entry */
				n = pi->n;
				*piold = pi->next;
				free(pi->str);
				/* create the new */
				pio = h[key2];
				pi->str = strdup(newinfo);
				pi->n = n;
				pi->next = pio;
				h[key2] = pi;
				return;
			}
			piold = &(pi->next);
		}
		fatal("hashlist element %s not found !", str);
	}
}
