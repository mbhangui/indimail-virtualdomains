/*-
 * $Log: parseAddress.c,v $
 * Revision 1.1  2019-04-18 08:31:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: parseAddress.c,v 1.1 2019-04-18 08:31:48+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <stralloc.h>
#endif
#include "storeHeader.h"
#include "common.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("parseAddress: out of memory", 0);
	_exit(111);
}

void
parseAddress(struct header_t *h, stralloc *addr_buf)
{
	struct group_t *g = (struct group_t *) 0;
	struct address_t *a = (struct address_t *) 0;

	addr_buf->len = 0;
	if (!(h->atoms)) {
		strerr_warn1("parseAddress: no atoms", 0);
		return;
	}
	if (!(h->atoms->next)) {
		strerr_warn1("parseAddress: no atoms", 0);
		return;
	}
	if (!(g = (struct group_t *) address_evaluate((char *) h->data))) {
		strerr_warn3("parseAddress: ", (char *) h->name, ": no valid addresses", 0);
		return;
	}
	if (verbose) {
		out("parseAddress", (char *) h->name);
		out("parseAddress", ": ");
		out("parseAddress", g->group ? "(of group " : "");
		out("parseAddress", g->group ? g->group : "");
		out("parseAddress", g->group ? ")" : "\0");
		flush("parseAddress");
	}
	for (a = g->members; a->next; a = a->next) {
		if (a->next->user && a->next->domain) {
			if (verbose) {
				if (a->next->name) {
					out("parseAddress", "  (");
					out("parseAddress", a->next->name ? a->next->name : "");
					out("parseAddress", ") { [");
					out("parseAddress", a->next->user ? a->next->user : "N/A");
					out("parseAddress", "] @ [");
					out("parseAddress", a->next->domain ? a->next->domain : "N/A");
					out("parseAddress", "] }\n");
				} else {
					out("parseAddress", "  { [");
					out("parseAddress", a->next->user ? a->next->user : "N/A");
					out("parseAddress", "] @ [");
					out("parseAddress", a->next->domain ? a->next->domain : "N/A");
					out("parseAddress", "] }\n");
				}
				flush("parseAddress");
			}
			if (a->next->user) {
				if (!stralloc_copys(addr_buf, a->next->user) ||
						!stralloc_0(addr_buf))
					die_nomem();
				addr_buf->len--;
			}
			if (a->next->domain) {
				if (!stralloc_append(addr_buf, "@") ||
						!stralloc_cats(addr_buf, a->next->domain) ||
						!stralloc_0(addr_buf))
					die_nomem();
				addr_buf->len--;
			}
			if (a->next->next) {
				if (!stralloc_append(addr_buf, ",") || !stralloc_0(addr_buf))
					die_nomem();
				addr_buf->len--;
			}
		}
	}
	address_kill(g);
}
#endif
