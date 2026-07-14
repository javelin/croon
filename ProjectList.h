/*
 * File  : ProjectList.h
 * Author: Mark Documento
 */

#ifndef _Croon_ProjectList_h_
#define _Croon_ProjectList_h_

struct KarData;
class WizardDlg;

struct ProjectItem : public Moveable<ProjectItem>, Pte<ProjectItem> {
    ProjectItem(String path, time_t tstamp, String title, String artist, String lyrics, Image img) :
        path(path), tstamp(tstamp), title(title), artist(artist), lyrics(lyrics.Left(200)), img(img) {}
        
    bool operator <(const ProjectItem& pi) const {
        return tstamp < pi.tstamp ||
                title.Compare(pi.title) < 0 ||
                artist.Compare(pi.artist) < 0 ||
                path.Compare(pi.path) < 0; }
    
    String path;
    time_t tstamp;
    String title;
    String artist;
    String lyrics;
    Image img;
};

class ProjectItemCtrl : public Ctrl, public Pte<ProjectItem> {
public:
    ProjectItemCtrl(const ProjectItem& item);
    void Paint(Draw& w) override { w.DrawRect(0, 0, GetSize().cx, GetSize().cy, White()); }
    const ProjectItem& GetProjectItem() const { return item; }
    static Size GetImageSize() { return Size(UiScaler::X(ImageWidth), UiScaler::Y(UiScaler::InverseY(UiScaler::X(ImageWidth)))); }
    
private:
    static const int ImageWidth{48};
    const ProjectItem& item;
    ImageCtrl thumbnail;
    Label detailsLbl;
};

class ProjectList : public WithCroonProjectListLayout<ParentCtrl> {
public:
    ProjectList(KarData& data, WizardDlg& wizardDlg);
    virtual ~ProjectList();
    void OpenProject();
    void NewProject();
    void UpdateListView() { ProjectsToListCtrl(); }
    
    Gate<> WhenOpeningProject;
    Event<> WhenLoadingProject;
    Event<> WhenProjectLoaded;
    
private:
    ProjectItem* FindProject(String path, int& index);
    void ProjectsToListCtrl();
    void UpdateList();
        
private:
    Vector<ProjectItem> projects;
    KarData& data;
    WizardDlg& wizardDlg;
};

#endif
