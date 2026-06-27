/*
 * File  : Page3.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

Page3::Page3(String gatherKey) : vidCount(0), gatherKey(gatherKey) {
    pageName = "Background Video";
    CtrlLayout(*this);
    nextBtn.SetLabel("Save");
    videoLst.SetOrientation(ListCtrl::VerticalGrid, _Zx(200), _Zy(200));
    tab.Add(videoLst.HSizePosZ(5, 5).VSizePosZ(5, 5), "Videos");
    String videoDir = Config::Get(VIDEO_DIR, GetHomeDirectory());
    Vector<String> paths = AppPaths::FindFiles(videoDir, "*.mp4");
    
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
    
    vizLst.SetOrientation(ListCtrl::VerticalGrid, _Zx(200), _Zy(200));
    tab.Add(vizLst.HSizePosZ(5, 5).VSizePosZ(5, 5), "Vizualizations");
    AddVideoItem(&vizLst, "@@freqs", "", VIZ::Thumbnail("@@freqs"), &videoLst);
    AddVideoItem(&vizLst, "@@spectrum", "", VIZ::Thumbnail("@@spectrum"), &videoLst);
    AddVideoItem(&vizLst, "@@vectorscopedot", "", VIZ::Thumbnail("@@vectorscopedot"), &videoLst);
    AddVideoItem(&vizLst, "@@vectorscopeline", "", VIZ::Thumbnail("@@vectorscopeline"), &videoLst);
    AddVideoItem(&vizLst, "@@waves", "", VIZ::Thumbnail("@@waves"), &videoLst);
    vizLst.Highlight(0);
    gatherBtn << [=] { GatherVideos(); };
    gatherBtn.Disable();
    gatherBtn.Hide();
    GetGatherDlg().WhenVideoAdded << [=](int i, String path, String tnPath, Image img) {
        if (i == 0) {
            videoLst.ClearChildren();
            vidCount = 0;
        }
        ++vidCount;
        AddVideoItem(&videoLst, path, tnPath, img, &vizLst);
    };
    
    nextBtn << [=] {
        auto& data = KarData::GetGlobal();
        data.timedLyrics = RawToUntimedLyrics(data);
        data.infoFilePath = AppIdentity::TempFileName(".json");
        
        FileSel fsel;
        String projectDir{Config::Get(PROJECT_DIR, GetHomeDirectory())};
        if (projectDir.IsEmpty()) projectDir = Config::Get(MUSIC_DIR);
        fsel <<= AppendFileName(projectDir, Format("%s - %s%s", data.artist, data.title, AppIdentity::ProjectExtension()));
        fsel.Type(AppIdentity::ProjectTypeName(), AppIdentity::ProjectGlob());
        if (fsel.ExecuteSaveAs("Save Project")) {
            String savePath{~fsel};
            if (!HasFileExt(savePath)) savePath += AppIdentity::ProjectExtension();
            else if (ToLower(GetFileExt(savePath)) != AppIdentity::ProjectExtension()) {
                savePath = GetFileTitle(savePath) + AppIdentity::ProjectExtension();
            }
            SaveProjectDlg saveDlg;
            if (saveDlg.Run(savePath, KarData::GetGlobal()) == IDOK) {
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
    videoLst.SetSizeHint(_Zx(hw), _Zy(hw));
    vizLst.SetSizeHint(_Zx(hw), _Zy(hw));
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
        GetGatherDlg().Run(~fsel);
        if (vidCount) Config::Set(VIDEO_DIR, ~fsel);
    }
}

void Page3::AddVideoItem(ListCtrl* list, String path, String tnPath, Image img, ListCtrl* other) {
    //auto& item = list->AddChild(*(new VidThumbnail(path, Rescale(img, Size(_Zx(200), _Zy(200))))));
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
        auto& data = KarData::GetGlobal();
        data.videoFilePath = path;
        data.origVideoFile = GetFileName(path);
        data.thumbnailFilePath = tnPath;
        data.videoThumbnail = img;
        WhenSelected(path, tnPath, img);
    };
}

void Page3::Populate() {
    auto& data = KarData::GetGlobal();
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
