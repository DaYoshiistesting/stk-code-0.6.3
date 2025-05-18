//  $Id: powerup_manager.cpp 3034 2009-01-23 05:23:22Z hikerstk $
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

#include "items/powerup_manager.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "file_manager.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "items/bowling.hpp" 
#include "items/cake.hpp"
#include "items/plunger.hpp"
#include "loader.hpp"

PowerupManager* powerup_manager=0;

//-----------------------------------------------------------------------------
PowerupManager::PowerupManager()
{
    for(int i=0; i<POWERUP_MAX; i++)
    {
        m_all_models[i] = (ssgEntity*)NULL;
        m_all_icons[i]  = (Material*)NULL;
    }
}   // PowerupManager

//-----------------------------------------------------------------------------
PowerupManager::~PowerupManager()
{
    for(unsigned int i=POWERUP_FIRST; i<=POWERUP_LAST; i++)
    {
        m_all_models[(PowerupType)i]->deRef();
    }
}   // ~PowerupManager

//-----------------------------------------------------------------------------
void PowerupManager::removeTextures()
{
    for(int i=0; i<POWERUP_MAX; i++)
    {
        if(m_all_icons [i]) ssgDeRefDelete(m_all_icons [i]->getState());
        if(m_all_models[i]) ssgDeRefDelete(m_all_models[i]);
    }   // for
    callback_manager->clear(CB_COLLECTABLE);

}   // removeTextures

//-----------------------------------------------------------------------------
void PowerupManager::loadPowerups()
{
    const lisp::Lisp* ROOT = 0;

    lisp::Parser parser;
    std::string powerups_file = file_manager->getConfigFile("powerups.data");
    ROOT = parser.parse(powerups_file);

    const lisp::Lisp* lisp = ROOT->getLisp("tuxkart-collectables");
    if(!lisp)
    {
        std::ostringstream msg;
        msg << "No 'tuxkart-collectables' node found while parsing '" 
            << powerups_file << "'.";
        throw std::runtime_error(msg.str());
    }
    LoadOnePowerup(lisp, "bubblegum", POWERUP_BUBBLEGUM);
    LoadOnePowerup(lisp, "cake",      POWERUP_CAKE     ); 
    LoadOnePowerup(lisp, "bowling",   POWERUP_BOWLING  ); 
    LoadOnePowerup(lisp, "zipper",    POWERUP_ZIPPER   ); 
    LoadOnePowerup(lisp, "plunger",   POWERUP_PLUNGER  ); 
    LoadOnePowerup(lisp, "parachute", POWERUP_PARACHUTE); 
    LoadOnePowerup(lisp, "anvil",     POWERUP_ANVIL    );  
    delete ROOT;

}   // loadPowerups

//-----------------------------------------------------------------------------
void PowerupManager::LoadOnePowerup(const lisp::Lisp* lisp, const char *name,
                                    PowerupType type)
{
    const lisp::Lisp* powerup_lisp = lisp->getLisp(name);
    std::string model;
    powerup_lisp->get("model", model);
    std::string iconfile; 
    powerup_lisp->get("icon", iconfile);

    // load material
    m_all_icons[type] = material_manager->getMaterial(iconfile,
                                  /* full_path */     false,
                                  /*make_permanent */ true);
    m_all_icons[type]->getState()->ref();


    if(model!="")
    {
        // FIXME LEAK: not freed (uniportant, since the models have to exist
        // for the whole game anyway).
        m_all_models[type] = loader->load(model, CB_COLLECTABLE);
        m_all_models[type]->ref();
        m_all_models[type]->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
        if(!m_all_models[type])
        {
              std::ostringstream o;
              o<<"Can't load model '"<<model
              << "' for powerup type '"<<type<<"'. - aborting.\n";
              throw std::runtime_error(o.str());
        }
    }
    else
    {
        m_all_models[type]  = 0;
        m_all_extends[type] = btVector3(0.0f,0.0f,0.0f);
    }


    // Load special attributes for certain powerups
    switch (type) {
        case POWERUP_BOWLING:          
             Bowling::init(powerup_lisp, m_all_models[type]); break;
        case POWERUP_PLUNGER:          
             Plunger::init(powerup_lisp, m_all_models[type]); break;
        case POWERUP_CAKE: 
             Cake::init(powerup_lisp, m_all_models[type]);    break;
        default:;
    }   // switch

}   // LoadOnePowerup

