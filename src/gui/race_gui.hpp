//  $Id: race_gui.hpp 3034 2009-01-23 05:23:22Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_RACEGUI_H
#define HEADER_RACEGUI_H

#include <string>
#include <vector>

#include "base_gui.hpp"
#include "material.hpp"
#include "player.hpp"
#include "user_config.hpp"
#include "race_manager.hpp"
#include "karts/kart.hpp"
#include "karts/player_kart.hpp"
#include "modes/world.hpp"

class InputMap;
class RaceSetup;

/**
  * Used to display the list of karts and their times or
  * whatever other info is relevant to the current mode.
  */
struct KartIconDisplayInfo
{
    std::string time;
    float r, g, b;
    std::string special_title;
    /** Current lap of this kart, or -1 if irrelevant
      */
    int lap;
};

class RaceGUI: public BaseGUI
{

    class TimedMessage
    {
     public:
        std::string m_message;            // message to display
        float       m_remaining_time;     // time remaining before removing this message from screen
        int         m_red,m_blue,m_green; // colour
        int         m_font_size;          // size
        const Kart *m_kart;
        // std::vector needs standard copy-ctor and std-assignment op.
        // let compiler create defaults .. they'll do the job, no
        // deep copies here ..
        TimedMessage(const std::string &message, 
                     const Kart *kart, float time, int size, 
                     int red, int green, int blue)
        {
            m_message    = message; 
            m_font_size  = size;
            m_kart       = kart;
            if( time < 0.0f ) m_remaining_time = -1.0f;
            else
            {
                m_remaining_time = time;
            }
            m_red=red; m_blue=blue; m_green=green; 
        }
        // in follow leader the clock counts backwards
        bool done(const float dt)
        {
            m_remaining_time -= dt;
            return m_remaining_time < 0;
        }
    };
public:

    RaceGUI();
    ~RaceGUI();
    void update(float dt);
    void select() {}
    void handle(GameAction, int);
    void handleKartAction(KartAction ka, int value);
    void addMessage(const std::string &m, const Kart *kart, float time, 
                    int fonst_size, int red=255, int green=0, int blue=255);

private:
    ulClock        m_fps_timer;
    int            m_fps_counter;
    char           m_fps_string[10];
    std::vector<std::string>
                   m_pos_string;
    Material      *m_speed_back_icon;
    Material      *m_speed_fore_icon;
    Material      *m_plunger_face;
    typedef        std::vector<TimedMessage> AllMessageType;
    AllMessageType m_messages;

    GLuint          *m_marker;
    GLuint          *m_mini_map;
    int              m_marker_rendered_size;
    int              m_marker_ai_size;
    int              m_marker_player_size;
    int              m_map_rendered_width;
    int              m_map_rendered_height;
    int              m_map_width;
    int              m_map_height;
    int              m_map_left;
    int              m_map_bottom;


    /* Display information on screen */
    void drawStatusText        (const float dt);
    void drawEnergyMeter       (Kart *player_kart,
                                float offset_x, float offset_y,
                                float ratio_x,  float ratio_y);
    void drawPowerupIcons      (Kart* player_kart,
                                float offset_x, float offset_y,
                                float ratio_x,  float ratio_y);
    void drawAllMessages       (Kart* player_kart,
                                float offset_x, float offset_y,
                                float ratio_x,  float ratio_y);
    void drawPlayerIcons       (const KartIconDisplayInfo* info);
    void drawMap               ();
    void drawTimer             ();
    void drawFPS               ();
    void drawMusicDescription  ();
    void cleanupMessages       (const float dt);
    void drawSpeed             (Kart* kart, float offset_x, float offset_y,
                                float ratio_x, float ratio_y);
    void drawLap               (const KartIconDisplayInfo* info, Kart* kart, float offset_x,
                                float offset_y, float ratio_x, float ratio_y);
};

#endif
