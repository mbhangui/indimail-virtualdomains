/*
 * $Log: logclient.c,v $
 * Revision 1.10  2021-04-05 21:57:58+05:30  Cprogrammer
 * fixed compilation errors
 *
 * Revision 1.9  2021-03-15 11:32:07+05:30  Cprogrammer
 * removed common.h
 *
 * Revision 1.8  2020-06-21 12:49:01+05:30  Cprogrammer
 * quench rpmlint
 *
 * Revision 1.7  2013-05-15 00:38:47+05:30  Cprogrammer
 * added SSL encryption
 *
 * Revision 1.6  2013-02-21 22:45:39+05:30  Cprogrammer
 * fold long line for readability
 *
 * Revision 1.5  2012-09-19 11:06:49+05:30  Cprogrammer
 * use environment variable SEEKDIR for the checkpoint files
 *
 * Revision 1.4  2012-09-18 17:39:43+05:30  Cprogrammer
 * fixed usage
 *
 * Revision 1.3  2012-09-18 17:29:34+05:30  Cprogrammer
 * use sockfd instead of sockfp
 *
 * Revision 1.2  2012-09-18 17:08:34+05:30  Cprogrammer
 * removed extra white space
 *
 * Revision 1.1  2012-09-18 13:23:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <getopt.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <errno.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef SOLARIS
#include <sys/systeminfo.h>
#endif
#ifdef HAVE_INDIMAIL
#include <indimail/tcpopen.h>
#include <indimail/filewrt.h>
#endif
#ifdef HAVE_QMAIL
#include <qmail/getEnvConfig.h>
#endif
#include "tls.h"

#define MAXBUF 8192
#define SEEKDIR PREFIX"/tmp/"

#ifndef	lint
static char     sccsid[] = "$Id: logclient.c,v 1.10 2021-04-05 21:57:58+05:30 Cprogrammer Exp mbhangui $";
#endif


struct msgtable
{
	char            fname[MAXBUF];
	int             fd;
	FILE           *fp;
	int             seekfd;
	long            machcnt;
};

struct msgtable *msghd;
int             log_timeout;

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#ifdef __STDC__
int             main(int, char **);
int             consclnt(char *, char **, char *);
static int      IOplex(char *, int);
static int      checkfiles(char *, FILE *, long *, int);
#else
int             main();
int             consclnt();
static int      IOplex();
static int      checkfiles();
#endif

void
usage(char *pname)
{
#ifdef HAVE_SSL
	fprintf(stderr, "USAGE: %s [-n certfile] [-f] hostname logfile(s)\n", pname);
#else
	fprintf(stderr, "USAGE: %s [-f] hostname logfile(s)\n", pname);
#endif
	return;
}

int
get_options(int argc, char **argv, char **hostname, char **certfile, int *foreground)
{
	int             c, errflag = 0;

	while (!errflag && (c = getopt(argc, argv, "fn:")) != -1)
	{
		switch (c)
		{
		case 'f':
			*foreground = 1;
			break;
#ifdef HAVE_SSL
		case 'n':
			*certfile = optarg;
			break;
#endif
		default:
			fprintf(stderr, "[%c] %d\n", c, __LINE__);
			errflag = 1;
			break;
		}
	}
	if (optind < argc)
		*hostname = argv[optind++];
	if (errflag)
		return (1);
	if (optind < argc)
		return (0);
	return (1);
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
#ifdef HOSTVALIDATE	
	struct hostent *hostptr;
#endif
	int             foreground;
	char           *ptr, *hostname, *certfile = (char *) 0;

	if ((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	if (get_options(argc, argv, &hostname, &certfile, &foreground)) {
		usage(ptr);
		return (1);
	}
#ifdef HOSTVALIDATE	
	if (!(hostptr = gethostbyname(argv[1])))
	{
		fprintf(stderr, "%s: Invalid hostname\n", argv[1]);
		return (1);
	}
	endhostent();
#endif	
	if (!(msghd = (struct msgtable *) malloc(sizeof(struct msgtable) * (argc - 1))))
	{
		perror("malloc");
		return(1);
	}
	return(consclnt(hostname, argv + optind, certfile));
}

int
consclnt(hostname, fname, clientcert)
	char          *hostname;
	char         **fname;
	char          *clientcert;
{
	char            lhost[MAXHOSTNAMELEN], seekfile[MAXBUF];
	int             sockfd, idx, fcount;
	long            seekval[2];
	struct msgtable *msgptr;
	char           *ptr, *seekdir;
	char          **fptr;

#ifdef SOLARIS
	(void) sysinfo(SI_HOSTNAME, lhost, MAXHOSTNAMELEN);
#else
	(void) gethostname(lhost, MAXHOSTNAMELEN);
#endif
	if ((fcount = (int) sysconf(_SC_OPEN_MAX)) == -1)
	{
		(void) fprintf(stderr, "sysconf: %s\n", strerror(errno));
		return(1);
	}
#ifdef SERVER
	switch (fork())
	{
	case -1:
		perror("fork");
		return (1);
	case 0:
		for(idx = 0;idx < fcount;idx++)
			close(idx);
		setsid();
		break;
	default:
		return (0);
	}
#endif
	for(fcount = 0, fptr = fname, msgptr = msghd;*fptr;fptr++)
	{
		if ((msgptr->fd = open(*fptr, O_RDONLY)) == -1)
		{
			perror(*fptr);
			continue;
		} else
		if (!(msgptr->fp = fdopen(msgptr->fd, "r")))
		{
			perror(*fptr);
			close(msgptr->fd);
			continue;
		}
		fcount++;
		(void) strcpy(msgptr->fname, *fptr);
		if ((ptr = strrchr(*fptr, '/')))
			ptr++;
		else
			ptr = *fptr;
		if (!(seekdir = getenv("SEEKDIR")))
			seekdir = SEEKDIR;
		(void) sprintf(seekfile, "%s/%s.seek", seekdir, ptr);
		if (!access(seekfile, R_OK))
		{
			if ((msgptr->seekfd = open(seekfile, O_RDWR)) == -1)
			{
				perror(seekfile);
				return (1);
			}
			if (read(msgptr->seekfd, seekval, sizeof(seekval)) == -1)
				return (1);
			fseek(msgptr->fp, seekval[0], SEEK_SET);
			msgptr->machcnt = seekval[1];
		} else
		if ((msgptr->seekfd = open(seekfile, O_CREAT | O_RDWR, 0644)) == -1)
		{
			perror(seekfile);
			return (1);
		} else
		{
			seekval[0] = 0l;
			seekval[1] = msgptr->machcnt = 0l;
		}
		msgptr++;
	}
	*(msgptr->fname) = 0;
	msgptr->fd = -1;
	msgptr->seekfd = -1;
	if (!fcount) {
		fprintf(stderr, "No log files opened\n");
		return (1);
	}
#ifdef DEBUG
	for(msgptr = msghd;msgptr->fd != -1;msgptr++)
	{
		printf("%s\n", msgptr->fname);
		printf("%d\n", msgptr->fd);
		printf("%d\n", msgptr->seekfd);
	}
#endif
	for (;;sleep(5))
	{
		for (;;)
		{
			if ((sockfd = tcpopen(hostname, "logsrv", 6340)) == -1)
			{
				if (errno == EINVAL)
					return(1);
				(void) sleep(5);
				continue;
			}
			break;
		}
#ifdef HAVE_SSL
	if (clientcert && tls_init(sockfd, clientcert))
		return (-1);
#endif
		/*- send my hostname */
		idx = strlen(lhost) + 1;
		getEnvConfigInt(&log_timeout, "LOGSRV_TIMEOUT", 120);
		if (safewrite(sockfd, lhost, idx, log_timeout) != idx)
		{
			filewrt(2, "unable to send hostname to %s\n", lhost);
#ifdef HAVE_SSL
			ssl_free();
#endif
			exit (1);
		}
		IOplex(lhost, sockfd);
		(void) close(sockfd);
	} /* for(;;) */
}

/* function for I/O multiplexing */
static int
IOplex(lhost, sockfd)
	char           *lhost;
	int             sockfd;
{
	register int    Bytes;
	int             sleepinterval;
	char            Buffer[MAXBUF];
	fd_set          FdSet;
	long            seekval[2];
	struct msgtable *msgptr;
	register char  *Ptr;

	if (!(Ptr = getenv("SLEEPINTERVAL")))
		sleepinterval = 5;
	else
		sleepinterval = atoi(Ptr);
	for (;;)
	{
		/* include message files and sockfd in file descriptor set */
		FD_ZERO(&FdSet);
		for(msgptr = msghd;msgptr->fd != -1;msgptr++)
			FD_SET(msgptr->fd, &FdSet);
		FD_SET(sockfd, &FdSet);
		if ((Bytes = select(sockfd + 1, &FdSet, (fd_set *) 0, (fd_set *) 0, 
			(struct timeval *) 0)) == -1)
		{
			if (errno == EINTR)
				continue;
			(void) filewrt(2, "select: %s\n", strerror(errno));
			(void) sslwrt(sockfd, log_timeout, "select: %s\n", strerror(errno));
			return (-1);
		}
		if (!Bytes)
			continue;
		if (FD_ISSET(sockfd, &FdSet))
		{
			for (;;)
			{
				errno = 0;
				if ((Bytes = saferead(sockfd, Buffer, MAXBUF, log_timeout)) == -1)
				{
					if (errno == EINTR)
						continue;
					return (-1);
				}
				break;
			}
			if (!Bytes)
				return (0);
		}
		for(msgptr = msghd;msgptr->fd != -1;msgptr++)
		{
			if (FD_ISSET(msgptr->fd, &FdSet))
			{
#ifdef DEBUG
				printf("Selecting file[%s] fd[%d] seekfd[%d]\n", msgptr->fname, msgptr->fd, msgptr->seekfd);
#endif
				for (;;)
				{
					errno = 0;
					if (!fgets(Buffer, MAXBUF - 2, msgptr->fp) || feof(msgptr->fp))
					{
						/*
						 * If message file has been moved than update
						 * seekfile
						 */
						(void) clearerr(msgptr->fp);
						if (checkfiles(msgptr->fname, msgptr->fp, seekval, msgptr->seekfd))
						{
							if (sslwrt(sockfd, log_timeout, "%s %d Update after EOF or File changed\n", lhost, (msgptr->machcnt)++) == -1)
								return(-1);
							continue;
						}
						else
							break;
					} else
					{
						seekval[0] = ftell(msgptr->fp);
						if (sslwrt(sockfd, log_timeout, "%s %ld %s", lhost, (msgptr->machcnt)++, Buffer) == -1)
							return (-1);
						seekval[1] = msgptr->machcnt;
						if (lseek(msgptr->seekfd, 0, SEEK_SET)) ;
						if (write(msgptr->seekfd, seekval, sizeof(seekval)) == -1) ;
					}
				} /* end of for(;;) */
			}	/* End of FD_ISSET(masterfd) */
		} /* end of for(msgptr = msghd;;) */
		sleep(sleepinterval);
	}	/* end of for(;;) */
}

static int
checkfiles(fname, msgfp, seekval, seekfd)
    char           *fname;
    FILE           *msgfp;
	long            *seekval;
	int             seekfd;
{
	int             fd, msgfd, count;
	long            tmpseekval;

	msgfd = fileno(msgfp);
	for (count = 0;; count++)
	{
		if ((fd = open(fname, O_RDONLY)) == -1)
		{
			sleep(5);
			continue;
		} else
		if (count)	/* new message file has been created */
		{
			(void) close(msgfd);
			(void) dup2(fd, msgfd);
			if (fd != msgfd)
				(void) close(fd);
			rewind(msgfp);
			if (lseek(seekfd, 0l, SEEK_SET)) ;
			seekval[0] = 0l;
			if (write(seekfd, seekval, sizeof(seekval)) == -1) ;
			return (1);
		} else
			break;
	}
	tmpseekval = lseek(fd, 0l, SEEK_END);
	if (tmpseekval == seekval[0])/* Just an EOF on message file */
	{
		(void) close(fd);
		return (0);
	} else
	if (tmpseekval > seekval[0]) /* update happened after EOF */
	{
		close(fd);
		return(1);
	} else	/* new message file has been created */
	{
		(void) close(msgfd);
		(void) dup2(fd, msgfd);
		if (fd != msgfd)
			(void) close(fd);
		rewind(msgfp);
		(void) lseek(seekfd, 0l, SEEK_SET);
		seekval[0] = 0l;
		if (write(seekfd, &seekval, sizeof(seekval)) == -1) ;
		return (1);
	}
}

#ifndef	lint
void
getversion_logclient_c()
{
	printf("%s\n", sccsid);
}
#endif
