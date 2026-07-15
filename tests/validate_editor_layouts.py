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
        "ProjectList.cpp": "ProjectList::ProjectList(KarData& data, WizardDlg& wizardDlg)",
        "Project.cpp": "Project::Project(KarData& projectData, VideoDlg& videoDialog)",
        "MainWindow.cpp": "MainWindow::MainWindow(KarData& data)",
    }
    for rel, marker in constructors.items():
        text = (root / rel).read_text()
        if "CtrlLayout(*this)" not in text:
            fail(f"{rel} does not call CtrlLayout")
        constructor_body = text.split(marker, 1)[-1].split("\n}\n", 1)[0]
        if "*this <<" in constructor_body:
            fail(f"{rel} still hardcodes top-level child placement")

    mainwindow_h = (root / "MainWindow.h").read_text()
    for runtime_member in ["VideoDlg videoDlg;", "WizardDlg wizardDlg;", "Project project;", "ProjectList projects;", "StatusBar status;", "MenuBar menuBar;"]:
        if runtime_member not in mainwindow_h:
            fail(f"MainWindow.h missing runtime frame member {runtime_member}")
    if "KarData& data;" in mainwindow_h:
        fail("MainWindow.h still stores unused KarData reference")

    mainwindow_impl = (root / "MainWindow.cpp").read_text()
    if '#include "Croon.h"' in mainwindow_impl:
        fail("MainWindow.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <ctime>",
        "#include <filesystem>",
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
        '#include "Visualization.h"',
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
        '#include "AppAudioPlayer.h"',
        '#include "TimingDlg.h"',
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
    for needle in ["#include <atomic>", "#include <SDL2/SDL.h>", "#include <SDL2/SDL_mixer.h>", "#include <SDL.h>", "#include <SDL_mixer.h>"]:
        if needle in mainwindow_impl:
            fail(f"MainWindow.cpp has stale direct SDL/audio dependency {needle}")
    for needle in [
        "MainWindow::MainWindow(KarData& data) : videoDlg(data), wizardDlg(data), project(data, videoDlg), projects(data, wizardDlg)",
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
        "#include <filesystem>",
        '#include "Constants.h"',
        '#include "AppIdentity.h"',
        '#include "AppPaths.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "LrcGenerator.h"',
        '#include "SubtitleGenerator.h"',
        '#include "TimeFormatter.h"',
        '#include "UiScaler.h"',
        '#include "GenreCatalog.h"',
        '#include "Visualization.h"',
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
        '#include "AppAudioPlayer.h"',
        '#include "TimingDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page3.h"',
        "#define LAYOUTFILE <Croon/CroonVideoDlg.lay>",
        '#include "VideoDlg.h"',
        '#include "Project.h"',
    ]:
        if needle not in project_impl:
            fail(f"Project.cpp missing direct dependency {needle}")
    for needle in ["#include <atomic>", "#include <SDL2/SDL.h>", "#include <SDL2/SDL_mixer.h>", "#include <SDL.h>", "#include <SDL_mixer.h>"]:
        if needle in project_impl:
            fail(f"Project.cpp has stale direct SDL/audio dependency {needle}")
    for needle in [
        "GenreCatalog::List()",
        "Project::Project(KarData& projectData, VideoDlg& videoDialog) : videoPath(\"\"), data(projectData), videoDlg(videoDialog)",
        "SubtitleGenerator::ToRichAss(data)",
        "subtitleLinesDrop.Add(3",
        "data.SetSubtitleLines",
        "subtitleLinesDrop.SetData(data.subtitleLines)",
        "videoDlg.Run()",
        "SaveProjectDlg().Run(data) == IDOK",
        "LyricsTransformer::RawToUntimed(data)",
        "TimingDlg tDlg",
        "SetDirty(dirty || tDlg.IsDirty())",
        "LyricsPartsDlg lpDlg",
        "ConvertDlg conDlg",
        "AppAudioPlayer::Open(conDlg.GetConvertedFile())",
        "SetDirty();\n            PromptOK(\"Audio replaced successfully.",
        "std::filesystem::path savePath",
        "ExportDlg expDlg",
        "void Project::ExportLrc()",
        "Config::Get(LRC_EXPORT_DIR)",
        "Config::Set(LRC_EXPORT_DIR",
        "SaveFile(outputPath, LrcGenerator::ToLrc(data))",
        "sub.Add(\"LRC File...\"",
        "menu.Sub(\"Project\"",
    ]:
        if needle not in project_impl:
            fail(f"Project.cpp missing editor workflow {needle}")

    project_list_impl = (root / "ProjectList.cpp").read_text()
    if '#include "Croon.h"' in project_list_impl:
        fail("ProjectList.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <ctime>",
        '#include "Constants.h"',
        '#include "AppIdentity.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "RichTextBuilder.h"',
        '#include "TextTools.h"',
        '#include "Visualization.h"',
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
        '#include "AppAudioPlayer.h"',
        '#include "SaveProjectDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page1.h"',
        '#include "Page2.h"',
        '#include "Page3.h"',
        "#define LAYOUTFILE <Croon/CroonWizardShell.lay>",
        '#include "WizardDlg.h"',
        '#include "ProjectList.h"',
    ]:
        if needle not in project_list_impl:
            fail(f"ProjectList.cpp missing direct dependency {needle}")
    for needle in ["#include <atomic>", "#include <SDL2/SDL.h>", "#include <SDL2/SDL_mixer.h>", "#include <SDL.h>", "#include <SDL_mixer.h>"]:
        if needle in project_list_impl:
            fail(f"ProjectList.cpp has stale direct SDL/audio dependency {needle}")
    for needle in [
        "RTHelper rth",
        "ProjectList::ProjectList(KarData& data, WizardDlg& wizardDlg) : data(data), wizardDlg(wizardDlg)",
        "InstallListHandlers();",
        "loader.WhenProjectLoaded",
        "AddLoadedProject(path, title, artist, lyrics, thumbnail)",
        "projectLst.AddChild(*(new ProjectItemCtrl(projects.back())))",
        "OpenProjectDlg opDlg",
        "AppAudioPlayer::Open(data.audioFilePath)",
        "RecentProjectService::SavePaths(vs)",
        "loader.ProjectPaths()",
        "ConvertDlg conDlg",
        "wizardDlg.Run(conDlg.GetConvertedFile()",
        "AppIdentity::ProjectTypeName()",
        "TextTools::ShortenMiddle(projects[index].path, 255)",
        "LyricsTransformer::TimedToRaw(data.timedLyrics, true)",
    ]:
        if needle not in project_list_impl:
            fail(f"ProjectList.cpp missing project-list workflow {needle}")

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
    if "data = &karData;\n        Populate();\n        return TopWindow::Run();" in timing_header:
        fail("TimingDlg.h still owns inline run setup")

    timing_impl = (root / "TimingDlg.cpp").read_text()
    if '#include "Croon.h"' in timing_impl:
        fail("TimingDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "TimeFormatter.h"',
        '#include "UiScaler.h"',
        '#include "TimingLine.h"',
        '#include "TimingCtrl.h"',
        "#define LAYOUTFILE <Croon/CroonTimingDlg.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "AppAudioPlayer.h"',
        '#include "TimingDlg.h"',
    ]:
        if needle not in timing_impl:
            fail(f"TimingDlg.cpp missing direct dependency {needle}")
    for needle in ["#include <atomic>", "#include <SDL2/SDL.h>", "#include <SDL2/SDL_mixer.h>", "#include <SDL.h>", "#include <SDL_mixer.h>"]:
        if needle in timing_impl:
            fail(f"TimingDlg.cpp has stale direct SDL/audio dependency {needle}")
    if "CtrlLayout(*this" not in timing_impl:
        fail("TimingDlg constructor does not call CtrlLayout")
    for needle in [
        "int TimingDlg::Run(KarData& karData)",
        "data = &karData",
        "Populate()",
        "return TopWindow::Run()",
    ]:
        if needle not in timing_impl:
            fail(f"TimingDlg.cpp missing run setup {needle}")
    constructor_body = timing_impl.split("TimingDlg::TimingDlg()", 1)[-1].split("\n}\n", 1)[0]
    if "*this <<" in constructor_body:
        fail("TimingDlg still hardcodes top-level child placement")
    for layout_member in ["Button playBtn;", "TimingCtrl timingCtrl;", "SliderCtrl sliderCtrl;"]:
        if layout_member in timing_header:
            fail(f"TimingDlg.h still declares layout member {layout_member}")
    for needle in [
        "AppAudioPlayer::IsPlaying()",
        "AppAudioPlayer::Seek(position)",
        "AppAudioPlayer::Duration()",
        "AppAudioPlayer::Position()",
        "LyricsTransformer::TimedToRaw(data->timedLyrics)",
        "TimeFormatter::Format(data->duration)",
        "timingCtrl.SetTimedLyrics(data->timedLyrics, data->duration)",
        "timingCtrl.SetMusicPosition(position, duration)",
        "AppAudioPlayer::Seek(value/100.0f*AppAudioPlayer::Duration())",
    ]:
        if needle not in timing_impl:
            fail(f"TimingDlg.cpp missing timing workflow {needle}")
    if "AppAudioPlayer::GetPlayer()" in timing_impl:
        fail("TimingDlg.cpp reaches through AppAudioPlayer to backend singleton")

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
    for inline_body in [
        "page3.Reset()",
        "SetTimeCallback(500",
        "page3.SetPath(path)",
        "okBtn.Enable(found)",
        "return Value(value)",
    ]:
        if inline_body in video_header:
            fail(f"VideoDlg.h still owns inline method body {inline_body}")
    for runtime_member in ["Page3 page3;"]:
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
        '#include "KarData.h"\n#include "Visualization.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        '#include "Page.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
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
        "VideoDlg::VideoDlg(KarData& data) : page3(data)",
        "Add(page3.HSizePosZ().VSizePosZ(0, 40))",
        'page3.GatherButton(true, true, "Find Videos")',
        "page3.WhenSelected",
        "SetData(path)",
        "tnPath = tnpath",
        "okBtn.Enable()",
        "int VideoDlg::Run()",
        "page3.Reset()",
        "SetTimeCallback(500, [=] { page3.Rehint(false); })",
        "int code = Execute()",
        "page3.StopGathering()",
        "return code",
        "void VideoDlg::SetData(const Value& data)",
        "Value VideoDlg::GetData() const",
        "void VideoDlg::SetPath(String path)",
        "bool found = page3.SetPath(path)",
        "Image VideoDlg::GetImage()",
        "String VideoDlg::GetThumbnailPath() const",
    ]:
        if needle not in video_impl:
            fail(f"VideoDlg.cpp missing selection workflow {needle}")


if __name__ == "__main__":
    main()
