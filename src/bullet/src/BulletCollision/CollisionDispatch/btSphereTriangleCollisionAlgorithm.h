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

#ifndef SPHERE_TRIANGLE_COLLISION_ALGORITHM_H
#define SPHERE_TRIANGLE_COLLISION_ALGORITHM_H

#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "BulletCollision/CollisionDispatch/btCollisionCreateFunc.h"
class btPersistentManifold;
#include "btCollisionDispatcher.h"

/// btSphereSphereCollisionAlgorithm  provides sphere-sphere collision detection.
/// Other features are frame-coherency (persistent data) and collision response.
/// Also provides the most basic sample for custom/user btCollisionAlgorithm
class btSphereTriangleCollisionAlgorithm : public btCollisionAlgorithm
{
    bool    m_ownManifold;
    btPersistentManifold*    m_manifoldPtr;
    bool    m_swapped;
    
public:
    btSphereTriangleCollisionAlgorithm(btPersistentManifold* mf,const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* body0,btCollisionObject* body1,bool swapped);

    btSphereTriangleCollisionAlgorithm(const btCollisionAlgorithmConstructionInfo& ci)
        : btCollisionAlgorithm(ci) {}

    virtual void processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut);

    virtual btScalar calculateTimeOfImpact(btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut);

    
    virtual ~btSphereTriangleCollisionAlgorithm();

    struct CreateFunc :public     btCollisionAlgorithmCreateFunc
    {
        
        virtual    btCollisionAlgorithm* CreateCollisionAlgorithm(btCollisionAlgorithmConstructionInfo& ci, btCollisionObject* body0,btCollisionObject* body1)
        {
            
            void* mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(btSphereTriangleCollisionAlgorithm));

            return new(mem) btSphereTriangleCollisionAlgorithm(ci.m_manifold,ci,body0,body1,m_swapped);
        }
    };

};

#endif //SPHERE_TRIANGLE_COLLISION_ALGORITHM_H

