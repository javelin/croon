/*
 * File  : Page1.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

Page1::Page1() {
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
    auto& data = KarData::GetGlobal();
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
    auto& data = KarData::GetGlobal();
    data.title = titleEd.TrimBoth().GetData();
    data.artist = artistEd.TrimBoth().GetData();
    Value year = yearEd.GetData();
    data.year = year.IsNull() ? 0:(int)year;
    data.genre = genreEd.TrimBoth().GetData();
    data.writer = writerEd.TrimBoth().GetData();
    data.owner = ownerEd.TrimBoth().GetData();
}
