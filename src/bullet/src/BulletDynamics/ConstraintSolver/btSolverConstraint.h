

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

#ifndef BT_SOLVER_CONSTRAINT_H
#define BT_SOLVER_CONSTRAINT_H

class    btRigidBody;
#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"

//#define NO_FRICTION_TANGENTIALS 1

///1D constraint along a normal axis between bodyA and bodyB. It can be combined to solve contact and friction constraints.
ATTRIBUTE_ALIGNED16 (struct)    btSolverConstraint
{
    BT_DECLARE_ALIGNED_ALLOCATOR();

    btVector3    m_relpos1CrossNormal;
    btVector3    m_contactNormal;

    btVector3    m_relpos2CrossNormal;
    btVector3    m_angularComponentA;

    btVector3    m_angularComponentB;
    mutable btScalar    m_appliedVelocityImpulse;
    mutable btScalar    m_appliedImpulse;
    int            m_solverBodyIdA;
    int            m_solverBodyIdB;
    
    btScalar    m_friction;
    btScalar    m_restitution;
    btScalar    m_jacDiagABInv;
    btScalar    m_penetration;
    

    
    int            m_constraintType;
    int            m_frictionIndex;
    void*        m_originalContactPoint;
    int            m_unusedPadding[1];


    enum        btSolverConstraintType
    {
        BT_SOLVER_CONTACT_1D = 0,
        BT_SOLVER_FRICTION_1D
    };
};






#endif //BT_SOLVER_CONSTRAINT_H


