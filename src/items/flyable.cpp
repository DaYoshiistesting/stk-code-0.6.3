//  $Id: flyable.cpp 1284 2007-11-08 12:31:54Z hikerstk $
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

#include "items/flyable.hpp"

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#  define isnan _isnan
#endif
#include <math.h>

#include "callback_manager.hpp"
#include "race_manager.hpp"
#include "graphics/scene.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "modes/linear_world.hpp"
#include "network/flyable_info.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/ssg_help.hpp"

// static variables:
float      Flyable::m_st_speed[POWERUP_MAX];
ssgEntity* Flyable::m_st_model[POWERUP_MAX];
float      Flyable::m_st_min_height[POWERUP_MAX];
float      Flyable::m_st_max_height[POWERUP_MAX];
float      Flyable::m_st_force_updown[POWERUP_MAX];
btVector3  Flyable::m_st_extend[POWERUP_MAX];
// ----------------------------------------------------------------------------

Flyable::Flyable(Kart *kart, PowerupType type, float mass) : Moveable()
{
    // get the appropriate data from the static fields
    m_speed             = m_st_speed[type];
    m_extend            = m_st_extend[type];
    m_max_height        = m_st_max_height[type];
    m_min_height        = m_st_min_height[type];
    m_average_height    = (m_min_height+m_max_height)/2.0f;
    m_force_updown      = m_st_force_updown[type];
    m_owner             = kart;
    m_has_hit_something = false;
    m_exploded          = false;
    m_shape             = NULL;
    m_mass              = mass;
    m_adjust_z_velocity = true;
    do_terrain_info     = true;
    m_time_since_thrown = 0;
    m_owner_has_temporary_immunity = true;
    m_max_lifespan = -1;
	
    // Add the graphical model
    ssgTransform *m     = getModelTransform();
    m->addKid(m_st_model[type]);
    scene->add(m);
}   // Flyable
// ----------------------------------------------------------------------------
/** Creates a bullet physics body for the flyable item.
 *  \param y_offset How far ahead of the kart the flyable should be 
 *         positioned. Necessary to avoid exploding a rocket inside of the
 *         firing kart.
 *  \param velocity Initial velocity of the flyable.
 *  \param shape Collision shape of the flyable.
 *  \param gravity Gravity to use for this flyable.
 *  \param rotates True if the item should rotate, otherwise the angular factor
 *         is set to 0 preventing rotations from happening.
 *  \param turn_around True if the item is fired backwards.
 *  \param customDirection If defined the initial heading for this item, 
 *         otherwise the kart's heading will be used.
 */
void Flyable::createPhysics(float y_offset, const btVector3 &velocity,
                            btCollisionShape *shape, const float gravity,
                            const bool rotates, const bool turn_around, 
                            const btTransform* customDirection)
{
    // Get Kart heading direction
    btTransform trans = ( customDirection == NULL ? m_owner->getKartHeading() : *customDirection );

    // Apply offset
    btTransform offset_transform;
    offset_transform.setIdentity();
    btVector3 offset = Vec3(0,y_offset,m_average_height);
    offset_transform.setOrigin(offset);
        
    // turn around
    if(turn_around)
    {
        btTransform turn_around_trans;
     // turn_around_trans.setOrigin(trans.getOrigin());
        turn_around_trans.setIdentity();
        turn_around_trans.setRotation(btQuaternion(btVector3(0, 0, 1), PI));
        trans  *= turn_around_trans;
    }
    
    trans  *= offset_transform;

    m_shape = shape;
    createBody(m_mass, trans, m_shape);
    m_user_pointer.set(this);
    RaceManager::getWorld()->getPhysics()->addBody(getBody());

    m_body->setGravity(btVector3(0.0f, 0.0f, gravity));

    // Rotate velocity to point in the right direction
    btVector3 v=trans.getBasis()*velocity;

    if(m_mass!=0.0f)  // Don't set velocity for kinematic or static objects
    {
        m_body->setLinearVelocity(v);
        if(!rotates) m_body->setAngularFactor(0.0f);   // prevent rotations
    }
    m_body->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

}   // createPhysics
// -----------------------------------------------------------------------------
void Flyable::init(const lisp::Lisp* lisp, ssgEntity *model, 
                   PowerupType type)
{
    m_st_speed[type]        = 25.0f;
    m_st_max_height[type]   = 1.0f;
    m_st_min_height[type]   = 3.0f;
    m_st_force_updown[type] = 15.0f;
    lisp->get("speed",           m_st_speed[type]       );
    lisp->get("min-height",      m_st_min_height[type]  );
    lisp->get("max-height",      m_st_max_height[type]  );
    lisp->get("force-updown",    m_st_force_updown[type]);

    // Store the size of the model
    Vec3 min, max;
    
    SSGHelp::MinMax(model, &min, &max);
    m_st_extend[type] = btVector3(max-min);
    m_st_model[type]  = model;
}   // init

// ----------------------------------------------------------------------------
Flyable::~Flyable()
{
    if(m_shape) delete m_shape;
    RaceManager::getWorld()->getPhysics()->removeBody(getBody());
}   // ~Flyable

// ----------------------------------------------------------------------------
void Flyable::getClosestKart(const Kart **minKart, float *minDistSquared,
                             btVector3 *minDelta, const Kart* inFrontOf, 
                             const bool backwards) const
{
    btTransform tProjectile = (inFrontOf != NULL ? inFrontOf->getTrans() : getTrans());
    
    *minDistSquared = -1.0f;
    *minKart = NULL;
    
    for(unsigned int i=0 ; i<race_manager->getNumKarts(); i++ )
    {
        Kart *kart = RaceManager::getKart(i);
        if(kart->isEliminated() || kart == m_owner || (!kart->isOnGround()) ) continue;
        btTransform t=kart->getTrans();
       
        btVector3 delta = t.getOrigin()-tProjectile.getOrigin();
        float distance2 = delta.length2();
        
        if(inFrontOf != NULL)
        {
            // Ignore karts behind the current one
            btVector3 to_target  = kart->getXYZ() - inFrontOf->getXYZ();
            const float distance = to_target.length();
            if(distance > 50) continue; // kart too far, don't aim at it
            
            btTransform trans = inFrontOf->getTrans();
            // get heading=trans.getBasis*(0,1,0) ... so save the multiplication:
            btVector3 direction(trans.getBasis().getColumn(1));
            const float angle = to_target.angle( backwards ? -direction : direction );
            if(fabsf(angle)>1) continue;
        }
        
        if(distance2 < *minDistSquared || *minDistSquared < 0 /* not yet set */)
        {
            *minDistSquared = distance2;
            *minKart        = kart;
            *minDelta       = delta;
        }
    }  // for i<getNumKarts
    
}   // getClosestKart
// ----------------------------------------------------------------------------
/** Returns information on the parameters needed to hit a target kart moving 
 *  at constant velocity and direction for a given speed in the XY-plane.
 *  \param origin Location of the kart shooting the item.
 *  \param target Which kart to target.
 *  \param item_xy_speed Speed of the item projected in XY plane.
 *  \param gravity The gravity used for this item.
 *  \param y_offset How far ahead of the kart the item is shot (so that
 *         the item does not originate inside of the shooting kart.
 *  \param projectileAngle Returns the angle to fire the item at.
 *  \param z_velocity Returns the upwards velocity to use for the item.
 */
void Flyable::getLinearKartItemIntersection (const Vec3 &origin, 
                                             const Kart *target,
                                             float item_XY_speed, 
                                             float gravity, 
                                             float y_offset,
                                             float *projectileAngle, 
                                             float *z_velocity)
{
    Vec3 relative_target_kart_loc = target->getXYZ() - origin;

    btTransform trans = target->getTrans();
    Vec3 target_direction(trans.getBasis().getColumn(1));

    float dx = relative_target_kart_loc.getX();
    float dy = relative_target_kart_loc.getY();
    float dz = relative_target_kart_loc.getZ();

    float gz = target_direction.getZ();

    //Projected onto X-Y plane
    float target_kart_speed = target_direction.length_2d() * target->getSpeed();

    float target_kart_heading = target->getHeading();

    float dist = -(target_kart_speed / item_XY_speed) * (dx * cosf(target_kart_heading) +
                                                         dy * sinf(target_kart_heading));

    float fire_th = (dx*dist - dy * sqrtf(dx*dx + dy*dy - dist*dist)) / (dx*dx + dy*dy);
    fire_th = (((dist - dx*fire_th) / dy < 0) ? -acosf(fire_th): acosf(fire_th));

    float time = 0.0f;
    float a = item_XY_speed * sinf (fire_th) + target_kart_speed * sinf (target_kart_heading);
    float b = item_XY_speed * cosf (fire_th) + target_kart_speed * cosf (target_kart_heading);

    if (fabsf(a) > fabsf(b)) 
		time = fabsf (dx / a);
    else if (b != 0.0f)      
		time = fabsf(dy / b);

    if (fire_th > PI)
        fire_th -= PI;
    else
        fire_th += PI;

    //createPhysics offset
    time -= y_offset / sqrt(a*a+b*b);

    *projectileAngle = fire_th;
    *z_velocity = (0.5f * time * gravity) + (dz / time) + (gz * target->getSpeed());

}   // getLinearKartItemIntersection
// ----------------------------------------------------------------------------
bool Flyable::updateAndDel(float dt)
{
    m_time_since_thrown += dt;
    if(m_max_lifespan > -1 && m_time_since_thrown > m_max_lifespan) hit(NULL);
	
    if(m_exploded) return false;
    if(m_has_hit_something) return true;
	
    Vec3 pos = getXYZ();
    // Check if the flyable is out of the track  boundary. If so, let it explode.
    Vec3 min, max;
    RaceManager::getTrack()->getAABB(&min, &max);

	// I have seen that the bullet AABB can be slightly different from the 
    // one computed here - I assume due to minor floating point errors
    // (e.g. 308.25842 instead of 308.25845). To avoid a crash with a bullet
    // assertion (see bug 3058932) I add an epsilon here - but admittedly
    // that does not really explain the bullet crash, since bullet tests
    // against its own AABB, and should therefore not cause the assertion.
    // But since we couldn't reproduce the problem, and the epsilon used
    // here does not hurt, I'll leave it in.
    float eps = 0.1f;
    assert(!isnan(pos.getX()));
    assert(!isnan(pos.getY()));
    assert(!isnan(pos.getZ()));
    if(pos[0]<(min)[0]+eps || pos[1]<(min)[1]+eps || pos[2]<(min)[2]+eps ||
       pos[0]>(max)[0]-eps || pos[1]>(max)[1]-eps || pos[2]>(max)[2]-eps   )   
    {
        hit(NULL);    // flyable out of track boundary
        return true;
    }
    if(do_terrain_info) 
        TerrainInfo::update(pos);

    if(m_adjust_z_velocity)
    {
        float hat = pos.getZ()-getHoT();
        // Use the Height Above Terrain to set the Z velocity.
        // HAT is clamped by min/max height. This might be somewhat
        // unphysical, but feels right in the game.
        float delta = m_average_height - std::max(std::min(hat, m_max_height), m_min_height);
        Vec3 v = getVelocity();
        float heading = atan2f(v.getX(), v.getY());
        float pitch   = getTerrainPitch(heading);
        float vel_up = m_force_updown*(delta);
        if (hat < m_max_height) // take into account pitch of surface
            vel_up += v.length_2d()*tanf(pitch);
        v.setZ(vel_up);
        setVelocity(v);
    }   // if m_adjust_z_velocity

    Moveable::update(dt);
    return false;
}   // update

// -----------------------------------------------------------------------------
/** Updates the position of a projectile based on information received from the
 *  server. 
 */
void Flyable::updateFromServer(const FlyableInfo &f, float dt)
{
    setXYZ(f.m_xyz);
    setRotation(f.m_rotation);
    // m_exploded is not set here, since otherwise when explode() is called,
    // the rocket is considered to be already exploded.
    // Update the graphical position
    Moveable::update(dt);
}   // updateFromServer

// -----------------------------------------------------------------------------
/** Returns true if the item hit the kart who shot it (to avoid that an item
 *  that's too close to the shoter hits the shoter).
 *  \param kart Kart who was hit.
 */
bool Flyable::isOwnerImmunity(const Kart* kart_hit) const
{
    return m_owner_has_temporary_immunity && 
           kart_hit == m_owner            && 
           m_time_since_thrown < 2.0f;
}   // isOwnerImmunity

// -----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or physical object is hit. 
 *  \param kart The kart hit (NULL if no kart was hit).
 *  \param object The object that was hit (NULL if none).
 *  \return True if there was actually a hit (i.e. not owner, and target is 
 *          not immune), false otherwise.
 */
bool Flyable::hit(Kart *kart_hit, MovingPhysics *mp)
{
    // the owner of this flyable should not be hit by his own flyable
    if(m_exploded || isOwnerImmunity(kart_hit)) return false;

    m_has_hit_something=true;

    // Notify the projectile manager that this rocket has hit something.
    // The manager will create the appropriate explosion object.
    projectile_manager->notifyRemove();

    // Now remove this projectile from the graph:
    ssgTransform *m = getModelTransform();
    m->removeAllKids();
    scene->remove(m);

    // The explosion is a bit higher in the air
    Vec3 pos_explosion=getXYZ();
    pos_explosion.setZ(pos_explosion.getZ()+1.2f);
    pos_explosion.setY(pos_explosion.getY()+3.2f);
    RaceManager::getWorld()->getPhysics()->removeBody(getBody());
    m_exploded=true;

    if(!needsExplosion()) return false;

    // Apply explosion effect
    // ----------------------
    for ( unsigned int i = 0 ; i < race_manager->getNumKarts() ; i++ )
    {
        Kart *kart = RaceManager::getKart(i);
        // Handle the actual explosion. The kart that fired a flyable will 
        // only be affected if it's a direct hit. This allows karts to use
        // rockets on short distance.
        if(m_owner!=kart || m_owner==kart_hit) 
        {
            // Set a flag it if was a direct hit.
            kart->handleExplosion(getXYZ(), kart==kart_hit);
            if(kart==kart_hit && RaceManager::getTrack()->isArena())
            {
                RaceManager::getWorld()->kartHit(kart->getWorldKartId());
            }
        }
    }
    callback_manager->handleExplosion(pos_explosion, mp);
    return true;
}

/* EOF */
