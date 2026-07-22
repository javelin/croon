# Croon Code Duplication Audit

This document records semantically equal (or near-equal) code discovered across
the Croon sources. It is a findings log, not a refactoring plan — each entry
notes where the duplication lives and what a consolidation would look like, so
the cleanup can be scheduled deliberately.

The audit concentrated on the higher-signal areas (lyrics providers, ffmpeg
command builders, project/list controls, dialogs, and the lyric/subtitle
transformers). It is not an exhaustive diff of every translation unit.

## Resolved

### `LrcText` — byte-for-byte duplicate (most important)

`String LrcText(String line, bool isMeta)` existed twice, once in
`LrcGenerator.cpp` and once in `LrcPreviewGenerator.cpp`, in each file's
anonymous namespace. The two bodies were **byte-for-byte identical** (verified
by diffing the function bodies pulled from git — zero differences, including the
NBSP blank-line handling).

Because U++ BLITZ builds concatenate multiple translation units into one, the
two anonymous-namespace definitions merged into a single namespace and collided,
producing `error: redefinition of 'LrcText'` in TheIDE. The CMake/`umk` build
happened to batch the files differently and did not surface the collision, so
the duplication was latent rather than obviously broken.

Resolution: the helper was promoted to a single `LrcGenerator::LrcText` static
method; `LrcPreviewGenerator` now calls `LrcGenerator::LrcText`. Pure refactor,
no behavior change (the promoted body matches the original exactly).

Files: `LrcGenerator.h`, `LrcGenerator.cpp`, `LrcPreviewGenerator.cpp`.

## Open findings

Ordered by strength of the match. None of these are fixed yet.

### 1. `SubtitleLineProcessor::ResolveStyle` vs `ResolveDimStyle`

`SubtitleLineProcessor.cpp`. The two functions are structurally identical: the
same `isMeta` -> strip leading `@`, `~` -> italic wrap, `(` -> italic wrap, then
a `switch(part)` cascade. The only differences are that the dim variant appends
`Normal` to every returned style name (`"V1"` -> `"V1Normal"`, and so on) and
`ResolveStyle` carries an extra count-in branch at the top. Cleanest
consolidation target: one function parameterized by a `bool dim` (or a style
suffix).

### 2. Linear "find string in a vector" reimplemented ~5 times

The same primitive — walk a `Vector<String>` (or vector of items) and test each
entry against a target — appears as separate hand-written loops:

- `ProjectList::ContainsProject` and `ProjectList::IsRemovedProject`
  (`ProjectList.cpp`) are semantically equal to each other (loop, `return true`
  on match, `return false` otherwise); they differ only in which member vector
  they scan.
- `ProjectList::AddUniquePath` (the file-local helper) is the same search
  followed by an append.
- `ProjectList::FindProject` is the same search returning a pointer + index.
- `RecentProjectService::FindPathIndex` (`RecentProjectService.cpp`) is the same
  search returning an index.

A single shared `IndexOf` / `Contains` helper would replace all of them.

### 3. `LyricsTransformer::RawToUntimed` — word-wrap loop duplicated in-function

`LyricsTransformer.cpp`. The word-wrap loop that accumulates `_line`, breaks at
`MaxLineLength`, and pushes `TimeLyrics` entries is written out **twice** in the
same function: once inside the `while` over interior lines, and again for the
trailing line after the loop. Extract one helper and call it from both places.

### 4. `FfmpegExportCommandBuilder::WithBackgroundVideo` vs `WithVisualization`

`FfmpegExportCommandBuilder.cpp`. Two fragments are duplicated verbatim between
the two builders:

- the `#ifdef PLATFORM_WIN32` block that escapes the `.ass` subtitle path, and
- the entire trailing `-metadata` argument list (title, artist, composer,
  copyright, genre, year, comment, lyrics) plus `outputPath`.

Both fragments would move cleanly into small shared helpers.

### 5. `GeniusLyricsProvider::BuildUrl` vs `SongLyricsProvider::BuildUrl`

`GeniusLyricsProvider.cpp` / `SongLyricsProvider.cpp`. The two bodies are
identical: `Format(UrlFormat(), LyricsProviderTools::HyphenSlug(artist),
LyricsProviderTools::HyphenSlug(title))`. Only the per-provider `UrlFormat()`
string differs, and that already lives in each provider. The shared body could
live in a common base or free helper.

### 6. `OpenProjectDlg` extract steps share a skeleton

`OpenProjectDlg.cpp`. `ExtractAudio`, `ExtractVideo`, and `LoadThumbnail` repeat
the same shape three times: set a temp path, build the command vector, call
`process.Start`, then the identical tail
`if (!res) { Exclamation(... Phase() ...); Break(IDCANCEL); } else PollProgress();`.
The start-or-fail tail is the duplicated part.

### 7. `ProjectList` project-open flow repeated

`ProjectList.cpp`. `OpenProjectItem` and `OpenProject` share the "run
`OpenProjectDlg` -> `pick` into `data` -> `UpdateList` -> open audio with the
same failure `Exclamation` -> `WhenProjectLoaded`" block. The audio-open-and-warn
snippet also appears a third time in `NewProject`.

## Intentional / left as-is

These look similar but are not worth collapsing:

- `ConfigService::Set(key, String)` vs `ConfigService::Set(key, int)` —
  identical bodies but different parameter types; this is ordinary overloading.
- `ListChildCtrl`'s two constructors share the focus-frame setup, which is
  normal constructor boilerplate.
- `TimingLine::Normal` vs `TimingLine::Highlight2` — differ only by a single
  color constant, so behavior is genuinely different.

## Coverage note

The controls (`TimingCtrl`, `LyricsPartsCtrl`), the serializer/loader pair
(`ProjectSerializer`, `ProjectLoader`), and `Page3` were not diffed line by line
and may contain further overlap. A follow-up pass can extend this log.
