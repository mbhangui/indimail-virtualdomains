/*-
 * GLOBAL.H - RSAREF types and constants 
 */
#include <string.h>
/*
 * Copyright (C) RSA Laboratories, a division of RSA Data Security,
 * Inc., created 1991. All rights reserved.
 */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_ 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsidglobalh[] = "$Id: global.h,v 2.1 2014-05-14 16:36:13+05:30 Cprogrammer Stab mbhangui $";
#endif
/*-
 * PROTOTYPES should be set to one if and only if the compiler supports
 * function argument prototyping.
 * The following makes PROTOTYPES default to 1 if it has not already been
 * defined as 0 with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif
/*
 * POINTER defines a generic pointer type 
 */
typedef unsigned char *POINTER;
/*
 * UINT2 defines a two byte word 
 */
typedef unsigned short int UINT2;
/*
 * UINT4 defines a four byte word 
 */
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef uint32_t UINT4;
#else
#if defined(__alpha) && (defined(__osf__) || defined(__linux__))
typedef unsigned int UINT4;
#else
typedef unsigned long int UINT4;
#endif
#endif

#ifndef NULL_PTR
#define NULL_PTR ((POINTER)0)
#endif
#ifndef UNUSED_ARG
#define UNUSED_ARG(x) x = *(&x);
#endif
/*
 * PROTO_LIST is defined depending on how PROTOTYPES is defined above.
 * If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
 * returns an empty list.  
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif
#endif /*- end _GLOBAL_H_ */

