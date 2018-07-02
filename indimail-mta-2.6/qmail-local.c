/*
 * $Log: qmail-local.c,v $
 * Revision 1.28  2017-03-10 11:26:36+05:30  Cprogrammer
 * no of duplicate Delivered-To configurable in maxdeliveredto control file
 *
 * Revision 1.27  2015-08-24 19:07:43+05:30  Cprogrammer
 * moved fmt_xlong() to separate source fmt_xlong.c
 *
 * Revision 1.26  2014-01-22 20:38:24+05:30  Cprogrammer
 * added hassrs.h
 *
 * Revision 1.25  2013-08-27 08:19:48+05:30  Cprogrammer
 * use sticky bit definition from header file
 *
 * Revision 1.24  2013-08-25 18:38:00+05:30  Cprogrammer
 * added SRS
 *
 * Revision 1.23  2011-07-07 19:55:08+05:30  Cprogrammer
 * added branch logic
 *
 * Revision 1.22  2010-08-09 18:30:41+05:30  Cprogrammer
 * use different archival rules for forwarding/aliases to prevent duplication
 * when sending to groups
 *
 * Revision 1.21  2010-07-16 14:12:15+05:30  Cprogrammer
 * formatted code and comments
 *
 * Revision 1.20  2009-12-05 20:07:40+05:30  Cprogrammer
 * ansic conversion
 *
 * Revision 1.19  2008-11-15 17:08:17+05:30  Cprogrammer
 * added localdomains control file
 *
 * Revision 1.18  2005-04-05 12:02:29+05:30  Cprogrammer
 * fix buffer overflow and made maildir file creation collision proof
 *
 * Revision 1.17  2005-03-24 20:03:26+05:30  Cprogrammer
 * removed useless code for sync
 *
 * Revision 1.16  2005-03-24 09:04:12+05:30  Cprogrammer
 * added Hack to force directory sync after link
 *
 * Revision 1.15  2004-10-22 20:28:23+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.14  2004-10-22 15:36:54+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.13  2004-10-11 23:56:41+05:30  Cprogrammer
 * set QQEH for mail forwarding
 *
 * Revision 1.12  2004-08-14 02:25:04+05:30  Cprogrammer
 * set QQEH only when executing another program
 *
 * Revision 1.11  2004-07-27 22:58:40+05:30  Cprogrammer
 * usage change for qqeh
 *
 * Revision 1.10  2004-07-17 21:20:40+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sig.h"
#include "env.h"
#include "byte.h"
#include "exit.h"
#include "open.h"
#include "wait.h"
#include "lock.h"
#include "seek.h"
#include "substdio.h"
#include "getln.h"
#include "strerr.h"
#include "subfd.h"
#include "sgetopt.h"
#include "alloc.h"
#include "error.h"
#include "stralloc.h"
#include "fmt.h"
#include "str.h"
#include "now.h"
#include "case.h"
#include "quote.h"
#include "qmail.h"
#include "slurpclose.h"
#include "myctime.h"
#include "gfrom.h"
#include "constmap.h"
#include "auto_qmail.h"
#include "control.h"
#include "variables.h"
#include "hassrs.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#include "auto_patrn.h"

void
usage()
{
	strerr_die1x(100, "qmail-local: usage: qmail-local [ -nN ] user homedir local dash ext domain sender aliasempty qqeh");
}

void
temp_nomem()
{
	strerr_die1x(111, "Out of memory. (#4.3.0)");
}

void
temp_rewind()
{
	strerr_die1x(111, "Unable to rewind message. (#4.3.0)");
}

void
temp_childcrashed()
{
	strerr_die1x(111, "Aack, child crashed. (#4.3.0)");
}

void
temp_fork()
{
	strerr_die3x(111, "Unable to fork: ", error_str(errno), ". (#4.3.0)");
}

void
temp_read()
{
	strerr_die3x(111, "Unable to read message: ", error_str(errno), ". (#4.3.0)");
}

void
temp_slowlock()
{
	strerr_die1x(111, "File has been locked for 30 seconds straight. (#4.3.0)");
}

void
temp_qmail(fn)
	char           *fn;
{
	strerr_die5x(111, "Unable to open ", fn, ": ", error_str(errno), ". (#4.3.0)");
}

char           *overquota = "Recipient's mailbox is full, message returned to sender. (#5.2.2)";
char           *user, *homedir, *local, *dash, *ext, *host, *sender, *aliasempty, *qqeh;
char            buf[1024];
char            outbuf[1024];
int             flagdoit;
int             flag99;
stralloc        safeext = { 0 };
stralloc        ufline = { 0 };
stralloc        rpline = { 0 };
stralloc        envrecip = { 0 };
stralloc        dtline = { 0 };
stralloc        qme = { 0 };
stralloc        ueo = { 0 };
stralloc        cmds = { 0 };
stralloc        messline = { 0 };
stralloc        foo = { 0 };
stralloc        hostname = { 0 };

/*- child process */
stralloc        fntmptph = { 0 };
stralloc        fnnewtph = { 0 };

#ifdef HAVESRS
void die_control()
{
	strerr_die1x(111, "Unable to read controls (#4.3.0)");
}

void die_srs()
{
	if (!stralloc_copys(&foo, srs_error.s))
		temp_nomem();
	if (!stralloc_cats(&foo, " (#4.3.0)"))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	strerr_die1x(111,foo.s);
}
#endif

void
tryunlinktmp()
{
	unlink(fntmptph.s);
}

void
sigalrm()
{
	tryunlinktmp();
	_exit(3);
}

/*
 * Some operating systems quickly recycle PIDs, which can lead 
 * to collisions between Maildir-style filenames, which must 
 * be unique and non-repeatable within one second.
 * 
 * This uses the format of the revised Maildir protocol,
 * available at:
 * 
 * http://cr.yp.to/proto/maildir.html
 * 
 * It uses four unique identifiers:
 * - inode number of the file written to Maildir/tmp
 * - device number of the file written to Maildir/tmp
 * - time in microseconds
 * - the PID of the writing process
 * 
 * A Maildir-style filename would look like the following:
 * 
 * In Maildir/tmp:
 * time.MmicrosecondsPpid.host
 * In Maildir/new:
 * time.IinodeVdeviceMmicrosecondsPpid.host
 * 
 * Additionally, this patch further comforms to the revised 
 * Maildir protocol by looking through the hostname for 
 * instances of '/' and ':', replacing them with "057" and 
 * "072", respectively, when writing it to disk.
 */

void
maildir_child(char *dir)
{
	char            strnum[FMT_ULONG], host[64];
	char           *s;
	struct timeval  tmval;
	struct stat     st;
	unsigned long   pid;
	int             loop, fd, i;
	substdio        ss, ssout;

	sig_alarmcatch(sigalrm);
	if (chdir(dir) == -1)
	{
		if (error_temp(errno))
			_exit(1);
		_exit(2);
	}
	pid = getpid();
	host[0] = 0;
	gethostname(host, sizeof(host));
	s = host;
	for (loop = 0; loop < str_len(host); ++loop)
	{
		if (host[loop] == '/')
		{
			if (!stralloc_cats(&hostname, "057"))
				temp_nomem();
			continue;
		}
		if (host[loop] == ':')
		{
			if (!stralloc_cats(&hostname, "072"))
				temp_nomem();
			continue;
		}
		if (!stralloc_append(&hostname, s + loop))
			temp_nomem();
	}
	for (loop = 0;; ++loop)
	{
		gettimeofday(&tmval, 0);
		if (!stralloc_copys(&fntmptph, "tmp/"))
			temp_nomem();
		strnum[fmt_ulong(strnum, tmval.tv_sec)] = 0;
		if (!stralloc_cats(&fntmptph, strnum))
			temp_nomem();
		if (!stralloc_append(&fntmptph, "."))
			temp_nomem();
		if (!stralloc_append(&fntmptph, "M"))
			temp_nomem();
		strnum[fmt_ulong(strnum, tmval.tv_usec)] = 0;
		if (!stralloc_cats(&fntmptph, strnum))
			temp_nomem();
		if (!stralloc_append(&fntmptph, "P"))
			temp_nomem();
		strnum[fmt_ulong(strnum, pid)] = 0;
		if (!stralloc_cats(&fntmptph, strnum))
			temp_nomem();
		if (!stralloc_append(&fntmptph, "."))
			temp_nomem();
		if (!stralloc_cat(&fntmptph, &hostname))
			temp_nomem();
		if (!stralloc_0(&fntmptph))
			temp_nomem();
		if (stat(fntmptph.s, &st) == -1 && errno == error_noent)
			break;
		/*
		 * really should never get to this point 
		 */
		if (loop == 2)
			_exit(1);
		sleep(2);
	}
	alarm(86400);
	if ((fd = open_excl(fntmptph.s)) == -1)
	{
		if (errno == error_dquot)
			_exit(5);
		else
			_exit(1);
	}
	substdio_fdbuf(&ss, read, 0, buf, sizeof(buf));
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_put(&ssout, rpline.s, rpline.len) == -1)
		goto fail;
	if (substdio_put(&ssout, dtline.s, dtline.len) == -1)
		goto fail;
	if (substdio_puts(&ssout, qqeh) == -1)
		goto fail;
	switch (substdio_copy(&ssout, &ss))
	{
	case -2:
		tryunlinktmp();
		_exit(4);
	case -3:
		goto fail;
	}
	if (substdio_flush(&ssout) == -1)
		goto fail;
	if (fstat(fd, &st) == -1)
		goto fail;
#ifdef USE_FSYNC
	if (use_fsync && fsync(fd) == -1)
		goto fail;
#endif
	if (close(fd) == -1)
		goto fail;	/*- NFS dorks */
	if (!stralloc_copys(&fnnewtph, "new/"))
		temp_nomem();
	strnum[fmt_ulong(strnum, tmval.tv_sec)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum))
		temp_nomem();
	if (!stralloc_append(&fnnewtph, "."))
		temp_nomem();
	/*- in hexadecimal */
	if (!stralloc_append(&fnnewtph, "I"))
		temp_nomem();
	s = alloc(fmt_xlong(0, st.st_ino) + fmt_xlong(0, st.st_dev));
	i = fmt_xlong(s, st.st_ino);
	if (!stralloc_catb(&fnnewtph, s, i))
		temp_nomem();
	if (!stralloc_append(&fnnewtph, "V"))
		temp_nomem();
	i = fmt_xlong(s, st.st_dev);
	if (!stralloc_catb(&fnnewtph, s, i))
		temp_nomem();
	alloc_free(s);
	/*- in decimal */
	if (!stralloc_append(&fnnewtph, "M"))
		temp_nomem();
	strnum[fmt_ulong(strnum, tmval.tv_usec)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum))
		temp_nomem();
	if (!stralloc_append(&fnnewtph, "P"))
		temp_nomem();
	strnum[fmt_ulong(strnum, pid)] = 0;
	if (!stralloc_cats(&fnnewtph, strnum))
		temp_nomem();
	if (!stralloc_append(&fnnewtph, "."))
		temp_nomem();
	if (!stralloc_cat(&fnnewtph, &hostname))
		temp_nomem();
	if (!stralloc_0(&fnnewtph))
		temp_nomem();
	if (link(fntmptph.s, fnnewtph.s) == -1)
		goto fail;
#ifdef USE_FSYNC
	if (!env_get("USE_SYNCDIR"))
	{
		if (use_fsync)
		{
			if ((fd = open(fnnewtph.s, O_RDONLY)) < 0 || fsync(fd) < 0 || close(fd) < 0)
				goto fail;
		}
	}
#endif
	/*
	 * if it was error_exist, almost certainly successful; i hate NFS 
	 */
	tryunlinktmp();
	_exit(0);
fail:
	if (errno == error_dquot)
	{
		tryunlinktmp();
		_exit(5);
	} else
	{
		tryunlinktmp();
		_exit(1);
	}
}

/*- end child process */

void
maildir(char *fn)
{
	int             child;
	int             wstat;

	if (seek_begin(0) == -1)
		temp_rewind();

	switch (child = fork())
	{
	case -1:
		temp_fork();
	case 0:
		maildir_child(fn);
		_exit(111);
	}
	wait_pid(&wstat, child);
	if (wait_crashed(wstat))
		temp_childcrashed();
	switch (wait_exitcode(wstat))
	{
	case 0:
		break;
	case 2:
		strerr_die1x(111, "Unable to chdir to maildir. (#4.2.1)");
	case 3:
		strerr_die1x(111, "Timeout on maildir delivery. (#4.3.0)");
	case 4:
		strerr_die1x(111, "Unable to read message. (#4.3.0)");
	case 5:
		strerr_die1x(100, overquota);
	default:
		strerr_die1x(111, "Temporary error on maildir delivery. (#4.3.0)");
	}
}

void
mailfile(char *fn)
{
	int             fd;
	substdio        ss;
	substdio        ssout;
	int             match;
	seek_pos        pos;
	int             flaglocked;

	if (seek_begin(0) == -1)
		temp_rewind();
	fd = open_append(fn);
	if (fd == -1)
		strerr_die5x(111, "Unable to open ", fn, ": ", error_str(errno), ". (#4.2.1)");
	sig_alarmcatch(temp_slowlock);
	alarm(30);
	flaglocked = (lock_ex(fd) != -1);
	alarm(0);
	sig_alarmdefault();
	seek_end(fd);
	pos = seek_cur(fd);
	substdio_fdbuf(&ss, read, 0, buf, sizeof(buf));
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_put(&ssout, ufline.s, ufline.len))
		goto writeerrs;
	if (substdio_put(&ssout, rpline.s, rpline.len))
		goto writeerrs;
	if (substdio_put(&ssout, dtline.s, dtline.len))
		goto writeerrs;
	if (substdio_puts(&ssout, qqeh) == -1)
		goto writeerrs;
	for (;;)
	{
		if (getln(&ss, &messline, &match, '\n') != 0)
		{
			strerr_warn3("Unable to read message: ", error_str(errno), ". (#4.3.0)", 0);
			if (flaglocked)
				seek_trunc(fd, pos);
			close(fd);
			_exit(111);
		}
		if (!match && !messline.len)
			break;
		if (gfrom(messline.s, messline.len))
			if (substdio_bput(&ssout, ">", 1))
				goto writeerrs;
		if (substdio_bput(&ssout, messline.s, messline.len))
			goto writeerrs;
		if (!match)
		{
			if (substdio_bputs(&ssout, "\n"))
				goto writeerrs;
			break;
		}
	}
	if (substdio_bputs(&ssout, "\n"))
		goto writeerrs;
	if (substdio_flush(&ssout))
		goto writeerrs;
#ifdef USE_FSYNC
	if (use_fsync && fsync(fd) == -1)
		goto writeerrs;
#endif
	close(fd);
	return;

writeerrs:
	if (errno == error_dquot)
	{
		if (flaglocked)
			seek_trunc(fd, pos);
		close(fd);
		strerr_die1x(100, overquota);
	} else
		strerr_warn5("Unable to write ", fn, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (flaglocked)
		seek_trunc(fd, pos);
	close(fd);
	_exit(111);
}

void
mailprogram(char *prog)
{
	int             child;
	char           *(args[4]);
	int             wstat;

	if (seek_begin(0) == -1)
		temp_rewind();

	switch (child = fork())
	{
	case -1:
		temp_fork();
	case 0:
		if (!env_put2("QQEH", qqeh))
			temp_nomem();
		args[0] = "/bin/sh";
		args[1] = "-c";
		args[2] = prog;
		args[3] = 0;
		sig_pipedefault();
		execv(*args, args);
		strerr_die3x(111, "Unable to run /bin/sh: ", error_str(errno), ". (#4.3.0)");
	}

	wait_pid(&wstat, child);
	if (wait_crashed(wstat))
		temp_childcrashed();
	switch (wait_exitcode(wstat))
	{
	case 100:
	case 64:
	case 65:
	case 70:
	case 76:
	case 77:
	case 78:
	case 112:
		_exit(100);
	case 0:
		break;
	case 99:
		flag99 = 1;
		break;
	default:
		_exit(111);
	}
}

unsigned long   mailforward_qp = 0;

void
mailforward(char **recips)
{
	struct qmail    qqt;
	char           *qqx;
	substdio        ss;
	int             match;

	if (seek_begin(0) == -1)
		temp_rewind();
	if (!env_put2("QQEH", qqeh))
		temp_nomem();
	substdio_fdbuf(&ss, read, 0, buf, sizeof(buf));
	if (qmail_open(&qqt) == -1)
		temp_fork();
	mailforward_qp = qmail_qp(&qqt);
	qmail_put(&qqt, dtline.s, dtline.len);
	qmail_puts(&qqt, qqeh);
	do
	{
		if (getln(&ss, &messline, &match, '\n') != 0)
		{
			qmail_fail(&qqt);
			break;
		}
		qmail_put(&qqt, messline.s, messline.len);
	}
	while (match);
#ifdef HAVESRS 
	switch(srsforward(ueo.s))
	{
	case -3:
		die_srs();
		break;
	case -2:
		temp_nomem();
		break;
	case -1:
		die_control();
		break;
	case 0:
		break;
	case 1:
		if (!stralloc_copy(&ueo, &srs_result))
			temp_nomem();
		break;
	} 
#endif 
	qmail_from(&qqt, ueo.s);
	while (*recips)
		qmail_to(&qqt, *recips++);
	qqx = qmail_close(&qqt);
	if (!*qqx)
		return;
	strerr_die3x(*qqx == 'D' ? 100 : 111, "Unable to forward message: ", qqx + 1, ".");
}

void
bouncexf()
{
	int             match, matches = 0, maxdeliveredto;
	substdio        ss;

	if (seek_begin(0) == -1)
		temp_rewind();
	if (control_readint(&maxdeliveredto, "maxdeliveredto") == -1)
		strerr_die1x(111, "Unable to read control file maxdeliveredto. (#4.3.0)");
	substdio_fdbuf(&ss, read, 0, buf, sizeof(buf));
	for (;;)
	{
		if (getln(&ss, &messline, &match, '\n') != 0)
			temp_read();
		if (!match)
			break;
		if (messline.len <= 1)
			break;
		if (messline.len == dtline.len && !str_diffn(messline.s, dtline.s, dtline.len))
			matches++;
		if (maxdeliveredto > -1 && matches > maxdeliveredto)
			strerr_die1x(100, "This message is looping: it already has my Delivered-To line. (#5.4.6)");
	}
}

void
checkhome(char *homedir)
{
	struct stat     st;
	int             ldmok;
	stralloc        ldm = { 0 };
	struct constmap mapldm;

	if (chdir(homedir) == -1)
		strerr_die5x(111, "Unable to switch to ", homedir, ": ", error_str(errno), ". (#4.3.0)");
	if (stat(".", &st) == -1)
		strerr_die3x(111, "Unable to stat home directory: ", error_str(errno), ". (#4.3.0)");
	if (st.st_mode & auto_patrn)
		strerr_die1x(111, "Uh-oh: home directory is writable. (#4.7.0)");
	if (st.st_mode & S_ISVTX)
	{
		if (flagdoit)
			strerr_die1x(111, "Home directory is sticky: user is editing his .qmail file. (#4.2.1)");
		else
			strerr_warn1("Warning: home directory is sticky.", 0);
	}
	if (!env_get("LOCALDOMAINS") && stat(".localdomains", &st))
		ldmok = 0;
	else
	{
		if (chdir(auto_qmail) == -1)
			strerr_die5x(111, "Unable to switch to ", auto_qmail, ": ", error_str(errno), ". (#4.3.0)");
		if ((ldmok = control_readfile(&ldm, "localdomains", 0)) == -1)
			strerr_die1x(111, "Unable to read control file [localdomains]. (#4.3.0)");
		if (chdir(homedir) == -1)
			strerr_die5x(111, "Unable to switch to ", homedir, ": ", error_str(errno), ". (#4.3.0)");
	}
	if (ldmok)
	{
		stralloc        sas = {0};
		int             j, len;

		if (!constmap_init(&mapldm, ldm.s, ldm.len, 0))
			temp_nomem();
		len = str_len(sender);
		if ((j = byte_rchr(sender, len, '@')) < len - 1)
		{
			if (!stralloc_copys(&sas, sender + j + 1))
				temp_nomem();
			if (!constmap(&mapldm, sas.s, sas.len))
				strerr_die1x(100, "This email address doesn't accept mails from remote domains. (#4.2.1)");
		}
	}
}

int
qmeox(char *dashowner)
{
	struct stat     st;

	if (!stralloc_copys(&qme, ".qmail"))
		temp_nomem();
	if (!stralloc_cats(&qme, dash))
		temp_nomem();
	if (!stralloc_cat(&qme, &safeext))
		temp_nomem();
	if (!stralloc_cats(&qme, dashowner))
		temp_nomem();
	if (!stralloc_0(&qme))
		temp_nomem();
	if (stat(qme.s, &st) == -1)
	{
		if (error_temp(errno))
			temp_qmail(qme.s);
		return -1;
	}
	return 0;
}

int
qmeexists(int *fd, int *cutable)
{
	struct stat     st;

	if (!stralloc_0(&qme))
		temp_nomem();

	*fd = open_read(qme.s);
	if (*fd == -1)
	{
		if (error_temp(errno))
			temp_qmail(qme.s);
		if (errno == error_perm)
			temp_qmail(qme.s);
		if (errno == error_acces)
			temp_qmail(qme.s);
		return 0;
	}

	if (fstat(*fd, &st) == -1)
		temp_qmail(qme.s);
	if ((st.st_mode & S_IFMT) == S_IFREG)
	{
		if (st.st_mode & auto_patrn)
			strerr_die1x(111, "Uh-oh: .qmail file is writable. (#4.7.0)");
		*cutable = !!(st.st_mode & 0100);
		return 1;
	}
	close(*fd);
	return 0;
}


/* "" "": "" */
/* "-/" "": "-/" "-/default" */
/* "-/" "a": "-/a" "-/default" */
/* "-/" "a-": "-/a-" "-/a-default" "-/default" */
/* "-/" "a-b": "-/a-b" "-/a-default" "-/default" */
/* "-/" "a-b-": "-/a-b-" "-/a-b-default" "-/a-default" "-/default" */
/* "-/" "a-b-c": "-/a-b-c" "-/a-b-default" "-/a-default" "-/default" */

void
qmesearch(int *fd, int *cutable)
{
	int             i;

	if (!stralloc_copys(&qme, ".qmail"))
		temp_nomem();
	if (!stralloc_cats(&qme, dash))
		temp_nomem();
	if (!stralloc_cat(&qme, &safeext))
		temp_nomem();
	if (qmeexists(fd, cutable))
	{
		if (safeext.len >= 7)
		{
			i = safeext.len - 7;
			if (!byte_diff("default", 7, safeext.s + i))
			{
				if (i <= str_len(ext))	/*- paranoia */
				{
					if (!env_put2("DEFAULT", ext + i))
						temp_nomem();
				}
			}
		}
		return;
	}
	for (i = safeext.len; i >= 0; --i)
	{
		if (!i || (safeext.s[i - 1] == '-'))
		{
			if (!stralloc_copys(&qme, ".qmail"))
				temp_nomem();
			if (!stralloc_cats(&qme, dash))
				temp_nomem();
			if (!stralloc_catb(&qme, safeext.s, i))
				temp_nomem();
			if (!stralloc_cats(&qme, "default"))
				temp_nomem();
			if (qmeexists(fd, cutable))
			{
				if (i <= str_len(ext))	/*- paranoia */
					if (!env_put2("DEFAULT", ext + i))
						temp_nomem();
				return;
			}
		}
	}
	*fd = -1;
}

unsigned long   count_file = 0;
unsigned long   count_forward = 0;
unsigned long   count_program = 0;
char            count_buf[FMT_ULONG];

void
count_print()
{
	substdio_puts(subfdoutsmall, "did ");
	substdio_put(subfdoutsmall, count_buf, fmt_ulong(count_buf, count_file));
	substdio_puts(subfdoutsmall, "+");
	substdio_put(subfdoutsmall, count_buf, fmt_ulong(count_buf, count_forward));
	substdio_puts(subfdoutsmall, "+");
	substdio_put(subfdoutsmall, count_buf, fmt_ulong(count_buf, count_program));
	substdio_puts(subfdoutsmall, "\n");
	if (mailforward_qp)
	{
		substdio_puts(subfdoutsmall, "qp ");
		substdio_put(subfdoutsmall, count_buf, fmt_ulong(count_buf, mailforward_qp));
		substdio_puts(subfdoutsmall, "\n");
	}
	substdio_flush(subfdoutsmall);
}

void
sayit(char *type, char *cmd, int len)
{
	substdio_puts(subfdoutsmall, type);
	substdio_put(subfdoutsmall, cmd, len);
	substdio_putsflush(subfdoutsmall, "\n");
}

int
main(int argc, char **argv)
{
	char           *x;
	char          **recips;
	int             opt, i, j, k, fd, numforward, flagforwardonly;
	datetime_sec    starttime;

	umask(077);
	sig_pipeignore();
	if (!env_init())
		temp_nomem();
	flagdoit = 1;
	while ((opt = getopt(argc, argv, "nN")) != opteof)
	{
		switch (opt)
		{
		case 'n':
			flagdoit = 0;
			break;
		case 'N':
			flagdoit = 1;
			break;
		default:
			usage();
		}
	} /*- while ((opt = getopt(argc, argv, "nN")) != opteof) */
	argc -= optind;
	argv += optind;
	if (!(user = *argv++))
		usage();
	if (!(homedir = *argv++))
		usage();
	if (!(local = *argv++))
		usage();
	if (!(dash = *argv++))
		usage();
	if (!(ext = *argv++))
		usage();
	if (!(host = *argv++))
		usage();
	if (!(sender = *argv++))
		usage();
	if (!(aliasempty = *argv++))
		usage();
	if (!(qqeh = *argv++))
		usage();
	if (*argv)
		usage();
	if (homedir[0] != '/')
		usage();
	checkhome(homedir);
	if (!env_put2("HOST", host))
		temp_nomem();
	if (!env_put2("HOME", homedir))
		temp_nomem();
	if (!env_put2("USER", user))
		temp_nomem();
	if (!env_put2("LOCAL", local))
		temp_nomem();
	if (!env_unset("SPAMFILTER"))
		temp_nomem();
#if defined(MAILARCHIVE)
	if (!env_put2("MAILARCHIVE", "forward.arch"))
		temp_nomem();
#endif
	if (!stralloc_copys(&envrecip, local))
		temp_nomem();
	if (!stralloc_cats(&envrecip, "@"))
		temp_nomem();
	if (!stralloc_cats(&envrecip, host))
		temp_nomem();
	if (!stralloc_copy(&foo, &envrecip))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	if (!env_put2("RECIPIENT", foo.s))
		temp_nomem();
	if (!stralloc_copys(&dtline, "Delivered-To: "))
		temp_nomem();
	if (!stralloc_cat(&dtline, &envrecip))
		temp_nomem();
	for (i = 0; i < dtline.len; ++i)
		if (dtline.s[i] == '\n')
			dtline.s[i] = '_';
	if (!stralloc_cats(&dtline, "\n"))
		temp_nomem();
	if (!stralloc_copy(&foo, &dtline))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	if (!env_put2("DTLINE", foo.s))
		temp_nomem();
	if (flagdoit)
		bouncexf();
	if (!env_put2("SENDER", sender))
		temp_nomem();
	if (!quote2(&foo, sender))
		temp_nomem();
	if (!stralloc_copys(&rpline, "Return-Path: <"))
		temp_nomem();
	if (!stralloc_cat(&rpline, &foo))
		temp_nomem();
	for (i = 0; i < rpline.len; ++i)
		if (rpline.s[i] == '\n')
			rpline.s[i] = '_';
	if (!stralloc_cats(&rpline, ">\n"))
		temp_nomem();
	if (!stralloc_copy(&foo, &rpline))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	if (!env_put2("RPLINE", foo.s))
		temp_nomem();
	if (!stralloc_copys(&ufline, "From "))
		temp_nomem();
	if (*sender)
	{
		int             len, i;
		char            ch;

		len = str_len(sender);
		if (!stralloc_readyplus(&ufline, len))
			temp_nomem();
		for (i = 0; i < len; ++i)
		{
			ch = sender[i];
			if ((ch == ' ') || (ch == '\t') || (ch == '\n'))
				ch = '-';
			ufline.s[ufline.len + i] = ch;
		}
		ufline.len += len;
	} else
	if (!stralloc_cats(&ufline, "MAILER-DAEMON"))
		temp_nomem();
	if (!stralloc_cats(&ufline, " "))
		temp_nomem();
	starttime = now();
	if (!stralloc_cats(&ufline, myctime(starttime)))
		temp_nomem();

	if (!stralloc_copy(&foo, &ufline))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	if (!env_put2("UFLINE", foo.s))
		temp_nomem();
	x = ext;
	if (!env_put2("EXT", x))
		temp_nomem();
	x += str_chr(x, '-');
	if (*x)
		++x;
	if (!env_put2("EXT2", x))
		temp_nomem();
	x += str_chr(x, '-');
	if (*x)
		++x;
	if (!env_put2("EXT3", x))
		temp_nomem();
	x += str_chr(x, '-');
	if (*x)
		++x;
	if (!env_put2("EXT4", x))
		temp_nomem();
	if (!stralloc_copys(&safeext, ext))
		temp_nomem();
	case_lowerb(safeext.s, safeext.len);
	for (i = 0; i < safeext.len; ++i)
	{
		if (safeext.s[i] == '.')
			safeext.s[i] = ':';
	}
	i = str_len(host);
	i = byte_rchr(host, i, '.');
	if (!stralloc_copyb(&foo, host, i))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	if (!env_put2("HOST2", foo.s))
		temp_nomem();
	i = byte_rchr(host, i, '.');
	if (!stralloc_copyb(&foo, host, i))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	if (!env_put2("HOST3", foo.s))
		temp_nomem();
	i = byte_rchr(host, i, '.');
	if (!stralloc_copyb(&foo, host, i))
		temp_nomem();
	if (!stralloc_0(&foo))
		temp_nomem();
	if (!env_put2("HOST4", foo.s))
		temp_nomem();
	flagforwardonly = 0;
	qmesearch(&fd, &flagforwardonly);
	if (fd == -1 && *dash)
		strerr_die1x(100, "Sorry, no mailbox here by that name. (#5.1.1)");
	if (!stralloc_copys(&ueo, sender))
		temp_nomem();
	if (str_diff(sender, ""))
	{
		if (str_diff(sender, "#@[]"))
		{
			if (qmeox("-owner") == 0)
			{
				if (qmeox("-owner-default") == 0)
				{
					if (!stralloc_copys(&ueo, local))
						temp_nomem();
					if (!stralloc_cats(&ueo, "-owner-@"))
						temp_nomem();
					if (!stralloc_cats(&ueo, host))
						temp_nomem();
					if (!stralloc_cats(&ueo, "-@[]"))
						temp_nomem();
				} else
				{
					if (!stralloc_copys(&ueo, local))
						temp_nomem();
					if (!stralloc_cats(&ueo, "-owner@"))
						temp_nomem();
					if (!stralloc_cats(&ueo, host))
						temp_nomem();
				}
			}
		}
	}
	if (!stralloc_0(&ueo))
		temp_nomem();
	if (!env_put2("NEWSENDER", ueo.s))
		temp_nomem();
	if (!stralloc_ready(&cmds, 0))
		temp_nomem();
	cmds.len = 0;
	if (fd != -1)
		if (slurpclose(fd, &cmds, 256) == -1)
			temp_nomem();
	if (!cmds.len)
	{
		if (!stralloc_copys(&cmds, aliasempty))
			temp_nomem();
		flagforwardonly = 0;
	}
	if (!cmds.len || (cmds.s[cmds.len - 1] != '\n'))
	{
		if (!stralloc_cats(&cmds, "\n"))
			temp_nomem();
	}
	numforward = 0;
	i = 0;
	for (j = 0; j < cmds.len; ++j)
	{
		if (cmds.s[j] == '\n')
		{
			switch (cmds.s[i])
			{
			case '#':
			case '.':
			case '/':
			case '|':
				break;
			default:
				++numforward;
			}
			i = j + 1;
		}
	}
	recips = (char **) alloc((numforward + 1) * sizeof(char *));
	if (!recips)
		temp_nomem();
	numforward = 0;
	flag99 = 0;
	i = 0;
#ifdef USE_FSYNC
	if (env_get("USE_FSYNC"))
		use_fsync = 1;
#endif
	for (j = 0; j < cmds.len; ++j)
	{
		if (cmds.s[j] == '\n')
		{
			cmds.s[j] = 0;
			k = j;
			while ((k > i) && ((cmds.s[k - 1] == ' ') || (cmds.s[k - 1] == '\t')))
				cmds.s[--k] = 0;
			switch (cmds.s[i])
			{
			case 0:	/*- k == i */
				if (i)
					break;
				strerr_die1x(111, "Uh-oh: first line of .qmail file is blank. (#4.2.1)");
			case '#':
			case ':':
				break;
			case '.':
			case '/':
				++count_file;
				if (flagforwardonly)
					strerr_die1x(111, "Uh-oh: .qmail has file delivery but has x bit set. (#4.7.0)");
				if (cmds.s[k - 1] == '/')
				{
					if (flagdoit)
						maildir(cmds.s + i);
					else
						sayit("maildir ", cmds.s + i, k - i);
				} else
				if (flagdoit)
					mailfile(cmds.s + i);
				else
					sayit("mbox ", cmds.s + i, k - i);
				break;
			case '|':
				++count_program;
				if (flagforwardonly)
					strerr_die1x(111, "Uh-oh: .qmail has prog delivery but has x bit set. (#4.7.0)");
				if (flagdoit)
					mailprogram(cmds.s + i + 1);
				else
					sayit("program ", cmds.s + i + 1, k - i - 1);
				break;
			case '?': /* branch */
				++i;
				{
					int             l;
					for (l = i; l < k; ++l)
						if (cmds.s[l] == ' ' || cmds.s[l] == '\t') {
							cmds.s[l] = 0;
							for (++l; l < k; ++l)
								if (cmds.s[l] != ' ' && cmds.s[l] != '\t') {
									++count_program;
									if (flagforwardonly)
										strerr_die1x(111, "Uh-oh: .qmail has prog delivery but has x bit set. (#4.7.0)");
									if (flagdoit)
										mailprogram(cmds.s + l);
									else
										sayit("program ", cmds.s + l, k - l);
									break;
								}
							break;
						}
					if (l == k || flag99) {
						flag99 = 0;
						cmds.s[j] = '\n';
						for (; j + 1 < cmds.len; ++j)
							if (cmds.s[j] == '\n' && cmds.s[j + 1] == ':') {
								j += 2;
								l = j;
								for (; j < cmds.len; ++j) {
									if (cmds.s[j] == 0)
										break;
									if (cmds.s[j] == '\t')
										break;
									if (cmds.s[j] == '\n')
										break;
									if (cmds.s[j] == ' ')
										break;
								}
								if (!str_diffn(cmds.s + i, cmds.s + l, j - l)) {
									for (; j < cmds.len; ++j)
										if (cmds.s[j] == '\n')
											break;
									break;
								}
								--j;
							}
					}
				}
				break;
			case '+':
				if (str_equal(cmds.s + i + 1, "list"))
					flagforwardonly = 1;
				break;
			case '&':
				++i;
			default:
				++count_forward;
				if (flagdoit)
					recips[numforward++] = cmds.s + i;
				else
					sayit("forward ", cmds.s + i, k - i);
				break;
			}
			i = j + 1;
			if (flag99)
				break;
		}
	}
	if (numforward)
	{
		if (flagdoit)
		{
			recips[numforward] = 0;
			mailforward(recips);
		}
	}
	count_print();
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_local_c()
{
	static char    *x = "$Id: qmail-local.c,v 1.28 2017-03-10 11:26:36+05:30 Cprogrammer Stab mbhangui $";

	x++;
}