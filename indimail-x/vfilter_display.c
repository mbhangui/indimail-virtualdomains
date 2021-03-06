/*
 * $Log: vfilter_display.c,v $
 * Revision 1.1  2019-04-18 08:33:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_display.c,v 1.1 2019-04-18 08:33:56+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <fmt.h>
#include <strerr.h>
#include <subfd.h>
#include <qprintf.h>
#endif
#include "vfilter_select.h"
#include "vfilter_header.h"
#include "common.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_display: out of memory", 0);
	_exit(111);
}

void
format_filter_display(int type, int filter_no, char *emailid, stralloc *filter_name, int header_name, int comparision,
		stralloc *keyword, stralloc *folder, stralloc *forward, int bounce_action)
{
	static stralloc _filterName = {0}, _keyword = {0};
	char           *ptr, *_hname;
	char            strnum[FMT_ULONG];
	int             max_header_value;
	static char   **header_list;

	if (!type) {
		if (!header_list && !(header_list = headerList()))
			header_list = vfilter_header;
		for (max_header_value = 0; header_list[max_header_value]; max_header_value++);
		if (header_name >= max_header_value)
			_hname = "invalid header";
		else
			_hname = header_list[header_name];
		strnum[fmt_uint(strnum, filter_no)] = 0;
		qprintf(subfdoutsmall, strnum, "%3s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, emailid, "%-29s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, filter_name->s, "%-20s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, header_name == -1 ? "N/A" : _hname, "%-15s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, vfilter_comparision[comparision], "%-26s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, keyword->len ? keyword->s : "N/A", "%-15s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, !str_diffn(folder->s, "/NoDeliver", 11) ? "Void" : folder->s + 1, "%-15s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, (bounce_action == 1 || bounce_action == 3) ? "Yes" : "No", "%s");
		if (bounce_action > 1)
			qprintf(subfdoutsmall, forward->s, "%s");
		qprintf(subfdoutsmall, "\n", "%s");
		qprintf_flush(subfdoutsmall);
	} else { /*- raw display*/
		if (!stralloc_copy(&_filterName, filter_name) || !stralloc_0(&_filterName))
			die_nomem();
		for (ptr = _filterName.s; *ptr; ptr++) {
			if (isspace((int) *ptr))
				*ptr = '~';
		}
		for (ptr = _keyword.s; *ptr; ptr++) {
			if (isspace((int) *ptr))
				*ptr = '~';
		}
		strnum[fmt_uint(strnum, filter_no)] = 0;
		qprintf(subfdoutsmall, strnum, "%s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, emailid, "%s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, _filterName.s, "%s");
		qprintf(subfdoutsmall, " ", "%s");
		strnum[fmt_uint(strnum, header_name)] = 0;
		qprintf(subfdoutsmall, strnum, "%s");
		qprintf(subfdoutsmall, " ", "%s");
		strnum[fmt_uint(strnum, comparision)] = 0;
		qprintf(subfdoutsmall, strnum, "%s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, _keyword.len ? _keyword.s : "N/A", "%s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, folder->s, "%s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, bounce_action ? (bounce_action == 2 ? forward->s : "Bounce") : (str_diffn(folder->s, "/NoDeliver", 11) ? "Deliver" : "Vapour"), "%s");
		qprintf(subfdoutsmall, "\n", "%s");
		qprintf_flush(subfdoutsmall);
	}
	return;
}

int
vfilter_display(char *emailid, int disp_type)
{
	int             i, j, status = -1;
	int                   filter_no, header_name, comparision, bounce_action;
	static stralloc       filter_name = {0}, keyword = {0}, folder = {0}, forward = {0};

	for (j = 0;;) {
		i = vfilter_select(emailid, &filter_no, &filter_name, &header_name, &comparision, &keyword, &folder, &bounce_action, &forward);
		if (i == -1)
			break;
		else
		if (i == -2)
			break;
		if (!j++ && !disp_type) {
			out("vfilter_display", "No  EmailId                       FilterName Header          Comparision                Keyword         Folder          Bounce Delivery\n");
			out("vfilter_display", "--------------------------------------------------------------------------------------------------------------------------------------------------\n");
			flush("vfilter_display");
		}
		status = 0;
		format_filter_display(disp_type, filter_no, emailid, &filter_name, header_name, comparision, &keyword, &folder, 
			&forward, bounce_action);
		if (!disp_type) {
			out("vfilter_display", "----------------------------------------------"
				"----------------------------------------------"
				"----------------------------------------------"
				"--------\n");
		}
	}
	if (status == -1 && i == -2)
		return(-2);
	return(status);
}
#endif
