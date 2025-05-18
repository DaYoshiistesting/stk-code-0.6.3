//  $Id: moveable.cpp 3027 2009-01-22 12:02:40Z hikerstk $
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

#include "karts/moveable.hpp"

#include "material_manager.hpp"
#include "material.hpp"
#include "user_config.hpp"
#include "karts/player_kart.hpp"
#include "utils/coord.hpp"

Moveable::Moveable()
{
    m_body            = 0;
    m_motion_state    = 0;
    m_first_time      = true ;
    m_model_transform = new ssgTransform();

    m_model_transform->ref();
}   // Moveable

//-----------------------------------------------------------------------------
Moveable::~Moveable()
{
    // The body is being removed from the world in kart/projectile
    if(m_body)         delete m_body;
    if(m_motion_state) delete m_motion_state;
    if(m_model_transform)     m_model_transform->deRef();
    // FIXME LEAK: what about model? ssgDeRefDelete(m_model_transform)
}   // ~Moveable

//-----------------------------------------------------------------------------
void Moveable::updateGraphics(const Vec3& off_xyz, const Vec3& off_hpr)
{
    Vec3 xyz = getXYZ()+off_xyz;
    Vec3 hpr = getHPR()+off_hpr;
    sgCoord c = Coord(xyz, hpr).toSgCoord();

    m_model_transform->setTransform(&c);
}   // updateGraphics

//-----------------------------------------------------------------------------
// The reset position must be set before calling reset
void Moveable::reset()
{
    if(m_body)
    {
        m_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
        m_body->setAngularVelocity(btVector3(0, 0, 0));
        m_body->setCenterOfMassTransform(m_transform);
    }
    Vec3 forw_vec = m_transform.getBasis().getColumn(1);
    m_heading     = atan2f(forw_vec.getY(), forw_vec.getX());
    Vec3 up       = getTrans().getBasis().getColumn(2);
    m_pitch       = -atan2(up.getY(), fabsf(up.getZ()));
    m_roll        = -atan2(up.getX(), up.getZ());
    m_velocityLC  = Vec3(0,0,0);
    Coord c(m_transform);
    m_hpr = c.getHPR();
}   // reset

//-----------------------------------------------------------------------------
void Moveable::update(float dt)
{
    m_motion_state->getWorldTransform(m_transform);
    m_velocityLC  = getVelocity()*getTrans().getBasis();
    m_hpr.setHPR(m_transform.getBasis());
    Vec3 forw_vec = m_transform.getBasis().getColumn(1);
    m_heading     = -atan2f(forw_vec.getX(), forw_vec.getY());

    // The pitch in hpr is in between -pi and pi. But for up-right constraint,
    // it must be restricted to -pi/2 and pi/2 - so recompute it by restricting
    // y to positive values, i.e. no pitch of more than pi/2.
    Vec3 up = getTrans().getBasis().getColumn(2);
    m_pitch = -atan2(up.getY(), (fabsf(up.getZ())));
    m_roll  = -atan2(up.getX(), up.getZ());

#ifdef DEBUG_
    printf("hpr. H = '%f' P = '%f' R = '%f'\nnew: H = '%f' P = '%f' R = '%f'\n",
            getHPR().getHeading(), getHPR().getPitch(), getHPR().getRoll(),
            m_heading, m_pitch, m_roll);
#endif
    updateGraphics(Vec3(0,0,0), Vec3(0,0,0));
    m_first_time  = false ;
}   // update


//-----------------------------------------------------------------------------
void Moveable::createBody(float mass, btTransform& trans,
                          btCollisionShape *shape) {
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    m_transform = trans;
    m_motion_state = new KartMotionState(trans);

    btRigidBody::btRigidBodyConstructionInfo info(mass, m_motion_state, shape, inertia);
    info.m_restitution=0.5f;

    // Then create a rigid body
    // ------------------------
    m_body = new btRigidBody(info);
    // The value of user_pointer must be set from the actual class, otherwise this
    // is only a pointer to moveable, not to (say) kart, and virtual 
    // functions are not called correctly. So only init the pointer to zero.
    m_user_pointer.zero();
    m_body->setUserPointer(&m_user_pointer);
    const btMatrix3x3& basis = m_body->getWorldTransform().getBasis();
    m_hpr.setHPR(basis);
}   // createBody

//-----------------------------------------------------------------------------
/** Places this moveable at a certain location and stores this transform in
 *  this Moveable, so that it can be accessed easily.
 *  \param t New transform for this moveable.
 */
void Moveable::setTrans(const btTransform &t)
{
    m_transform = t;
    m_motion_state->setWorldTransform(t);
}   // setTrans
