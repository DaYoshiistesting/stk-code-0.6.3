//  $Id: challenge_manager.cpp 1259 2007-09-24 12:28:19Z hiker $
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

#include "unlock_manager.hpp"

#include <set>
#include <string>
#include <stdio.h>

#include "race_manager.hpp"
#include "file_manager.hpp"
#include "user_config.hpp"
#include "challenges/challenge_data.hpp"
#include "utils/string_utils.hpp"

UnlockManager* unlock_manager=0;
//-----------------------------------------------------------------------------

UnlockManager::UnlockManager()
{
    // The global variable 'unlock_manager' is needed in the challenges,
    // but it's not set yet - so we define it here (and it gets re-assign
    // in main).
    unlock_manager=this;

    // Read challenges from .../data/challenges/
    // -----------------------------------------
    std::set<std::string> result;
    file_manager->listFiles(result, "data/challenges");
    for(std::set<std::string>::iterator i  = result.begin();
                                        i != result.end()  ; i++)
    {
        if (StringUtils::has_suffix(*i, ".challenge")) 
            addChallenge(file_manager->getConfigFile("challenges/"+*i));
    }

    std::set<std::string> dirs;
    file_manager->listFiles(dirs, file_manager->getChallengeDir(), /*is_full_path*/ true);

    computeActive();

}   // UnlockManager

//-----------------------------------------------------------------------------
void UnlockManager::addChallenge(Challenge *c)
{
    m_all_challenges[c->getId()]=c;
}   // addChallenge
//-----------------------------------------------------------------------------
void UnlockManager::addChallenge(const std::string& filename)
{
    addChallenge(new ChallengeData(filename));
}   // addChallenge
//-----------------------------------------------------------------------------
/** Checks if all challenges are valid, i.e. contain a valid track or GP. 
 *  If not, STK is aborted with an error message.
 */
void UnlockManager::check() const
{
    for(AllChallengesType::const_iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        i->second->check();
    }   // for i
           
}   // check

//-----------------------------------------------------------------------------
std::vector<const Challenge*> UnlockManager::getActiveChallenges()
{
    computeActive();
    std::vector<const Challenge*> all_active;
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        if(i->second->isActive()) all_active.push_back(i->second);
    }
    return all_active;
}   // getActiveChallenges

//-----------------------------------------------------------------------------
Challenge* UnlockManager::getChallenge(const std::string& id)
{
    if(m_all_challenges.find(id)==m_all_challenges.end()) return NULL;
    return m_all_challenges[id];
}   // getChallenge

//-----------------------------------------------------------------------------
/** This is called from user_config when reading the config file
*/
void UnlockManager::load(const lisp::Lisp* config)
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        i->second->load(config);
    }
    computeActive();
}   // load

//-----------------------------------------------------------------------------
void UnlockManager::save(lisp::Writer* writer)
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        i->second->save(writer);
    }   // for i in m_all_challenges
}   // save

//-----------------------------------------------------------------------------
void UnlockManager::computeActive()
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        // Changed challenge
        // -----------------
        if((i->second)->isSolved()) 
        {
            // The constructor calls computeActive, which actually locks 
            // all features, so unlock the solved ones (and don't try to
            // save the state, since we are currently reading it)
            
            unlockFeature(i->second, /*save*/ false);
            continue;
        }

        // Otherwise lock the feature, and check if the challenge is active
        // ----------------------------------------------------------------
        lockFeature(i->second);
        std::vector<std::string> pre_req=(i->second)->getPrerequisites();
        bool allSolved=true;
        for(std::vector<std::string>::iterator pre =pre_req.begin();
                                               pre!=pre_req.end(); pre++)
        {
            const Challenge*p = getChallenge(*pre);
            if(!p)
            {
                fprintf(stderr,"Challenge prerequisite '%s' of '%s' not found - ignored\n",
                        pre->c_str(), i->first.c_str());
                continue;
            }
            if(!p->isSolved())
            {
                allSolved=false;
                break;
            }
        }   // for all pre in pre_req
        if(allSolved)
        {
            i->second->setActive();
        }   // if solved
    }   // for i
    clearUnlocked();
}   // computeActive

//-----------------------------------------------------------------------------
/** This is called when a race is finished. Call all active challenges
*/
void UnlockManager::raceFinished()
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        if(i->second->isActive() && i->second->raceFinished())
        {
            unlockFeature(i->second);
        }   // if isActive && challenge solved
    }
    race_manager->setEnergyTarget(0);  //reset
}   // raceFinished

//-----------------------------------------------------------------------------
void UnlockManager::grandPrixFinished()
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        if(i->second->isActive() &&i->second->grandPrixFinished())
        {
            unlockFeature(i->second);
        }
    }
    race_manager->setEnergyTarget(0);
}   // grandPrixFinished

//-----------------------------------------------------------------------------
void UnlockManager::lockFeature(Challenge* challenge)
{
    const unsigned int amount = (unsigned int)challenge->getFeatures().size();
    for(unsigned int n=0; n<amount; n++)
        m_locked_features[challenge->getFeatures()[n].name]=true;
}   // lockFeature

//-----------------------------------------------------------------------------

void UnlockManager::unlockFeature(Challenge* c, bool save)
{
    const unsigned int amount = (unsigned int)c->getFeatures().size();
    for(unsigned int n=0; n<amount; n++)
    {
        std::string feature = c->getFeatures()[n].name;
        std::map<std::string,bool>::iterator p=m_locked_features.find(feature);
        if(p==m_locked_features.end())
        {
            //fprintf(stderr,"Unlocking feature '%s' failed: feature is not locked.\n",
            //        (feature).c_str());
            return;
        }
        m_locked_features.erase(p);
    }
    
    // Add to list of recently unlocked features
    m_unlocked_features.push_back(c);
    c->setSolved();  // reset isActive flag
    
    // Save the new unlock information
    if(save) user_config->saveConfig();
}   // unlockFeature

//-----------------------------------------------------------------------------
bool UnlockManager::isLocked(const std::string& feature)
{
    return m_locked_features.find(feature)!=m_locked_features.end();
}  // featureIsLocked
