/*
 * sipcmd, channels.cpp
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

#include <assert.h>
#include <ptlib.h>
#include <ptclib/memfile.h>
#include <ptclib/pwavfile.h>
#include <ptclib/random.h>
#include <ptlib/file.h>
#include "channels.h"
#include "main.h"
#include "state.h"

bool TestChanAudio::PlaybackAudio(const bool raw_rtp) {

    std::cout << __func__ << std::endl;

    //start playback
    playback = true;
    
    if (!raw_rtp) {
      sync.Signal();
      playsync.Wait();
      cout << "TestChanAudio::PlaybackAudio: play back done "
        << playback << endl;

      // check if playback ok
      bool playbackfailed = !playback;
      playback = false;
      return !playbackfailed;
    }
    
    Manager *m = TPState::Instance().GetManager();
    // write directly to rtp stream.
    //size_t recordbytes = recordmillisec * BYTES_PER_MILLIS;
    
    PAdaptiveDelay delay;
    int i = 0;
   
    unsigned timestamp = PRandom::Number();

    while(true) {
      RTP_DataFrame *frame = new RTP_DataFrame();
      frame->SetPayloadSize(640);
      timestamp += m->CalculateTimestamp(640);
      frame->SetTimestamp(timestamp);
      //frame->SetTimestamp(m->CalculateTimestamp(1));
      if (!playfile->Read(frame->GetPayloadPtr(), frame->GetPayloadSize())) {
        delete frame;
        break;
      }
      if (!m->WriteFrame(*frame)) {
        std::cerr << "RTP write failed" << std::endl;
        break;
      }
      i++;
      delete frame;
      //delay.Delay(20);
    }
    cout << "TestChanAudio::PlaybackAudio: play back done "
         << playback << endl
         << "wrote " << i * 640 << " bytes " << endl;
    playback = false;
    return true;
}


void TestChanAudio::StopAudioPlayback(bool ioerror) {

    std::cout << __func__ << std::endl;

    if(playfile) {
        PFile *ftemp = playfile;
        playfile = NULL;
        ftemp->Close();
        delete ftemp;

        if(playback) {
            playback = !ioerror;
            playsync.Signal();
        }
    }
}

void TestChanAudio::StopAudioRecording(bool ioerror) {
    
    std::cout << __func__ << std::endl;
    if(recfile) {
        PFile *ftemp = recfile;
        recfile = NULL;
        ftemp->Close();
        delete ftemp;

        if(record) {
            record = !ioerror;
            recsync.Signal();
        }
    }
}

bool TestChanAudio::PlaybackAudioBuffer(PBYTEArray &buffer) {

    //std::cout << __func__ << std::endl;
    sync.Wait();
    if(TPState::Instance().GetState() != TPState::ESTABLISHED) {
        cout << __func__ << ": state "
            << TPState::Instance().GetState() << endl;
        sync.Signal();
        return true;
    }

    // open file
    assert(!playfile);
    playfile = new PMemoryFile(buffer);
    cout << __func__ << ": starting playback of "
        << playfile->GetLength() << " bytes" << endl;

    if(!playfile->SetPosition(0, PFile::Start)) {
        cerr << __func__ << ": unable to set "
            << "initial position to 0" << endl;
    }

    // start playback
    return PlaybackAudio(TPState::Instance().GetProtocol() == TPState::RTP);
}

bool TestChanAudio::PlaybackAudioFile(PString &filename) {
    std::cout << __func__ << std::endl;
    sync.Wait();
    if(TPState::Instance().GetState() != TPState::ESTABLISHED) {
        cout << __func__ << ": state "
            << TPState::Instance().GetState() << endl;
        sync.Signal();
        return true;
    }

    //open file
    assert(!playfile);
    PINDEX extind = filename.GetLength() - 4;

    // check if WAV file
    if(extind >= 1  &&  filename.Mid(extind).ToLower() == ".wav") {
        cout << __func__ << ": opening file \""
            << filename << "\" as WAV" << endl;

        playfile = new PWAVFile(
                filename, PFile::ReadOnly, PFile::MustExist);
    }
    // raw data it is then
    else {
        cout << __func__ << ": opening file \""
            << filename << "\" as raw" << endl;
        playfile = new PFile(filename, PFile::ReadOnly, PFile::MustExist);
    }

    // start playback
    return PlaybackAudio(TPState::Instance().GetProtocol() == TPState::RTP);
} 


void TestChanAudio::FillPlaybackBuffer(char *buf, size_t len) {
  //cout << "TestChanAudio::FillPlaybackBuffer: begin " << len << endl;
  AutoSync a(sync);
  size_t readcount = 0U;

  if (playfile) {
    bool ok = playfile->Read( buf, len);

    // memoryfile returns ioerror when file ends...
    ok |= playfile->GetPosition() == playfile->GetLength();

    if(ok)
      readcount = playfile->GetLastReadCount();
    else
      cerr << "TestChanAudio::FillPlaybackBuffer: I/O error" << endl;

    if (readcount < len) {
      StopAudioPlayback( !ok);
    }
  }
  if (readcount < len) {
    memset(&buf[readcount], 0, len - readcount);
  }

  //cout << "TestChanAudio::FillPlaybackBuffer: end " << readcount << endl;

    /*
    //cout << __func__ << " begin: " << len << std::endl;
    AutoSync a(sync);
    size_t readcount = 0U;

    if (playfile) {
        //bool ok = playfile->RawRead(buf, len);
        // memoryfile returns ioerror when file ends...
        //ok |= playfile->GetPosition() == playfile->GetLength();

        int i = 0;
        bool ok = false;
        for (; i < 5; i++) {
            ok = playfile->Read(buf, len);
            if (ok)
                break;

            sleep(1);
        }

        if (ok)
            readcount = playfile->GetLastReadCount();
        else
            std::cerr << __func__ << " I/O error" << std::endl;

        if (readcount < len)
            StopAudioPlayback(!ok);
    }
    if (readcount < len)
        memset(&buf[ readcount], 0, len - readcount);
        */
}


bool TestChanAudio::RecordAudioFile(PString &filename,
        bool append_file, bool stop_on_silence, int max_millisec) {

    //std::cout << __func__ << std::endl;
    sync.Wait();
    /*
    if(TPState::Instance().GetState() != TPState::ESTABLISHED) {
        cout << __func__ << ": state "
            << TPState::Instance().GetState() << endl;

        sync.Signal();
        return true;
    }
*/
    assert(!recfile);
    PINDEX extind = filename.GetLength() - 4;
    // check if WAV file
    if(extind >= 1  &&  filename.Mid(extind).ToLower() == ".wav") {
        cout << __func__ << ": opening file \""
            << filename << "\" as WAV" << endl;
        recfile = new PWAVFile(filename, PFile::ReadWrite,
                append_file? PFile::Create: PFile::Create | PFile::Truncate);
    }
    // raw data it is then
    else {
        cout << __func__ << ": opening file \""
            << filename << "\" as raw" << endl;
        recfile = new PFile(filename, PFile::ReadWrite,
                append_file? PFile::Create: PFile::Create | PFile::Truncate);
    }

    // set append
    if(append_file) {
        cerr << __func__ << ": appending to file" << endl;
        bool appok = recfile->SetPosition(0, PFile::End);
        if(!appok  ||  !recfile->IsEndOfFile()) {
            cerr << __func__ << ": setting file pointer"
                << " to end of file failed" << std::endl;

                sync.Signal();
            return false;
        }
    }

    // set other flags
    TPState::Instance().SetSilenceState(false, 0U);
    stop_recording_when_silent = stop_on_silence;
    recordmillisec = max_millisec;

    // start recording
    record = true;
    sync.Signal();
    recsync.Wait();
    cout << __func__ << ": recording done " << record << endl;

    // check if recorded ok
    bool recordfailed = !record;
    record = false;
    return !recordfailed;

}

void TestChanAudio::RecordFromBuffer(
    const char *buf, size_t len, bool currently_silent) {

  //cout << __func__ << ": begin " << len << endl;
  AutoSync a(sync);
  // silence detection
  TPState::Instance().SetSilenceState(currently_silent, len);
  size_t writecount = 0U;

  if(recfile) {
    // check if silent
    bool is_silent = TPState::Instance().IsSilent(
        RECORD_SILENCE_TIME_IN_MS * BYTES_PER_MILLIS);
    size_t recordbytes = recordmillisec * BYTES_PER_MILLIS;
    recordbytes = recordbytes > len? len: recordbytes;

    // stop on silence?
    if(!stop_recording_when_silent  &&  is_silent) {
      cout << __func__ << ": silence detected" << endl;
      StopAudioRecording();
    }
    else {
      bool ok = recfile->Write(buf, recordbytes);
      if(ok) {
          writecount = recfile->GetLastWriteCount();
          recordmillisec -= writecount / BYTES_PER_MILLIS;
      }
      else {
          cerr << __func__ << ": I/O error" << endl;
      }
      if(writecount < len)
          StopAudioRecording(!ok);
    }
  }
}

bool TestChannel::Close() {
    cout << __func__ << " [ " << this->connection 
	 << " - " << this <<  " ]" << endl;
    audiohandle.CloseChannel();
    return true;
}

bool TestChannel::IsOpen() const {
    return is_open;
}

// Read reads TO phone line
bool TestChannel::Read(void *buf, PINDEX len) {
    //std::cout << "TestChannel::Read" << std::endl;
    audiohandle.FillPlaybackBuffer(reinterpret_cast< char *>(buf), len);

    lastReadCount = len;
    readDelay.Delay(len / BYTES_PER_MILLIS);
    return true;
}

// Write writes FROM phone line
bool TestChannel::Write(const void *buf, PINDEX len) {
    std::cout << "TestChannel::Write" << std::endl;
    audiohandle.RecordFromBuffer(
            reinterpret_cast< const char *>(buf), len, false);
            //audiocodec.DetectSilence());
    lastWriteCount = len;
    writeDelay.Delay(len / BYTES_PER_MILLIS);
    return true;
}

LocalConnection::LocalConnection(
        OpalCall &call, LocalEndPoint &ep, void *userData, 
        unsigned opts, OpalConnection::StringOptions *stropts)
    : OpalLocalConnection(call, ep, userData, opts, stropts) {
        std::cout << __func__ << std::endl;
}

LocalConnection::~LocalConnection() {
    std::cout << __func__ << std::endl;
}

OpalMediaStream *LocalConnection::CreateMediaStream(
        const OpalMediaFormat & mediaFormat,
        unsigned sessionID,
        bool isSource) {

    std::cout << __func__ << std::endl;

    PIndirectChannel *chan = NULL;

    //create the appropriate channel
    chan = isSource ? 
         new TestChannel(*this, TPState::Instance().GetPlayBackAudio()) :
         new TestChannel(*this, TPState::Instance().GetRecordAudio());

    OpalMediaStream *s = new RawMediaStream(*this, mediaFormat, sessionID, 
            isSource, chan, false);

    return s;
}

bool RawMediaStream::ReadData(BYTE *data, PINDEX size, PINDEX &length) {
    //cout << __func__ << endl;
    length = 0;
    if (!isOpen) {
        cout << "channel not open!" << endl;
        return false;
    }

    if (IsSink()) {
        cout << "tried to read from a sink stream!" << endl;
        return false;
    }

    if (m_channel == NULL) {
        cout << "no channel!" << endl;
        return false;
    }

    if (!m_channel->Read(data, size)) {
        cout << "read failed!" << endl;
        return false;
    }

    length = m_channel->GetLastReadCount();
    CollectAverage(data, length);
    //cout << "read " << length << endl;
    return true;
    //return OpalRawMediaStream::ReadData(data, size, length);
}

bool RawMediaStream::WriteData(const BYTE *data, 
        PINDEX length, PINDEX &written)
{
//    cout << __func__ << endl;
    if (!isOpen) {
        cout << "channel not open!" << endl;
        return false;
    }

    if (IsSource()) {
        cout << "tried to write to a source stream!" << endl;
        return false;
    }
    
    if (m_channel == NULL) {
        cout << "no channel!" << endl;
        return false;
    }

    if (data != NULL && length != 0) {
        if (!m_channel->Write(data, length)) {
            cout << "data write failed!" << endl;
            return false;
            CollectAverage(data, written);
        }
    } else {
        PBYTEArray silence(defaultDataSize);
        if (!m_channel->Write(silence, defaultDataSize)) {
            cout << "silence write failed!" << endl;
            return false;
        }
        CollectAverage(silence, written);
    }
    written = m_channel->GetLastWriteCount();
    // cout << "wrote " << written << endl;
    return true;
    //return OpalRawMediaStream::WriteData(data, length, written);
}
