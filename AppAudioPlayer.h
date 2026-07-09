/*
 * File  : AppAudioPlayer.h
 * Author: Mark Documento
 */

#ifndef _Croon_AppAudioPlayer_h_
#define _Croon_AppAudioPlayer_h_

#include "AudioPlayer.h"
#include "SDLMixerAudioPlayer.h"

typedef AudioPlayer<SDLMixerAudioPlayer> AppAudioPlayer;

#endif
