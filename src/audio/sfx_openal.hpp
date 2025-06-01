//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
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

#ifndef HEADER_SFX_OPENAL_HPP
#define HEADER_SFX_OPENAL_HPP

#include <assert.h>
#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"

class SFXOpenAL : public SFXBase
{
private:
    ALuint       m_soundBuffer;   // Buffers hold sound data.
    ALuint       m_soundSource;   // Sources are points emitting sound.
    bool         m_ok;
    bool         m_positional;
public:
                                  SFXOpenAL(ALuint buffer, bool positional, float rolloff, float gain);
    virtual                      ~SFXOpenAL();
    virtual void                  play();
    virtual void                  loop();
    virtual void                  stop();
    virtual void                  pause();
    virtual void                  resume();
    virtual void                  speed(float factor);
    virtual void                  position(const Vec3 &position);
    virtual SFXManager::SFXStatus getStatus();

};   // SFXOpenAL

#endif // HEADER_SFX_OPENAL_HPP

