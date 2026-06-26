# Croon Services

This file records service boundaries expected during migration. Some services begin as extracted wrappers around existing logic and can be refined in later phases.

## Extracted Services

- `AppIdentity`: product name, version, project extension, app data name, temp prefixes, and metadata attachment names.
- `ConfigService`: persisted Croon settings and default values.
- `ProjectSerializer`: versioned JSON serialization and deserialization for Croon project metadata.
- `FfmpegCommandBuilder`: deterministic command argument construction.
- `RecentProjectService`: recent project list persistence and normalization.
- `SubtitleGenerator`: ASS subtitle generation from timed lyrics and vocal parts.
- `MediaProcessRunner`: process execution boundary for `ffmpeg` and related long-running tasks.

## UI Boundary

UI classes should bind controls to services and models. Static control placement belongs in `.lay` files when the screen is form-like or dialog-like. Custom runtime controls may expose stable methods that UI shells call after `CtrlLayout`.
