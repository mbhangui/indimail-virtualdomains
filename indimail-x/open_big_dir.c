/*
 * $Log: open_big_dir.c,v $
 * Revision 1.1  2019-04-18 08:31:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "GetPrefix.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: open_big_dir.c,v 1.1 2019-04-18 08:31:45+05:30 Cprogrammer Exp mbhangui $";
#endif

char *
open_big_dir(char *username, char *domain, char *path)
{
	char           *filesys_prefix;

	if (!(filesys_prefix = GetPrefix(username, path)))
		return ((char *) 0);
	if (vread_dir_control(filesys_prefix, &vdir, domain))
		return ((char *) 0);
	return (filesys_prefix);
}
