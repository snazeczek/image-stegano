#pragma once

#include <SFML/System/Time.hpp>

namespace sf {
    class Event;
    class RenderWindow;
}
class Application;

class AppState {
public:
    virtual ~AppState() = default;

    virtual void handleEvents(const sf::Event& event) = 0;
    virtual void update(sf::Time dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};