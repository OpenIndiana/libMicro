/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright 2005 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 * Copyright 2019, Joyent, Inc.
 */

/*
 * memcpy
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libmicro.h"

#define	DEFS			8192
#define	DEFR			1

static long long		opts = DEFS;
static int			optf;
static int			opta;

typedef struct {
	char			*ts_src;
	int			ts_srcsize;
	size_t			ts_fakegcc;
} tsd_t;

int
benchmark_init()
{
	lm_tsdsize = sizeof (tsd_t);

	(void) sprintf(lm_optstr, "a:s:ft");

	(void) sprintf(lm_usage,
	    "       [-s buffer-size (default %d)]\n"
	    "       [-a relative alignment (default page aligned)]\n"
	    "       [-f (rotate \"from\" buffer to keep it out of cache)]\n"
	    "notes: measures memchr()\n",
	    DEFS);

	(void) sprintf(lm_header, "%8s", "size");

	return (0);
}

int
benchmark_optswitch(int opt, char *optarg)
{
	switch (opt) {
	case 'f':
		optf++;
		break;
	case 's':
		opts = sizetoll(optarg);
		break;
	case 'a':
		opta = sizetoint(optarg);
		break;
	default:
		return (-1);
	}
	return (0);
}

int
benchmark_initworker(void *tsd)
{
	tsd_t			*ts = (tsd_t *)tsd;

	if (optf)
		ts->ts_srcsize = 64 * 1024 * 1024;
	else
		ts->ts_srcsize = opts + opta;

	ts->ts_src = opta + (char *)valloc(ts->ts_srcsize);

	return (0);
}

int
benchmark(void *tsd, result_t *res)
{
	tsd_t			*ts = (tsd_t *)tsd;
	char			*src = ts->ts_src;

	int bump = (int)opts;

	if (bump < 1024)
		bump = 1024; /* avoid prefetched area */

	ts->ts_fakegcc += (uintptr_t)memchr(src, 'X', opts);
	if (optf) {
		src += bump;
		if (src + opts > ts->ts_src + ts->ts_srcsize)
			src = ts->ts_src;
	}

	return (0);
}

char *
benchmark_result()
{
	static char		result[256];

	(void) sprintf(result, "%8lld", opts);

	return (result);
}
