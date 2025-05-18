//  $Id: stk_config.hpp 2944 2009-01-16 11:13:55Z hikerstk $
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

#ifndef HEADER_STK_CONFIG_HPP
#define HEADER_STK_CONFIG_HPP

#include "karts/kart_properties.hpp"

class Lisp;
class MusicInformation;

/** Global STK configuration information. Parameters here can be tuned without
 *  recompilation, but the user shouldn't actually modify them. It also
 *  includes the list of default kart physics parameters which are used for
 *  each kart (but which can be overwritten for each kart, too).
 */
class STKConfig 
{
protected:
    KartProperties  m_kart_properties; /**< Default kart properties. */
public:
    static float UNDEFINED;
    float m_anvil_weight;            /**<Additional kart weight if anvil is 
                                      *  attached.                           */
    float m_anvil_speed_factor;      /**<Speed decrease when attached first. */
    float m_parachute_friction;      /**<Increased parachute air friction.   */
    float m_parachute_done_fraction; /**<Fraction of speed when lost will 
                                      *  detach parachute.                   */
    float m_parachute_time;          /**<Time a parachute is active.         */
    float m_parachute_time_other;    /**<Time a parachute attached to other 
                                      *  karts is active.                    */
    float m_bomb_time;               /**<Time before a bomb explodes.        */
    float m_bomb_time_increase;      /**<Time added to bomb timer when it's 
                                      *  passed on.                          */
    float m_anvil_time;              /**<Time an anvil is active.            */
    float m_zipper_time;             /**<Duration a zipper is active.        */
    float m_zipper_force;            /**<Additional force added to the 
                                      *  acceleration.                       */
    float m_zipper_speed_gain;       /**<Initial one time speed gain.        */
    float m_shortcut_length;         /**<Skipping more than this distance
                                      *  in segments triggers a shortcut.    */
    float m_offroad_tolerance;       /**<Road width is extended by that 
                                      *  fraction to make shortcut detection
                                      *  more forgiving.                     */
    float m_final_camera_time;       /**<Time for the movement of the final
                                      *  camera.                             */
	int   m_bubble_gum_counter;      /**<How many times a bubble gum must be 
                                      *   eaten before it disappear.         */
    float m_explosion_impulse;       /**<Impulse affecting each non-hit kart.*/
    float m_explosion_impulse_obj;   /**<Impulse of explosion on moving 
                                      *  objects, e.g. road cones, ...       */
    float m_delay_finish_time;       /**<Delay after a race finished before
                                      *  the results are displayed.          */
    float m_music_credit_time;       /**<Time the music credits are
                                      *  displayed.                          */
    int   m_max_karts;               /**<Maximum number of karts.            */
    int   m_grid_order;              /**<Whether grand prix grid is in point
                                      *  or reverse point order.             */
    int   m_max_history;             /**<Maximum number of frames to save in
                                      *  a history files.                    */
    int   m_max_skidmarks;           /**<Maximum number of skid marks/kart.  */
    float m_skid_fadeout_time;       /**<Time till skidmarks fade away.      */ 
    float m_slowdown_factor;         /**<Used in terrain specific slowdown.  */
    float m_near_ground;             /**<Determines when a kart is not near 
                                      *  ground anymore and the upright
                                      *  constraint is disabled to allow for
                                      *  more violent explosions.            */
    int   m_min_kart_version,        /**<The minimum and maximum .kart file  */
          m_max_kart_version;        /** version supported by this binary.   */
    int   m_min_track_version,       /**<The minimum and maximum .track file */
          m_max_track_version;       /** version supported by this binary.   */
    bool  m_enable_networking;       /**<Enable or disable networking.       */

    std::vector<float> 
          m_leader_intervals;        /**<Interval in follow the leader till 
                                      *  last kart is reomved.               */
    std::vector<int>
          m_scores;                  /**<Scores depending on position.       */

    MusicInformation 
         *m_title_music;             /**<Filename of the title music to play.*/
    MusicInformation 
         *m_default_music;           /**<Filename of the default track music 
                                      *  to play.                            */
    std::vector<std::string>
          m_mainmenu_background;     /**<Picture used as menu background.    */
    std::vector<std::string>
          m_menu_background;         /**<Picture used as background for 
                                      *  other menus.                        */
    
    /** Empty constructor. The actual work is done in load. */
         STKConfig() {};
    void init_defaults    ();
    void getAllData       (const lisp::Lisp* lisp);
    void load             (const std::string &filename);
    /** Returns the default kart properties for each kart. */
    const KartProperties &
         getDefaultKartProperties() const {return m_kart_properties; }
    const std::string &getMainMenuPicture(int n);
    const std::string &getBackgroundPicture(int n);
}
;   // STKConfig

extern STKConfig* stk_config;
#endif
