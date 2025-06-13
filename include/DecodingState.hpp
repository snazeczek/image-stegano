#pragma once

#include "AppState.hpp"
#include "../include/LSB.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <vector>
#include <opencv2/core/mat.hpp>

class Application;

class DecodingState : public AppState {
public:
    explicit DecodingState(Application& app);
    void handleEvents(const sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw(sf::RenderWindow& window) override;


private:
    void handleImageLoadingAndDecoding();
    void handleDecryption(const std::string& password);
    void drawPasswordPopup();
    void resetState();
    void buildStatusInfo();

    Application& m_app;
    bool m_wantsToReturnToMenu = false;
    sf::Texture m_backArrowTexture;

    // --- Zmienne stanu UI ---
    bool m_imageLoaded = false;
    std::string m_statusInfo;
    std::string m_loadedImagePath;

    // --- Dane z dekodera ---
    SteganoLSB::DecodedPayload m_decodedInfo;

    // --- Dane finalne ---
    std::vector<char> m_finalData;
    std::string m_decodedText;

    // --- Zmienne dla popupa ---
    bool m_showPasswordPopup = false;
    char m_passwordBuffer[128] = {0};
};