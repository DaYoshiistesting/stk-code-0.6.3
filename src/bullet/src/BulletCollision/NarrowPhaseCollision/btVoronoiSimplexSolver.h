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



#ifndef btVoronoiSimplexSolver_H
#define btVoronoiSimplexSolver_H

#include "btSimplexSolverInterface.h"



#define VORONOI_SIMPLEX_MAX_VERTS 5

struct btUsageBitfield{
    btUsageBitfield()
    {
        reset();
    }

    void reset()
    {
        usedVertexA = false;
        usedVertexB = false;
        usedVertexC = false;
        usedVertexD = false;
    }
    unsigned short usedVertexA    : 1;
    unsigned short usedVertexB    : 1;
    unsigned short usedVertexC    : 1;
    unsigned short usedVertexD    : 1;
    unsigned short unused1        : 1;
    unsigned short unused2        : 1;
    unsigned short unused3        : 1;
    unsigned short unused4        : 1;
};


struct    btSubSimplexClosestResult
{
    btPoint3    m_closestPointOnSimplex;
    //MASK for m_usedVertices
    //stores the simplex vertex-usage, using the MASK, 
    // if m_usedVertices & MASK then the related vertex is used
    btUsageBitfield    m_usedVertices;
    btScalar    m_barycentricCoords[4];
    bool m_degenerate;

    void    reset()
    {
        m_degenerate = false;
        setBarycentricCoordinates();
        m_usedVertices.reset();
    }
    bool    isValid()
    {
        bool valid = (m_barycentricCoords[0] >= btScalar(0.)) &&
            (m_barycentricCoords[1] >= btScalar(0.)) &&
            (m_barycentricCoords[2] >= btScalar(0.)) &&
            (m_barycentricCoords[3] >= btScalar(0.));


        return valid;
    }
    void    setBarycentricCoordinates(btScalar a=btScalar(0.),btScalar b=btScalar(0.),btScalar c=btScalar(0.),btScalar d=btScalar(0.))
    {
        m_barycentricCoords[0] = a;
        m_barycentricCoords[1] = b;
        m_barycentricCoords[2] = c;
        m_barycentricCoords[3] = d;
    }

};

/// btVoronoiSimplexSolver is an implementation of the closest point distance algorithm from a 1-4 points simplex to the origin.
/// Can be used with GJK, as an alternative to Johnson distance algorithm.
#ifdef NO_VIRTUAL_INTERFACE
class btVoronoiSimplexSolver
#else
class btVoronoiSimplexSolver : public btSimplexSolverInterface
#endif
{
public:

    int    m_numVertices;

    btVector3    m_simplexVectorW[VORONOI_SIMPLEX_MAX_VERTS];
    btPoint3    m_simplexPointsP[VORONOI_SIMPLEX_MAX_VERTS];
    btPoint3    m_simplexPointsQ[VORONOI_SIMPLEX_MAX_VERTS];

    

    btPoint3    m_cachedP1;
    btPoint3    m_cachedP2;
    btVector3    m_cachedV;
    btVector3    m_lastW;
    bool        m_cachedValidClosest;

    btSubSimplexClosestResult m_cachedBC;

    bool    m_needsUpdate;
    
    void    removeVertex(int index);
    void    reduceVertices (const btUsageBitfield& usedVerts);
    bool    updateClosestVectorAndPoints();

    bool    closestPtPointTetrahedron(const btPoint3& p, const btPoint3& a, const btPoint3& b, const btPoint3& c, const btPoint3& d, btSubSimplexClosestResult& finalResult);
    int        pointOutsideOfPlane(const btPoint3& p, const btPoint3& a, const btPoint3& b, const btPoint3& c, const btPoint3& d);
    bool    closestPtPointTriangle(const btPoint3& p, const btPoint3& a, const btPoint3& b, const btPoint3& c,btSubSimplexClosestResult& result);

public:

     void reset();

     void addVertex(const btVector3& w, const btPoint3& p, const btPoint3& q);


     bool closest(btVector3& v);

     btScalar maxVertex();

     bool fullSimplex() const
     {
         return (m_numVertices == 4);
     }

     int getSimplex(btPoint3 *pBuf, btPoint3 *qBuf, btVector3 *yBuf) const;

     bool inSimplex(const btVector3& w);
    
     void backup_closest(btVector3& v) ;

     bool emptySimplex() const ;

     void compute_points(btPoint3& p1, btPoint3& p2) ;

     int numVertices() const 
     {
         return m_numVertices;
     }


};

#endif //VoronoiSimplexSolver
