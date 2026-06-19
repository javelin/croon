# Croon

Croon is an Ultimate++ karaoke authoring tool.

The project is being migrated in phases. The authoritative U++ application package will be `Croon.upp`; CMake support will drive the U++ `umk` builder rather than replacing the U++ package graph.

## Build

Configure and run tests:

```sh
cmake -S . -B build
ctest --test-dir build --output-on-failure
```

Build the U++ application:

```sh
cmake --build build --target croon
```

By default CMake uses the sandboxed U++ root at `/Users/mdoc/Dev/u++/upp`, the sandboxed `CLANG.bm` build method when available, and `umk` options `-brs` for BLITZ, release, shared-link builds. Because U++ expects packages to live under a nest directory named after the package, CMake creates a build-local nest at `build/upp-nest/Croon` that symlinks back to this repository. If `umk` is not on `PATH`, the `croon` target bootstraps `/Users/mdoc/Dev/u++/upp/umk` from `/Users/mdoc/Dev/u++/upp/umkMakefile` when that makefile is available. You can override the executable with `-DUPP_UMK=/path/to/umk`.
