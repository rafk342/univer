#pragma once

#include "vendor/SFML/Graphics.hpp"
#include "base/WidgetsBase.h"
#include "base/RenderRequests.h"

class WidgetsBase;

class ImageButton : public WidgetsBase
{
    sf::IntRect InactiveSpriteRect{};
    sf::IntRect ActiveSpriteRect{};
    sf::Color hovered_color = { 212, 212, 212 };
    sf::Color unhovered_color = { 255, 255, 255 };
    bool was_pressed_over_the_button = false;
    bool was_pressed = false;

    bool ButtonBehavior();

public:
    using WidgetsBase::WidgetsBase;

    void SetInactiveImageRectSprite(const sf::IntRect& rect);
    void SetActiveImageRectSprite(const sf::IntRect& rect);
    void SetHoveredTint(sf::Color col);
    void SetUnhoveredTint(sf::Color col);

    operator bool() { return ButtonBehavior(); }
};

