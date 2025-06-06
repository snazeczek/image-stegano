#include "Application.hpp"
#include <imgui-SFML.h>
#include <opencv2/opencv.hpp>
#include <iostream>

#define NOMINMAX
#include <windows.h>

std::string openFileDialog(HWND owner) {
    OPENFILENAMEA ofn;
    CHAR szFile[MAX_PATH] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }
    return std::string();
}


Application::Application()
    : m_window(sf::VideoMode({400, 600}), "Project Cerberus", sf::Style::None)
{
    m_window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(m_window, false)) {
        throw std::runtime_error("Error during ImGui-SFML initialization");
    }

    setupFonts();
    setupStyle();

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    m_window.setPosition({(int)(desktop.width - m_window.getSize().x) / 2,
                          (int)(desktop.height - m_window.getSize().y) / 2});
}

void Application::run() {
    while (m_window.isOpen()) {
        processEvents();
        update(m_deltaClock.restart());
        render();
    }
    ImGui::SFML::Shutdown();
}

void Application::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(m_window, event);
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }
    }
}

void Application::update(sf::Time dt) {
    ImGui::SFML::Update(m_window, dt);
}

void Application::render() {
    m_window.clear(sf::Color(30, 30, 50, 200));
    drawUi();
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void Application::drawUi() {
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin("MainCanvas", NULL, window_flags);

    handleWindowDragging();

    const char* titleText = "video-image-stegano";
    if (m_titleFont) ImGui::PushFont(m_titleFont);
    float titleWidth = ImGui::CalcTextSize(titleText).x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - titleWidth) * 0.5f);
    ImGui::SetCursorPosY(20.0f);
    ImGui::Text(titleText);
    if (m_titleFont) ImGui::PopFont();

    ImGui::SameLine(ImGui::GetWindowWidth() - 50.0f);
    ImGui::SetCursorPosY(20.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    if (ImGui::Button("X", ImVec2(30, 20))) {
        m_window.close();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    if (m_imageLoaded) {
        ImGui::Text("Image Preview:");
        ImVec2 availableSize = ImGui::GetContentRegionAvail();
        availableSize.y -= 80.0f;
        float aspectRatio = (float)m_imageTexture.getSize().y / (float)m_imageTexture.getSize().x;
        float imageWidth = availableSize.x;
        float imageHeight = availableSize.x * aspectRatio;
        if (imageHeight > availableSize.y) {
            imageHeight = availableSize.y;
            imageWidth = imageHeight / aspectRatio;
        }
        ImGui::Image(m_imageTexture, {imageWidth, imageHeight});
    }

    const char* buttonText = "Load Image...";
    ImGuiStyle& style = ImGui::GetStyle();
    float buttonWidth = ImGui::CalcTextSize(buttonText).x + style.FramePadding.x * 2.0f;
    float buttonHeight = ImGui::GetFrameHeight();
    float posX = (ImGui::GetWindowSize().x - buttonWidth) * 0.5f;
    float posY = ImGui::GetWindowSize().y - buttonHeight - 50.0f;
    ImGui::SetCursorPos(ImVec2(posX, posY));
    if (ImGui::Button(buttonText)) {
        loadSelectedImage();
    }

    ImGui::End();
}

void Application::loadSelectedImage() {
    std::string filePath = openFileDialog(m_window.getSystemHandle());
    if (!filePath.empty()) {
        cv::Mat image = cv::imread(filePath);
        if (!image.empty()) {
            std::cout << "Loaded image: " << filePath << std::endl;
            cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
            m_imageTexture.create(image.cols, image.rows);
            m_imageTexture.update(image.ptr());
            m_imageLoaded = true;
        } else {
            std::cerr << "Failed to load image: " << filePath << std::endl;
            m_imageLoaded = false;
        }
    } else {
        std::cout << "File selection canceled." << std::endl;
    }
}


void Application::handleWindowDragging() {
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        m_isDraggingWindow = true;
        m_windowStartPos = m_window.getPosition();
        m_mouseStartPos = sf::Mouse::getPosition();
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_isDraggingWindow = false;
    }
    if (m_isDraggingWindow) {
        sf::Vector2i mouseDelta = sf::Mouse::getPosition() - m_mouseStartPos;
        m_window.setPosition(m_windowStartPos + mouseDelta);
    }
}


void Application::setupFonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("resources/Roboto-Regular.ttf", 16.0f);
    m_titleFont = io.Fonts->AddFontFromFileTTF("resources/Roboto-Regular.ttf", 24.0f);
    if (!ImGui::SFML::UpdateFontTexture()) {
        std::cerr << "Failed to update font texture" << std::endl;
    }
}

void Application::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.28f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.47f, 0.32f, 0.78f, 0.70f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.40f, 0.86f, 0.80f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.45f, 0.90f, 0.90f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.47f, 0.32f, 0.78f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.40f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.45f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 4.0f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 6.0f);
}