#pragma once

#include <deque>
#include <functional>

#include "base/SFMLRenderer.h"
#include "vendor/SFML/Graphics.hpp"
#include "common/sm_assert.h"

class RenderRequests
{
    static inline std::vector<std::function<void()>> Tasks;
public:

    static sf::RenderWindow* getWindow();

    static void DrawInvoke(const std::function<void()>& delegate);
    static void DrawAll();
};
