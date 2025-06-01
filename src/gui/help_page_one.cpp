//  $Id: help_menu.cpp 812 2006-10-07 11:43:57Z hiker $
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

#include "gui/help_page_one.hpp"

#include "user_config.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "items/item_manager.hpp"
#include "utils/translation.hpp"

enum WidgetTokens
{
    WTOK_MSG1,
    WTOK_MSG2,
    WTOK_MSG3,
    WTOK_MSG4,
    WTOK_MSG5,
    WTOK_MSG6,
    WTOK_MSG7,
    WTOK_MSG8,
    
    WTOK_LOCK,

    WTOK_SECOND_PAGE,
    WTOK_QUIT
};

HelpPageOne::HelpPageOne()
{
    //The ssgContext constructor calls makeCurrent(), it has to be restored
    ssgContext* oldContext = ssgGetCurrentContext();
    m_context = new ssgContext;
    oldContext->makeCurrent();

    m_box = 0;
    m_small_nitro = 0;
    m_big_nitro = 0;
    m_banana = 0;

    m_clock = 0;

    //FIXME: instead of using setInitialTextState, the gui & widget manager macros should improve it's design
    widget_manager->setInitialTextState
    (
        false,
        "",
        WGT_FNT_SML,
        WGT_FONT_GUI,
        WGT_WHITE,
        true
    );
    widget_manager->addTextWgt( WTOK_MSG1, 50, 7,
        _("Force your rivals bite *your* dust!") );
    widget_manager->breakLine();

    widget_manager->addTextWgt( WTOK_MSG2, 60, 7,
        _("Grab blue boxes and nitro") );
    widget_manager->addTextWgt( WTOK_MSG3, 30, 7, _("Avoid bananas") );
    widget_manager->breakLine();

    /*Rotating 3D models*/
    ssgEntity* hm = ItemManager::getItemModel(Item::ITEM_BONUS_BOX);
    ssgDeRefDelete(m_box);
    m_box = new ssgTransform;
    m_box->ref();
    m_box->addKid(hm);

    hm = ItemManager::getItemModel(Item::ITEM_SMALL_NITRO);
    ssgDeRefDelete(m_small_nitro);
    m_small_nitro = new ssgTransform;
    m_small_nitro->ref();
    m_small_nitro->addKid(hm);

    hm = ItemManager::getItemModel(Item::ITEM_BIG_NITRO);
    ssgDeRefDelete(m_big_nitro);
    m_big_nitro = new ssgTransform;
    m_big_nitro->ref();
    m_big_nitro->addKid(hm);

    hm = ItemManager::getItemModel(Item::ITEM_BANANA);
    ssgDeRefDelete(m_banana);
    m_banana = new ssgTransform;
    m_banana->ref();
    m_banana->addKid(hm);

    /*Empty widget to cover the space for the 3D models*/
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 15);
    widget_manager->breakLine();
/*
    widget_manager->addTextWgt(WTOK_MSG4, 100, 13,
//Next line starts at column 0 to avoid spaces in the GUI
_("At high speeds wheelies drive you faster, but you can't steer. If you\n\
get stuck or fall too far, use the rescue button to get back on track."));
    widget_manager->setWgtResizeToText( WTOK_MSG4, false );
    widget_manager->breakLine();*/

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1);
    widget_manager->breakLine();

    widget_manager->addTextWgt(WTOK_MSG5, 80, 10,
        _("The current key bindings can be seen/changed in the\nOptions->Player Config menu."));
    widget_manager->breakLine();
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1);
    widget_manager->breakLine();

    widget_manager->addTextWgt(WTOK_MSG6, 100, 17,
_("Collecting nitro allows you to get speed boosts whenever you\nwish by pressing the appropriate key. You can see your\ncurrent level of nitro in the bar at the right of the game screen."));
    widget_manager->setWgtResizeToText( WTOK_MSG6, false);
    widget_manager->breakLine();
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1);
    widget_manager->breakLine();

    widget_manager->addTextWgt(WTOK_MSG7, 87, 8,
_("If you see a button with a lock like the one to the right,\n\
you need to complete a challenge to unlock it."));

    widget_manager->addImgWgt(WTOK_LOCK, 20, 8, 0);
    widget_manager->setWgtLockTexture(WTOK_LOCK);
    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1);
    widget_manager->breakLine();

    
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1);
    widget_manager->breakLine();
    
    widget_manager->addTextWgt(WTOK_MSG8, 87, 8,
                               _("The 'sharp turn' key allows you to do sharp turns\nand have better control in tight curves"));
    widget_manager->breakLine();
    
    /*Buttons at the bottom*/
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 16);
    widget_manager->breakLine();
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 30, 7);
    widget_manager->addTextButtonWgt(WTOK_QUIT, 40, 7,
                                     _("Back to the menu"));
    
    widget_manager->addTextButtonWgt(WTOK_SECOND_PAGE, 30, 7,
        _("Next >"));

    widget_manager->layout( WGT_AREA_TOP );
}   // HelpPageOne

//-----------------------------------------------------------------------------
HelpPageOne::~HelpPageOne()
{
    widget_manager->reset();

    if (m_box != NULL && m_small_nitro != NULL && m_big_nitro != NULL
        && m_banana != NULL )
    {
        ssgDeRefDelete(m_box);
        ssgDeRefDelete(m_small_nitro);
        ssgDeRefDelete(m_big_nitro);
        ssgDeRefDelete(m_banana);
    }

    delete m_context;

}   // ~HelpPageOne

//-----------------------------------------------------------------------------
void HelpPageOne::update(float dt)
{
    m_clock += dt * 40.0f;

    if (m_box != NULL && m_small_nitro != NULL && m_big_nitro != NULL
        && m_banana != NULL )
    {
        ssgContext* oldContext = ssgGetCurrentContext();
        m_context -> makeCurrent();

        glClear(GL_DEPTH_BUFFER_BIT);

        sgCoord cam_pos;
        sgSetCoord(&cam_pos, 0, 0, 0, 0, 0, 0);
        m_context -> setCamera ( &cam_pos ) ;

        glEnable (GL_DEPTH_TEST);
        sgCoord trans;
        sgSetCoord(&trans, -4, 10, 1.85f, m_clock, 0, 0);
        m_box->setTransform (&trans);

        sgSetCoord(&trans, -2, 8, 1.5f, m_clock, 0, 0);
        m_small_nitro->setTransform (&trans);

        sgSetCoord(&trans, -1, 8, 1.5f, m_clock, 0, 0);
        m_big_nitro->setTransform (&trans);

        sgSetCoord(&trans, 5, 15, 3, m_clock, 0, 0);
        m_banana->setTransform (&trans);

        //glShadeModel(GL_SMOOTH);
        ssgCullAndDraw ( m_box ) ;
        ssgCullAndDraw ( m_small_nitro ) ;
        ssgCullAndDraw ( m_big_nitro ) ;
        ssgCullAndDraw ( m_banana ) ;
        glViewport ( 0, 0, user_config->m_width, user_config->m_height ) ;

        glDisable (GL_DEPTH_TEST);
        oldContext->makeCurrent();
    }

    widget_manager->update(dt);
}

//-----------------------------------------------------------------------------
void HelpPageOne::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
        case WTOK_SECOND_PAGE:
            //This switches the first page with the second page, so they
            //are not stacked by the menu manager, and the menu that called
            //this help is the one that gets called back when the next page
            //is popped.
            menu_manager->popMenu();
            menu_manager->pushMenu(MENUID_HELP2);
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
