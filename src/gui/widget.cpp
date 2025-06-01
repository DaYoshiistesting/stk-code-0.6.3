//  $Id: widget_set.cpp 1094 2007-05-21 06:49:06Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  This code originally from Neverball copyright (C) 2003 Robert Kooima
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

#include "widget.hpp"

#include <cmath>
#include <iostream>

//FIXME: this should be removed when the scrolling is cleaned
#include "user_config.hpp"

#include "material_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"

const int Widget::MAX_SCROLL = 1000000;

const float Widget::MAX_TEXT_SCALE = 1.2f;
const float Widget::MIN_TEXT_SCALE = 1.0f;

const GLfloat WGT_WHITE  [4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat WGT_GRAY   [4] = { 0.5f, 0.5f, 0.5f, 1.0f };
const GLfloat WGT_BLACK  [4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat WGT_YELLOW [4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const GLfloat WGT_RED    [4] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat WGT_GREEN  [4] = { 0.0f, 1.0f, 0.0f, 1.0f };
const GLfloat WGT_BLUE   [4] = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat WGT_TRANS_WHITE  [4] = { 1.0f, 1.0f, 1.0f, 0.5f };
const GLfloat WGT_TRANS_GRAY   [4] = { 0.5f, 0.5f, 0.5f, 0.5f };
const GLfloat WGT_TRANS_BLACK  [4] = { 0.0f, 0.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_YELLOW [4] = { 1.0f, 1.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_RED    [4] = { 1.0f, 0.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_GREEN  [4] = { 0.0f, 1.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_BLUE   [4] = { 0.0f, 0.0f, 1.0f, 0.5f };

//FIXME: I should change 'LIGHT' for 'LIT'.
const GLfloat WGT_LIGHT_GRAY   [4] = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat WGT_LIGHT_BLACK  [4] = {0.5f, 0.5f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_YELLOW [4] = {1.0f, 1.0f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_RED    [4] = {1.0f, 0.5f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_GREEN  [4] = {0.5f, 1.0f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_BLUE   [4] = {0.5f, 0.5f, 1.0f, 1.0f};
const GLfloat WGT_LIGHT_TRANS_GRAY   [4] = {1.0f, 1.0f, 1.0f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_BLACK  [4] = {0.5f, 0.5f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_YELLOW [4] = {1.0f, 1.0f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_RED    [4] = {1.0f, 0.5f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_GREEN  [4] = {0.5f, 1.0f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_BLUE   [4] = {0.5f, 0.5f, 1.0f, 0.8f};

const GLfloat WGT_TRANSPARENT [4] = {1.0f, 1.0f, 1.0f, 0.0f};

Widget::Widget
(
    const int X_,
    const int Y_,
    const int WIDTH_,
    const int HEIGHT_
) :
//Switch features are not set here to sane defaults because the WidgetManager
//handles that.
    m_x(X_), m_y(Y_),
    m_width(WIDTH_), m_height(HEIGHT_),
    m_fixed_position(false),
    m_rect_list(0),
    m_round_corners(WGT_AREA_ALL),
    m_border_list(0),
    m_scroll_pos_x(0), m_scroll_pos_y(0),
    m_text_scale(1.0f)
{
}

//-----------------------------------------------------------------------------
Widget::~Widget()
{
    if(glIsList(m_rect_list))
    {
        glDeleteLists(m_rect_list, 1);
    }

    if(glIsList(m_border_list))
    {
        glDeleteLists(m_border_list, 1);
    }

}

//-----------------------------------------------------------------------------
void Widget::setPosition(WidgetDirection horizontal, float percentage_horizontal, 
                         const Widget *w_hori,
                         WidgetDirection vertical,   float percentage_vertical,
                         const Widget *w_verti)
{
    m_fixed_position    = true;
    m_horizontal        = horizontal;
    m_percentage_x      = percentage_horizontal;
    m_widget_horizontal = w_hori;
    // If the direction is left/right of a widget, but that widget is not defined
    // use left/right (of screen). This simplifies programming, since the e.g. 
    // left-most widget will be positioned relative to the side of the screen
    if(!w_hori)
    {
        if(m_horizontal==WGT_DIR_LEFT_WIDGET ) m_horizontal = WGT_DIR_FROM_RIGHT;
        if(m_horizontal==WGT_DIR_RIGHT_WIDGET) m_horizontal = WGT_DIR_FROM_LEFT;
    }
    m_vertical          = vertical;
    m_percentage_y      = percentage_vertical;
    m_widget_vertical   = w_verti;
    if(!w_verti)
    {
        if(m_vertical==WGT_DIR_ABOVE_WIDGET) m_vertical = WGT_DIR_FROM_BOTTOM;
        if(m_vertical==WGT_DIR_UNDER_WIDGET) m_vertical = WGT_DIR_FROM_TOP;
    }

}   // setPosition
// ----------------------------------------------------------------------------
void Widget::layout()
{
    if(!hasFixedPosition())
    {
        std::cerr << "Warning: layout called for widget without fixed position.\n";
        return;
    }
    if( !createRect() ) return;

    switch(m_horizontal)
    {
    case WGT_DIR_FROM_LEFT: 
        m_x = (int)(user_config->m_width*m_percentage_x); break;
    case WGT_DIR_FROM_RIGHT:
        m_x = (int)(user_config->m_width*(1-m_percentage_x)-m_width); break;
    case WGT_DIR_CENTER:
        m_x = (int)((user_config->m_width-m_width)*0.5f); break;
    case WGT_DIR_LEFT_WIDGET:
        m_x = m_widget_horizontal->m_x - m_width; break;
    case WGT_DIR_RIGHT_WIDGET:
        m_x = m_widget_horizontal->m_x+m_widget_horizontal->m_width
            + (int)(user_config->m_width*m_percentage_x); break;
    default:
        break;
    }   // switch

    switch(m_vertical)
    {
    case WGT_DIR_FROM_TOP: 
        m_y = (int)(user_config->m_height*(1-m_percentage_y)-m_height); break;
    case WGT_DIR_FROM_BOTTOM:
        m_y = (int)(user_config->m_height*m_percentage_y); break;
    case WGT_DIR_CENTER:
        m_y = (int)((user_config->m_height-m_height)*0.5f); break;
    case WGT_DIR_ABOVE_WIDGET:
        m_y = m_widget_vertical->m_y + m_widget_vertical->m_height; break;
    case WGT_DIR_UNDER_WIDGET:
        m_y = m_widget_vertical->m_y-m_height-(int)(m_percentage_y*user_config->m_height); break;
    default:
        break;
    }   // switch

}   // layout

//-----------------------------------------------------------------------------
void Widget::update(const float DELTA)
{
    updateVariables(DELTA);
    draw();
}

//-----------------------------------------------------------------------------
void Widget::resizeToText()
{
    if( !m_text.empty() )
    {
        float left, right, bottom, top;
        m_font->getBBoxMultiLine(m_text, (float)m_text_size, false, &left, &right, &bottom, &top);

        const int TEXT_WIDTH = (int)(right - left);
        const int TEXT_HEIGHT = (int)(top - bottom);

        if( TEXT_WIDTH > m_width ) m_width = TEXT_WIDTH;
        if( TEXT_HEIGHT > m_height ) m_height = TEXT_HEIGHT;
    }
}

//-----------------------------------------------------------------------------
/* Please note that this function only lightens 'non-light' colors */
void Widget::lightenColor()
{
    if(m_rect_color == WGT_GRAY)
    {
        m_rect_color = WGT_LIGHT_GRAY;
    }
    if(m_rect_color == WGT_BLACK)
    {
        m_rect_color = WGT_LIGHT_BLACK;
    }
    else if (m_rect_color == WGT_YELLOW)
    {
        m_rect_color = WGT_LIGHT_YELLOW;
    }
    else if (m_rect_color == WGT_RED)
    {
        m_rect_color = WGT_LIGHT_RED;
    }
    else if (m_rect_color == WGT_GREEN)
    {
        m_rect_color = WGT_LIGHT_GREEN;
    }
    else if (m_rect_color == WGT_BLUE)
    {
        m_rect_color = WGT_LIGHT_BLUE;
    }
    else if (m_rect_color == WGT_TRANS_GRAY)
    {
        m_rect_color = WGT_LIGHT_TRANS_GRAY;
    }
    else if (m_rect_color == WGT_TRANS_BLACK)
    {
        m_rect_color = WGT_LIGHT_TRANS_BLACK;
    }
    else if (m_rect_color == WGT_TRANS_YELLOW)
    {
        m_rect_color = WGT_LIGHT_TRANS_YELLOW;
    }
    else if (m_rect_color == WGT_TRANS_RED)
    {
        m_rect_color = WGT_LIGHT_TRANS_RED;
    }
    else if (m_rect_color == WGT_TRANS_GREEN)
    {
        m_rect_color = WGT_LIGHT_TRANS_GREEN;
    }
    else if (m_rect_color == WGT_TRANS_BLUE)
    {
        m_rect_color = WGT_LIGHT_TRANS_BLUE;
    }
}

//-----------------------------------------------------------------------------
/* Please note that this function only darkens 'light' colors. */
void Widget::darkenColor()
{
    if(m_rect_color == WGT_LIGHT_GRAY)
    {
        m_rect_color = WGT_GRAY;
    }
    if(m_rect_color == WGT_LIGHT_BLACK)
    {
        m_rect_color = WGT_BLACK;
    }
    else if (m_rect_color == WGT_LIGHT_YELLOW)
    {
        m_rect_color = WGT_YELLOW;
    }
    else if (m_rect_color == WGT_LIGHT_RED)
    {
        m_rect_color = WGT_RED;
    }
    else if (m_rect_color == WGT_LIGHT_GREEN)
    {
        m_rect_color = WGT_GREEN;
    }
    else if (m_rect_color == WGT_LIGHT_BLUE)
    {
        m_rect_color = WGT_BLUE;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_GRAY)
    {
        m_rect_color = WGT_TRANS_GRAY;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_BLACK)
    {
        m_rect_color = WGT_TRANS_BLACK;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_YELLOW)
    {
        m_rect_color = WGT_TRANS_YELLOW;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_RED)
    {
        m_rect_color = WGT_TRANS_RED;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_GREEN)
    {
        m_rect_color = WGT_TRANS_GREEN;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_BLUE)
    {
        m_rect_color = WGT_TRANS_BLUE;
    }
}

//-----------------------------------------------------------------------------
void Widget::setFont(const WidgetFont FONT)
{
    switch(FONT)
    {
        case WGT_FONT_GUI:
            m_font = font_gui;
            break;

        case WGT_FONT_RACE:
            m_font = font_race;
            break;
    };

    //TODO: the curr_widget_font variable exists only for a bug around; after
    //some restructuration, it should be fine to remove this.
    m_curr_widget_font = FONT;
}

//-----------------------------------------------------------------------------
void Widget::setTexture(const std::string& FILENAME, bool is_full_path)
{
    Material *m = material_manager->getMaterial( FILENAME, is_full_path );
    m_texture = m->getState()->getTextureHandle();
}

/** Initialize a display list containing a rectangle that can have rounded
 *  corners, with texture coordinates to properly apply a texture
 *  map to the rectangle as though the corners were not rounded . Returns
 *  false if the call to glGenLists failed, otherwise it returns true.
 */
bool Widget::createRect()
{

    //TODO: show warning if text > rect
    if(m_radius > m_width * 0.5)
    {
        std::cerr << "Warning: widget's radius > half width.\n";
    }
    if(m_radius > m_height * 0.5)
    {
        std::cerr << "Warning: widget's radius > half height.\n";
    }
    if(m_radius < 1)
    {
        std::cerr << "Warning: widget's radius < 1, setting to 1.\n";
        m_radius = 1;
    }

    if(m_width == 0)
    {
        std::cerr << "Warning: creating widget rect with width 0, " <<
            "setting to 1.\n";
        m_width = 1;
    }
    if(m_height == 0)
    {
        std::cerr << "Warning: creating widget rect with height 0, " <<
            "setting to 1.\n";
        m_height = 1;
    }

    if(!glIsList(m_rect_list))
    {
        m_rect_list = glGenLists(1);
        if(m_rect_list == 0)
        {
            std::cerr << "Error: could not create a widget's rect list.\n";
            return false;
        }
    }

    //Calculate the number of quads each side should have. The algorithm
    //isn't based just on logic, since it went through visual testing to give
    //the perception of roundness.
    const int MIN_QUADS = 2;
    const int NUM_QUADS = MIN_QUADS + m_radius;

    int i;

    const int SMALLER_SIDE_LENGTH = m_height < m_width ? m_height / 2 : m_width / 2;
    const float BORDER_LENGTH = SMALLER_SIDE_LENGTH * m_border_percentage;

    typedef std::vector<float> float3;
    std::vector<float3> inner_vertex;
    std::vector<float3> outer_vertex;

    //NUM_QUADS + 1, because we have to add the union between the sides, and
    //multiplied by 2, because there are two sides
    inner_vertex.resize((NUM_QUADS + 1) * 2);
    outer_vertex.resize((NUM_QUADS + 1) * 2);

    const float HALF_WIDTH = m_width * 0.5f;
    const float HALF_HEIGHT = m_height * 0.5f;

    glNewList(m_rect_list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            //These are used to center the widget; without centering, the
            //widget's 0,0 coordinates are at the lower left corner.
            float angle;
            float circle_x, circle_y;

            //Draw the left side of the inside
            for (i = 0; i <= NUM_QUADS; ++i)
            {
                //To find the position in the X and Y axis of each point of
                //the quads, we use the property of the unit circle (a circle
                //with radius = 1) that at any given angle, cos(angle) is the
                //position of the unit circle at that angle in the X axis,
                //and that sin(angle) is the position of the unit circle at
                //that angle in the Y axis. Then the values from cos(angle)
                //and sin(angle) are multiplied by the radius.
                //
                //First we find the angle: since 2 * pi is the number of
                //radians in an entire circle, 0.5 * pi is a quarter of the
                //circle, which is a corner of the rounded rectangle. Based
                //on that, we just split the radians in a corner in NUM_QUADS
                //+ 1 parts, and use the angles at those parts to find the
                //X and Y position of the points.
                angle = 0.5f * M_PI * (float)i / (float)NUM_QUADS;
                circle_x = m_radius * cos(angle);
                circle_y = m_radius * sin(angle);

                //After we generate the positions in circle for the angles,
                //we have to position each rounded corner properly depending
                //on the position of the rectangle and the radius. The y
                //position for the circle is dependant on rect; if a corner
                //wasn't given, then the y position is computed as if it was
                //for a rectangle without rounder corners.
                //
                //The value in the position 0 of these vectors is X, the
                //second the Y for the top part of the widget and the third
                //the Y for the lower part of the widget.
                inner_vertex[i].resize(3);
                outer_vertex[i].resize(3);
                outer_vertex[i][0] = m_radius - circle_x;
                inner_vertex[i][0] = outer_vertex[i][0] + BORDER_LENGTH;

                if( m_round_corners & WGT_AREA_NW )
                {
                    outer_vertex[i][1] = m_height + circle_y - m_radius;
                    inner_vertex[i][1] = outer_vertex[i][1] - BORDER_LENGTH;
                }
                else
                {
                    outer_vertex[i][1] =(float) m_height;
                    inner_vertex[i][1] = outer_vertex[i][1] - BORDER_LENGTH;
                }

                if( m_round_corners & WGT_AREA_SW )
                {
                    outer_vertex[i][2] = m_radius - circle_y;
                    inner_vertex[i][2] = outer_vertex[i][2] + BORDER_LENGTH;
                }
                else
                {
                    outer_vertex[i][2] = 0;
                    inner_vertex[i][2] = outer_vertex[i][2] + BORDER_LENGTH;
                }

                glTexCoord2f(inner_vertex[i][0] / (float)m_width, inner_vertex[i][1] / (float)m_height);
                glVertex2f(inner_vertex[i][0] - HALF_WIDTH, inner_vertex[i][1] - HALF_HEIGHT);

                glTexCoord2f(inner_vertex[i][0] / (float)m_width, inner_vertex[i][2] / (float)m_height);
                glVertex2f(inner_vertex[i][0] - HALF_WIDTH, inner_vertex[i][2] - HALF_HEIGHT);
            }

            //Draw the right side of a rectangle
            for (i = NUM_QUADS; i < NUM_QUADS * 2 + 1; ++i)
            {
                angle = 0.5f * M_PI * (float) (i - NUM_QUADS) / (float) NUM_QUADS;

                //By inverting the use of sin and cos we get corners that are
                //drawn from left to right instead of right to left
                circle_x = m_radius * sin(angle);
                circle_y = m_radius * cos(angle);

                inner_vertex[i+1].resize(3);
                outer_vertex[i+1].resize(3);
                outer_vertex[i+1][0] = m_width - m_radius + circle_x;
                inner_vertex[i+1][0] = outer_vertex[i+1][0] - BORDER_LENGTH;

                if( m_round_corners & WGT_AREA_NE )
                {
                    outer_vertex[i+1][1] = m_height - m_radius + circle_y;
                    inner_vertex[i+1][1] = outer_vertex[i+1][1] - BORDER_LENGTH;
                }
                else
                {
                    outer_vertex[i+1][1] = (float)m_height;
                    inner_vertex[i+1][1] = outer_vertex[i+1][1] - BORDER_LENGTH;
                }

                if( m_round_corners & WGT_AREA_SE )
                {
                    outer_vertex[i+1][2] = m_radius - circle_y;
                    inner_vertex[i+1][2] = outer_vertex[i+1][2] + BORDER_LENGTH;
                }
                else
                {
                    outer_vertex[i+1][2] = 0;
                    inner_vertex[i+1][2] = outer_vertex[i+1][2] + BORDER_LENGTH;
                }

                glTexCoord2f(inner_vertex[i+1][0] / (float)m_width, inner_vertex[i+1][1] / (float)m_height);
                glVertex2f(inner_vertex[i+1][0] - HALF_WIDTH, inner_vertex[i+1][1] - HALF_HEIGHT);

                glTexCoord2f(inner_vertex[i+1][0] / (float)m_width, inner_vertex[i+1][2] / (float)m_height);
                glVertex2f(inner_vertex[i+1][0] - HALF_WIDTH, inner_vertex[i+1][2] - HALF_HEIGHT);
            }
        }
        glEnd();
    }
    glEndList();


    if(!glIsList(m_border_list))
    {
        m_border_list = glGenLists(1);
        if(m_border_list == 0)
        {
            std::cerr << "Error: could not create a widget's border list.\n";
            return false;
        }
    }

    glNewList(m_border_list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = NUM_QUADS * 2 + 1; i >= 0 ; --i)
            {
                glVertex2f(outer_vertex[i][0] - HALF_WIDTH, outer_vertex[i][1] - HALF_HEIGHT);
                glVertex2f(inner_vertex[i][0] - HALF_WIDTH, inner_vertex[i][1] - HALF_HEIGHT);
            }

            for (i = 0; i <= NUM_QUADS * 2 + 1; ++i)
            {
                glVertex2f(outer_vertex[i][0] - HALF_WIDTH, outer_vertex[i][2] - HALF_HEIGHT);
                glVertex2f(inner_vertex[i][0] - HALF_WIDTH, inner_vertex[i][2] - HALF_HEIGHT);
            }

            //Close the border
            glVertex2f(outer_vertex[NUM_QUADS * 2 + 1][0] - HALF_WIDTH, outer_vertex[NUM_QUADS * 2 + 1][1] - HALF_HEIGHT);
            glVertex2f(inner_vertex[NUM_QUADS * 2 + 1][0] - HALF_WIDTH, inner_vertex[NUM_QUADS * 2 + 1][1] - HALF_HEIGHT);
        }
        glEnd();

    }
    glEndList();

    return true;
}

//-----------------------------------------------------------------------------
void Widget::updateVariables( const float DELTA )
{
    if( m_enable_rotation ) m_rotation_angle += m_rotation_speed * DELTA;

    /*Handle delta time dependent features*/
    if(m_text_scale > MIN_TEXT_SCALE)
    {
        m_text_scale -= MIN_TEXT_SCALE * DELTA;
        if(m_text_scale < MIN_TEXT_SCALE) m_text_scale = MIN_TEXT_SCALE;
    }


    //For multilines we have to do a *very* ugly workaround for a plib
    //bug which causes multiline strings to move to the left, at least
    //while centering, and also gives wrong values for the size of the
    //text when there are multiple lines. Hopefully this work around will
    //be removed when we move away from plib; the scrolling and the other
    //text handling should be cleaned. Also, for some reason, different
    //positions are needed if the text is centered, and on top of that,
    //it's not 100% exact. Sorry for the mess.
    size_t line_end = 0;
    int lines = 0;

    do
    {
        line_end = m_text.find_first_of('\n', line_end + 1);
        ++lines;
    } while(line_end != std::string::npos);


    /* Handle preset scrolling positions */
    // In the Y-axis, a scroll position of 0 leaves the text centered, and
    // positive values lowers the text, and negatives (obviously) raise the
    // text, in the X-axis, a position of 0 leaves the text aligned to the
    // left; positive values move to the right and negative
    // values to the left.

    float left, right;
    m_font->getBBoxMultiLine(m_text, (float)m_text_size, false, &left, &right, NULL, NULL);
    int text_width = (int)(right - left + 0.99);

    const int Y_LIMIT = lines * m_text_size + m_height;

    //A work around for yet another bug with multilines: we get the wrong
    //width when using multilines.
    if(text_width > m_width)
    {
        text_width = m_width;
    }

    //With the preset positions, we do comparations with the equal sign on
    //floating point variables; however, no operations are done of the
    //variables between the assignment of these integer values and the
    //comparation and the values are small enough to fit in a few bytes,
    //so no inaccuracies because of floating point rounding should happen.
    //X-axis preset positions
    if( m_scroll_pos_x == WGT_SCROLL_START_LEFT )
    {
        m_scroll_pos_x = 0;
    }
    else if( m_scroll_pos_x == WGT_SCROLL_START_RIGHT )
    {
        m_scroll_pos_x = (float)m_width;
    }
    else if( m_scroll_pos_x == WGT_SCROLL_CENTER )
    {
        m_scroll_pos_x = (float)( (m_width - text_width) / 2 );
    }
    else if( m_scroll_pos_x == WGT_SCROLL_END_LEFT )
    {
        m_scroll_pos_x = (float)(-text_width);
    }
    else if( m_scroll_pos_x == WGT_SCROLL_END_RIGHT )
    {
        m_scroll_pos_x = (float)(m_width - text_width);
    }
    else if( m_scroll_pos_x > MAX_SCROLL )
    {
        std::cerr << "WARNING: text position is too much to the right to " <<
            "scroll!.\n";
    }
    else if( m_scroll_pos_x < -MAX_SCROLL )
    {
        std::cerr << "WARNING: text position is too much to the left to " <<
            "to scroll!.\n";
    }

    //Y-axis preset positions
    if( m_scroll_pos_y == WGT_SCROLL_START_TOP )
    {
        m_scroll_pos_y =(float)(Y_LIMIT / 2 - m_height);
    }
    else if( m_scroll_pos_y == WGT_SCROLL_START_BOTTOM )
    {
        m_scroll_pos_y = (float)(Y_LIMIT / 2);
    }
    else if( m_scroll_pos_y == WGT_SCROLL_CENTER )
    {
        m_scroll_pos_y = 0;
    }
    else if( m_scroll_pos_y == WGT_SCROLL_END_TOP )
    {
        m_scroll_pos_y = (float)(-Y_LIMIT / 2);
    }
    else if( m_scroll_pos_y == WGT_SCROLL_END_BOTTOM )
    {
        m_scroll_pos_y = (float)(-Y_LIMIT / 2 + m_height);
    }
    else if( m_scroll_pos_y > MAX_SCROLL )
    {
        std::cerr << "WARNING: text position too high to scroll!.\n";
    }
    else if( m_scroll_pos_y < -MAX_SCROLL )
    {
        std::cerr << "WARNING: text position too low to scroll!.\n";
    }

    if(m_enable_scroll)
    {
        //TODO: constrain speed to sane values
        m_scroll_pos_x += m_scroll_speed_x * DELTA;
        m_scroll_pos_y += m_scroll_speed_y * DELTA;

        //Y-axis wrapping
        if(m_scroll_pos_y * 2 > Y_LIMIT)
        {
            m_scroll_pos_y = WGT_SCROLL_END_TOP;
        }
        else if(-m_scroll_pos_y * 2 > Y_LIMIT)
        {
            m_scroll_pos_y = WGT_SCROLL_START_BOTTOM;
        }

        //X-axis wrapping
        if(m_scroll_pos_x > m_width )
        {
            m_scroll_pos_x = WGT_SCROLL_END_LEFT;
        }
        else if(m_scroll_pos_x < -text_width )
        {
            m_scroll_pos_x = WGT_SCROLL_START_RIGHT;
        }
    }
}

//-----------------------------------------------------------------------------
void Widget::draw()
{
    glPushMatrix();

    glClear( GL_STENCIL_BUFFER_BIT );

    applyTransformations();

    /*Start handling on/off features*/
    if( m_enable_texture )
    {
        glEnable( GL_TEXTURE_2D );
        if( glIsTexture ( m_texture ))
        {
            glBindTexture( GL_TEXTURE_2D, m_texture );
        }
        else
        {
            std::cerr << "Warning: widget tried to draw null texture.\n";
            std::cerr << "(Did you set the texture?)\n";
        }
    }
    else
    {
        //This ensures that a texture from another module doesn't affects the widget
        glDisable ( GL_TEXTURE_2D );
    }

    if( glIsList ( m_rect_list ))
    {
        //m_enable_rect == false doesn't disables this chunk of code because
        //we still need to draw the rect into OpenGL's selection buffer even
        //if it's not visible

        //FIXME: maybe there is some sort of stacking method to disable/enable
        //color masking
        if(!m_enable_rect)
        {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        }
        else
        {
            glColor4fv(m_rect_color);
        }

        //FIXME: I should probably revert the values to the defaults within the widget manager
        //(if glPushAttrib() doesn't), but right now this is the only thing using the
        //stencil test anyways.
        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glCallList(m_rect_list);

        if(!m_enable_rect)
        {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }
    else
    {
        std::cerr << "Warning: widget tried to draw null rect list.\n";
        std::cerr << "(Did you created the rect?)\n";
    }

    if(glIsList(m_border_list))
    {
        if( m_enable_border )
        {
            glDisable ( GL_TEXTURE_2D );
            glColor4fv(m_border_color);

            //FIXME: I should probably revert the values to the defaults within the widget manager
            //(if glPushAttrib() doesn't)
            glStencilFunc(GL_ALWAYS, 0x1, 0x1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glCallList(m_border_list);
        }
    }
    else
    {
        std::cerr << "Warning: widget tried to draw null border list.\n";
        std::cerr << "(Did you created the border?)\n";
    }

    if( m_enable_track )
    {
        if( m_track_num > (int)(track_manager->getNumberOfTracks()) - 1)
        {
            std::cerr << "Warning: widget tried to draw a track with a " <<
                "number bigger than the amount of tracks available.\n";
        }

        if( m_track_num != -1 )
        {
            /*track_manager->getTrack( m_track_num )->drawScaled2D( 0.0f, 
                0.0f, (float)m_width, (float)m_height);*/
        }
        else
        {
            std::cerr << "Warning: widget tried to draw an unset track.\n";
        }
    }

    if(m_enable_text)
    {
        //For multilines we have to do a *very* ugly workaround for a plib
        //bug which causes multiline strings to move to the left, at least
        //while centering, and also gives wrong values for the size of the
        //text when there are multiple lines. Hopefully this work around will
        //be removed when we move away from plib; the scrolling and the other
        //text handling should be cleaned. Also, for some reason, different
        //positions are needed if the text is centered, and on top of that,
        //it's not 100% exact. Sorry for the mess.
        size_t line_end = 0;
        int lines = 0;

        do
        {
            line_end = m_text.find_first_of('\n', line_end + 1);
            ++lines;
        } while( line_end != std::string::npos );


        float x_pos = (float)(m_scroll_pos_x - m_width * 0.5f);
        float y_pos = - (float)m_scroll_pos_y + (lines - 1 )* m_text_size / 2;

        size_t line_start = 0;
        bool draw;

        glStencilFunc(GL_EQUAL,0x1,0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        do
        {
            draw = true;
            if(y_pos + m_text_size / 2 > m_height / 2 )
            {
                if(y_pos - m_text_size / 2 >  m_height / 2) draw = false;
            }
            else if(y_pos + (m_height - m_text_size) / 2 < 0)
            {
                if(y_pos + (m_height + m_text_size) / 2 < 0) draw = false;
            }

            line_end = m_text.find_first_of('\n', line_start);

            if(draw)
            {
                glScalef(m_text_scale, m_text_scale, 1.0f);
                m_font->PrintBold(m_text.substr(line_start, line_end - line_start).c_str(), 
                    (float)m_text_size,
                    x_pos, y_pos - ((float)m_text_size / 2),
                    m_text_color, 1.0f, 1.0f);
                glScalef(1.0f/m_text_scale, 1.0f/m_text_scale, 1.0f);
            }

            y_pos -= m_text_size;
            line_start = line_end + 1;

        } while( line_end != std::string::npos );
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
void Widget::applyTransformations()
{
    /* OpenGL transformations are affected by the order of the calls; but the
     * operations must be called in the inverse order that you want them to
     * be applied, since the calls are stacked, and the one at the top is
     * done first, till the one at the bottom.
     */
    glTranslatef ( (GLfloat)(m_x + m_width * 0.5f), (GLfloat)(m_y + m_height * 0.5f), 0);

    if( m_enable_rotation )
    {
        glRotatef( (GLfloat)m_rotation_angle, 0.0f, 0.0f, (GLfloat)1.0f );
    }
}
