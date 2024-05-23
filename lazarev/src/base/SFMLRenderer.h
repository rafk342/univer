#pragma once
#include <iostream>
#include <print>

#include "vendor/SFML/Graphics.hpp"
#include "scheme/Scheme.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SFMLRenderer
{
    static SFMLRenderer* self;
    SFMLRenderer() = default;
    std::unique_ptr<sf::RenderWindow> m_Window;
        
    sf::View    m_view;
    sf::Font    m_font;
    sf::Event   m_event{};
    unsigned    frameLimit = 120;
    sf::Clock   m_Clock;
    float       m_frameTime = 0;
    float       m_fps = 0;
  
    sf::Vector2f delta_mouse;
    sf::Vector2f delta_mouse_in_world;

    void handleEvents();

public:

    static SFMLRenderer*    Create();
    static SFMLRenderer*    Get();
    static void             Destroy();

    void Init();
    void OnRender();
    
    sf::RenderWindow*  get_sfWindow();
    sf::Event*         get_sfEvents();
    sf::View*          get_sfView();
    sf::Font&          get_font();
    sf::Vector2f       GetDeltaMouse();
    sf::Vector2f       GetWorldDeltaMouse();
    sf::Vector2f       GetWorldMousePos();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define m_SFMLRenderer (*SFMLRenderer::Get())

