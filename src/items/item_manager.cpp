//  $Id: item_manager.cpp 3034 2009-01-23 05:23:22Z hikerstk $
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

#include <stdexcept>
#include <string>
#include <sstream>

#include "file_manager.hpp"
#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "user_config.hpp"
#include "items/item_manager.hpp"
#include "karts/kart.hpp"
#include "modes/linear_world.hpp"
#include "network/network_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

ItemManager* item_manager;
std::vector<ssgEntity *> ItemManager::m_item_model;
ItemManager * ItemManager::m_item_manager = NULL;

//-----------------------------------------------------------------------------
/** Creates one instance of the item manager. */
void ItemManager::create()
{
    assert(!m_item_manager);
    m_item_manager = new ItemManager();
}   // create

//-----------------------------------------------------------------------------
/** Destroys the one instance of the item manager. */
void ItemManager::destroy()
{
    assert(m_item_manager);
    delete m_item_manager;
    m_item_manager = NULL;
}   // destroy

//-----------------------------------------------------------------------------
void ItemManager::loadDefaultItems()
{
    m_item_model.resize(Item::ITEM_LAST-Item::ITEM_FIRST+1, NULL);
    std::map<Item::ItemType, std::string> item_names;
    item_names[Item::ITEM_BANANA     ] = "banana";
    item_names[Item::ITEM_BONUS_BOX  ] = "bonus-box";
    item_names[Item::ITEM_BUBBLEGUM  ] = "bubblegum";
    item_names[Item::ITEM_BIG_NITRO  ] = "big-nitro";
    item_names[Item::ITEM_SMALL_NITRO] = "small-nitro";
    item_names[Item::ITEM_TRIGGER    ] = "trigger";

    const lisp::Lisp* ROOT = 0;
    std::string items_file = file_manager->getConfigFile("items.items");
    lisp::Parser parser;
    ROOT = parser.parse(items_file);
    for(unsigned int i=Item::ITEM_FIRST; i<=Item::ITEM_LAST; i++)
    {
        const std::string &name = item_names[(Item::ItemType)i];
        const lisp::Lisp *lisp = ROOT->getLisp("tuxkart-items");
        const lisp::Lisp *node = lisp->getLisp(name);
        if(!node) continue;

        std::string model_file;
        node->get("model", model_file);

        ssgEntity* model = loader->load(model_file, CB_ITEM);

        if(!node || !model)
        {
            fprintf(stderr, "Item model '%s' in items.items could not be loaded - aborting",
                    name.c_str());
            exit(-1);
        }
        model->ref();
        m_item_model[i] = model;
		
    } //for i
    delete ROOT;
}  //loadDefaultItems

//-----------------------------------------------------------------------------
void ItemManager::removeTextures()
{
    for(unsigned int i=0; i<Item::ITEM_LAST-Item::ITEM_FIRST+1; i++)
    {
        if(m_item_model[i]) m_item_model[i]->deRef();
           m_item_model[i] = NULL;
    }
    callback_manager->clear(CB_ITEM);
}   // removeTextures

//-----------------------------------------------------------------------------
ItemManager::ItemManager()
{
    // The actual loading is done in loadDefaultItems
    if(Track::get())
    {
        m_items_in_sector = new std::vector<AllItemTypes>;
        m_items_in_sector->resize(Track::get()->m_driveline.size()+1);
    }
    else
    {
        m_items_in_sector = NULL;
    }

}   // ItemManager

//-----------------------------------------------------------------------------
ItemManager::~ItemManager()
{
    if(m_items_in_sector)
        delete m_items_in_sector;

    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if(*i)
            delete *i;
    }
    m_all_items.clear();
    callback_manager->clear(CB_ITEM);
}   // ~ItemManager

//-----------------------------------------------------------------------------
/** Inserts the new item into the items management data structures, if possible
 *  reusing an existing, unused entry (e.g. due to a removed bubble gum). Then 
 *  the item is also added to the sector-wise list of items.
 */
void ItemManager::insertItem(Item *h)
{
    // Find where the item can be stored in the index list: either in a
    // previously deleted entry, otherwise at the end.
    int index = -1;
    for(index=m_all_items.size()-1; index>=0 && m_all_items[index]; index--) {}
    
    if(index==-1) index = m_all_items.size();

    if(index<(int)m_all_items.size())
        m_all_items[index] = h;
    else
        m_all_items.push_back(h);
    h->setItemId(index);

    if(m_items_in_sector)
	{
        const Vec3 &xyz = h->getXYZ();
        int sector = Track::UNKNOWN_SECTOR;
        Track::get()->findRoadSector(xyz, &sector);
        if(sector==Track::UNKNOWN_SECTOR)
            (*m_items_in_sector)[m_items_in_sector->size()-1].push_back(h);
        else
            (*m_items_in_sector)[sector].push_back(h);
    }
}   // insertItem

//-----------------------------------------------------------------------------
/** Creates a new item.
 *  \param type Type of the item.
 *  \param xyz Position of the item.
 *  \param normal The normal of the terrain to set roll and pitch.
 *  \param parent In case of a dropped item used to avoid that a kart
 *         is affected by its own items.
 */
Item* ItemManager::newItem(Item::ItemType type, const Vec3& xyz, const Vec3 &normal,
                           Kart* parent)
{ 
    Item *h = new Item(type, xyz, normal, m_item_model[type]);
    
    insertItem(h);
    if(parent != NULL) h->setParent(parent);

    return h;
}   // newItem

//-----------------------------------------------------------------------------
/** Creates a new trigger item.
 *  \param xyz  Position of the item.
 */
Item* ItemManager::newItem(const Vec3& xyz, float distance, 
                           TriggerItemListener* listener)
{
    Item* h;
    h = new Item(xyz, distance, listener);
    insertItem(h);

    return h;
}   // newItem

//-----------------------------------------------------------------------------
/** Set an item as collected.
 *  This function is called on the server when an item is collected, or on the
 *  client upon receiving information about collected items.                  */
void ItemManager::collectedItem(Item *h, Kart *kart, int add_info)
{
    assert(h);
    h->isCollected(kart);
    kart->collectedItem(h, add_info);
}   // collectedItem

//-----------------------------------------------------------------------------
void ItemManager::hitItem(Kart* kart)
{
    // Only do this on the server
    if(network_manager->getMode()==NetworkManager::NW_CLIENT) return;

    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if((!*i) || (*i)->wasCollected()) continue;
        if((*i)->hitKart(kart, kart->getXYZ()))
        {
            collectedItem(*i, kart);
        }   // if hit
    }   // for m_all_items
}   // hitItem

//-----------------------------------------------------------------------------
/** Remove all item instances, and the track specific models. This is used
 * just before a new track is loaded and a race is started
 */
void ItemManager::reset()
{
    AllItemTypes::iterator i=m_all_items.begin();
    while(i!=m_all_items.end())
    {
        if(!*i)
        {
            i++;
            continue;
        }
        if((*i)->canBeUsedUp() || (*i)->getType()==Item::ITEM_BUBBLEGUM)
        {
            deleteItem(*i);
            i++;
        }
        else
        {
            (*i)->reset();
            i++;
        }
    }  // for i
}   // reset

//-----------------------------------------------------------------------------
/** Updates all items.
* \param dt Time step.
*/
void ItemManager::update(float delta)
{
    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if(*i)
        {
            (*i)->update(delta);
            if( (*i)->isUsedUp())
            {
                deleteItem(*i);
            }   // if usedUp
        }   // if *i
    }   // for m_all_items
}   // delta
//-----------------------------------------------------------------------------
/** Removes an items from the items-in-sector list, from the list of all
 *  items, and then frees the item itself.
 *  \param h The item to delete.
 */
void ItemManager::deleteItem(Item *h)
{
    // First check if the item needs to be removed from the items-in-sector list
    if(m_items_in_sector)
    {
        const Vec3 &xyz = h->getXYZ();
        int sector = Track::UNKNOWN_SECTOR;
        Track::get()->findRoadSector(xyz, &sector);
        unsigned int indx = sector==Track::UNKNOWN_SECTOR
                          ? m_items_in_sector->size()-1
                          : sector;
        AllItemTypes &items = (*m_items_in_sector)[indx];
        AllItemTypes::iterator it = std::find(items.begin(), items.end(), h);
        assert(it!=items.end());
        items.erase(it);
    }   // if m_items_in_sector

    int index = h->getItemId();
    m_all_items[index] = NULL;
    delete h;
}   // deleteItem
//------------------------------------------------------------------------------
