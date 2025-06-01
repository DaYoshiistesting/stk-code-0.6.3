//  $Id: item.hpp 2812 2008-12-29 21:20:05Z auria $
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

#ifndef HEADER_ITEM_H
#define HEADER_ITEM_H

// num_players triggers  'already defined' messages without the WINSOCKAPI define. Don't ask me :(
#define _WINSOCKAPI_
#include <plib/sg.h>
#include "karts/kart.hpp"
#include "utils/coord.hpp"

class ssgTransform;
class ssgEntity;
class Item;

class TriggerItemListener
{
public:
    virtual ~TriggerItemListener() {}
    virtual void onTriggerItemApproached(Item* who) = 0;
};
// -----------------------------------------------------------------------------
class Item
{
public:
    enum ItemType
    {
        ITEM_FIRST,
        ITEM_BONUS_BOX = ITEM_FIRST,
        ITEM_BANANA,
        ITEM_BIG_NITRO,
        ITEM_SMALL_NITRO,
        ITEM_BUBBLEGUM,
        ITEM_TRIGGER,
        ITEM_LAST = ITEM_TRIGGER,
        ITEM_COUNT,
        ITEM_NONE
    };

private:
    ItemType      m_type;              // Item type.
    ItemType      m_original_type;     // Contains the original type and ITEM_NONE.
    Vec3          m_original_hpr;      // Stores the original rotation of an item.
    bool          m_collected;         // True if item was collected & is not displayed.
    float         m_time_till_return;  // Time till a collected item reappears.
    ssgEntity    *m_original_model;    // Stores the original model to reset it.
    Coord         m_coord;             // Original coordinates, used mainly when
                                       // collected items reappear.
    ssgTransform* m_root;              // The actual root of the item.
    Vec3          m_xyz;               // Saves calls to m_root->getPosition().
    unsigned int  m_item_id;           // Index in item_manager field.
    bool          m_rotate;            // Set to false if item should not rotate.
    
    /** optionally, set this if this item was laid by a particular kart. in this case,
        the 'm_deactive_time' will also be set - see below. */ 
    const Kart*   m_parent;
    /** optionally, if item was placed by a kart, a timer can be used to temporarly
       deactivate collision so a kart is not hit by its own item */
    float         m_deactive_time;

    /** Counts how often an item is used before it disappears. Used for 
     *  bubble gum to make them disappear after a while. A value >0
     *  indicates that the item still exists, =0 that the item can be
     *  deleted, and <0 that the item will never be deleted. */
    int           m_disappear_counter;

    /** callback used if type == ITEM_TRIGGER */
    TriggerItemListener* m_listener;

    /** square distance at which item is collected */
    float m_distance_2;

    void          initItem(ItemType type, const Vec3 &xyz);
    void          setType(ItemType type);
    
public:
                  Item (ItemType type, const Vec3& xyz, const Vec3& normal,
                        ssgEntity* model);
                  Item(const Vec3& xyz, float distance, 
                       TriggerItemListener* trigger);
    virtual       ~Item ();
    void          update  (float delta);
    virtual void  isCollected(const Kart *kart, float t=2.0f);
    
    // ------------------------------------------------------------------------
    /** Returns true if the Kart is close enough to hit this item, and
     *  the item is not deactivated anymore.
     *  \param kart Kart to test.
     */
    bool hitKart (Kart* kart, const Vec3& xyz) const
    {
        return (m_parent!=kart || m_deactive_time <=0) &&
               (xyz-m_coord.getXYZ()).length2()<0.8f;
    }   // hitKart

    // ------------------------------------------------------------------------
    /** Deactivates the item for a certain amount of time. It is used to
     *  prevent bubble gum from hitting a kart over and over again (in each
     *  frame) by giving it time to drive away.
     *  \param t Time the item is deactivated.
     */
    void          deactivate(float t)  { m_deactive_time=t; }
    // ------------------------------------------------------------------------
    /** Sets the index of this item in the item manager list. */
    void          setItemId(unsigned int n)  { m_item_id = n; }
    // ------------------------------------------------------------------------
    unsigned int  getItemId()    const {return m_item_id;   }
    ssgTransform* getRoot()      const {return m_root;      }
    ItemType      getType()      const {return m_type;      }
    bool          wasCollected() const {return m_collected; }
    const Vec3&   getXYZ()       const {return m_xyz;       }
    bool          isUsedUp()     const {return m_disappear_counter==0; }
    bool          canBeUsedUp()  const {return m_disappear_counter>-1; }
    void          setParent(Kart* parent);
    void          reset();
};   // class Item

#endif
