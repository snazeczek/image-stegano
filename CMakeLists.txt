cmake_minimum_required(VERSION 3.20)
project(video_image_stegano)

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
        GIT_SHALLOW    ON
        EXCLUDE_FROM_ALL
        SYSTEM
)
set(SFML_STATIC_LIBRARIES TRUE)
option(SFML_BUILD_AUDIO "Build audio" ON)
option(SFML_BUILD_NETWORK "Build network" OFF)
option(SFML_BUILD_EXAMPLES "Build SFML examples" OFF)
option(SFML_BUILD_DOC "Build SFML documentation" OFF)
FetchContent_MakeAvailable(SFML)


set(IMGUI_VERSION 1.89.9)
FetchContent_Declare(
        imgui_fc
        URL "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip"
)
FetchContent_GetProperties(imgui_fc)
if(NOT imgui_fc_POPULATED)
    FetchContent_Populate(imgui_fc)
endif()

add_library(ImGui STATIC
        ${imgui_fc_SOURCE_DIR}/imgui.cpp
        ${imgui_fc_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_fc_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_fc_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_fc_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_fc_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)
target_include_directories(ImGui PUBLIC
        ${imgui_fc_SOURCE_DIR}
        ${imgui_fc_SOURCE_DIR}/backends
)


set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_DIR ${imgui_fc_SOURCE_DIR} CACHE PATH "Path to ImGui source for ImGui-SFML")
FetchContent_Declare(
        ImGui-SFML
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG        2.6.x
        GIT_SHALLOW    ON
        EXCLUDE_FROM_ALL
        SYSTEM
)
option(IMGUI_SFML_BUILD_EXAMPLES "Build ImGui-SFML examples" OFF)
FetchContent_MakeAvailable(ImGui-SFML)



set(OpenCV_DIR "C:/tools/opencv/build" CACHE PATH "Path to OpenCV build directory containing OpenCVConfig.cmake")

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio imgcodecs)


add_executable(${PROJECT_NAME} src/main.cpp)


target_include_directories(${PROJECT_NAME} PRIVATE
        ${SFML_INCLUDE_DIR}

        ${OpenCV_INCLUDE_DIRS}
)


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


if(WIN32 AND TARGET sfml-audio)
    set(OPENAL_DLL_PATH "${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll")
    if(EXISTS "${OPENAL_DLL_PATH}")
        add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OPENAL_DLL_PATH}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMENT "Copying OpenAL DLL"
        )
    else()
    endif()
endif()