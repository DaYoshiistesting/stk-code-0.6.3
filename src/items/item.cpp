//  $Id: item.cpp 3029 2009-01-22 22:27:13Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include "items/item.hpp"

#include "graphics/scene.hpp"
#include "karts/kart.hpp"
#include "utils/coord.hpp"
#include "utils/vec3.hpp"

Item::Item(ItemType type, const Vec3& xyz, const Vec3& normal,
           ssgEntity* model)
{
	assert(type != ITEM_TRIGGER);
	initItem(type, xyz);
    // Sets heading to 0, and sets pitch and roll depending on the normal. */
    Vec3 hpr           = Vec3(0, normal);
    m_coord            = Coord(xyz, hpr);
    m_root             = new ssgTransform();
    m_distance_2       = 0.8f;
    m_original_model   = model;
    m_listener         = NULL;
    m_root->ref();
    m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    m_root->addKid(model);
    scene->add(m_root);
}   // Item
//-----------------------------------------------------------------------------

/** \brief Constructor to create a trigger item.
  * Trigger items are invisible and can be used to trigger a behavior when
  * approaching a point.
  */
Item::Item(const Vec3& xyz, float distance, TriggerItemListener* trigger)
{
    initItem(ITEM_TRIGGER, xyz);
    // Sets heading to 0, and sets pitch and roll depending on the normal. */
    m_original_hpr      = Vec3(0, 0, 0);
    m_original_model    = NULL;
    m_root              = NULL;
    m_listener          = trigger;
    m_distance_2        = distance*distance;
}   // Item(xyz, distance, trigger)

//-----------------------------------------------------------------------------
/** Initialises the item.
 *  \param type Type of the item.
 */
void Item::initItem(ItemType type, const Vec3 &xyz)
{
	m_type              = type;
	m_parent            = NULL;
	m_xyz               = xyz;
    m_deactive_time     = 0;
    m_item_id           = -1;
	m_original_type     = ITEM_NONE;
    m_collected         = false;
    m_time_till_return  = 0.0f;  // not strictly necessary, see isCollected()
	m_rotate            = (type!=ITEM_BUBBLEGUM) && (type!=ITEM_TRIGGER);
    m_disappear_counter = m_type==ITEM_BUBBLEGUM 
                        ? stk_config->m_bubble_gum_counter
                        : -1 ;
}   //initItem

//-----------------------------------------------------------------------------
/** Sets the type of this item, but also derived values, e.g. m_rotate.
 *  (bubblegums do not return).
 *  \param type New type of the item.
 */
void Item::setType(ItemType type)
{
    m_type   = type;
    m_rotate = (type!=ITEM_BUBBLEGUM) && (type!=ITEM_TRIGGER);
}   // setType

//-----------------------------------------------------------------------------
Item::~Item()
{
    if (m_root != NULL)
    {
        scene->remove(m_root);
        ssgDeRefDelete(m_root);
    }
}   // ~Item

//-----------------------------------------------------------------------------
void Item::reset()
{
    m_collected        = false;
    m_time_till_return = 0.0f;
    m_deactive_time    = 0.0f;
	m_disappear_counter = m_type==ITEM_BUBBLEGUM 
                        ? stk_config->m_bubble_gum_counter
                        : -1 ;
	if(m_original_type!=ITEM_NONE)
    {
        setType(m_original_type);
        m_original_type = ITEM_NONE;
    }

    if (m_root !=NULL)
    {
        m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    }
}   // reset
//-----------------------------------------------------------------------------
void Item::setParent(Kart* parent)
{
    m_parent        = parent;
    m_deactive_time = 1.5f;
}

//-----------------------------------------------------------------------------
void Item::update(float delta)
{
    if(m_deactive_time > 0) m_deactive_time -= delta;
    
    if(m_collected)
    {
        m_time_till_return -= delta;
        if ( m_time_till_return > 0 )
        {
            if (m_root != NULL)
            {
                Vec3 hell(m_coord.getXYZ());
                hell.setZ( (m_time_till_return>1.0f) ? -1000000.0f 
		            : m_coord.getXYZ().getZ() - m_time_till_return / 2.0f);
                m_root->setTransform(hell.toFloat());
            }
        }
        else
        {
            m_collected    = false;
            if(m_root != NULL)
                m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
        }   // T>0

    }
    else
    {   // not m_collected
        
        if(!m_rotate) return;
        // have it rotate
        Vec3 rotation(delta*M_PI, 0, 0);
		m_coord.setHPR(m_coord.getHPR()+rotation);
        m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    }
}   // update

//-----------------------------------------------------------------------------
/** Is called when the item is hit by a kart.  It sets the flag that the item
 *  has been collected, and the time to return to the parameter. 
 *  \param t Time till the object reappears (defaults to 2 seconds).
 */
void Item::isCollected(const Kart *kart, float t)
{
    m_collected = true;
    m_parent    = kart;
    if(m_type==ITEM_BUBBLEGUM && m_disappear_counter>0)
    {
        m_disappear_counter --;
        // Deactivates the item for a certain amount of time. It is used to
        // prevent bubble gum from hitting a kart over and over again (in each
        // frame) by giving it time to drive away.
        m_deactive_time = 0.0f;
        // Set the time till reappear to -1 seconds --> the item will 
        // reappear immediately.
        m_time_till_return = -1;
    }
    else
    {
        // Note if the time is negative, in update the m_collected flag will
        // be automatically set to false again.
        m_time_till_return = t;
        if (m_root != NULL)
        {
            m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
        }
    }

    if (m_listener != NULL)
    {
        m_listener->onTriggerItemApproached(this);
    }
}  // isCollected

