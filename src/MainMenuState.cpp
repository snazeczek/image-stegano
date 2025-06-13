#include "../include/MainMenuState.hpp"
#include "../include/EncodingState.hpp"
#include "../include/DecodingState.hpp"
#include "../include/Application.hpp"
#include <imgui.h>
#include <iostream>

MainMenuState::MainMenuState(Application& app) : m_app(app), m_wantsToSwitchToEncoding(false) {
    std::cout << "Entering Main Menu State" << std::endl;
}

void MainMenuState::handleEvents(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            m_app.getWindow().close();
        }
    }
}

void MainMenuState::update(sf::Time dt) {
    if (m_wantsToSwitchToEncoding) {
        m_app.changeState(std::make_unique<EncodingState>(m_app));
    }
}

void MainMenuState::draw(sf::RenderWindow& window) {
    const char* titleText = "image-stegano";
    ImFont* titleFont = m_app.getTitleFont();
    if (titleFont) ImGui::PushFont(titleFont);
    float titleWidth = ImGui::CalcTextSize(titleText).x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - titleWidth) * 0.5f);
    ImGui::SetCursorPosY(100.0f);
    ImGui::Text(titleText);
    if (titleFont) ImGui::PopFont();

    const ImVec2 windowSize = ImGui::GetWindowSize();
    const char* encryptText = "Encrypt Image / Video";
    const char* decryptText = "Decrypt Image / Video";
    ImVec2 buttonSize = { 250, 40 };

    ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);
    ImGui::SetCursorPosY((windowSize.y - (buttonSize.y * 2 + 10)) * 0.5f);

    if (ImGui::Button(encryptText, buttonSize)) {
        m_wantsToSwitchToEncoding = true;
    }

    ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);
    if (ImGui::Button(decryptText, buttonSize)) {
        m_app.changeState(std::make_unique<DecodingState>(m_app));
    }
}