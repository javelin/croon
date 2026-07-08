/*
 * File  : Page3.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "Constants.h"
#include "ConfigService.h"
#include "Config.h"
#include "UiScaler.h"
#include "LyricsPartsCtrl.h"
#include "ListCtrl.h"
#include "AppIdentity.h"
#include "AppPaths.h"
#include "KarData.h"
#include "Visualization.h"
#include "VideoCatalog.h"
#include "FfmpegCommandBuilder.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"
#include "Page.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "GatherDlg.h"
#include "SaveProjectDlg.h"
#include "VidThumbnail.h"
#include "Page3.h"

Page3::Page3(KarData& data, GatherDlg& gatherDlg, String gatherKey) :
        vidCount(0), gatherKey(gatherKey), data(data), gatherDlg(gatherDlg) {
    pageName = "Background Video";
    CtrlLayout(*this);
    nextBtn.SetLabel("Save");
    videoLst.SetOrientation(ListCtrl::VerticalGrid, UiScaler::X(200), UiScaler::Y(200));
    tab.Add(videoLst.HSizePosZ(5, 5).VSizePosZ(5, 5), "Videos");
    String videoDir = Config::Get(VIDEO_DIR, GetHomeDirectory());
    Vector<String> paths = VideoCatalog::FindVideoFiles(videoDir);
    
    for (int i = 0; i < paths.GetCount(); ++i) {
        String tnPath = AppendFileName(AppPaths::DataDirectory(), GetFileName(paths[i]));
        tnPath.Replace(".mp4", ".thumbnail.png");
        
        Image img;
        if (FileExists(tnPath) && (img = StreamRaster::LoadFileAny(tnPath))) {
            ++vidCount;
            AddVideoItem(&videoLst, paths[i], tnPath, img, &vizLst);
        }
    }
    videoLst.Highlight(0);
    
    vizLst.SetOrientation(ListCtrl::VerticalGrid, UiScaler::X(200), UiScaler::Y(200));
    tab.Add(vizLst.HSizePosZ(5, 5).VSizePosZ(5, 5), "Vizualizations");
    AddVideoItem(&vizLst, "@@freqs", "", Visualization::Thumbnail("@@freqs"), &videoLst);
    AddVideoItem(&vizLst, "@@spectrum", "", Visualization::Thumbnail("@@spectrum"), &videoLst);
    AddVideoItem(&vizLst, "@@vectorscopedot", "", Visualization::Thumbnail("@@vectorscopedot"), &videoLst);
    AddVideoItem(&vizLst, "@@vectorscopeline", "", Visualization::Thumbnail("@@vectorscopeline"), &videoLst);
    AddVideoItem(&vizLst, "@@waves", "", Visualization::Thumbnail("@@waves"), &videoLst);
    vizLst.Highlight(0);
    gatherBtn << [=] { GatherVideos(); };
    gatherBtn.Disable();
    gatherBtn.Hide();
    gatherDlg.WhenVideoAdded << [this](int i, String path, String tnPath, Image img) {
        if (i == 0) {
            videoLst.ClearChildren();
            vidCount = 0;
        }
        ++vidCount;
        AddVideoItem(&videoLst, path, tnPath, img, &vizLst);
    };
    
    nextBtn << [=] {
        this->data.timedLyrics = LyricsTransformer::RawToUntimed(this->data);
        this->data.infoFilePath = AppIdentity::TempFileName(".json");
        
        FileSel fsel;
        String projectDir{Config::Get(PROJECT_DIR, GetHomeDirectory())};
        if (projectDir.IsEmpty()) projectDir = Config::Get(MUSIC_DIR);
        fsel <<= AppendFileName(projectDir, Format("%s - %s%s", this->data.artist, this->data.title, AppIdentity::ProjectExtension()));
        fsel.Type(AppIdentity::ProjectTypeName(), AppIdentity::ProjectGlob());
        if (fsel.ExecuteSaveAs("Save Project")) {
            String savePath{~fsel};
            if (!HasFileExt(savePath)) savePath += AppIdentity::ProjectExtension();
            else if (ToLower(GetFileExt(savePath)) != AppIdentity::ProjectExtension()) {
                savePath = GetFileTitle(savePath) + AppIdentity::ProjectExtension();
            }
            SaveProjectDlg saveDlg;
            if (saveDlg.Run(savePath, this->data) == IDOK) {
                Config::Set(PROJECT_DIR, ::GetFileDirectory(~fsel));
                WhenProjectSaved();
            }
        }
    };
}

void Page3::Layout() {
    Page::Layout();
    if (!rehint) return;
    auto hw = (videoLst.GetSize().cx - 40)/4;
    videoLst.SetSizeHint(UiScaler::X(hw), UiScaler::Y(hw));
    vizLst.SetSizeHint(UiScaler::X(hw), UiScaler::Y(hw));
}

bool Page3::SetPath(String path) {
    auto isViz = path.StartsWith("@@");
    auto exists = false;
    for (int i = 0; i < videoLst.GetCount(); ++i) {
        auto* vt = dynamic_cast<VidThumbnail*>(videoLst.GetChildCtrl(i)->GetCtrl());
        bool select = !isViz && vt->GetPath() == path;
        if (vt) vt->SetSelected(select);
        exists = exists || select;
    }
    for (int i = 0; i < vizLst.GetCount(); ++i) {
        auto* vt = dynamic_cast<VidThumbnail*>(vizLst.GetChildCtrl(i)->GetCtrl());
        bool select = isViz && vt->GetPath() == path;
        if (vt) vt->SetSelected(select);
        exists = exists || select;
    }
    tab.Set(isViz ? 1:0);
    return exists;
}

void Page3::GatherVideos() {
    FileSel fsel;
    fsel.Type("MP4 Videos", "*.mp4");
    fsel <<= Config::Get(VIDEO_DIR, GetHomeDirectory());
    if(fsel.ExecuteSelectDir()) {
        gatherDlg.Run(~fsel);
        if (vidCount) Config::Set(VIDEO_DIR, ~fsel);
    }
}

void Page3::AddVideoItem(ListCtrl* list, String path, String tnPath, Image img, ListCtrl* other) {
    //auto& item = list->AddChild(*(new VidThumbnail(path, Rescale(img, Size(UiScaler::X(200), UiScaler::Y(200))))));
    auto& item = list->AddChild(*(new VidThumbnail(path, img)), false);
    auto* vt = dynamic_cast<VidThumbnail*>(item.GetCtrl());
    vt->WhenSelected << [=] (String path) {
        for (int i = 0; i < list->GetCount(); ++i) {
            auto* vc = dynamic_cast<VidThumbnail*>(list->GetChildCtrl(i)->GetCtrl());
            if (vc) vc->SetSelected(false);
            if (vt == vc) {
                list->ClearHighlights();
                list->Highlight(i);
            }
        }
        list->Refresh();
        
        for (int i = 0; i < other->GetCount(); ++i) {
            auto* vc = dynamic_cast<VidThumbnail*>(other->GetChildCtrl(i)->GetCtrl());
            if (vc) vc->SetSelected(false);
        }
        other->Refresh();
        nextBtn.Enable((enableNext = true));
        this->data.videoFilePath = path;
        this->data.origVideoFile = GetFileName(path);
        this->data.thumbnailFilePath = tnPath;
        this->data.videoThumbnail = img;
        WhenSelected(path, tnPath, img);
    };
}

void Page3::Populate() {
    enableNext = !data.videoFilePath.IsEmpty();
    gatherBtn.Show();
    gatherBtn.Enable();
}

void Page3::Reset() {
    videoLst.ClearHighlights();
    for (int i = 0; i < videoLst.GetCount(); ++i) {
        auto* vt = dynamic_cast<VidThumbnail*>(videoLst.GetChildCtrl(i)->GetCtrl());
        if (vt) vt->SetSelected(false);
    }
    videoLst.FocusItem(0);
    for (int i = 0; i < vizLst.GetCount(); ++i) {
        auto* vt = dynamic_cast<VidThumbnail*>(vizLst.GetChildCtrl(i)->GetCtrl());
        if (vt) vt->SetSelected(false);
    }
    vizLst.FocusItem(0);
}

void Page3::SaveData() {
    gatherBtn.Disable();
    gatherBtn.Hide();
}
