/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2006 Daniel Gollub <dgollub@suse.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#include <ctime>
#include "timegm.h"

/*
 * On solaris no timegm function exists,
 * we must implement it here
 */
std::time_t timegm(struct std::tm *t)
{
	std::time_t tl, tb;
	struct std::tm *tg;

	tl = std::mktime (t);
	if (tl == -1)
	{
		t->tm_hour--;
		tl = std::mktime (t);
		if (tl == -1)
			return -1; /* can't deal with output from strptime */
		tl += 3600;
	}
	tg = std::gmtime (&tl);
	tg->tm_isdst = 0;
	tb = std::mktime (tg);
	if (tb == -1)
	{
		tg->tm_hour--;
		tb = std::mktime (tg);
		if (tb == -1)
			return -1; /* can't deal with output from gmtime */
		tb += 3600;
	}
	return (tl - (tb - tl));
}

