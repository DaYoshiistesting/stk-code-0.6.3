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

#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "LinearMath/btScalar.h"

class btCollisionAlgorithm;
struct btBroadphaseProxy;
class btRigidBody;
class    btCollisionObject;
class btOverlappingPairCache;


class btPersistentManifold;
class btStackAlloc;

struct btDispatcherInfo
{
    enum DispatchFunc
    {
        DISPATCH_DISCRETE = 1,
        DISPATCH_CONTINUOUS
    };
    btDispatcherInfo()
        :m_timeStep(btScalar(0.)),
        m_stepCount(0),
        m_dispatchFunc(DISPATCH_DISCRETE),
        m_timeOfImpact(btScalar(1.)),
        m_useContinuous(false),
        m_debugDraw(0),
        m_enableSatConvex(false),
        m_enableSPU(true),
        m_stackAllocator(0)
    {

    }
    btScalar    m_timeStep;
    int        m_stepCount;
    int        m_dispatchFunc;
    btScalar    m_timeOfImpact;
    bool    m_useContinuous;
    class btIDebugDraw*    m_debugDraw;
    bool    m_enableSatConvex;
    bool    m_enableSPU;
    btStackAlloc*    m_stackAllocator;
    
};

/// btDispatcher can be used in combination with broadphase to dispatch overlapping pairs.
/// For example for pairwise collision detection or user callbacks (game logic).
class btDispatcher
{


public:
    virtual ~btDispatcher() ;

    virtual btCollisionAlgorithm* findAlgorithm(btCollisionObject* body0,btCollisionObject* body1,btPersistentManifold* sharedManifold=0) = 0;

    virtual btPersistentManifold*    getNewManifold(void* body0,void* body1)=0;

    virtual void releaseManifold(btPersistentManifold* manifold)=0;

    virtual void clearManifold(btPersistentManifold* manifold)=0;

    virtual bool    needsCollision(btCollisionObject* body0,btCollisionObject* body1) = 0;

    virtual bool    needsResponse(btCollisionObject* body0,btCollisionObject* body1)=0;

    virtual void    dispatchAllCollisionPairs(btOverlappingPairCache* pairCache,btDispatcherInfo& dispatchInfo,btDispatcher* dispatcher)=0;

    virtual int getNumManifolds() const = 0;

    virtual btPersistentManifold* getManifoldByIndexInternal(int index) = 0;

    virtual    btPersistentManifold**    getInternalManifoldPointer() = 0;

    virtual    void* allocateCollisionAlgorithm(int size) = 0;

    virtual    void freeCollisionAlgorithm(void* ptr) = 0;

};


#endif //_DISPATCHER_H
