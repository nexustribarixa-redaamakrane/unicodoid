cmake_minimum_required(VERSION 3.20)

project(Unicodoid
    VERSION 1.0.0
    DESCRIPTION "Unicodoid encoding-system SDK"
    LANGUAGES C CXX
)

option(UNICODOID_BUILD_TESTS "Build the C and C++ test suites" ON)
option(UNICODOID_BUILD_CPP "Build the C++ wrapper" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UNICODOID_C_SOURCES
    src/unicodoid_system.c
    src/unicodoid_codec.c
    src/unicodoid_string.c
    src/unicodoid_transcode.c
    src/unicodoid_unicode.c
    src/unicodoid_ucd_data.c
    src/unicodoid_ucd.c
    src/unicodoid_properties.c
    src/unicodoid_normalization.c
    src/unicodoid_segmentation.c
    src/unicodoid_collation.c
    src/unicodoid_registry.c
    src/ConvertUTF.c
    src/uax/uax29_segmentation.c
    src/uax/uax34_named_sequences.c
    src/uts/uts58_link_detection.c
    src/uts/uts10_collation.c
    src/uts/uts51_emoji_sequences.c
    src/utr/utr59_east_asian_spacing.c
    src/systems/unicodoid_utf8.c
    src/systems/unicodoid_utf16.c
    src/systems/unicodoid_utf32.c
    src/systems/unicodoid_ascii.c
    src/systems/unicodoid_custom_conlang.c
    src/systems/unicodoid_shiftmodal16.c
)

add_library(unicodoid STATIC ${UNICODOID_C_SOURCES})
add_library(Unicodoid::unicodoid ALIAS unicodoid)

target_include_directories(unicodoid
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(MSVC)
    target_compile_options(unicodoid PRIVATE /W4)
else()
    target_compile_options(unicodoid PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(UNICODOID_BUILD_CPP)
    add_library(unicodoid_cpp STATIC
        cpp/src/unicodoid_cpp.cpp
        cpp/src/ConvertUTF.cpp
        cpp/src/uax/uax15_normalization.cpp
        cpp/src/uax/uax29_segmentation.cpp
        cpp/src/uax/uax34_named_sequences.cpp
        cpp/src/uax/uax44_properties.cpp
        cpp/src/uts/uts10_collation.cpp
        cpp/src/uts/uts51_emoji_sequences.cpp
        cpp/src/uts/uts58_link_detection.cpp
        cpp/src/utr/utr59_east_asian_spacing.cpp
    )
    add_library(Unicodoid::cpp ALIAS unicodoid_cpp)

    target_include_directories(unicodoid_cpp
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(unicodoid_cpp PUBLIC unicodoid)

    if(MSVC)
        target_compile_options(unicodoid_cpp PRIVATE /W4)
    else()
        target_compile_options(unicodoid_cpp PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

if(UNICODOID_BUILD_TESTS)
    enable_testing()

    add_executable(test_core tests/test_core.c)
    target_link_libraries(test_core PRIVATE unicodoid)
    add_test(NAME core COMMAND test_core)

    add_executable(test_unicode tests/test_unicode.c)
    target_link_libraries(test_unicode PRIVATE unicodoid)
    add_test(NAME unicode COMMAND test_unicode)

    add_executable(test_stateful tests/test_stateful.c)
    target_link_libraries(test_stateful PRIVATE unicodoid)
    add_test(NAME stateful COMMAND test_stateful)

    add_executable(test_conformance tests/test_ucd18_conformance.c)
    target_link_libraries(test_conformance PRIVATE unicodoid)
    add_test(NAME conformance COMMAND test_conformance)

    add_executable(test_uts58 tests/test_uts58.c)
    target_link_libraries(test_uts58 PRIVATE unicodoid)
    target_compile_definitions(test_uts58 PRIVATE
        UNICODOID_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0"
    )
    add_test(NAME linkification COMMAND test_uts58)

    add_executable(test_collation tests/test_collation.c)
    target_link_libraries(test_collation PRIVATE unicodoid)
    target_compile_definitions(test_collation PRIVATE
        UNICODOID_UCA_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/uca/test/CollationTest"
    )
    add_test(NAME collation COMMAND test_collation)

    add_executable(test_uax34 tests/test_uax34.c)
    target_link_libraries(test_uax34 PRIVATE unicodoid)
    target_compile_definitions(test_uax34 PRIVATE
        UNICODOID_NAMED_SEQUENCES_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/ucd/NamedSequences.txt"
    )
    add_test(NAME named_sequences COMMAND test_uax34)

    add_executable(test_uts51 tests/test_uts51.c)
    target_link_libraries(test_uts51 PRIVATE unicodoid)
    target_compile_definitions(test_uts51 PRIVATE
        UNICODOID_EMOJI_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/emoji"
    )
    add_test(NAME emoji_sequences COMMAND test_uts51)

    add_executable(test_utr59 tests/test_utr59.c)
    target_link_libraries(test_utr59 PRIVATE unicodoid)
    target_compile_definitions(test_utr59 PRIVATE
        UNICODOID_TR59_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/tr59/east-asian-spacing.txt"
    )
    add_test(NAME east_asian_spacing COMMAND test_utr59)

    add_executable(test_normalization tests/test_normalization.c)
    target_link_libraries(test_normalization PRIVATE unicodoid)
    target_compile_definitions(test_normalization PRIVATE
        UNICODOID_UCD_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/ucd"
    )
    add_test(NAME normalization COMMAND test_normalization)

    add_executable(test_convertutf tests/harness.c)
    target_link_libraries(test_convertutf PRIVATE unicodoid)
    add_test(NAME convertutf COMMAND test_convertutf)

    if(UNICODOID_BUILD_CPP)
        add_executable(test_cpp tests/test_cpp.cpp)
        target_link_libraries(test_cpp PRIVATE unicodoid_cpp unicodoid)
        add_test(NAME cpp COMMAND test_cpp)

        add_executable(test_cpp_conformance cpp/tests/test_cpp_conformance.cpp)
        target_link_libraries(test_cpp_conformance PRIVATE unicodoid_cpp unicodoid)
        target_compile_definitions(test_cpp_conformance PRIVATE
            UNICODOID_UCD_AUX_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/ucd/auxiliary"
        )
        add_test(NAME cpp_conformance COMMAND test_cpp_conformance)

        add_executable(test_cpp_specs cpp/tests/test_cpp_specs.cpp)
        target_link_libraries(test_cpp_specs PRIVATE unicodoid_cpp unicodoid)
        target_compile_definitions(test_cpp_specs PRIVATE
            UNICODOID_UCD_AUX_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/ucd/auxiliary"
            UNICODOID_UCD_TR59_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/tr59/east-asian-spacing.txt"
            UNICODOID_UCD_NAMEREP_SEQ_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/ucd/NamedSequences.txt"
            UNICODOID_UCD_EMOJI_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/emoji"
            UNICODOID_UCD_LINK_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/linkification"
            UNICODOID_UCD_UCA_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/ucd/18.0.0/uca/test/CollationTest"
        )
        add_test(NAME cpp_specs COMMAND test_cpp_specs)

        add_executable(test_convertutf_cpp cpp/tests/harness.cpp)
        target_link_libraries(test_convertutf_cpp PRIVATE unicodoid_cpp unicodoid)
        add_test(NAME convertutf_cpp COMMAND test_convertutf_cpp)
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS unicodoid
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(UNICODOID_BUILD_CPP)
    install(TARGETS unicodoid_cpp
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(DIRECTORY cpp/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
