/*
 * $Log: sql_active.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * sql_active.h
 *
 */
#ifndef SQL_ACTIVE_H
#define SQL_ACTIVE_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

int             sql_active(struct passwd *, char *, int);

#endif
