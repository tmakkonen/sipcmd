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

#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <ctime>
#include <cassert>

#include "commands.h"
#include "state.h"

////
// Command
std::string Command::errorstring;

bool Command::Parse(const char *cmds, std::vector< Command*> &sequence) {
  const char *ptr = cmds;
  Command *newcmd = NULL;
  while(*ptr) {
    switch(*ptr) {
      case 'c':
	newcmd = new Call();
	break;
      case 'a':
	newcmd = new Answer();
	break;
      case 'h':
	newcmd = new Hangup();
	break;
      case 'd':
	newcmd = new DTMF();
	break;
      case 'v':
	newcmd = new Voice();
	break;
      case 'r':
	newcmd = new Record();
	break;
      case 'w':
	newcmd = new Wait();
	break;
      case 'l':
	newcmd = new Label();
	break;
      case 'j':
	newcmd = new Loop();
	break;
      default:
	newcmd = NULL;
	break;
    };
    ptr++;
    if(newcmd) {
      if(!newcmd->ParseCommand(&ptr, sequence))
	return false;
    }
  }
  return true;
}

bool Command::Run(
    std::vector< Command*> &sequence, const std::string &loopsuffix) {

  std::vector< Command*>::iterator it = sequence.begin();
  for(; it != sequence.end(); it++)
    if(!(*it)->RunCommand(loopsuffix))
      return false;

  return true;
}

void Command::DeleteSequence(std::vector< Command*> &sequence) {

  while(!sequence.empty()) {
    delete sequence[0];
    sequence.erase(sequence.begin());
  }
}



////
// Call
bool Call::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {
  size_t i = 0U;
  for(; (*cmds)[i]  &&  (*cmds)[i] != ';'; i++)
    ;

  if(!i) {

    errorstring = "Call: Empty remote party";
    return false;
  }

  remoteparty = PString(*cmds, i);
  *cmds = &((*cmds)[i]);
  sequence.push_back(this);
  return true;
}

bool Call::RunCommand(const std::string &loopsuffix) {
  std::cout << "## Call ##" << std::endl;
  // set up
  PString token;
  TPState &tpstate = TPState::Instance();

  // concatenate gw to remote party name
  // if one has been specified and there is no address for username
  PString rp = remoteparty;
  PString gw = TPState::Instance().GetGateway();
  char buf[256];
  time_t secsnow = time(NULL);
  if(rp.Find('@') == P_MAX_INDEX  &&  !gw.IsEmpty()) {
    cout << "TestPhone::Main: calling \""
      << rp << "\" using gateway \"" << gw << "\""
      << " at " << ctime_r(&secsnow, buf) << endl;

    switch (TPState::Instance().GetProtocol()) {
      case TPState::SIP: rp = "sip:" + rp + "@" + gw; break;
      case TPState::TPState::H323: rp = "h323:" + rp + "@" + gw; break;
      case TPState::RTP: rp = "rtp:" + rp;  break;
      default: assert(0);
    }

    //rp += "@";
    //rp += gw;
  }
  else {
    cout << "TestPhone::Main: calling \"" << rp << "\""
      << " at " << ctime_r(&secsnow, buf) << endl;
  }

  // dial out
  TPState::TPConnState state = TPState::CONNECTING;
  if (!TPState::Instance().GetManager()->MakeCall(rp))
    return false;
  
  // wait for connection (or termination)
  // rtp does not change state via callback, we can skip the following.
  if (tpstate.GetProtocol() != TPState::RTP)
    tpstate.SetState(state);

  do {
    cout << "TestPhone::Main: calling \"" << rp << "\""
	 << " for " << difftime(time(NULL), secsnow) << endl;
    state = tpstate.WaitForStateChange(TPState::ESTABLISHED);
    if(state == TPState::TERMINATED) {
      errorstring = "Call: application terminated";
      return false;
    } else if (difftime(time(NULL), secsnow) > DIAL_TIMEOUT) {
      errorstring = "Call: Dial timed out";
      return false;
    }
  } while(state == TPState::CONNECTING);

  tpstate.SetSilenceState(false);
  
  /* TODO
  cout << "Call: connection established, "
    << "RemotePartyName='" << connection->GetRemotePartyName() << "'"
    << endl;
    */
  return true;
}



////
// Answer
bool Answer::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {

  size_t i = 0U;
  for(; (*cmds)[i]  &&  (*cmds)[i] != ';'; i++)
    ;

  if(i) 
    expectedparty = PString(*cmds, i);

  *cmds = &((*cmds)[i]);
  sequence.push_back(this);

  TPState::Instance().GetManager()->SetListenMode(true);
  return true;
}

bool Answer::RunCommand(const std::string &loopsuffix) {
  std::cout << "## Answer ##" << std::endl;
  char buf[256];
  time_t secsnow = time(NULL);
  cout << "Answer: starting at " << ctime_r(&secsnow, buf) << endl;

  // set up
  PString token;
  TPState &tpstate = TPState::Instance();

  // Start listener thread
  TPState::TPConnState state = TPState::CONNECTING;
  tpstate.SetState(state);

  if (!tpstate.GetManager()->IsListenerUp() && 
      !tpstate.GetManager()->StartListener())
    return false;

  // wait for connection (or termination)
  do {
    state = tpstate.WaitForStateChange(TPState::ESTABLISHED);
    if(state == TPState::TERMINATED) {
      errorstring = "Answer: application terminated";
      return false;
    }
  } while(state == TPState::CONNECTING);

  // TODO tpstate.SetSilenceState(false);
  cout << "Answer: connection established" << endl;
  return true;
}



////
// Hangup
bool Hangup::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {
  
  for(; **cmds  &&  **cmds != ';'; (*cmds)++)
    ;
  
  sequence.push_back(this);
  return true;
}

bool Hangup::RunCommand(const std::string &loopsuffix) {

  std::cout << "## Hangup ##" << std::endl;
  char buf[256];
  time_t secsnow = time(NULL);
  cout << "Hangup: at " << ctime_r(&secsnow, buf) << endl;
  TPState &tpstate = TPState::Instance();

  // hangup
  tpstate.GetManager()->ClearAllCalls();
  return true;
}



////
// DTMF
bool DTMF::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {

  size_t i = 0U;
  for(; (*cmds)[i]  &&  (*cmds)[i] != ';'
      &&  strchr("0123456789#*ABCD!", (*cmds)[i]); i++) 
    ;

  if(!i  ||  ((*cmds)[i]  &&  (*cmds)[i] != ';')) {
    errorstring = "DTMF: No digits or invalid digits specified";
    return false;
  }

  digits = PString(*cmds, i);
  *cmds = &((*cmds)[i]);
  sequence.push_back(this);
  return true;
}

bool DTMF::RunCommand(const std::string &loopsuffix) {

    cout << "## DTMF \"" << digits << "\" ##" << endl;
    return
      TPState::Instance().GetManager()->SendDTMF(digits);
}


////
// Voice
bool Voice::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {
  size_t i = 0U;
  for(; (*cmds)[i]  &&  (*cmds)[i] != ';'; i++);
  if(!i) {
    errorstring = "Voice: empty audio filename";
    return false;
  }

  audiofilename = PString(*cmds, i);
  *cmds = &((*cmds)[i]);
  sequence.push_back(this);
  return true;
}

bool Voice::RunCommand(const std::string &loopsuffix) {

  std::cout << "## Voice audiofile="<< audiofilename << " ##" << std::endl;

  // playback audio
   bool ok = 
       TPState::Instance().GetPlayBackAudio().PlaybackAudioFile(audiofilename);

  // check result
  if(TPState::Instance().GetState() == TPState::TERMINATED) {
    errorstring = "Voice: application terminated";
    return false;
  }
  if(!ok) {
    std::string f = audiofilename;
    errorstring = "Voice: error reading file \"" + f + "\"";
  }
  return ok;
}



////
// Record
bool Record::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {
  size_t optsize = strcspn(*cmds, "0123456789;");
  if(optsize) {
    // append
    append = memchr(*cmds, 'a', optsize);
    if(!append)
      append = memchr(*cmds, 'A', optsize);
    // silence
    silence = memchr(*cmds, 's', optsize);
    if(!silence)
      silence = memchr(*cmds, 'S', optsize);
    // iteration-based naming
    iterationsuffix = memchr(*cmds, 'i', optsize);
    if(!iterationsuffix)
      iterationsuffix = memchr(*cmds, 'I', optsize);
    *cmds += optsize;
  }
  else
    append = silence = iterationsuffix = false;
  // millis
  size_t i = 0U;
  for(; (*cmds)[i]  &&  (*cmds)[i] != ';'  &&  isdigit((*cmds)[i]); i++);
  if(!i) {
    errorstring = "Record: No digits or invalid digits specified";
    return false;
  }
  sscanf(*cmds, "%u", &millis);
  *cmds = &((*cmds)[i]);
  // filename
  for(i = 0U; (*cmds)[i]  &&  (*cmds)[i] != ';'; i++);
  if(!i) {
    errorstring = "Record: empty audio filename";
    return false;
  }
  audiofilename = PString(*cmds, i);
  *cmds = &((*cmds)[i]);
  sequence.push_back(this);
  return true;
}

bool Record::RunCommand(const std::string &loopsuffix) {
  // create filename
  PString filename;
  if(iterationsuffix) {
    PINDEX fn = audiofilename.FindLast('/');
    PINDEX ext = audiofilename.Find('.', fn == P_MAX_INDEX? 0: fn);
    if(ext == P_MAX_INDEX)
      filename = audiofilename + loopsuffix;
    else
      filename =
	audiofilename.Left(ext)
	+ loopsuffix
	+ audiofilename.Mid(ext);
  }
  else
    filename = audiofilename;
  
  // record audio
  bool ok = 
    TPState::Instance().GetRecordAudio().RecordAudioFile(
        filename, append, silence, millis);
  
  // check result
  if(TPState::Instance().GetState() == TPState::TERMINATED) {
    errorstring = "Record: application terminated";
    return false;
  }
  if(!ok) {
    std::string f = filename;
    errorstring = "Record: error writing file \"" + f + "\"";
  }
  return ok;
}



////
// Wait
bool Wait::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {
  silence = (tolower(**cmds) == 's');
  activity = (tolower(**cmds) == 'a');
  
  if(silence  ||  activity) 
    (*cmds)++;
  
  closed = (tolower(**cmds) == 'c');
  
  if(closed) 
    (*cmds)++;
  
  size_t i = 0U;
  for(; (*cmds)[i]  &&  (*cmds)[i] != ';'  &&  isdigit((*cmds)[i]); i++)
    ;
  
  if(!i  ||  ((*cmds)[i]  &&  (*cmds)[i] != ';')) {
    errorstring = "Wait: No digits or invalid digits specified";
    return false;
  }

  sscanf(*cmds, "%u", &millis);
  *cmds = &((*cmds)[i]);
  sequence.push_back(this);
  return true;
}

bool Wait::RunCommand(const std::string &loopsuffix) {

  cout << "## Wait: waiting for " << millis << "ms ##" << endl;
  for(int n = millis / WAIT_SLEEP_ACCURACY; n >= 0; n--) {
    // silence detection
    if(silence
        &&  TPState::Instance().IsSilent(
	  WAIT_SILENCE_TIME_IN_MS * BYTES_PER_MILLIS)) {
      cout << "Wait: silence detected" << endl;
      break;
    }
    // activity detection
    else if(activity
        &&  TPState::Instance().IsActive(
          WAIT_ACTIVITY_TIME_IN_MS * BYTES_PER_MILLIS)) {
      cout << "Wait: activity detected" << endl;
      break;
    }
    // disconnect detection
    if(closed
        &&  (TPState::Instance().GetState() == TPState::TERMINATED
          ||  TPState::Instance().GetState() == TPState::CLOSED)) {
      cout << "Wait: connection closed" << endl;
      return true;
    }
    //cout << "Wait: usleep " << n << endl;
    usleep(WAIT_SLEEP_ACCURACY * 1000);
    if(!closed
        &&  TPState::Instance().GetState() == TPState::TERMINATED) {
      errorstring = "Wait: application terminated";
      return false;
    }
  }
  cout << "Wait: wait done" << endl;
  return true;
}


////
// Label
bool Label::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {
  size_t i = 0U;
  for(; (*cmds)[i]  &&  (*cmds)[i] != ';'; i++);
  if(!i) {
    errorstring = "Label: Empty label specified";
    return false;
  }
  label = PString(*cmds, i);
  *cmds = &((*cmds)[i]);
  sequence.push_back(this);
  return true;
}



////
// Loop
bool Loop::PopToInnerSequence(
    PString &label, std::vector< Command*> &sequence) {

  while(!sequence.empty()) {
    Command *cmd = *(sequence.rbegin());
    Label *alabel = dynamic_cast< Label *>(cmd);
    if(alabel  &&  label == alabel->GetLabel()) {
      return true;
    }
    sequence.pop_back();
    loopedsequence.insert(loopedsequence.begin(), cmd);
  }
  if(sequence.empty()  &&  label.IsEmpty()) {
    return true;
  }
  errorstring = "Loop: Nonexistant label specified";
  return false;
}

bool Loop::ParseCommand(
    const char **cmds, std::vector< Command*> &sequence) {

  size_t numdigits = strspn(*cmds, "0123456789");
  if(numdigits  &&  numdigits < 8)
    sscanf(*cmds, "%d", &loops);

  (*cmds) += numdigits;
  if(tolower(**cmds) == 'l') {
    (*cmds)++;
    size_t i = 0U;
    
    for(; (*cmds)[i]  &&  (*cmds)[i] != ';'; i++)
      ;

    if(!i) {
      errorstring = "Loop: Empty label specified";
    }
    else {
      PString l = PString(*cmds, i);
      if(PopToInnerSequence(l, sequence)) {
        (*cmds) += i;
        sequence.push_back(this);
        return true;
      }
    }
  }
  else if(**cmds == ';'  ||  !**cmds) {
    PString l = "";
    if(PopToInnerSequence(l, sequence)) {
      sequence.push_back(this);
      return true;
    }
  }
  else {
    errorstring = "Loop: invalid characters after iteration count";
  }
  return false;
}

bool Loop::RunCommand(const std::string &loopsuffix) {

  int timesleft = 0;
  do {
    char buf[256];
    time_t secsnow = time(NULL);
    stringstream newsuffix;
    newsuffix << loopsuffix << "_" << timesleft;
    cout << "Loop: iteration \"" << newsuffix.str()
      << "\" at " << ctime_r(&secsnow, buf) << endl;

    if(!Command::Run(loopedsequence, newsuffix.str()))
      return false;
  
  } while(++timesleft < loops);
  return true;
}


//**// END OF FILE //**//
// vim: tw=78 sw=2 sts=2
