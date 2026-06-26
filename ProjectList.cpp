/*
 * File  : ProjectList.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

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

ProjectList::ProjectList() {
    CtrlLayout(*this);
    openBtn << [=] { OpenProject(); };
    newBtn << [=] { NewProject(); };
    clearBtn << [=] {
        if (PromptYesNoCancel(DeQtfLf("Are you sure you want to clear the projects list?\n"
                                        "The saved projects won't be deleted.")) == 1) {
            projects.Clear();
            projectLst.ClearChildren();
            clearBtn.Enable(!projects.IsEmpty());
        }
    };
    projectLst.SetOrientation(ListCtrl::VerticalGrid, 250, 70);
    loader.WhenProjectLoaded << [=] (
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
    loader.WhenDoneLoading << [=] {
        loader.Hide();
        clearBtn.Enable(!projects.IsEmpty());
        projectLst.ConsumeMouseEvents().WhenLeftDouble = [=] (int index, Ctrl* ctrl) {
            if (index < 0 || index >= projects.GetCount()) return;
            const auto& item = projects[index];
            KarData tdata;
            OpenProjectDlg opDlg;
            if (opDlg.Run(item.path, tdata) == IDOK) {
                WhenLoadingProject();
                KarData& data = KarData::GetGlobal();
                data = pick(tdata);
                UpdateList();
                if (!MusicPlayer::GetPlayer().Open(data.audioFilePath)) {
                    Exclamation("Player is unable to load audio file. You will not be able to set timing!");
                }
                WhenProjectLoaded();
            }
        };
        projectLst.WhenRightUp = [=] (int index, Ctrl* ctrl) {
            if (index < 0 || index >= projects.GetCount()) return;
            MenuBar::Execute([=](Bar& bar) {
                bar.Add(ShortenMiddle(projects[index].path, 255), CroonImg::Icon16(), [=] {
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
                            KarData& data = KarData::GetGlobal();
                            data = pick(tdata);
                            UpdateList();
                            if (!MusicPlayer::GetPlayer().Open(data.audioFilePath)) {
                                Exclamation("Player is unable to load audio file. You will not be able to set timing!");
                            }
                            WhenProjectLoaded();
                        }
                    }
                });
                bar.Add("Open Project", CtrlImg::open(), [=] {
                    const auto& item = projects[index];
                    KarData tdata;
                    OpenProjectDlg opDlg;
                    if (opDlg.Run(item.path, tdata) == IDOK) {
                        WhenLoadingProject();
                        KarData& data = KarData::GetGlobal();
                        data = pick(tdata);
                        UpdateList();
                        if (!MusicPlayer::GetPlayer().Open(data.audioFilePath)) {
                            Exclamation("Player is unable to load audio file. You will not be able to set timing!");
                        }
                        WhenProjectLoaded();
                    }
                });
                bar.Separator();
                bar.Add("Delete Project", CroonImg::Trash(), [=] {
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
            auto& wizDlg = GetWizardDlg();
            if (wizDlg.Run(conDlg.GetConvertedFile(), conDlg.GetDurationn(), ~fsel) == IDOK) {
                UpdateList();
                if (!MusicPlayer::GetPlayer().Open(KarData::GetGlobal().audioFilePath)) {
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
            KarData& data = KarData::GetGlobal();
            data = pick(tdata);
            UpdateList();
            if (!MusicPlayer::GetPlayer().Open(data.audioFilePath)) {
                Exclamation("Player is unable to load audio file. You will not be able to set timing!");
            }
            Config::Set(PROJECT_DIR, GetFileDirectory(~fsel));
            WhenProjectLoaded();
        }
    }
}

void ProjectList::UpdateList() {
    KarData& data = KarData::GetGlobal();
    int index;
    auto project = FindProject(data.projectPath, index);
    if (!project) {
        projects.Insert(0, *(new ProjectItem(data.projectPath,
                                                time(0),
                                                data.title,
                                                data.artist,
                                                TimedLyricsToRaw(data.timedLyrics, true),
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
