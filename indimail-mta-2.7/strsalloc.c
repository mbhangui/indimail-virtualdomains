/*
 * $Log: strsalloc.c,v $
 * Revision 1.3  2004-10-22 20:31:01+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-11 14:09:08+05:30  Cprogrammer
 * prevent inclusion of alloc.h with prototypes
 *
 * Revision 1.1  2004-08-15 19:52:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "gen_allocdefs.h"
#include "stralloc.h"
#include "strsalloc.h"

GEN_ALLOC_readyplus(strsalloc, stralloc, sa, len, a, i, n, x, 10, strsalloc_readyplus)
GEN_ALLOC_append(strsalloc, stralloc, sa, len, a, i, n, x, 10, strsalloc_readyplus, strsalloc_append)

void
getversion_strsalloc_c()
{
	static char    *x = "$Id: strsalloc.c,v 1.3 2004-10-22 20:31:01+05:30 Cprogrammer Stab mbhangui $";

	x++;
}