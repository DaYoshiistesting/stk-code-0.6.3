//  $Id: item_manager.hpp 2443 2008-11-11 08:33:06Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_ITEMMANAGER_H
#define HEADER_ITEMMANAGER_H


#include <assert.h>
#include <vector>
#include <map>
#include <string>
#include "items/item.hpp"
#include "lisp/lisp.hpp"

class Kart;
class ssgEntity;

class ItemManager
{

private:
    // The vector of all items of the current track
    typedef std::vector<Item*> AllItemTypes;
    AllItemTypes m_all_items;

    // This stores all item models
    static std::vector<ssgEntity *> m_item_model;

    // The instance of ItemManager while a race is on
    static ItemManager *m_item_manager;

    void insertItem(Item *h);
    void deleteItem(Item *h);

    // Stores which items are on which sectors
    std::vector< AllItemTypes > *m_items_in_sector;

    ItemManager();
   ~ItemManager();

public:
    // Return an instance of the item manager
    static       ItemManager *get() 
    { 
        assert(m_item_manager); 
        return m_item_manager;
    }
    static void  loadDefaultItems();
    static void  removeTextures  ();
    static void  create          ();
    static void  destroy         ();
    Item*        newItem         (Item::ItemType type, const Vec3& xyz, 
                                 const Vec3 &normal, Kart* parent=NULL);
    Item*        newItem         (const Vec3& xyz, float distance, 
                                 TriggerItemListener* listener);
    void         update          (float delta);
    void         hitItem         (Kart* kart);
    void         reset           ();
    void         collectedItem   (Item *h, Kart *kart,
                                 int add_info=-1);
    unsigned int getNumberOfItems()      const {return m_all_items.size();}
    const Item*  getItem(unsigned int n) const {return m_all_items[n];};
    Item*        getItem(unsigned int n)       {return m_all_items[n];};
    static ssgEntity*    getItemModel   (Item::ItemType type)
                                        {return m_item_model[type];}
    const AllItemTypes& getItemsInDriveline (unsigned int n) const 
    {
        assert(m_items_in_sector); 
        assert(n<(*m_items_in_sector).size());
        return (*m_items_in_sector)[n];
    }
};

#endif
