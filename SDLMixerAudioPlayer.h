/*
 * File  : SDLMixerAudioPlayer.h
 * Author: Mark Documento
 */

#ifndef _Croon_SDLMixerAudioPlayer_h_
#define _Croon_SDLMixerAudioPlayer_h_

#include <atomic>

#ifdef PLATFORM_POSIX
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif

class SDLMixerAudioPlayer {
public:
    enum AudioPlayerState {
        Playing = 0,
        Paused,
        Closed
    };

    static void InitPlayer() {}
    static void DeInitPlayer() { SDL_Quit(); }
    static SDLMixerAudioPlayer& GetPlayer() { return player; }
    SDLMixerAudioPlayer() : state(Closed), music(nullptr) {}
    virtual ~SDLMixerAudioPlayer() { if (music) { Mix_FreeMusic(music); Mix_CloseAudio(); } }
    const String& LastError() const { return lastError; }
    bool Open(const String& filename);
    bool Pause();
    bool Play();
    bool Seek(double seconds);
    bool Close();
    bool IsPlaying() { if (!Mix_PlayingMusic()) Close(); return state == Playing; }
    bool IsOpen() { if (!Mix_PlayingMusic()) Close(); return state != Closed; }
    bool Reopen() { Close(); return Open(path); }
    double Duration();
    double Position();
    Event<String> WhenError;
    
private:
    void LastError(const String& error) { lastError = error; }

    static SDLMixerAudioPlayer player;
    std::atomic<AudioPlayerState> state;
    Mix_Music* music;
    String path;
    String lastError;
};

#endif
