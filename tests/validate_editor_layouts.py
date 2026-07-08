#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_editor_layouts: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    lay = (root / "Croon.lay").read_text()
    main_lay = (root / "CroonMainWindow.lay").read_text()
    for layout in [
        "LAYOUT(CroonProjectListLayout",
        "LAYOUT(CroonProjectLayout",
        "ITEM(ListCtrl, projectLst",
        "ITEM(ProjectLoader, loader",
        "ITEM(TabCtrl, tab",
        "ITEM(ImageCtrl, videoImg",
    ]:
        if layout not in lay:
            fail(f"missing {layout}")

    for layout in [
        "LAYOUT(CroonMainWindowLayout",
    ]:
        if layout not in main_lay:
            fail(f"missing {layout}")
    for layout_member in ["ITEM(ProjectList, projects", "ITEM(Project, project"]:
        if layout_member in main_lay:
            fail(f"CroonMainWindow.lay still default-constructs {layout_member}")

    expected_bases = {
        "ProjectList.h": "WithCroonProjectListLayout<ParentCtrl>",
        "Project.h": "WithCroonProjectLayout<ParentCtrl>",
        "MainWindow.h": "WithCroonMainWindowLayout<TopWindow>",
    }
    for rel, base in expected_bases.items():
        if base not in (root / rel).read_text():
            fail(f"{rel} does not inherit {base}")

    constructors = {
        "ProjectList.cpp": "ProjectList::ProjectList()",
        "Project.cpp": "Project::Project()",
        "MainWindow.cpp": "MainWindow::MainWindow()",
    }
    for rel, marker in constructors.items():
        text = (root / rel).read_text()
        if "CtrlLayout(*this)" not in text:
            fail(f"{rel} does not call CtrlLayout")
        constructor_body = text.split(marker, 1)[-1].split("\n}\n", 1)[0]
        if "*this <<" in constructor_body:
            fail(f"{rel} still hardcodes top-level child placement")

    mainwindow_h = (root / "MainWindow.h").read_text()
    for runtime_member in ["KarData& data;", "VideoDlg videoDlg;", "WizardDlg wizardDlg;", "Project project;", "ProjectList projects;", "StatusBar status;", "MenuBar menuBar;"]:
        if runtime_member not in mainwindow_h:
            fail(f"MainWindow.h missing runtime frame member {runtime_member}")

    mainwindow_impl = (root / "MainWindow.cpp").read_text()
    if '#include "Croon.h"' in mainwindow_impl:
        fail("MainWindow.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <atomic>",
        "#include <ctime>",
        "#include <filesystem>",
        "#include <SDL2/SDL.h>",
        "#include <SDL2/SDL_mixer.h>",
        '#include "Constants.h"',
        '#include "AppIdentity.h"',
        '#include "AppPaths.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "SubtitleGenerator.h"',
        '#include "TimeFormatter.h"',
        '#include "UiScaler.h"',
        '#include "GenreCatalog.h"',
        '#include "RichTextBuilder.h"',
        '#include "TextTools.h"',
        '#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "FfmpegProgressParser.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ListCtrl.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ProjectLoader.h"',
        '#include "Page.h"',
        '#include "LyricsDownloadService.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "ConvertDlg.h"',
        '#include "OpenProjectDlg.h"',
        '#include "SaveProjectDlg.h"',
        '#include "ExportDlg.h"',
        '#include "LyricsPartsDlg.h"',
        '#include "TimingLine.h"',
        '#include "TimingCtrl.h"',
        "#define LAYOUTFILE <Croon/CroonTimingDlg.lay>",
        '#include "AudioPlayerBase.h"',
        '#include "AudioPlayer.h"',
        '#include "SDLMixerAudioPlayer.h"',
        '#include "MusicPlayer.h"',
        '#include "TimingDlg.h"',
        '#include "GatherDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page1.h"',
        '#include "Page2.h"',
        '#include "Page3.h"',
        "#define LAYOUTFILE <Croon/CroonWizardShell.lay>",
        "#define LAYOUTFILE <Croon/CroonVideoDlg.lay>",
        '#include "VideoDlg.h"',
        '#include "WizardDlg.h"',
        '#include "Project.h"',
        '#include "ProjectList.h"',
        "#define LAYOUTFILE <Croon/CroonMainWindow.lay>",
        '#include "SettingsDlg.h"',
        '#include "MainWindow.h"',
    ]:
        if needle not in mainwindow_impl:
            fail(f"MainWindow.cpp missing direct dependency {needle}")
    for needle in [
        "MainWindow::MainWindow() : MainWindow(KarData::GetGlobal())",
        "MainWindow::MainWindow(KarData& data) : data(data), videoDlg(data), wizardDlg(data), project(data, videoDlg), projects(data, wizardDlg)",
        "Add(projects.HSizePos().VSizePos())",
        "Add(project.HSizePos().VSizePos())",
        "Title(AppIdentity::ProductName())",
        "Config::GetInt(WIN_X",
        "project.WhenProjectSaved",
        "projects.WhenOpeningProject",
        "projects.WhenLoadingProject",
        "projects.WhenProjectLoaded",
        "SettingsDlg sDlg",
        "project.MainMenu()(menu)",
        "PromptOK(AppIdentity::VersionText())",
    ]:
        if needle not in mainwindow_impl:
            fail(f"MainWindow.cpp missing main-window workflow {needle}")

    project_h = (root / "Project.h").read_text()
    for runtime_member in ["DocEdit lyricsEd;", "RichTextCtrl previewRT;"]:
        if runtime_member not in project_h:
            fail(f"Project.h missing runtime tab member {runtime_member}")

    project_impl = (root / "Project.cpp").read_text()
    if '#include "Croon.h"' in project_impl:
        fail("Project.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <atomic>",
        "#include <filesystem>",
        "#include <SDL2/SDL.h>",
        "#include <SDL2/SDL_mixer.h>",
        '#include "Constants.h"',
        '#include "AppIdentity.h"',
        '#include "AppPaths.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "SubtitleGenerator.h"',
        '#include "TimeFormatter.h"',
        '#include "UiScaler.h"',
        '#include "GenreCatalog.h"',
        '#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "FfmpegProgressParser.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ListCtrl.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ProjectLoader.h"',
        '#include "Page.h"',
        '#include "LyricsDownloadService.h"',
        '#include "TextTools.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "SaveProjectDlg.h"',
        '#include "ExportDlg.h"',
        '#include "ConvertDlg.h"',
        '#include "LyricsPartsDlg.h"',
        '#include "TimingLine.h"',
        '#include "TimingCtrl.h"',
        "#define LAYOUTFILE <Croon/CroonTimingDlg.lay>",
        '#include "AudioPlayerBase.h"',
        '#include "AudioPlayer.h"',
        '#include "SDLMixerAudioPlayer.h"',
        '#include "MusicPlayer.h"',
        '#include "TimingDlg.h"',
        '#include "GatherDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page3.h"',
        "#define LAYOUTFILE <Croon/CroonVideoDlg.lay>",
        '#include "VideoDlg.h"',
        '#include "Project.h"',
        "VideoDlg& GetVideoDlg();",
    ]:
        if needle not in project_impl:
            fail(f"Project.cpp missing direct dependency {needle}")
    for needle in [
        "GenreCatalog::List()",
        "Project::Project() : Project(KarData::GetGlobal())",
        "Project::Project(KarData& projectData) : Project(projectData, GetVideoDlg())",
        "Project::Project(KarData& projectData, VideoDlg& videoDialog) : videoPath(\"\"), data(projectData), videoDlg(videoDialog)",
        "SubtitleGenerator::ToRichAss(data)",
        "videoDlg.Run()",
        "SaveProjectDlg().Run(data)",
        "LyricsTransformer::RawToUntimed(data)",
        "TimingDlg tDlg",
        "LyricsPartsDlg lpDlg",
        "ConvertDlg conDlg",
        "MusicPlayer::GetPlayer().Open(conDlg.GetConvertedFile())",
        "std::filesystem::path savePath",
        "ExportDlg expDlg",
        "menu.Sub(\"Project\"",
    ]:
        if needle not in project_impl:
            fail(f"Project.cpp missing editor workflow {needle}")

    project_list_impl = (root / "ProjectList.cpp").read_text()
    if '#include "Croon.h"' in project_list_impl:
        fail("ProjectList.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <atomic>",
        "#include <ctime>",
        "#include <SDL2/SDL.h>",
        "#include <SDL2/SDL_mixer.h>",
        '#include "Constants.h"',
        '#include "AppIdentity.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "RichTextBuilder.h"',
        '#include "TextTools.h"',
        '#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ListCtrl.h"',
        '#include "ProjectLoader.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "GenreCatalog.h"',
        '#include "LyricsDownloadService.h"',
        '#include "AppPaths.h"',
        '#include "Page.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "ConvertDlg.h"',
        '#include "OpenProjectDlg.h"',
        '#include "AudioPlayerBase.h"',
        '#include "AudioPlayer.h"',
        '#include "SDLMixerAudioPlayer.h"',
        '#include "MusicPlayer.h"',
        '#include "GatherDlg.h"',
        '#include "SaveProjectDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page1.h"',
        '#include "Page2.h"',
        '#include "Page3.h"',
        "#define LAYOUTFILE <Croon/CroonWizardShell.lay>",
        '#include "WizardDlg.h"',
        '#include "ProjectList.h"',
        "WizardDlg& GetWizardDlg();",
    ]:
        if needle not in project_list_impl:
            fail(f"ProjectList.cpp missing direct dependency {needle}")
    for needle in [
        "RTHelper rth",
        "ProjectList::ProjectList() : ProjectList(KarData::GetGlobal())",
        "ProjectList::ProjectList(KarData& data) : ProjectList(data, GetWizardDlg())",
        "ProjectList::ProjectList(KarData& data, WizardDlg& wizardDlg) : data(data), wizardDlg(wizardDlg)",
        "loader.WhenProjectLoaded",
        "OpenProjectDlg opDlg",
        "MusicPlayer::GetPlayer().Open(data.audioFilePath)",
        "RecentProjectService::SavePaths(vs)",
        "ConvertDlg conDlg",
        "wizardDlg.Run(conDlg.GetConvertedFile()",
        "AppIdentity::ProjectTypeName()",
        "TextTools::ShortenMiddle(projects[index].path, 255)",
        "LyricsTransformer::TimedToRaw(data.timedLyrics, true)",
    ]:
        if needle not in project_list_impl:
            fail(f"ProjectList.cpp missing project-list workflow {needle}")

    croon_h = (root / "Croon.h").read_text()
    if croon_h.find('#include "ProjectLoader.h"') > croon_h.find("#include <CtrlCore/lay.h>"):
        fail("Croon.h includes layouts before ProjectLoader declaration")
    if croon_h.find('#include "TimingCtrl.h"') > croon_h.find("<Croon/CroonTimingDlg.lay>"):
        fail("Croon.h includes TimingDlg layout before TimingCtrl declaration")

    timing_lay = (root / "CroonTimingDlg.lay").read_text()
    for layout in [
        "LAYOUT(CroonTimingDlgLayout",
        "ITEM(TimingCtrl, timingCtrl",
        "ITEM(SliderCtrl, sliderCtrl",
    ]:
        if layout not in timing_lay:
            fail(f"missing {layout}")

    timing_header = (root / "TimingDlg.h").read_text()
    if "WithCroonTimingDlgLayout<TopWindow>" not in timing_header:
        fail("TimingDlg does not inherit WithCroonTimingDlgLayout")

    timing_impl = (root / "TimingDlg.cpp").read_text()
    if '#include "Croon.h"' in timing_impl:
        fail("TimingDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <atomic>",
        "#include <SDL2/SDL.h>",
        "#include <SDL2/SDL_mixer.h>",
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "TimeFormatter.h"',
        '#include "UiScaler.h"',
        '#include "TimingLine.h"',
        '#include "TimingCtrl.h"',
        "#define LAYOUTFILE <Croon/CroonTimingDlg.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "AudioPlayerBase.h"',
        '#include "AudioPlayer.h"',
        '#include "SDLMixerAudioPlayer.h"',
        '#include "MusicPlayer.h"',
        '#include "TimingDlg.h"',
    ]:
        if needle not in timing_impl:
            fail(f"TimingDlg.cpp missing direct dependency {needle}")
    if "CtrlLayout(*this" not in timing_impl:
        fail("TimingDlg constructor does not call CtrlLayout")
    constructor_body = timing_impl.split("TimingDlg::TimingDlg()", 1)[-1].split("\n}\n", 1)[0]
    if "*this <<" in constructor_body:
        fail("TimingDlg still hardcodes top-level child placement")
    for layout_member in ["Button playBtn;", "TimingCtrl timingCtrl;", "SliderCtrl sliderCtrl;"]:
        if layout_member in timing_header:
            fail(f"TimingDlg.h still declares layout member {layout_member}")
    for needle in [
        "MusicPlayer::GetPlayer()",
        "LyricsTransformer::TimedToRaw(data->timedLyrics)",
        "TimeFormatter::Format(data->duration)",
        "timingCtrl.SetTimedLyrics(data->timedLyrics, data->duration)",
        "timingCtrl.SetMusicPosition(position, duration)",
        "player.Seek(value/100.0f*player.Duration())",
    ]:
        if needle not in timing_impl:
            fail(f"TimingDlg.cpp missing timing workflow {needle}")

    if croon_h.find('#include "Page3.h"') > croon_h.find("<Croon/CroonVideoDlg.lay>"):
        fail("Croon.h includes VideoDlg layout before Page3 declaration")

    video_lay = (root / "CroonVideoDlg.lay").read_text()
    for layout in [
        "LAYOUT(CroonVideoDlgLayout",
        "ITEM(Button, okBtn",
        "ITEM(Button, cancelBtn",
    ]:
        if layout not in video_lay:
            fail(f"missing {layout}")
    if "ITEM(Page3, page3" in video_lay:
        fail("CroonVideoDlg.lay still default-constructs Page3")

    video_header = (root / "VideoDlg.h").read_text()
    if "WithCroonVideoDlgLayout<TopWindow>" not in video_header:
        fail("VideoDlg does not inherit WithCroonVideoDlgLayout")
    for runtime_member in ["GatherDlg gatherDlg;", "Page3 page3;"]:
        if runtime_member not in video_header:
            fail(f"VideoDlg.h missing runtime member {runtime_member}")
    for layout_member in ["Button okBtn;", "Button cancelBtn;"]:
        if layout_member in video_header:
            fail(f"VideoDlg.h still declares layout member {layout_member}")

    video_impl = (root / "VideoDlg.cpp").read_text()
    if '#include "Croon.h"' in video_impl:
        fail("VideoDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "AppPaths.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        '#include "Page.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "GatherDlg.h"',
        '#include "SaveProjectDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page3.h"',
        "#define LAYOUTFILE <Croon/CroonVideoDlg.lay>",
        '#include "VideoDlg.h"',
    ]:
        if needle not in video_impl:
            fail(f"VideoDlg.cpp missing direct dependency {needle}")
    if "CtrlLayout(*this" not in video_impl:
        fail("VideoDlg constructor does not call CtrlLayout")
    constructor_body = video_impl.split("VideoDlg::VideoDlg(KarData& data)", 1)[-1].split("\n}\n", 1)[0]
    for line in constructor_body.splitlines():
        if "*this <<" in line and "GatherButton" not in line:
            fail("VideoDlg still hardcodes non-gather child placement")
    for needle in [
        "VideoDlg::VideoDlg() : VideoDlg(KarData::GetGlobal())",
        "VideoDlg::VideoDlg(KarData& data) : gatherDlg(), page3(data, gatherDlg)",
        "Add(page3.HSizePosZ().VSizePosZ(0, 40))",
        'page3.GatherButton(true, true, "Find Videos")',
        "page3.WhenSelected",
        "SetData(path)",
        "tnPath = tnpath",
        "okBtn.Enable()",
    ]:
        if needle not in video_impl:
            fail(f"VideoDlg.cpp missing selection workflow {needle}")


if __name__ == "__main__":
    main()
