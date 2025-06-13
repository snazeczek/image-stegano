#pragma once
#include "AppState.hpp"
#include <memory>

class MainMenuState : public AppState {
public:
    explicit MainMenuState(Application& app);

    void handleEvents(const sf::Event& event) override;
    void update(sf::Time dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    Application& m_app;
    bool m_wantsToSwitchToEncoding = false;
};