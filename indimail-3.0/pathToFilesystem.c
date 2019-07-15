/*
 * $Log: pathToFilesystem.c,v $
 * Revision 1.1  2019-04-18 08:31:50+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef linux
#include <mntent.h>
#elif defined(sun)
#include <sys/types.h>
#include <sys/mnttab.h>
#elif defined(DARWIN)
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#endif
#include "Dirname.h"
#include "getactualpath.h"

#ifndef	lint
static char     sccsid[] = "$Id: pathToFilesystem.c,v 1.1 2019-04-18 08:31:50+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("pathToFilesystem: out of memory", 0);
	_exit(111);
}

char           *
pathToFilesystem(char *path)
{
	char           *ptr;
	int             pathlen, len;
	static stralloc tmpbuf = {0}, _path = {0};
#ifdef linux
	FILE           *fp;
	struct mntent  *mntptr;
#elif defined(sun)
#define mnt_dir    mnt_mountp
	FILE           *fp;
	struct mnttab   _MntTab;
	struct mnttab  *mntptr = &_MntTab;
#elif defined(DARWIN)
	int             num;
	struct statfs  *mntinf;
#endif

	if (!path || !*path)
		return ((char *) 0);
	if (_path.len && !str_diffn(path, _path.s, _path.len + 1))
		return (tmpbuf.s);
	/* 
	 * if directory does not exists, find parent
	 * directory recursively
	 */
	for (ptr = path; access(ptr, F_OK);) {
		if (!(ptr = Dirname(ptr)))
			break;
		if (!access(ptr, F_OK))
			break;
	}
	/*- Resolve links and Find the actual path */
	if (!(ptr = getactualpath(ptr)))
		return ((char *) 0);
#ifdef DARWIN
	if (!(num = getmntinfo(&mntinf, MNT_WAIT)))
		return ((char *) 0);
	for (*tmpbuf = 0, pathlen = 0;num--;) {
		if (str_str(ptr, mntinf->f_mntonname)) {
			if ((len = strlen(mntinf->f_mntonname)) > pathlen) {
				if (!stralloc_copys(&tmpbuf, mntinf->f_mntonname))
					die_nomem();
				pathlen = len;
			}
		}
		mntinf++;
	}
#else
	fp = (FILE *) 0;
#ifdef linux
	if (!access("/etc/mtab", F_OK))
		fp = setmntent("/etc/mtab", "r");
#elif defined(sun)
	if (!access("/etc/mnttab", F_OK))
		fp = fopen("/etc/mnttab", "r");
#endif
	if (!fp)
		return ((char *) 0);
	for (tmpbuf.len = 0, pathlen = 0;;) {
#ifdef linux
		if (!(mntptr = getmntent(fp)))
#elif defined(sun)
		if (getmntent(fp, mntptr))
#endif
			break;
		if (str_str(ptr, mntptr->mnt_dir)) {
			if ((len = str_len(mntptr->mnt_dir)) > pathlen) {
				if (!stralloc_copys(&tmpbuf, mntptr->mnt_dir) || !stralloc_0(&tmpbuf))
					die_nomem();
				tmpbuf.len--;
				pathlen = len;
			}
		}
	}
	fclose(fp);
#endif /*- #ifdef DARWIN */
	if (!stralloc_copys(&_path, path) || !stralloc_0(&_path))
		die_nomem();
	_path.len--;
	return (tmpbuf.s);
}