dnl Check if the C++ compiler supports exceptions handling
dnl (try, throw and catch)
AC_DEFUN([AC_CXX_EXCEPTIONS],
[AC_CACHE_CHECK(whether the compiler supports exceptions,
ac_cv_cxx_exceptions,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(,[try { throw  1; } catch (int i) { return i; }],
 ac_cv_cxx_exceptions=yes, ac_cv_cxx_exceptions=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_exceptions" != yes; then
  AC_MSG_ERROR([no C++ exception support found])
fi
])



dnl Check if the compiler recognizes bool as a separate built-in type.
AC_DEFUN([AC_CXX_BOOL],
[AC_CACHE_CHECK(whether the compiler recognizes bool as a built-in type,
ac_cv_cxx_bool,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
int f(int  x){return 1;}
int f(char x){return 1;}
int f(bool x){return 1;}
],[bool b = true; return f(b);],
 ac_cv_cxx_bool=yes, ac_cv_cxx_bool=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_bool" != yes; then
  AC_MSG_ERROR([no built-in 'bool' C++ type])
fi
])



dnl Check if the compiler can prevent names clashes using namespaces.
AC_DEFUN([AC_CXX_NAMESPACES],
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ac_cv_cxx_namespaces,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([namespace Outer { namespace Inner { int i = 0; }}],
                [using namespace Outer::Inner; return i;],
 ac_cv_cxx_namespaces=yes, ac_cv_cxx_namespaces=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_namespaces" != yes; then
  AC_MSG_ERROR([C++ namespaces not supported])
fi
])



dnl Check if the C++ library has a working stringstream.
AC_DEFUN([AC_CXX_HAVE_SSTREAM],
[AC_CACHE_CHECK(whether the compiler has stringstream,
ac_cv_cxx_have_sstream,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <sstream>],
[std::stringstream message; message << "Hello"; return 0;],
 ac_cv_cxx_have_sstream=yes, ac_cv_cxx_have_sstream=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_sstream" != yes; then
  AC_MSG_ERROR([no functional stringstream])
  AC_DEFINE(HAVE_SSTREAM,,[define if the compiler has stringstream])
fi
])



dnl Check if the compiler supports ISO C++ standard library
dnl (i.e., can include the files iostream, map, iomanip and cmath}).
AC_DEFUN([AC_CXX_HAVE_STD],
[AC_CACHE_CHECK(whether the compiler supports ISO C++ standard library,
ac_cv_cxx_have_std,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <iostream>
#include <map>
#include <iomanip>
#include <cmath>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[return 0;],
 ac_cv_cxx_have_std=yes, ac_cv_cxx_have_std=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std" != yes; then
  AC_MSG_ERROR([no support for ISO C++ standard library])
fi
])



dnl Check if the compiler supports the Standard Template Library.
AC_DEFUN([AC_CXX_HAVE_STL],
[AC_CACHE_CHECK(whether the compiler supports Standard Template Library,
ac_cv_cxx_have_stl,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <list>
#include <deque>],
[std::list<int> x; x.push_back(5); std::list<int>::iterator iter = x.begin();
 if (iter != x.end()) ++iter; return 0;],
 ac_cv_cxx_have_stl=yes, ac_cv_cxx_have_stl=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_stl" != yes; then
  AC_MSG_ERROR([no support for C++ STL])
fi
])



dnl Check if the compiler supports member constants.
AC_DEFUN([AC_CXX_MEMBER_CONSTANTS],
[AC_CACHE_CHECK(whether the compiler supports member constants,
ac_cv_cxx_member_constants,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([class C {public: static const int i = 0;}; const int C::i;],
[return C::i;],
 ac_cv_cxx_member_constants=yes, ac_cv_cxx_member_constants=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_member_constants" != yes; then
  AC_MSG_ERROR([no support by C++ compiler for members constants])
fi
])


AC_DEFUN(
        [CHECK_GNU_MAKE], [ AC_CACHE_CHECK( for GNU make,_cv_gnu_make_command,
                _cv_gnu_make_command='' ;
dnl Search all the common names for GNU make
                for a in "$MAKE" make gmake gnumake ; do
                        if test -z "$a" ; then continue ; fi ;
                        if  ( sh -c "$a --version" 2> /dev/null | grep GNU  2>&1 > /dev/null ) ;  then
                                _cv_gnu_make_command=$a ;
                                break;
                        fi
                done ;
        ) ;
dnl If there was a GNU version, then set @ifGNUmake@ to the empty string, '#' otherwise
        if test  "x$_cv_gnu_make_command" != "x"  ; then
                ifGNUmake='' ;
        else
                ifGNUmake='#' ;
                AC_MSG_RESULT("Not found");
        fi
        AC_SUBST(ifGNUmake)
] )


dnl Test for wxWidgets, and define WX_CXXFLAGS and WX_LIBS
dnl   to be used as follows:
dnl AM_PATH_WXWIDGETS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_WXWIDGETS,
[dnl 
dnl Get the cflags and libraries from the wx-config script
dnl
AC_ARG_WITH(wx-config,
[[  --with-wx-config=FILE   Use the given path to wx-config when determining
                            wxWidgets configuration; defaults to "wx-config"]],
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WXCONFIG=$withval
    fi
])

  AC_PATH_PROG(WXCONFIG, wx-config, no)
  min_wx_version=ifelse([$1], ,2.2.0,$1)

  if test "$WXCONFIG" != "no"; then
    AC_MSG_CHECKING(for wxWidgets version >= $min_wx_version)
    wx_version=`$WXCONFIG --version`
    if test "x$wx_version" = "x"; then
	  AC_MSG_RESULT([not found])
	  AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
    fi

    # TODO: test version
    v_maj=`echo $wx_version | \
		sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    v_min=`echo $wx_version | \
		sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    v_rev=`echo $wx_version | \
		sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    req_maj=`echo $min_wx_version | \
		sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    req_min=`echo $min_wx_version | \
		sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    req_rev=`echo $min_wx_version | \
		sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    proper_version=`expr \
		$v_maj \> $req_maj \| \
		$v_maj \= $req_maj \& \
		$v_min \> $req_min \| \
		$v_maj \= $req_maj \& \
		$v_min \= $req_min \& \
		$v_rev \>= $req_rev`
    if test "$proper_version" = "1"; then
        AC_MSG_RESULT([ok (found $wx_version)])
    else
        AC_MSG_ERROR([sorry, found $wx_version])
    fi

    WX_CXXFLAGS=`$WXCONFIG --cxxflags`
    WX_LIBS=`$WXCONFIG --libs`
    AC_SUBST(WX_CXXFLAGS)
    AC_SUBST(WX_LIBS)
  else
    # Die hard, with a vengeance!
    echo "#############################################"
    echo "# Sorry, wxWidgets is required to build.    #"
    echo "#   You can download it from:               #"
    echo "#     http://www.wxwidgets.org/             #"
    echo "#############################################"
    exit 1
  fi
])

