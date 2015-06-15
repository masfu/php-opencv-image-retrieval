dnl
dnl $ Id: image_retrieval 1.0.1$
dnl

PHP_ARG_WITH(image_retrieval, for ImageRetrieval support,
[  --with-image_retrieval           Enable ImageRetrieval support], yes)

if test "$PHP_IMAGE_RETRIEVAL" != "no"; then
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_OPENCV"

  export CPPFLAGS="$OLD_CPPFLAGS"

  PHP_REQUIRE_CXX()
  PHP_SUBST(IMAGE_RETRIEVAL_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, IMAGE_RETRIEVAL_SHARED_LIBADD)
  AC_DEFINE(HAVE_IMAGE_RETRIEVAL, 1, [ ])

  PHP_NEW_EXTENSION(
	image_retrieval, 
	image_retrieval.cpp, 
	$ext_shared,
	,
	,
	 "yes")

  EXT_IMAGE_RETRIEVAL_HEADERS="php_image_retrieval_api.h"

  ifdef([PHP_INSTALL_HEADERS], [
    PHP_INSTALL_HEADERS(ext/opencv, $EXT_IMAGE_RETRIEVAL_HEADERS)
  ])

  if test "$PHP_IMAGE_RETRIEVAL" != "no"; then
      OPENCV_CHECK_DIR=$PHP_IMAGE_RETRIEVAL
      OPENCV_TEST_FILE=opencv/include/cv.h
      OPENCV_LIBNAME=opencv
  fi
  condition="$OPENCV_CHECK_DIR$OPENCV_TEST_FILE"

  if test -r $condition; then
   OPENCV_DIR=$OPENCV_CHECK_DIR
     CFLAGS="$CFLAGS -I$OPENCV_DIR/include"
   LDFLAGS=`$OPENCV_DIR/bin/opencv-config --libs`
  else
    AC_MSG_CHECKING(for pkg-config)
  
    if test ! -f "$PKG_CONFIG"; then
      PKG_CONFIG=`which pkg-config`
    fi

      if test -f "$PKG_CONFIG"; then
        AC_MSG_RESULT(found)
        AC_MSG_CHECKING(for opencv)
    
        if $PKG_CONFIG --exists opencv; then
            if $PKG_CONFIG --atleast-version=2.1.0 opencv; then
                opencv_version_full=`$PKG_CONFIG --modversion opencv`
                AC_MSG_RESULT([found $opencv_version_full])
                OPENCV_LIBS="$LDFLAGS `$PKG_CONFIG --libs opencv`"
                OPENCV_INCS="$CFLAGS `$PKG_CONFIG --cflags-only-I opencv`"
                PHP_EVAL_INCLINE($OPENCV_INCS)
                PHP_EVAL_LIBLINE($OPENCV_LIBS, IMAGE_RETRIEVAL_SHARED_LIBADD)
                AC_DEFINE(HAVE_IMAGE_RETRIEVAL, 1, [whether opencv exists in the system])
            else
                AC_MSG_RESULT(too old)
                AC_MSG_ERROR(Ooops ! You need at least opencv 2.1.0)
            fi
        else
            AC_MSG_RESULT(not found)
            AC_MSG_ERROR(Ooops ! no opencv detected in the system)
        fi
      else
        AC_MSG_RESULT(not found)
        AC_MSG_ERROR(Ooops ! no pkg-config found .... )
      fi
   fi
	
fi
