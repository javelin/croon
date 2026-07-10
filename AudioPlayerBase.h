/*
 * File  : AudioPlayerBase.h
 * Author: Mark Documento
 */

#ifndef _Croon_AudioPlayerBase_h_
#define _Croon_AudioPlayerBase_h_

class AudioPlayerBase {
public:
	enum AudioPlayerState {
        Playing = 0,
        Paused,
        Closed
    };

    AudioPlayerBase() : lastError("") {}
    const String& LastError() const { return lastError; }

    Event<String> WhenError;
	
protected:
    void LastError(const String& error) { lastError = error; }
	
protected:
    String lastError;
};

#endif
