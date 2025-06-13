#include "Application.hpp"
#include "../include/MainMenuState.hpp"
#include <imgui-SFML.h>
#include <iostream>

Application::Application()
    : m_window(sf::VideoMode({400, 600}), "Project Cerberus", sf::Style::None),
      m_isDraggingWindow(false)
{
    m_window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(m_window, false)) {
        throw std::runtime_error("Error during ImGui-SFML initialization");
    }

    setupFonts();
    setupStyle();

    m_currentState = std::make_unique<MainMenuState>(*this);

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    m_window.setPosition({(int)(desktop.width - m_window.getSize().x) / 2,
                          (int)(desktop.height - m_window.getSize().y) / 2});
}

Application::~Application() {
    ImGui::SFML::Shutdown();
}

void Application::run() {
    while (m_window.isOpen()) {
        if (m_wantsToChangeState) {
            m_currentState = std::move(m_nextState);
            m_wantsToChangeState = false;

            ImGui::GetIO().ClearInputKeys();
        }

        processEvents();
        update(m_deltaClock.restart());
        render();
    }
}

void Application::processEvents() {
    sf::Event event{};
    while (m_window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(m_window, event);
        if (m_currentState) {
            m_currentState->handleEvents(event);
        }
        if (event.type == sf::Event::Closed) {
            m_window.close();
        }
    }
}

void Application::update(sf::Time dt) {
    ImGui::SFML::Update(m_window, dt);

    if (m_currentState) {
        m_currentState->update(dt);
    }
}

void Application::render() {
    m_window.clear(sf::Color(30, 30, 50));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize((sf::Vector2f)m_window.getSize());

    ImGui::Begin("MainCanvas", nullptr, window_flags);

    handleWindowDragging();

    if (m_currentState) {
        m_currentState->draw(m_window);
    }
    drawGlobalUi();
    ImGui::End();

    ImGui::SFML::Render(m_window);
    m_window.display();
}

void Application::changeState(std::unique_ptr<AppState> newState) {
    if (newState) {
        m_nextState = std::move(newState);
        m_wantsToChangeState = true;
    }
}
sf::RenderWindow& Application::getWindow() {
    return m_window;
}

ImFont* Application::getTitleFont() {
    return m_titleFont;
}

void Application::drawGlobalUi() {
    const float buttonWidth = 30.0f;
    const float buttonHeight = 20.0f;
    const float margin = 20.0f;

    ImGui::SetCursorPosX(ImGui::GetWindowSize().x - buttonWidth- 18.0f);
    ImGui::SetCursorPosY(20.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    if (ImGui::Button("X", {buttonWidth, buttonHeight})) {
        m_window.close();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}


void Application::handleWindowDragging() {
    const float titleBarHeight = 60.0f;

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if ( !ImGui::IsAnyItemHovered()) {
            m_isDraggingWindow = true;
            m_windowStartPos = m_window.getPosition();
            m_mouseStartPos = sf::Mouse::getPosition();
        }
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
    m_headerFont = io.Fonts->AddFontFromFileTTF("resources/Roboto-Regular.ttf", 17.0f);
    if (!m_headerFont) {
        throw std::runtime_error("Failed to load header font");
    }
    m_titleFont = io.Fonts->AddFontFromFileTTF("resources/Roboto-Regular.ttf", 24.0f);
    if (!ImGui::SFML::UpdateFontTexture()) {
        throw std::runtime_error("Failed to update font texture");
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
ImFont* Application::getHeaderFont() {
    return m_headerFont;
}