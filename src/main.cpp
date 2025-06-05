#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <imgui.h>
#include <imgui-SFML.h>


#include <opencv2/opencv.hpp>
#include <iostream>

int main() {

    std::cout << "SFML Version: "
              << SFML_VERSION_MAJOR << "."
              << SFML_VERSION_MINOR << std::endl;
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    std::cout << "ImGui version: " << IMGUI_VERSION << std::endl;


    sf::RenderWindow window(sf::VideoMode(1280, 720), "Cerberus - Project Running!");
    window.setFramerateLimit(60);


    if (!ImGui::SFML::Init(window, true)) {
        std::cerr << "Failed to initialize ImGui-SFML!" << std::endl;
        return -1;
    }


    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Cyan);
    shape.setPosition(350.f, 250.f);

    sf::Clock deltaClock;


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);


            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }


        ImGui::SFML::Update(window, deltaClock.restart());


        ImGui::Begin("Cerberus Control Panel");
        ImGui::Text("SFML, OpenCV, and ImGui are working together!");
        ImGui::Text("This is a major success!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        static float color[3] = { 0.f, 1.f, 0.f };
        if (ImGui::ColorEdit3("Circle Color", color)) {
            shape.setFillColor(sf::Color(static_cast<sf::Uint8>(color[0] * 255),
                                         static_cast<sf::Uint8>(color[1] * 255),
                                         static_cast<sf::Uint8>(color[2] * 255)));
        }
        ImGui::End();

        window.clear(sf::Color(30, 30, 50));

        window.draw(shape);

        ImGui::SFML::Render(window);


        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}