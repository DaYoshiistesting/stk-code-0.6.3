﻿//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) 2008      Joerg Henrichs
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


//The AI debugging works best with just 1 AI kart, so set the number of karts
//to 2 in main.cpp with quickstart and run supertuxkart with the arg -N.
#undef AI_DEBUG

#ifdef AI_DEBUG
#define SHOW_FUTURE_PATH //If defined, it will put a bunch of spheres when it
//checks for crashes with the outside of the track.
#define ERASE_PATH   //If not defined, the spheres drawn in the future path
//won't be erased the next time the function is called.
#define SHOW_NON_CRASHING_POINT //If defined, draws a green sphere where the
//n farthest non-crashing point is.
#define _WINSOCKAPI_
#include <plib/ssgAux.h>
#endif

#include "default_robot.hpp"

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <plib/sg.h>
#include "race_manager.hpp"
#include "graphics/scene.hpp"
#include "modes/linear_world.hpp"
#include "network/network_manager.hpp"
#include "robots/track_info.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

const TrackInfo *DefaultRobot::m_track_info = NULL;
int DefaultRobot::m_num_of_track_info_instances = 0;

DefaultRobot::DefaultRobot(const std::string& kart_name,
                           int position, const btTransform& init_pos, 
                           const Track *track) :
    AutoKart(kart_name, position, init_pos)
{
    if(m_num_of_track_info_instances==0)
    {
        m_track_info = new TrackInfo(track);
        m_num_of_track_info_instances++;
    }
    reset();
    m_kart_length = m_kart_properties->getKartModel()->getLength();
    m_kart_width  = m_kart_properties->getKartModel()->getWidth();
    m_track = RaceManager::getTrack();
    m_world = dynamic_cast<LinearWorld*>(RaceManager::getWorld());
    assert(m_world != NULL);
    
    switch(race_manager->getDifficulty())
    {
    case RaceManager::RD_EASY:
        m_wait_for_players   = true;
        m_max_handicap_accel = 0.9f;
        m_fallback_tactic    = FT_AVOID_TRACK_CRASH;
        m_item_tactic        = IT_TEN_SECONDS;
        m_max_start_delay    = 0.5f;
        m_min_steps          = 0;
        m_skidding_threshold = 4.0f;
        m_nitro_level        = NITRO_NONE;
        m_handle_bomb        = false;
        break;
    case RaceManager::RD_MEDIUM:
        m_wait_for_players   = true;
        m_max_handicap_accel = 0.95f;
        // FT_PARALLEL had problems on some tracks when suddenly a smaller 
        // section occurred (e.g. bridge in stone track): the AI would drive
        // over and over into the river
        m_fallback_tactic    = FT_FAREST_POINT;
        m_item_tactic        = IT_CALCULATE;
        m_max_start_delay    = 0.4f;
        m_min_steps          = 1;
        m_skidding_threshold = 2.0f;
        m_nitro_level        = NITRO_SOME;
        m_handle_bomb        = true;
        break;
    case RaceManager::RD_HARD:
        m_wait_for_players   = false;
        m_max_handicap_accel = 1.0f;
        m_fallback_tactic    = FT_FAREST_POINT;
        m_item_tactic        = IT_CALCULATE;
        m_max_start_delay    = 0.1f;
        m_min_steps          = 2;
        m_skidding_threshold = 1.3f;
        m_nitro_level        = NITRO_ALL;
        m_handle_bomb        = true;
        break;
    }
}   // DefaultRobot

//-----------------------------------------------------------------------------
/** The destructor deletes the shared TrackInfo objects if no more DefaultRobot
 *  instances are around.
 */
DefaultRobot::~DefaultRobot()
{
    m_num_of_track_info_instances--;
  //m_item_to_collect = NULL;

    if(m_num_of_track_info_instances==0)
    {
        delete m_track_info;
    }
}   // ~DefaultRobot

//-----------------------------------------------------------------------------
//TODO: if the AI is crashing constantly, make it move backwards in a straight
//line, then move forward while turning.
void DefaultRobot::update(float dt)
{
    // This is used to enable firing an item backwards.
    m_controls.m_look_back = false;
    m_controls.m_nitro     = false;
    m_track_sector         = m_world->m_kart_info[getWorldKartId()].m_track_sector;
    // The client does not do any AI computations.
    if(network_manager->getMode()==NetworkManager::NW_CLIENT) 
    {
        AutoKart::update(dt);
        return;
    }

    if(m_world->isStartPhase())
    {
        handleRaceStart();
        AutoKart::update(dt);
        return;
    }

    /*Get information that is needed by more than 1 of the handling funcs*/
    //Detect if we are going to crash with the track and/or kart
    int steps = 0;

    // This should not happen (anymore), but it keeps the game running
    // in case that m_future_sector becomes undefined.
    if(m_future_sector == Track::UNKNOWN_SECTOR)
    {
#ifdef DEBUG
        fprintf(stderr,"DefaultRobot: m_future_sector is undefined.\n");
        fprintf(stderr,"This shouldn't happen, but can be ignored.\n");
#endif
        forceRescue();
        m_future_sector = 0;
    }
    else
    {
        steps = calcSteps();
    }
    computeNearestKarts();
    checkCrashes(steps, getXYZ());
    findCurve();

    // Special behaviour if we have a bomb attach: try to hit the kart ahead 
    // of us.
    bool commands_set = false;
    if(m_handle_bomb && getAttachment()->getType()==ATTACH_BOMB && 
       m_kart_ahead)
    {
        // Use nitro if the kart is far ahead, or faster than this kart
        m_controls.m_nitro = m_distance_ahead>10.0f || 
                             m_kart_ahead->getSpeed() > getSpeed();
        // If we are close enough, try to hit this kart
        if(m_distance_ahead<=10)
        {
            Vec3 target = m_kart_ahead->getXYZ();

            // If we are faster, try to predict the point where we will hit
            // the other kart
            if(m_kart_ahead->getSpeed() < getSpeed())
            {
                float time_till_hit = m_distance_ahead
                                    / (getSpeed()-m_kart_ahead->getSpeed());
                target += m_kart_ahead->getVelocity()*time_till_hit;
            }
            float steer_angle = steerToPoint(m_kart_ahead->getXYZ().toFloat(), dt);
            setSteering(steer_angle, dt);
            commands_set = true;
        }
        handleRescue(dt);
    }
    if(!commands_set)
    {
        /*Response handling functions*/
        handleAcceleration(dt);
        handleSteering(dt);
        handleItems(dt, steps);
        handleRescue(dt);
        handleBraking();
        // If a bomb is attached, nitro might already be set.
        if(!m_controls.m_nitro)
            handleNitroAndZipper();
    }
    // If we are supposed to use nitro, but have a zipper, 
    // use the zipper instead
    if(m_controls.m_nitro && m_powerup.getType()==POWERUP_ZIPPER && 
       getSpeed()>1.0f && m_zipper_time_left<=0)
    {
        // Make sure that not all AI karts use the zipper at the same
        // time in time trial at start up, so during the first 5 seconds
        // this is done at random only.
        if(race_manager->getMinorMode()!=RaceManager::MINOR_MODE_TIME_TRIAL ||
          (m_world->getTime()<3.0f && rand()%50==1))
        {
            m_controls.m_nitro = false;
            m_controls.m_fire  = true;
        }
    }

    /*And obviously general kart stuff*/
    AutoKart::update(dt);
    m_collided = false;
}   // update

//-----------------------------------------------------------------------------
void DefaultRobot::handleBraking()
{
    // In follow the leader mode, the kart should brake if they are ahead of
    // the leader (and not the leader, i.e. don't have initial position 1)
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER &&
       getPosition() < RaceManager::getKart(0)->getPosition() &&
       getInitialPosition()>1)
    {
        m_controls.m_brake = true;
        return;
    }
        
    const float MIN_SPEED = m_track->getWidth()[m_track_sector];
    KartInfo &kart_info   = m_world->m_kart_info[getWorldKartId()];
    //We may brake if we are about to get out of the road, but only if the
    //kart is on top of the road, and if we won't slow down below a certain
    //limit.
    if(m_crashes.m_road && kart_info.m_on_road && getVelocityLC().getY() > MIN_SPEED)
    {
        float kart_ang_diff = m_track->m_angle[m_track_sector] -
                              RAD_TO_DEGREE(getHeading());
        kart_ang_diff = normalizeAngle(kart_ang_diff);
        kart_ang_diff = fabsf(kart_ang_diff);

        const float MIN_TRACK_ANGLE = 20.0f;
        const float CURVE_INSIDE_PERC = 0.25f;

        //Brake only if the road does not goes somewhat straight.
        if(m_curve_angle > MIN_TRACK_ANGLE) //Next curve is left
        {
            //Avoid braking if the kart is in the inside of the curve, but
            //if the curve angle is bigger than what the kart can steer, brake
            //even if we are in the inside, because the kart would be 'thrown'
            //out of the curve.
            if(!(m_world->getDistanceToCenterForKart(getWorldKartId()) > m_track->getWidth()[m_track_sector] *
                 -CURVE_INSIDE_PERC || m_curve_angle > RAD_TO_DEGREE(getMaxSteerAngle())))
            {
                m_controls.m_brake = false;
                return;
            }
        }
        else if(m_curve_angle < -MIN_TRACK_ANGLE) //Next curve is right
        {
            if(!(m_world->getDistanceToCenterForKart(getWorldKartId()) < m_track->getWidth()[m_track_sector] *
                 CURVE_INSIDE_PERC || m_curve_angle < -RAD_TO_DEGREE(getMaxSteerAngle())))
            {
                m_controls.m_brake = false;
                return;
            }
        }

        //Brake if the kart's speed is bigger than the speed we need
        //to go through the curve at the widest angle, or if the kart
        //is not going straight in relation to the road.
        if(getVelocityLC().getY() > m_curve_target_speed ||
           kart_ang_diff > MIN_TRACK_ANGLE)
        {
#ifdef AI_DEBUG
        std::cout << "BRAKING" << std::endl;
#endif
            m_controls.m_brake = true;
            return;
        }
    }
    m_controls.m_brake = false;
}   // handleBraking

//-----------------------------------------------------------------------------
void DefaultRobot::handleSteering(float dt)
{
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_track->m_driveline.size();
    const size_t NEXT_SECTOR = (unsigned int)m_track_sector + 2 < DRIVELINE_SIZE
                             ? m_track_sector + 2 : (m_track_sector + 2) % DRIVELINE_SIZE;
    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    if(fabsf(m_world->getDistanceToCenterForKart(getWorldKartId())) + 0.5f >
       m_track->getWidth()[m_track_sector])
    {
        steer_angle = steerToPoint(m_track->m_driveline[NEXT_SECTOR], dt);

#ifdef AI_DEBUG
        std::cout << "- Outside of road: steer to center point." <<
            std::endl;
#endif
    }
    //If we are going to crash against a kart, avoid it if it doesn't
    //drives the kart out of the road
    else if(m_crashes.m_kart != -1 && !m_crashes.m_road)
    {
        //-1 = left, 1 = right, 0 = no crash.
        if(m_start_kart_crash_direction == 1)
        {
            steer_angle = steerToAngle(NEXT_SECTOR, -M_PI*0.5f);
            m_start_kart_crash_direction = 0;
        }
        else if(m_start_kart_crash_direction == -1)
        {
            steer_angle = steerToAngle(NEXT_SECTOR, M_PI*0.5f);
            m_start_kart_crash_direction = 0;
        }
        else
        {
            if(m_world->getDistanceToCenterForKart(getWorldKartId()) >
               m_world->getDistanceToCenterForKart(m_crashes.m_kart))
            {
                steer_angle = steerToAngle(NEXT_SECTOR, -M_PI*0.5f);
                m_start_kart_crash_direction = 1;
            }
            else
            {
                steer_angle = steerToAngle(NEXT_SECTOR, M_PI*0.5f);
                m_start_kart_crash_direction = -1;
            }
        }

#ifdef AI_DEBUG
        std::cout << "- Velocity vector crashes with kart and doesn't " <<
            "crashes with road : steer 90 degrees away from kart." <<
            std::endl;
#endif

    }
    else
    {
        m_start_kart_crash_direction = 0;
        switch(m_fallback_tactic)
        {
        case FT_FAREST_POINT:
            {
                Vec3 straight_point;
                findNonCrashingPoint(straight_point);
                steer_angle = steerToPoint(straight_point, dt);
            }
            break;

        case FT_PARALLEL:
            steer_angle = steerToAngle(NEXT_SECTOR, 0.0f);
            break;

        case FT_AVOID_TRACK_CRASH:
            if(m_crashes.m_road)
            {
                steer_angle = steerToAngle(m_track_sector, 0.0f);
            }
            else steer_angle = 0.0f;

            break;
        }

#ifdef AI_DEBUG
        std::cout << "- Fallback."  << std::endl;
#endif
        // Potentially adjust the point to aim for in order to either
        // aim to collect item, or steer to avoid a bad item.
        //if(m_item_behaviour!=ITEM_COLLECT_NONE)
        //handleItemCollectionAndAvoidance(straight_point, last_node);

    }
    setSteering(steer_angle, dt);
}   // handleSteering

//-----------------------------------------------------------------------------
/*void DefaultRobot::handleItemCollectionAndAvoidance(Vec3 *straight_point, int m_sector)
{
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_track->m_driveline.size();
    const size_t NEXT_SECTOR = (unsigned int)m_track_sector + 1 < DRIVELINE_SIZE 
                             ? m_track_sector + 1 : 0;

    // Angle of line from kart to aim_point
    float kart_aim_angle = atan2(straight_point->getX()- getXYZ().getX(),
                                 straight_point->getY()- getXYZ().getY());

    if(m_item_to_collect)
    {
        if(handleSelectedItem(kart_aim_angle, straight_point, m_sector))
        {
            // Still aim at the previsouly selected item.
            *straight_point = m_item_to_collect->getXYZ();
            return;
        }
        // Otherwise remove the pre-selected item (and start
        // looking for a new item).
         m_item_to_collect = NULL;
    } // m_item_to_collect

    // Make sure we have a valid sector
    if(m_sector==Track::UNKNOWN_SECTOR)
       m_sector = NEXT_SECTOR;

    int sector = m_track_sector;
    float distance = 0;
    const Item *item_to_collect = NULL;
    const Item *item_to_avoid   = NULL;

    const float max_item_lookahead_distance = 30.f;
    while(distance < max_item_lookahead_distance)
    {
        int s_index = m_track_mesh;
        const std::vector<Item *> &items_ahead = item->getXYZ();
            ItemManager::get()->getItemsInQuads(s_index);
        for(unsigned int i=0; i<items_ahead.size(); i++)
        {
            evaluateItems(items_ahead[i],  kart_aim_angle, 
                          &item_to_avoid, &item_to_collect);
        }   // for i<items_ahead;
        distance += sgDistanceVec2
                   (m_track->m_driveline[sector], m_track->m_driveline[NEXT_SECTOR]); 

        sector = m_track->m_driveline[NEXT_SECTOR];
        // Stop when we have reached the last quad
        if(sector==m_sector) break;
    }   // while (distance < max_item_lookahead_distance)

    sgFloat Line1
    sgFloat Line2;
    Line1 = sgSetVec2(30.f, getXYZ().getX(), getXYZ().getY());
    Line2 = sgSetVec2(30.f, straight_point->getX(), straight_point->getY());
    sgVec2 *line_to_target = Line1 + Line2;


    if(item_to_collect)
    {
        sgFloat collect(item_to_collect->getXYZ().getX(),
                                item_to_collect->getXYZ().getY());
        core::vector2df cp = line_to_target.getClosestPoint(collect);
        Vec3 xyz(cp.X, item_to_collect->getXYZ().getY(), cp.Y);
}//*/
//-----------------------------------------------------------------------------
void DefaultRobot::handleItems(const float DELTA, const int STEPS)
{
    m_controls.m_fire = false;
    if(isRescue() || m_powerup.getType() == POWERUP_NOTHING) return;

    m_time_since_last_shot += DELTA;

    // Tactic 1: wait ten seconds, then use item
    // -----------------------------------------
    if(m_item_tactic==IT_TEN_SECONDS)
    {
        if(m_time_since_last_shot > 10.0f)
        {
           m_controls.m_fire = true;
           m_time_since_last_shot = 0.0f;
        }
        return;
    }

    // Tactic 2: calculate
    // -------------------
    switch(m_powerup.getType())
    {
    case POWERUP_ZIPPER:
        // Do nothing. Further up a zipper is used if nitro should be selected,
        // saving the (potential more valuable nitro) for later
        break;

    case POWERUP_BUBBLEGUM:
        // Either use the bubble gum after 10 seconds, or if the next kart 
        // behind is 'close' but not too close (too close likely means that the
        // kart is not behind but more to the side of this kart and so won't 
        // be hit by the bubble gum anyway). Should we check the speed of the
        // kart as well? I.e. only drop if the kart behind is faster? Otoh 
        // this approach helps preventing an overtaken kart to overtake us 
        // again.
        m_controls.m_fire = (m_distance_behind < 15.0f &&
                             m_distance_behind > 2.3f) || 
                             m_time_since_last_shot>10.0f;
        if(m_distance_behind < 10.0f && m_distance_behind > 2.0f)
            m_distance_behind *= 1.0f;
        break;
    // All the thrown/fired items might be improved by considering the angle
    // towards m_kart_ahead. And some of them can fire backwards, too - which
    // isn't yet supported for AI karts.
    case POWERUP_CAKE:
        {
            // Since cakes can be fired all around, just use a sane distance
            // with a bit of extra for backwards, as enemy will go towards cake
            bool fire_backwards = (m_kart_behind && m_kart_ahead &&
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance = fire_backwards ? m_distance_behind
                                            : m_distance_ahead;
            m_controls.m_fire = (fire_backwards && distance < 25.0f)     ||
                               (!fire_backwards && distance < 20.0f)     ||
                                 m_time_since_last_shot > 3.0f;
            if(m_controls.m_fire)
                m_controls.m_look_back = fire_backwards;
            break;
        }
    case POWERUP_BOWLING:
        {
            // Bowling balls slower, so only fire on closer karts - but when
            // firing backwards, the kart can be further away, since the ball
            // acts a bit like a mine (and the kart is racing towards it, too)
            bool fire_backwards = (m_kart_behind && m_kart_ahead && 
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance = fire_backwards ? m_distance_behind 
                                            : m_distance_ahead;
            m_controls.m_fire = ( fire_backwards && distance < 30.0f)    || 
                                (!fire_backwards && distance < 10.0f)    ||
                                m_time_since_last_shot > 10.0f;
            if(m_controls.m_fire)
                m_controls.m_look_back = fire_backwards;
            break;
        }
    case POWERUP_PLUNGER:
        {
            // Plungers can be fired backwards and are faster,
            // so allow more distance for shooting.
            bool fire_backwards = (m_kart_behind && m_kart_ahead && 
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance = fire_backwards ? m_distance_behind 
                                            : m_distance_ahead;
            m_controls.m_fire = distance < 30.0f                         || 
                                m_time_since_last_shot > 10.0f;
            if(m_controls.m_fire)
                m_controls.m_look_back = fire_backwards;
            break;
        }
    case POWERUP_ANVIL:
        if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            m_controls.m_fire = m_world->getTime()<1.0f && getPosition()>2;
        }
        else
        {
            m_controls.m_fire = m_time_since_last_shot > 3.0f && 
                                getPosition()>1;
        }
    default:
        m_controls.m_fire = true;
    }
    if(m_controls.m_fire)  m_time_since_last_shot = 0.0f;
    return;
}   // handleItems

//-----------------------------------------------------------------------------
/** Determines the closest karts just behind and in front of this kart. The
 *  'closeness' is for now simply based on the position, i.e. if a kart is
 *  more than one lap behind or ahead, it is not considered to be closest.
 */
void DefaultRobot::computeNearestKarts()
{
    bool need_to_check = false;
    int my_position    = getPosition();
    // See if the kart ahead has changed:
    if((m_kart_ahead && m_kart_ahead->getPosition()+1!=my_position) ||
      (!m_kart_ahead && my_position>1))
       need_to_check = true;
    // See if the kart behind has changed:
    if((m_kart_behind && m_kart_behind->getPosition()-1!=my_position) ||
      (!m_kart_behind && my_position<(int)m_world->getCurrentNumKarts()))
        need_to_check = true;
    if(!need_to_check) return;

    m_kart_behind    = m_kart_ahead      = NULL;
    m_distance_ahead = m_distance_behind = 9999999.9f;
    float my_dist = m_world->getDistanceDownTrackForKart(getWorldKartId());
    for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
    {
        Kart *k = m_world->getKart(i);
        if(k->isEliminated() || k==this) continue;
        if(k->getPosition()==my_position+1) 
        {
            m_kart_behind = k;
            m_distance_behind = my_dist - m_world->getDistanceDownTrackForKart(i);
            if(m_distance_behind<0.0f)
                m_distance_behind += m_track->getTrackLength();
        }
        else 
            if(k->getPosition()==my_position-1)
            {
                m_kart_ahead = k;
                m_distance_ahead = m_world->getDistanceDownTrackForKart(i) - my_dist;
                if(m_distance_ahead<0.0f)
                    m_distance_ahead += m_track->getTrackLength();
            }
    }   // for i<world->getNumKarts()
}   // computeNearestKarts

//-----------------------------------------------------------------------------
void DefaultRobot::handleAcceleration(const float DELTA)
{
    //Do not accelerate until we have delayed the start enough
    if(m_time_till_start > 0.0f)
    {
       m_time_till_start -= DELTA;
       m_controls.m_accel = 0.0f;
       return;
    }

    if(m_controls.m_brake == true)
    {
       m_controls.m_accel = 0.0f;
       return;
    }

    if(hasViewBlockedByPlunger())
    {
        if(!(getSpeed() > getMaxSpeedOnTerrain()/7))
            m_controls.m_accel = 0.005f;
        else 
            m_controls.m_accel = 0.0f;
        return;
    }
    
    if(m_wait_for_players)
    {
        //Find if any player is ahead of this kart
        bool player_winning = false;
        for(unsigned int i=0; i<race_manager->getNumPlayers(); ++i)
            if(m_race_position > RaceManager::getPlayerKart(i)->getPosition())
            {
                player_winning = true;
                break;
            }

        if(player_winning)
        {
            m_controls.m_accel = m_max_handicap_accel;
            return;
        }
    }
    m_controls.m_accel = 1.0f;
}   // handleAcceleration

//-----------------------------------------------------------------------------
void DefaultRobot::handleRaceStart()
{
    //FIXME: make karts able to get a penalty for accelerating too soon
    //like players, should happen to about 20% of the karts in easy,
    //5% in medium and less than 1% of the karts in hard.
    if(m_time_till_start <  0.0f)
    {
        srand((unsigned)time(0));

        //Each kart starts at a different, random time, and the time is
        //smaller depending on the difficulty.
        m_time_till_start = (float)rand() / RAND_MAX * m_max_start_delay;
    }
}   // handleRaceStart

//-----------------------------------------------------------------------------
void DefaultRobot::handleRescue(const float DELTA)
{
    // check if kart is stuck
    if(getSpeed()<2.0f && !isRescue() && !m_world->isStartPhase())
    {
        m_time_since_stuck += DELTA;
        if(m_time_since_stuck > 2.0f)
        {
            forceRescue();
            m_time_since_stuck=0.0f;
        }   // m_time_since_stuck > 2.0f
    }
    else
    {
        m_time_since_stuck = 0.0f;
    }
}   // handleRescue

//-----------------------------------------------------------------------------
/** Decides wether to use nitro or not.
 */
void DefaultRobot::handleNitroAndZipper()
{
    m_controls.m_nitro = false;
    // If we are already very fast, save nitro.
    if(getSpeed() > 0.95f*getMaxSpeedOnTerrain())
        return;
    // Don't use nitro when the AI has a plunger in the face!
    if(hasViewBlockedByPlunger()) return;
    
    // Don't use nitro when the race is finished.
    if(hasFinishedRace()) return;

    // Don't use nitro if the kart is not on ground.
    if(!isOnGround()) return;
    
    // Don't compute nitro usage if we don't have nitro or are not supposed
    // to use it, and we don't have a zipper or are not supposed to use
    // it (calculated).
    if((getEnergy()==0                       || m_nitro_level==NITRO_NONE) &&
       (m_powerup.getType()!=POWERUP_ZIPPER  || m_item_tactic==IT_TEN_SECONDS))
        return;

    // If a parachute or anvil is attached, the nitro doesn't give much
    // benefit. Better wait till later.
    const bool has_slowdown_attachment = 
                                   m_attachment.getType()==ATTACH_PARACHUTE ||
                                   m_attachment.getType()==ATTACH_ANVIL;
    if(has_slowdown_attachment) return;

    // If the kart is very slow (e.g. after rescue), use nitro
    if(getSpeed()<5)
    {
        m_controls.m_nitro = true;
        return;
    }

    // If this kart is the last kart, and we have enough 
    // (i.e. more than 2) nitro, use it.
    // -------------------------------------------------
    const unsigned int num_karts = m_world->getCurrentNumKarts();
    if(getPosition()== (int)num_karts && getEnergy()>2.0f)
    {
        m_controls.m_nitro = true;
        return;
    }

    // On the last track shortly before the finishing line, use nitro 
    // anyway. Since the kart is faster with nitro, estimate a 50% time
    // decrease (additionally some nitro will be saved when top speed
    // is reached).
    if(m_world->getLapForKart(getWorldKartId())==race_manager->getNumLaps()-1 &&
       m_nitro_level == NITRO_ALL)
    {
        float finish = m_world->getEstimatedFinishTime(getWorldKartId());
        if(1.5f*getEnergy() >= finish - m_world->getTime())
        {
            m_controls.m_nitro = true;
            return;
        }
    }

    // A kart within this distance is considered to be overtaking (or to be
    // overtaken).
    const float overtake_distance = 10.0f;

    // Try to overtake a kart that is close ahead, except 
    // when we are already much faster than that kart
    // --------------------------------------------------
    if(m_kart_ahead && m_distance_ahead < overtake_distance &&
       m_kart_ahead->getSpeed()+5.0f > getSpeed())
    {
        m_controls.m_nitro = true;
        return;
    }

    if(m_kart_behind && m_distance_behind < overtake_distance &&
       m_kart_behind->getSpeed() > getSpeed())
    {
        // Only prevent overtaking on highest level
        m_controls.m_nitro = m_nitro_level==NITRO_ALL;
        return;
    }
}   // handleNitroAndZipper

//-----------------------------------------------------------------------------
float DefaultRobot::steerToAngle(const size_t SECTOR, const float ANGLE)
{
    float angle = m_track->m_angle[SECTOR];

    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = angle - getHeading();

    if(hasViewBlockedByPlunger())
        steer_angle += ANGLE/5;
    else
        steer_angle += ANGLE;
    steer_angle = normalizeAngle(steer_angle);

    return steer_angle;
}   // steerToAngle

//-----------------------------------------------------------------------------
/** Computes the steering angle to reach a certain point. Note that the
 *  steering angle depends on the velocity of the kart (simple setting the
 *  steering angle towards the angle the point has is not correct: a slower
 *  kart will obviously turn less in one time step than a faster kart).
 *  \param point Point to steer towards.
 *  \param dt    Time step.
 */
float DefaultRobot::steerToPoint(const Vec3 point, float dt)
{
    // No sense steering if we are not driving.
    if(getSpeed()==0) return 0.0f;
    float dx        = point.getX() - getXYZ().getX();
    float dy        = point.getY() - getXYZ().getY();
    /** Angle from the kart position to the point in world coordinates. */
    float theta           = -atan2(dx, dy);

    // Angle is the point is relative to the heading - but take the current
    // angular velocity into account, too. The value is multiplied by two
    // to avoid 'oversteering' - experimentally found.
    float angle_2_point   = theta - getHeading() 
                                  - dt*m_body->getAngularVelocity().getZ();
    angle_2_point         = normalizeAngle(angle_2_point);
    if(fabsf(angle_2_point)<0.1f) return 0.0f;

    /** To understand this code, consider how a given steering angle determines
     *  the angle the kart is facing after one timestep:
     *  sin(steer_angle) = wheel_base / radius;  --> compute radius of turn
     *  circumference    = radius * 2 * M_PI;    --> circumference of turn circle
     *  The kart drives dt*V units during a timestep of size dt. So the ratio
     *  of the driven distance to the circumference is the same as the angle
     *  the whole circle, or:
     *  angle / (2*M_PI) = dt*V / circumference
     *  Reversly, if the angle to drive to is given, the circumference can be
     *  computed, and from that the turn radius, and then the steer angle.
     *  (note: the 2*M_PI can be removed from the computations)
     */
    float radius          = dt*getSpeed()/angle_2_point;
    float sin_steer_angle = m_kart_properties->getWheelBase()/radius;
#ifdef DEBUG_OUTPUT
    printf("theta %f a2p %f angularv %f radius %f ssa %f\n",
        theta, angle_2_point, m_body->getAngularVelocity().getZ(),
        radius, sin_steer_angle);
#endif
    // Add 0.1 since rouding errors will otherwise result in the kart
    // not using drifting.
    if(sin_steer_angle <= -1.0f) return -getMaxSteerAngle()*m_skidding_threshold-0.1f;
    if(sin_steer_angle >=  1.0f) return  getMaxSteerAngle()*m_skidding_threshold+0.1f;
    float steer_angle     = asin(sin_steer_angle);    
    return steer_angle;
}   // steerToPoint

//-----------------------------------------------------------------------------
void DefaultRobot::checkCrashes(const int STEPS, const Vec3& pos)
{
    // Right now there are 2 kind of 'crashes': with other karts and another
    // with the track. The sight line is used to find if the karts crash with
    // each other, but the first step is twice as big as other steps to avoid
    // having karts too close in any direction. The crash with the track can
    // tell when a kart is going to get out of the track so it steers.
    Vec3 vel_normal;

    // This is first used as a 2d-vector, but later on it is passed
    // to m_track->findRoadSector, there it is used as a 3d-vector
    // to find distance to plane, so z must be initialized to zero.
    Vec3 step_coord;
    float kart_distance;

    step_coord.setZ(0.0);

    m_crashes.clear();

    const size_t NUM_KARTS = race_manager->getNumKarts();

    // Protection against having vel_normal with nan values
    const Vec3 &VEL = getVelocity();
    vel_normal.setValue(VEL.getX(), VEL.getY(), 0.0);
    float len=vel_normal.length();
    // If the velocity is zero, no sense in checking for crashes in time
    if(len==0) return;

    // Time it takes to drive for m_kart_length units.
    float dt = m_kart_length / len; 
    vel_normal/=len;

    for(int i=1; STEPS>i; ++i)
    {
        step_coord = pos + vel_normal* m_kart_length * float(i);

        // Find if we crash with any kart, as long as we haven't found one yet.
        if(m_crashes.m_kart == -1)
        {
            for(unsigned int j=0; j<NUM_KARTS; ++j)
            {
                const Kart* kart = RaceManager::getKart(j);
                if(kart==this||kart->isEliminated()) continue;   // ignore eliminated karts
                const Kart *other_kart = RaceManager::getKart(j);
                Vec3 other_kart_xyz = other_kart->getXYZ() + other_kart->getVelocity()*(i*dt);
                kart_distance = (step_coord - other_kart_xyz).length_2d();

                if(kart_distance < m_kart_length &&
                   getVelocityLC().getY() > other_kart->getVelocityLC().getY())
                   m_crashes.m_kart = j;
            }
        }

        /*Find if we crash with the drivelines*/
        m_track->findRoadSector(step_coord, &m_sector);

#undef SHOW_FUTURE_PATH
#ifdef SHOW_FUTURE_PATH

        ssgaSphere *sphere = new ssgaSphere;

#ifdef ERASE_PATH
        static ssgaSphere *last_sphere = 0;

        if(last_sphere) scene->remove(last_sphere);

        last_sphere = sphere;
#endif

        sgVec3 center;
        center[0] = step_coord[0];
        center[1] = step_coord[1];
        center[2] = pos[2];
        sphere->setCenter(center);
        sphere->setSize(m_kart_properties->getKartModel()->getLength());
        if(m_sector == Track::UNKNOWN_SECTOR)
        {
            sgVec4 colour;
            colour[0] = colour[3] = 255;
            colour[1] = colour[2] = 0;
            sphere->setColour(colour);
        }
        else if(i==1)
        {
            sgVec4 colour;
            colour[0] = colour[1] = colour[2] = 0;
            colour[3] = 255;
            sphere->setColour(colour);
        }
        scene->add(sphere);
#endif
        m_future_location[0] = step_coord[0]; 
        m_future_location[1] = step_coord[1];

        if(m_sector == Track::UNKNOWN_SECTOR)
        {
            m_future_sector = m_track->findOutOfRoadSector(step_coord,
            Track::RS_DONT_KNOW, m_future_sector);
            m_crashes.m_road = true;
            break;
        }
        else
        {
            m_future_sector = m_sector;
        }
    }
}   // checkCrashes

//-----------------------------------------------------------------------------
/** Find the sector that at the longest distance from the kart, that can be
 *  driven to without crashing with the track, then find towards which of
 *  the two edges of the track is closest to the next curve after wards,
 *  and return the position of that edge.
 */
void DefaultRobot::findNonCrashingPoint(sgVec2 result)
{
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_track->m_driveline.size();
    
    unsigned int sector = (unsigned int)m_track_sector + 1 < DRIVELINE_SIZE 
                        ? m_track_sector + 1 : 0;
    int target_sector;

    Vec3 direction;
    Vec3 step_track_coord;
    float distance;
    int steps;

    //We exit from the function when we have found a solution
    while(1)
    {
        //target_sector is the sector at the longest distance that we can drive
        //to without crashing with the track.
        target_sector = sector + 1 < DRIVELINE_SIZE ? sector + 1 : 0;

        //direction is a vector from our kart to the sectors we are testing
        direction = m_track->m_driveline[target_sector] - getXYZ();

        float len=direction.length_2d();
        steps = int(len / m_kart_length);
        if(steps < 3) steps = 3;

        //Protection against having vel_normal with nan values
        if(len>0.0f) {
            direction*= 1.0f/len;
        }
        Vec3 step_coord;
        //Test if we crash if we drive towards the target sector
        for(int i=2; i<steps; ++i)
        {
            step_coord = getXYZ()+direction*m_kart_length * float(i);

            m_track->spatialToTrack(step_track_coord, step_coord,
                                    sector);

            distance = step_track_coord[0] > 0.0f ? step_track_coord[0]
                       : -step_track_coord[0];

            //If we are outside, the previous sector is what we are looking for
            if(distance + m_kart_width * 0.5f > m_track->getWidth()[sector])
            {
                sgCopyVec2(result, m_track->m_driveline[sector]);

#ifdef SHOW_NON_CRASHING_POINT
                ssgaSphere *sphere = new ssgaSphere;

                static ssgaSphere *last_sphere = 0;

                if(last_sphere) scene->remove(last_sphere);

                last_sphere = sphere;

                sgVec3 center;
                center[0] = result[0];
                center[1] = result[1];
                center[2] = getXYZ().getZ();
                sphere->setCenter(center);
                sphere->setSize(0.5f);

                sgVec4 colour;
                colour[1] = colour[3] = 255;
                colour[0] = colour[2] = 0;
                sphere->setColour(colour);

                scene->add(sphere);
#endif
                return;
            }
        }
        sector = target_sector;
    }
}   // findNonCrashingPoint

//-----------------------------------------------------------------------------
void DefaultRobot::reset()
{
    m_time_since_last_shot       = 0.0f;
    m_start_kart_crash_direction = 0;
    m_sector                     = Track::UNKNOWN_SECTOR;
    m_inner_curve                = 0;
    m_curve_target_speed         = getMaxSpeedOnTerrain();
    m_curve_angle                = 0.0;
    m_future_location[0]         = 0.0;
    m_future_location[1]         = 0.0;
    m_future_sector              = 0;
    m_time_till_start            = -1.0f;
    m_crash_time                 = 0.0f;
    m_collided                   = false;
    m_time_since_stuck           = 0.0f;
    m_kart_ahead                 = NULL;
    m_distance_ahead             = 0.0f;
    m_kart_behind                = NULL;
    m_distance_behind            = 0.0f;

    AutoKart::reset();
}   // reset

//-----------------------------------------------------------------------------
inline float DefaultRobot::normalizeAngle(float angle)
{
    while(angle >  2*M_PI) angle -= 2*M_PI;
    while(angle < -2*M_PI) angle += 2*M_PI;

    if(angle > M_PI) angle -= 2*M_PI;
    else if(angle < -M_PI) angle += 2*M_PI;

    return angle;
}   //normalizeAngle

//-----------------------------------------------------------------------------
/** calcSteps() divides the velocity vector by the lenght of the kart,
 *  and gets the number of steps to use for the sight line of the kart.
 *  The calling sequence guarantees that m_future_sector is not UNKNOWN.
 */
int DefaultRobot::calcSteps()
{
    int steps = int(getVelocityLC().getY() / m_kart_length);
    if(steps < m_min_steps) steps = m_min_steps;

    //Increase the steps depending on the width, if we steering hard,
    //mostly for curves.
    if(fabsf(m_controls.m_steer) > 0.95)
    {
        const int WIDTH_STEPS = 
            (int)(m_track->getWidth()[m_future_sector] 
                /(m_kart_length * 2.0));

        steps += WIDTH_STEPS;
    }

    return steps;
}   // calcSteps

//-----------------------------------------------------------------------------
/** Converts the steering angle to a lr steering in the range of -1 to 1. 
 *  If the steering angle is too great, it will also trigger skidding. This 
 *  function uses a 'time till full steer' value specifying the time it takes
 *  for the wheel to reach full left/right steering similar to player karts 
 *  when using a digital input device. This is done to remove shaking of
 *  AI karts (which happens when the kart frequently changes the direction
 *  of a turn). The parameter is defined in the kart properties.
 *  \param angle Steering angle.
 *  \param dt Time step.
 */
void DefaultRobot::setSteering(float angle, float dt)
{
    float steer_fraction = angle / getMaxSteerAngle();
    m_controls.m_drift   = fabsf(steer_fraction)>=m_skidding_threshold;
    if(hasViewBlockedByPlunger() && !RaceManager::RD_HARD) 
       m_controls.m_drift = false;
    float old_steer      = m_controls.m_steer;

    if     (steer_fraction >  1.0f) steer_fraction =  1.0f;
    else if(steer_fraction < -1.0f) steer_fraction = -1.0f;

    if(hasViewBlockedByPlunger() && !RaceManager::RD_HARD)
    {
        if     (steer_fraction >  0.5f) steer_fraction =  0.5f;
        else if(steer_fraction < -0.5f) steer_fraction = -0.5f;
    }
    
    // The AI has its own 'time full steer' value (which is the time
    float max_steer_change = dt/m_kart_properties->getTimeFullSteerAI();
    if(old_steer < steer_fraction)
    {
        m_controls.m_steer = (old_steer+max_steer_change > steer_fraction) 
                           ? steer_fraction : old_steer+max_steer_change;
    }
    else
    {
        m_controls.m_steer = (old_steer-max_steer_change < steer_fraction) 
                           ? steer_fraction : old_steer-max_steer_change;
    }
}   // setSteering

//-----------------------------------------------------------------------------
/** Finds the approximate radius of a track's curve. It needs two arguments,
 *  the number of the drivepoint that marks the beginning of the curve, and
 *  the number of the drivepoint that marks the ending of the curve.
 *
 *  Based on that you can construct any circle out of 3 points in it, we use
 *  the two arguments to use the drivelines as the first and last point; the
 *  middle sector is averaged.
 */
float DefaultRobot::getApproxRadius(const int START, const int END) const
{
    const int MIDDLE = (START + END) / 2;

    //If the START and END sectors are very close, their average will be one
    //of them, and using twice the same point just generates a huge radius
    //(too big to be of any use) but it also can generate a division by zero,
    //so here is some special handling for that case.
    if(MIDDLE == START || MIDDLE == END) return 99999.0f;

    float X1, Y1, X2, Y2, X3, Y3;

    //The next line is just to avoid compiler warnings.
    X1 = X2 = X3 = Y1 = Y2 = Y3 = 0.0f;
    if(m_inner_curve == -1)
    {
        X1 = m_track->m_left_driveline[START][0];
        Y1 = m_track->m_left_driveline[START][1];

        X2 = m_track->m_left_driveline[MIDDLE][0];
        Y2 = m_track->m_left_driveline[MIDDLE][1];

        X3 = m_track->m_left_driveline[END][0];
        Y3 = m_track->m_left_driveline[END][1];
    }
    else if(m_inner_curve == 0)
    {
        X1 = m_track->m_driveline[START][0];
        Y1 = m_track->m_driveline[START][1];

        X2 = m_track->m_driveline[MIDDLE][0];
        Y2 = m_track->m_driveline[MIDDLE][1];

        X3 = m_track->m_driveline[END][0];
        Y3 = m_track->m_driveline[END][1];
    }
    else if(m_inner_curve == 1)
    {
        X1 = m_track->m_right_driveline[START][0];
        Y1 = m_track->m_right_driveline[START][1];

        X2 = m_track->m_right_driveline[MIDDLE][0];
        Y2 = m_track->m_right_driveline[MIDDLE][1];

        X3 = m_track->m_right_driveline[END][0];
        Y3 = m_track->m_right_driveline[END][1];
    }

    const float A = X2 - X1;
    const float B = Y2 - Y1;
    const float C = X3 - X1;
    const float D = Y3 - Y1;

    const float E = A * (X1+X2) + B * (Y1+Y2);
    const float F = C * (X1+X3) + D * (Y1+Y3);

    const float G = 2.0f * (A*(Y3-Y2) - B*(X3-X2));

    const float pX = (D*E - B*F) / G;
    const float pY = (A*F - C*E) / G;

    const float radius = sqrt((X1-pX) * (X1-pX) + (Y1-pY) * (Y1-pY));

    return radius;
}   // getApproxRadius

//-----------------------------------------------------------------------------
/**FindCurve() gathers info about the closest sectors ahead: the curve
 * angle, the direction of the next turn, and the optimal speed at which the
 * curve can be travelled at it's widest angle.
 *
 * The number of sectors that form the curve is dependant on the kart's speed.
 */
void DefaultRobot::findCurve()
{
    const int DRIVELINE_SIZE = (unsigned int)m_track->m_driveline.size();
    float total_dist = 0.0f;
    int next_hint    = m_track_sector + 1;
    int i;

    for(i = m_track_sector; total_dist < getVelocityLC().getY(); i = next_hint)
    {
        next_hint = i + 1 < DRIVELINE_SIZE ? i + 1 : 0;
        total_dist += sgDistanceVec2(m_track->m_driveline[i], m_track->m_driveline[next_hint]);
    }
    m_curve_angle = normalizeAngle(m_track->m_angle[i] - m_track->m_angle[m_track_sector]);
    m_inner_curve = m_curve_angle > 0.0 ? -1 : 1;

    m_curve_target_speed = getMaxSpeedOnTerrain();
}   // findCurve
