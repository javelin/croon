/*
 * File  : SDLMixerAudioPlayer.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "SDLMixerAudioPlayer.h"

SDLMixerAudioPlayer SDLMixerAudioPlayer::player;

bool SDLMixerAudioPlayer::Open(const String& filename) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        LastError(String("SDL_Init Error: ") + SDL_GetError());
        LOGF(LastError());
        WhenError(LastError());
        return false;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        LastError(String("Mix_OpenAudio Error: ") + Mix_GetError());
        LOGF(LastError());
        WhenError(LastError());
        return false;
    }
    
    music = Mix_LoadMUS(filename);
    if (!music) {
        LastError(String("Mix_LoadMUS Error: ") + Mix_GetError());
        LOGF(LastError());
        Mix_CloseAudio();
        WhenError(LastError());
        return false;
    }
    
    if (Mix_PlayMusic(music, 1) == -1) {
        LastError(String("Mix_PlayMusic Error: ") + Mix_GetError());
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        music = nullptr;
        WhenError(LastError());
        return false;
    }
    
    path = filename;
    Mix_PauseMusic();
    state = Paused;
    
    return true;
}

double SDLMixerAudioPlayer::Duration() {
    return Mix_MusicDuration(music);
}

double SDLMixerAudioPlayer::Position() {
    return Mix_GetMusicPosition(music);
}

bool SDLMixerAudioPlayer::Pause() {
    Mix_PauseMusic();
    state = Paused;
    return true;
}

bool SDLMixerAudioPlayer::Play() {
    Mix_ResumeMusic();
    state = Playing;
    return true;
}

bool SDLMixerAudioPlayer::Seek(double seconds) {
    int res = Mix_SetMusicPosition(seconds);
    if (res != 0) {
        LastError(String("Mix_SetMusicPosition Error: ") + Mix_GetError());
        LOGF(LastError());
        Mix_CloseAudio();
        Mix_FreeMusic(music);
        music = nullptr;
        WhenError(LastError());
        state = Closed;
    }
    return res == 0;
}

bool SDLMixerAudioPlayer::Close() {
    if (state == Closed) return false;
    int res = Mix_HaltMusic();
    if (res != 0) {
        LastError(String("Mix_HaltMusic Error: ") + Mix_GetError());
        LOGF(LastError());
        WhenError(LastError());
    }
    state = Closed;
    Mix_CloseAudio();
    Mix_FreeMusic(music);
    music = nullptr;
    return res == 0;
}
