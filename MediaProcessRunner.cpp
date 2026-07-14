/*
 * File  : MediaProcessRunner.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "MediaProcessRunner.h"

bool MediaProcessRunner::Start(const String& command) {
    return process.Start(command);
}

bool MediaProcessRunner::Start(const String& executable, const Vector<String>& args) {
    return process.Start(executable, args);
}

bool MediaProcessRunner::Start(const String& executable, const Vector<String>& args, const char *envptr, const char *dir) {
    return process.Start(executable, args, envptr, dir);
}

bool MediaProcessRunner::Read(String& output) {
    return process.Read(output);
}

bool MediaProcessRunner::IsRunning() {
    return process.IsRunning();
}

int MediaProcessRunner::GetExitCode() {
    return process.GetExitCode();
}

void MediaProcessRunner::Kill() {
    process.Kill();
}
