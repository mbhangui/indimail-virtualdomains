/*-
 * $Log: md5.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * md5.h
 *
 * Revision 2.2  2011-12-05 13:47:41+05:30  Cprogrammer
 * moved out global defines to global.h
 *
 * Revision 2.1  2002-05-10 23:18:17+05:30  Cprogrammer
 * New revision of md5 routines
 *
 * MD5.H - header file for MD5C.C
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 * 
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 * 
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.  
 * 
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.  
 * 
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.  
 */

#ifndef _MD5_H_
#define _MD5_H_ 1

#ifndef	lint
static char     sccsidmd5h[] = "$Id: md5.h,v 1.1 2019-04-13 23:39:27+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef __cplusplus
extern          "C"
{
#endif

	/*- MD5 context.  */
	typedef struct
	{
		UINT4           state[4];	/*- state (ABCD) */
		UINT4           count[2];	/*- number of bits, modulo 2^64 (lsb first) */
		unsigned char   buffer[64];	/*- input buffer */
	}
	MD5_CTX;
	void MD5Init    PROTO_LIST((MD5_CTX *));
	void MD5Update  PROTO_LIST((MD5_CTX *, unsigned char *, unsigned int));
	void MD5Final   PROTO_LIST((unsigned char[16], MD5_CTX *));

#ifdef __cplusplus
}
#endif
#endif