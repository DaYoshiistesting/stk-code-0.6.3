/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_DYNAMICS_WORLD_H
#define BT_DYNAMICS_WORLD_H

#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"
class btTypedConstraint;
class btRaycastVehicle;
class btConstraintSolver;

class btDynamicsWorld;
/// Type for the callback for each tick
typedef void (*btInternalTickCallback)(const btDynamicsWorld *world, btScalar timeStep);

enum btDynamicsWorldType
{
    BT_SIMPLE_DYNAMICS_WORLD=1,
    BT_DISCRETE_DYNAMICS_WORLD=2,
    BT_CONTINUOUS_DYNAMICS_WORLD=3
};

///btDynamicsWorld is the baseclass for several dynamics implementation, basic, discrete, parallel, and continuous
class btDynamicsWorld : public btCollisionWorld
{
    public:
        

        btDynamicsWorld(btDispatcher* dispatcher,btBroadphaseInterface* broadphase,btCollisionConfiguration* collisionConfiguration)
        :btCollisionWorld(dispatcher,broadphase,collisionConfiguration), m_internalTickCallback(0)
        {
        }

        virtual ~btDynamicsWorld()
        {
        }
        
        ///stepSimulation proceeds the simulation over timeStep units
        ///if maxSubSteps > 0, it will interpolate time steps
        virtual int        stepSimulation( btScalar timeStep,int maxSubSteps=1, btScalar fixedTimeStep=btScalar(1.)/btScalar(60.))=0;
            
        virtual void    debugDrawWorld() = 0;
                
        virtual void    addConstraint(btTypedConstraint* constraint, bool disableCollisionsBetweenLinkedBodies=false) { (void)constraint;};

        virtual void    removeConstraint(btTypedConstraint* constraint) {(void)constraint;};

        virtual void    addVehicle(btRaycastVehicle* vehicle) {(void)vehicle;};

        virtual void    removeVehicle(btRaycastVehicle* vehicle) {(void)vehicle;};

        //once a rigidbody is added to the dynamics world, it will get this gravity assigned
        //existing rigidbodies in the world get gravity assigned too, during this method
        virtual void    setGravity(const btVector3& gravity) = 0;
        virtual btVector3 getGravity () const = 0;

        virtual void    addRigidBody(btRigidBody* body) = 0;

        virtual void    removeRigidBody(btRigidBody* body) = 0;

        virtual void    setConstraintSolver(btConstraintSolver* solver) = 0;

        virtual btConstraintSolver* getConstraintSolver() = 0;
        
        virtual    int        getNumConstraints() const {    return 0;        }
        
        virtual btTypedConstraint* getConstraint(int index)        {    (void)index;        return 0;        }
        
        virtual const btTypedConstraint* getConstraint(int index) const    {    (void)index;    return 0;    }

        virtual btDynamicsWorldType    getWorldType() const=0;

        virtual void    clearForces() = 0;

        /// Set the callback for when an internal tick (simulation substep) happens
        void setInternalTickCallback(btInternalTickCallback cb) { m_internalTickCallback = cb; }
        
        btInternalTickCallback m_internalTickCallback;


};

#endif //BT_DYNAMICS_WORLD_H


