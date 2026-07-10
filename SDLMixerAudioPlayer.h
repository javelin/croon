/*
 * File  : SDLMixerAudioPlayer.h
 * Author: Mark Documento
 */

#ifndef _Croon_SDLMixerAudioPlayer_h_
#define _Croon_SDLMixerAudioPlayer_h_

#include <Core/Core.h>

#include <atomic>

typedef struct Mix_Music Mix_Music;

class SDLMixerAudioPlayer {
public:
    enum AudioPlayerState {
        Playing = 0,
        Paused,
        Closed
    };

    static void InitPlayer();
    static void DeInitPlayer();
    static SDLMixerAudioPlayer& GetPlayer() { return player; }
    SDLMixerAudioPlayer();
    virtual ~SDLMixerAudioPlayer();
    bool Open(const Upp::String& filename);
    bool Pause();
    bool Play();
    bool Seek(double seconds);
    bool Close();
    bool IsPlaying();
    bool IsOpen();
    bool Reopen();
    double Duration();
    double Position();
    
private:
    void ReportError(const Upp::String& error);

    static SDLMixerAudioPlayer player;
    std::atomic<AudioPlayerState> state;
    Mix_Music* music;
    Upp::String path;
};

#endif
