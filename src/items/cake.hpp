//  $Id: missile.hpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
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

#ifndef HEADER_CAKE_H
#define HEADER_CAKE_H

#include "flyable.hpp"

class Cake : public Flyable
{
private:
    static float m_st_max_distance;    // maximum distance for a missile to be attracted
    static float m_st_max_distance_squared;
    static float m_gravity;

    btVector3    m_initial_velocity;
    Kart*        m_target;            // which kart is targeted by this
                                      // projectile (NULL if none)
public:
    Cake (Kart *kart);
    static  void init     (const lisp::Lisp* lisp, ssgEntity* cake_model);
    virtual bool hit      (Kart *kart, MovingPhysics *mp=NULL);
    virtual void hitTrack ()                      {hit(NULL);                }
    // Kinematic objects are not allowed to have a velocity (assertion in 
    // bullet), so we have to do our own velocity handling here
    virtual const btVector3 &getVelocity() const  {return m_initial_velocity;}
    virtual void  setVelocity(const btVector3& v) {m_initial_velocity=v;     }

};   // Cake

#endif
