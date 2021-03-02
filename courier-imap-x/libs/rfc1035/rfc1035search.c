/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<string.h>
#include	<stdlib.h>


static int found(const struct rfc1035_reply *r,
	const struct rfc1035_rr *rrp, const char *h,
	unsigned qclass, unsigned qtype)
{
char	namebuf[RFC1035_MAXNAMESIZE+1];

	if ((rrp->rrtype != qtype &&
		rrp->rrtype != RFC1035_TYPE_CNAME) ||
			rrp->rrclass != qclass)	return (0);

	if (rfc1035_replyhostname(r, rrp->rrname, namebuf) == 0)
		return (0);

	if (rfc1035_hostnamecmp(namebuf, h) == 0)	return (1);
	return (0);
}

int rfc1035_replysearch_an(const struct rfc1035_res *res,
			   const struct rfc1035_reply *r, const char *h,
			   unsigned qtype, unsigned qclass, int pos)
{
	if (res->rfc1035_defaultdomain && strchr(h, '.') == NULL)
	{
		char *buf;
		int rc;

		/* Append default domain */

		if ((buf=malloc(strlen(h)+
				strlen(res->rfc1035_defaultdomain)+2)) == 0)
			return (-1);

		strcat(strcat(strcpy(buf, h), "."),
		       res->rfc1035_defaultdomain);

		rc=rfc1035_replysearch_an(res, r, buf, qtype, qclass, pos);
		free(buf);
		return (rc);
	}

	for ( ; pos >= 0 && pos < r->ancount; pos++)
		if (found(r, r->anptr+pos, h, qclass, qtype))	return (pos);
	return (-1);
}

int rfc1035_replysearch_ns(const struct rfc1035_res *res,
			   const struct rfc1035_reply *r, const char *h,
			   unsigned qtype, unsigned qclass, int pos)
{
	if (res->rfc1035_defaultdomain && strchr(h, '.') == NULL)
	{
		char *buf;
		int rc;

		/* Append default domain */

		if ((buf=malloc(strlen(h)+
				strlen(res->rfc1035_defaultdomain)+2)) == 0)
			return (-1);

		strcat(strcat(strcpy(buf, h), "."),
		       res->rfc1035_defaultdomain);

		rc=rfc1035_replysearch_ns(res, r, buf, qtype, qclass, pos);
		free(buf);
		return (rc);
	}

	for ( ; pos >= 0 && pos < r->nscount; pos++)
		if (found(r, r->nsptr+pos, h, qclass, qtype))	return (pos);
	return (-1);
}

int rfc1035_replysearch_ar(const struct rfc1035_res *res,
			   const struct rfc1035_reply *r, const char *h,
			   unsigned qtype, unsigned qclass, int pos)
{
	if (res->rfc1035_defaultdomain && strchr(h, '.') == NULL)
	{
		char *buf;
		int rc;

		/* Append default domain */

		if ((buf=malloc(strlen(h)+
				strlen(res->rfc1035_defaultdomain)+2)) == 0)
			return (-1);

		strcat(strcat(strcpy(buf, h), "."),
		       res->rfc1035_defaultdomain);

		rc=rfc1035_replysearch_ar(res, r, buf, qtype, qclass, pos);
		free(buf);
		return (rc);
	}

	for ( ; pos >= 0 && pos < r->arcount; pos++)
		if (found(r, r->arptr+pos, h, qclass, qtype))	return (pos);
	return (-1);
}

int rfc1035_replysearch_all(const struct rfc1035_res *res,
			   const struct rfc1035_reply *r, const char *h,
			    unsigned qtype, unsigned qclass, int pos)
{
	unsigned s;

	if (res->rfc1035_defaultdomain && strchr(h, '.') == NULL)
	{
		char *buf;
		int rc;

		/* Append default domain */

		if ((buf=malloc(strlen(h)+
				strlen(res->rfc1035_defaultdomain)+2)) == 0)
			return (-1);

		strcat(strcat(strcpy(buf, h), "."),
		       res->rfc1035_defaultdomain);

		rc=rfc1035_replysearch_all(res, r, buf, qtype, qclass, pos);
		free(buf);
		return (rc);
	}

	s=r->ancount+r->nscount+r->arcount;

	for ( ; pos >= 0 && pos < s; pos++)
		if (found(r, r->allrrs[pos], h, qclass, qtype))	return (pos);
	return (-1);
}

int rfc1035_resolve_cname(struct rfc1035_res *res,
			  char *namebuf,
			  unsigned qtype,
			  unsigned qclass,
			  struct rfc1035_reply **ptr,
			  int x_flags)
{
int	n;
int	recursion_count=10;

	if ( (*ptr=rfc1035_resolve(res, RFC1035_OPCODE_QUERY,
			namebuf, qtype, qclass)) == 0)
		return (-1);

	if ((x_flags & RFC1035_X_RANDOMIZE) &&
	    (*ptr)->rcode == RFC1035_RCODE_NOERROR)
		rfc1035_rr_rand(*ptr);

	if ( (*ptr)->rcode != RFC1035_RCODE_NOERROR ||
		(n=rfc1035_replysearch_an( res,
					   *ptr, namebuf, qtype, qclass, 0))<0)
		return (-1);

	if ( (*ptr)->anptr[n].rrtype == qtype)	return (n);

	/* Must've gotten back a CNAME when we were looking for something else
	*/

	for (;;)
	{

		if (rfc1035_replyhostname( *ptr, (*ptr)->anptr[n].rr.domainname,
			namebuf) == 0)
		{
			rfc1035_replyfree( *ptr );
			*ptr=0;
			return (-1);
		}

	/* Let's see if we already have the answer for the canonical name */

		if ( (n=rfc1035_replysearch_all( res, *ptr, namebuf, qtype,
				qclass, 0)) >= 0)
		{
			if ( (*ptr)->anptr[n].rrtype == qtype)	return (n);
			if ( --recursion_count > 0)
				continue;

			rfc1035_replyfree( *ptr );	/* Recursive CNAME */
			*ptr=0;
			return (RFC1035_ERR_CNAME_RECURSIVE);
		}

		rfc1035_replyfree( *ptr );
		if ( (*ptr=rfc1035_resolve(res, RFC1035_OPCODE_QUERY,
				namebuf, qtype, qclass)) == 0)
			return (-1);

		if ( (*ptr)->rcode == RFC1035_RCODE_NOERROR &&
			(n=rfc1035_replysearch_an( res, *ptr, namebuf,
				qtype, qclass, 0)) >= 0)
		{
			if ( (*ptr)->anptr[n].rrtype == qtype)	return (n);
			if ( --recursion_count > 0)
				continue;
			rfc1035_replyfree( *ptr );	/* Recursive CNAME */
			*ptr=0;
			return (RFC1035_ERR_CNAME_RECURSIVE);
		}
		return (-1);
	}
}

int rfc1035_resolve_cname_multiple(struct rfc1035_res *res,
				   char *namebuf,
				   unsigned char *qtypes,
				   unsigned qclass,
				   struct rfc1035_reply **ptr,
				   int x_flags)
{
	int	n, query_again;
	int	retry_count=10;
	unsigned nqueries=strlen((char*)qtypes), u, good, cname;
	struct rfc1035_query qu[nqueries];
	struct rfc1035_reply *p;

	char	savename[RFC1035_MAXNAMESIZE+1],
		cmpname1[RFC1035_MAXNAMESIZE+1],
		cmpname2[RFC1035_MAXNAMESIZE+1];

	static const char forbidden[] =
		{
		 RFC1035_TYPE_CNAME,
		 RFC1035_TYPE_AXFR,
		 RFC1035_TYPE_ANY,
		 0
		};

	*ptr=NULL;

	if (nqueries == 0)
		return -1;

	strcpy(savename, namebuf);
	for (u=0; u<nqueries; ++u)
	{
		qu[u].name=namebuf;
		qu[u].qclass=qclass;
		qu[u].qtype=qtypes[u];

		if (strchr(forbidden, qtypes[u]))
		{
			if (nqueries == 1)
			{
				return rfc1035_resolve_cname(res, namebuf,
							     qtypes[0],
							     qclass,
							     ptr,
							     x_flags);
			}
			return -1;
		}
	}

	do // query loop
	{
		if (*ptr)
			rfc1035_replyfree( *ptr );

		if ( (*ptr=rfc1035_resolve_multiple(res, RFC1035_OPCODE_QUERY,
				qu, nqueries)) == 0)
			return (-1);

		query_again=0;

		for (;;) // change name loop
		{
			int fatal = 0;

			good = cname = 0;
			for (u=0, p=*ptr; p; ++u, p=p->next)
			{
				if (p->rcode != RFC1035_RCODE_NOERROR ||
					(n=rfc1035_replysearch_all(res,
								   p, namebuf,
								   qu[u].qtype,
								   qclass, 0))
				    < 0)
					continue;
				else if (p->allrrs[n]->rrtype == qu[u].qtype)
				{
					++good;
				}
				else // CNAME (not the queried type).
				{
					if (rfc1035_replyhostname(p, p->anptr[n].rr.domainname,
						cname? cmpname1: cmpname2) == 0)
							fatal=-1;

					else if (cname && strcmp(cmpname1, cmpname2))
						query_again=1;  // different aliases: try again

					++cname;
				}
			}

			if (good == 0 && cname == 0)
			{
				return (-1);
			}

			if (good > 0 && cname > 0)
				query_again=1; // have both cnames and non-cnames

			if (query_again)
			{
				if (--retry_count > 0)
				{
				  /* Query back from the original alias. */
					strcpy(namebuf, savename);
					break;
				}

				fatal=RFC1035_ERR_CNAME_RECURSIVE;
			}

			if (fatal)
			{
				rfc1035_replyfree( *ptr );
				*ptr=0;
				return fatal;
			}

			if (good)
				break;

		/* CNAME found.  Return alias to caller, and restart from there */

			strcpy(namebuf, cmpname1);
		}

	} while (query_again);

	if (x_flags & RFC1035_X_RANDOMIZE)
		for (p=*ptr; p; p=p->next)
			if (p->rcode == RFC1035_RCODE_NOERROR)
				rfc1035_rr_rand(p);

	return (0);
}
