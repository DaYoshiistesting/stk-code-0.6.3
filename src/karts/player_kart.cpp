//  $Id: player_kart.cpp 3086 2009-01-31 04:58:33Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#include "karts/player_kart.hpp"

#include "history.hpp"
#include "player.hpp"
#include "sdldrv.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "graphics/camera.hpp"
#include "graphics/scene.hpp"
#include "gui/menu_manager.hpp"
#include "gui/race_gui.hpp"
#include "items/item.hpp"
#include "modes/world.hpp"
#include "utils/constants.hpp"
#include "utils/translation.hpp"

PlayerKart::PlayerKart(const std::string& kart_name, int position, Player *player,
                       const btTransform& init_pos, int player_index) :
            Kart(kart_name, position, init_pos)
{
    m_player       = player;
    m_penalty_time = 0.0f;
    m_camera       = scene->createCamera(player_index, this);
    m_camera->setMode(Camera::CM_NORMAL);

    m_bzzt_sound  = sfx_manager->newSFX(SFXManager::SOUND_BZZT);
    m_ugh_sound   = sfx_manager->newSFX(SFXManager::SOUND_UGH );
    m_grab_sound  = sfx_manager->newSFX(SFXManager::SOUND_GRAB);
    m_full_sound  = sfx_manager->newSFX(SFXManager::SOUND_FULL);

    reset();
}   // PlayerKart

//-----------------------------------------------------------------------------
PlayerKart::~PlayerKart()
{
    sfx_manager->deleteSFX(m_bzzt_sound);
    sfx_manager->deleteSFX(m_ugh_sound );
    sfx_manager->deleteSFX(m_grab_sound);
    sfx_manager->deleteSFX(m_full_sound);
}   // ~PlayerKart

//-----------------------------------------------------------------------------
void PlayerKart::reset()
{
    m_steer_val_l  = 0;
    m_steer_val_r  = 0;
    m_steer_val    = 0;
    m_prev_brake   = 0;
    m_prev_accel   = 0;
    m_penalty_time = 0;
    Kart::reset();
    m_camera->reset();
}   // reset

// ----------------------------------------------------------------------------
/** Resets the state of control keys. This is used after the in-game menu to
 *  avoid that any keys pressed at the time the menu is opened are still 
 *  considered to be pressed.
 */
void PlayerKart::resetInputState()
{
    m_steer_val_l  = 0;
    m_steer_val_r  = 0;
    m_steer_val    = 0;
    m_prev_brake   = 0;
    m_prev_accel   = 0;
}   // resetKeyState

// ----------------------------------------------------------------------------
/** This function interprets a kart action and value, and set the corresponding
 *  entries in the kart control data structure. This function handles esp. 
 *  cases like 'press left, press right, release right' - in this case after
 *  releasing right, the steering must switch to left again. Similarly it 
 *  handles 'press left, press right, release left' (in which case still
 *  right must be selected). Similarly for braking and acceleration.
 * \param action  The action to be executed.
 * \param value   If 32768, it indicates a digital value of 'fully set'
 *                if between 1 and 32767, it indicates an analog value,
 *                and if it's 0 it indicates that the corresponding button
 *                was released.
 */
void PlayerKart::action(KartAction action, int value)
{
    switch (action)
    {
    case KA_LEFT:
        m_steer_val_l = -value;
        if (value)
          m_steer_val = -value;
        else
          m_steer_val = m_steer_val_r;

        break;
    case KA_RIGHT:
        m_steer_val_r = value;
        if (value)
          m_steer_val = value;
        else
          m_steer_val = m_steer_val_l;

        break;
    case KA_ACCEL:
        m_prev_accel = value;
        if(value)
        {
            m_controls.m_accel = value/32768.0f;
            m_controls.m_brake = false;
        }
        else
        {
            m_controls.m_accel = 0.0f;
            m_controls.m_brake = m_prev_brake;
        }
        break;
    case KA_BRAKE:
        m_prev_brake = value!=0;
        if(value)
        {
            m_controls.m_brake = true;
            m_controls.m_accel = 0.0f;
        }
        else
        {
            m_controls.m_brake = false;
            m_controls.m_accel = m_prev_accel/32768.0f;
        }
        break;
    case KA_NITRO:
        m_controls.m_nitro = (value!=0);
        break;
    case KA_RESCUE:
        m_controls.m_rescue = (value!=0);
        break;
    case KA_FIRE:
        m_controls.m_fire = (value!=0);
        break;
    case KA_LOOK_BACK:
        m_controls.m_look_back = (value!=0);
        break;
    case KA_DRIFT:
        m_controls.m_drift = (value!=0);
        break;
    }

}   // action

//-----------------------------------------------------------------------------
void PlayerKart::steer(float dt, int steer_val)
{
    if(user_config->m_gamepad_debug)
    {
        printf("steering: steer_val %d ", steer_val);
    }
    const float STEER_CHANGE = dt/getTimeFullSteer();  // amount the steering is changed
    if (steer_val < 0)
    {
      // If we got analog values do not cumulate.
      if (steer_val > -32767)
        m_controls.m_steer = -steer_val/32767.0f;
      else
        m_controls.m_steer += STEER_CHANGE;
    }
    else if(steer_val > 0)
    {
      // If we got analog values do not cumulate.
      if (steer_val < 32767)
        m_controls.m_steer = -steer_val/32767.0f;
      else
        m_controls.m_steer -= STEER_CHANGE;
    }
    else
    {   // no key is pressed
        if(m_controls.m_steer>0.0f)
        {
            m_controls.m_steer -= STEER_CHANGE;
            if(m_controls.m_steer<0.0f) m_controls.m_steer=0.0f;
        }
        else
        {   // m_controls.m_steer<=0.0f;
            m_controls.m_steer += STEER_CHANGE;
            if(m_controls.m_steer>0.0f) m_controls.m_steer=0.0f;
        }   // if m_controls.m_steer<=0.0f
    }   // no key is pressed
    if(user_config->m_gamepad_debug)
    {
        printf("  set to: %f\n", m_controls.m_steer);
    }

    m_controls.m_steer = std::min(1.0f, std::max(-1.0f, m_controls.m_steer));

}   // steer

//-----------------------------------------------------------------------------
void PlayerKart::update(float dt)
{
    // Don't do steering if it's replay. In position only replay it doesn't 
    // matter, but if it's physics replay the gradual steering causes 
    // incorrect results, since the stored values are already adjusted.
    if(!history->replayHistory())
        steer(dt, m_steer_val);

    if(RaceManager::getWorld()->isStartPhase())
    {
        if(m_controls.m_accel || m_controls.m_brake ||
           m_controls.m_fire || m_controls.m_nitro  || m_controls.m_drift)
        {
            if(m_penalty_time == 0.0)//eliminates machine-gun-effect for SOUND_BZZT
            {
                m_penalty_time=1.0;
                m_bzzt_sound->play();
            }
            // A warning gets displayed in RaceGUI
        }
        else
        {
            // The call to update is necessary here (even though the kart
            // shouldn't actually change) to update m_transform. Otherwise
            // the camera gets the wrong position. 
            Kart::update(dt);
        }
        
        return;
    }
    if(m_penalty_time>0.0)
    {
        m_penalty_time-=dt;
        return;
    }

    if(m_controls.m_fire && !isRescue())
    {
        if (m_powerup.getType()==POWERUP_NOTHING) 
            Kart::beep();
    }

    // We can't restrict rescue to fulfill isOnGround() (which would be more like
    // MK), since e.g. in the City track it is possible for the kart to end
    // up sitting on a brick wall, with all wheels in the air :((
    if(m_controls.m_rescue)
    {
        forceRescue();
        m_controls.m_rescue=false;
    }
    // Play bzzt sound if the kart is getting rescued.
    if(isRescue() && m_attachment.getType() != ATTACH_TINYTUX)
    {
        m_bzzt_sound->play();
    }
    Kart::update(dt);
}   // update

//-----------------------------------------------------------------------------
void PlayerKart::crashed(Kart *kart)
{
    Kart::crashed(kart);
}   // crashed

//-----------------------------------------------------------------------------
/** Checks if the kart was overtaken, and if so plays a sound
*/
void PlayerKart::setPosition(int p)
{
    if(getPosition()<p)
    {
        //have the kart that did the passing beep.
        //I'm not sure if this method of finding the passing kart is fail-safe.
        for(unsigned int i = 0 ; i < race_manager->getNumKarts(); i++ )
        {
            Kart *kart = RaceManager::getWorld()->getKart(i);
            if(kart->getPosition() == p + 1)
            {
                kart->beepAI();
                break;
            }
        }
    }
    Kart::setPosition(p);
}   // setPosition

//-----------------------------------------------------------------------------
/** Called when a kart finishes race.
 *  /param time Finishing time for this kart.
 */
void PlayerKart::raceFinished(float time)
{
    Kart::raceFinished(time);
    // Set race over camera (but not in follow the leader mode, since the kart
    // will most likely not be at the starting line at the end of the race
    if(race_manager->getMinorMode()!=RaceManager::MINOR_MODE_FOLLOW_LEADER)
        m_camera->setMode(Camera::CM_FINAL);
    RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
    if(m)
    {
        m->addMessage(getPosition()==1 ? _("You won the race!") : _("You finished the race!") ,
                      this, 2.0f, 50);
    }
}   // raceFinished

//-----------------------------------------------------------------------------
/** Called when a kart hits an item.
 *  \param item Item that was collected.
 *  \param add_info Additional info to be used then handling the item. If
 *                  this is -1 (default), the item type is selected 
 *                  randomly. Otherwise it contains the powerup or 
 *                  attachment for the kart. This is used in network mode to 
 *                  let the server determine the powerup/attachment for
 *                  the clients.
 */
void PlayerKart::collectedItem(const Item *item, int add_info)
{
    // FIXME - how does the old item relate to the total amount of items?
    const float old_energy= getEnergy();
    Kart::collectedItem(item, add_info);

    if(old_energy < MAX_ITEMS_COLLECTED &&
       getEnergy() == MAX_ITEMS_COLLECTED)
    {
        m_full_sound->play();
    }
    else
    {
        switch(item->getType())
        {
            case Item::ITEM_BANANA:
                m_ugh_sound->play();
                break;
            case Item::ITEM_BUBBLEGUM:
                //The ugh sound is played by the kart class. Do nothing here.
                //See Kart::collectedItem()
                break;
            default:
                m_grab_sound->play();
                break; 
        }           
    }
}   // collectedItem

//-----------------------------------------------------------------------------
/** Called when the kart is doing a shortcut. The player kart will display
 *  a message for the user.
 */
void PlayerKart::doingShortcut()
{
    RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
    // Can happen if the option menu is called
    if(m)
        m->addMessage(_("Invalid short-cut!!"), this, 2.0f, 50);
}   // doingShortcut
