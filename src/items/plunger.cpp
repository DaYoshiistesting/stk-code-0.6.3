//  $Id: missile.cpp 1284 2007-11-08 12:31:54Z hikerstk $
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

#include "items/plunger.hpp"

#include "race_manager.hpp"
#include "graphics/scene.hpp"
#include "items/rubber_band.hpp"
#include "items/projectile_manager.hpp"
#include "karts/player_kart.hpp"
#include "modes/world.hpp"
#include "physics/moving_physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

// -----------------------------------------------------------------------------
Plunger::Plunger(Kart *kart) : Flyable(kart, POWERUP_PLUNGER)
{
    const float gravity = 0.0f;

    float y_offset = 0.5f*kart->getKartLength()+0.5f*m_extend.getY();
    float up_velocity = 0.0f;
    float plunger_speed = 2 * m_speed;
    
    // if the kart is looking backwards, release from the back
    m_reverse_mode = kart->getControls().m_look_back;
    
    // find closest kart in front of the current one
    const Kart *closest_kart=0;   btVector3 direction;   float kartDistSquared;
    getClosestKart(&closest_kart, &kartDistSquared, &direction, kart /* search in front of this kart */, m_reverse_mode);
    
    btTransform trans = kart->getKartHeading();
    btMatrix3x3 kart_rotation = trans.getBasis();
    
    // The current y vector is rotation*(0,1,0), or:
    btVector3 y(kart_rotation.getColumn(1));

    float heading = kart->getHeading();
    float pitch   = kart->getTerrainPitch(heading);

    // aim at this kart if it's not too far
    if(closest_kart != NULL && kartDistSquared < 30*30)
    {
        float projectileAngle = 0.0f;
        getLinearKartItemIntersection (kart->getXYZ(), closest_kart,
                                       plunger_speed, gravity, y_offset,
                                       &projectileAngle, &up_velocity);

        btTransform trans = kart->getTrans();

        // apply transformation to the bullet object (without pitch)
        trans.setRotation(btQuaternion(btVector3(0,0,1), projectileAngle));

        m_initial_velocity = btVector3(0.0f, plunger_speed, up_velocity);

        createPhysics(y_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend), gravity, false /* rotates */, false, &trans );
    }
    else
    {
        createPhysics(y_offset, btVector3(pitch, plunger_speed, 0.0f),
                      new btCylinderShape(0.5f*m_extend), gravity, false /* rotates */, m_reverse_mode, &trans );
    }
    
	//adjust height according to terrain
    setAdjustZVelocity(false);

    // pulling back makes no sense in battle mode, since this mode is not a race.
    // so in battle mode, always hide view
    if( m_reverse_mode || race_manager->isBattleMode(race_manager->getMinorMode()) )
        m_rubber_band = NULL;
    else
    {
        m_rubber_band = new RubberBand(this, *kart);
        m_rubber_band->ref();
    }
    m_keep_alive = -1;
}   // Plunger

// -----------------------------------------------------------------------------
Plunger::~Plunger()
{
    m_rubber_band->removeFromScene();
    ssgDeRefDelete(m_rubber_band);
}   // ~Plunger

// -----------------------------------------------------------------------------
void Plunger::init(const lisp::Lisp* lisp, ssgEntity *plunger_model)
{
    Flyable::init(lisp, plunger_model, POWERUP_PLUNGER);
}   // init

// -----------------------------------------------------------------------------
bool Plunger::updateAndDel(float dt)
{
    // In keep-alive mode, just update the rubber band
    if(m_keep_alive >= 0)
    {
        m_keep_alive -= dt;
        if(m_keep_alive<=0)
        {
            setHasHit();
            projectile_manager->notifyRemove();
            ssgTransform *m = getModelTransform();
            m->removeAllKids();
            scene->remove(m);
			return true;
        }
        if(m_rubber_band != NULL) m_rubber_band->update(dt);
        return false;
    }

    // Else: update the flyable and rubber band
    bool ret = Flyable::updateAndDel(dt);
    if(m_rubber_band != NULL) m_rubber_band->update(dt);
    return ret;
}   // updateAndDel

// -----------------------------------------------------------------------------
/** Virtual function called when the plunger hits something.
 *  The plunger is special in that it is not deleted when hitting an object.
 *  Instead it stays around (though not as a graphical or physical object)
 *  till the rubber band expires.
 *  \param kart Pointer to the kart hit (NULL if not a kart).
 *  \param mp  Pointer to MovingPhysics object if hit (NULL otherwise).
 */
bool Plunger::hit(Kart *kart, MovingPhysics *mp)
{
    if(isOwnerImmunity(kart)) return false;

    // pulling back makes no sense in battle mode, since this mode is not a race.
    // so in battle mode, always hide view
    if( m_reverse_mode || race_manager->isBattleMode(race_manager->getMinorMode()) )
    {
        if(kart) kart->blockViewWithPlunger();

        m_keep_alive = 0;
        // Make this object invisible by placing it faaar down. Note that if this
        // objects is simply removed from the scene graph, it might be auto-deleted
        // because the ref count reaches zero.
        Vec3 hell(0, 0, -10000);
        getModelTransform()->setTransform(hell.toFloat());
        RaceManager::getWorld()->getPhysics()->removeBody(getBody());
    }
    else
    {
        m_keep_alive = m_owner->getKartProperties()->getRubberBandDuration();

        // Make this object invisible by placing it faaar down. Note that if this
        // objects is simply removed from the scene graph, it might be auto-deleted
        // because the ref count reaches zero.
        Vec3 hell(0, 0, -10000);
        getModelTransform()->setTransform(hell.toFloat());
        RaceManager::getWorld()->getPhysics()->removeBody(getBody());
        
        if(kart)
        {
            m_rubber_band->hit(kart);
            return false;
        }
        else if(mp)
        {
            Vec3 pos(mp->getBody()->getWorldTransform().getOrigin());
            m_rubber_band->hit(NULL, &pos);
        }
        else
        {
            m_rubber_band->hit(NULL, &(getXYZ()));
        }
    }

	// Ruberband attached.
	return false;
}   // hit

// -----------------------------------------------------------------------------
/** Called when the plunger hits the track. In this case, notify the rubber
 *  band, and remove the plunger (but keep it alive). 
 */
void Plunger::hitTrack()
{
    hit(NULL, NULL);
}   // hitTrack

// -----------------------------------------------------------------------------
