//  $Id: homing.cpp 1284 2007-11-08 12:31:54Z hikerstk $
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

#include "items/cake.hpp"

#include <iostream>

#include "karts/kart.hpp"
#include "utils/constants.hpp"

float Cake::m_st_max_distance;
float Cake::m_st_max_distance_squared;
float Cake::m_gravity;

Cake::Cake (Kart *kart) : Flyable(kart, POWERUP_CAKE)
{
    m_target = NULL;
    
    // A bit of a hack: the mass of this kinematic object is still 1.0 
    // (see flyable), which enables collisions. I tried setting 
    // collisionFilterGroup/mask, but still couldn't get this object to 
    // collide with the track. By setting the mass to 1, collisions happen.
    // (if bullet is compiled with _DEBUG, a warning will be printed the first
    // time a homing-track collision happens).
    float y_offset=kart->getKartLength()/2.0f + m_extend.getY()/2.0f;
    
    float z_velocity = m_speed/7.0f;
    
    // give a speed proportional to kart speed.
    m_speed = kart->getSpeed() * m_speed / 23.0f;

    //when going backwards, decrease speed of cake by less.
    if (kart->getSpeed() < 0) m_speed /= 3.6f;

    m_speed += 16.0f;

    if (m_speed < 1.0f) m_speed = 1.0f;

    btTransform trans = kart->getTrans();

	float heading = kart->getHeading();
    float pitch   = kart->getTerrainPitch(heading);

    // find closest kart in front of the current one.
    const bool  backwards = kart->getControls().m_look_back;
    const Kart *closest_kart=NULL;   btVector3 direction;   float kartDistSquared;
    getClosestKart(&closest_kart, &kartDistSquared, &direction, kart /* search in front and behind this kart */, backwards);

    // aim at this kart if 1) it's not too far, 2) if the aimed kart's speed
    // allows the projectile to catch up with it.
    if(closest_kart != NULL && kartDistSquared < m_st_max_distance_squared && m_speed>closest_kart->getSpeed())
    {
        m_target = (Kart*)closest_kart;

        float projectileAngle = 0.0f;
        getLinearKartItemIntersection (kart->getXYZ(), closest_kart,
                                       m_speed, m_gravity, y_offset,
                                       &projectileAngle, &z_velocity);

        // apply transformation to the bullet object (without pitch).
        trans.setRotation(btQuaternion(btVector3(0,0,1), projectileAngle));
        
        m_initial_velocity = btVector3(0.0f, m_speed, z_velocity);
    
            createPhysics(y_offset, m_initial_velocity, 
                  new btCylinderShape(0.5f*m_extend), -m_gravity,
                  true /* rotation */, false /* backwards */, &trans);
    }
    else
    {
        m_target = NULL;
        // kart is too far to be hit. so throw the projectile in a generic way,
        // straight ahead, without trying to hit anything in particular
        trans = kart->getKartHeading(pitch);

        m_initial_velocity = btVector3(0.0f, m_speed, z_velocity);
    
            createPhysics(y_offset, m_initial_velocity, 
                  new btCylinderShape(0.5f*m_extend), -m_gravity,
                  true /* rotation */, backwards, &trans);
    }

    setAdjustZVelocity(false);

    m_body->setActivationState(DISABLE_DEACTIVATION);
    
    m_body->applyTorque(btVector3(5,-3,7));
    
}   // Cake

// -----------------------------------------------------------------------------
void Cake::init(const lisp::Lisp* lisp, ssgEntity *cake_model)
{
    Flyable::init(lisp, cake_model, POWERUP_CAKE);
    m_st_max_distance         = 80.0f;
    m_st_max_distance_squared = 80.0f * 80.0f;
    m_gravity                 = 9.8f;

	if (m_gravity < 0) m_gravity *= -1.0f;

    lisp->get("max-distance",    m_st_max_distance  );
    m_st_max_distance_squared = m_st_max_distance*m_st_max_distance;
}   // init

// -----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or physical object is hit. 
 *  The cake triggers an explosion when hit.
 *  \param kart The kart hit (NULL if no kart was hit).
 *  \param object The object that was hit (NULL if none).
 *  \returns True if there was actually a hit (i.e. not owner, and target is 
 *           not immune), false otherwise.
 */
bool Cake::hit(Kart* kart, MovingPhysics* mp)
{
    bool was_real_hit = Flyable::hit(kart, mp);
    if(was_real_hit)
       hit(kart, mp);

    return was_real_hit;
}   // hit
