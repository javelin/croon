#ifndef _Croon_ProjectLoader_h_
#define _Croon_ProjectLoader_h_

class ProjectLoader : public ProgressIndicator {
public:
    ProjectLoader();
    void LoadProjects();
    Event<String /*path*/,
            String /*title*/,
            String /*artist*/,
            String /*lyrics*/,
            Image /*thumbnail*/> WhenProjectLoaded;
    Event<> WhenDoneLoading;
    
private:
    void StartNextProcess();
    void PollProgress();
    
public:
    static const int timerId{1};
    
private:
    String ffmpeg;
    Vector<String> projects;
    int curPath;
    MediaProcessRunner process;
    String projectPath;
    String infoFilePath;
    String thumbnailPath;
};

#endif
