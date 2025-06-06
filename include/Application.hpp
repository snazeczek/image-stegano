#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>

class Application {
public:
    Application();
    void run();

private:
    void processEvents();
    void update(sf::Time dt);
    void render();

    void setupStyle();
    void setupFonts();
    void handleWindowDragging();
    void drawUi();
    void loadSelectedImage();

private:
    sf::RenderWindow m_window;
    sf::Clock m_deltaClock;

    sf::Texture m_imageTexture;
    bool m_imageLoaded = false;
    ImFont* m_titleFont = nullptr;

    bool m_isDraggingWindow = false;
    sf::Vector2i m_windowStartPos;
    sf::Vector2i m_mouseStartPos;
};