//  $Id: race_results_gui.cpp 3835 2009-08-11 22:41:47Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Eduardo Hernandez Munoz
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

#include "gui/race_results_gui.hpp"

#include <SDL/SDL.h>

#include "race_manager.hpp"
#include "highscore_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "modes/world.hpp"
#include "karts/kart_properties.hpp"
#include "network/network_manager.hpp"
#include "utils/translation.hpp"

/** It can happen (e.g. if a player has skidding assigned to space or enter)
 *  that a selection key is still pressed when this menu is shown (e.g. follow
 *  the leader race which doesn't have a end-race-camera). To avoid that this
 *  menu is left as soon as it is entered (since back to main menu is selected
 *  as default), WAITING_TIME seconds have to pass before a selection is 
 *  accepted. Admittedly a somehwat ugly work around.
 */
#define WAITING_TIME 1

RaceResultsGUI::RaceResultsGUI()
{
    m_waiting_time    = WAITING_TIME;
    m_first_time      = true;
    m_selected_widget = WTOK_NONE;

    // Switch to barrier mode: server waits for ack from each client
    network_manager->beginRaceResultBarrier();
    Widget *bottom_of_list;

    bottom_of_list = displayRaceResults();

    // If it's a server, the user can only select 'ok', since the
    // server does all the game mode selection etc.
    if(network_manager->getMode()==NetworkManager::NW_CLIENT)
    {
        Widget *w=widget_manager->addTextButtonWgt( WTOK_CONTINUE, 60, 7, _("OK") );
        w->setPosition(WGT_DIR_CENTER, 0, NULL, WGT_DIR_UNDER_WIDGET, 0.1f, bottom_of_list);
    }
    else
    {
        // If a new feature was unlocked, only offer 'continue' otherwise add the 
        // full menu choices. The new feature menu returns to this menu, and will
        // then display the whole menu.
        if(unlock_manager->getUnlockedFeatures().size()>0)
        {
            Widget *w=widget_manager->addTextButtonWgt( WTOK_CONTINUE, 60, 7, _("Continue") );
            w->setPosition(WGT_DIR_CENTER, 0, NULL, WGT_DIR_UNDER_WIDGET, 0.1f, bottom_of_list);
        } else
        {
            Widget *w;
            if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
            {
                w=widget_manager->addTextButtonWgt( WTOK_CONTINUE, 60, 7, _("Continue Grand Prix"));
            }
            else
            {
                w=widget_manager->addTextButtonWgt( WTOK_SETUP_NEW_RACE, 60, 7, _("Setup New Race"));
            }
            w->setPosition(WGT_DIR_CENTER, 0.0, NULL, WGT_DIR_UNDER_WIDGET, 0, bottom_of_list);
            Widget *w_prev = w;
            if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
            {
                w=widget_manager->addTextButtonWgt ( WTOK_CONTINUE_GP, 60, 7, _("Back to the main menu"));
            }
            else
            {
            w=widget_manager->addTextButtonWgt( WTOK_RESTART_RACE, 60, 7, _("Race in this track again"));
            }
            w->setPosition(WGT_DIR_CENTER, 0.0, NULL, WGT_DIR_UNDER_WIDGET, 0, w_prev);
            w_prev = w;
            if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_SINGLE)
            {
                w=widget_manager->addTextButtonWgt( WTOK_CONTINUE, 60, 7, _("Back to the main menu"));
                w->setPosition(WGT_DIR_CENTER, 0, NULL, WGT_DIR_UNDER_WIDGET, 0, w_prev);
            }
        }   // if !unlock_manager has something unlocked*/
    }   // if not server
    widget_manager->layout(WGT_AREA_ALL);

}   // RaceResultsGUI

// ----------------------------------------------------------------------------

Widget *RaceResultsGUI::displayRaceResults()
{
    Widget *w_prev=widget_manager->addTextWgt(WTOK_RESULTS, 5, 7, _("Race results"));
    widget_manager->hideWgtRect(WTOK_RESULTS);
    w_prev->setPosition(WGT_DIR_FROM_LEFT, 0.01f, NULL, WGT_DIR_FROM_TOP, 0.01f, NULL);

    const unsigned int MAX_STR_LEN = 60;
    const unsigned int NUM_KARTS = race_manager->getNumKarts();

    int*  order = new int [NUM_KARTS];
    RaceManager::getWorld()->raceResultOrder(order);
    
    unsigned int max_name_len = 1;

    for(unsigned int i=0; i < NUM_KARTS; i++)
    {
        Kart *k = race_manager->getKart(i);             // Display even for eliminated karts!
        const std::string& s = k->getName();
        unsigned int l = (unsigned int)s.size();
        if(l>max_name_len) max_name_len = l;
    }   // for i

    // save bottom of result list for later
    Widget *bottom_of_list=displayKartList(w_prev, order, 0.01f);

    delete[] order;
    
    const HighscoreEntry *hs = RaceManager::getWorld()->getHighscores();
    if(hs != NULL)
    {
        w_prev=widget_manager->addTextWgt(WTOK_HIGHSCORES, 5, 7, _("Highscores"));
        widget_manager->hideWgtRect(WTOK_HIGHSCORES);
        w_prev->setPosition(WGT_DIR_FROM_RIGHT, 0.01f, NULL, WGT_DIR_FROM_TOP, 0.01f, NULL);
        
        unsigned int num_scores = hs->getNumberEntries();
        char *highscores = new char[num_scores * MAX_STR_LEN];
        
        for(unsigned int i=0; i<num_scores; i++)
        {
            std::string kart_name, name;
            float T;
            hs->getEntry(i, kart_name, name, &T);
            const int   MINS   = (int) floor(T / 60.0);
            const int   SECS   = (int) floor(T - (float) (60 * MINS));
            const int   TENTHS = (int) floor(10.0f * (T - (float)(SECS + 60*MINS)));
            sprintf((char*)(highscores + MAX_STR_LEN * i),
                    "%s: %3d:%02d.%01d", name.c_str(), MINS, SECS, TENTHS);
            
            Widget *w=widget_manager->addTextWgt(WTOK_FIRST_HIGHSCORE + i, 5, 7,
                                                 (char*)(highscores+MAX_STR_LEN*i));
            w->setPosition(WGT_DIR_FROM_RIGHT, 0.05f, NULL, WGT_DIR_UNDER_WIDGET, 0, w_prev);
            w_prev=w;
        } // next score
        
        widget_manager->sameWidth(WTOK_HIGHSCORES, WTOK_FIRST_HIGHSCORE+num_scores-1);
        
        bottom_of_list = (num_scores > NUM_KARTS) ? w_prev : bottom_of_list;
    } // end if hs != NULL
    
    return bottom_of_list;
}  // displayRaceResults

//-----------------------------------------------------------------------------
Widget *RaceResultsGUI::displayKartList(Widget *w_prev, int *order, float horizontal)
{
    const bool display_time = RaceManager::getWorld()->getClockMode() == CHRONO;
    const unsigned int NUM_KARTS = race_manager->getNumKarts();
    
    const int MAX_STR_LEN=60;
    char *score = new char[NUM_KARTS * MAX_STR_LEN];
    int kart_id = 0; // 'i' below is not reliable because some karts (e.g. leader) will be skipped
    for(unsigned int i=0; i<NUM_KARTS; ++i)
    {
        if(order[i] == -1) continue;
        
        const Kart *current_kart = race_manager->getKart(order[i]);
        const std::string& kart_name = current_kart->getName();
        char sTime[20]; sTime[0]=0;
        const float T = current_kart->getFinishTime();

        if(display_time)
            TimeToString(T, sTime);

        //This shows position + driver name + time + points earned + total points
        if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            const int prev_score = race_manager->getKartPrevScore(order[i]);
            const int new_score = race_manager->getKartScore(order[i]);

            sprintf((char*)(score + MAX_STR_LEN * i), "#%d. %s (%d + %d = %d)",
                current_kart->getPosition(), kart_name.c_str(),// sTime,
                prev_score, (new_score - prev_score), new_score);
        }
        else
        {
            sprintf((char*)(score + MAX_STR_LEN * i), "%d. %s %s",
                    current_kart->getPosition(), kart_name.c_str(), sTime);
        }

        Widget *image=widget_manager->addImgButtonWgt(WTOK_FIRST_IMAGE + kart_id, 5, 7,
                                       current_kart->getKartProperties()->getIconFile());
        widget_manager->deactivateWgt(WTOK_FIRST_IMAGE+kart_id);

        image->setPosition(WGT_DIR_FROM_LEFT, horizontal, NULL, 
                           WGT_DIR_UNDER_WIDGET, 0.0f, w_prev);
        Widget *w=widget_manager->addTextWgt(WTOK_FIRST_RESULT + kart_id, 6, 7,
                                             (char*)(score + MAX_STR_LEN * i));
        w->setPosition(WGT_DIR_RIGHT_WIDGET, 0.0f, image,
                       WGT_DIR_UNDER_WIDGET, 0.0f, w_prev);
        w_prev=w;
        
        kart_id++;
    }
    widget_manager->sameWidth(WTOK_FIRST_RESULT, WTOK_FIRST_RESULT+kart_id-1);
    return w_prev;
}   // displayKartList

//-----------------------------------------------------------------------------
RaceResultsGUI::~RaceResultsGUI()
{
    widget_manager->reset();
}   // ~RaceResultsGUI

//-----------------------------------------------------------------------------
/** If an item is selected, store the selection to be handled in the next
 *  update call. This is necessary for network support so that the right
 *  action is executed once clients and server are all synchronised.
 */
void RaceResultsGUI::select()
{
    // Ignore a selection if the menu is still in 'waiting' mode. This helps
    // in case that someone presses space or enter while a FTL race finishes
    // --> this menu isn't immediately canceled.
    if(m_waiting_time>=0) return;
    // Push the unlocked-feature menu in for now
    if(unlock_manager->getUnlockedFeatures().size()>0)
    {
        // Push the new feature menu on the stack, from where
        // control will be returned to this menu.
        menu_manager->pushMenu(MENUID_UNLOCKED_FEATURE);
        return;
    }
    // The selected token is saved here, which triggers a change of the text
    // in update().
    m_selected_widget = (WidgetTokens)widget_manager->getSelectedWgt();
    // Clients send the ack to the server
    if(network_manager->getMode()==NetworkManager::NW_CLIENT)
        network_manager->sendRaceResultAck();

}   // select

//-----------------------------------------------------------------------------
void RaceResultsGUI::handle(GameAction ga, int value)
{
    // Only accept 'esc' when it's the end of a race, otherwise silently 
    // discard attempts to close the menu with esc.
    if (ga == GA_LEAVE)
    {
        // Don't accept it when a GP is done, or a new feature is unlocked
        if(widget_manager &&
           race_manager->getMajorMode()!=RaceManager::MAJOR_MODE_GRAND_PRIX &&
           unlock_manager->getUnlockedFeatures().size()==0)
        {
            RaceManager::getWorld()->unpause();
            widget_manager->setWgtText(WTOK_CONTINUE, _("Loading race..."));
            race_manager->next();
        }
        return;
    }
    BaseGUI::handle(ga, value);
}   // handle

//-----------------------------------------------------------------------------
/** Sets the selected token. This is used on the clients to allow the 
 *  NetworkManager to set the widget selected on the server. The clients will
 *  then be able to select the correct next menu.
 *  \param token Token to set as being selected.
 */
void RaceResultsGUI::setSelectedWidget(int token)
{
    m_selected_widget = (WidgetTokens)token;
}   // setSelectedToken

//-----------------------------------------------------------------------------
/** This is used on the client and server to display a message while waiting
 *  in a barrier for clients and server to ack the display.
 */
void RaceResultsGUI::update(float dt)
{
    m_waiting_time -= dt;
    BaseGUI::update(dt);
    // If an item is selected (for the first time), change the text
    // so that the user has feedback about his selection.
    if(m_selected_widget!=WTOK_NONE && m_first_time)
    {
        m_first_time = false;
        // User feedback on first selection: display message, and on the
        // server remove unnecessary widgets.
        widget_manager->setWgtText(WTOK_CONTINUE, _("Synchronising."));
        if(network_manager->getMode()==NetworkManager::NW_SERVER)
        {
            widget_manager->hideWgt(WTOK_RESTART_RACE);
            widget_manager->hideWgt(WTOK_SETUP_NEW_RACE);        
        }
    }   // m_selected_token defined and not first time

    // Wait till the barrier is finished. On the server this is the case when
    // the state ie MAIN_MENU, on the client when it is wait_for_available_characters.
    if(network_manager->getMode() !=NetworkManager::NW_NONE         &&
       network_manager->getState()!=NetworkManager::NS_MAIN_MENU    &&
       network_manager->getState()!=NetworkManager::NS_RACE_RESULT_BARRIER_OVER ) 
       return;

    // Send selected menu to all clients
    if(m_selected_widget!=WTOK_NONE &&
        network_manager->getMode()==NetworkManager::NW_SERVER)
    {
        network_manager->sendRaceResultAck(m_selected_widget);
    }

    switch(m_selected_widget)
    {
    case WTOK_CONTINUE_GP:
        // Needed to create this
        // to make players not 
        // restarting a race.
        race_manager->exit_race();
        menu_manager->switchToMainMenu();
        break;
    case WTOK_CONTINUE:
        // Gets called when:
        // 1) something was unlocked
        // 2) a Grand Prix is run
        // 3) "back to the main menu" otherwise
        RaceManager::getWorld()->unpause();
        widget_manager->setWgtText(WTOK_CONTINUE, _("Loading race..."));
        race_manager->next();
        break;
    case WTOK_RESTART_RACE:
        network_manager->setState(NetworkManager::NS_MAIN_MENU);
        RaceManager::getWorld()->unpause();
        menu_manager->popMenu();
        race_manager->rerunRace();
        break;
    case WTOK_SETUP_NEW_RACE:
        RaceManager::getWorld()->unpause();
        race_manager->exit_race();
        if(network_manager->getMode()==NetworkManager::NW_CLIENT)
        {
            network_manager->setState(NetworkManager::NS_WAIT_FOR_AVAILABLE_CHARACTERS);
            menu_manager->pushMenu(MENUID_CHARSEL_P1);
        }
        else
        {
            menu_manager->pushMenu(MENUID_GAMEMODE);
        }
        break;

    default:
        break;
    }

}   // update
/* EOF */
