##   -*- autoconf -*-

dnl this file is a fragment of acinclude.m4 from kdelibs-3.1.2
dnl (copyright notice and AC_CHECK_SOCKLEN_T function)

dnl    This file is part of the KDE libraries/packages
dnl    Copyright (C) 1997 Janos Farkas (chexum@shadow.banki.hu)
dnl              (C) 1997,98,99 Stephan Kulow (coolo@kde.org)

dnl    This file is free software; you can redistribute it and/or
dnl    modify it under the terms of the GNU Library General Public
dnl    License as published by the Free Software Foundation; either
dnl    version 2 of the License, or (at your option) any later version.

dnl    This library is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl    Library General Public License for more details.

dnl    You should have received a copy of the GNU Library General Public License
dnl    along with this library; see the file COPYING.LIB.  If not, write to
dnl    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl    Boston, MA 02111-1307, USA.

dnl Check for the type of the third argument of getsockname
AC_DEFUN(AC_CHECK_SOCKLEN_T, [
  AC_MSG_CHECKING(for socklen_t)
  AC_CACHE_VAL(ac_cv_socklen_t, [
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
      ],[
socklen_t a=0;
getsockname(0,(struct sockaddr*)0, &a);
      ],
      ac_cv_socklen_t=socklen_t,
      AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
        ],[
int a=0;
getsockname(0,(struct sockaddr*)0, &a);
        ],
        ac_cv_socklen_t=int,
        ac_cv_socklen_t=size_t
      )
    )
    AC_LANG_RESTORE
  ])

  AC_MSG_RESULT($ac_cv_socklen_t)
  if test "$ac_cv_socklen_t" != "socklen_t"; then
    AC_DEFINE_UNQUOTED(socklen_t, $ac_cv_socklen_t,
        [Define the real type of socklen_t])
  fi
dnl gpsdrive doesn't need ksize_t
dnl  AC_DEFINE_UNQUOTED(ksize_t, socklen_t, [Compatibility define])

])
