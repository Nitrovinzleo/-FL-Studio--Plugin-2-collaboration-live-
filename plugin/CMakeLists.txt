cmake_minimum_required(VERSION 3.22)
project(FLStudioCollabPlugin VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Fetch JUCE Framework
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
)
FetchContent_MakeAvailable(juce)

# 2. Fetch IXWebSocket C++ library
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
set(USE_TLS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG        v11.4.4
)
FetchContent_MakeAvailable(ixwebsocket)

if(APPLE)
    set(PLUGIN_FORMATS VST3 AU Standalone)
else()
    set(PLUGIN_FORMATS VST3 Standalone)
endif()

# Create VST3, AU (macOS) and Standalone Plugin targets
juce_add_plugin(FLStudioCollabPlugin
    COMPANY_NAME "FLStudioCollab"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE FLCB
    PLUGIN_CODE Flcb
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "FL Studio Collab Plugin"
)

juce_generate_juce_header(FLStudioCollabPlugin)

# Source files
target_sources(FLStudioCollabPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/WebSocketClient.cpp
    Source/WebSocketClient.h
)

# Target dependencies
target_link_libraries(FLStudioCollabPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    ixwebsocket
)

target_compile_definitions(FLStudioCollabPlugin PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
