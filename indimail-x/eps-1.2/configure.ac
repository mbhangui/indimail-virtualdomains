dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT([eps],m4_normalize(m4_include([conf-version])),[Manvendra Bhangui indimail-virtualdomains@indimail.org],indimail, [https://github.com/mbhangui/indimail-virtualdomains])
AC_CANONICAL_HOST
LT_INIT
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CC_C99

case "$host" in
*-*-sunos4.1.1*)
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
*-*-solaris*)
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
*-*-linux*)
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
*-*-darwin*)
	CFLAGS="$CFLAGS -O3 -Wall -fPIC"
	;;
*-*-freebsd*)
	CFLAGS="$CFLAGS -O3 -Wall -fPIC"
	;;
*)
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
esac

defaultprefix="/var/indimail"
LIBVER=1:0:0
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(defaultprefix)
AC_SUBST(LIBVER)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_ISC_POSIX

dnl Checks for libraries.
dnl Checks for header files.
AC_HEADER_STDC
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_PROG_INSTALL

AC_OUTPUT(Makefile)
