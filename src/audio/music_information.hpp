//  $Id: music_information.hpp 1610 2008-03-01 03:18:53Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_MUSIC_INFORMATION_HPP
#define HEADER_MUSIC_INFORMATION_HPP

#include <string>
#include <vector>

class Music;

class MusicInformation
{
private:
    std::string              m_composer;
    std::string              m_title;
    std::string              m_normal_filename;
    std::string              m_fast_filename;
    std::vector<std::string> m_all_tracks;      
    int                      m_numLoops;
    /** If faster music is enabled at all (either separate file or using
     *  the pitch shift approach). */
    bool                     m_enable_fast;    
                                                
    float                    m_gain;
    float                    m_adjustedGain;
    float                    m_faster_time;    // Either time for fading faster
                                               // music in, or time to change pitch
    float                    m_max_pitch;      // maximum pitch for faster music
    static const int         LOOP_FOREVER = -1;
    Music                   *m_normal_music,
                            *m_fast_music;
    enum {SOUND_NORMAL,                        // normal music is played
          SOUND_FADING,                        // normal music fading out, faster fading in
          SOUND_FASTER,                        // change pitch of normal music
          SOUND_FAST}                          // playing faster music or max pitch reached
                             m_mode; 
    float                    m_time_since_faster;

public:
                             MusicInformation (const std::string& filename);
                            ~MusicInformation ();
    const std::string&       getComposer      () const {return m_composer;        }
    const std::string&       getTitle         () const {return m_title;           }
    const std::string&       getNormalFilename() const {return m_normal_filename; }
    const std::string&       getFastFilename  () const {return m_fast_filename;   }
    int                      getNumLoops      () const {return m_numLoops;        }
    float                    getFasterTime    () const {return m_faster_time;     }
    float                    getMaxPitch      () const {return m_max_pitch;       }
    void                     addMusicToTracks ();
    void                     update           (float dt);
    void                     startMusic       ();
    void                     stopMusic        ();
    void                     pauseMusic       ();
    void                     resumeMusic      ();
    void                     volumeMusic      (float gain);
    void                     switchToFastMusic();
};   // MusicInformation
#endif
