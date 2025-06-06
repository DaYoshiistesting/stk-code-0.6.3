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

#ifndef PERSISTENT_MANIFOLD_H
#define PERSISTENT_MANIFOLD_H


#include "LinearMath/btVector3.h"
#include "LinearMath/btTransform.h"
#include "btManifoldPoint.h"
#include "LinearMath/btAlignedAllocator.h"

struct btCollisionResult;

///contact breaking and merging threshold
extern btScalar gContactBreakingThreshold;

typedef bool (*ContactDestroyedCallback)(void* userPersistentData);
typedef bool (*ContactProcessedCallback)(btManifoldPoint& cp,void* body0,void* body1);
extern ContactDestroyedCallback    gContactDestroyedCallback;




#define MANIFOLD_CACHE_SIZE 4

///btPersistentManifold is a contact point cache, it stays persistent as long as objects are overlapping in the broadphase.
///Those contact points are created by the collision narrow phase.
///The cache can be empty, or hold 1,2,3 or 4 points. Some collision algorithms (GJK) might only add one point at a time.
///updates/refreshes old contact points, and throw them away if necessary (distance becomes too large)
///reduces the cache to 4 points, when more then 4 points are added, using following rules:
///the contact point with deepest penetration is always kept, and it tries to maximuze the area covered by the points
///note that some pairs of objects might have more then one contact manifold.
ATTRIBUTE_ALIGNED16( class) btPersistentManifold 
{

    btManifoldPoint m_pointCache[MANIFOLD_CACHE_SIZE];

    /// this two body pointers can point to the physics rigidbody class.
    /// void* will allow any rigidbody class
    void* m_body0;
    void* m_body1;
    int    m_cachedPoints;

    
    /// sort cached points so most isolated points come first
    int    sortCachedPoints(const btManifoldPoint& pt);

    int        findContactPoint(const btManifoldPoint* unUsed, int numUnused,const btManifoldPoint& pt);

public:

    BT_DECLARE_ALIGNED_ALLOCATOR();

    int m_index1a;

    btPersistentManifold();

    btPersistentManifold(void* body0,void* body1,int bla)
        : m_body0(body0),m_body1(body1),m_cachedPoints(0)
    {
        (void)bla;
    }

    SIMD_FORCE_INLINE void* getBody0() { return m_body0;}
    SIMD_FORCE_INLINE void* getBody1() { return m_body1;}

    SIMD_FORCE_INLINE const void* getBody0() const { return m_body0;}
    SIMD_FORCE_INLINE const void* getBody1() const { return m_body1;}

    void    setBodies(void* body0,void* body1)
    {
        m_body0 = body0;
        m_body1 = body1;
    }

    void clearUserCache(btManifoldPoint& pt);

#ifdef DEBUG_PERSISTENCY
    void    DebugPersistency();
#endif //
    
    SIMD_FORCE_INLINE int    getNumContacts() const { return m_cachedPoints;}

    SIMD_FORCE_INLINE const btManifoldPoint& getContactPoint(int index) const
    {
        btAssert(index < m_cachedPoints);
        return m_pointCache[index];
    }

    SIMD_FORCE_INLINE btManifoldPoint& getContactPoint(int index)
    {
        btAssert(index < m_cachedPoints);
        return m_pointCache[index];
    }

    /// todo: get this margin from the current physics / collision environment
    btScalar    getContactBreakingThreshold() const;
    
    int getCacheEntry(const btManifoldPoint& newPoint) const;

    void AddManifoldPoint( const btManifoldPoint& newPoint);

    void removeContactPoint (int index)
    {
        clearUserCache(m_pointCache[index]);

        int lastUsedIndex = getNumContacts() - 1;
//        m_pointCache[index] = m_pointCache[lastUsedIndex];
        if(index != lastUsedIndex) 
        {
            m_pointCache[index] = m_pointCache[lastUsedIndex]; 
            //get rid of duplicated userPersistentData pointer
            m_pointCache[lastUsedIndex].m_userPersistentData = 0;
            m_pointCache[lastUsedIndex].m_appliedImpulse = 0.f;
            m_pointCache[lastUsedIndex].m_lifeTime = 0;
        }

        btAssert(m_pointCache[lastUsedIndex].m_userPersistentData==0);
        m_cachedPoints--;
    }
    void replaceContactPoint(const btManifoldPoint& newPoint,int insertIndex)
    {
        btAssert(validContactDistance(newPoint));

#define MAINTAIN_PERSISTENCY 1
#ifdef MAINTAIN_PERSISTENCY
        int    lifeTime = m_pointCache[insertIndex].getLifeTime();
        btScalar    appliedImpulse = 0.f;//m_pointCache[insertIndex].m_appliedImpulse;
        btAssert(lifeTime>=0);
        void* cache = m_pointCache[insertIndex].m_userPersistentData;
        
        m_pointCache[insertIndex] = newPoint;

        m_pointCache[insertIndex].m_userPersistentData = cache;
        m_pointCache[insertIndex].m_appliedImpulse = appliedImpulse;
        m_pointCache[insertIndex].m_lifeTime = lifeTime;
#else
        clearUserCache(m_pointCache[insertIndex]);
        m_pointCache[insertIndex] = newPoint;
    
#endif
    }

    bool validContactDistance(const btManifoldPoint& pt) const
    {
        return pt.m_distance1 <= getContactBreakingThreshold();
    }
    /// calculated new worldspace coordinates and depth, and reject points that exceed the collision margin
    void    refreshContactPoints(  const btTransform& trA,const btTransform& trB);

    
    SIMD_FORCE_INLINE    void    clearManifold()
    {
        int i;
        for (i=0;i<m_cachedPoints;i++)
        {
            clearUserCache(m_pointCache[i]);
        }
        m_cachedPoints = 0;
    }



}
;





#endif //PERSISTENT_MANIFOLD_H
