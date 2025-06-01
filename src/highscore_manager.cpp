//  $Id: highscores.hpp 921 2007-02-28 05:43:34Z hiker $
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

#include "highscore_manager.hpp"

#include <stdexcept>
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

#include "race_manager.hpp"
#include "file_manager.hpp"
#include "user_config.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

HighscoreManager* highscore_manager=0;

HighscoreManager::HighscoreManager()
{
    m_can_write=true;
    SetFilename();
    Load();
}   // HighscoreManager

// -----------------------------------------------------------------------------
HighscoreManager::~HighscoreManager()
{
    Save();
    for(type_all_scores::iterator i  = m_allScores.begin(); 
                                  i != m_allScores.end();  i++)
        delete *i;
}   // ~HighscoreManager

// -----------------------------------------------------------------------------
/** Determines the path to store the highscore file in
 */
void HighscoreManager::SetFilename()
{
    if (getenv("SUPERTUXKART_HIGHSCOREDIR") != NULL )
    {
        m_filename = getenv("SUPERTUXKART_HIGHSCOREDIR")
                 + std::string("/highscore.data");
    }
    else 
    {
        m_filename=file_manager->getHighscoreFile("highscore.data");
    }

    return;
}   // SetFilename

// -----------------------------------------------------------------------------
void HighscoreManager::Load()
{

    const lisp::Lisp* root = 0;
    std::exception err;

    try
    {
        lisp::Parser parser;
        root = parser.parse(m_filename);
    }
    catch(std::exception& err)
    {
        (void)err;   // remove warning about unused variable
        Save();
        if(m_can_write)
        {
            fprintf(stderr, "New highscore file '%s' created.\n", 
                    m_filename.c_str());
        }
        delete root;
        return;
    }
    try
    {
        // check for opening 'highscores' nodes
        const lisp::Lisp* const node = root->getLisp("highscores");
        if(!node)
        {
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf(msg, sizeof(msg), "No 'highscore' node found.");
            throw std::runtime_error(msg);
        }
        
        // check file version
        int v;
        if (!node->get("file-version",v) || v<(int)CURRENT_HSCORE_FILE_VERSION)
        {
            fprintf(stderr, "Highscore file format too old, a new one will be created.\n");
            std::string warning = _("The highscore file was too old,\nall highscores have been erased.");
            user_config->setWarning( warning );
            
            // since we haven't had the chance to load the current scores yet,
            // calling Save() now will generate an empty file.
            Save();
            return;
        }
        
        // get number of entries
        int n;
        if (!node->get("number-entries",n))
        {
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf(msg, sizeof(msg), "No 'number-entries' node found.");
            throw std::runtime_error(msg);
        }

        // read all entries one by one and store them in 'm_allScores'
        for(int i=0; i<n; i++)
        {
            char record_name[255];
            snprintf(record_name, sizeof(record_name), "record-%d", i);
            const lisp::Lisp* const node_record=node->getLisp(record_name);
            if(!node_record) 
            {
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg),"Can't find record '%d' in '%s'",
                         i,m_filename.c_str());
                throw std::runtime_error(msg);
            }
            HighscoreEntry *highscores = new HighscoreEntry(node_record);
            m_allScores.push_back(highscores);
            //highscores->Read(node_record);
        }// next entry
        
        fprintf(stderr, "Highscores will be saved in '%s'.\n",m_filename.c_str());
    }
    catch(std::exception& err)
    {
        fprintf(stderr, "Error while parsing highscore file '%s':\n", 
                m_filename.c_str());
        fprintf(stderr, "%s", err.what());
        fprintf(stderr, "\n");
        fprintf(stderr, "No old highscores will be available.\n");
    }
    delete root;
}   // Load

// -----------------------------------------------------------------------------
void HighscoreManager::Save()
{
    // Print error message only once
    if(!m_can_write) return;
    try
    {
        lisp::Writer writer(m_filename);
        writer.beginList("highscores");
        writer.writeComment("File format version");
        writer.write("file-version\t", CURRENT_HSCORE_FILE_VERSION);
        writer.writeComment("Number of highscores in this file");
        writer.write("number-entries\t",(unsigned int)m_allScores.size());
        int record_number=0;
        for(type_all_scores::iterator i  = m_allScores.begin(); 
            i != m_allScores.end();  i++)
        {
            char record_name[255];
            snprintf(record_name, sizeof(record_name),"record-%d\t",record_number);
            record_number++;
            writer.beginList(record_name);
            (*i)->Write(&writer);
            writer.endList(record_name);
        } // next score
        writer.endList("highscores");
        m_can_write=true;
    }   // try
    catch(std::exception &e)
    {
        printf("Problems saving highscores in '%s'\n",
               m_filename.c_str());
        puts(e.what());
        m_can_write=false;
    }
}   // Save

// -----------------------------------------------------------------------------
/*
 * Returns the high scores entry for a specific type of race. Creates one if none exists yet.
 */
HighscoreEntry* HighscoreManager::getHighscoreEntry(const HighscoreEntry::HighscoreType highscore_type,
                                                    int num_karts, const RaceManager::Difficulty difficulty, 
                                                    const std::string trackName, const int number_of_laps)
{
    HighscoreEntry *highscores = 0;

    // See if we already have a record for this type
    for(type_all_scores::iterator i  = m_allScores.begin(); 
                                  i != m_allScores.end();  i++)
    {
        if((*i)->matches(highscore_type, num_karts, difficulty, trackName, number_of_laps) )
        {
            // we found one entry for this kind of race, return it
            return (*i);
        }
    }   // for i in m_allScores

    // we don't have an entry for such a race currently. Create one.
    highscores = new HighscoreEntry(highscore_type, num_karts, difficulty, trackName, number_of_laps);
    m_allScores.push_back(highscores);
    return highscores;
}   // getHighscoreEntry
