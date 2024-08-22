SUPERTUXKART INSTALLATION INSTRUCTIONS :
========================================

Windows Installation Process :
------------------------------

First, make sure that you have the following packages installed/compiled :

  * Visual Studio 2008 C++ (only to execute the IDE package).
  * OpenGL (GLUT or freeGLUT/Mesa 3.0 or later).
  * PLIB v1.8.5.
  * SDL 1.2 or later (not SDL2 and after).
  * OpenAL (SDK).
  * ENet (don't get a version using an old protocol).
  * Libintl3.
  * LibVorbis.
  * LibOgg.

ALL OF THE PACKAGES ABOVE MUST BE COMPILED IN RELEASE AND WIN32 CONFIG!!

Secondly, make a folder for the includes and another one for the libraries.

Copy your includes in folders and this is what you should get in you include folder :

 * C:/yourpath/yourincludefolder/AL/yourOpenALincludes,
 * C:/yourpath/yourincludefolder/enet/yourENetincludes,
 * C:/yourpath/yourincludefolder/GL/yourOpenGLincludes,
 * C:/yourpath/yourincludefolder/ogg/yourLibOggincludes,
 * C:/yourpath/yourincludefolder/plib/yourPLIBincludes,
 * C:/yourpath/yourincludefolder/SDL/yourSDL1.2includes,
 * C:/yourpath/yourincludefolder/vorbis/yourLibVorbisincludes.

and libintl.h (this file should be in your include folder).

Copy your libraries in your libraries folder and you should get something like this :

C:/yourpath/yourlibrariesfolder/allofyourlibraries.

To get the libraries files (.lib), you will need to compile all of the packages.

Don't forget to get the binary files of your packages (.dll) to the stk folder.

Now, you will need to create 2 environment variables :

 * STK_INCLUDE
 * STK_LIB

The STK_INCLUDE variable must lead to your include folder.
The STK_LIB variable must lead to your libraries folder.

When you've done this, go to : stkfolder/src/ide/vc9 and open supertuxkart.vcproj

To compile just change the BulletDebug to BulletRelease and the game will be compiled.

Go to the stk folder and enjoy the game!

Linux Installation Process :
----------------------------

I don't know how to compile the game on modern Linux :/

I would recommend to get help on the Unofficial Discord server of SuperTuxKart :
https://discord.gg/u2AJDpF

MacOs Installlation Process (not tested) :
------------------------------------------
(*) The latest information about compilation on Mac are on SuperTuxKart wiki:
http://supertuxkart.sourceforge.net/Building_and_packaging_on_OSX
(*) There is an experimental Xcode project in /src/ide/Xcode/. It will still require that all dependencies are installed as explained on the wiki.

System requirements Mac: 
PowerPC- or Intel-Mac with  800 MHz or more, 
1 GHz recommended 3D-graphics card with 32 MB or more. 
100 MB free disk space. Supporting Mac OS X 10.3.9 or later.

Compiling SuperTuxKart on OS X 10.4.x

01. Install all updates for OS X.

02. Download xcode 2.4.1_8m1910_6936315 from http://developer.apple.com/macosx/

You must be an Apple Developer Connection member to download this package.

03. Installing XCode.

04. Edit "/etc/profile":

--------------------------------------------------------------------------------------------------------

# System-wide .profile for sh(1)

PATH="/sw/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/X11R6/bin"

export PATH

PKG_CONFIG_PATH="/sw/lib/pkgconfig:/usr/lib/pkgconfig:/usr/X11R6/lib/pkgconfig:/usr/local/lib/pkgconfig"

export PKG_CONFIG_PATH

if [ "${BASH-no}" != "no" ]; then [ -r /etc/bashrc ] && . /etc/bashrc fi

--------------------------------------------------------------------------------------------------------

05. Download Fink from http://www.finkproject.org/download/index.php?phpLang=en

06. Installing Fink.

07. Copy the FinkCommander folder to Applications and launch FinkCommander.

08. Run selfupdate.

09. Run update-all.

10. Run the index command.

11. Installing the following 'fink' packages:


11.01 SDL

11.02 audiofile

11.03 esound

11.04 svn

11.05 svn-client


12. Download the SDL framework from http://www.libsdl.org/download-1.2.php and copy it to /Library/Frameworks.

13. Installing Vorbis.framework and Ogg.framework:

13.01 Download SuperTux from http://developer.berlios.de/project/showfiles.php?group_id=3467&release_id=11879

13.02 Right-click the SuperTux icon and select Show Package Contents from the context menu.

13.03 Change in the folder Contents:Frameworks. And copy Vorbis.framework and Ogg.framework to <hd>:Library:Frameworks.


14. Installing plib: ////////////////////////////////////////


14.01 Download PLIB from http://plib.sourceforge.net/download.html


14.02 Unpack plib-1.8.4.tar.gz.


14.03 Change in the folder PLIB.


14.04 Download "pwMacOSX.cxx"-patch from ftp://ftp.berlios.de/pub/supertuxkart/plib_patch_for_osx.zip


14.05 Installing pwMacOSX.cxx-patch.


14.06 Run ./configure --prefix=<..... Universal/plib_ppc>" in the folder PLIB.


e.g.: /Users/christian/Desktop/Universal/plib_ppc.


14.07 Modify the plib:


jsMacOSX.cxx:


#include <IOKit/IOkitLib.h>


replace with:


#include <IOKit/IOKitLib.h>


14.08 jsMacOSX.cxx:


static void os_specific_s::elementEnumerator( const void *element, void* vjs)


replace with:


void os_specific_s::elementEnumerator( const void *element, void* vjs)


14.09 make

14.10 make install

14.11 make clean


14.12 Run "./configure --prefix=<..... Universal/plib_x86>" in the folder PLIB.

e.g.: /Users/christian/Desktop/Universal/plib_x86.

14.13 Edit the following Makefiles:

<PLIB-Ordner>/Makefile

<PLIB-Ordner>/src/Makefile

<PLIB-Ordner>/src/fnt/Makefile

<PLIB-Ordner>/src/js/Makefile

<PLIB-Ordner>/src/net/Makefile

<PLIB-Ordner>/src/psl/Makefile

<PLIB-Ordner>/src/puAux/Makefile

<PLIB-Ordner>/src/pui/Makefile

<PLIB-Ordner>/src/pw/Makefile

<PLIB-Ordner>/src/sg/Makefile

<PLIB-Ordner>/src/sl/Makefile

<PLIB-Ordner>/src/ssg/Makefile

<PLIB-Ordner>/src/ssgAux/Makefile

<PLIB-Ordner>/src/util/Makefile

replace the line: "CXXFLAGS =" with "CXXFLAGS = -g -O2 -Wall -arch i386"

14.14 make install

14.15 Create a universal (multi-architecture) plib:

lipo -create /Users/christian/Desktop/Universal/plib_ppc/lib/libplibfnt.a /Users/christian/Desktop/Universal/

plib_x86/lib/libplibfnt.a -output /usr/lib/libplibfnt.a

Repeat this step for all libraries.

15. Installing Freealut-1.1.0 from OpenAL.org ////////////////////

15.01 Makefile.in:

Replace the line:

SUBDIRS = admin src include examples test_suite

with

SUBDIRS = admin src include


15.02 configure:

Replace the line

for ac_header in AL/alc.h AL/al.h basetsd.h ctype.h math.h stdio.h time.h

with

for ac_header in OpenAL/alc.h OpenAL/al.h basetsd.h ctype.h math.h stdio.h time.h


15.03 export CXXFLAGS="-framework OpenAL"


15.04 export LDFLAGS="-framework OpenAL"


15.05 ./configure --prefix=/usr


15.06 make


15.07 Open admin/pkgconfig/freealut.pc

Replace the line

Requires: openal

with

Requires:


15.08 Replace the line:

Libs: -L${libdir} -lalut Cflags: -I${includedir}

with

Libs: -framework OpenAL -L${libdir} -lalut Cflags: -framework OpenAL -I${includedir}


15.09 sudo make install


16. Copy the plib files to "/usr/local":

16.01 sudo cp /usr/lib/libplib* /usr/local/lib/

16.02 sudo cp -R /usr/include/plib /usr/local/include/


Installing SuperTuxKart //////////


1. Copy the following code and paste it in the new file "buildUB.sh".


buildUB.sh:

---------------------------------------------------------------------------------------------------------


#!/bin/bash

if [ -r ./configure ]; then

echo "Configure found!"

else

echo "No Config file found! Runing autogen.sh..."

sh autogen.sh

fi


if [ -r ./config.guess ]; then

echo "config.guess found!"

else

echo "config.guess not present! Copying it..."

cp /usr/share/libtool/config.guess ./config.guess

fi


if [ -r ./config.sub ]; then

echo "config.sub found!"

else

echo "config.sub not present! Copying it..."

cp /usr/share/libtool/config.sub ./config.sub

fi


if [ -r ./Makefile ]; then

echo "Makefile found!"

else

echo "Makefile missing! Running ./configure..."

./configure

if [ -r ./Makefile ]; then

echo "Makefile present, ready to compile!"

else

echo "Configure not completed, Makefile still missing! Exiting..."

exit 1

fi

fi


echo "Setting environment variables..."

export sdl_LIBS=""

export LDFLAGS="-framework OpenAL -Wl,-framework,Cocoa -framework SDL -framework Cocoa -lSDLmain

-framework Vorbis -framework Ogg -L/usr/local/lib -L/opt/local/lib -L/sw/lib"

export openal_LIBS="/usr/local/lib/libmikmod.a"


if [ -r ./bin/supertuxkartPPC ]; then

echo "supertuxkartPPC is present, nothing to do."

else

echo "Cleaning up..."

make clean -s


echo "Building PPC Binary..."

make -e -s


echo "Copying PPC Binary..."

mkdir ./bin

if [ -r ./src/supertuxkart ]; then

cp ./src/supertuxkart ./bin/supertuxkartPPC

else

echo "Error!"

exit 1

fi

fi


if [ -r ./bin/supertuxkartx86 ]; then

echo "supertuxkartx86 is present, nothing to do."

else

echo "Cleaning up..."

make clean -s


echo "Building x86 Binary..."

export CXXFLAGS="-g -O2 -Wall -arch i386 `sdl-config --cflags`"

export LDFLAGS=$LDFLAGS" -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -L/usr/local/lib -L/opt/local/lib -L/sw/lib"

make -e -s


echo "Copying x86 Binary..."

if [ -r ./src/supertuxkart ]; then

cp ./src/supertuxkart ./bin/supertuxkartx86

else

echo "Error!"

exit 1

fi

fi


echo "Creating Universal Binary..."

lipo -create ./bin/supertuxkartPPC ./bin/supertuxkartx86 -output ./bin/supertuxkart


echo "Done!"


-------------------------------------------------------------------------------------------------------------

2. Set the permissions for the script "buildUB.sh"

e.g.: chmod 744 buildUB.sh


3. Download SuperTuxKart-Source from "http://sourceforge.net/project/showfiles.php?group_id=202302".


4. Go in the folder SuperTuxKart.


5. Copy the script "buildUB.sh" in the directory "SuperTuxKart".


6. ./buildUB.sh


7. Check-up:


7.01 Go in the folder "bin".


7.02 ./supertuxkart --version

Print version information

e.g.:
-------------------------------------------------------------------------
SuperTuxKart, 0.3alpha.

SuperTuxKart, SVN revision number '975M'.
-------------------------------------------------------------------------


7.03 Is this a universal binary?

file supertuxkart


e.g.:
-------------------------------------------------------------------------
supertuxkart: Mach-O fat file with 2 architectures

supertuxkart (for architecture ppc): Mach-O executable ppc

supertuxkart (for architecture i386): Mach-O executable i386
-------------------------------------------------------------------------


7.04 otool -L supertuxkart

Print shared library dependencies.

e.g.:
------------------------------------------------------------------------------------------------
supertuxkart:

/System/Library/Frameworks/OpenAL.framework/Versions/A/OpenAL (compatibility version 1.0.0,

current version 1.0.0)

/System/Library/Frameworks/Cocoa.framework/Versions/A/Cocoa (compatibility version 1.0.0,

current version 11.0.0)

@executable_path/../Frameworks/SDL.framework/Versions/A/SDL (compatibility version 1.0.0, current

version 1.0.0)

@executable_path/../Frameworks/Vorbis.framework/Versions/A/Vorbis (compatibility version 1.0.0,

current version 1.0.0)

@executable_path/../Frameworks/Ogg.framework/Versions/A/Ogg (compatibility version 1.0.0, current

version 1.0.0)

/System/Library/Frameworks/Carbon.framework/Versions/A/Carbon (compatibility version 2.0.0, current

version 128.0.0)

/System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL (compatibility version 1.0.0, current

version 1.0.0)

/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit (compatibility version 1.0.0, current

version 275.0.0)

/System/Library/Frameworks/AGL.framework/Versions/A/AGL (compatibility version 1.0.0, current version

1.0.0)

/usr/lib/libstdc++.6.dylib (compatibility version 7.0.0, current version 7.4.0)

/usr/lib/libgcc_s.1.dylib (compatibility version 1.0.0, current version 1.0.0)

/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 88.1.8)

(Please post to the development list if you have any problems.)



