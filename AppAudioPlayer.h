/*
 * File  : AppAudioPlayer.h
 * Author: Mark Documento
 */

#ifndef _Croon_AppAudioPlayer_h_
#define _Croon_AppAudioPlayer_h_

#include "SDLMixerAudioPlayer.h"

struct AppAudioPlayer {
    static void InitPlayer() { SDLMixerAudioPlayer::InitPlayer(); }
    static void DeInitPlayer() { SDLMixerAudioPlayer::DeInitPlayer(); }
    static bool Open(const String& filename) { return Player().Open(filename); }
    static bool Reopen() { return Player().Reopen(); }
    static bool Pause() { return Player().Pause(); }
    static bool Play() { return Player().Play(); }
    static bool Seek(double seconds) { return Player().Seek(seconds); }
    static bool IsPlaying() { return Player().IsPlaying(); }
    static bool IsOpen() { return Player().IsOpen(); }
    static double Duration() { return Player().Duration(); }
    static double Position() { return Player().Position(); }

private:
    static SDLMixerAudioPlayer& Player() { return SDLMixerAudioPlayer::GetPlayer(); }
};

#endif
