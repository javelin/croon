/*
 * File  : SDLMixerAudioPlayer.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include <SDL.h>
#include <SDL_mixer.h>

#include "SDLMixerAudioPlayer.h"

void SDLMixerAudioPlayer::InitPlayer() {
}

void SDLMixerAudioPlayer::DeInitPlayer() {
    SDL_Quit();
}

SDLMixerAudioPlayer& SDLMixerAudioPlayer::GetPlayer() {
    static SDLMixerAudioPlayer player;
    return player;
}

SDLMixerAudioPlayer::SDLMixerAudioPlayer() : state(Closed), music(nullptr) {
}

SDLMixerAudioPlayer::~SDLMixerAudioPlayer() {
    if (music) {
        Mix_FreeMusic(music);
        Mix_CloseAudio();
    }
}

void SDLMixerAudioPlayer::ReportError(const String& error) {
    LOGF(error);
}

bool SDLMixerAudioPlayer::Open(const Upp::String& filename) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        ReportError(String("SDL_Init Error: ") + SDL_GetError());
        return false;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        ReportError(String("Mix_OpenAudio Error: ") + Mix_GetError());
        return false;
    }
    
    music = Mix_LoadMUS(filename);
    if (!music) {
        ReportError(String("Mix_LoadMUS Error: ") + Mix_GetError());
        Mix_CloseAudio();
        return false;
    }
    
    if (Mix_PlayMusic(music, 1) == -1) {
        ReportError(String("Mix_PlayMusic Error: ") + Mix_GetError());
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        music = nullptr;
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
        ReportError(String("Mix_SetMusicPosition Error: ") + Mix_GetError());
        Mix_CloseAudio();
        Mix_FreeMusic(music);
        music = nullptr;
        state = Closed;
    }
    return res == 0;
}

bool SDLMixerAudioPlayer::Close() {
    if (state == Closed) return false;
    int res = Mix_HaltMusic();
    if (res != 0) {
        ReportError(String("Mix_HaltMusic Error: ") + Mix_GetError());
    }
    state = Closed;
    Mix_CloseAudio();
    Mix_FreeMusic(music);
    music = nullptr;
    return res == 0;
}

bool SDLMixerAudioPlayer::IsPlaying() {
    if (!Mix_PlayingMusic()) Close();
    return state == Playing;
}

bool SDLMixerAudioPlayer::IsOpen() {
    if (!Mix_PlayingMusic()) Close();
    return state != Closed;
}

bool SDLMixerAudioPlayer::Reopen() {
    Close();
    return Open(path);
}
