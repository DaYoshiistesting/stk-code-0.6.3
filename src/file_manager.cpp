//  $Id: file_manager.cpp 4123 2009-10-16 01:29:19Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//            (C) 2008 Steve Baker, Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "file_manager.hpp"

#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#include <string>
#ifdef WIN32
#  include <io.h>
#  include <stdio.h>
#  ifndef __CYGWIN__
#    define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
     //   Some portabilty defines
#    define snprintf       _snprintf
#    define access         _access
#    define                F_OK  04
#  endif
#  define CONFIGDIR       "."
#else
#  define CONFIGDIR       ".supertuxkart"
#endif
// ul.h includes windows.h, so this define is necessary
#define _WINSOCKAPI_

#include <plib/ul.h>
#include "btBulletDynamicsCommon.h"

#include "material_manager.hpp"
#include "utils/string_utils.hpp"

#ifdef __APPLE__
// dynamic data path detection onmac
#  include <CoreFoundation/CoreFoundation.h>

bool macSetBundlePathIfRelevant(std::string& data_dir)
{
    printf("checking whether we are using an app bundle... ");
    // the following code will enable STK to find its data when placed in an app bundle on mac OS X.
    // returns true if path is set, returns false if path was not set
    char path[1024];
    CFBundleRef main_bundle = CFBundleGetMainBundle(); assert(main_bundle);
    CFURLRef main_bundle_URL = CFBundleCopyBundleURL(main_bundle); assert(main_bundle_URL);
    CFStringRef cf_string_ref = CFURLCopyFileSystemPath( main_bundle_URL, kCFURLPOSIXPathStyle); assert(cf_string_ref);
    CFStringGetCString(cf_string_ref, path, 1024, kCFStringEncodingASCII);
    CFRelease(main_bundle_URL);
    CFRelease(cf_string_ref);
    
    std::string contents = std::string(path) + std::string("/Contents");
    if(contents.find(".app") != std::string::npos)
    {
        printf("yes\n");
        // executable is inside an app bundle, use app bundle-relative paths
        data_dir = contents + std::string("/Resources/data");
        return true;
    }
    else
    {
        printf("no\n");
        return false;
    }
}
#endif

FileManager* file_manager = 0;

FileManager::FileManager()
{
    m_is_full_path = false;
    
    if (getenv("SUPERTUXKART_DATADIR") != NULL)
        m_root_dir= getenv("SUPERTUXKART_DATADIR");
#ifdef __APPLE__
    else if( macSetBundlePathIfRelevant( m_root_dir ) ) { /* nothing to do */ }
#endif
    else if(access("data/stk_config.data", F_OK) == 0)
        m_root_dir = "." ;
    else if (access("../data/stk_config.data", F_OK) == 0)
        m_root_dir = ".." ;
    else
#ifdef SUPERTUXKART_DATADIR
        m_root_dir = SUPERTUXKART_DATADIR ;
#else
        m_root_dir = "/usr/local/share/games/supertuxkart" ;
#endif
    // We can't use _() here, since translations will only be initalised
    // after the filemanager (to get the path to the tranlsations from it)
    fprintf(stderr, "Data files will be fetched from: '%s'\n", 
            m_root_dir.c_str() );

    pushTextureSearchPath(m_root_dir+"/data/textures");
    pushModelSearchPath  (m_root_dir+"/data/models"  );
    pushMusicSearchPath  (m_root_dir+"/data/music"   );
    // Add more paths from the STK_MUSIC_PATH environment variable
    if(getenv("SUPERTUXKART_MUSIC_PATH")!=NULL)
    {
        std::string path=getenv("SUPERTUXKART_MUSIC_PATH");
        std::vector<std::string> dirs=StringUtils::split(path,':');
        for(int i=(int)dirs.size()-1; i>=0; i--)
        {
            // Remove '/' at the end of paths, since this can cause
            // problems with windows when using stat()
            while(dirs[i].size()>=1 && dirs[i][dirs[i].size()-1]=='/')
            {
                dirs[i]=dirs[i].substr(0, dirs[i].size()-1);
            }
            // remove empty entries
            if(dirs[i].size()==0)
            {
                dirs.erase(dirs.begin()+i);
                continue;
            }
        }
#ifdef WIN32
        // Handle filenames like d:/dir, which becomes ["d","/dir"]
        for(int i=(int)dirs.size()-1; i>=0; i--)
        {
            if(dirs[i].size()>1) continue;
            if(i==dirs.size()-1)    // last element
            {
                dirs[i]+=":";      // turn "c" back into "c:"
            }
            else
            {
                dirs[i]+=":"+dirs[i+1]; // restore "d:/dir" back 
                dirs.erase(dirs.begin()+i+1);
            }
        }
#endif
        for(int i=0;i<(int)dirs.size(); i++)
            pushMusicSearchPath(dirs[i]);
    }
}  // FileManager

//-----------------------------------------------------------------------------
FileManager::~FileManager()
{
    popMusicSearchPath();
    popModelSearchPath();
    popTextureSearchPath();
}   // ~FileManager

//-----------------------------------------------------------------------------
bool FileManager::findFile(std::string& full_path,
                      const std::string& fname, 
                      const std::vector<std::string>& search_path) const
{
    struct stat mystat;
    
    for(std::vector<std::string>::const_iterator i = search_path.begin();
        i != search_path.end(); ++i)
    {
        //full_path=m_root_dir + "/" + *i + "/" + fname;
        full_path = *i + "/" + fname;
        if(stat(full_path.c_str(), &mystat) >= 0) return true;
    }
    full_path="";
    return false;
}   // findFile

//-----------------------------------------------------------------------------
std::string FileManager::getTextureFile(const std::string& FNAME) const
{
    std::string path;
    findFile(path, FNAME, m_texture_search_path);
    return path;
}   // getTextureFile

//-----------------------------------------------------------------------------
std::string FileManager::getModelFile(const std::string& FNAME) const
{
    std::string path;
    findFile(path, FNAME, m_model_search_path);
    return path;
}   // getModelFile

//-----------------------------------------------------------------------------
std::string FileManager::getTrackDir() const
{
    return m_root_dir+"/data/tracks";
}   // getTrackDir

//-----------------------------------------------------------------------------
std::string FileManager::getKartDir() const
{
    return m_root_dir+"/data/karts";
}   // getKartDir

//-----------------------------------------------------------------------------
std::string FileManager::getChallengeDir() const
{
    return m_root_dir+"/data/challenges";
}   // getChallengeDir

//-----------------------------------------------------------------------------
std::string FileManager::getTranslationDir() const
{
    return m_root_dir+"/data/po";
}   // getTranslationDir

//-----------------------------------------------------------------------------
std::vector<std::string> FileManager::getMusicDirs() const
{
    return m_music_search_path;
}   // getMusicDirs

//-----------------------------------------------------------------------------
std::string FileManager::getTrackFile(const std::string& fname,
                                      const std::string& track_name) const
{
    // tracks file are in data/tracks/TRACKNAME/TRACKNAME.ext
    // but if a track name is supplied use it (which is necessary
    // e.g. to load a model from a track directory
    std::string basename = (track_name!="") ? track_name 
                           : StringUtils::without_extension(fname);
    return getTrackDir()+"/"+basename+"/"+fname;
}   // getTrackFile

//-----------------------------------------------------------------------------
std::string FileManager::getKartFile(const std::string& fname,
                                     const std::string& kart_name) const
{
    // kart file are in data/karts/KARTNAME/KARTNAME.ext
    // but if a kart name is supplied use it (which is necessary
    // e.g. to load a model from a kart directory
    std::string basename = (kart_name!="") ? kart_name 
                           : StringUtils::without_extension(fname);
    return getKartDir()+"/"+basename+"/"+fname;
}   // getKartFile

//-----------------------------------------------------------------------------
std::string FileManager::getConfigFile(const std::string& fname) const
{
    return m_root_dir+"/data/"+fname;
}   // getConfigFile

//-----------------------------------------------------------------------------
std::string FileManager::getChallengeFile(const std::string& fname) const
{
    return getChallengeDir()+"/"+fname;
}   // getChallengeFile

//-----------------------------------------------------------------------------
std::string FileManager::getHomeDir() const
{
    std::string DIRNAME;
#ifdef WIN32
    // Try to use the APPDATA directory to store config files and highscore
    // lists. If not defined, used the current directory.
    std::ostringstream s;
    if(getenv("APPDATA")!=NULL)
    {
        s<<getenv("APPDATA")<<"/supertuxkart/";
        DIRNAME=s.str();
    }
    else DIRNAME=".";
#else
    if(getenv("HOME")!=NULL)
    {
        DIRNAME = getenv("HOME");
    }
    else
    {
        DIRNAME = ".";
    }
    DIRNAME += "/";
    DIRNAME += CONFIGDIR;
#endif
    return DIRNAME;
}   // getHomeDir

//-----------------------------------------------------------------------------
std::string FileManager::getLogFile(const std::string& fname) const
{
    return getHomeDir()+"/"+fname;
}   // getLogFile

//-----------------------------------------------------------------------------
std::string FileManager::getMusicFile(const std::string& fname) const
{
    std::string path;
    findFile(path, fname, m_music_search_path);
    return path;
}   // getMusicFile

//-----------------------------------------------------------------------------
std::string FileManager::getSFXFile(const std::string& fname) const
{
    return m_root_dir+"/data/sfx/"+fname;
}   // getSFXFile
//-----------------------------------------------------------------------------
std::string FileManager::getFontFile(const std::string& fname) const
{
    return m_root_dir+"/data/fonts/"+fname;
}   // getFontFile
//-----------------------------------------------------------------------------
std::string FileManager::getHighscoreFile(const std::string& fname) const
{
    return getHomeDir()+"/"+fname;
}   // getHighscoreFile
//-----------------------------------------------------------------------------
void FileManager::initConfigDir()
{
#ifdef WIN32
    /*nothing*/
#else
    /*if HOME environment variable exists
    create directory $HOME/.supertuxkart*/
    if(getenv("HOME")!=NULL)
    {
        std::string pathname;
        pathname = getenv("HOME");
        pathname += "/.supertuxkart";
        mkdir(pathname.c_str(), 0755);
    }
#endif
}   // initConfigDir

//-----------------------------------------------------------------------------
void FileManager::listFiles(std::set<std::string>& result, const std::string& dir,
                            bool is_full_path, bool make_full_path) const
{
        struct stat mystat;

        // don't list directories with a slash on the end, it'll fail on win32
//        assert(dir[dir.size()-1] != '/');

        result.clear();

        std::string path = is_full_path ? dir : m_root_dir+"/"+dir;

        if(stat(path.c_str(), &mystat) < 0) return;
        if(! S_ISDIR(mystat.st_mode))       return; 

        ulDir* mydir = ulOpenDir(path.c_str());
        if(!mydir) return;

        ulDirEnt* mydirent;
        while( (mydirent = ulReadDir(mydir)) != 0)
        {
            result.insert(make_full_path ? path+"/"+mydirent->d_name
                                         :          mydirent->d_name);
        }
        ulCloseDir(mydir);
}   // listFiles
