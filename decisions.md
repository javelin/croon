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

Croon must keep `.croon` as the supported project artifact format. Project metadata compatibility is versioned through `ProjectSerializer`: current `1.0` metadata is supported, legacy unversioned `.croon` metadata is treated as the current readable format when it is object-shaped JSON, unsupported explicit metadata versions are rejected by load gates while preserving their version for future migration decisions, and invalid metadata is rejected before hydration.

### Explicit Runtime Project State

Croon runtime UI code owns and passes `KarData` explicitly from `RunCroon` through `MainWindow` into child dialogs and controls. The legacy global `KarData` accessor has been removed from the application surface.

### VideoCatalog Owns Video Discovery

Configured video directory enumeration, thumbnail file path construction, and cached thumbnail image loading belong behind `VideoCatalog`. `Page3` may still bind controls directly until a later cache service moves expensive thumbnail reuse and background scanning out of constructors.

## Deferred Decisions

- Whether Croon needs backward-compatible import of legacy product artifacts outside the `.croon` metadata compatibility policy.
- Whether to replace AZLyrics scraping with a different lyrics provider. Until a reliable provider API is chosen, AZLyrics remains an internal implementation detail behind `LyricsDownloadService`, provider-neutral URL/extraction infrastructure is preserved for future work, and the user-facing download workflow stays opaque.
- Whether to expand `VideoCatalog` into a `VideoLibraryCache` service for startup-time or background video discovery, thumbnail reuse, and shared video candidates across `WizardDlg` and `VideoDlg`. This remains the preferred future answer to expensive video directory scans; eager dialog construction may continue to hide that cost until the cache service exists.
