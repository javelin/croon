/*
 * File  : ProjectList.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <atomic>
#include <ctime>
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
#include "ConfigService.h"
#include "Config.h"
#include "UiScaler.h"
#include "KarData.h"
#include "LyricsTransformer.h"
#include "RichTextBuilder.h"
#include "TextTools.h"
#include "Visualization.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ListCtrl.h"
#include "ProjectLoader.h"
#include "LyricsPartsCtrl.h"
#include "GenreCatalog.h"
#include "LyricsDownloadService.h"
#include "AppPaths.h"
#include "Page.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "ConvertDlg.h"
#include "OpenProjectDlg.h"
#include "AudioPlayerBase.h"
#include "AudioPlayer.h"
#include "SDLMixerAudioPlayer.h"
#include "AppAudioPlayer.h"
#include "GatherDlg.h"
#include "SaveProjectDlg.h"
#include "VidThumbnail.h"
#include "Page1.h"
#include "Page2.h"
#include "Page3.h"

#define LAYOUTFILE <Croon/CroonWizardShell.lay>
#include <CtrlCore/lay.h>

#include "WizardDlg.h"
#include "ProjectList.h"

ProjectItemCtrl::ProjectItemCtrl(const ProjectItem& item) :
                    item(item) {
    auto sz = GetImageSize();
    *this << thumbnail.SetImage(item.img).LeftPos(0, sz.cx).VCenterPos(sz.cy)
            << detailsLbl.AlignTop().HSizePos(sz.cx, 0).VSizePosZ(1, 1);
    RTHelper rth;
    auto text = rth.Fmt("*+60")
                    .Text(item.title).NL()
                    .Fmt("/")
                    .Text(item.artist).NL()
                    .EFmt()
                    .EFmt("+48")
                    //.Fmt("*")
                    //.Text(ts).NL()
                    //.EFmt("/")
                    .Text(item.lyrics)
                    .EFmt()//.EFmt()
                    .ToString();
    detailsLbl.SetText("\1" + text);
}

ProjectList::ProjectList(KarData& data, WizardDlg& wizardDlg) : data(data), wizardDlg(wizardDlg) {
    CtrlLayout(*this);
    openBtn << [this] { OpenProject(); };
    newBtn << [this] { NewProject(); };
    clearBtn << [this] {
        if (PromptYesNoCancel(DeQtfLf("Are you sure you want to clear the projects list?\n"
                                        "The saved projects won't be deleted.")) == 1) {
            projects.Clear();
            projectLst.ClearChildren();
            clearBtn.Enable(!projects.IsEmpty());
        }
    };
    projectLst.SetOrientation(ListCtrl::VerticalGrid, 250, 70);
    loader.WhenProjectLoaded << [this] (
        String path, String title, String artist, String lyrics, Image thumbnail) {
        projects.Add(*(new ProjectItem(path,
                                        time(0),
                                        title,
                                        artist,
                                        lyrics,
                                        Rescale(thumbnail, ProjectItemCtrl::GetImageSize()))));
        projectLst.AddChild(*(new ProjectItemCtrl(projects.back())), false);
        projectLst.Scroll();
    };
    loader.WhenDoneLoading << [this] {
        loader.Hide();
        clearBtn.Enable(!projects.IsEmpty());
        projectLst.ConsumeMouseEvents().WhenLeftDouble = [this] (int index, Ctrl* ctrl) {
            if (index < 0 || index >= projects.GetCount()) return;
            const auto& item = projects[index];
            KarData tdata;
            OpenProjectDlg opDlg;
            if (opDlg.Run(item.path, tdata) == IDOK) {
                WhenLoadingProject();
                this->data = pick(tdata);
                UpdateList();
                if (!AppAudioPlayer::GetPlayer().Open(this->data.audioFilePath)) {
                    Exclamation("Player is unable to load audio file. You will not be able to set timing!");
                }
                WhenProjectLoaded();
            }
        };
        projectLst.WhenRightUp = [this] (int index, Ctrl* ctrl) {
            if (index < 0 || index >= projects.GetCount()) return;
            MenuBar::Execute([this, index](Bar& bar) {
                bar.Add(TextTools::ShortenMiddle(projects[index].path, 255), CroonImg::Icon16(), [this, index] {
                    const auto& item = projects[index];
                    auto msg = Format("{{1:9 Project:: %s:: Title:: %s:: Artist:: %s}}",
                                        DeQtf(item.path),
                                        DeQtf(item.title),
                                        DeQtf(item.artist));
                    if (Prompt(item.title.IsEmpty() ? "Untitled":item.title,
                            item.img, msg, "Open", "Cancel") == 1) {
                        KarData tdata;
                        OpenProjectDlg opDlg;
                        if (opDlg.Run(item.path, tdata) == IDOK) {
                            WhenLoadingProject();
                            this->data = pick(tdata);
                            UpdateList();
                            if (!AppAudioPlayer::GetPlayer().Open(this->data.audioFilePath)) {
                                Exclamation("Player is unable to load audio file. You will not be able to set timing!");
                            }
                            WhenProjectLoaded();
                        }
                    }
                });
                bar.Add("Open Project", CtrlImg::open(), [this, index] {
                    const auto& item = projects[index];
                    KarData tdata;
                    OpenProjectDlg opDlg;
                    if (opDlg.Run(item.path, tdata) == IDOK) {
                        WhenLoadingProject();
                        this->data = pick(tdata);
                        UpdateList();
                        if (!AppAudioPlayer::GetPlayer().Open(this->data.audioFilePath)) {
                            Exclamation("Player is unable to load audio file. You will not be able to set timing!");
                        }
                        WhenProjectLoaded();
                    }
                });
                bar.Separator();
                bar.Add("Delete Project", CroonImg::Trash(), [this, index] {
                    if (PromptYesNoCancel("Are you sure you want to [* permanently] delete this project?") == 1) {
                        const auto& item = projects[index];
                        if (!FileDelete(item.path)) {
                            ErrorOK("Unable to delete project.");
                        }
                        else {
                            projects.Remove(index);
                            PromptOK("Project deleted.");
                            UpdateListView();
                        }
                    }
                });
            });
        };
        projectLst.FocusItem(0);
    };
    loader.LoadProjects();
}

ProjectList::~ProjectList() {
    Vector<String> vs;
    for (const auto& project : projects) {
        vs.Add(project.path);
    }
    RecentProjectService::SavePaths(vs);
}

void ProjectList::NewProject() {
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
            Config::Set(MUSIC_DIR, ::GetFileDirectory(~fsel));
            if (wizardDlg.Run(conDlg.GetConvertedFile(), conDlg.GetDurationn(), ~fsel) == IDOK) {
                UpdateList();
                if (!AppAudioPlayer::GetPlayer().Open(data.audioFilePath)) {
                    Exclamation("Player is unable to load audio file. You will not be able to set timing!");
                }
                WhenProjectLoaded();
            }
        }
    }
}

void ProjectList::OpenProject() {
    if (!WhenOpeningProject()) return;
    FileSel fsel;
    String projectDir{Config::Get(PROJECT_DIR, GetHomeDirectory())};
    if (projectDir.IsEmpty()) projectDir = Config::Get(MUSIC_DIR);
    fsel <<= projectDir;
    fsel.Type(AppIdentity::ProjectTypeName(), AppIdentity::ProjectGlob());
    if (fsel.ExecuteOpen("Open Project")) {
        KarData tdata;
        OpenProjectDlg opDlg;
        if (opDlg.Run(~fsel, tdata) == IDOK) {
            WhenLoadingProject();
            data = pick(tdata);
            UpdateList();
            if (!AppAudioPlayer::GetPlayer().Open(data.audioFilePath)) {
                Exclamation("Player is unable to load audio file. You will not be able to set timing!");
            }
            Config::Set(PROJECT_DIR, GetFileDirectory(~fsel));
            WhenProjectLoaded();
        }
    }
}

void ProjectList::UpdateList() {
    int index;
    auto project = FindProject(data.projectPath, index);
    if (!project) {
        projects.Insert(0, *(new ProjectItem(data.projectPath,
                                                time(0),
                                                data.title,
                                                data.artist,
                                                LyricsTransformer::TimedToRaw(data.timedLyrics, true),
                                                Rescale(data.videoThumbnail, ProjectItemCtrl::GetImageSize()))));
    }
    else {
        ProjectItem pi = pick(*project);
        projects.Remove(index);
        projects.Insert(0, pi);
    }
    ProjectsToListCtrl();
    clearBtn.Enable(!projects.IsEmpty());
}

ProjectItem* ProjectList::FindProject(String path, int& index) {
    for (auto& project : projects) {
        if (project.path == path) {
            index = projects.GetIndex(project);
            return &project;
        }
    }
    index = -1;
    return nullptr;
}

void ProjectList::ProjectsToListCtrl() {
    projectLst.ClearChildren();
    for (const auto& project : projects) {
        projectLst.AddChild(*(new ProjectItemCtrl(project)));
    }
    projectLst.FocusItem(0);
}
