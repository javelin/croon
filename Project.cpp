/*
 * File  : Project.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <atomic>
#include <filesystem>
#ifdef PLATFORM_POSIX
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "Constants.h"
#include "AppIdentity.h"
#include "AppPaths.h"
#include "ConfigService.h"
#include "Config.h"
#include "KarData.h"
#include "LyricsTransformer.h"
#include "SubtitleGenerator.h"
#include "TimeFormatter.h"
#include "UiScaler.h"
#include "GenreCatalog.h"
#include "Visualization.h"
#include "FfmpegProgressParser.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ListCtrl.h"
#include "LyricsPartsCtrl.h"
#include "ProjectLoader.h"
#include "Page.h"
#include "LyricsDownloadService.h"
#include "TextTools.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "SaveProjectDlg.h"
#include "ExportDlg.h"
#include "ConvertDlg.h"
#include "LyricsPartsDlg.h"
#include "TimingLine.h"
#include "TimingCtrl.h"

#define LAYOUTFILE <Croon/CroonTimingDlg.lay>
#include <CtrlCore/lay.h>

#include "AudioPlayerBase.h"
#include "AudioPlayer.h"
#include "SDLMixerAudioPlayer.h"
#include "AppAudioPlayer.h"
#include "TimingDlg.h"
#include "GatherDlg.h"
#include "VidThumbnail.h"
#include "Page3.h"

#define LAYOUTFILE <Croon/CroonVideoDlg.lay>
#include <CtrlCore/lay.h>

#include "VideoDlg.h"
#include "Project.h"

Project::Project(KarData& projectData, VideoDlg& videoDialog) : videoPath(""), data(projectData), videoDlg(videoDialog) {
    CtrlLayout(*this);
    
    for (const auto& genre : GenreCatalog::List()) genreEd.AddList(genre);
            
    exportOpts.Add(30, "30-secs, 180p")
                .Add(60, "1-min, 180p")
                .Add(0, "Complete, 180p")
                .AddSeparator()
                .Add(1, "Complete, Full-Sized")
                .SetData(30);
    
    saveBtn.Disable();
    
    auto updateData = [this] {
        SetDirty();
        KillTimeCallback(timerId);
        SetTimeCallback(1000, [this]{
            previewRT.SetQTF(Format("[2 %s]", SubtitleGenerator::ToRichAss(data)));
        }, timerId);
    };
    
    titleEd.WhenAction = [this, updateData] {
        data.title = ~titleEd.TrimBoth();
        exportBtn.Enable(!data.title.IsEmpty() &&
                            !data.artist.IsEmpty() &&
                            data.timed == data.timedLyrics.GetCount() - 1);
        updateData();
    };
    artistEd.WhenAction = [this, updateData] {
        data.artist = ~artistEd.TrimBoth();
        exportBtn.Enable(!data.title.IsEmpty() &&
                            !data.artist.IsEmpty() &&
                            data.timed == data.timedLyrics.GetCount() - 1);
        updateData();
    };
    genreEd.WhenAction = [this, updateData] {
        data.genre = ~genreEd.TrimBoth();
        updateData();
    };
    yearEd.WhenAction = [this, updateData] {
        Value val = yearEd.GetData();
        data.year = val.IsNull() ? 0:(int)val;
        updateData();
    };
    writerEd.WhenAction = [this, updateData] {
        data.writer = ~writerEd.TrimBoth();
        updateData();
    };
    ownerEd.WhenAction = [this, updateData] {
        data.owner = ~ownerEd.TrimBoth();
        updateData();
    };
    fontSizeEd.WhenAction = [this, updateData] {
        Value val = fontSizeEd.GetData();
        data.SetFontSize(val.IsNull() ? 0:(int)val);
        updateData();
    };
    dehissOpt.WhenAction = [this, updateData] {
        data.dehiss = dehissOpt.GetData();
        updateData();
    };
    
    videoBtn << [this] {
        if (videoDlg.Run() == IDOK) {
            videoImg.SetImage(data.videoThumbnail);
            videoImg.Tip(data.origVideoFile);
            SetDirty();
        }
    };
    
    timingBtn << [this] { Timing(); };
    
    partsBtn << [this] { VocalParts(); };
    
    exportBtn << [this] { ExportVideo(exportOpts.GetData()); };
    
    saveBtn << [this] { SaveProject(); };
    
    tab.Add(lyricsEd.HSizePosZ(5, 5).VSizePosZ(5, 5), "Lyrics");
    
    lyricsEd.WhenAction << [this] {
        KillTimeCallback(timerId);
        SetTimeCallback(1000, [this]{
            UpdateLyricsData();
        }, timerId);
    };
    
    tab.Add(previewRT.HSizePosZ(5, 5).VSizePosZ(5, 5), "ASS Preview");
}

void Project::Populate() {
    SetDirty(false);
    titleEd.SetData(data.title);
    artistEd.SetData(data.artist);
    genreEd.SetData(data.genre);
    if (data.year > 0) yearEd.SetData(data.year);
    else yearEd.Clear();
    ownerEd.SetData(data.owner);
    writerEd.SetData(data.writer);
    videoImg.SetImage(data.videoThumbnail);
    videoImg.Tip(data.origVideoFile);
    previewRT.SetQTF(Format("[2 %s]", SubtitleGenerator::ToRichAss(data)));
    
    lyricsEd.SetData(data.rawLyrics);
    exportBtn.Enable(!data.title.IsEmpty() &&
                        !data.artist.IsEmpty() &&
                        data.timed == data.timedLyrics.GetCount() - 1);
    fontSizeEd.SetData(data.fontSize);
    dehissOpt.SetData(data.dehiss);
    open = true;
    WhenDirty(dirty);
}

void Project::SaveProject() {
    if (open && dirty) {
        SaveProjectDlg().Run(data);
        SetDirty(false);
        WhenProjectSaved();
    }
}

void Project::SaveProjectAs() {
    if (open) {
        FileSel fsel;
        String projectDir{Config::Get(PROJECT_DIR, GetHomeDirectory())};
        if (projectDir.IsEmpty()) projectDir = Config::Get(MUSIC_DIR);
        fsel <<= ::GetFileDirectory(data.projectPath);
        fsel.Type(AppIdentity::ProjectTypeName(), AppIdentity::ProjectGlob());
        if (fsel.ExecuteSaveAs("Save Project As")) {
            String savePath{~fsel};
            if (!HasFileExt(savePath)) savePath += AppIdentity::ProjectExtension();
            else if (ToLower(GetFileExt(savePath)) != AppIdentity::ProjectExtension()) {
                savePath = GetFileTitle(savePath) + AppIdentity::ProjectExtension();
            }
            SaveProjectDlg saveDlg;
            if (saveDlg.Run(savePath, data) == IDOK) {
                Config::Set(PROJECT_DIR, ::GetFileDirectory(~fsel));
                WhenProjectSaved();
                SetDirty(false);
            }
        }
    }
}

void Project::UpdateLyricsData() {
    auto rawLyrics = TrimBoth((String)~lyricsEd);
    if (data.rawLyrics != rawLyrics) {
        dirty = true;
        data.rawLyrics = TrimBoth((String)~lyricsEd);
    }
    auto& vtl0 = data.timedLyrics;
    auto vtl1 = LyricsTransformer::RawToUntimed(data);
    if (vtl0.GetCount() != vtl1.GetCount()) {
        data.timed = 0;
        for (int i = 1, j = 1; i < vtl0.GetCount() && j < vtl1.GetCount(); ++i, ++j) {
            auto& tl0 = vtl0[i];
            auto& tl1 = vtl1[j];
            if (tl0.lyrics != tl1.lyrics) break;
            ++data.timed;
        }
        dirty = true;
    }
    vtl1[0].time = vtl0[0].time;
    for (int i = 1; i <= data.timed; ++i) {
        vtl1[i].time = vtl0[i].time;
    }
    data.timedLyrics = pick(vtl1);
    previewRT.SetQTF(Format("[2 %s]", SubtitleGenerator::ToRichAss(data)));
    saveBtn.Enable(dirty);
}

bool Project::CloseProject(bool quitting) {
    if (IsProjectDirty()) {
        String verb = quitting ? "quit":"close";
        auto res = PromptYesNoCancel(DeQtfLf(Format(
                            "Save project and %s?\nChoose 'No' to %s without saving.",
                            verb,
                            verb)));
        if (res == 1) {
            SaveProject();
        }
        else if (res == 0) {
            verb = quitting ? "Quit":"Close";
            if (PromptYesNo(Format("%s without saving project?", verb)) != 1) {
                return false;
            }
        }
        else return false;
    }
    CleanUp();
    open = false;
    SetDirty(false);
    return true;
}

void Project::CleanUp() {
    if (!open) return;
    FileDelete(data.audioFilePath);
    FileDelete(data.infoFilePath);
    if (GetFileDirectory(data.videoFilePath) == GetTempDirectory()) {
        FileDelete(data.videoFilePath);
    }
    if (GetFileDirectory(data.thumbnailFilePath) == GetTempDirectory()) {
        FileDelete(data.thumbnailFilePath);
    }
}

void Project::Timing() {
    KillTimeCallback(timerId);
    UpdateLyricsData();
    TimingDlg tDlg;
    tDlg.Run(data);
    SetDirty(tDlg.IsDirty());
    previewRT.SetQTF(Format("[2 %s]", SubtitleGenerator::ToRichAss(data)));
    lyricsEd.SetData(data.rawLyrics);
    exportBtn.Enable(!data.title.IsEmpty() &&
                        !data.artist.IsEmpty() &&
                        data.timed == data.timedLyrics.GetCount() - 1);
}

void Project::VocalParts() {
    KillTimeCallback(timerId);
    UpdateLyricsData();
    LyricsPartsDlg lpDlg;
    if (lpDlg.Run(data) == IDOK) {
        data.parts = lpDlg.GetParts();
        SetDirty();
    }
}

void Project::ReplaceAudio() {
    KillTimeCallback(timerId);
    UpdateLyricsData();
    
    FileSel fsel;
    fsel <<= Config::Get(MUSIC_DIR, GetHomeDirectory());
    fsel.Type("Music Files (*.m4a;*.mp3;*.mp4;*.ogg)", "*.m4a;*.mp3;*.mp4;*.ogg");
    fsel.Type("M4A | MPEG-4 Audio (*.m4a)", "*.m4a");
    fsel.Type("MP3 | MPEG-1 Audio Layer-3 (*.mp3)", "*.mp3");
    fsel.Type("MP4 | MPEG-4 Part 14 (*.mp4)", "*.mp4");
    fsel.Type("Ogg-Vorbis (*.ogg)", "*.ogg");
    if (fsel.ExecuteOpen("Open Music File")) {
        ConvertDlg conDlg;
        if (conDlg.Run(~fsel) == IDOK) {
            if (!AppAudioPlayer::GetPlayer().Open(conDlg.GetConvertedFile())) {
                Exclamation("Player is unable to load audio file.");
                return;
            }
            Config::Set(MUSIC_DIR, ::GetFileDirectory(~fsel));
            data.audioFilePath = conDlg.GetConvertedFile();
            data.origAudioFilePath = ~fsel;
            data.duration = conDlg.GetDurationn();
            PromptOK("Audio replaced successfully. You may want to review the timing before exporting HD video.");
        }
    }
}

void Project::ExportVideo(int length) {
    String ext = "mp4";
    if (length != 1) {
        ext = length == 0 ? "FULL":(IntStr(length) + "-SEC");
        ext += "_PREVIEW.mp4";
    }

#ifdef PLATFORM_POSIX
    std::filesystem::path savePath((const char*)data.projectPath);
    savePath.replace_extension(ext.ToStd());
    String outputPath = savePath.filename().c_str();
#else
    String outputPath = GetFileTitle(data.projectPath) + "." + ext;
#endif
    
    FileSel fsel;
    String saveDir = Config::Get(EXPORT_DIR);
    if (saveDir.IsEmpty()) saveDir = Config::Get(PROJECT_DIR);
    fsel <<= saveDir;
    fsel.Set(outputPath);
    fsel.ClearTypes();
    fsel.Type("MP4 | MPEG-4 Part 14 (*.mp4)", "*.mp4");
    
    bool b = fsel.ExecuteSaveAs();
    outputPath =  b ? ~fsel : String::GetVoid();
    if (outputPath.IsEmpty()) return;
    saveDir = GetFileDirectory(~fsel);
    Config::Set(EXPORT_DIR, saveDir);
    
    ExportDlg expDlg;
    expDlg.Run(data, outputPath, length);
}

Event<Bar&> Project::MainMenu() {
    Event<Bar&> event;
    event << [=](Bar& menu) {
        if (!IsProjectOpen()) return;
        menu.Sub("Project", [=](Bar& bar) {
            bar.Add("Timing...", [=] {
                Timing();
            }).Enable(IsProjectOpen());
            bar.Add("Vocal Parts...", [=] {
                VocalParts();
            });
            bar.Separator();
            bar.Add("Replace Audio...", [=] {
                ReplaceAudio();
            });
            bar.Separator();
            bar.Sub("Export", [=](Bar& sub) {
                sub.Add("30-sec Preview...", [=] {
                    ExportVideo(30);
                });
                sub.Add("60-sec Preview...", [=] {
                    ExportVideo(60);
                });
                sub.Add("Full Preview...", [=] {
                    ExportVideo(0);
                });
                sub.Separator();
                sub.Add("HD Video...", [=] {
                    ExportVideo();
                });
            });
        });
    };
    return event;
}
