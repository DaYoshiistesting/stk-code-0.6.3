//  $Id: history.hpp 2539 2008-12-01 22:43:50Z hikerstk $
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

#ifndef HEADER_HISTORY_HPP
#define HEADER_HISTORY_HPP

#include <vector>

#include "LinearMath/btQuaternion.h"
#include "karts/kart_control.hpp"
#include "utils/vec3.hpp"

class Kart;

class History
{
public:
    /** Determines which replay mode is selected:
     *  HISTORY_NONE: no history replay.
     *  HISTORY_POSITION: replay the positions and orientations of the karts,
     *                    but don't simulate the physics.
     *  HISTORY_PHYSICS:  Simulate the phyics based on the recorded actions.
     *  These values can be used together, e.g. HISTORY_POSITION|HISTORY_CONTROL
     */
    enum HistoryReplayMode { HISTORY_NONE     = 0,
                             HISTORY_POSITION = 1,
                             HISTORY_PHYSICS  = 2 };
private:
    // maximum number of history events to store
    HistoryReplayMode         m_replay_mode;
    int                       m_current;
    bool                      m_wrapped;
    int                       m_size;
    std::vector<float>        m_all_deltas;
    std::vector<KartControl>  m_all_controls;
    std::vector<Vec3>         m_all_xyz;
    std::vector<btQuaternion> m_all_rotations;
    void  allocateMemory(int number_of_frames);
    void  updateSaving(float dt);
    void  updateReplay(float dt);
public:
          History        ();
    void  startReplay    ();
    void  initRecording  ();
    void  update         (float dt);
    void  Save           ();
    void  Load           ();
    float getNextDelta   () const { return m_all_deltas[m_current];         }

    // ------------------------------------------------------------------------
    /** Returns if a history is replayed, i.e. the history mode is not none. */
    bool  replayHistory  () const { return m_replay_mode != HISTORY_NONE;    }
    // ------------------------------------------------------------------------
    /** Enable replaying a history, enabled from the command line. */
    void  doReplayHistory(HistoryReplayMode m) {m_replay_mode = m;           }
    // ------------------------------------------------------------------------
    /** Returns true if the physics should not be simulated in replay mode. 
     *  I.e. either no replay mode, or physics replay mode. */
    bool dontDoPhysics   () const { return m_replay_mode == HISTORY_POSITION;}
};

extern History* history;

#endif
