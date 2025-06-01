//  $Id: track.cpp 3834 2009-08-11 12:27:04Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include "track.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>
#define _WINSOCKAPI_
#include <plib/ssgAux.h>
#include <GL/glut.h>

#include "file_manager.hpp"
#include "loader.hpp"
#include "stk_config.hpp"
#include "material_manager.hpp"
#include "isect.hpp"
#include "user_config.hpp"
#include "audio/sound_manager.hpp"
#include "graphics/scene.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "karts/kart.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "modes/world.hpp"
#include "physics/moving_physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "race_manager.hpp"
#include "utils/ssg_help.hpp"
#include "utils/string_utils.hpp"

const float Track::NOHIT           = -99999.9f;
const int   Track::QUAD_TRI_NONE   = -1;
const int   Track::QUAD_TRI_FIRST  =  1;
const int   Track::QUAD_TRI_SECOND =  2;
const int   Track::UNKNOWN_SECTOR  = -1;
Track      *Track::m_track         = NULL;

//-------------------------------------------------------------------------------------------------
Track::Track(std::string filename_)
{
    m_filename         = filename_;
    m_description      = "";
    m_designer         = "";
    m_screenshot       = "";
    m_top_view         = "";
    m_version          = 0;
    m_has_final_camera = false;
    m_is_arena         = false;
    m_track            = this;
    loadTrack(m_filename);
    loadDriveline();

}   // Track

//-------------------------------------------------------------------------------------------------
Track::~Track()
{
}   // ~Track
//-------------------------------------------------------------------------------------------------
/** Removes the physical body from the world.
 *  Called at the end of a race.
 */
void Track::cleanup()
{
    ItemManager::destroy();
    delete m_non_collision_mesh;
    delete m_track_mesh;

    // remove temporary materials loaded by the material manager
    material_manager->popTempMaterial();
}   // cleanup

//-------------------------------------------------------------------------------------------------
/** Finds on which side of the line segment a given point is.
 */
inline float Track::pointSideToLine(const Vec3& L1, const Vec3& L2,
                                    const Vec3& P) const
{
    return (L2.getX()-L1.getX())*
           ( P.getY()-L1.getY())-
           (L2.getY()-L1.getY())*
           ( P.getX()-L1.getX());
}   // pointSideToLine

//-------------------------------------------------------------------------------------------------
/** pointInQuad() works by checking if the given point is 'to the right' in clock-wise direction 
 *  (which would be to look towards the inside of the quad) of each line segment that forms 
 *  the quad. If it is to the left of all the segments, then the point is inside. This idea
 *  works for convex polygons, so we have to test it for the two triangles that compose the quad,
 *  in case that the quad is concave, not for the quad itself.
 */
int Track::pointInQuad(const Vec3& A, const Vec3& B, const Vec3& C,
                       const Vec3& D, const Vec3& POINT) const
{
    if(pointSideToLine(C, A, POINT) >= 0.0)
    {
        //Test the first triangle
        if(pointSideToLine(A, B, POINT) >  0.0 &&
           pointSideToLine(B, C, POINT) >= 0.0)
            return QUAD_TRI_FIRST;
        return QUAD_TRI_NONE;
    }

    //Test the second triangle
    if(pointSideToLine(C, D, POINT) > 0.0 &&
       pointSideToLine(D, A, POINT) > 0.0)
        return QUAD_TRI_SECOND;

    return QUAD_TRI_NONE;
}   // pointInQuad

//-------------------------------------------------------------------------------------------------
/** findRoadSector returns in which sector on the road the position xyz is. If xyz is not on 
 *  top of the road, it returns UNKNOWN_SECTOR.
 *
 *  The 'sector' could be defined as the number of the closest track segment to XYZ.
 *  \param XYZ Position for which the segment should be determined.
 *  \param sector Contains the previous sector (as a shortcut, since usually
 *         the sector is the same as the last one), and on return the result
 *  \param with_tolerance If true, the drivelines with tolerance are used.
 *         This reduces the impact of driving slightly off road.
 */
void Track::findRoadSector(const Vec3& XYZ, int *sector, 
                           bool with_tolerance )const
{
    if(*sector!=UNKNOWN_SECTOR)
    {
        int next = (unsigned)(*sector) + 1 < m_left_driveline.size() ? *sector + 1 : 0;
        if(with_tolerance)
        {
            if(pointInQuad(m_dl_with_tolerance_left[*sector],
                           m_dl_with_tolerance_right[*sector],
                           m_dl_with_tolerance_right[next],
                           m_dl_with_tolerance_left[next], 
                           XYZ) != QUAD_TRI_NONE)
                // Still in the same sector, no changes
                return;
        }
        else
        {
            if(pointInQuad(m_left_driveline[*sector],
                           m_right_driveline[*sector],
                           m_right_driveline[next],   
                           m_left_driveline[next], XYZ) != QUAD_TRI_NONE)
                // Still in the same sector, no changes
                return;
        }
    }
    /* To find in which 'sector' of the track the kart is, we use a
       'point in triangle' algorithm for each triangle in the quad
       that forms each track segment.
     */
    std::vector <SegmentTriangle> possible_segment_tris;
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_left_driveline.size();
    int triangle;
    int next;

    for(size_t i = 0; i < DRIVELINE_SIZE ; ++i)
    {
        next = (unsigned int)i + 1 <  DRIVELINE_SIZE ? (int)i + 1 : 0;
        triangle = with_tolerance 
                ? pointInQuad(m_dl_with_tolerance_left[i], 
                              m_dl_with_tolerance_right[i],
                              m_dl_with_tolerance_right[next],
                              m_dl_with_tolerance_left[next], XYZ)
                : pointInQuad(m_left_driveline[i], m_right_driveline[i],
                              m_right_driveline[next], m_left_driveline[next],
                              XYZ);

        if(triangle != QUAD_TRI_NONE && ((XYZ.getZ()-m_left_driveline[i].getZ()) < 1.0f))
        {
            possible_segment_tris.push_back(SegmentTriangle((int)i, triangle));
        }
    }

    /* Since xyz can be on more than one 2D track segment, we have to
       find on top of which one of the possible track segments it is.
     */
    const int POS_SEG_SIZE = (int)possible_segment_tris.size();
    if(POS_SEG_SIZE == 0)
    {
        //xyz is not on the road
        *sector = UNKNOWN_SECTOR;
        return;
    }

    //POS_SEG_SIZE > 1
    /* To find on top of which track segment the variable xyz is,
       we get which of the possible triangles that are under xyz
       has the lower distance on the height(Y or Z) axis.
    */
    float dist;
    float near_dist = 99999;
    int nearest = QUAD_TRI_NONE;
    size_t segment;
    sgVec4 plane;
    
    for(int i = 0; i < POS_SEG_SIZE; ++i)
    {
        segment = possible_segment_tris[i].segment;
        next = segment + 1 < DRIVELINE_SIZE ? (int)segment + 1 : 0;
        
        // Note: we can make the plane with the normal driveliens
        // (not the one with tolerance), since the driveliens with
        // tolerance lie in the same plane.
        if(possible_segment_tris[i].triangle == QUAD_TRI_FIRST)
        {
            sgMakePlane(plane, m_left_driveline[segment].toFloat(),
                               m_right_driveline[segment].toFloat(), 
                               m_right_driveline[next].toFloat());
        }
        else if(possible_segment_tris[i].triangle == QUAD_TRI_SECOND)
        {
            sgMakePlane(plane, m_right_driveline[next].toFloat(),
                               m_left_driveline[next].toFloat(),
                               m_left_driveline[segment].toFloat());
        }
        
        dist = sgHeightAbovePlaneVec3(plane, XYZ.toFloat());
        
        /* sgHeightAbovePlaneVec3 gives a negative dist if the plane
           is on top, so we have to rule it out.
           
           However, for some reason there are cases where we get
           negative values for the track segment we should be on.
        */
        if(dist > -2.0 && dist < near_dist)
        {
            near_dist = dist;
            nearest = i;
        }
    }
    
    if(nearest != QUAD_TRI_NONE)
    {
        *sector=possible_segment_tris[nearest].segment;
        return;
    }
    *sector = UNKNOWN_SECTOR;
    return;                         // This only happens if the position is
                                    // under all the possible sectors
}   // findRoadSector

//-------------------------------------------------------------------------------------------------
/** findOutOfRoadSector finds the sector where XYZ is, but as it name implies, it is more accurate
 *  for the outside of the track than the inside, and for STK's needs the accuracy on top of
 *  the track is unacceptable; but if this was a 2D function, the accuracy for out of road
 *  sectors would be perfect.
 *
 *  To find the sector we look for the closest line segment from the right and left drivelines,
 *  and the number of that segment will be the sector.
 *
 *  The SIDE argument is used to speed up the function only; if we know that XYZ is on the left
 *  or right side of the track, we know that the closest driveline must be the one that matches
 *  that condition. In reality, the side used in STK is the one from the previous frame,
 *  but in order to move from one side to another a point would go through the middle, that is
 *  handled by findRoadSector() which doesn't has speed ups based on the side.
 *
 *  NOTE: This method of finding the sector outside of the road is *not* perfect: if two line
 *  segments have a similar altitude (but enough to let a kart get through) and they are very
 *  close on a 2D system, if a kart is on the air it could be closer to the top line segment even
 *  if it is supposed to be on the sector of the lower line segment. Probably the best solution
 *  would be to construct a quad that reaches until the next higher overlapping line segment,
 *  and find the closest one to XYZ.
 */
int Track::findOutOfRoadSector
(
    const Vec3& XYZ,
    const RoadSide SIDE,
    const int CURR_SECTOR
) const
{
    int sector = UNKNOWN_SECTOR;
    float dist;
    //FIXME: it can happen that dist is bigger than nearest_dist for all the
    //the points we check (currently a limit of +/- 10), and if so, the
    //function will return UNKNOWN_SECTOR, and if the AI get this, it will
    //trigger an assertion. I increased the nearest_dist default value from
    //99999 to 9999999, which is a lot more than the situation that caused
    //the discovery of this problem, but the best way to solve this, is to
    //find a better way of handling the shortcuts, and maybe a better way of
    //calculating the distance.
    float nearest_dist = 9999999;
    const int DRIVELINE_SIZE = (int)m_left_driveline.size();

    int begin_sector = 0;
    int count      = DRIVELINE_SIZE;
    if(CURR_SECTOR != UNKNOWN_SECTOR )
    {
        const int LIMIT = 10; //The limit prevents shortcuts
        if(CURR_SECTOR - LIMIT < 0)
        {
            begin_sector = DRIVELINE_SIZE - 1 + CURR_SECTOR - LIMIT;
        }
        else begin_sector = CURR_SECTOR - LIMIT;
        count = 2*LIMIT;
    }

    sgLineSegment3 line_seg;
    int next_sector;
    for(int j=0; j<count; j++)
    {
        next_sector  = begin_sector+1 == DRIVELINE_SIZE ? 0 : begin_sector+1;

        if(SIDE != RS_RIGHT)
        {
            sgCopyVec3(line_seg.a, m_left_driveline[begin_sector].toFloat());
            sgCopyVec3(line_seg.b, m_left_driveline[next_sector].toFloat());
            dist = sgDistSquaredToLineSegmentVec3(line_seg, XYZ.toFloat());
            if(dist < nearest_dist)
            {
                nearest_dist = dist;
                sector       = begin_sector;
            }
        }   // SIDE != RS_RIGHT

        if(SIDE != RS_LEFT)
        {
            sgCopyVec3(line_seg.a, m_right_driveline[begin_sector].toFloat());
            sgCopyVec3(line_seg.b, m_right_driveline[next_sector].toFloat());
            dist = sgDistSquaredToLineSegmentVec3(line_seg, XYZ.toFloat());
            if (dist < nearest_dist)
            {
                nearest_dist = dist;
                sector       = begin_sector;
            }
        }   // SIDE != RS_LEFT
        begin_sector = next_sector;
    }   // for j

    if(sector==UNKNOWN_SECTOR || sector >=DRIVELINE_SIZE)
    {
        printf("unknown sector found.\n");
    }
    return sector;
}   // findOutOfRoadSector

//-------------------------------------------------------------------------------------------------
/** spatialToTrack() takes absolute coordinates (coordinates in OpenGL space) and transforms them 
 *  into coordinates based on the track. It is for 2D coordinates, thought it can be used on 
 *  3D vectors. The y-axis of the returned vector is how much of the track the point has gone
 *  through, the x-axis is on which side of the road it is, and the z-axis contains half 
 *  the width of the track at this point. The return value is p1, i.e. the first of 
 *  the two driveline points between which the kart is currently located.
 */
int Track::spatialToTrack
(
    Vec3& dst, /* out */
    const Vec3& POS,
    const int SECTOR
) const
{
    if(SECTOR == UNKNOWN_SECTOR)
    {
        std::cerr << "WARNING: UNKNOWN_SECTOR in spatialToTrack().\n";
        return -1;
    }

    const unsigned int DRIVELINE_SIZE = (unsigned int)m_driveline.size();
    const size_t PREV = SECTOR == 0 ? DRIVELINE_SIZE - 1 : SECTOR - 1;
    const size_t NEXT = (size_t)SECTOR+1 >= DRIVELINE_SIZE ? 0 : SECTOR + 1;

    const float DIST_PREV = (m_driveline[PREV]-POS).length2_2d();
    const float DIST_NEXT = (m_driveline[NEXT]-POS).length2_2d();

    size_t p1, p2;
    if(DIST_NEXT < DIST_PREV)
    {
        p1 = SECTOR; p2 = NEXT;
    }
    else
    {
        p1 = PREV; p2 = SECTOR;
    }

    sgVec3 line_eqn;
    sgVec2 tmp;

    sgMake2DLine(line_eqn, m_driveline[p1].toFloat(), m_driveline[p2].toFloat());

    dst.setX(sgDistToLineVec2(line_eqn, POS.toFloat()));

    sgAddScaledVec2(tmp, POS.toFloat(), line_eqn, -dst.getX());

    float dist_from_driveline_p1 = sgDistanceVec2(tmp, m_driveline[p1].toFloat());
    dst.setY(dist_from_driveline_p1 + m_distance_from_start[p1]);
    // Set z-axis to half the width (linear interpolation between the
    // width at p1 and p2) - m_path_width is actually already half the width
    // of the track. This is used to determine if a kart is too far
    // away from the road and is therefore considered taking a shortcut.

    float fraction = dist_from_driveline_p1
                   / (m_distance_from_start[p2]-m_distance_from_start[p1]);
    dst.setZ(m_path_width[p1]*(1-fraction)+fraction*m_path_width[p2]);

    return (int)p1;
}   // spatialToTrack

//-------------------------------------------------------------------------------------------------
const Vec3& Track::trackToSpatial(const int SECTOR) const
{
    return m_driveline[SECTOR];
}   // trackToSpatial

//-------------------------------------------------------------------------------------------------
/** Returns the start coordinates for a kart on a given position pos
 *  (with pos ranging from 0 to kart_num-1).
 */
btTransform Track::getStartTransform(unsigned int pos) const
{
    Vec3 orig;
    float angle;
    if(isArena())
    {
        assert(pos<m_start_positions.size());
        orig.setX(m_start_positions[pos][0]);
        orig.setY(m_start_positions[pos][1]);
        orig.setZ(m_start_positions[pos][2]);
        angle = 0;
    }
    else
    {    
        // sometimes the first kart would be too close
        // to the first driveline point and not to the last one -->
        // This kart would not get any lap counting done in the first
        // lap! Therefore an offset is substracted from its Y location,
        // and this offset is calculated based on the drivelines
        float offset = 1.5f;
        if(m_left_driveline[0].getY() > 0 || m_right_driveline[0].getY() > 0)
            offset += std::max(m_left_driveline[0].getY(), m_left_driveline[0].getY());
        
        // FIXME: Wrong calculation of start positions: uses the center point of the model
        // instead of the center point between right and left drivelines (i.e, in The Island
        // track, if you delete the start settings, karts will spawn under the track, because
        // the center of the model is not on the start of the track but on the left). Every kart
        // is located in a sector (which is a fragment of the track, i.e, a quad in STK 0.7)
        // which means that, for the new calculation, we will need to use karts positions,
        // sectors, and rotation of the point we found for the kart in that sector. 
        orig.setX(pos<m_start_x.size() ? m_start_x[pos] : ((pos%2==0)?1.5f:-1.5f));
        orig.setY(pos<m_start_y.size() ? m_start_y[pos] : -1.5f*pos-offset       );
        orig.setZ(pos<m_start_z.size() ? m_start_z[pos] :  1.0f                  );
    }
    btTransform start;
    start.setOrigin(orig);
    start.setRotation(btQuaternion(btVector3(0,0,1), 
                                   pos<m_start_heading.size() 
                                   ? DEGREE_TO_RAD(m_start_heading[pos]) 
                                   : 0.0f));
    return start;
}   // getStartTransform

//-------------------------------------------------------------------------------------------------
/** Determines if a kart moving from sector OLDSEC to sector NEWSEC
 *  would be taking a shortcut, i.e. if the distance is larger
 *  than a certain delta
 */
bool Track::isShortcut(const int OLDSEC, const int NEWSEC) const
{
    // If the kart was off the road, don't do any shortcuts
    if(OLDSEC==UNKNOWN_SECTOR || NEWSEC==UNKNOWN_SECTOR) return false;
    int next_sector = OLDSEC==(int)m_driveline.size()-1 ? 0 : OLDSEC+1;
    if(next_sector==NEWSEC) 
        return false;

    int distance_sectors = (int)(m_distance_from_start[std::max(NEWSEC, OLDSEC)] -
                                 m_distance_from_start[std::min(NEWSEC, OLDSEC)]  );
    
    // Handle 'warp around'
    const int track_length = (int)m_distance_from_start[m_driveline.size()-1];
    if(distance_sectors < 0) distance_sectors += track_length;
    //else if(distance_sectors > track_length*3.0f/4.0f) distance_sectors -= track_length;
    
    if(std::max(NEWSEC, OLDSEC) > (int)RaceManager::getTrack()->m_distance_from_start.size()-6 &&
       std::min(NEWSEC, OLDSEC) < 6) distance_sectors -= track_length; // crossed start line
    
    return (distance_sectors > stk_config->m_shortcut_length);
}   // isShortcut

//-------------------------------------------------------------------------------------------------
void Track::addDebugToScene(int type) const
{
    if(type&1)
    {
        ssgaSphere *sphere;
        sgVec3 center;
        sgVec4 colour;
        for(unsigned int i = 0; i < m_driveline.size(); ++i)
        {
            sphere = new ssgaSphere;
            sgCopyVec3(center, m_driveline[i].toFloat());
            sphere->setCenter(center);
            sphere->setSize(getWidth()[i] / 4.0f);
        
            if(i == 0)
            {
                colour[0] = colour[2] = colour[3] = 255;
                colour[1] = 0;
            }
            else
            {
                colour[0] = colour[1] = colour[3] = 255;
                colour[2] = 0;
            }
            sphere->setColour(colour);
            scene->add(sphere);
        }   // for i
    }  /// type ==1
    // 2: drivelines, 4: driveline with tolerance
    if(type&6)
    {
        ssgVertexArray* v_array = new ssgVertexArray();
        ssgColourArray* c_array = new ssgColourArray();
        const std::vector<Vec3> &left  = type&2 ? m_left_driveline 
                                                : m_dl_with_tolerance_left;
        const std::vector<Vec3> &right = type&2 ? m_right_driveline 
                                               : m_dl_with_tolerance_right;
        for(unsigned int i = 0; i < m_driveline.size(); i++)
        {
            int ip1 = i==m_driveline.size()-1 ? 0 : i+1;
            // The segment display must be slightly higher than the
            // track, otherwise it's not clearly visible.
            sgVec3 v;
            sgCopyVec3(v,left [i  ].toFloat()); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,right[i  ].toFloat()); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,right[ip1].toFloat()); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,left [ip1].toFloat()); v[2]+=0.1f; v_array->add(v);
            sgVec4 vc;
            vc[0] = i%2==0 ? 1.0f : 0.0f;
            vc[1] = 1.0f-v[0];
            vc[2] = 0.0f;
            vc[3] = 0.1f;
            c_array->add(vc);c_array->add(vc);c_array->add(vc);c_array->add(vc);
        }   // for i
        // if GL_QUAD_STRIP is used, the colours are smoothed, so the changes
        // from one segment to the next are not visible.
        ssgVtxTable* l = new ssgVtxTable(GL_QUADS, v_array,
                                         (ssgNormalArray*)NULL,
                                         (ssgTexCoordArray*)NULL,
                                         c_array);
        scene->add(l);
    }
}   // addDebugToScene

//-------------------------------------------------------------------------------------------------
/** This draws the MiniMap. Before, the MiniMap used to be drawn with two functions: drawScaled2D 
 *  and draw2DView. drawScaled2D used temporary variables while the other one used
 *  pre-computed variables m_scale_x and m_scale_y, previously used by glVtx.
 *  drawScaled2D was called from gui/TrackSel, draw2Dview from RaceGUI.
 *  Now, the MiniMap is entirely drawn from RaceGUI, including karts points.
 */
void Track::draw2DMiniMap(float x, float y, float w, float h, float sx, float sy) const
{
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_driveline.size();

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1, 1, 1, 0.4f);

    // This only draw the white space of the map (which 0.7 does).
    glBegin(GL_QUAD_STRIP);
     for (size_t i=0; i<DRIVELINE_SIZE; ++i)
    {
        glVertex2f(x + (m_left_driveline[i].getX() - m_driveline_min.getX()) * sx,
                   y + (m_left_driveline[i].getY() - m_driveline_min.getY()) * sy);

        glVertex2f(x + (m_right_driveline[i].getX() - m_driveline_min.getX()) * sx,
                   y + (m_right_driveline[i].getY() - m_driveline_min.getY()) * sy);
    }
    glVertex2f(x + (m_left_driveline[0].getX() - m_driveline_min.getX()) * sx,
               y + (m_left_driveline[0].getY() - m_driveline_min.getY()) * sy);

    glVertex2f(x + (m_right_driveline[0].getX() - m_driveline_min.getX()) * sx,
               y + (m_right_driveline[0].getY() - m_driveline_min.getY()) * sy);

    glDisable(GL_DEPTH_TEST);
    glEnd();
    glPopAttrib();
}   //drawMiniMap

//-------------------------------------------------------------------------------------------------
void Track::loadTrack(std::string filename_)
{
    m_filename      = filename_;

    m_ident = StringUtils::basename(StringUtils::without_extension(m_filename));
    std::string path = StringUtils::without_extension(m_filename);

    // Default values
    m_use_fog = false;
    sgSetVec4 (m_fog_color, 0.3f, 0.7f, 0.9f, 1.0f);
    m_fog_density   = 1.0f/100.0f;
    m_fog_start     = 0.0f;
    m_fog_end       = 1000.0f;
    m_gravity       = 9.80665f;

    sgSetVec3(m_sun_position, 0.4f, 0.4f, 0.4f      );
    sgSetVec4(m_sky_color,    0.3f, 0.7f, 0.9f, 1.0f);
    sgSetVec4(m_ambient_col,  0.5f, 0.5f, 0.5f, 1.0f);
    sgSetVec4(m_specular_col, 1.0f, 1.0f, 1.0f, 1.0f);
    sgSetVec4(m_diffuse_col,  1.0f, 1.0f, 1.0f, 1.0f);

    lisp::Parser parser;
    const lisp::Lisp* const ROOT = parser.parse(m_filename);

    const lisp::Lisp* const LISP = ROOT->getLisp("tuxkart-track");
    if(!LISP)
    {
        delete ROOT;
        std::ostringstream msg;
        msg <<"Couldn't load map '"<<m_filename<<"': no tuxkart-track node found.";
        throw std::runtime_error(msg.str());
    }

    LISP->get      ("name",                  m_name);
    LISP->get      ("description",           m_description);
    LISP->get      ("designer",              m_designer);
    LISP->get      ("version",               m_version);
    std::vector<std::string> filenames;
    LISP->getVector("music",                 filenames);
    getMusicInformation(filenames, m_music);
    LISP->get      ("screenshot",            m_screenshot);
    LISP->get      ("topview",               m_top_view);
    LISP->get      ("sky-color",             m_sky_color);
    LISP->getVector("start-x",               m_start_x);
    LISP->getVector("start-y",               m_start_y);
    LISP->getVector("start-z",               m_start_z);
    LISP->getVector("start-heading",         m_start_heading);
    LISP->get      ("use-fog",               m_use_fog);
    LISP->get      ("fog-color",             m_fog_color);
    LISP->get      ("fog-density",           m_fog_density);
    LISP->get      ("fog-start",             m_fog_start);
    LISP->get      ("fog-end",               m_fog_end);
    LISP->get      ("sun-position",          m_sun_position);
    LISP->get      ("sun-ambient",           m_ambient_col);
    LISP->get      ("sun-specular",          m_specular_col);
    LISP->get      ("sun-diffuse",           m_diffuse_col);
    LISP->get      ("gravity",               m_gravity);
    LISP->get      ("arena",                 m_is_arena);
    LISP->getVector("groups",                m_groups);
    if(m_groups.size()==0)
        m_groups.push_back("Standard");
    // if both camera position and rotation are defined,
    // set the flag that the track has final camera position
    m_has_final_camera  = LISP->get("camera-final-position", m_camera_final_pos);
    m_has_final_camera &= LISP->get("camera-final-hpr",      m_camera_final_hpr);
    m_camera_final_hpr.degreeToRad();

    // Set the correct paths
    m_screenshot = file_manager->getTrackFile(m_screenshot, getIdent());
    m_top_view   = file_manager->getTrackFile(m_top_view,   getIdent());
    
    delete ROOT;
}   // loadTrack

//-------------------------------------------------------------------------------------------------
void Track::getMusicInformation(std::vector<std::string>& filenames, 
                                std::vector<MusicInformation*>& music)
{
    for(int i=0; i<(int)filenames.size(); i++)
    {
        std::string full_path = file_manager->getTrackFile(filenames[i], getIdent());
        MusicInformation* mi;
        try
        {
            mi = sound_manager->getMusicInformation(full_path);
        }
        catch(std::runtime_error)
        {
            mi = sound_manager->getMusicInformation(file_manager->getMusicFile(filenames[i]));
        }
        if(!mi)
        {
            fprintf(stderr, "Music information file '%s' not found - ignored.\n",
                    filenames[i].c_str());
            continue;
        }
        m_music.push_back(mi);
    }   // for i in filenames
    
    if (m_music.empty())
    {
        m_music.push_back(stk_config->m_default_music);
        fprintf(stderr, "Music information for track '%s' not found, default music will play.\n",
        m_ident.c_str());
    }

}   // getMusicInformation

//-------------------------------------------------------------------------------------------------
void Track::startMusic() const 
{
    // In case that the music wasn't found (a warning was already printed)
    if(m_music.size()>0)
        sound_manager->startMusic(m_music[rand()% m_music.size()]);
}   // startMusic

//-------------------------------------------------------------------------------------------------
void Track::loadDriveline()
{
    // FIXME: Make the game able to read multiple drivelines
    // to have multiple paths, still with multiple drivelines 
    // files, to have backwards compatibility.
    readDrivelineFromFile(m_left_driveline, ".drvl");
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_left_driveline.size();
    m_right_driveline.reserve(DRIVELINE_SIZE);
    readDrivelineFromFile(m_right_driveline, ".drvr");

    if(m_right_driveline.size() != m_left_driveline.size())
        std::cout << "Error: driveline's sizes do not match, right " <<
        "driveline is " << m_right_driveline.size() << " vertex long " <<
        "and the left driveline is " << m_left_driveline.size()
        << " vertex long. The targeted track is " << m_name << " ." << std::endl;

    m_dl_with_tolerance_left.reserve(DRIVELINE_SIZE);
    m_dl_with_tolerance_right.reserve(DRIVELINE_SIZE);
    m_driveline.reserve(DRIVELINE_SIZE);
    m_path_width.reserve(DRIVELINE_SIZE);
    m_angle.reserve(DRIVELINE_SIZE);
    for(unsigned int i=0; i<DRIVELINE_SIZE; ++i)
    {
        Vec3 center_point = (m_left_driveline[i]+m_right_driveline[i])*0.5;
        m_driveline.push_back(center_point);

        float width = (m_right_driveline[i] - center_point).length();
        m_path_width.push_back(width);

        // Compute the drivelines with tolerance
        Vec3 diff = (m_left_driveline[i] - m_right_driveline[i]) 
                  *  stk_config->m_offroad_tolerance;
        m_dl_with_tolerance_left.push_back(m_left_driveline[i]+diff);
        m_dl_with_tolerance_right.push_back(m_right_driveline[i]-diff);
    }

    for(unsigned int i=0; i<DRIVELINE_SIZE; ++i)
    {
        unsigned int next = i+1 >= DRIVELINE_SIZE ? 0 : i+1;
        float dx = m_driveline[next].getX() - m_driveline[i].getX();
        float dy = m_driveline[next].getY() - m_driveline[i].getY();

        float theta = -atan2(dx, dy);
        m_angle.push_back(theta);
    }

    m_driveline_min = Vec3( SG_MAX/2.0f);
    m_driveline_max = Vec3(-SG_MAX/2.0f);

    m_distance_from_start.reserve(DRIVELINE_SIZE);
    float d = 0.0f;
    for(size_t i=0; i<DRIVELINE_SIZE; ++i)
    {
        //Both drivelines must be checked to get the true size of
        //the drivelines, and using the center driveline is not
        //good enough.
        m_driveline_min.min(m_right_driveline[i]);
        m_driveline_min.min(m_left_driveline [i]);
        m_driveline_max.max(m_right_driveline[i]);
        m_driveline_max.max(m_left_driveline [i]);

        m_distance_from_start.push_back(d);  // dfs[i] is not valid in windows here!
        d += (m_driveline[i]-m_driveline[i==DRIVELINE_SIZE-1 ? 0 : i+1]).length();
    }
    m_total_distance = d;

}   // loadDriveline

//-------------------------------------------------------------------------------------------------
void Track::readDrivelineFromFile(std::vector<Vec3>& line, const std::string& file_ext)
{
    std::string path = file_manager->getTrackFile(m_ident+file_ext);
    FILE *fd = fopen(path.c_str(), "r");

    if(fd == NULL)
    {
        std::ostringstream msg;
        msg<<"Can't open '"<<path<<"' for reading.\n";
        throw std::runtime_error(msg.str());
    }

    int prev_sector = UNKNOWN_SECTOR;
    SGfloat prev_distance = 1.51f;
    while(!feof(fd))
    {
        char s[1024];

        if (fgets(s, 1023, fd) == NULL)
            break ;

        if (*s == '#' || *s < ' ')
            continue;

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        if(sscanf(s, "%f,%f,%f", &x, &y, &z) != 3)
        {
            std::ostringstream msg;
            msg<<"Syntax error in '"<<path<<"'\n";
            throw std::runtime_error(msg.str());
        }
        Vec3 point(x,y,z);
        if(prev_sector != UNKNOWN_SECTOR) 
            prev_distance = (point-line[prev_sector]).length2_2d();

        //1.5f was choosen because it's more or less the length of the tuxkart
        if(prev_distance < 0.0000001)
        {
            fprintf(stderr, "File %s point %d is duplicated!.\n",
                    path.c_str(), prev_sector+1);
        }
#if 0
        else if(prev_distance < 1.5f)
        {
            fprintf(stderr,"File %s point %d is too close(<1.5) to previous point.\n",
                    path.c_str(), prev_sector + 1);
        }
        if(prev_distance > 15.0f)
        {
            fprintf(stderr,"In file %s point %d is too far(>15.0) from next point at %d.\n",
                    path, prev_sector, prev_distance);
        }
#endif
        line.push_back(point);
        ++prev_sector;
        prev_distance -= 1.5f;
    }
    fclose(fd);
}   // readDrivelineFromFile

//-------------------------------------------------------------------------------------------------
/** Convert the ssg track tree into its physics equivalents.
 */
void Track::createPhysicsModel()
{
    if(!m_model) return;

    m_track_mesh         = new TriangleMesh();
    m_non_collision_mesh = new TriangleMesh();
    
    // Collect all triangles in the track_mesh
    sgMat4 mat;
    sgMakeIdentMat4(mat);
    convertTrackToBullet(m_model, mat);
    m_track_mesh->createBody();
    m_non_collision_mesh->createBody(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    
}   // createPhysicsModel

//-------------------------------------------------------------------------------------------------
/** Convert the ssg track tree into its physics equivalents.
 */
void Track::convertTrackToBullet(ssgEntity *track, sgMat4 m)
{
    if(!track) return;
    MovingPhysics *mp = dynamic_cast<MovingPhysics*>(track);
    if(mp)
    {
        // If the track contains objects of type MovingPhysics,
        // these objects will be real rigid bodies and are already
        // part of the world. So these objects mustn't be converted
        // to triangle meshes.
    }
    else if(track->isAKindOf(ssgTypeLeaf()))
    {
        ssgLeaf  *leaf     = (ssgLeaf*)(track);
        Material *material = material_manager->getMaterial(leaf);
        // Don't convert triangles with material that's ignored (e.g. fuzzy_sand)
        if(!material || material->isIgnore()) return;

        for(int i=0; i<leaf->getNumTriangles(); i++) 
        {
            short v1,v2,v3;
            sgVec3 vv1, vv2, vv3;
            
            leaf->getTriangle(i, &v1, &v2, &v3);
            sgXformPnt3(vv1, leaf->getVertex(v1), m);
            sgXformPnt3(vv2, leaf->getVertex(v2), m);
            sgXformPnt3(vv3, leaf->getVertex(v3), m);
            btVector3 vb1(vv1[0],vv1[1],vv1[2]);
            btVector3 vb2(vv2[0],vv2[1],vv2[2]);
            btVector3 vb3(vv3[0],vv3[1],vv3[2]);
            if(material->isZipper()) 
            {
                m_non_collision_mesh->addTriangle(vb1, vb2, vb3, material);
            }
            else
            {
                m_track_mesh->addTriangle(vb1, vb2, vb3, material);
            }
        }
        
    }   // if(track isAKindOf leaf)
    else if(track->isAKindOf(ssgTypeTransform()))
    {
        ssgBaseTransform *t = (ssgBaseTransform*)(track);
        sgMat4 tmpT, tmpM;
        t->getTransform(tmpT);
        sgCopyMat4(tmpM, m);
        sgPreMultMat4(tmpM,tmpT);
        for(ssgEntity *e = t->getKid(0); e!=NULL; e=t->getNextKid())
        {
            convertTrackToBullet(e, tmpM);
        }   // for i
    }
    else if(track->isAKindOf(ssgTypeBranch())) 
    {
        ssgBranch *b = (ssgBranch*)track;
        for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid()) 
        {
            convertTrackToBullet(e, m);
        }   // for i<getNumKids
    }
    else
    {
        assert(!"Unknown ssg type in convertTrackToBullet");
    }
}   // convertTrackToBullet

//-------------------------------------------------------------------------------------------------
void Track::loadTrackModel()
{
    // Add the track directory to the texture search path
    file_manager->pushTextureSearchPath(file_manager->getTrackFile("",getIdent()));
    file_manager->pushModelSearchPath  (file_manager->getTrackFile("",getIdent()));
    // First read the temporary materials.dat file if it exists
    try
    {
        std::string materials_file = file_manager->getTrackFile("materials.dat",getIdent());
        material_manager->pushTempMaterial(materials_file);
    }
    catch(std::exception& e)
    {
        // no temporary materials.dat file, ignore
        (void)e;
    }
    std::string path = file_manager->getTrackFile(getIdent()+".loc");

    FILE *fd = fopen(path.c_str(), "r");
    if(fd == NULL)
    {
        std::ostringstream msg;
        msg<<"Can't open track location file '"<<path<<"'.";
        throw std::runtime_error(msg.str());
    }

    // Start building the scene graph
    m_model = new ssgBranch;
    scene->add(m_model);

    char s[1024];

    while (fgets(s, 1023, fd) != NULL)
    {
        if (*s == '#' || *s < ' ')
            continue ;

        int need_hat = false;
        int fit_skin = false;
        char fname[1024];
        sgCoord loc;
        sgZeroVec3(loc.xyz);
        sgZeroVec3(loc.hpr);
        char htype = '\0' ;

        // The old names are only here for backwards compatibility. Don't use 'herring' names in any new track.
        if(sscanf(s, "%cHERRING,%f,%f,%f", &htype,
                      &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 4)
        {
            Item::ItemType type=Item::ITEM_BANANA;
            if(htype=='Y' || htype=='y') {type = Item::ITEM_BIG_NITRO   ;}
            if(htype=='G' || htype=='g') {type = Item::ITEM_BANANA      ;}
            if(htype=='R' || htype=='r') {type = Item::ITEM_BONUS_BOX   ;}
            if(htype=='S' || htype=='s') {type = Item::ITEM_SMALL_NITRO ;}
            itemCommand(&loc.xyz, type, false);
        }
        else if(sscanf(s, "%cHERRING,%f,%f", &htype,
                           &(loc.xyz[0]), &(loc.xyz[1])) == 3)
        {
            Item::ItemType type=Item::ITEM_BANANA;
            if(htype=='Y' || htype=='y') {type = Item::ITEM_BIG_NITRO   ;}
            if(htype=='G' || htype=='g') {type = Item::ITEM_BANANA      ;}
            if(htype=='R' || htype=='r') {type = Item::ITEM_BONUS_BOX   ;}
            if(htype=='S' || htype=='s') {type = Item::ITEM_SMALL_NITRO ;}
            itemCommand (&loc.xyz, type, true);
        }
        else if(sscanf(s, "COIN,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 3)
        {
            itemCommand(&loc.xyz, Item::ITEM_SMALL_NITRO, false);
        }
        else if(sscanf(s, "COIN,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1])) == 2)
        {
            itemCommand(&loc.xyz, Item::ITEM_SMALL_NITRO, true);
        }
        else if(sscanf(s, "GOLD,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 3)
        {
            itemCommand(&loc.xyz, Item::ITEM_BIG_NITRO, false);
        }
        else if(sscanf(s, "GOLD,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1])) == 2)
        {
            itemCommand(&loc.xyz, Item::ITEM_BIG_NITRO, true);
        }

        // Here are listed the names you should use now.
        else if(sscanf(s, "BBOX,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 3)
        {
            itemCommand(&loc.xyz, Item::ITEM_BONUS_BOX, false);
        }
        else if(sscanf(s, "BBOX,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1])) == 2)
        {
            itemCommand(&loc.xyz, Item::ITEM_BONUS_BOX, true);
        }
        else if(sscanf(s, "BANA,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 3)
        {
            itemCommand(&loc.xyz, Item::ITEM_BANANA, false);
        }
        else if(sscanf(s, "BANA,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1])) == 2)
        {
            itemCommand(&loc.xyz, Item::ITEM_BANANA, true);
        }
        else if(sscanf(s, "SMALLTANK,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 3)
        {
            itemCommand(&loc.xyz, Item::ITEM_SMALL_NITRO, false);
        }
        else if(sscanf(s, "SMALLTANK,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1])) == 2)
        {
            itemCommand(&loc.xyz, Item::ITEM_SMALL_NITRO, true);
        }
        else if(sscanf(s, "BIGTANK,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 3)
        {
            itemCommand(&loc.xyz, Item::ITEM_BIG_NITRO, false);
        }
        else if(sscanf(s, "BIGTANK,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1])) == 2)
        {
            itemCommand(&loc.xyz, Item::ITEM_BIG_NITRO, true);
        }

        // Arena start positions in .loc file.
        else if(sscanf(s, "START,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2])) == 3)
        {
            m_start_positions.push_back(Vec3(loc.xyz[0], loc.xyz[1], loc.xyz[2]));
        }
        else if(s[0] == '\"')
        {
            if(sscanf(s, "\"%[^\"]\",%f,%f,%f,%f,%f,%f",
                          fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
                          &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2]) ) == 7)
            {
                /* All 6 DOF specified */
                need_hat = false;
            }
            else if(sscanf(s, "\"%[^\"]\",%f,%f,{},%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2])) == 6)
            {
                /* All 6 DOF specified - but need height */
                need_hat = true;
            }
            else if(sscanf(s, "\"%[^\"]\",%f,%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
                               &(loc.hpr[0])) == 5)
            {
                /* No Roll/Pitch specified - assumed zero */
                need_hat = false;
            }
            else if(sscanf(s, "\"%[^\"]\",%f,%f,{},%f,{},{}",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0])) == 4)
            {
                /* All 6 DOF specified - but need height, roll, pitch */
                need_hat = true;
                fit_skin = true;
            }
            else if(sscanf(s, "\"%[^\"]\",%f,%f,{},%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0])) == 4)
            {
                /* No Roll/Pitch specified - but need height */
                need_hat = true;
            }
            else if(sscanf(s, "\"%[^\"]\",%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.xyz[2])) == 4)
            {
                /* No Heading/Roll/Pitch specified - but need height */
                need_hat = false;
            }
            else if(sscanf(s, "\"%[^\"]\",%f,%f,{}",
                               fname, &(loc.xyz[0]), &(loc.xyz[1])) == 3)
            {
                /* No Roll/Pitch specified - but need height */
                need_hat = true;
            }
            else if(sscanf(s, "\"%[^\"]\",%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1])) == 3)
            {
                /* No Z/Heading/Roll/Pitch specified */
                need_hat = false;
            }
            else if(sscanf(s, "\"%[^\"]\"", fname) == 1)
            {
                /* Nothing specified */
                need_hat = false;
            }
            else
            {
                fclose(fd);
                std::ostringstream msg;
                msg<< "Syntax error in '"<<path<<"': "<<s;
                throw std::runtime_error(msg.str());
            }

            if(need_hat)
            {
                sgVec3 nrm;
                loc.xyz[2] = 1000.0f;
                loc.xyz[2] = getHeightAndNormal(m_model, loc.xyz, nrm);

                if(fit_skin)
                {
                    float sy = sin(-loc.hpr[0] * SG_DEGREES_TO_RADIANS);
                    float cy = cos(-loc.hpr[0] * SG_DEGREES_TO_RADIANS);

                    loc.hpr[2] =  SG_RADIANS_TO_DEGREES * atan2(nrm[0]*cy -
                                  nrm[1]*sy, nrm[2]);
                    loc.hpr[1] = -SG_RADIANS_TO_DEGREES * atan2(nrm[1]*cy +
                                  nrm[0]*sy, nrm[2]);
                }
            }   // if need_hat

            ssgEntity *obj = loader->load(file_manager->getModelFile(fname),
                                          CB_TRACK, /*optimise*/ true,
                                          /*is_full_path*/ true);
            if(!obj)
            {
                fclose(fd);
                std::ostringstream msg;
                msg<<"Can't open track model '"<<fname<<"'.";
                file_manager->popTextureSearchPath();
                file_manager->popModelSearchPath  ();
                throw std::runtime_error(msg.str());
            }
            SSGHelp::createDisplayLists(obj);
            ssgRangeSelector *lod   = new ssgRangeSelector;
            ssgTransform     *trans = new ssgTransform(&loc);

            float r[2] = {-10.0f, 7000.0f};

            lod->addKid(obj);
            trans->addKid(lod);
            m_model->addKid(trans);
            lod->setRanges(r, 2);
            if(user_config->m_track_debug)
                addDebugToScene(user_config->m_track_debug);
        }
        else
        {
            fprintf(stderr, "Warning: Syntax error in '%s': %s",
                     path.c_str(), s);
        }
    }   // while fgets

    fclose(fd) ;
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath();

    SSGHelp::MinMax(m_model, &m_aabb_min, &m_aabb_max);
    RaceManager::getWorld()->getPhysics()->init(m_aabb_min, m_aabb_max);
    createPhysicsModel();
}   // loadTrack

//-------------------------------------------------------------------------------------------------
void Track::itemCommand(sgVec3 *xyz, int type, int bNeedHeight )
{

    // if only 2d coordinates are given, let the item fall from very high
    if(bNeedHeight) (*xyz)[2] = 1000000.0f;

    // Even if 3d data are given, make sure that the item is on the ground
    (*xyz)[2] = getHeight(m_model, *xyz) + 0.06f;

    // Some modes (e.g. time trial) don't have any bonus boxes
    if(type==Item::ITEM_BONUS_BOX && !RaceManager::getWorld()->enableBonusBoxes()) 
        return;
    Vec3 loc((*xyz));

    // Don't tilt the items, since otherwise the rotation will look odd,
    // i.e. the items will not rotate around the normal, but 'wobble'
    // around.
    Vec3 normal(0.0f, 0.0f, 0.0f);
    ItemManager::get()->newItem((Item::ItemType)type, loc, normal);
}   // itemCommand

//-------------------------------------------------------------------------------------------------
void Track::getTerrainInfo(const Vec3 &pos, float *hot, Vec3 *normal, 
                           const Material **material) const
{
    btVector3 to_pos(pos);
    to_pos.setZ(-100000.f);

    class MaterialCollision : public btCollisionWorld::ClosestRayResultCallback
    {
    public:
        const Material* m_material;
        MaterialCollision(btVector3 p1, btVector3 p2) : 
            btCollisionWorld::ClosestRayResultCallback(p1,p2) {m_material=NULL;}
        virtual btScalar AddSingleResult(btCollisionWorld::LocalRayResult& rayResult,
                                         bool normalInWorldSpace) {
             if(rayResult.m_localShapeInfo && rayResult.m_localShapeInfo->m_shapePart>=0 )
             {
                 m_material = ((TriangleMesh*)rayResult.m_collisionObject->getUserPointer())
                              ->getMaterial(rayResult.m_localShapeInfo->m_triangleIndex);
             }
             else
             {
                 // This can happen if the raycast hits a kart. This should 
                 // actually be impossible (since the kart is removed from
                 // the collision group), but now and again the karts don't
                 // have the broadphase handle set (kart::update() for 
                 // details), and it might still happen. So in this case
                 // just ignore this callback and don't add it.
                 return 1.0f;
             }
             return btCollisionWorld::ClosestRayResultCallback::AddSingleResult(rayResult, 
                                                                                normalInWorldSpace);
        }   // AddSingleResult
    };   // myCollision
    MaterialCollision rayCallback(pos, to_pos);
    RaceManager::getWorld()->getPhysics()->getPhysicsWorld()->rayTest(pos, to_pos, rayCallback);

    if(!rayCallback.HasHit()) 
    {
        *hot      = NOHIT;
        *material = NULL;
        return;
    }

    *hot      = rayCallback.m_hitPointWorld.getZ();
    *normal   = rayCallback.m_hitNormalWorld;
    *material = rayCallback.m_material;
    // Note: material might be NULL. This happens if the ray cast does not
    // hit the track, but another rigid body (kart, moving_physics) - e.g.
    // assume two karts falling down, one over the other. Bullet does not
    // have any triangle/material information in this case!
}   // getTerrainInfo
