cmake_minimum_required(VERSION 3.20)

project(Croon LANGUAGES NONE)

set(CROON_UPP_ROOT "/Users/mdoc/Dev/u++/upp" CACHE PATH "Ultimate++ root containing uppsrc")
set(CROON_UPPSRC "${CROON_UPP_ROOT}/uppsrc" CACHE PATH "Ultimate++ uppsrc nest")
set(CROON_DEFAULT_BUILD_METHOD "CLANG")
if(EXISTS "${CROON_UPP_ROOT}/.config/u++/theide/CLANG.bm")
    set(CROON_DEFAULT_BUILD_METHOD "${CROON_UPP_ROOT}/.config/u++/theide/CLANG.bm")
endif()
set(CROON_BUILD_METHOD "${CROON_DEFAULT_BUILD_METHOD}" CACHE STRING "U++ build method name or .bm path")
if(CROON_BUILD_METHOD STREQUAL "CLANG" AND NOT CROON_DEFAULT_BUILD_METHOD STREQUAL "CLANG")
    set(CROON_BUILD_METHOD "${CROON_DEFAULT_BUILD_METHOD}" CACHE STRING "U++ build method name or .bm path" FORCE)
endif()
set(CROON_UMK_OPTIONS "-brs" CACHE STRING "Options passed to umk for the Croon build")

find_program(UPP_UMK NAMES umk DOC "Ultimate++ command-line builder")
find_program(CROON_MAKE_PROGRAM NAMES gmake make DOC "Make tool used to bootstrap umk")
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CROON_BOOTSTRAPPED_UMK "${CROON_UPP_ROOT}/umk")
set(CROON_UMK_MAKEFILE "${CROON_UPP_ROOT}/umkMakefile")
set(CROON_UMK_EXECUTABLE "")
set(CROON_PACKAGE_NEST "${CMAKE_CURRENT_BINARY_DIR}/upp-nest")
set(CROON_PACKAGE_DIR "${CROON_PACKAGE_NEST}/Croon")
file(GLOB CROON_PACKAGE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.upp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.lay"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.iml"
)

if(UPP_UMK)
    set(CROON_UMK_EXECUTABLE "${UPP_UMK}")
elseif(EXISTS "${CROON_BOOTSTRAPPED_UMK}")
    set(CROON_UMK_EXECUTABLE "${CROON_BOOTSTRAPPED_UMK}")
elseif(EXISTS "${CROON_UMK_MAKEFILE}" AND CROON_MAKE_PROGRAM)
    add_custom_command(
        OUTPUT "${CROON_BOOTSTRAPPED_UMK}"
        COMMAND "${CROON_MAKE_PROGRAM}" -f "${CROON_UMK_MAKEFILE}"
        WORKING_DIRECTORY "${CROON_UPP_ROOT}"
        COMMENT "Bootstrapping Ultimate++ umk"
        VERBATIM
    )
    add_custom_target(croon_umk DEPENDS "${CROON_BOOTSTRAPPED_UMK}")
    set(CROON_UMK_EXECUTABLE "${CROON_BOOTSTRAPPED_UMK}")
endif()

enable_testing()

add_test(
    NAME croon_identity
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_identity.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME croon_core_contracts
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_core_contracts.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME croon_process_contracts
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_process_contracts.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME croon_simple_layouts
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_simple_layouts.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME croon_wizard_layouts
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_wizard_layouts.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME croon_editor_layouts
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_editor_layouts.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME croon_export_audit
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_export_audit.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME croon_cmake_umk_contract
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_cmake_umk_contract.py" "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_target(croon_package_nest
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CROON_PACKAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}"
        "-DCROON_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        "-DCROON_PACKAGE_DIR=${CROON_PACKAGE_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sync_upp_package.cmake"
    SOURCES
        ${CROON_PACKAGE_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sync_upp_package.cmake"
    COMMENT "Creating build-local U++ Croon nest"
    VERBATIM
)

set(CROON_NESTS "${CROON_PACKAGE_NEST},${CROON_UPPSRC}")

if(CROON_UMK_EXECUTABLE)
    set(CROON_CORE_TEST_BINARY "${CMAKE_CURRENT_BINARY_DIR}/tests/CroonCoreTests")

    add_custom_target(croon_core_behavior
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/tests"
        COMMAND "${CROON_UMK_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/upp,${CROON_PACKAGE_NEST},${CROON_UPPSRC}"
            CroonCoreTests
            "${CROON_BUILD_METHOD}"
            "${CROON_UMK_OPTIONS}"
            "${CROON_CORE_TEST_BINARY}"
        COMMAND "${CROON_CORE_TEST_BINARY}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building and running Croon core behavior tests"
        VERBATIM
    )
    add_dependencies(croon_core_behavior croon_package_nest)

    add_test(
        NAME croon_core_behavior
        COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}" --target croon_core_behavior
    )
endif()

if(CROON_UMK_EXECUTABLE)
    add_custom_target(croon
        COMMAND "${CROON_UMK_EXECUTABLE}" "${CROON_NESTS}" Croon "${CROON_BUILD_METHOD}" "${CROON_UMK_OPTIONS}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building Croon with Ultimate++ umk"
        VERBATIM
    )
    if(TARGET croon_umk)
        add_dependencies(croon croon_umk)
    endif()
    add_dependencies(croon croon_package_nest)

    add_test(
        NAME croon_app_build
        COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}" --target croon
    )
else()
    add_custom_target(croon
        COMMAND "${CMAKE_COMMAND}" -E echo "umk was not found and could not be bootstrapped. Reconfigure with -DUPP_UMK=/path/to/umk, add umk to PATH, or provide ${CROON_UMK_MAKEFILE}."
        COMMAND "${CMAKE_COMMAND}" -E false
        COMMENT "Cannot build Croon without Ultimate++ umk"
        VERBATIM
    )
endif()
