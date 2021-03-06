/*
 * $Log: ipasswd.c,v $
 * Revision 1.2  2020-04-01 18:56:04+05:30  Cprogrammer
 * added encrypt flag to mkpasswd()
 *
 * Revision 1.1  2019-04-14 18:29:26+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <mkpasswd.h>
#endif
#include "lowerit.h"
#include "iopen.h"
#include "iclose.h"
#include "sqlOpen_user.h"
#include "sql_getpw.h"
#include "check_quota.h"
#include "vset_lastauth.h"
#include "getpeer.h"
#include "sql_passwd.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: ipasswd.c,v 1.2 2020-04-01 18:56:04+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("ipasswd: out of memory", 0);
	_exit(111);
}

/*
 * update a users virtual password file entry with a different password
 */
int
ipasswd(char *username, char *domain, char *password, int apop)
{
	struct passwd  *pw;
	char           *ptr;
	int             i;
	static stralloc Dir = {0}, Crypted = {0}, email = {0};
	mdir_t          quota;

	lowerit(username);
	lowerit(domain);
	if (!stralloc_copys(&email, username) ||
			!stralloc_append(&email, "@") ||
			!stralloc_cats(&email, domain) ||
			!stralloc_0(&email))
		die_nomem();
#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email.s, 0))
#else
	if (iopen((char *) 0))
#endif
	{
		if (userNotFound)
			return (0);
		else
			return (-1);
	}
	if (!(pw = sql_getpw(username, domain)))
		return (0);
	if (pw->pw_gid & NO_PASSWD_CHNG) {
		strerr_warn1("User not allowed to change passwd", 0);
		return (-1);
	}
	if (apop & USE_APOP) {
		i = sql_passwd (username, domain, password, apop);
		iclose();
		return (i);
	} else {
		mkpasswd(password, &Crypted, encrypt_flag);
		if ((i = sql_passwd(username, domain, Crypted.s, apop)) == 1) {
			if (!stralloc_copys(&Dir, pw->pw_dir) ||
					!stralloc_catb(&Dir, "/Maildir", 8) ||
					!stralloc_0(&Dir))
				die_nomem();
			if (access(Dir.s, F_OK))
				quota = 0l;
			else {
#ifdef USE_MAILDIRQUOTA
				quota = check_quota(Dir.s, 0);
#else
				quota = check_quota(Dir.s);
#endif
			}
#ifdef ENABLE_AUTH_LOGGING
			if (!(ptr = GetPeerIPaddr())) {
				strerr_warn1("ipasswd: GetPeerIPaddr: ", &strerr_sys);
				return (-1);
			}
			vset_lastauth(username, domain, "pass", ptr, pw->pw_gecos, quota);
#endif
		}
		iclose();
		return (i);
	}
}
