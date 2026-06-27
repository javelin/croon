/*
 * File  : SDLMixerAudioPlayer.h
 * Author: Mark Documento
 */

#ifndef _Croon_SDLMixerAudioPlayer_h_
#define _Croon_SDLMixerAudioPlayer_h_

class SDLMixerAudioPlayer : public AudioPlayerBase {
public:
    static void InitPlayer() {}
    static void DeInitPlayer() { SDL_Quit(); }
    static SDLMixerAudioPlayer& GetPlayer() { return player; }
    SDLMixerAudioPlayer() : state(Closed), music(nullptr) {}
    virtual ~SDLMixerAudioPlayer() { if (music) { Mix_FreeMusic(music); Mix_CloseAudio(); } }
    bool Open(const String& filename) override;
    bool Pause() override;
    bool Play() override;
    bool Seek(double seconds) override;
    bool Close() override;
    bool IsPlaying() override { if (!Mix_PlayingMusic()) Close(); return state == Playing; }
    bool IsOpen() override { if (!Mix_PlayingMusic()) Close(); return state != Closed; }
    bool Reopen() override { Close(); return Open(path); }
    double Duration() override;
    double Position() override;
    AudioPlayerState State() override { return state; }
    String GetPath() const override { return path; }
    
private:
    static SDLMixerAudioPlayer player;
    std::atomic<AudioPlayerState> state;
    Mix_Music* music;
    String path;
};

#endif
