/*
 * sipcmd, main.cpp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 *
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include <ptlib.h>

// See README.txt for <prog> syntax.

class Command {
  protected:
    static std::string errorstring;

  public:
    virtual ~Command() { }

    // parses command sequence
    static bool Parse( const char *cmds, std::vector< Command*> &sequence);

    // runs command sequence
    // -returns whether successful or not
    static bool Run(
        std::vector< Command*> &sequence,
        const std::string &loopsuffix = "");

    // deletes all commands in the sequence
    static void DeleteSequence( std::vector< Command*> &sequence);
    
    // returns the error message
    static const std::string &GetErrorString( void) {
      return errorstring;
    }
    
    // parses this command
    // -appends command to the vector
    // -increments cmds pointer
    // -returns whether successful or not
    virtual bool ParseCommand(
        const char **cmds, std::vector< Command*> &sequence) = 0;
    
    // runs command
    // -returns whether successful or not
    virtual bool RunCommand( const std::string &loopsuffix = "") = 0;
};


// call	    := 'c' remoteparty
class Call : public Command {
  private:
    PString remoteparty;

  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};


// answer   := 'a' [ expectedremoteparty ]
class Answer : public Command {
  private:
    PString expectedparty;
  
  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};


// hangup   := 'h'
class Hangup : public Command {
  private:
  
  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};


// dtmf	    := 'd' digits
class DTMF : public Command {
  private:
    PString digits;
  
  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};


// voice    := 'v' Wav-audiofile
class Voice : public Command {
  private:
    PString audiofilename;
  
  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};


// record   :=  'r' [ append ] [ silence ] [ iter ] millis audiofile
// append   :=  'a'
// silence  :=  's'
// iter	    :=  'i'
class Record : public Command {
  private:
    bool append;
    bool silence;
    bool iterationsuffix;
    PString audiofilename;
    int millis;
  
  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};


// wait	    := 'w' [ activity | silence ] [ closed ] millis
// activity := 'a'
// silence  := 's'
// closed   := 'c'
class Wait : public Command {
  private:
    bool activity;
    bool silence;
    bool closed;
    size_t millis;


  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};


// setlabel := 'l' label
class Label : public Command {
  private:
    PString label;

  public:
    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "") { return true; }
    const PString &GetLabel() const {
      return label;
    }
};

// loop	    := 'j' [ how-many-times ] [ 'l' label ]
class Loop : public Command {
  private:
    int loops;
    std::vector< Command*> loopedsequence;

    bool PopToInnerSequence(
        PString &label, std::vector< Command*> &sequence);

  public:
    ~Loop() {
      Command::DeleteSequence( loopedsequence);
    }

    bool ParseCommand( const char **cmds, std::vector< Command*> &sequence);
    bool RunCommand( const std::string &loopsuffix = "");
};

#endif // COMMANDS_H
//**// END OF FILE //**//
// vim: tw=78 sw=2 sts=2
