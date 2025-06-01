//  $Id: lexer.cpp 3037 2009-01-25 19:57:13Z auria $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
//  code in this file based on lispreader from Mark Probst
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
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <cstring>

#include "lisp/lexer.hpp"

namespace lisp
{

    class EOFException
        {}
    ;

    Lexer::Lexer(std::istream& newstream)
            : m_stream(newstream), m_is_eof(false)
    {
        try
        {
            // trigger a refill of the m_buffer
            m_c = 0;
            m_buffer_end = m_c + 1;
            nextChar();
        }
        catch(EOFException& e)
        {(void)e;  // avoid 'unreferenced local variable' warning
        }
    }

//-----------------------------------------------------------------------------

    Lexer::~Lexer()
    {}

//-----------------------------------------------------------------------------

    void
    Lexer::nextChar()
    {
        ++m_c;
        if(m_c >= m_buffer_end)
        {
            if(m_is_eof)
                throw EOFException();
            m_stream.read(m_buffer, LEXER_BUFFER_SIZE);
            std::streamsize n = m_stream.gcount();

            m_c = m_buffer;
            m_buffer_end = m_buffer + n;

            // the following is a hack that appends an additional ' ' at the end of
            // the file to avoid problems when parsing symbols/elements and a sudden
            // EOF. This is faster than relying on unget and IMO also nicer.
            if(n < LEXER_BUFFER_SIZE || n == 0)
            {
                *m_buffer_end = ' ';
                ++m_buffer_end;
                m_is_eof = true;
            }
        }
    }

//-----------------------------------------------------------------------------

    Lexer::TokenType
    Lexer::getNextToken()
    {
        static const char* delims = "\"();";

        try
        {
            while(isspace(*m_c))
            {
                nextChar();
                if(*m_c == '\n')
                    ++m_line_number;
            };

            m_token_length = 0;

            switch(*m_c)
            {
            case ';': // comment
                while(true)
                {
                    nextChar();
                    if(*m_c == '\n')
                    {
                        ++m_line_number;
                        break;
                    }
                }
                return getNextToken(); // and again
            case '(':
                nextChar();
                return TOKEN_OPEN_PAREN;
            case ')':
                nextChar();
                return TOKEN_CLOSE_PAREN;
            case '"': // string
                {
                    const int STARTLINE = m_line_number;
                    try
                    {
                        while(1)
                        {
                            nextChar();
                            if(*m_c == '"')
                                break;

                            if(*m_c == '\\')
                            {
                                nextChar();
                                switch(*m_c)
                                {
                                case 'n':
                                    *m_c = '\n';
                                    break;
                                case 't':
                                    *m_c = '\t';
                                    break;
                                }
                            }
                            if(m_token_length < MAX_TOKEN_LENGTH)
                                m_token_string[m_token_length++] = *m_c;
                        }
                        m_token_string[m_token_length] = 0;
                    }
                    catch(EOFException& )
                    {
                        std::ostringstream msg;
                        msg << "Parse error in line " << STARTLINE
                            << ": EOF while parsing string.";
                        throw std::runtime_error(msg.str());
                    }
                    nextChar();
                    return TOKEN_STRING;
                }
            case '#': // constant
                try
                {
                    nextChar();

                    while(isalnum(*m_c) || *m_c == '_')
                    {
                        if(m_token_length < MAX_TOKEN_LENGTH)
                            m_token_string[m_token_length++] = *m_c;
                        nextChar();
                    }
                    m_token_string[m_token_length] = 0;
                }
                catch(EOFException& )
                {
                    std::ostringstream msg;
                    msg << "Parse Error in line " << m_line_number
                        << ": EOF while parsing constant.";
                    throw std::runtime_error(msg.str());
                }

                if(strcmp(m_token_string, "t") == 0)
                    return TOKEN_TRUE;
                if(strcmp(m_token_string, "f") == 0)
                    return TOKEN_FALSE;

                // this would be the place to add more sophisticated handling of
                // constants

                {
                    std::ostringstream msg;
                    msg << "Parse Error in line " << m_line_number 
                        << ": Unknown constant '" << m_token_string<<"'.";
                    throw std::runtime_error(msg.str());
                }

            case '_': // can be begin translation
                try
        {
            nextChar();
            if(*m_c == '(')
            {
              nextChar();
              return TOKEN_TRANSLATION;
            }
            m_token_string[m_token_length++] = '_';
            // Fall through to symbol handling
        }  
                catch(EOFException& )
                {
        }
            default:
                if(isdigit(*m_c) || *m_c == '-')
                {
                    bool have_nondigits = false;
                    bool have_digits = false;
                    int have_floating_point = 0;

                    do
                    {
                        if(isdigit(*m_c))
                            have_digits = true;
                        else if(*m_c == '.')
                            ++have_floating_point;
                        else if(isalnum(*m_c) || *m_c == '_')
                            have_nondigits = true;

                        if(m_token_length < MAX_TOKEN_LENGTH)
                            m_token_string[m_token_length++] = *m_c;

                        nextChar();
                    }
                    while(!isspace(*m_c) && !strchr(delims, *m_c));

                    m_token_string[m_token_length] = 0;

                    // no nextChar

                    if(have_nondigits || !have_digits || have_floating_point > 1)
                        return TOKEN_SYMBOL;
                    else if(have_floating_point == 1)
                        return TOKEN_REAL;
                    else
                        return TOKEN_INTEGER;
                }
                else
                {
                    do
                    {
                        if(m_token_length < MAX_TOKEN_LENGTH)
                            m_token_string[m_token_length++] = *m_c;
                        nextChar();
                    }
                    while(!isspace(*m_c) && !strchr(delims, *m_c));
                    m_token_string[m_token_length] = 0;

                    // no nextChar

                    return TOKEN_SYMBOL;
                }
            }
        }
        catch(EOFException& )
        {
            return TOKEN_EOF;
        }
    }

} // end of namespace lisp

