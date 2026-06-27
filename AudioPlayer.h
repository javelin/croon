/*
 * File  : AudioPlayer.h
 * Author: Mark Documento
 */

#ifndef _Croon_AudioPlayer_h_
#define _Croon_AudioPlayer_h_

template <typename _Player_>
class AudioPlayer : public IAudioPlayer {
public:
    static void InitPlayer() { _Player_::InitPlayer(); }
    static void DeInitPlayer() { _Player_::DeInitPlayer(); }
    static _Player_& GetPlayer() { return _Player_::GetPlayer(); }
    AudioPlayer() {
        player.WhenError = Proxy(WhenError);
        player.WhenProgress = Proxy(WhenProgress);
        player.WhenPlaybackDone = Proxy(WhenPlaybackDone);
    }
    const String& LastError() const override { return player.LastError(); }
    bool Open(const String& filename) override { return player.Open(filename); }
    bool Reopen() override { return player.Reopen(); }
    bool Pause() override { return player.Pause(); }
    bool Play() override { return player.Play(); }
    bool Seek(double seconds) override { return player.Seek(seconds); };
    bool Close() override { return player.Close(); };
    bool IsPlaying() override { return player.IsPlaying(); };
    bool IsOpen() override { return player.IsOpen(); };
    double Duration() override { return player.Duration(); }
    double Position() override { return player.Position(); }
    AudioPlayerState State() override { return player.State(); }
    String GetPath() const override { return player.GetPath(); }
	
private:
    _Player_ player;
};

#endif
