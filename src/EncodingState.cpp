#include "../include/EncodingState.hpp"
#include "../include/MainMenuState.hpp"
#include "Application.hpp"
#include "Utils.hpp"
#include "../include/Crypto.hpp"
#include <imgui.h>
#include <imgui-SFML.h>
#include <imgui_stdlib.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include "../include/LSB.hpp"
#include <fstream>

std::vector<char> readFileToBuffer(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Could not open file for reading: " << filePath << std::endl;
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        return buffer;
    }

    std::cerr << "Error while reading file: " << filePath << std::endl;
    return {};
}
EncodingState::EncodingState(Application& app) : m_app(app), m_wantsToReturnToMenu(false), m_isEncoding(false), m_encodingProgress(0.0f){
    std::cout << "[DEBUG] Creating EncodingState object... Address: " << this << std::endl;

    if (!m_backArrowTexture.loadFromFile("resources/arrow_back.png")) {
        std::cerr << "Failed to load back arrow texture!" << std::endl;
    }

    handleImageLoading();
    if (!m_imageLoaded)
    {
        m_wantsToReturnToMenu = true;
    }

}

EncodingState::~EncodingState() {
    std::cout << "[DEBUG] Destroying EncodingState object... Address: " << this << std::endl;
    if (m_encodingFuture.valid()) {
        m_encodingFuture.wait();
    }
    std::cout << "[DEBUG] EncodingState object destroyed." << std::endl;
}

void EncodingState::handleEvents(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            m_app.changeState(std::make_unique<MainMenuState>(m_app));
        }
    }
}

void EncodingState::update(sf::Time dt) {
    if (m_wantsToReturnToMenu) {
        m_app.changeState(std::make_unique<MainMenuState>(m_app));
    }
    if (m_isEncoding && m_encodingFuture.valid()) {
        auto status = m_encodingFuture.wait_for(std::chrono::seconds(0));
        if (status == std::future_status::ready) {
            m_encodingFuture.get();
            m_isEncoding = false;
        }
    }
}
void EncodingState::drawPasswordPopup() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 0));

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,ImGui::GetStyle().Colors[ImGuiCol_Button]);
    if (ImGui::BeginPopupModal("PasswordPopup", &m_showPasswordPopup, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::InputTextWithHint("##Password", "Password", m_passwordBuffer, sizeof(m_passwordBuffer), ImGuiInputTextFlags_Password);
        ImGui::InputTextWithHint("##PasswordConfirm", "Confirm password", m_passwordConfirmBuffer, sizeof(m_passwordConfirmBuffer), ImGuiInputTextFlags_Password);

        if (!m_passwordError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            ImGui::TextWrapped("%s", m_passwordError.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2;

        if (ImGui::Button("Encrypt and Save", ImVec2(buttonWidth, 0))) {
            if (strlen(m_passwordBuffer) == 0) {
                m_passwordError = "Password cannot be empty.";
            } else if (strcmp(m_passwordBuffer, m_passwordConfirmBuffer) != 0) {
                m_passwordError = "Passwords do not match.";
            } else {
                m_passwordError.clear();
                m_finalPassword = m_passwordBuffer;
                m_showPasswordPopup = false;
                ImGui::CloseCurrentPopup();
                handleSteganography();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            m_passwordError.clear();
            m_showPasswordPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();

    }
    ImGui::PopStyleColor();
}


void EncodingState::draw(sf::RenderWindow& window)
{
    drawPasswordPopup();

    auto validateSecretFileSizeLambda = [this]() {
        if (m_secretFilePath.empty()) { m_fileSizeError = false; return; }
        try {
            uintmax_t fileSize = std::filesystem::file_size(m_secretFilePath);
            m_fileSizeError = (fileSize > m_steganoCapacityBytes);
        } catch (const std::filesystem::filesystem_error& e) {
            m_fileSizeError = true;
            m_statusMessage = "Error reading file to hide!";
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    };

    const char* titleText = "Encoding Mode";
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
        if (m_isEncoding || m_encodingSucceeded) {
            m_wantsToReturnToMenu = true;
        } else if (m_currentMode == EncodeMode::File || m_currentMode == EncodeMode::Text) {
            m_currentMode = EncodeMode::None;
            m_secretFilePath.clear();
            m_secretMessage.clear();
            m_statusMessage.clear();
        } else {
            m_wantsToReturnToMenu = true;
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine(0,0);
    ImGui::SetCursorPos({20.0f + 7, 20.0f + 3});
    ImGui::Image(m_backArrowTexture, {15, 15});

    if (!m_imageLoaded) return;

    if (m_isEncoding || !m_statusMessage.empty())
    {
        float windowWidth = ImGui::GetWindowWidth();
        float columnWidth = windowWidth * 0.75f;
        float columnStartX = (windowWidth - columnWidth) * 0.5f;

        ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y * 0.25f));
        ImGui::SetCursorPosX(columnStartX);

        ImGui::BeginGroup();
        ImGui::PushItemWidth(columnWidth);

        if (m_isEncoding) {
            ImGui::Text("Encoding in progress...");
            const ImVec4 purpleColor = {0.4f, 0.3f, 0.6f, 1.0f};
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, purpleColor);
            ImGui::ProgressBar(m_encodingProgress, ImVec2(columnWidth, 0.0f));
            ImGui::Text("%.0f%%", m_encodingProgress * 100.0f);
            ImGui::PopStyleColor();
        }
        else if (!m_statusMessage.empty()) {
            ImGui::TextWrapped("Status: %s", m_statusMessage.c_str());

            if (m_encodingSucceeded) {
                ImGui::Dummy(ImVec2(0.0f, 20.0f));
                ImGui::PushStyleColor(ImGuiCol_Button, {0.1f, 0.7f, 0.2f, 1.0f});
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.2f, 0.8f, 0.3f, 1.0f});
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.1f, 0.6f, 0.15f, 1.0f});
                if (ImGui::Button("Back to Main Menu", ImVec2(columnWidth, 35))) {
                    m_wantsToReturnToMenu = true;
                }
                ImGui::PopStyleColor(3);
            }
        }
        ImGui::PopItemWidth();
        ImGui::EndGroup();
    }
    else
    {
        if (m_currentMode == EncodeMode::None) {
            const float topBarBottomY = 60.0f;
            const float bottomPadding = 20.0f;
            const float horizontalPadding = 20.0f;
            const char* previewLabel = "Image preview:";
            const float previewLabelHeight = ImGui::CalcTextSize(previewLabel).y + ImGui::GetStyle().ItemSpacing.y;
            float previewAreaWidth = ImGui::GetWindowWidth() - (horizontalPadding * 2.0f);
            float previewAreaHeight = ImGui::GetWindowHeight() * 0.4f - bottomPadding;
            float totalBlockHeight = previewAreaHeight + previewLabelHeight;
            float blockStartY = ImGui::GetWindowHeight() - bottomPadding - totalBlockHeight;
            float availableCenterSpace = blockStartY - topBarBottomY;
            const float minDisplayDimension = 150.0f;

            float nativeWidth = (float)m_imageTexture.getSize().x;
            float nativeHeight = (float)m_imageTexture.getSize().y;

            float displayWidth = nativeWidth;
            float displayHeight = nativeHeight;

            if (displayWidth < minDisplayDimension && displayHeight < minDisplayDimension)
            {
                float aspectRatio = nativeHeight / nativeWidth;
                if (displayWidth >= displayHeight) {
                    displayWidth = minDisplayDimension;
                    displayHeight = minDisplayDimension * aspectRatio;
                } else {
                    displayHeight = minDisplayDimension;
                    displayWidth = minDisplayDimension / aspectRatio;
                }
            }

            if (displayWidth > previewAreaWidth || displayHeight > previewAreaHeight) {
                float currentAspectRatio = displayHeight / displayWidth;
                displayWidth = previewAreaWidth;
                displayHeight = previewAreaWidth * currentAspectRatio;
                if (displayHeight > previewAreaHeight) {
                    displayHeight = previewAreaHeight;
                    displayWidth = displayHeight / currentAspectRatio;
                }
            }

            ImGui::SetCursorPos({horizontalPadding, blockStartY});
            ImGui::Text(previewLabel);

            float imagePosX = horizontalPadding + (previewAreaWidth - displayWidth) * 0.5f;
            float imagePosY = blockStartY + previewLabelHeight + (previewAreaHeight - displayHeight) * 0.5f;

            ImGui::SetCursorPos({imagePosX, imagePosY});
            ImGui::Image(m_imageTexture, {displayWidth, displayHeight});

            ImFont* headerFont = m_app.getHeaderFont();
            if (headerFont) ImGui::PushFont(headerFont);
            const char* choiceText = "Choose what you want to hide in the image:";
            ImVec2 buttonSize = {210, 35};
            float choiceTextHeight = ImGui::CalcTextSize(choiceText).y;
            float totalControlsHeight = choiceTextHeight + ImGui::GetStyle().ItemSpacing.y * 2 + buttonSize.y * 2;

            float controlsStartY = topBarBottomY + (availableCenterSpace - totalControlsHeight) * 0.5f;

            ImGui::SetCursorPos({0, controlsStartY-20});
            float choiceTextWidth = ImGui::CalcTextSize(choiceText).x;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - choiceTextWidth) * 0.5f);
            ImGui::Text(choiceText);
            if (headerFont) ImGui::PopFont();

            ImGui::Spacing();

            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonSize.x) * 0.5f);
            if (ImGui::Button("Text Message", buttonSize)) {
                m_currentMode = EncodeMode::Text;
            }

            ImGui::Spacing();

            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonSize.x) * 0.5f);
            if (ImGui::Button("File", buttonSize)) {
                m_currentMode = EncodeMode::File;
                std::string selectedPath = openFileDialog(m_app.getWindow().getSystemHandle(), false);
                if (!selectedPath.empty()) {
                    m_secretFilePath = selectedPath;
                    validateSecretFileSizeLambda();
                    m_currentMode = EncodeMode::File;
                }

            }
            ImGui::Spacing();
            ImGui::Spacing();
            std::string line1 = "Image capacity:";
            std::string line2;
            std::string line3;
            const long long megabyte = 1024 * 1024;
            const int kilobyte = 1024;
            char buffer[64];
            if (m_steganoCapacityBytes >= megabyte) {
                snprintf(buffer, sizeof(buffer), "%zu characters", m_steganoCapacityBytes);
                line2 = buffer;
                snprintf(buffer, sizeof(buffer), "%.2f MB", (m_steganoCapacityBytes) / (float)megabyte);
                line3 = buffer;
            }
            else if (m_steganoCapacityBytes >= kilobyte) {
                snprintf(buffer, sizeof(buffer), "%zu characters", m_steganoCapacityBytes);
                line2 = buffer;
                snprintf(buffer, sizeof(buffer), "%.2f KB", m_steganoCapacityBytes / (float)kilobyte);
                line3 = buffer;
            }
            else {snprintf(buffer, sizeof(buffer), "%zu characters / Bytes", m_steganoCapacityBytes );
                line2 = buffer;
            }

            float windowWidth = ImGui::GetWindowWidth();

            float textWidth1 = ImGui::CalcTextSize(line1.c_str()).x;
            ImGui::SetCursorPosX((windowWidth - textWidth1) * 0.5f);
            ImGui::TextUnformatted(line1.c_str());

            float textWidth2 = ImGui::CalcTextSize(line2.c_str()).x;
            ImGui::SetCursorPosX((windowWidth - textWidth2) * 0.5f);
            ImGui::TextUnformatted(line2.c_str());

            if (!line3.empty()) {
                float textWidth3 = ImGui::CalcTextSize(line3.c_str()).x;
                ImGui::SetCursorPosX((windowWidth - textWidth3) * 0.5f);
                ImGui::TextUnformatted(line3.c_str());
            }
        }
        else if (m_currentMode == EncodeMode::Text) {
            float windowWidth = ImGui::GetWindowWidth();
            float columnWidth = windowWidth * 0.7f;
            float columnStartX = (windowWidth - columnWidth) * 0.5f;

            ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y * 0.10f));
            const char* modeText = "Mode: Hiding Text";
            float modeTextWidth = ImGui::CalcTextSize(modeText).x;
            ImGui::SetCursorPosX((windowWidth - modeTextWidth) * 0.5f);
            ImGui::TextUnformatted(modeText);
            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            char capacityText[128];
            size_t usableCapacity = m_steganoCapacityBytes - 15;
            snprintf(capacityText, sizeof(capacityText), "Image capacity: %zu bytes (Entered: %zu)", usableCapacity, m_secretMessage.length());
            float capacityTextWidth = ImGui::CalcTextSize(capacityText).x;
            ImGui::SetCursorPosX((windowWidth - capacityTextWidth) * 0.5f);
            ImGui::TextUnformatted(capacityText);

            if (m_secretMessage.length() > usableCapacity) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                ImGui::TextUnformatted("Message is too long!");
                ImGui::PopStyleColor();
            }

            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SetCursorPosX(columnStartX);

            ImGui::BeginGroup();
            {
                ImGui::PushItemWidth(columnWidth);
                ImGui::InputTextMultiline("##SecretMessage", &m_secretMessage, ImVec2(columnWidth, ImGui::GetTextLineHeight() * 8));
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::Checkbox("Encrypt", &encrypt_payload);
                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                bool isButtonDisabled = m_isEncoding || (m_secretMessage.length() > usableCapacity) || m_secretMessage.empty();
                ImGui::BeginDisabled(isButtonDisabled);

                if (ImGui::Button("Hide Text and Save", ImVec2(columnWidth, 35))) {
                    if (encrypt_payload) {
                        m_passwordError.clear();
                        memset(m_passwordBuffer, 0, sizeof(m_passwordBuffer));
                        memset(m_passwordConfirmBuffer, 0, sizeof(m_passwordConfirmBuffer));
                        m_showPasswordPopup = true;
                        ImGui::OpenPopup("PasswordPopup");
                    } else {
                        m_finalPassword.clear();
                        handleSteganography();
                    }
                }
                ImGui::EndDisabled();
                ImGui::PopItemWidth();
            }
            ImGui::EndGroup();
        }
        else if (m_currentMode == EncodeMode::File) {
            float windowWidth = ImGui::GetWindowWidth();
            float columnWidth = windowWidth * 0.75f;
            float columnStartX = (windowWidth - columnWidth) * 0.5f;

            ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y * 0.10f));
            const char* modeText = "Mode: Hiding File";
            float modeTextWidth = ImGui::CalcTextSize(modeText).x;
            ImGui::SetCursorPosX((windowWidth - modeTextWidth) * 0.5f);
            ImGui::TextUnformatted(modeText);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SetCursorPosX(columnStartX);

            ImGui::BeginGroup();
            {
                ImGui::PushItemWidth(columnWidth);
                if (ImGui::Button("Select another file to hide...", ImVec2(columnWidth, 35))) {
                    std::string selectedPath = openFileDialog(m_app.getWindow().getSystemHandle(), false);
                    if (!selectedPath.empty()) {
                        m_secretFilePath = selectedPath;
                        validateSecretFileSizeLambda();
                    }
                }

                if (!m_secretFilePath.empty()) {
                    ImGui::Dummy(ImVec2(0.0f, 15.0f));
                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0.0f, 15.0f));

                    ImGui::Text("Selected file:");
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.7f, 1.0f, 1.0f));
                    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + columnWidth);
                    ImGui::TextWrapped("%s", m_secretFilePath.c_str());
                    ImGui::PopTextWrapPos();
                    ImGui::PopStyleColor();
                    ImGui::Dummy(ImVec2(0.0f, 15.0f));

                    if (m_fileSizeError) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                        char buffer[256];
                        uintmax_t fileSize = 0;
                        try { fileSize = std::filesystem::file_size(m_secretFilePath); } catch(...) {}
                        snprintf(buffer, sizeof(buffer),
                                 "ERROR: File is too large (%.2f KB)! \nMaximum image capacity is %.2f KB.",
                                 (double)fileSize / 1024.0,
                                 (double)m_steganoCapacityBytes / 1024.0);
                        ImGui::TextWrapped("%s", buffer);
                        ImGui::PopStyleColor();
                        ImGui::Dummy(ImVec2(0.0f, 15.0f));
                    }

                    ImGui::Separator();
                    ImGui::Dummy(ImVec2(0.0f, 15.0f));

                    bool isButtonDisabled = m_isEncoding || m_fileSizeError;
                    ImGui::BeginDisabled(isButtonDisabled);
                    ImGui::Checkbox("Encrypt", &encrypt_payload);
                    ImGui::Dummy(ImVec2(0.0f, 10.0f));
                    if (ImGui::Button("Hide File and Save", ImVec2(columnWidth, 35))) {
                         if (encrypt_payload) {
                            m_passwordError.clear();
                            memset(m_passwordBuffer, 0, sizeof(m_passwordBuffer));
                            memset(m_passwordConfirmBuffer, 0, sizeof(m_passwordConfirmBuffer));
                            m_showPasswordPopup = true;
                             ImGui::OpenPopup("PasswordPopup");
                        } else {
                            m_finalPassword.clear();
                            handleSteganography();
                        }
                    }
                    ImGui::EndDisabled();
                }
                ImGui::PopItemWidth();
            }
            ImGui::EndGroup();
        }
    }
}

void EncodingState::handleImageLoading()
{
    std::string filePath = openFileDialog(m_app.getWindow().getSystemHandle());

    m_secretFilePath = filePath;
    try {
        uintmax_t fileSize = std::filesystem::file_size(m_secretFilePath);
        if (fileSize > m_steganoCapacityBytes) {
            m_fileSizeError = true;
        } else {
            m_fileSizeError = false;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        m_fileSizeError = true;
        m_statusMessage = "Error: Cannot read file size. " + std::string(e.what());
    }

    if (!filePath.empty()) {
        m_inputFilePath = filePath;
        m_originalImage = std::make_unique<cv::Mat>(cv::imread(filePath));

        if (m_originalImage && !m_originalImage->empty()) {
            cv::Mat displayImage;
            cv::cvtColor(*m_originalImage, displayImage, cv::COLOR_BGR2RGBA);

            if (!m_imageTexture.create(displayImage.cols, displayImage.rows)) {
                std::cerr << "Failed to create texture for the image!" << std::endl;
                m_imageLoaded = false;
                m_app.changeState(std::make_unique<MainMenuState>(m_app));
                return;
            }
            m_imageTexture.update(displayImage.ptr());
            m_imageLoaded = true;

            m_steganoCapacityBytes = SteganoLSB::getCapacity(*m_originalImage) -15;
            std::cout << "Loaded image: " << m_inputFilePath << " | Capacity: " << m_steganoCapacityBytes << " bytes" << std::endl;
            m_currentMode = EncodeMode::None;
            m_secretMessage.clear();
            m_secretFilePath.clear();
        } else {
            std::cerr << "Failed to load image: " << filePath << std::endl;
            m_imageLoaded = false;
            m_originalImage.reset();
            m_app.changeState(std::make_unique<MainMenuState>(m_app));
        }
    } else
    {
        std::cout << "File selection canceled." << std::endl;
        m_app.changeState(std::make_unique<MainMenuState>(m_app));
    }
}

void EncodingState::handleSteganography() {
    if (!m_imageLoaded || !m_originalImage || m_isEncoding) return;

    m_isEncoding = true;
    m_encodingProgress = 0.0f;
    m_statusMessage.clear();
    m_encodingSucceeded = false;

    auto imageCopyPtr = std::make_unique<cv::Mat>(m_originalImage->clone());

    EncodeMode mode = m_currentMode;
    std::string message = m_secretMessage;
    std::string secretFile = m_secretFilePath;
    std::string inputFile = m_inputFilePath;
    size_t capacity = m_steganoCapacityBytes;

    bool encrypt = encrypt_payload;
    std::string password = m_finalPassword;

    std::filesystem::path inputPath(inputFile);
    std::string newFilename = inputPath.stem().string() + "_encoded.png";
    std::filesystem::path outputPath = inputPath.parent_path() / newFilename;

    m_encodingFuture = std::async(std::launch::async,
        [this, image = std::move(imageCopyPtr), mode, message, secretFile, capacity, outputPath, encrypt, password]() -> bool {

            std::vector<char> payload;
            SteganoLSB::DataType dataType;
            std::string fileExtension;

            if (mode == EncodeMode::Text) {
                if (message.empty()) {
                    this->m_statusMessage = "Error: Text message is empty.";
                    return false;
                }
                payload.assign(message.begin(), message.end());
                dataType = SteganoLSB::DataType::Text;
                fileExtension = ".txt";
            } else if (mode == EncodeMode::File) {
                if (secretFile.empty()) {
                    this->m_statusMessage = "Error: No file selected to hide.";
                    return false;
                }
                payload = readFileToBuffer(secretFile);
                dataType = SteganoLSB::DataType::File;
                fileExtension = std::filesystem::path(secretFile).extension().string();
            }

            if (payload.empty()) {
                this->m_statusMessage = "CRITICAL ERROR: Failed to load data (file is empty or a read error occurred).";
                return false;
            }

            if (encrypt) {
                if (password.empty()) {
                    this->m_statusMessage = "Internal error: Encryption enabled, but password is empty.";
                    return false;
                }
                try {
                    this->m_statusMessage = "Encrypting data...";
                    std::cout << "[DEBUG] Before encryption. Data size: " << payload.size() << std::endl;
                    payload = Crypto::encryptAES(payload, password);
                    this->m_statusMessage = "Encryption finished.";
                     std::cout << "[DEBUG] After encryption. New data size: " << payload.size() << std::endl;
                } catch (const std::exception& e) {
                    this->m_statusMessage = "Critical error during encryption: " + std::string(e.what());
                    return false;
                }
            }

            if (payload.size() > capacity) {
                this->m_statusMessage = "Error: Data is too large for this image!";
                return false;
            }

            auto progressCallback = [this](float progress) {
                this->m_encodingProgress = progress;
            };

            cv::Mat encodedImage;
            if (SteganoLSB::encode(*image, payload, dataType, fileExtension, encodedImage, progressCallback, encrypt)) {
                if (cv::imwrite(outputPath.string(), encodedImage)) {
                    this->m_statusMessage = "Success! \nSaved file: " + outputPath.string();
                    this->m_encodingSucceeded = true;
                    return true;
                } else {
                    this->m_statusMessage = "Critical error: Failed to save the file!";
                }
            } else {
                this->m_statusMessage = "Critical error: Encoding failed.";
            }

            m_encodingSucceeded = false;
            return false;
    });
}