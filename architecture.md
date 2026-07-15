# Croon Architecture

Croon is an Ultimate++ desktop application for karaoke authoring. It is being migrated in small, validated phases while preserving TheIDE compatibility and adding a CMake entry point that drives the U++ `umk` builder.

## System Shape

- `Croon.upp` is the authoritative U++ package definition for source files, package dependencies, and TheIDE loading.
- `CMakeLists.txt` is a build convenience layer that configures paths and invokes `umk`; it does not replace the U++ package graph.
- `ffmpeg` is an external runtime executable. Croon constructs command arguments and launches the configured executable rather than linking libav libraries.
- Static form layouts should live in U++ Designer `.lay` files. Runtime-populated lists, custom-painted controls, timing rows, and menu wiring may remain in C++.
- Deterministic behavior should be isolated behind testable services before or during UI migration.
- `RunCroon` owns the normal application `KarData` instance and passes it into `MainWindow`.
- `MainWindow` is the normal composition root for shared project UI state. It owns the runtime `Project`, `ProjectList`, `VideoDlg`, and `WizardDlg` instances and wires them to the shared `KarData` model and dialog dependencies.
- Runtime UI code uses explicit `KarData` and dialog ownership; legacy global data and dialog accessors have been retired from the application surface.
- Long-running list population should keep the UI usable. The project list installs interactions before background loading completes, and the video picker streams discovered thumbnails directly into the picker instead of showing a modal gather dialog.

## Primary Flows

1. Import or open a Croon project.
2. Convert or extract media through external `ffmpeg` commands.
3. Edit metadata, lyrics, timing, vocal parts, and video/visualization settings.
4. Save a `.croon` project artifact.
5. Export preview or final video with generated subtitles.

## Migration Principle

Each phase must keep the repository buildable or intentionally build-neutral, add validation for the behavior introduced in that phase, and avoid implementing future-phase behavior early.
