#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <memory>
#include "AppState.hpp"

class Application {
public:
    Application();
    ~Application();
    void run();

    void changeState(std::unique_ptr<AppState> newState);
    sf::RenderWindow& getWindow();
    ImFont* getTitleFont();
    ImFont* getHeaderFont();

private:
    void drawGlobalUi();
    void processEvents();
    void update(sf::Time dt);
    void render();

    void setupStyle();
    void setupFonts();
    void handleWindowDragging();


    sf::RenderWindow m_window;
    sf::Clock m_deltaClock;
    std::unique_ptr<AppState> m_currentState;

    ImFont* m_titleFont = nullptr;
    ImFont* m_headerFont = nullptr;

    bool m_isDraggingWindow = false;
    sf::Vector2i m_windowStartPos;
    sf::Vector2i m_mouseStartPos;

    std::unique_ptr<AppState> m_nextState;
    bool m_wantsToChangeState = false;
};