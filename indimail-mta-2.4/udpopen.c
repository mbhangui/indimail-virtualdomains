/*
 * $Log: udpopen.c,v $
 * Revision 1.6  2018-01-09 12:37:19+05:30  Cprogrammer
 * removed header hasindimail.h
 *
 * Revision 1.5  2015-08-19 16:27:32+05:30  Cprogrammer
 * added missing call to getservbyname()
 *
 * Revision 1.4  2015-07-23 16:19:10+05:30  Cprogrammer
 * fixed compilation error for non-indimail install
 *
 * Revision 1.3  2015-04-14 20:02:48+05:30  Cprogrammer
 * use udp and htons for port
 *
 * Revision 1.2  2015-04-10 19:43:19+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.1  2015-04-10 19:38:13+05:30  Cprogrammer
 * Initial revision
 *
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: udpopen.c,v 1.6 2018-01-09 12:37:19+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include "subfd.h"
#include "strerr.h"
#include "fmt.h"
#include "byte.h"

#define FMT_ULONG 40

static int
isnum(str)
	char           *str;
{
	char           *ptr;

	for (ptr = str; *ptr; ptr++)
		if (!isdigit((int) *ptr))
			return (0);
	return (1);
}

int
udpopen(char *rhost, char *servicename)
{
	int             fd, retval;
	struct servent *sp;	/*- pointer to service information */
#ifdef IPV6
	struct addrinfo hints, *res, *res0;
	char            serv[FMT_ULONG];
#else
	struct hostent *hp;	/*- pointer to host info for nameserver host */
	struct sockaddr_in tcp_srv_addr;
#ifdef HAVE_IN_ADDR_T
	in_addr_t       inaddr;
#else
	unsigned long   inaddr;
#endif
#endif

#ifdef IPV6
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if (!servicename)
	{
		errno = EINVAL;
		return (-1);
	} 
	if (isnum(servicename))
		byte_copy(serv, FMT_ULONG, servicename);
	else
	{
		if ((sp = getservbyname(servicename, "udp")) == NULL)
		{
			errno = EINVAL;
			return (-1);
		}
		serv[fmt_ulong(serv, htons(sp->s_port))] = 0;
	}
	if ((retval = getaddrinfo(rhost, serv, &hints, &res0)))
	{
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_puts(subfderr, "getaddrinfo: ") == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_puts(subfderr, rhost) == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, ": ") == -1)
			strerr_die2sys(111, "tcpopen", "write: ");
		if (substdio_puts(subfderr, (char *) gai_strerror(retval)) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_put(subfderr, "\n", 1) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_flush(subfderr) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		return (-1);
	}
	for (fd = -1, res = res0; res && fd == -1; res = res->ai_next)
	{
		if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
			continue; /*- Try the next address record in the list. */
		if (!(retval = connect(fd, res->ai_addr, res->ai_addrlen)))
			break;
		retval = errno;
		close(fd);
		fd = -1;
		errno = retval;
	} /*- for (res = res0; res; res = res->ai_next) */
	freeaddrinfo(res0);
#else
	/*- clear out address structures */
	memset((char *) &tcp_srv_addr, 0, sizeof(struct sockaddr_in));
	if (isnum(servicename))
		tcp_srv_addr.sin_port = htons(atoi(servicename));
	else
	{
		if ((sp = getservbyname(servicename, "udp")) == NULL)
		{
			errno = EINVAL;
			return (-1);
		}
		tcp_srv_addr.sin_port = htons(sp->s_port);
	}
	tcp_srv_addr.sin_family = AF_INET;
	if ((inaddr = inet_addr(rhost)) != INADDR_NONE) /*- It's a dotted decimal */
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, (char *) &inaddr, sizeof(inaddr));
	else
	if ((hp = gethostbyname(rhost)) == NULL)
	{
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_puts(subfderr, "gethostbyname: ") == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_puts(subfderr, rhost) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_puts(subfderr, ": No such host\n") == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_flush(subfderr) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		errno = EINVAL;
		return (-1);
	} else
		(void) memcpy((char *) &tcp_srv_addr.sin_addr, hp->h_addr, hp->h_length);
	/*- Create the socket. */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);
	if ((retval = connect(fd, (struct sockaddr *) &tcp_srv_addr, sizeof(tcp_srv_addr))) == -1)
	{
		retval = errno;
		close(fd);
		errno = retval;
		return (-1);
	}
#endif
	return(fd);
}

void
getversion_udpopen_c()
{
	char *x = sccsid;
	if (x)
		x++;
}