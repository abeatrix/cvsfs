dnl acinclude.m4 - additional macros
dnl
dnl Author: Petric Frank <pfrank@gmx.de>
dnl
dnl Description: Macro to check some stl headers.
dnl
dnl              Following things are defined afterwards:
dnl
dnl                Macro              ! value if stl v3 ! value if stl < v3
dnl                -------------------+-----------------+------------------
dnl                HAVE_STREAMBUF     ! 1               ! undefined
dnl                HAVE_STREAMBUF_H   ! undefined       ! 1
dnl
dnl Usage:
dnl AC_CHECK_STL
#
AC_DEFUN([AC_CHECK_STL],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_CXX])

  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  
  AC_CHECK_HEADER(streambuf,AC_DEFINE(HAVE_STREAMBUF,1,[#include <streambuf> will work]),)
  AC_CHECK_HEADER(streambuf.h,AC_DEFINE(HAVE_STREAMBUF_H,1,[#include <streambuf> will work]),)

  AC_LANG_RESTORE
])
