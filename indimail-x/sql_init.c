/*
 * $Log: sql_init.c,v $
 * Revision 1.3  2021-01-26 00:28:56+05:30  Cprogrammer
 * renamed sql_init() to in_sql_init() to avoid clash with dovecot sql authentication driver
 *
 * Revision 1.2  2020-04-01 18:58:04+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:36:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <mysql.h>
#include <getEnvConfig.h>
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: sql_init.c,v 1.3 2021-01-26 00:28:56+05:30 Cprogrammer Exp mbhangui $";
#endif

/*- NOTE: Not safe to be called on a socket by multiple processes simultaneously */
void
in_sql_init(int which, MYSQL *mysqlStruct)
{
	mysql[which]=*mysqlStruct;
	/* adjust connection pointers */
	mysql[which].net.last_error[0]=0;
	mysql[which].net.last_errno=0;
	mysql[which].info=0;
	mysql[which].affected_rows= ~(my_ulonglong) 0;
	switch (which)
	{
#ifdef CLUSTERED_SITE
		case 0:
			isopen_cntrl = 1;
			isopen_vauthinit[0] = 1;
			getEnvConfigStr(&cntrl_table, "CNTRL_TABLE", CNTRL_DEFAULT_TABLE);
			break;
#endif /*- #ifdef CLUSTERED_SITE */
		case 1:
			is_open = 1; /*- prevent sql_open() from opening a new connection */
#ifdef CLUSTERED_SITE
			isopen_vauthinit[1] = 1;
#endif /*- #ifdef CLUSTERED_SITE */
			getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
			getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
			break;
	}
	return;
}
