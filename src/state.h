/*
 * sipcmd, state.h
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

#ifndef STATE_H
#define STATE_H

#include <string>
#include "main.h"
#include "channels.h"

// audio properties
#define BYTES_PER_MILLIS			16
// sleep accuracy for 'Wait' command
#define WAIT_SLEEP_ACCURACY			100
// silence detection parameters
#define MAX_SILENCE_DETECTION			(60000U * BYTES_PER_MILLIS)
#define MAX_ACTIVITY_DETECTION			(60000U * BYTES_PER_MILLIS)
#define WAIT_SILENCE_TIME_IN_MS			300U
#define WAIT_ACTIVITY_TIME_IN_MS		100U
#define RECORD_SILENCE_TIME_IN_MS		300U
// global dailing timeout in seconds
#define DIAL_TIMEOUT				10

class TPState {
  private:
    static TPState *instance;

  public:
    enum TPConnState {
      STARTING = 0,
      CONNECTING = 1,
      ESTABLISHED = 2,
      CLOSED = 3,
      TERMINATED = 666
    };

    enum TPProtocol {
      SIP,
      H323,
      RTP
    };

    static TPState &Instance() {
      if (!instance)
        instance = new TPState();
      return *instance;
    }

    void SetState( TPConnState newstate) {
      stateSync.Wait();
      if( state != TERMINATED)
        state = newstate;
      if( someonewaiting)
        stateEventSync.Signal( PTimeInterval( 100 ) ); // 0.1 seconds to allow calls to timeout with more accuracy
      stateSync.Signal();
    }

    TPConnState GetState( void) {
      TPConnState st;
      stateSync.Wait();
      st = state;
      stateSync.Signal();
      return st;
    }

    TPConnState WaitForStateChange( TPConnState breakonstate = TERMINATED) {
      // NOTE: not synchronized for multiple waiting threads
      // Waiting for a state change
      stateSync.Wait();
      if( state == TERMINATED  ||  state == breakonstate) {
        stateSync.Signal();
        return state;
      }
      someonewaiting = true;
      stateSync.Signal();

      stateEventSync.Wait( PTimeInterval( 100 ) );
      TPConnState newstate = state;
      someonewaiting = false;
      stateEventSync.Acknowledge();
      return newstate;
    }

    void SetProtocol( const TPProtocol p) { protocol = p; }
    void SetGateway( const PString &gw) { gateway = gw; }
    void SetLocalAddress( const PString &addr) { localaddress = addr; }
    void SetUserName( const PString &name) { username = name; }
    void SetAliasName( const PString &alias) { aliasname = alias; }
    void SetGateKeeper( const PString &gk) { gatekeeper = gk; }
    void SetListenPort( const int portnum) { listenport = portnum; }
    //void SetConnection( H323Connection *conn) { connection = conn; }
    void SetToken( const PString &calltoken) { token = calltoken; }
    void SetManager(Manager *m) { manager = m; }

    void SetSilenceState( bool is_silent, size_t buflen = 0U) {
      if( state == STARTING  ||  state == CONNECTING) {
        silence = 0; activity = 0; }
      else if( !is_silent) { silence = 0;
        if( activity < MAX_ACTIVITY_DETECTION) activity += buflen; }
      else { activity = 0;
        if( silence < MAX_SILENCE_DETECTION) silence += buflen; }
    }

    const TPProtocol GetProtocol( void) { return protocol; }
    const PString &GetGateway( void) { return gateway; }
    const PString &GetLocalAddress( void) { return localaddress; }
    const PString &GetUserName( void) { return username; }
    const PString &GetAliasName( void) { return aliasname; }
    const PString &GetGateKeeper( void) { return gatekeeper; }
    const int GetListenPort( void) { return listenport; }
    //H323Connection *GetConnection( void) { return connection; }
    const PString &GetToken( void) { return token; }
    Manager *GetManager( void) { return manager; }

    TestChanAudio &GetPlayBackAudio() { 
      return playbackaudio; 
    }
    
    TestChanAudio &GetRecordAudio() { 
      return recordaudio; 
    }

    bool IsSilent( size_t buflen) {
      if( buflen > MAX_SILENCE_DETECTION) buflen = MAX_SILENCE_DETECTION;
      return silence >= buflen; 
    }

    bool IsActive( size_t buflen) {
      if( buflen > MAX_ACTIVITY_DETECTION) buflen = MAX_ACTIVITY_DETECTION;
      return activity >= buflen; 
    }

  private:

    PSyncPointAck stateEventSync;
    PSemaphore stateSync;
    TPConnState state;
    volatile bool someonewaiting;

    volatile size_t activity;
    volatile size_t silence;
    PString gateway;
    PString localaddress;
    PString username;
    PString aliasname;
    PString gatekeeper;
    int listenport;
    //H323Connection *connection;
    PString token;
    Manager *manager;

    TestChanAudio playbackaudio;
    TestChanAudio recordaudio;
    TPProtocol protocol;

    TPState()
      : stateEventSync(), stateSync( 1, 1), state( STARTING),
      someonewaiting( false), activity( 0U), silence( 0U),
      gateway(), localaddress(), username(),
      aliasname(), gatekeeper(), listenport(5060), 
      token(), manager( NULL), playbackaudio(), recordaudio()
  { }
};


#endif // STATE_H
//**// END OF FILE //**//
// vim: tw=78 sw=2 sts=2
