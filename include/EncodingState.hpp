#pragma once

#include "AppState.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <future>
#include <atomic>
#include <memory>

namespace cv {
    class Mat;
}

class EncodingState : public AppState {
public:
    enum class EncodeMode {
        None,
        Text,
        File
    };
    explicit EncodingState(Application& app);
    ~EncodingState();

    void handleEvents(const sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw(sf::RenderWindow& window) override;
private:
    void drawPasswordPopup();

    void handleImageLoading();
    void handleSteganography();

    Application& m_app;

    EncodeMode m_currentMode = EncodeMode::None;

    std::string m_secretFilePath;
    std::string m_inputFilePath;
    std::string m_secretMessage;
    std::string m_statusMessage;

    size_t m_steganoCapacityBytes = 0;

    sf::Texture m_imageTexture;
    sf::Texture m_backArrowTexture;

    bool m_wantsToReturnToMenu = false;
    std::unique_ptr<cv::Mat> m_originalImage;
    bool m_imageLoaded = false;
    bool encrypt_payload = false;
    bool m_fileSizeError = false;
    bool m_encodingSucceeded = false;

    std::future<bool> m_encodingFuture;
    std::atomic<bool> m_isEncoding = false;
    std::atomic<float> m_encodingProgress = 0.0f;

    bool m_showPasswordPopup = false;
    char m_passwordBuffer[128] = {0};
    char m_passwordConfirmBuffer[128] = {0};
    std::string m_passwordError;
    std::string m_finalPassword;




};