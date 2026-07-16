# Croon Decisions

## Accepted Decisions

### U++ Package Is Source Of Truth

Croon will keep a `Croon.upp` package as the authoritative application definition. TheIDE and `umk` must be able to load and build the package.

### CMake Drives `umk`

CMake support means configuring a CMake build that invokes `umk` with the Croon package and U++ nests. Native CMake compilation of the U++ dependency graph is out of scope unless a later decision explicitly replaces this one.

### ffmpeg Is External

Croon depends on a configured `ffmpeg` executable at runtime. The project should not add libav compile/link dependencies during the current migration.

### Croon Uses Croon Artifacts

Legacy product names must be renamed when artifacts are created or migrated. This includes package names, image classes, project extensions, app data folders, temp prefixes, visible product strings, and metadata attachment names.

### Designer-Editable Static UI

Static TopWindow and form-like Ctrl layouts should move to U++ `.lay` files. Dynamic controls remain C++-driven when Designer ownership would reduce clarity or testability.

### Croon Metadata Compatibility

Croon must keep `.croon` as the supported project artifact format. Project metadata compatibility is versioned through `ProjectSerializer`: current `1.1` metadata is supported, legacy `1.0` and unversioned `.croon` metadata are treated as the current readable format when object-shaped JSON, unsupported explicit metadata versions are rejected by load gates while preserving their version for future migration decisions, and invalid metadata is rejected before hydration.

### Explicit Runtime Project State

Croon runtime UI code owns and passes `KarData` explicitly from `RunCroon` through `MainWindow` into child dialogs and controls. The legacy global `KarData` accessor has been removed from the application surface.

### VideoCatalog Owns Video Discovery

Configured video directory enumeration, cached video listing, thumbnail file path construction, and cached thumbnail image loading belong behind `VideoCatalog`. `Page3` owns the current incremental picker flow and streams discovered video thumbnails into the picker without the retired modal gather dialog. A later cache service may still move thumbnail reuse and shared candidate state out of the UI layer.

### No Legacy Product Artifact Import

Croon will not add import support for older legacy product metadata or other legacy product artifacts outside the `.croon` compatibility policy. Legacy product names remain migrated when Croon creates or owns artifacts, and `.croon` metadata compatibility remains the supported backward-compatibility surface.

### Disable Live RichText ASS Preview

The editor no longer shows or refreshes the RichText ASS preview tab. Maintaining an eager QTF approximation of ASS is expensive on lyrics/timing changes and less faithful than libass/ffmpeg rendering; a future lightweight preview can reuse the tab area for LRC-formatted lyrics.

### Probe Wrapping With Rendered Highlight Lines

Subtitle wrapping should be measured from libass/ffmpeg-rendered output rather than predicted from text length. The probe path renders one highlighted normal-size lyric per frame, because the grayed line has the same effective size and incoming lines are smaller. `ExportDlg::ExportASS` uses the synchronous probe runner to raise the grayed row only when the current highlighted line wraps; live UI preview remains disabled.

## Deferred Decisions

- Whether to replace scraping with a reliable lyrics provider API. Until a reliable API is chosen, scraped providers remain internal implementation details behind `LyricsDownloadService`; AZLyrics is tried first, Genius and SongLyrics are fallback candidates, and the user-facing download workflow stays opaque.
- Whether to expand `VideoCatalog` into a `VideoLibraryCache` service for shared video-library state, thumbnail reuse policy, and cross-session cache behavior. Incremental video picker loading handles the current responsiveness problem, so this is no longer required just to avoid picker startup blocking.
