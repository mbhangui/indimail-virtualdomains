/*
 * $Log: add_user_assign.c,v $
 * Revision 1.1  2019-04-18 07:43:37+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#include <open.h>
#endif
#include "getEnvConfig.h"
#include "update_file.h"
#include "update_newu.h"
#include "get_indimailuidgid.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: add_user_assign.c,v 1.1 2019-04-18 07:43:37+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("add_user_assign: out of memory", 0);
	_exit(111);
}

void
do_assign1(stralloc *s, char *user, char *dir)
{
	char            strnum[FMT_ULONG];
	int             i;

	if (!stralloc_copyb(s, "=", 1) ||
			!stralloc_cats(s, user) ||
			!stralloc_append(s, ":") ||
			!stralloc_cats(s, user) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailuid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailgid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	if (!stralloc_cats(s, DOMAINDIR) || !stralloc_append(s, "/"))
		die_nomem();
	if (dir) {
		if (!stralloc_cats(s, dir) || !stralloc_append(s, "/"))
		die_nomem();
	}
	if (!stralloc_cats(s, user) || !stralloc_0(s))
		die_nomem();
	return;
}

void
do_assign2(stralloc *s, char *user, char *dir)
{
	char            strnum[FMT_ULONG];
	int             i;

	if (!stralloc_copyb(s, "+", 1) ||
			!stralloc_cats(s, user) ||
			!stralloc_catb(s, "-:", 2) ||
			!stralloc_cats(s, user) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailuid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	strnum[i = fmt_ulong(strnum, indimailgid)] = 0;
	if (!stralloc_catb(s, strnum, i) ||
			!stralloc_append(s, ":"))
		die_nomem();
	if (!stralloc_cats(s, DOMAINDIR) || !stralloc_append(s, "/"))
		die_nomem();
	if (dir && *dir) {
		if (!stralloc_cats(s, dir) || !stralloc_append(s, "/"))
		die_nomem();
	}
	if (!stralloc_cats(s, user) || !stralloc_0(s))
		die_nomem();
	return;
}

/*
 * add a local user to the users/assign file and compile it
 */
int
add_user_assign(char *user, char *dir)
{
	static stralloc filename = {0}, tmpstr1 = {0}, tmpstr2 = {0};
	char           *assigndir;
	int             fd;

	/*
	 * stat assign file, if it's not there create one 
	 */
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	if (!stralloc_copys(&filename, assigndir) ||
			!stralloc_catb(&filename, "/assign", 7) ||
			!stralloc_0(&filename))
		die_nomem();
	if (access(filename.s, F_OK)) {
		if ((fd = open_append(filename.s)) == -1) {
			strerr_warn3("add_user_assign: open: ", filename.s, ": ", &strerr_sys);
			return (-1);
		}
		if (write(fd, ".\n", 2) != 2) {
			strerr_warn3("add_user_assign: write: ", filename.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		close(fd);
	}
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	do_assign1(&tmpstr1, user, dir);
	do_assign2(&tmpstr1, user, dir);
	if (update_file(filename.s, tmpstr1.s, INDIMAIL_QMAIL_MODE) || update_file(filename.s, tmpstr2.s, INDIMAIL_QMAIL_MODE))
		return (-1);
	update_newu();
	return (0);
}