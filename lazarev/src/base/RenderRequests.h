#pragma once

#include <deque>
#include <functional>

#include "base/SFMLRenderer.h"
#include "vendor/SFML/Graphics.hpp"
#include "common/m_assert.h"

class RenderRequests
{
    static std::deque<std::function<void()>> Tasks;
public:

    static sf::RenderWindow* getWindow();

    static void InvokeWidgetUpdate(const std::function<void()>& delegate);
    static void DrawAll();
};
