//  $Id: race_gui.cpp 3034 2009-01-23 05:23:22Z hikerstk $
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

#include "gui/race_gui.hpp"

#include "input.hpp"
#include "sdldrv.hpp"
#include "user_config.hpp"
#include "history.hpp"
#include "race_manager.hpp"
#include "material_manager.hpp"
#include "audio/sound_manager.hpp"
#include "gui/font.hpp"
#include "gui/menu_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/translation.hpp"
#include <GL/glut.h>

#undef USE_WIDGET_MANAGER
#ifdef USE_WIDGET_MANAGER
#include "gui/widget_manager.hpp"
#endif


#ifdef USE_WIDGET_MANAGER
//MAX_TOP_POS is the maximum number of racers to be shown in the bar to the
//left where the positions are drawn.
static const int MAX_TOP_POS = 10;

static const int MAX_HUMANS = 4;

enum WidgetTokens
{
    WTOK_FPS,
    WTOK_EMPTY1,
    WTOK_CLOCK,

    WTOK_EMPTY2,

    WTOK_FIRST_TOP_IMG,
    WTOK_LAST_TOP_IMG = WTOK_FIRST_TOP_IMG + MAX_TOP_POS,

    WTOK_FIRST_TOP_TEXT,
    WTOK_LAST_TOP_TEXT = WTOK_FIRST_TOP_TEXT + MAX_TOP_POS,

    WTOK_FIRST_MESSAGE,
    WTOK_LAST_MESSAGE = WTOK_FIRST_MESSAGE + MAX_HUMANS,

    WTOK_FIRST_POWERBAR,
    WTOK_LAST_POWERBAR = WTOK_FIRST_POWERBAR + MAX_HUMANS,

    WTOK_FIRST_POSITION,
    WTOK_LAST_POSITION = WTOK_FIRST_POSITION + MAX_HUMANS,

    WTOK_MAP,

    WTOK_FIRST_LAP,
    WTOK_LAST_LAP = WTOK_FIRST_LAP + MAX_HUMANS,

    WTOK_FIRST_WHEEL,
    WTOK_LAST_WHEEL = WTOK_FIRST_WHEEL + MAX_HUMANS,

    WTOK_FIRST_SPEED,
    WTOK_LAST_SPEED = WTOK_FIRST_SPEED + MAX_HUMANS
};
#endif

RaceGUI::RaceGUI()
{
    m_pos_string.push_back("?!?");
    m_pos_string.push_back("1st");
    m_pos_string.push_back("2nd");
    m_pos_string.push_back("3rd");

    for(int i=4; i<= 20; ++i) 
    {
        char buffer[6];
        sprintf(buffer, "%dth", i);
        m_pos_string.push_back(buffer);
    }

    m_speed_back_icon = material_manager->getMaterial("speedback.rgb");
    m_speed_back_icon->getState()->disable(GL_CULL_FACE);
    m_speed_fore_icon = material_manager->getMaterial("speedfore.rgb");
    m_speed_fore_icon->getState()->disable(GL_CULL_FACE);
    
    m_plunger_face = material_manager->getMaterial("plungerface.rgb");
    m_plunger_face->getState()->disable(GL_CULL_FACE);
    
    m_fps_counter   = 0;
    m_fps_string[0] = 0;
    m_fps_timer.reset();
    m_fps_timer.update();
    m_fps_timer.setMaxDelta(1000);

#ifdef USE_WIDGET_MANAGER
    const bool HIDE_TEXT = false;
    widget_manager->setInitialTextState(HIDE_TEXT, "", WGT_FNT_LRG,
                                        WGT_FONT_RACE, WGT_WHITE);

    widget_manager->addWgt(WTOK_FPS, 30, 10);
    widget_manager->addWgt(WTOK_EMPTY1, 40, 10);
    widget_manager->addWgt(WTOK_CLOCK, 30, 10);
    widget_manager->breakLine();

    widget_manager->layout(WGT_AREA_TOP);
#endif
}   // RaceGUI

//-----------------------------------------------------------------------------
RaceGUI::~RaceGUI()
{
#ifdef USE_WIDGET_MANAGER
    widget_manager->reset();
#endif

    //FIXME: does all that material stuff need freeing somehow?
}   // ~Racegui

//-----------------------------------------------------------------------------
void RaceGUI::handle(GameAction ga, int value)
{
    static int isWireframe = false;
    
    // The next lines find out the player and kartaction that belongs
    // to a certain gameaction value (GameAction -> Player number, Kartaction).
    // Since the numbers are fixed this can be done through computation
    // (instead of using e.g. a separate data structure).
    // Note that the kartaction enum value and their representatives in
    // gameaction enum have the same order (Otherwise the stuff below would
    // not work ...)!
    if(ga >= GA_FIRST_KARTACTION && ga <= GA_LAST_KARTACTION)
    {
        // 'Pulls down' the gameaction value to make them multiples of the
        // kartaction values.
        int ka = ga - GA_FIRST_KARTACTION;
        
        int playerNo = ka / KC_COUNT;
        ka = ka % KC_COUNT;
        
        RaceManager::getWorld()->getLocalPlayerKart(playerNo)->action((KartAction) ka, value);
        return;
    }
    
    if(value)
        return;
    
    switch(ga)
    {
        case GA_DEBUG_ADD_BOWLING:
            if(race_manager->getNumPlayers() ==1)
            {
                Kart* kart = RaceManager::getWorld()->getLocalPlayerKart(0);
                kart->setPowerup(POWERUP_BOWLING, 10000);
            }
            break;
        case GA_DEBUG_ADD_MISSILE:
            if(race_manager->getNumPlayers() ==1 )
            {
                Kart* kart = RaceManager::getPlayerKart(0);
                kart->setPowerup(POWERUP_PLUNGER, 10000);
            }
            break;
        case GA_DEBUG_ADD_HOMING:
            if (race_manager->getNumPlayers() ==1 )
            {
                Kart* kart = RaceManager::getPlayerKart(0);
                kart->setPowerup(POWERUP_CAKE, 10000);
            }
            break;
        case GA_DEBUG_TOGGLE_FPS:
            user_config->m_display_fps = !user_config->m_display_fps;
            if(user_config->m_display_fps)
            {
                m_fps_timer.reset();
                m_fps_timer.setMaxDelta(1000);
                m_fps_counter=0;
#ifdef USE_WIDGET_MANAGER
                widget_manager->showWgtText(WTOK_FPS);
            }
            else
            {
                widget_manager->hideWgtText(WTOK_FPS);
#endif
            }
            break;
        case GA_DEBUG_TOGGLE_WIREFRAME:
            glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_FILL : GL_LINE);
            isWireframe = !isWireframe;
            break;
#ifndef WIN32
        // For now disable F9 toggling fullscreen, since windows requires
        // to reload all textures, display lists etc. Fullscreen can
        // be toggled from the main menu (options->display).
        case GA_TOGGLE_FULLSCREEN:
            inputDriver->toggleFullscreen(false);   // 0: do not reset textures
            // Fall through to put the game into pause mode.
#endif
        case GA_LEAVE_RACE:
            RaceManager::getWorld()->pause();
            menu_manager->pushMenu(MENUID_RACEMENU);
        break;
        case GA_DEBUG_HISTORY:
            history->Save();
            break;
        default:
            break;
    } // switch
}
//-----------------------------------------------------------------------------
void RaceGUI::update(float dt)
{
    drawStatusText(dt);
    cleanupMessages(dt);

    BaseGUI::update(dt);
}   // update

//-----------------------------------------------------------------------------
void RaceGUI::drawFPS()
{
    if(++m_fps_counter>=50)
    {
        m_fps_timer.update();
        sprintf(m_fps_string, "FPS: %d",
          (int)(m_fps_counter/m_fps_timer.getDeltaTime()));
        m_fps_counter = 0;
        m_fps_timer.setMaxDelta(1000);
    }
    // scaling values
    float ratio_x  = (float)(user_config->m_width/800.f);
    float ratio_y  = (float)(user_config->m_height/600.f);
    float minRatio = std::min(ratio_x, ratio_y);

    // Since Font::CENTER_OF_SCREEN doesn't align with powerup icons, 
    // we have to create our own center for the FPS counter.
    float center_x = (float)((800/2-50)*ratio_x);
#ifdef USE_WIDGET_MANAGER
    widget_manager->setWgtText(WTOK_FPS, m_fps_string);
#else
    font_race->PrintShadow(m_fps_string, 30.f*minRatio, center_x, 
                          (600-32.f)*ratio_y);
#endif
}   // drawFPS

//-----------------------------------------------------------------------------
void RaceGUI::drawTimer()
{
    assert(RaceManager::getWorld() != NULL);

    if(!RaceManager::getWorld()->shouldDrawTimer()) return;
    char str[256];
    
    TimeToString(RaceManager::getWorld()->getTime(), str);
#ifdef USE_WIDGET_MANAGER
    widget_manager->showWgtText(WTOK_CLOCK);
    widget_manager->setWgtText(WTOK_CLOCK, str);
#else
    // scaling values
    float ratio_x  = (float)(user_config->m_width/800.f);
    float ratio_y  = (float)(user_config->m_height/600.f);
    float minRatio = std::min(ratio_x, ratio_y);

    font_race->PrintShadow(str, 60.f*minRatio, (800-200)*ratio_x,
                          (600-64)*ratio_y);
#endif
}   // drawTimer

//-----------------------------------------------------------------------------
/** Draws the mini map with the karts on it.
 */
void RaceGUI::drawMap()
{
    // Arenas don't have a map.
    if(RaceManager::getTrack()->isArena()) return;
    glDisable(GL_TEXTURE_2D);
    assert(RaceManager::getWorld() != NULL);

    // Scaling variables.
    float ratio = user_config->m_height/480.f;

    // sizes
    const float map_w = 100.f*ratio;
    const float map_h = 100.f*ratio;

    Track* m_track = RaceManager::getTrack();

    // Get the track's width and height
    const float track_width  = m_track->m_driveline_max.getX() - m_track->m_driveline_min.getX();
    const float track_height = m_track->m_driveline_max.getY() - m_track->m_driveline_min.getY();

    // Calculate the scaling for x and y axis
    float sx = map_w / track_width;
    float sy = map_h / track_height;

    // offsets
    float map_x = (10.f*ratio)+(map_w - track_width * sx)/2.f;
    float map_y = (10.f*ratio)+(map_h - track_height * sy)/2.f;

    // Draw minimap.
    RaceManager::getTrack()->draw2DMiniMap(map_x, map_y, sx, sy);
    glBegin(GL_QUADS);

    for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
    {
        const Kart* kart = RaceManager::getKart(i);
        // Don't draw eliminated kart.
        if(kart->isEliminated()) continue;
        glColor3fv(kart->getColor().toFloat());
        const Vec3& xyz = kart->getXYZ();

        // If it's a player, draw a bigger sign.
        if(kart->isPlayerKart())
        {
            RaceManager::getTrack()->glVtx(xyz, map_x+(3*ratio), map_y+(3*ratio), sx, sy);
            RaceManager::getTrack()->glVtx(xyz, map_x-(2*ratio), map_y+(3*ratio), sx, sy);
            RaceManager::getTrack()->glVtx(xyz, map_x-(2*ratio), map_y-(2*ratio), sx, sy);
            RaceManager::getTrack()->glVtx(xyz, map_x+(3*ratio), map_y-(2*ratio), sx, sy);
        }
        else
        {
            RaceManager::getTrack()->glVtx(xyz, map_x+(2*ratio), map_y+(2*ratio), sx, sy);
            RaceManager::getTrack()->glVtx(xyz, map_x-(1*ratio), map_y+(2*ratio), sx, sy);
            RaceManager::getTrack()->glVtx(xyz, map_x-(1*ratio), map_y-(1*ratio), sx, sy);
            RaceManager::getTrack()->glVtx(xyz, map_x+(2*ratio), map_y-(1*ratio), sx, sy);
        }
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}   // drawMap

//-----------------------------------------------------------------------------
/** Draw players position on the race.
 */
void RaceGUI::drawPlayerIcons(const KartIconDisplayInfo* info)
{
    assert(RaceManager::getWorld() != NULL);

    // scaling variables
    GLfloat viewport[4];
    glGetFloatv(GL_VIEWPORT, viewport);
    float ratio_x = viewport[2]/800;
    float ratio_y = viewport[3]/600;
    float minRatio = std::min(ratio_x, ratio_y);

    // offsets and sizes of the icons
    float x = 5.0f*ratio_x;
    float y;
    float ICON_WIDTH=40.0f*minRatio;
    float ICON_PLAYER_WIDTH=50.0f*minRatio;

    glEnable(GL_TEXTURE_2D);
    Material *last_players_gst = NULL;
    glDisable(GL_CULL_FACE);
    
    Kart* kart;
    const unsigned int kart_amount = race_manager->getNumKarts();
    const unsigned int max_on_screen = 8;
    
    unsigned int drawn = 0;
    unsigned int non_eliminated = 0;
    
    // First, count the karts that are doing the race 
    // (i.e in FTL, the non-eliminated karts).
    for(unsigned int i=0; i<kart_amount; ++i)
    {
        kart = RaceManager::getKart(i);
        if(!kart->isEliminated())
            ++non_eliminated;
    }

    // Draw "Top 8" above icons when there are more than 8 karts driving.
    if(non_eliminated > max_on_screen)
        font_race->PrintShadow("Top 8", 30*minRatio, x, (600-35)*minRatio);
    
    // Since, drawn starts at 0, we have to give a max of 7, because the 1st kart
    // will be counted as 0 in drawn.
    for(unsigned int pos=1; drawn<max_on_screen && pos<=kart_amount; ++pos)
    {
        int i = -1;
        for(unsigned int j=0; j<kart_amount; ++j)
        {
            kart = RaceManager::getKart(j);
            if(!kart->isEliminated() && pos == kart->getPosition())
            {
                i = j;
                break;
            }
        }
        if(i == -1) continue;

        y = viewport[3]*0.85f-(drawn*(ICON_PLAYER_WIDTH+(2*minRatio)));

        GLfloat COLOR[] = {info[i].r, info[i].g, info[i].b, 1.0f};
        font_race->PrintShadow(info[i].time.c_str(), 30*minRatio, ICON_PLAYER_WIDTH+x, y+5, COLOR);

        if(info[i].special_title.length()>0)
        {
            GLfloat const RED[] = {1.0f, 0, 0, 1.0f};
            font_race->PrintShadow(info[i].special_title.c_str(), 30*minRatio, ICON_PLAYER_WIDTH+x, y+5, RED);
        }
        
        glEnable(GL_CULL_FACE);
        // draw icons
        Material* players_gst = kart->getKartProperties()->getIconMaterial();
        // Hmm - if the same icon is displayed more than once in a row,
        // plib does only do the first setTexture, therefore nothing is
        // displayed for the remaining icons. So we have to call force() 
        // if the same icon is displayed more than once in a row.
        if(last_players_gst==players_gst)
        {
            players_gst->getState()->force();
        }
        // The material of the icons should not have a non-zero alpha_ref value,
        // because if so the next call can make the text look aliased.
        players_gst->apply();
        last_players_gst = players_gst;
        glBegin(GL_QUADS);
        glColor4f(1, 1, 1, 1);
        if(kart->isPlayerKart())
        {
            glTexCoord2f(0, 0);glVertex2f(x,                   y);
            glTexCoord2f(1, 0);glVertex2f(x+ICON_PLAYER_WIDTH, y);
            glTexCoord2f(1, 1);glVertex2f(x+ICON_PLAYER_WIDTH, y+ICON_PLAYER_WIDTH);
            glTexCoord2f(0, 1);glVertex2f(x,                   y+ICON_PLAYER_WIDTH);
        }
        else
        {
            glTexCoord2f(0, 0);glVertex2f(x,            y);
            glTexCoord2f(1, 0);glVertex2f(x+ICON_WIDTH, y);
            glTexCoord2f(1, 1);glVertex2f(x+ICON_WIDTH, y+ICON_WIDTH);
            glTexCoord2f(0, 1);glVertex2f(x,            y+ICON_WIDTH);
        }
        glEnd();
        // draw position (1st, 2nd...)
        glDisable(GL_CULL_FACE);
        char str[256];
        sprintf(str, "%d", pos);
            font_race->PrintShadow(str, 27*minRatio, x-7*minRatio, y-4*minRatio);
        if(pos == 1)
            font_race->PrintShadow("st", (13*minRatio),
                                   x-7*minRatio + (17*minRatio),
                                   y-5*minRatio + (17*minRatio));
        else if(pos == 2)
            font_race->PrintShadow("nd", (13*minRatio),
                                   x-7*minRatio + (17*minRatio),
                                   y-5*minRatio + (17*minRatio));
        else if(pos == 3)
            font_race->PrintShadow("rd", (13*minRatio),
                                   x-7*minRatio + (17*minRatio),
                                   y-5*minRatio + (17*minRatio));
        else
            font_race->PrintShadow("th", (13*minRatio),
                                   x-7*minRatio + (17*minRatio),
                                   y-5*minRatio + (17*minRatio));

        // Draw the icons until it hits the 8th position.
        drawn++;
    } // next kart
    glEnable(GL_CULL_FACE);
}   // drawPlayerIcons

//-------------------------------------------------------------------------------
/** Draws the powerup icons when a player collects a gift.
 */
void RaceGUI::drawPowerupIcons(Kart* player_kart, int offset_x, int offset_y, 
                               float ratio_x, float ratio_y)
{
    // If player doesn't have anything, do nothing.
    Powerup* powerup=player_kart->getPowerup();
    if(powerup->getType() == POWERUP_NOTHING) return;

    int n = player_kart->getNumPowerup();
    float minRatio = std::min(ratio_x, ratio_y);
    
    // Originally the hardcoded sizes were 320-32 and 400.
    float x = (float)(offset_x + ((800/2-8)*ratio_x)-(n*(minRatio*30))/2);
    float y = (float)(offset_y + (600*5/6)*ratio_y);

    float nSize = 64*minRatio;
    powerup->getIcon()->apply();

    if(n>7) n=7;
    if(n<1) n=1;

    glBegin(GL_QUADS);
    glColor4f(1,1,1,1);
    for(int i=0; i<n; i++)
    {
        float x2=x+i*(minRatio*30);
        glTexCoord2f(0, 0); glVertex2f(x2,       y);
        glTexCoord2f(1, 0); glVertex2f(x2+nSize, y);
        glTexCoord2f(1, 1); glVertex2f(x2+nSize, y+nSize);
        glTexCoord2f(0, 1); glVertex2f(x2,       y+nSize);
    }   // for i
    glEnd();

}   // drawPowerupIcons

//-------------------------------------------------------------------------------
/* Energy meter that gets filled with coins */

// Meter fluid color (0 - 255)
#define METER_TOP_COLOR    240, 000, 0, 255
#define METER_BOTTOM_COLOR 240, 200, 0, 160
// Meter border color (0.0 - 1.0)
#define METER_BORDER_BLACK 0.0, 0.0, 0.0
#define METER_BORDER_WHITE 1.0, 1.0, 1.0
#define METER_TARGET_RED   1.0, 0.0, 0.0

//-------------------------------------------------------------------------------
void RaceGUI::drawEnergyMeter(Kart *player_kart, int offset_x, int offset_y,
                              float ratio_x, float ratio_y)
{
    float state = (float)(player_kart->getEnergy())/MAX_ITEMS_COLLECTED;
    float x  = (((800-27)*ratio_x)) + offset_x;
    float y  = (250*ratio_y) + offset_y;
    float w  = 16*ratio_x;
    float h  = 600/3*ratio_y;
    // Each graduation equals 5 items.
    const int GRADS = MAX_ITEMS_COLLECTED/5; 
    // Graduation height.
    float gh = (float)(h/GRADS); 
    float energy_target = (float)race_manager->getEnergyTarget();
    float th = h*(energy_target/MAX_ITEMS_COLLECTED);

    glDisable(GL_TEXTURE_2D);
    // Draw a Meter border
    x-=1;
    y-=1;
    // left side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_BLACK);
    glVertex2f(x-1, y-1);
    glVertex2f(x,   y-1);
    glVertex2f(x,   y+h+1);
    glVertex2f(x-1, y+h+1);
    glEnd();

    // right side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_BLACK);
    glVertex2f(x+w,   y-1);
    glVertex2f(x+w+1, y-1);
    glVertex2f(x+w+1, y+h+1);
    glVertex2f(x+w,   y+h+1);
    glEnd();

    // down side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_BLACK);
    glVertex2f(x,   y-1);
    glVertex2f(x+w, y-1);
    glVertex2f(x+w, y);
    glVertex2f(x,   y);
    glEnd();

    // up side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_BLACK);
    glVertex2f(x,   y+h);
    glVertex2f(x+w, y+h);
    glVertex2f(x+w, y+h+1);
    glVertex2f(x,   y+h+1);
    glEnd();

    x+=1;
    y+=1;

    // left side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_WHITE);
    glVertex2f(x-1, y-1);
    glVertex2f(x,   y-1);
    glVertex2f(x,   y+h+1);
    glVertex2f(x-1, y+h+1);
    glEnd();

    // right side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_WHITE);
    glVertex2f(x+w,   y-1);
    glVertex2f(x+w+1, y-1);
    glVertex2f(x+w+1, y+h+1);
    glVertex2f(x+w,   y+h+1);
    glEnd();

    // down side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_WHITE);
    glVertex2f(x,   y-1);
    glVertex2f(x+w, y-1);
    glVertex2f(x+w, y);
    glVertex2f(x,   y);
    glEnd();

    //Graduations
    float gh_incr = gh;
    for(int i=0; i<GRADS-1; i++)
    {
        glBegin(GL_QUADS);
        glColor3f(METER_BORDER_WHITE);
        glVertex2f(x,   y+gh);
        glVertex2f(x+w, y+gh);
        glVertex2f(x+w, y+gh+1);
        glVertex2f(x,   y+gh+1);
        glEnd();
        gh+=gh_incr;
    }
    
    //Target line
    if(energy_target>0)
    {
        glBegin(GL_QUADS);
        glColor3f(METER_TARGET_RED);
        glVertex2f(x,   y+th);
        glVertex2f(x+w, y+th);
        glVertex2f(x+w, y+th+1);
        glVertex2f(x,   y+th+1);
        glEnd();
    }
    
    // up side
    glBegin(GL_QUADS);
    glColor3f(METER_BORDER_WHITE);
    glVertex2f(x,   y+h);
    glVertex2f(x+w, y+h);
    glVertex2f(x+w, y+h+1);
    glVertex2f(x,   y+h+1);
    glEnd();

    // Draw the Meter fluid
    glBegin(GL_QUADS);
    glColor4ub(METER_BOTTOM_COLOR);
    glVertex2f(x,   y);
    glVertex2f(x+w, y);

    glColor4ub(METER_TOP_COLOR);
    glVertex2f(x+w, y+state*h);
    glVertex2f(x,   y+state*h);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}   // drawEnergyMeter

//-----------------------------------------------------------------------------
void RaceGUI::drawSpeed(Kart* kart, int offset_x, int offset_y,
                        float ratio_x, float ratio_y)
{
    float minRatio = std::min(ratio_x, ratio_y);
    const int SPEEDWIDTH = 120;
    int width  = (int)(SPEEDWIDTH*minRatio);
    int height = (int)(SPEEDWIDTH*minRatio);
    offset_x += (int)((800*ratio_x-10)) - width;
    offset_y += (int)(10*ratio_y);

    glMatrixMode(GL_MODELVIEW);
    m_speed_back_icon->getState()->force();
    // If the colour isn't set, the speedometer is blended with the last
    // used colour.
    glColor4f(1,1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2i(offset_x,       offset_y);
    glTexCoord2f(1, 0); glVertex2i(offset_x+width, offset_y);
    glTexCoord2f(1, 1); glVertex2i(offset_x+width, offset_y+height);
    glTexCoord2f(0, 1); glVertex2i(offset_x,       offset_y+height);
    glEnd();

    // convention taken from btRaycastVehicle::updateVehicle
    const float speed =  kart->getSpeed();
    if(speed<=0) return;
    else
    {
        // The original speed ratio was speed/KM_PER_H/110.
        float speedRatio = speed/KILOMETERS_PER_HOUR/119;
        if(speedRatio>1) speedRatio = 1;

        m_speed_fore_icon->getState()->force();

        // There are three different polygons used, depending on 
        // the speed ratio. Consider the speed-display texture:
        //
        //   C----x----D         
        //   |         |   (the position of w,x,y vary depending on
        //   w         y    the kart's speed)
        //   |         |
        //   B----A----E
        //
        // For speed ratio <= 0.4 the triangle ABw is used, with w moving between B and C.
        // For speed ratio <= 0.8 the quad ABCx is used, with x moving between C and D.
        // For speed ratio >  0.8 the poly ABCDy is used, with y moving between D and E.

        glBegin(GL_POLYGON);
        glTexCoord2f(0.5f, 0);
        glVertex2i(offset_x+width/2, offset_y);
        glTexCoord2f(0.0f, 0);
        glVertex2i(offset_x, offset_y);

        // These values should be adjusted in case that the 
        // speed display is not linear enough. Mostly the speed values are 
        // below 0.8, it needs some zipper/nitro to get closer to 1.
        if(speedRatio <= 0.4f)
        {
            float w = speedRatio/0.4f;
            glTexCoord2f(0, w);
            glVertex2f((float)(offset_x), (float)(offset_y+w*height));
        }
        else if(speedRatio <= 0.8f)
        {
            float x = (speedRatio-0.4f)/0.4f;
            glTexCoord2f(0, 1);
            glVertex2i(offset_x, offset_y+height);
            glTexCoord2f(x, 1);
            glVertex2f((float)(offset_x+x*width), (float)(offset_y+height));
        }
        else
        {
            float y = (speedRatio-0.8f)/0.2f;
            glTexCoord2f(0, 1);
            glVertex2i(offset_x, offset_y+height);
            glTexCoord2f(1, 1);
            glVertex2i(offset_x+width, offset_y+height);
            glTexCoord2f(1, 1-y);
            glVertex2f((float)(offset_x+width), (float)(offset_y+(1-y)*height));
        }
        glEnd();
    }   // speed>0
} // drawSpeed

//-----------------------------------------------------------------------------
void RaceGUI::drawLap(const KartIconDisplayInfo* info, Kart* kart, 
                      int offset_x, int offset_y, float ratio_x, float ratio_y)
{
    // Don't display laps in follow the leader mode.
    if(!RaceManager::getWorld()->raceHasLaps()) return;
    
    const int lap = info[kart->getWorldKartId()].lap;
    float size;

    // Don't display 'lap 0/...', or do nothing if laps are disabled.
    if(lap<0) return; 

    float minRatio = std::min(ratio_x, ratio_y);
    char str[256];
    if(race_manager->getNumLocalPlayers() < 2)
    {
        offset_x += (int)(160*minRatio);
        offset_y += (int)(70*minRatio);
        size = 40*minRatio;
    }
    else
    {
        offset_x += (int)(615*minRatio);
        offset_y += (int)(750*minRatio);
        size = 37*minRatio;
    }

    if(kart->hasFinishedRace())
        return;
    else
    {
        font_race->PrintShadow(_("Lap"), size, (float)offset_x, (float)offset_y);
        offset_y -= (int)(size+2*minRatio);
        sprintf(str, "%d/%d", lap<0 ? 0 : lap+1, race_manager->getNumLaps());
        font_race->PrintShadow(str, size, (float)offset_x, (float)offset_y);
    }
} // drawLap

//-----------------------------------------------------------------------------
/** Removes messages which have been displayed long enough. This function
* must be called after drawAllMessages, otherwise messages which are only
* displayed once will not be drawn!
 **/
void RaceGUI::cleanupMessages(const float dt)
{
    AllMessageType::iterator p =m_messages.begin(); 
    while(p!=m_messages.end())
    {
        if((*p).done(dt))
        {
            p = m_messages.erase(p);
        }
        else
        {
            ++p;
        }
    }
}   // cleanupMessages

//-----------------------------------------------------------------------------
/** Displays all messages in the message queue
 **/
void RaceGUI::drawAllMessages(Kart* player_kart, int offset_x, int offset_y,
                              float ratio_x, float ratio_y)
{
    float y;
    float minRatio = std::min(ratio_x, ratio_y);
    // First line of text somewhat under the top of the screen. For now
    // start just under the timer display
    y = ((600-164)*ratio_y) + offset_y;
    // The message are displayed in reverse order, so that a multi-line
    // message (addMessage("1", ...); addMessage("2",...) is displayed
    // in the right order: "1" on top of "2"
    for(AllMessageType::const_iterator i=m_messages.begin();i!=m_messages.end(); ++i)
    {
        TimedMessage const &msg = *i;

        // Display only messages for all karts, or messages for this kart
        if(msg.m_kart && msg.m_kart!=player_kart) continue;
        GLfloat const COLORS[] = {msg.m_red/255.0f, msg.m_green/255.0f, msg.m_blue/255.0f, 255.0f};
        font_race->Print(msg.m_message.c_str(), msg.m_font_size*minRatio, 
                         (float)Font::CENTER_OF_SCREEN, y,
                         COLORS, 1.0f, 1.0f, offset_x, (int)(offset_x+(800*ratio_x)));
        // Add 20% of font size as space between the lines
        y-=(msg.m_font_size*12/10)*minRatio;
        
    }   // for i in all messages
}   // drawAllMessages

//-----------------------------------------------------------------------------
/** Adds a message to the message queue. The message is displayed for a
* certain amount of time (unless time<0, then the message is displayed
* once).
 */
void RaceGUI::addMessage(const std::string &msg, const Kart *kart, float time, 
                         int font_size, int red, int green, int blue)
{
    m_messages.push_back(TimedMessage(msg, kart, time, font_size, red, green, blue));
}   // addMessage

//-----------------------------------------------------------------------------
/** Displays the description given for the music currently being played -
* usually the title and composer.
 */
void RaceGUI::drawMusicDescription()
{
    // scaling values
    float ratio_x  = (float)(user_config->m_width/800.f);
    float ratio_y  = (float)(user_config->m_height/600.f);
    float minRatio = std::min(ratio_x, ratio_y);

    // show the music description only when music is activated in settings.
    if(user_config->doMusic())
    {
        const MusicInformation* mi=sound_manager->getCurrentMusic();
        if(!mi) return;
        float y=0;
        if(mi->getComposer()!="")
        {
            std::string s="by "+mi->getComposer();
            font_race->Print(s.c_str(), 25*minRatio, 
                            (float)Font::CENTER_OF_SCREEN, y);
            y+=20*ratio_y;
        }
        std::string s="\""+mi->getTitle()+"\"";
        font_race->Print(s.c_str(), 25*minRatio, 
                        (float)Font::CENTER_OF_SCREEN, y);
    }
    else return;

}   // drawMusicDescription

//-----------------------------------------------------------------------------
void RaceGUI::drawStatusText(const float dt)
{
    assert(RaceManager::getWorld() != NULL);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glEnable(GL_BLEND);

    glOrtho(0, user_config->m_width, 0, user_config->m_height, 0, 100);

    // scaling values for start.
    float ratio_x  = (float)(user_config->m_width/800.f);
    float ratio_y  = (float)(user_config->m_height/600.f);
    float minRatio = std::min(ratio_x, ratio_y);

    switch (RaceManager::getWorld()->getPhase())
    {
    case READY_PHASE:
        {
            GLfloat const COLORS[] = {0.9f, 0.66f, 0.62f, 1.0f};
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font_race->PrintShadow(_("Ready?"), 75*minRatio,
                                   (float)Font::CENTER_OF_SCREEN,
                                   (float)Font::CENTER_OF_SCREEN,
                                   COLORS);
        }
        break;
    case SET_PHASE:
        {
            GLfloat const COLORS[] = {0.9f, 0.9f, 0.62f, 1.0f};
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font_race->PrintShadow(_("Set?!"), 75*minRatio,
                                   (float)Font::CENTER_OF_SCREEN,
                                   (float)Font::CENTER_OF_SCREEN,
                                   COLORS);
        }
        break;
    case GO_PHASE:
        {
            GLfloat const COLORS[] = {0.39f, 0.82f, 0.39f, 1.0f};
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font_race->PrintShadow(_("Go!"), 75*minRatio, 
                                   (float)Font::CENTER_OF_SCREEN,
                                   (float)Font::CENTER_OF_SCREEN,
                                   COLORS);
        }
        break;
    default:
        break;
    }   // switch

    for(int i = 0; i<10; ++i)
    {
        if(RaceManager::getWorld()->m_debug_text[i] != "")
        {
            GLfloat const COLORS[] = {0.39f, 0.82f, 0.39f, 1.0f};
            font_race->Print(RaceManager::getWorld()->m_debug_text[i].c_str(),
                             20, 20, (float)(200 -i*20), COLORS);
        }
    }

    // The penalty message needs to be displayed for up to one second
    // after the start of the race, otherwise it disappears if 
    // "Go" is displayed and the race starts
    if(RaceManager::getWorld()->isStartPhase() || RaceManager::getWorld()->getTime()<1.0f)
    {
        for(unsigned int i=0; i<race_manager->getNumLocalPlayers(); i++)
        {
            if(RaceManager::getWorld()->getLocalPlayerKart(i)->earlyStartPenalty())
            {
                GLfloat const COLORS[] = {0.78f, 0.025f, 0.025f, 1.0f};

                font_race->PrintShadow(_("Penalty time!!"), 56*minRatio,
                                       (float)Font::CENTER_OF_SCREEN, 170*ratio_y,
                                       COLORS);
            }   // if penalty
        }  // for i < getNumPlayers
    }  // if not RACE_PHASE

    if(RaceManager::getWorld()->isRacePhase())
    {
        KartIconDisplayInfo* info = RaceManager::getWorld()->getKartsDisplayInfo(this);
        const int numPlayers = race_manager->getNumLocalPlayers();

        for(int pla = 0; pla<numPlayers; pla++)
        {
            // camera's viewport width and height.
            int viewport_width  = user_config->m_width;
            int viewport_height = user_config->m_height;

            // camera's offset x and y.
            int offset_x = 0, offset_y = 0;

            if(numPlayers == 2)
            {
                viewport_width /= 2;
                if(pla == 1) offset_x = user_config->m_width/2;
            }
            else if(numPlayers == 3)
            {
                if (pla == 0  || pla == 1)
                    offset_y = user_config->m_height/2;
                else
                {
                    // this fixes the 3rd player view.
                    viewport_width = user_config->m_width;
                }
                if (pla == 1)
                    offset_x = user_config->m_width/2;

                if (pla == 0 || pla == 1)
                    viewport_width /= 2;

                viewport_height /= 2;
            }
            else if(numPlayers == 4)
            {
                viewport_width  /= 2;
                viewport_height /= 2;
                if(pla == 0 || pla == 1)
                    offset_y = user_config->m_height/2;
                if(pla == 1 || pla == 3)
                    offset_x = user_config->m_width/2;
            }

            float split_screen_ratio_x = (float)viewport_width/800.0f;
            float split_screen_ratio_y = (float)viewport_height/600.0f;

            Kart* player_kart = RaceManager::getWorld()->getLocalPlayerKart(pla);

            if(player_kart->hasViewBlockedByPlunger())
            {
                int plunger_size = viewport_height;
                int screen_width = viewport_width;
                int plunger_x = offset_x + screen_width/2 - plunger_size/2;

                glColor4f(1,1,1,1);
                m_plunger_face->getState()->force();
                glBegin(GL_QUADS);
                glTexCoord2f(1, 0); glVertex2i(plunger_x+plunger_size, offset_y);
                glTexCoord2f(0, 0); glVertex2i(plunger_x, offset_y);
                glTexCoord2f(0, 1); glVertex2i(plunger_x, offset_y+plunger_size);
                glTexCoord2f(1, 1); glVertex2i(plunger_x+plunger_size, offset_y+plunger_size);
                glEnd();
            }
            drawPowerupIcons    (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y);
            drawEnergyMeter     (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y);
            drawSpeed           (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y);
            drawLap             (info, player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y);
            drawAllMessages     (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y);
        }   // next player

        drawTimer();

        if(RaceManager::getWorld()->getPhase() == GO_PHASE ||
           RaceManager::getWorld()->getPhase() == MUSIC_PHASE)
           drawMusicDescription();

        drawMap();

        if(user_config->m_display_fps) 
            drawFPS();
 
        drawPlayerIcons(info);

    }   // if RACE_PHASE

    glPopAttrib();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}   // drawStatusText

/* EOF */
