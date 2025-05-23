dnl Process this file with autogen.sh to produce a configure script.

AC_INIT(supertuxkart, 0.6.2a)
AC_CONFIG_SRCDIR([src/main.cpp])

AC_PREREQ(2.61)

dnl ================================================
dnl Initialize the automake stuff
dnl set the $host variable based on local machine/os
dnl ================================================
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([foreign tar-ustar])


dnl =====================================================
dnl Set the default CFLAGS/CXXFLAGS
dnl If these were not set, the AC_PROG_CC and AC_PROG_CXX
dnl scripts set them to "-g -O2".
dnl =====================================================
CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -Wmissing-prototypes"
CXXFLAGS="$CXXFLAGS -Wall"

dnl ===================
dnl Check for compilers
dnl ===================
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX

# This macro defines WORDS_BIGENDIAN if building for a big endian machine.
# This is required by ov_read (Ogg Vorbis)
AC_C_BIGENDIAN


AC_LANG(C++)


dnl ===========================
dnl Check for `configure' flags
dnl ===========================
SUMMARY="\nSummary of optional components:\n==============================="

if test "x$with_plib" != "x" ; then
   echo "plib prefix is $with_plib"
   EXTRA_DIRS="${EXTRA_DIRS} $with_plib"
fi

AC_ARG_ENABLE(debug, [AS_HELP_STRING(--enable-debug,
                                     [enable debugging info])])
if test x$enable_debug = xyes; then
    AC_DEFINE([DEBUG], [],  [enable debugging cheats and shortcuts])
    CFLAGS="$CFLAGS -g -pg"
    CXXFLAGS="$CXXFLAGS -g -pg"
    SUMMARY="$SUMMARY\nEnabled debugging, please disable for a release!!"
else
    CFLAGS="$CFLAGS -DNDEBUG"
    CXXFLAGS="$CXXFLAGS -DNDEBUG"
    SUMMARY="$SUMMARY\nDisabled debugging, asserts have been turned off"
fi

AC_ARG_ENABLE(optimization, [AS_HELP_STRING(--disable-optimization,
                                            [disable compiler optimizations])],,
    enable_optimization=yes)
if test x$enable_optimization = xyes; then
    CFLAGS="$CFLAGS -O2"
    CXXFLAGS="$CXXFLAGS -O2"
else
    SUMMARY="$SUMMARY\nDisabled compiler optimizations."
fi

dnl ===================
dnl Checks for programs.
dnl ===================
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL

dnl ===============================
dnl Check for MS Windows environment
dnl ================================
AC_CHECK_HEADER(windows.h)

dnl ==================================
dnl check for OpenGL related libraries
dnl ==================================
case "${host}" in
*darwin*|*macosx*)
    opengl_LIBS="$LIBS -framework Carbon -framework OpenGL -framework IOKit -framework AGL"
    ;;

*-*-cygwin* | *-*-mingw32*)
    dnl CygWin under Windoze.

    echo Win32 specific hacks...
    AC_DEFINE([WIN32], 1, [Define for Win32 platforms])
    AC_DEFINE([NOMINMAX], 1, [Define for Win32 platforms])

    opengl_LIBS="-lglut32 -lglu32 -lopengl32 -luser32 -lgdi32 -lm -lwinmm"
    joystick_LIBS="$joystick_LIBS -lwinmm"

    echo "Will link apps with $LIBS"
    ;;

*)
   dnl X-Windows based machines

   save_LIBS=$LIBS
   AC_SEARCH_LIBS(glNewList, [ GL GLcore MesaGL ])
   opengl_LIBS="$LIBS"
   LIBS=$save_LIBS
   if test "x$ac_cv_search_glNewList" = "x-lGLcore"; then
       dnl if GLcore found, then also check for GL
       AC_SEARCH_LIBS(glXCreateContext, GL)
   fi
   ;;
esac

dnl ========================================
dnl check for OpenAL libraries
dnl ========================================
save_LIBS=$LIBS

case "${host}" in
*darwin*|*macosx*)
    # search for OpenAL installed in a UNIX way first. This is necessary since OS X 10.4
    # comes with an OpenAL framework that's too old, so to retain 10.4 support we need to
    # build OpenAL from source, and can't use Apple's framework.
    AC_SEARCH_LIBS(alGenBuffers, [openal alut], have_al_lib=yes)
    
    # We couldn't find OpenAL installed the UNIX way, so fall back to using frameworks
    if test x$have_al_lib != xyes; then
        LIBS="-framework OpenAL $LIBS"
        have_al_lib=yes
    fi
    ;;
*)
    AC_SEARCH_LIBS(alGenBuffers, [openal alut], have_al_lib=yes)
    ;;
esac

if test x$have_al_lib = xyes; then
    # Apple has the headers in OpenAL/al.h instead of AL/al.h 
    AC_CHECK_HEADERS([AL/al.h OpenAL/al.h], have_al_hdr=yes)
fi

if test x$have_al_hdr != xyes; then
    AC_MSG_ERROR([[cannot find useable OpenAL installation. You need to install OpenAL.]])
fi

openal_LIBS="$LIBS"
LIBS="$save_LIBS"

# ====================
# check for Ogg Vorbis
# ====================

save_LIBS=$LIBS

dnl search for the libraries needed to use ogg vorbis
AC_SEARCH_LIBS(ov_open, vorbisfile, have_vorbisfile_lib=yes)

if test x$have_vorbisfile_lib = xyes; then       
    dnl check for the headers needed by ogg vorbis
    AC_CHECK_HEADER(vorbis/vorbisfile.h, have_vorbisfile_hdr=yes)
    AC_CHECK_HEADER(ogg/ogg.h, have_ogg_hdr=yes)
fi
SUMMARY="$SUMMARY\n"

if test x$have_vorbisfile_hdr = xyes -a x$have_ogg_hdr = xyes; then
    AC_DEFINE([HAVE_OGGVORBIS], 1, [Defined when Ogg Vorbis is available])
    SUMMARY="$SUMMARY\nWe have Ogg Vorbis support."
else
    AC_MSG_ERROR([[cannot find useable Ogg Vorbis installation. You need to install libogg and libvorbis.]])
    #SUMMARY="$SUMMARY\nNo Ogg Vorbis support - the game will play with sound effects, but without music."
    #SUMMARY="$SUMMARY\nPlease install ogg vorbis!"
fi

oggvorbis_LIBS="$LIBS"
LIBS=$save_LIBS

# The following are C++ items that need to be tested for with the c++
# compiler
AC_LANG_PUSH(C++)


# ==============================================
# Check for "plib" without which we cannot go on
# ==============================================
# specify the plib location
AC_ARG_WITH(plib, [AS_HELP_STRING(--with-plib=PREFIX,
                                  [specify the prefix path to plib])])
if test "x$with_plib" != "x" ; then
   echo "plib prefix is $with_plib"
   EXTRA_DIRS="${EXTRA_DIRS} $with_plib"
fi

AC_CHECK_HEADER(plib/ul.h)
if test "x$ac_cv_header_plib_ul_h" != "xyes"; then
    AC_MSG_ERROR([[cannot find useable plib installation. You need to install plib 1.8.4 or newer.]])
fi

AC_MSG_CHECKING([for plib 1.8.4 or newer])
AC_TRY_RUN([
#  include <plib/ul.h>
 
#  define MIN_PLIB_VERSION 184
   int main() {
   if ( PLIB_VERSION < MIN_PLIB_VERSION ) {
      return -1;
    }
   return 0;
   }
 ],
  AC_MSG_RESULT(yes),
  [ AC_MSG_RESULT(wrong version);
    AC_MSG_ERROR([plib 1.8.4 or later is needed. Please intall it first.])],
  AC_MSG_RESULT(yes)
)



dnl =======================
dnl check for SDL libraries
dnl =======================
dnl Potentially sdl-config could be used, but apparently that might not
dnl exist on all platforms, so doing the 'classical' test is safer :(
case "${host}" in
*)
   dnl default unix style machines

   dnl check for libraries
   save_LIBS=$LIBS 
   AC_SEARCH_LIBS(SDL_Init, SDL, have_SDL_lib=yes)
   sdl_LIBS="$LIBS"
   LIBS=$save_LIBS

   dnl check for header files
   AC_CHECK_HEADER(SDL/SDL.h, have_SDL_hdr=yes)
   
   if test x$have_SDL_hdr != xyes -o x$have_SDL_lib != xyes; then
      AC_MSG_ERROR([Install SDL first...])
   fi

    AC_TRY_RUN([
    #  include <SDL/SDL.h>
       int main() {
       if ( SDL_VERSION_ATLEAST(1,2,10)) {
          return 0;
        }
       return -1;
       }
     ],
      :,
        AC_MSG_WARN([Your SDL version uses deprecated
    headers. Please update to version 1.2.10 or higher if possible.
    Currently this doesn't affects the game at all.])
    )

esac


dnl ======================================================
dnl Try to supply the SVN revision number for the compiler
dnl ======================================================
AC_CHECK_PROGS(have_svnversion,[svnversion],"no")
if test x$have_svnversion != xno; then
  SVNVERSION="`svnversion $srcdir`"
  AC_DEFINE_UNQUOTED([SVNVERSION],"$SVNVERSION",["SVN revision number"])
fi

AC_MSG_CHECKING([for gcc -fno-regmove option ])

dnl =========================
dnl test regmove compiler bug
dnl =========================
dnl Test if -fno-regmove is supported, since -fregmove (which is part of -O2 on
dnl g++) triggers an optimisation problem with static_ssg:
ORIGCXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -fno-regmove"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
                  AC_MSG_RESULT(yes); NOREGMOVE=-fno-regmove,
                  AC_MSG_RESULT(no);  NOREGMOVE="")
CXXFLAGS="$ORIGCXXFLAGS"
AC_SUBST(NOREGMOVE)

dnl ==============================================
dnl Glut (optional), used by bullet debugging only
dnl ==============================================
save_LIBS=$LIBS
save_LDFLAGS=$LDFLAGS
save_CXXFLAGS=$CXXFLAGS
case "${host}" in
*darwin*|*macosx*)
    dnl Apparently -framework is needed for include files and libraries
    CXXFLAGS="$CXXFLAGS -framework GLUT"
    LDFLAGS="$LDFLAGS -framework GLUT"
esac
AC_SEARCH_LIBS(glutInit, [glut], have_glut_lib=yes)

if test x$have_glut_lib = xyes; then
    # Apple has the headers in GLUT/glut.h instead of GL/glut.h 
    AC_CHECK_HEADERS([GL/glut.h GLUT/glut.h], have_glut_hdr=yes)
fi
if test x$have_glut_hdr = xyes; then
  AC_DEFINE(HAVE_GLUT)
  SUMMARY="$SUMMARY\nGlut found, enabling optional bullet debug"
else
  SUMMARY="$SUMMARY\nNo glut found, disabling bullet debug"
  dnl Remove the unnecessary -framework GLUT on Macs (no impact
  dnl of this on any other platforms)
  CXXFLAGS=$save_CSSFLAGS
  LDFLAGS=$save_LDFLAGS
fi
glut_LIBS="$LIBS"
LIBS=$save_LIBS

dnl ==============
dnl Bullet physics
dnl ==============
AC_DEFINE([BT_NO_PROFILE], [], [Disable bullet internal profiling])
SUMMARY="$SUMMARY\nUsing bullet physics."
BULLETTREE="src/bullet"
bullet_LIBS=""
if test x$have_glut_hdr = xyes; then
  bullet_LIBS="-Lbullet/Demos/OpenGL -lbulletopenglsupport"
fi
bullet_LIBS="$bullet_LIBS -Lbullet/src -lbulletdynamics -lbulletcollision -lbulletmath"


dnl ====
dnl enet
dnl ====
AC_CHECK_FUNC(gethostbyaddr_r, [AC_DEFINE(HAS_GETHOSTBYADDR_R)])
AC_CHECK_FUNC(gethostbyname_r, [AC_DEFINE(HAS_GETHOSTBYNAME_R)])
AC_CHECK_FUNC(poll, [AC_DEFINE(HAS_POLL)])
AC_CHECK_FUNC(fcntl, [AC_DEFINE(HAS_FCNTL)])
AC_CHECK_FUNC(inet_pton, [AC_DEFINE(HAS_INET_PTON)])
AC_CHECK_FUNC(inet_ntop, [AC_DEFINE(HAS_INET_NTOP)])

AC_CHECK_MEMBER(struct msghdr.msg_flags, [AC_DEFINE(HAS_MSGHDR_FLAGS)], , [#include <sys/socket.h>])

AC_CHECK_TYPE(socklen_t, [AC_DEFINE(HAS_SOCKLEN_T)], , 
              #include <sys/types.h>
              #include <sys/socket.h>
)

AC_EGREP_HEADER(MSG_MAXIOVLEN, /usr/include/sys/socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))
AC_EGREP_HEADER(MSG_MAXIOVLEN, socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))

case "${host}" in
*-*-cygwin* | *-*-mingw32*)
  enet_LIBS="-Lenet -lenet -lws2_32"
  ;;
*)
  enet_LIBS="-Lenet -lenet"
  ;;
esac
SUMMARY="$SUMMARY\nUsing enet network support."
ENETTREE="src/enet"

# Check for gettext
CHECK_GETTEXT(external)

dnl =======================================================
dnl Add special compiler flags for SDL on certain platforms
dnl =======================================================
case "${host}" in
*darwin*|*macosx*)
  dnl SDL on Macs needs to link in SDLmain, since it includes the
  dnl actual main program (on Macs #include "SDL/SDL.h" renames
  dnl main into SDL_main, which then gets called from the new
  dnl main program included in SDLmain. This is necessary to 
  dnl install a proper framework for SDL).
  dnl This can't be done earlier, since all SDL tests program
  dnl from configure do NOT include SDL/SDL.h, therefore the 
  dnl main program gets not replaced, and then getting an linking
  dnl error about main defined twice :((
  AC_CHECK_PROGS(have_sdl_config,[sdl-config],"no")
  if test x$have_sdl_config != xno; then
    LDFLAGS="$LDFLAGS `sdl-config --libs`"
    CXXFLAGS="$CXXFLAGS `sdl-config --cflags`"
  fi
esac

# =========================
# subst bars in Makefile.am
# =========================
AC_SUBST(bullet_LIBS)
AC_SUBST(enet_LIBS)
AC_SUBST(opengl_LIBS)
AC_SUBST(openal_LIBS)
AC_SUBST(oggvorbis_LIBS)
AC_SUBST(glut_LIBS)
AC_SUBST(sdl_LIBS)
AC_SUBST(BULLETTREE)
AC_SUBST(ENETTREE)

dnl ================
dnl Create makefiles
dnl ================
AC_CONFIG_FILES([                   \
 Makefile                           \
 data/Makefile                      \
 data/fonts/Makefile                \
 data/grandprix/Makefile            \
 data/items/Makefile                \
 data/karts/Makefile                \
 data/models/Makefile               \
 data/music/Makefile                \
 data/po/Makefile                   \
 data/sfx/Makefile                  \
 data/textures/Makefile             \
 data/tracks/Makefile               \
 doc/Makefile                       \
 src/Makefile                       \
 src/ide/Makefile                   \
 src/bullet/Makefile                \
 src/bullet/src/Makefile            \
 src/bullet/Demos/OpenGL/Makefile   \
 src/enet/Makefile                  \
])
AC_OUTPUT
echo -e $SUMMARY
