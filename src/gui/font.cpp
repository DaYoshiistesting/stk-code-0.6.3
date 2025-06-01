//  $Id: font.hpp 907 2007-02-04 01:38:54Z coz $
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

#include <vector>

#include "file_manager.hpp"
#include "user_config.hpp"
#include "gui/font.hpp"
#include "utils/string_utils.hpp"

Font* font_gui;
Font* font_race;

int init_fonts()
{
    // As we don't have any cyrillic fonts, we'll get it from 0.7.
    font_gui = new Font("DomesticManners.txf");
    //font_gui_cyrillic = new Font("LayneHansomCyrillic.txf");
    font_race = new Font("DomesticManners.txf");
    //font_race_cyrillic = new Font("LayneHansomCyrillic.txf");
    return(font_gui && font_race);
}   // init_fonts

// =============================================================================
int delete_fonts()
{
    delete font_gui;
    //delete font_gui_cyrillic;
    delete font_race;
    //delete font_race_cyrillic;
    return 0;
}   // delete_fonts

// =============================================================================
Font::Font(const char *fontname)
{
    m_fnt      = new fntTexFont(file_manager->getFontFile(fontname).c_str(),
                                GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_text_out = new fntRenderer();
    m_text_out->setFont(m_fnt);
}   // Font

// -----------------------------------------------------------------------------
Font::~Font()
{
    delete m_text_out;
    delete m_fnt;
}   // ~Font

// -----------------------------------------------------------------------------
void Font::Print(const char *text, float size, float x, float y, 
                 const GLfloat* color, float scale_x, float scale_y,
                 int left, int right, int top, int bottom, bool doShadow)
{
    float fontScaling = user_config->m_width<800 ? ((float)user_config->m_width/800.0f)
                                                 : 1.0f;
    float sz = size*std::min(scale_x,scale_y)*fontScaling;

    float l,r,t,b;
    m_fnt->getBBox(text, sz, 0, &l, &r, &b, &t);
    const float W = r-l+0.99f;
    const float H = t-b+0.99f;

    if(x==CENTER_OF_SCREEN)
    {
        if(left ==-1) left  = 0;
        if(right==-1) right = user_config->m_width-1;
        int width = right-left+1;
        x = (float)((width - W)/2 + left);
    }

    if(y==CENTER_OF_SCREEN)
    {
        if(top    == -1) top    = user_config->m_height-1;
        if(bottom == -1) bottom = 0;
        int height = top-bottom+1;
        y = (float)((height - H)/2 + bottom);
    }

    m_text_out->begin();
    m_text_out->setPointSize((float)sz);
    if(doShadow)
    {
        m_text_out->start2f(x-2, y-2);
        glColor4ub(0, 0, 0, 100);
        m_text_out->puts(text);
    }
    m_text_out->start2f(x, y);

    if(color == NULL)
    {
        glColor4f(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        glColor4fv(color);
    }
    m_text_out->puts(text);
    m_text_out->end();

}   // Print
// -----------------------------------------------------------------------------
void Font::PrintBold(const std::string &text, float size, float x, float y,
                     const GLfloat* color, float scale_x, float scale_y,
                     int left, int right, int top, int bottom)
{
    float fontScaling = user_config->m_width<800 ? ((float)user_config->m_width/800.0f)
                                                 : 1.0f;
    float sz = size*std::max(scale_x,scale_y)*fontScaling;

    float l,r,t,b;
    getBBoxMultiLine(text, sz, 0, &l, &r, &b, &t);
    const float W = r-l+0.99f;
    const float H = t-b+0.99f;

    if(x==CENTER_OF_SCREEN)
    {
        if(left ==-1) left  = 0;
        if(right==-1) right = user_config->m_width-1;
        int width = right-left+1;
        x = (float)((width - W)/2 + left);
    }

    if(y==CENTER_OF_SCREEN)
    {
        if(top    == -1) top    = user_config->m_height-1;
        if(bottom == -1) bottom = 0;
        int height = top-bottom+1;
        y = (float)((height - H)/2 + bottom);
    }

    m_text_out->begin();
    m_text_out->setPointSize((float)sz);

    // print shadow
    // ------------
    glColor4f(0.0f,0.0f,0.0f,1.0f);
    float xf=(GLfloat)x+2;
    float yf=(GLfloat)y-2;
    for(float r=-1; r<=0; r+=0.5)
    {
        m_text_out->start2f(xf-r, yf-r);
        m_text_out->puts(text.c_str());
    }

    if(color == NULL)
    {
        glColor4f(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        glColor4fv(color);
    }
    xf=x;
    yf=y;
    for(float r=-1.0f; r<=0.0f; r+=0.5f)
    {
        // This kind of simulates an outline, but it's not too good
        //if(r==-1.0f || r==0.0f)
        //    glColor4f(0.0f,0.0f,0.0f,1.0f);
        //else
        //    glColor4f(1.0f,1.0f,1.0f,1.0f);

        m_text_out->start2f(x+r, y+r);
        m_text_out->puts(text.c_str());
    }
    m_text_out->end();

}   // PrintBold

// ----------------------------------------------------------------------------
void Font::getBBox(const std::string &text, float size, bool italic,
                  float *left, float *right, float *bot, float *top)
{
    m_fnt->getBBox(text.c_str(), size, italic, left, right, bot, top);
    if(user_config->m_width<800)
    {
        float fract=(float)user_config->m_width/800.0f;
        *left  *= fract;
        *right *= fract;
        if(bot) *bot   *= fract;
        if(top) *top   *= fract;
    }

}
// ----------------------------------------------------------------------------
void Font::getBBoxMultiLine(const std::string &text, float size, bool italic,
                            float *left, float *right, float *bot, float *top)
{
    // Plib does not handle multi-lines strings as expected. So as a work
    // around we split strings into lines, and compute the size for each
    // line, and take the maximum size at the end.
    std::vector<std::string> s=StringUtils::split(text,'\n');
    m_fnt->getBBox(s[0].c_str(), size, italic, left, right, bot, top);
    for(unsigned int i=1; i<s.size(); i++)
    {
        float l,r,b,t;
        m_fnt->getBBox(s[i].c_str(), size, italic, &l, &r, &b, &t);
        if(left) *left  = std::min(*left,  l);
        if(bot ) *bot   = std::min(*bot,   b);
        if(right)*right = std::max(*right, r); 
        if(top)  *top   = std::max(*top,   t);
    }
    if(user_config->m_width<800) {
        float fract=(float)user_config->m_width/800.0f;
        *left  *= fract;
        *right *= fract;
        if(bot) *bot   *= fract;
        if(top) *top   *= fract;
    }

}
