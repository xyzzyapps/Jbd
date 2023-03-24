cmake_minimum_required(VERSION 3.15)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(JBD VERSION 0.1)

# find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE

set(CMAKE_CXX_STANDARD 14)
set(BUILD_SHARED_LIBS  OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

juce_add_plugin(Jbd
    VERSION "0.1"
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    COMPANY_NAME "Xyzzy Apps"
    PRODUCT_NAME "Jbd"
    FORMATS VST3 
    BUNDLE_ID "org.xyzzy.apps"
    PLUGIN_MANUFACTURER_CODE VmbA
    PLUGIN_CODE MdMo
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Tools"
    VST_NUM_MIDI_OUTS 1
    VST_NUM_MIDI_INS 1
    VST3_COPY_DIR "C:/Program Files/Common Files/VST3"
    )



target_sources(Jbd
    PRIVATE
        Main.cpp
        )

target_compile_definitions(Jbd
    PRIVATE
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Jbd,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Jbd,JUCE_VERSION>")

# juce_add_binary_data(JbdData SOURCES ...)

include_directories(include)
link_directories(lib)

target_link_libraries(Jbd
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
            )

target_link_libraries(Jbd
PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/lib/asmjit.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/c10.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/clog.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/cpuinfo.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/dnnl.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/fbgemm.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/fbjni.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/kineto.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/libprotobuf-lite.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/libprotobuf.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/libprotoc.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/pthreadpool.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/pytorch_jni.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/torch.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/torch_cpu.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/XNNPACK.lib
        )


juce_generate_juce_header(Jbd)

# add_custom_command(
#        TARGET GuiAppExample POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#                "GuiAppExample_artefacts\\Debug\\Gui App Example.exe"
#                lib)