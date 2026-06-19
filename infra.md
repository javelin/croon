# Croon Infrastructure

## Local Dependencies

- Ultimate++ source tree with `uppsrc`.
- `umk` or TheIDE build tooling.
- SDL2 and SDL2_mixer for audio playback.
- OpenSSL through U++ `Core/SSL`.
- PCRE through U++ `plugin/pcre`.
- External `ffmpeg` executable at runtime.

## Build Entry Points

- TheIDE: open `Croon.upp` from a nest containing the Croon repository and U++ `uppsrc`.
- CMake: configure from the Croon repository root, then build the generated target that invokes `umk`.
- Default U++ root: `/Users/mdoc/Dev/u++/upp`.
- CMake creates a build-local U++ nest at `build/upp-nest/Croon` so the lowercase repository directory can still expose package `Croon` to `umk`.
- If `umk` is not on `PATH`, CMake can bootstrap `/Users/mdoc/Dev/u++/upp/umk` from `/Users/mdoc/Dev/u++/upp/umkMakefile`.

## Repository Hygiene

- Generated build directories should stay out of source control.
- Generated U++ output should stay outside the package root when possible.
- No build phase should depend on machine-specific Homebrew Cellar or Windows user paths copied from another local package.

## Validation

Each phase should document and run the narrowest validation that proves the migrated slice works. Build and test commands should be repeatable from the repository root once Phase 1 exists.
