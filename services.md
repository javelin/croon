# Croon Services

This file records service boundaries expected during migration. Some services begin as extracted wrappers around existing logic and can be refined in later phases.

## Extracted Services

- `AppIdentity`: product name, version, project extension, app data name, temp prefixes, and metadata attachment names.
- `AppPaths`: runtime data directory creation and simple file discovery helpers.
- `ConfigService`: persisted Croon settings and default values.
- `DownloadDefaults`: shared HTTP download defaults such as the browser user agent.
- `ProjectSerializer`: versioned JSON serialization and deserialization for Croon project metadata.
- `LyricsTransformer`: deterministic conversion between raw lyric text and Croon timed lyric rows.
- `FfmpegCommandBuilder`: deterministic command argument construction.
- `FfmpegProgressParser`: timestamp extraction from `ffmpeg` progress output.
- `GenreCatalog`: shared application genre reference list.
- `LyricsDownloadService`: AZLyrics URL construction, page extraction, and download workflow.
- `RecentProjectService`: recent project list persistence and normalization.
- `RichTextBuilder`: QTF/RichText construction helper for shared list and subtitle previews.
- `SubtitleLineProcessor`: subtitle metadata expansion, count-in insertion, and vocal style resolution.
- `SubtitleGenerator`: ASS subtitle generation from timed lyrics and vocal parts.
- `TextTools`: shared text spacing, filtering, and display-shortening helpers.
- `TimeFormatter`: timestamp and count-in duration formatting helpers.
- `UiScaler`: U++ zoom-ratio scaling helpers for layout dimensions.
- `MediaProcessRunner`: process execution boundary for `ffmpeg` and related long-running tasks.

## UI Boundary

UI classes should bind controls to services and models. Static control placement belongs in `.lay` files when the screen is form-like or dialog-like. Custom runtime controls may expose stable methods that UI shells call after `CtrlLayout`.

## Compatibility Facade

- `Util`: legacy compatibility facade for older call sites and tests. New production code should call the extracted services directly.
