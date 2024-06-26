cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME "VHFRadioEffect")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum macOS version required")

project("${PROJECT_NAME}" VERSION 1.0.0)

set(formats "VST3" "Standalone")

if(APPLE)
    list(APPEND formats "AU")
endif()

include(FetchContent)
FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.1
)
FetchContent_MakeAvailable(JUCE)

set(RTNEURAL_STL ON CACHE BOOL "Use RTNeural with this backend" FORCE)
add_subdirectory(Modules/RTNeural)
include_directories(Modules/RTNeural)

juce_add_plugin("${PROJECT_NAME}"
    VERSION "1.0.0"
    COMPANY_NAME "saggiefx"
    PLUGIN_MANUFACTURER_CODE "SGGX"
    PLUGIN_CODE "VHFR"
    FORMATS ${formats}
    COPY_PLUGIN_AFTER_BUILD FALSE
    IS_SYNTH FALSE     
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PRODUCT_NAME "${PROJECT_NAME}"
    PLUGIN_NAME "Tactical Comm")

juce_generate_juce_header("${PROJECT_NAME}")

file(GLOB_RECURSE SOURCE_FILES "Source/*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_20)

target_compile_definitions("${PROJECT_NAME}"
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

file(GLOB_RECURSE BUCKET "Bucket/*")

juce_add_binary_data(BinaryData
    	SOURCES
	${BUCKET})

target_link_libraries("${PROJECT_NAME}"
    	PRIVATE
        BinaryData
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
	RTNeural)
