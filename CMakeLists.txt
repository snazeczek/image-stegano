cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0135 NEW)
project(image_stegano)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(SFML_VERSION 2.6.x)
FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        ${SFML_VERSION}
)
set(SFML_STATIC_LIBRARIES TRUE)
option(SFML_BUILD_AUDIO "Build audio" ON)
option(SFML_BUILD_NETWORK "Build network" OFF)
option(SFML_BUILD_EXAMPLES "Build SFML examples" OFF)
option(SFML_BUILD_DOC "Build SFML documentation" OFF)
FetchContent_MakeAvailable(SFML)

set(IMGUI_VERSION v1.89.9)
FetchContent_Declare(
        imgui_fc
        URL "https://github.com/ocornut/imgui/archive/refs/tags/${IMGUI_VERSION}.zip"
)
FetchContent_MakeAvailable(imgui_fc)

add_library(ImGui STATIC
        ${imgui_fc_SOURCE_DIR}/imgui.cpp
        ${imgui_fc_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_fc_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_fc_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_fc_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_fc_SOURCE_DIR}/backends/imgui_impl_win32.cpp
        ${imgui_fc_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(ImGui PUBLIC
        ${imgui_fc_SOURCE_DIR}
        ${imgui_fc_SOURCE_DIR}/backends
        ${imgui_fc_SOURCE_DIR}/misc/cpp
)

set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_DIR ${imgui_fc_SOURCE_DIR} CACHE PATH "Path to ImGui source for ImGui-SFML")
FetchContent_Declare(
        ImGui-SFML
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG        2.6.x
)
option(IMGUI_SFML_BUILD_EXAMPLES "Build ImGui-SFML examples" OFF)
FetchContent_MakeAvailable(ImGui-SFML)

include(FetchContent)
FetchContent_Declare(
        tiny_aes_fc
        GIT_REPOSITORY https://github.com/kokke/tiny-AES-c.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(tiny_aes_fc)

set(OpenCV_DIR "C:/tools/opencv/build" CACHE PATH "Path to OpenCV build directory")
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio imgcodecs)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Application.cpp
        src/Utils.cpp
        src/MainMenuState.cpp
        src/EncodingState.cpp
        src/LSB.cpp
        src/DecodingState.cpp
        src/Crypto.cpp
        ${tiny_aes_fc_SOURCE_DIR}/aes.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
        ${tiny_aes_fc_SOURCE_DIR}
)
target_sources(${PROJECT_NAME} PRIVATE ${tiny_aes_fc_SOURCE_DIR}/aes.c)
target_compile_definitions(${PROJECT_NAME} PRIVATE SFML_STATIC)

target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        ImGui
        ImGui-SFML
        ${OpenCV_LIBS}
)

add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)

if(WIN32 AND TARGET sfml-audio)
    set(OPENAL_DLL_PATH "${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll")

    if(EXISTS "${OPENAL_DLL_PATH}")
        add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENAL_DLL_PATH}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endif()
endif()