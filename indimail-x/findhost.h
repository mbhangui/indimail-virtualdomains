/*
 * $Log: findhost.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * findhost.h
 *
 */
#ifndef FINDHOST_H
#define FINDHOST_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int             open_central_db(char *);
char           *findhost(char *, int);
#ifdef QUERY_CACHE
void            findhost_cache(char);
#endif

#endif
