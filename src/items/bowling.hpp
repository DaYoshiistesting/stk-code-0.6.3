//  $Id: bowling.hpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs, Marianne Gagnon
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

#ifndef HEADER_BOWLING_H
#define HEADER_BOWLING_H

#include "flyable.hpp"

class Bowling : public Flyable
{
private:
    static float m_st_max_distance;   // maximum distance for a bowling ball to be attracted
    static float m_st_max_distance_squared;
    static float m_st_force_to_target;
    
public:
    Bowling(Kart* kart);
    static  void init(const lisp::Lisp* lisp, ssgEntity* bowling);
    virtual bool updateAndDel(float dt);
    virtual bool hit(Kart* kart, MovingPhysics* mp=NULL);
    
    int getExplosionSound() const { return SFXManager::SOUND_BOWLING_STRIKE; }
    
};   // Bowling

#endif
