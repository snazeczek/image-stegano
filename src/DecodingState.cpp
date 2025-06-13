#include "../include/DecodingState.hpp"
#include "../include/MainMenuState.hpp"
#include "../include/Crypto.hpp"
#include "Application.hpp"
#include "Utils.hpp"

#include <imgui_stdlib.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>

DecodingState::DecodingState(Application& app) : m_app(app) {
    if (!m_backArrowTexture.loadFromFile("resources/arrow_back.png")) {
        std::cerr << "Failed to load back arrow texture!" << std::endl;
    }
    handleImageLoadingAndDecoding();
}

void DecodingState::handleEvents(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        m_wantsToReturnToMenu = true;
    }
}

void DecodingState::update(sf::Time dt) {
    if (m_wantsToReturnToMenu) {
        m_app.changeState(std::make_unique<MainMenuState>(m_app));
    }
}

void DecodingState::drawPasswordPopup() {
    if (m_showPasswordPopup) {
        ImGui::OpenPopup("PasswordPopupDecode");
        m_showPasswordPopup = false;
    }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.07f, 0.1f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.3f, 0.2f, 0.5f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.3f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.4f, 0.7f, 1.0f));
    if (ImGui::BeginPopupModal("PasswordPopupDecode", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Enter the password to decrypt the data");
        ImGui::Separator();
        ImGui::InputTextWithHint("##Password", "Password", m_passwordBuffer, sizeof(m_passwordBuffer), ImGuiInputTextFlags_Password);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        if (ImGui::Button("Decrypt", ImVec2(ImGui::GetContentRegionAvail().x - 120, 0))) {
            if (strlen(m_passwordBuffer) > 0) {
                handleDecryption(m_passwordBuffer);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(-1, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(4);
}

void DecodingState::draw(sf::RenderWindow& window) {
    drawPasswordPopup();

    const char* titleText = "Decoding Mode";
    ImFont* titleFont = m_app.getTitleFont();
    if (titleFont) ImGui::PushFont(titleFont);
    float titleWidth = ImGui::CalcTextSize(titleText).x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - titleWidth) * 0.5f);
    ImGui::SetCursorPosY(20.0f);
    ImGui::Text(titleText);
    if (titleFont) ImGui::PopFont();

    ImGui::SetCursorPos({20.0f, 20.0f});
    ImGui::PushStyleColor(ImGuiCol_Button, {0.1f, 0.8f, 0.2f, 0.7f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.2f, 0.9f, 0.3f, 0.8f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.1f, 0.7f, 0.15f, 0.8f});
    if (ImGui::Button("##backButton", {30, 20})) {
        m_wantsToReturnToMenu = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine(0,0);
    ImGui::SetCursorPos({20.0f + 7, 20.0f + 3});
    ImGui::Image(m_backArrowTexture, {15, 15});

    if (!m_imageLoaded) {
        ImVec2 windowSize = ImGui::GetWindowSize();
        const char* initText = "Select an image to decode data from.";
        ImVec2 textSize = ImGui::CalcTextSize(initText);
        ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.4f));
        ImGui::Text(initText);

        ImVec2 buttonSize = {300, 40};
        ImGui::SetCursorPos(ImVec2((windowSize.x - buttonSize.x) * 0.5f, (windowSize.y - buttonSize.y) * 0.5f));
        if (ImGui::Button("Select Image...", buttonSize)) {
            handleImageLoadingAndDecoding();
        }
    }
    else {
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 boxSize = {windowSize.x * 0.8f, 300.0f};
        float posX = (windowSize.x - boxSize.x) * 0.5f;
        float posY = (windowSize.y  - boxSize.y/2)*0.45f;
        ImGui::SetCursorPos(ImVec2(posX, posY));

        if (ImGui::BeginChild("CenteredResultBox", boxSize, false, ImGuiWindowFlags_None))
        {
            float contentWidth = ImGui::GetContentRegionAvail().x;
            ImGui::Separator();
            ImGui::Spacing();

            std::stringstream statusStream;
            if (!m_decodedInfo.isValid) {
                statusStream << "No hidden data found in the image.";
            }
            else {
                if (m_decodedInfo.type == SteganoLSB::DataType::Text) {
                    ImGui::SetCursorPosY(0);
                    statusStream << "Data type: Text\n";

                    if (m_decodedInfo.isEncrypted) {
                    } else {
                        statusStream << "Status: Data is not encrypted.";
                    }

                    if (!m_decodedText.empty()) {
                        ImGui::Spacing();
                        const char* contentLabel = "Decoded content:";
                        ImVec2 contentLabelSize = ImGui::CalcTextSize(contentLabel);
                        ImGui::SetCursorPosX((contentWidth - contentLabelSize.x) * 0.5f);
                        ImGui::TextUnformatted(contentLabel);

                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.2f));
                        ImGui::InputTextMultiline("##decoded_text_output", &m_decodedText,
                                                  ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 7),
                                                  ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopStyleColor();
                    }

                }
                else if (m_decodedInfo.type == SteganoLSB::DataType::File) {
                    statusStream << "Data type: File\n";
                    statusStream << "Extension: " << m_decodedInfo.fileExtension << "\n";
                }
                statusStream << "Size: " << m_decodedInfo.data.size() << " bytes\n\n";

                if (m_decodedInfo.isEncrypted) {
                    statusStream << "Status: Data is encrypted.";
                }
                else {
                    statusStream << "Status: Data is not encrypted.";
                }
            }

            std::string statusText = statusStream.str();
            std::stringstream lineStream(statusText);
            std::string line;
            while (std::getline(lineStream, line, '\n'))
            {
                if (!line.empty()) {
                    ImVec2 lineSize = ImGui::CalcTextSize(line.c_str());
                    ImGui::SetCursorPosX((contentWidth - lineSize.x) * 0.5f);
                    ImGui::TextUnformatted(line.c_str());
                }
                else {
                    ImGui::Spacing();
                }
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = (contentWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0f;
            float buttonHeight = 35.0f;

            bool canSave = !m_finalData.empty() && m_decodedInfo.type == SteganoLSB::DataType::File;
            ImGui::BeginDisabled(!canSave);
                if (ImGui::Button("Save File As...", ImVec2(buttonWidth, buttonHeight))) {
                    std::string defaultName = "recovered_file" + m_decodedInfo.fileExtension;
                    std::string fileDescription = "File (" + m_decodedInfo.fileExtension + ")";
                    std::string filePattern = "*" + m_decodedInfo.fileExtension;
                    std::string filter = fileDescription + '\0' + filePattern + '\0' + "All Files (*.*)" + '\0' + "*.*" + '\0';

                    std::string savePath = saveFileDialog(m_app.getWindow().getSystemHandle(), filter.c_str(), defaultName.c_str());

                    if (!savePath.empty()) {
                        if (!std::filesystem::path(savePath).has_extension()) {
                            savePath += m_decodedInfo.fileExtension;
                        }

                        if (writeBufferToFile(savePath, m_finalData)) {
                        } else {
                        }
                    }
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::Button("Load Another Image", ImVec2(buttonWidth, buttonHeight))) {
                handleImageLoadingAndDecoding();
            }
        }
        ImGui::EndChild();
    }
}

void DecodingState::resetState() {
    m_imageLoaded = false;
    m_statusInfo.clear();
    m_loadedImagePath.clear();
    m_finalData.clear();
    m_decodedText.clear();
    m_decodedInfo = {};
    memset(m_passwordBuffer, 0, sizeof(m_passwordBuffer));
}

void DecodingState::buildStatusInfo() {
    if (!m_decodedInfo.isValid) {
        m_statusInfo = "No hidden data found in this image (missing 'magic number').";
        return;
    }
    char buffer[256];
    if (m_decodedInfo.type == SteganoLSB::DataType::Text) {
        snprintf(buffer, sizeof(buffer), "Data type: Text | Size: %zu bytes.", m_decodedInfo.data.size());
    } else if (m_decodedInfo.type == SteganoLSB::DataType::File) {
        snprintf(buffer, sizeof(buffer), "Data type: File | Extension: '%s' | Size: %zu bytes.",
                 m_decodedInfo.fileExtension.c_str(), m_decodedInfo.data.size());
    }
    m_statusInfo = buffer;

    if (m_decodedInfo.isEncrypted) {
        m_statusInfo += "\nStatus: Encrypted data detected! Enter the password to continue.";
    } else {
        m_statusInfo += "\nStatus: Unencrypted data detected. Ready to save/read.";
    }
}

void DecodingState::handleImageLoadingAndDecoding() {
    resetState();

    std::string filePath = openFileDialog(m_app.getWindow().getSystemHandle());
    if (filePath.empty()) { return; }

    m_loadedImagePath = filePath;
    cv::Mat encodedImage = cv::imread(filePath);
    if (encodedImage.empty()) {
        m_statusInfo = "Failed to load the image or the file is corrupted.";
        m_imageLoaded = true;
        return;
    }

    m_decodedInfo = SteganoLSB::decode(encodedImage);
    m_imageLoaded = true;
    buildStatusInfo();

    if (m_decodedInfo.isValid && !m_decodedInfo.isEncrypted) {
        m_finalData = m_decodedInfo.data;
        if (m_decodedInfo.type == SteganoLSB::DataType::Text) {
            m_decodedText.assign(m_finalData.begin(), m_finalData.end());
        }
    } else if (m_decodedInfo.isValid && m_decodedInfo.isEncrypted) {
        m_showPasswordPopup = true;
    }
}

void DecodingState::handleDecryption(const std::string& password) {
    if (password.empty() || m_decodedInfo.data.empty()) return;

    m_statusInfo = "Decrypting...";
    m_finalData = Crypto::decryptAES(m_decodedInfo.data, password);

    if (m_finalData.empty()) {
        m_statusInfo = "ERROR: Decryption failed! The password may be incorrect or the data is corrupt.";
    } else {
        buildStatusInfo();
        m_statusInfo += "\nStatus: Decryption successful! Data is ready to be saved/read.";
        if (m_decodedInfo.type == SteganoLSB::DataType::Text) {
            m_decodedText.assign(m_finalData.begin(), m_finalData.end());
        }
    }
}