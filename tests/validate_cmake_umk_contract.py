#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_cmake_umk_contract: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    cmake = (root / "CMakeLists.txt").read_text()

    for needle in [
        'set(CROON_UPP_ROOT "/Users/mdoc/Dev/u++/upp"',
        'set(CROON_DEFAULT_BUILD_METHOD "${CROON_UPP_ROOT}/.config/u++/theide/CLANG.bm")',
        'set(CROON_BUILD_METHOD "${CROON_DEFAULT_BUILD_METHOD}" CACHE STRING "U++ build method name or .bm path" FORCE)',
        'set(CROON_UMK_OPTIONS "-brs" CACHE STRING "Options passed to umk for the Croon build")',
        'set(CROON_BOOTSTRAPPED_UMK "${CROON_UPP_ROOT}/umk")',
        'set(CROON_UMK_MAKEFILE "${CROON_UPP_ROOT}/umkMakefile")',
        'set(CROON_PACKAGE_NEST "${CMAKE_CURRENT_BINARY_DIR}/upp-nest")',
        'set(CROON_PACKAGE_DIR "${CROON_PACKAGE_NEST}/Croon")',
        "file(GLOB CROON_PACKAGE_SOURCES CONFIGURE_DEPENDS",
        'cmake/sync_upp_package.cmake',
        "add_custom_target(croon_package_nest",
        "add_custom_target(croon_core_behavior",
        "add_dependencies(croon_core_behavior croon_package_nest)",
        "NAME croon_core_behavior",
        "CroonCoreTests",
        "ProjectSerializer.cpp",
        'set(CROON_NESTS "${CROON_PACKAGE_NEST},${CROON_UPPSRC}")',
        "add_custom_target(croon_umk",
        "add_dependencies(croon croon_umk)",
        "add_dependencies(croon croon_package_nest)",
        "NAME croon_app_build",
        '--target croon',
        'COMMAND "${CROON_UMK_EXECUTABLE}" "${CROON_NESTS}" Croon "${CROON_BUILD_METHOD}" "${CROON_UMK_OPTIONS}"',
    ]:
        if needle not in cmake:
            fail(f"CMakeLists.txt missing {needle}")

    readme = (root / "README.md").read_text()
    for needle in [
        "/Users/mdoc/Dev/u++/upp",
        "-brs",
        "build/upp-nest/Croon",
        "build-local package copy",
        "umkMakefile",
        "-DUPP_UMK=/path/to/umk",
    ]:
        if needle not in readme:
            fail(f"README.md missing {needle}")


if __name__ == "__main__":
    main()
