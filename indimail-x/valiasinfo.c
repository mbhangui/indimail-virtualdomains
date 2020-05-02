/*
 * $Log: valiasinfo.c,v $
 * Revision 1.3  2020-04-01 18:58:29+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 14:23:56+05:30  Cprogrammer
 * fixed directory length
 * added missing new line
 *
 * Revision 1.1  2019-04-15 12:04:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <error.h>
#include <open.h>
#include <getln.h>
#include <substdio.h>
#include <getEnvConfig.h>
#endif
#include "get_assign.h"
#include "sql_getpw.h"
#include "valias_select.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: valiasinfo.c,v 1.3 2020-04-01 18:58:29+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("valiasinfo: out of memory", 0);
	_exit(111);
}

int
valiasinfo(char *user, char *domain)
{
	int             flag1, fd, match;
	static stralloc tmpbuf = {0}, Dir = {0}, line = {0};
	struct passwd  *pw;
	char           *qmaildir, *tmpalias, *ptr;
	struct substdio ssin;
	char            inbuf[4096];
#ifdef VALIAS
	int             flag2;
#endif
	uid_t           uid;
	gid_t           gid;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	if (!domain || !*domain) {
		if (!stralloc_copys(&Dir, qmaildir) ||
				!stralloc_catb(&Dir, "/alias", 6) ||
				!stralloc_0(&Dir))
			die_nomem();
		Dir.len--;
	} else
	if (!get_assign(domain, &Dir, &uid, &gid))
		Dir.len = 0;
	if (Dir.len) {
		if (!stralloc_copy(&tmpbuf, &Dir) ||
				!stralloc_catb(&tmpbuf, "/.qmail-", 8) ||
				!stralloc_cats(&tmpbuf, user) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		/* replace all dots with ':' */
		for (ptr = tmpbuf.s + Dir.len + 8; *ptr; ptr++) {
			if (*ptr == '.')
				*ptr = ':';
		}
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno != error_noent) {
				strerr_warn3("valiasinfo: ", tmpbuf.s, ": ", &strerr_sys);
				return (-1);
			} else
				flag1 = 0;
		}
		if (fd != -1) {
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			for (flag1 = 0;;) {
				if (getln(&ssin, &line, &match, '\n') == -1) {
					strerr_warn3("valiasinfo: read: ", tmpbuf.s, ": ", &strerr_sys);
					break;
				}
				if (!match && line.len == 0)
					break;
				line.len--;
				line.s[line.len] = 0;
				match = str_chr(line.s, '#');
				if (line.s[match])
					line.s[match] = 0;
				for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
				if (!*ptr)
					continue;
				out("valiasinfo", !flag1++ ? "Forwarding    : " : "              : ");
				out("valiasinfo", *ptr == '&' ? ptr + 1 : ptr);
				out("valiasinfo", "\n");
			}
			close(fd);
			if (!flag1++) {
				out("valiasinfo", "Forwarding    : ");
				out("valiasinfo", Dir.s);
				out("valiasinfo", "/Maildir/\n");
			}
			flush("valiasinfo");
		}
		if ((pw = sql_getpw(user, domain)) != (struct passwd *) 0) {
			if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
					!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			if ((fd = open_read(tmpbuf.s)) == -1) {
				if (errno != error_noent) {
					strerr_warn3("valiasinfo: ", tmpbuf.s, ": ", &strerr_sys);
					return (-1);
				}
			}
			if (fd != -1) {
				substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
				for (;;) {
					if (getln(&ssin, &line, &match, '\n') == -1) {
						strerr_warn3("valiasinfo: read: ", tmpbuf.s, ": ", &strerr_sys);
						break;
					}
					if (!match && line.len == 0)
						break;
					line.len--;
					line.s[line.len] = 0;
					match = str_chr(line.s, '#');
					if (line.s[match])
						line.s[match] = 0;
					for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
					if (!*ptr)
						continue;
					out("valiasinfo", !flag1++ ? "Forwarding    : " : "              : ");
					out("valiasinfo", *ptr == '&' ? ptr + 1 : ptr);
					out("valiasinfo", "\n");
				}
				flush("valiasinfo");
				close(fd);
			}
		}
	} else
		flag1 = 0;
#ifdef VALIAS
	for (flag2 = 0;;) {
		if (!(tmpalias = valias_select(user, domain)))
			break;
		if (*tmpalias == '&')
			tmpalias++;
		out("valiasinfo", !flag2++ ? "Forwarding    : " : "              : ");
		out("valiasinfo", tmpalias);
		out("valiasinfo", "\n");
	}
	flush("valiasinfo");
	return(flag1 + flag2);
#else
	return(flag1);
#endif
}