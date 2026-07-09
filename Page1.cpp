/*
 * File  : Page1.cpp
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
#include "KarData.h"
#include "Visualization.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"
#include "GenreCatalog.h"
#include "Page.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "Page1.h"

Page1::Page1(KarData& data) : data(data) {
    pageName = "Song Details";
    CtrlLayout(*this);
    prevBtn.Disable();
    enablePrev = false;
    
    for (const auto& genre : GenreCatalog::List()) genreEd.AddList(genre);
    genreEd.AlwaysDrop();
    
    auto fn = [=] {
        String value = ~titleEd.TrimBoth();
        enableNext = value.GetLength() > 0;
        value = ~artistEd.TrimBoth();
        enableNext = enableNext && value.GetLength() > 0;
        nextBtn.Enable(enableNext);
    };
    
    titleEd.WhenAction << fn;
    artistEd.WhenAction << fn;
}

void Page1::Reset() {
    titleEd.Clear();
    artistEd.Clear();
    yearEd.Clear();
    genreEd.Clear();
    writerEd.Clear();
    ownerEd.Clear();
    loadedAudioLbl.SetText("");
    enableNext = false;
}

void Page1::Populate() {
    titleEd.SetData(data.title);
    artistEd.SetData(data.artist);
    if (data.year > 0) yearEd.SetData(data.year);
    else yearEd.Clear();
    genreEd.SetData(data.genre);
    writerEd.SetData(data.writer);
    ownerEd.SetData(data.owner);
    loadedAudioLbl.SetLabel(data.origAudioFilePath);
    enableNext = !TrimBoth(data.title).IsEmpty() &&
                !TrimBoth(data.artist).IsEmpty() &&
                !TrimBoth(data.genre).IsEmpty();
    titleEd.SetFocus();
}

void Page1::SaveData() {
    data.title = titleEd.TrimBoth().GetData();
    data.artist = artistEd.TrimBoth().GetData();
    Value year = yearEd.GetData();
    data.year = year.IsNull() ? 0:(int)year;
    data.genre = genreEd.TrimBoth().GetData();
    data.writer = writerEd.TrimBoth().GetData();
    data.owner = ownerEd.TrimBoth().GetData();
}
