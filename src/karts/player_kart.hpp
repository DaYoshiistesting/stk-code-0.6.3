//  $Id: player_kart.hpp 3086 2009-01-31 04:58:33Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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


#ifndef HEADER_PLAYERKART_H
#define HEADER_PLAYERKART_H

#include "player.hpp"
#include "karts/kart.hpp"

class SFXBase;
class Player;
class Camera;

/** PlayerKart manages control events from the player and moves
    them to the Kart */
class PlayerKart : public Kart
{
private:
    int     m_steer_val, m_steer_val_l, m_steer_val_r;
    int     m_prev_accel;
    bool    m_prev_brake;

    Player *m_player;
    float   m_penalty_time;
    Camera *m_camera;

    SFXBase *m_bzzt_sound;
    SFXBase *m_ugh_sound;
    SFXBase *m_grab_sound;
    SFXBase *m_full_sound;

    void steer(float, int);
public:
                 PlayerKart(const std::string& kart_name,
                            int position, Player *_player,
                            const btTransform& init_pos, int player_index);
                ~PlayerKart        ();
    int          earlyStartPenalty () {return m_penalty_time>0; }
    Player      *getPlayer         () {return m_player;        }
    void         update            (float);
    void         action            (KartAction action, int value);
    void         collectedItem     (const Item *item, int add_info=-1);
    virtual void crashed           (Kart *k);
    virtual void setPosition       (int p);
    virtual void raceFinished      (float time);
    virtual void doingShortcut     ();
    bool         isPlayerKart      () const {return true;}
    Camera*      getCamera         () {return m_camera;}
    void         reset             ();
    void         resetInputState   ();
};

#endif
