//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
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

#ifndef HEADER_DEFAULT_H
#define HEADER_DEFAULT_H

#include "karts/auto_kart.hpp"

class Track;
class LinearWorld;
class TrackInfo;

class DefaultRobot : public AutoKart
{
private:
    enum FallbackTactic
    {
        FT_AVOID_TRACK_CRASH, //Only steer to avoid getting out of the road,
                              //otherwise, don't steer at all
        FT_PARALLEL,    //Stay parallel to the road
        FT_FAREST_POINT //Drive towards the farest non-crashing point that
                        //the kart can drive to in a straight line without
                        //crashing with the track.
    };

    /** How the AI uses nitro. */
    enum {NITRO_NONE, NITRO_SOME, NITRO_ALL} m_nitro_level;
    enum ItemTactic
    {
        IT_TEN_SECONDS, //Fire after 10 seconds have passed, since the item
                        //was grabbed.
        IT_CALCULATE //Aim carefully, check for enough space for boosters,
                     //and that other conditions are meet before firing.
    };

    class CrashTypes
    {
        public:

        bool m_road; //true if we are going to 'crash' with the bounds of the road
        int m_kart; //-1 if no crash, pos numbers are the kart it crashes with
        CrashTypes() : m_road(false), m_kart(-1) {};
        void clear() {m_road = false; m_kart = -1;}
    } m_crashes;

    /*Difficulty handling variables*/
    float m_max_start_delay; //Delay before accelerating at the start of each
                             //race
    int m_min_steps; //Minimum number of steps to check. If 0, the AI doesn't
                     //even has check around the kart, if 1, it checks around
                     //the kart always, and more than that will check the
                     //remaining number of steps in front of the kart, always
    bool  m_wait_for_players; //If true, the acceleration is decreased when
                              //the AI is in a better position than all the
                              //human players.
    float m_max_handicap_accel; //The allowed maximum speed, in percentage,
                                //from 0.0 to 1.0. Used only when
                                //m_wait_for_players == true.
    FallbackTactic m_fallback_tactic; //General steering procedure. Used
                                      //mostly on straight lines and on curves
                                      //that re too small to need special
                                      //handling.
    
    ItemTactic m_item_tactic; //How are items going to be used?

    /** True if the kart should try to pass on a bomb to another kart. */

    bool m_handle_bomb;
    /*General purpose variables*/
    //The crash percentage is how much of the time the AI has been crashing,
    //if the AI has been crashing for some time, use the rescue.
    float m_crash_time;
    int   m_collided;           // true if the kart collided with the track

    /** Pointer to the closest kart ahead of this kart. NULL if this
     *  kart is first. */
    Kart *m_kart_ahead;
    /** Distance to the kart ahead. */
    float m_distance_ahead;

    /** Pointer to the closest kart behind this kart. NULL if this kart
     *  is last. */
    Kart *m_kart_behind;
    /** Distance to the kard behind. */
    float m_distance_behind;

    /** Time an item has been collected and not used. */
    float m_time_since_last_shot;
    int   m_future_sector;
    sgVec2 m_future_location;

    float m_time_till_start; //Used to simulate a delay at the start of the
                             //race, since human players don't accelerate
                             //at the same time and rarely repeat the a
                             //previous timing.

    int m_inner_curve;//-1 left, 1 = right, 0 = center
    float m_curve_target_speed;
    float m_curve_angle;

    /** Keep a pointer to the track to reduce calls */
    Track       *m_track;

    /** Keep a pointer to world. */
    LinearWorld *m_world;
    /** Cache kart_info.m_track_sector. */
    int   m_track_sector;

    
    float m_time_since_stuck;

    int m_start_kart_crash_direction; //-1 = left, 1 = right, 0 = no crash.

    /** Length of the kart, storing it here saves many function calls. */
    float m_kart_length;

    /** Cache width of kart. */
    float m_kart_width;
    /** All AIs share the track info object, so that its information needs 
     *  only to be computed once. */
    static const TrackInfo *m_track_info;
    /** This counts how many AIs have a pointer to the TrackInfo object. If
     *  this number reaches zero, the shared TrackInfo object is 
     *  deallocated. */
    static int m_num_of_track_info_instances;

    /** The minimum steering angle at which the AI adds skidding. Lower values
     *  tend to improve the line the AI is driving. This is used to adjust for
     *  different AI levels.
     */
    float m_skidding_threshold;

    int   m_sector;

    /*Functions called directly from update(). They all represent an action
     *that can be done, and end up setting their respective m_controls
     *variable, except handle_race_start() that isn't associated with any
     *specific action (more like, associated with inaction).
     */
    void  handleRaceStart();
    void  handleAcceleration(const float DELTA);
    void  handleSteering(float dt);
    void  handleItems(const float DELTA, const int STEPS);
    void  handleRescue(const float DELTA);
    void  handleBraking();
    void  handleNitroAndZipper();
    void  computeNearestKarts();
    void  handleItemCollectionAndAvoidance(Vec3 *straight_point,
                                           int m_sector);
    bool  handleSelectedItem(float kart_aim_angle,
                             Vec3 *straight_point, int m_sector);
    void  evaluateItems(const Item *item, float kart_aim_angle, 
                        const Item **item_to_avoid, 
                        const Item **item_to_collect);

    /* Lower level functions not called directly from update() */
    float steerToAngle(const size_t SECTOR, const float ANGLE);
    float steerToPoint(const Vec3 point, float dt);

    void  checkCrashes(const int STEPS, const Vec3& pos);
    void  findNonCrashingPoint(sgVec2 result);

    float normalizeAngle(float angle);
    int   calcSteps();
    void  setSteering(float angle, float dt);
    float getApproxRadius(const int START, const int END) const;
    void  findCurve();

public:
                 DefaultRobot(const std::string& kart_name, int position,
                              const btTransform& init_pos, const Track *track);
                ~DefaultRobot();
    void         update      (float delta) ;
    void         reset       ();
    virtual void crashed     (Kart *k) {if(k) m_collided = true;};
};

#endif

/* EOF */
