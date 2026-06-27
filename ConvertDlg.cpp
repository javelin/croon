/*
 * File  : ConvertDlg.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

ConvertDlg::ConvertDlg() {
    ffmpeg = Config::Get(FFMPEG_LOCATION);
    renderTime = "";
    
    WhenProcessOutput << [=] (String part) {
        if (!gotDuration) {
            output += part;
            String dummy;
            double tPos = 0.0f;
            if (FfmpegProgressParser::ParseTimestamp(output, tPos, dummy, "Duration: ")) {
                duration = tPos;
                gotDuration = true;
            }
        }
        else if (duration > 0.0f) {
            double tPos = 0.0f;
            if (FfmpegProgressParser::ParseTimestamp(part, tPos, renderTime)) {
                progressVal = (int)(tPos/duration*100.0f);
            }
        }
    };
    
    WhenProcessEnded << [=] (int code) {
        if (code == 0) {
            if (!done) {
                done = true;
                progressVal = 0;
                renderTime = "";
            }
        }
        else {
            ErrorOK(Format("Child process result: %d. Unable to save video.", code));
        }
        return code != 0;
    };
    
    WhenProcessAborted << [=] (bool byUser) {
        if (byUser) {
            if (FileExists(outputPath)) FileDelete(outputPath);
        }
    };
    
    WhenStartNextProcess << [=] {
        if (done) {
            Break(IDOK);
            //std::cout << "Done conversion: " << outputPath.ToStd() << std::endl;
        }
        else {
            StartConversion();
        }
    };
    
    WhenUpdateProgress << [=] {
        progress.Set(progressVal, 100);
        monitor.SetLabel(Format("%s %s",
                                gotDuration ? "Converting audio...":"Getting duration...",
                                renderTime));
    };
}

int ConvertDlg::Run(String audioPath) {
    this->audioPath = audioPath;
    gotDuration = false;
    duration = 0.0f;
    done = false;
    progressVal = 0;
    PostCallback([=] { StartNextProcess(); });
    return RunDlg("Create Project");
}

void ConvertDlg::StartConversion() {
    outputPath = AppIdentity::TempFileName(".ogg");
    auto res = process.Start(ffmpeg, FfmpegCommandBuilder::ConvertAudioToVorbis(audioPath, outputPath));
    if (!res) {
        Exclamation("Unable to convert audio file!");
        Break(IDOK);
    }
    else {
        PollProgress();
    }
}
