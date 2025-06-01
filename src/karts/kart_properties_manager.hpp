//  $Id: kart_properties_manager.hpp 2489 2008-11-21 02:58:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_KARTPROPERTIESMANAGER_H
#define HEADER_KARTPROPERTIESMANAGER_H

#include <vector>
#include <map>
#include "network/remote_kart_info.hpp"

class KartProperties;

class KartPropertiesManager
{
private:
    std::vector<std::string>                 m_all_groups;
    std::map<std::string, std::vector<int> > m_groups;
    /** Vector containing kart numbers that have been selected in multiplayer
     * games.  This it used to ensure the same kart can not be selected more
     * than once. */
    std::vector<int>         m_selected_karts;
    /** Contains a flag for each kart indicating wether it is available on
     *  all clients or not. */
    std::vector<bool>        m_kart_available;
protected:

    typedef std::vector<KartProperties*> KartPropertiesVector;
    /** All available kart configurations */
    KartPropertiesVector m_karts_properties;

public:
    KartPropertiesManager();
    ~KartPropertiesManager();
        
    const KartProperties*    getKartById            (int i) const;
    const KartProperties*    getKart                (const std::string &ident) const;
    const int                getKartId              (const std::string &ident) const;
    int                      getKartByGroup         (const std::string& group, int i) const;
    void                     loadKartData           (bool dont_load_models=false);
    const unsigned int       getNumberOfKarts       () const {return (unsigned int)m_karts_properties.size();}
    const std::vector<std::string>& 
                             getAllGroups           () const {return m_all_groups;     }
    const std::vector<int>&  getKartsInGroup        (const std::string& g)
                                                             {return m_groups[g];      }
    void                     clearAllSelectedKarts()         {m_selected_karts.clear();}
    void                     removeLastSelectedKart()        {m_selected_karts.pop_back();}
    int                      getNumSelectedKarts() const     {return m_selected_karts.size();}
    bool                     kartAvailable(int kartid);
    std::vector<std::string> getAllAvailableKarts() const;
    void                     setUnavailableKarts(std::vector<std::string>);
    /** Sets a kartid to be selected (without any tests). */
    void                     selectKart(int kartid) {m_selected_karts.push_back(kartid);}
    void                     selectKartName(const std::string &kart_name);
    bool                     testAndSetKart(int kartid);
    std::vector<std::string> getRandomKartList(int count, RemoteKartInfoList& existing_karts);
    void                     removeTextures      ();
};

extern KartPropertiesManager *kart_properties_manager;

#endif

/* EOF */
