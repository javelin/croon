# Croon Contracts

## Project Artifacts

- Primary project extension: `.croon`.
- Project metadata attachment name: `croon.info`.
- Current project metadata format version: `1.1`.
- Project metadata must include a `version` field. Format changes must be represented by explicit version handling in `ProjectSerializer`.
- Project metadata readers must tolerate legacy unversioned metadata and explicit `1.0` metadata by treating them as the current readable format.
- Legacy unversioned metadata must still be object-shaped JSON metadata.
- Unsupported explicit metadata versions must remain visible on read so future migrations can make compatibility decisions.
- `ProjectSerializer` must classify metadata compatibility as current, legacy unversioned, unsupported, or invalid before callers decide whether to load.
- `ProjectSerializer` compatibility labels must remain stable for diagnostics and future UI/logging.
- Invalid metadata hydration must not produce a current-version project record.
- Runtime app data directory name: `Croon`.
- Temporary file prefixes should use `Croon_`.

## Build Contracts

- TheIDE loads `Croon.upp`.
- CMake configures from the repository root and invokes `umk` for application builds.
- The U++ source nest must include this repository and the U++ `uppsrc` directory.

## Runtime Contracts

- `ffmpeg` is discovered from configuration or defaults to `ffmpeg` on `PATH`.
- Missing or invalid `ffmpeg` must be reported before media workflows run.
- Media commands are represented as argument vectors, not shell-concatenated strings.

## Test Contracts

- Tests must avoid launching the GUI.
- Tests must not require real media files unless a phase explicitly adds fixtures.
- Command-generation tests should validate executable arguments without spawning `ffmpeg`.
- Serialization tests should validate round-trip behavior and Croon artifact names.
