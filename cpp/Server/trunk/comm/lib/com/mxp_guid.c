/*
 * $Id: ft_guid.c,v 1.2 2003/05/05 09:49:09 jasta Exp $
 *
 * Copyright (C) 2001-2003 giFT project (gift.sourceforge.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifdef WIN32
#include "config_win32.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <WinSock2.h>
//#include <time.h>
//#include <sys/timeb.h>
#else
#include "config_unix.h"
#include <time.h>
#include <sys/time.h>
#endif
//
#include "mxp_guid.h"
#include "gettimeofday.h"

/*****************************************************************************/
static unsigned int seed = 0;

/*****************************************************************************/
//#ifdef WIN32
//int mxp_gettimeofday(struct timeval *tp, void *p)
//{
//	struct _timeb timebuffer;   
//
//	_ftime( &timebuffer );
//	tp->tv_sec  = timebuffer.time;
//	tp->tv_usec = timebuffer.millitm * 1000;
//	return 0;
//}
//#endif

ft_guid_t *ft_guid_new (void)
{
	ft_guid_t *buf;
	//uint32_t  *buf32;
	int        i;
	struct timeval tv;
	//int seed = 0;

	/*if (!seed)
	{
		struct timeval tv;
		gettimeofday (&tv, NULL);
		seed = tv.tv_usec ^ tv.tv_sec;
		srand (seed);
	}*/
#ifdef WIN32
	Sleep(1);
#else
	usleep(1000);
#endif

//#ifdef WIN32
//	mxp_gettimeofday (&tv, NULL);
//#else
	gettimeofday (&tv, NULL);
//#endif
	seed = tv.tv_usec ^ tv.tv_sec;
	srand (seed);
	seed = rand();

	if (!(buf = malloc (FT_GUID_SIZE)))
		return NULL;

	//buf32 = (uint32_t *)buf;
	for (i = 0; i < FT_GUID_SIZE; i++)
	{
		srand((unsigned int)time(NULL)+seed);
		seed >>= 1;
		//buf32[i] = rand () % 256;
		buf[i] = rand () % 256;
	}
	return buf;
}

void ft_guid_free (ft_guid_t *guid)
{
	free (guid);
}

/*****************************************************************************/

ft_guid_t *ft_guid_dup (ft_guid_t *src)
{
	ft_guid_t *dst;

	if (!src)
		return NULL;

	if (!(dst = malloc (FT_GUID_SIZE)))
		return NULL;

	memcpy (dst, src, FT_GUID_SIZE);
	return dst;
}

/*****************************************************************************/

char *ft_guid_fmt (ft_guid_t *guid)
{
	static char buf[64];
	char        buf2[4];
	int         i;

	if (!guid)
		return "(null)";

	sprintf(buf, "%s", "");

	for (i = 0; i < FT_GUID_SIZE; i++)
	{
		sprintf (buf2, "%02X", guid[i]);
		strncat (buf, buf2, 2);
	}

	return buf;
}


char * ft_guid_str (void)
{
	char *buf;
	ft_guid_t *guid;
	if (!(guid = ft_guid_new()))
		return "(null)";

	buf = ft_guid_fmt (guid);
	ft_guid_free (guid);
	return buf;
}

/*****************************************************************************/

#if 0
int main (int argc, char **argv)
{
	Dataset   *guids;
	ft_guid_t *guid;
	int        i;

	guids = dataset_new (DATASET_HASH);
	assert (guids != NULL);

	for (i = 0 ;; i++)
	{
		guid = ft_guid_new ();
		assert (guid != NULL);

		if (dataset_lookup (guids, guid, FT_GUID_SIZE))
			break;

		dataset_insert (&guids, guid, FT_GUID_SIZE, "ft_guid_t", 0);

		if (i % 100000 == 0)
			printf ("i=%i...\n", i);
	}

	printf ("collision detected, i=%i\n", i);

	return 0;
}
#endif
