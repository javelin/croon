/*
 * File  : MediaProcessRunner.h
 * Author: Mark Documento
 */

#ifndef _Croon_MediaProcessRunner_h_
#define _Croon_MediaProcessRunner_h_

class MediaProcessRunner {
public:
    bool Start(const String& command) {
        return process.Start(command);
    }
    
    bool Start(const String& executable, const Vector<String>& args) {
        return process.Start(executable, args);
    }
    
    bool Start(const String& executable, const Vector<String>& args, const char *envptr, const char *dir) {
        return process.Start(executable, args, envptr, dir);
    }
    
    bool Read(String& output) {
        return process.Read(output);
    }
    
    bool IsRunning() {
        return process.IsRunning();
    }
    
    int GetExitCode() {
        return process.GetExitCode();
    }
    
    void Kill() {
        process.Kill();
    }

private:
    LocalProcess process;
};

#endif
