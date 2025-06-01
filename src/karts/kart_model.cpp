//  $Id: kart_model.hpp 2400 2008-10-30 02:02:56Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "karts/kart_model.hpp"

#include "loader.hpp"
#include "stk_config.hpp"
#include "user_config.hpp"
#include "utils/constants.hpp"
#include "utils/ssg_help.hpp"

float KartModel::UNDEFINED = -99.9f;

/** The constructor reads the model file name and wheel specification from the
 *  kart config file.
 */
KartModel::KartModel()
{
    for(unsigned int i=0; i<4; i++)
    {
        m_wheel_graphics_position[i]     = Vec3(UNDEFINED);
        m_wheel_physics_position[i]      = Vec3(UNDEFINED);
        // For kart without separate wheels.
        m_wheel_graphics_radius[i]       = 0.0f;   
        m_wheel_model[i]                 = NULL;
        // Default value for kart suspensions. 
        // Move to config file later if we find each kart needs custom values.
        m_min_suspension[i]              = -1.3f;
        m_max_suspension[i]              =  1.3f;
        m_dampen_suspension_amplitude[i] = 2.5f;
    }
    m_wheel_filename[0] = "wheel-front-right.ac";
    m_wheel_filename[1] = "wheel-front-left.ac";
    m_wheel_filename[2] = "wheel-rear-right.ac";
    m_wheel_filename[3] = "wheel-rear-left.ac";

    m_root = NULL;
}   // KartModel

// ----------------------------------------------------------------------------
/** This function loads the information about the kart from a lisp file. It 
 *  does not actually load the models (see load()).
 *  \param lisp  Lisp object of configuration file.
 */
void KartModel::loadInfo(const lisp::Lisp* lisp)
{
    lisp->get("model-file", m_model_filename);
    //lisp->get("christ-offset", m_christ_offset);
    loadWheelInfo(lisp, "wheel-front-right", 0);
    loadWheelInfo(lisp, "wheel-front-left",  1);
    loadWheelInfo(lisp, "wheel-rear-right",  2);
    loadWheelInfo(lisp, "wheel-rear-left",   3);
}   // init
// ----------------------------------------------------------------------------
/** Destructor.
 */
KartModel::~KartModel()
{
    // This automatically frees the wheels and the kart model.
    // m_root can be zero in case of STKConfig, which has a kart_properties
    // attribute (for the default values) as well.
    if(m_root) m_root->removeAllKids();
    ssgDeRefDelete(m_root);
}  // ~KartModel

// ----------------------------------------------------------------------------
/** Loads the 3d model and all wheels.
 */
void KartModel::loadModels()
{
    ssgEntity *obj = loader->load(m_model_filename, CB_KART, false);
    if(!obj)
    {
        fprintf(stderr, "Can't find kart model '%s'.\n",m_model_filename.c_str());
        return;
    }
    m_root = new ssgTransform();
    m_root->ref();
    m_root->addKid(obj);
    ssgStripify(obj);
    Vec3 min, max;
    SSGHelp::MinMax(obj, &min, &max);
    m_z_offset    = min.getZ();
    m_kart_width  = max.getX()-min.getX();
    m_kart_length = max.getY()-min.getY();
    m_kart_height = max.getZ()-min.getZ();
    sgVec3 move_kart_to_0_z;
    sgSetVec3(move_kart_to_0_z, 0, 0, m_z_offset);
    m_root->setTransform(move_kart_to_0_z);

    // Now set default some default parameters (if not defined) that 
    // depend on the size of the kart model (wheel position, center
    // of gravity shift)
    for(unsigned int i=0; i<4; i++)
    {
        if(m_wheel_graphics_position[i].getX()==UNDEFINED)
        {
            m_wheel_graphics_position[i].setX((i==1||i==3) 
                                               ? -0.5f*m_kart_width
                                               :  0.5f*m_kart_width);
            m_wheel_graphics_position[i].setY((i<2) 
                                               ?  0.5f*m_kart_length
                                               : -0.5f*m_kart_length);
            m_wheel_graphics_position[i].setZ(0);
        }
    }

    // Load the wheel models. This can't be done early, since the default
    // values for the graphical position must be defined, which in turn
    // depend on the size of the model.
    for(unsigned int i=0; i<4; i++)
    {
        m_wheel_model[i] = loader->load(m_wheel_filename[i], CB_KART);
        m_wheel_transform[i] = new ssgTransform();
#ifdef DEBUG
        m_wheel_transform[i]->setName("wheeltransform");
#endif
        if(m_wheel_model[i]) 
        {
            m_wheel_transform[i]->addKid(m_wheel_model[i]);
            m_root->addKid(m_wheel_transform[i]);

            Vec3 min_wheel, max_wheel;
            SSGHelp::MinMax(m_wheel_model[i], &min_wheel, &max_wheel);
            m_wheel_graphics_radius[i] = (max_wheel.getZ()-min_wheel.getZ())*0.5f;
            sgMat4 wheel_loc;
            sgVec3 hpr;
            sgZeroVec3(hpr);
            sgMakeCoordMat4(wheel_loc, m_wheel_graphics_position[i].toFloat(),
                            hpr);        
            m_wheel_transform[i]->setTransform(wheel_loc);
        }   // if m_wheel_model[i]
    }   // for i<4
    if(!m_wheel_model[0])
    {
        m_z_offset = m_kart_height*0.5f;
    }
}   // load

// ----------------------------------------------------------------------------
/** Loads a single wheel node. Currently this is the name of the wheel model
 *  and the position of the wheel relative to the kart.
 *  \param wheel_name Name of the wheel, e.g. wheel-rear-left.
 *  \param index Index of this wheel in the global m_wheel* fields.
 */
void KartModel::loadWheelInfo(const lisp::Lisp* const lisp,
                              const std::string &wheel_name, int index)
{
    const lisp::Lisp* const wheel = lisp->getLisp(wheel_name);
    if(!wheel)
    {
        // Only print the warning if a model filename is given. Otherwise the
        // stk_config file is read (which has no model information).
        if(m_model_filename!="")
        {
            fprintf(stderr, "Missing wheel information '%s' for model '%s'.\n",
                wheel_name.c_str(), m_model_filename.c_str());
            fprintf(stderr, "This can be ignored, but the wheels will not rotate.\n");
        }
        return;
    }
    wheel->get("model",            m_wheel_filename[index]         );
    wheel->get("position",         m_wheel_graphics_position[index]);
    wheel->get("physics-position", m_wheel_physics_position[index] );
    wheel->get("min-suspension",   m_min_suspension[index]         );
    wheel->get("max-suspension",   m_max_suspension[index]         );
}   // loadWheelInfo

// ----------------------------------------------------------------------------
/** Sets the default position for the physical wheels if they are not defined
 *  in the data file. The default position is to have the wheels at the corner
 *  of the chassis. But since the position is relative to the center of mass,
 *  this must be specified.
 *  \param center_shift Amount the kart chassis is moved relative to the center
 *                      of mass.
 *  \param wheel_radius Radius of the physics wheels.
 */
void  KartModel::setDefaultPhysicsPosition(const Vec3 &center_shift,
                                           float wheel_radius)
{
    for(unsigned int i=0; i<4; i++)
    {
        if(m_wheel_physics_position[i].getX()==UNDEFINED)
        {
            m_wheel_physics_position[i].setX((i==1||i==3) 
                                               ? -0.5f*m_kart_width
                                               :  0.5f*m_kart_width
                                               +  center_shift.getX());
            m_wheel_physics_position[i].setY((0.5f*m_kart_length-wheel_radius)
                                               * ((i<2)? 1 : -1)
                                               +  center_shift.getY());
            // Set the connection point so that a maximum compressed wheel
            // (susp. length=0) will still poke a little bit out under the 
            // kart
            m_wheel_physics_position[i].setZ(wheel_radius-0.05f);
        }   // if physics position is not defined
    }   // for i<4
}   // setDefaultPhysicsPosition

// ----------------------------------------------------------------------------
/** Rotates and turns the wheels appropriately, and adjust for suspension.
 *  \param rotation How far the wheels should rotate.
 *  \param steer How much the front wheels are turned for steering.
 *  \param suspension Suspension height for all four wheels.
 */
void KartModel::adjustWheels(float rotation, float steer,
                             const float suspension[4])
{
    sgMat4 wheel_front;
    sgMat4 wheel_steer;
    sgMat4 wheel_rota;

    float clamped_suspension[4];
    // Clamp suspension to minimum and maximum suspension length, so that
    // the graphical wheel models don't look too wrong.
    for(unsigned int i=0; i<4; i++)
    {
        const float suspension_length = (m_max_suspension[i]-m_min_suspension[i])/2;

        // limit amplitude between set limits, first dividing it by a
        // somewhat arbitrary constant to reduce visible wheel movement
        clamped_suspension[i] = std::min(std::max(suspension[i]/m_dampen_suspension_amplitude[i],
                                                  m_min_suspension[i]),
                                                  m_max_suspension[i]);
        float ratio = clamped_suspension[i] / suspension_length;
        const int sign = ratio < 0 ? -1 : 1;
        ratio = sign * fabsf(ratio*(2-ratio)); // expanded form of 1 - (1 - x)^2, i.e. making suspension display quadratic and not linear
        clamped_suspension[i] = ratio*suspension_length;
    }   // for i<4

    sgMakeRotMat4(wheel_rota, 0, RAD_TO_DEGREE(-rotation), 0);
    sgMakeRotMat4(wheel_steer, steer, 0, 0);
    sgMultMat4(wheel_front, wheel_steer, wheel_rota);

    sgCopyVec3(wheel_front[3], m_wheel_graphics_position[0].toFloat());
    wheel_front[3][2] += clamped_suspension[0];
    m_wheel_transform[0]->setTransform(wheel_front);

    sgCopyVec3(wheel_front[3], m_wheel_graphics_position[1].toFloat());
    wheel_front[3][2] += clamped_suspension[1];
    m_wheel_transform[1]->setTransform(wheel_front);

    sgCopyVec3(wheel_rota[3],  m_wheel_graphics_position[2].toFloat());
    wheel_rota[3][2] +=  clamped_suspension[2];
    m_wheel_transform[2]->setTransform(wheel_rota);

    sgCopyVec3(wheel_rota[3],  m_wheel_graphics_position[3].toFloat());
    wheel_rota[3][2] +=  clamped_suspension[3];
    m_wheel_transform[3]->setTransform(wheel_rota);
}   // adjustWheels

// ----------------------------------------------------------------------------
/** Puts all wheels in the default position. Used when displaying the karts
 *  in the character selection screen.
 */
void KartModel::resetWheels()
{
    for(unsigned int i=0; i<4; i++)
    {
        const float suspension[4]={0,0,0,0};
        adjustWheels(0, 0, suspension);
    }
}   // reset