/*
 * Copyright (c) 2005 Erwin Coumans http://continuousphysics.com/Bullet/
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies.
 * Erwin Coumans makes no representations about the suitability 
 * of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
*/
#ifndef HEADER_BT_KART_HPP
#define HEADER_BT_KART_HPP

#include "btBulletDynamicsCommon.h"

class btDynamicsWorld;
struct btWheelInfo;
/** The btKart is a raycast vehicle, that does not skid. It therefore solves
 *  the problems with the plain bullet physics that karts would often rotate
 *  on a spot if one of the wheels loses contact with the ground.
 */
class btKart : public btRaycastVehicle
{
    void         defaultInit(const btVehicleTuning& tuning);
    btScalar     m_track_connect_accel;
    int          m_num_wheels_on_ground;
    btScalar     m_skidding_factor;
    bool         m_zipper_active;
    btScalar     m_zipper_velocity;
public:
                 btKart(const btVehicleTuning& tuning,btRigidBody* chassis,    
                        btVehicleRaycaster* raycaster, float track_connect_accel );
    virtual     ~btKart() ;
    btScalar     rayCast(btWheelInfo& wheel);
    btScalar     rayCast(btWheelInfo& wheel, const btVector3& ray);
    bool         projectVehicleToSurface(const btVector3& ray, bool translate_vehicle);
    void         setSkidding(btScalar sf)     { m_skidding_factor = sf; }
    virtual void updateVehicle(btScalar step);
    void         resetSuspension();
    int          getNumWheelsOnGround() const { return m_num_wheels_on_ground; }
    void         setRaycastWheelInfo(int wheelIndex , bool isInContact, 
                                     const btVector3& hitPoint, 
                                     const btVector3& hitNormal,btScalar depth);
    void         setPitchControl(btScalar pitch) { m_pitchControl = pitch; }
    void         activateZipper(btScalar vel) { m_zipper_active = true; m_zipper_velocity = vel; }
    void         deactivateZipper() { m_zipper_active = false; }
    void         updateSuspension(btScalar deltaTime);
    virtual void updateFriction(btScalar timeStep);
};

#endif //BT_KART_H

