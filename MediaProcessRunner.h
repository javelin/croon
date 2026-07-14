/*
 * File  : MediaProcessRunner.h
 * Author: Mark Documento
 */

#ifndef _Croon_MediaProcessRunner_h_
#define _Croon_MediaProcessRunner_h_

class MediaProcessRunner {
public:
    bool Start(const String& command);
    bool Start(const String& executable, const Vector<String>& args);
    bool Start(const String& executable, const Vector<String>& args, const char *envptr, const char *dir);
    bool Read(String& output);
    bool IsRunning();
    int GetExitCode();
    void Kill();

private:
    LocalProcess process;
};

#endif
