#include "RenderRequests.h"

sf::RenderWindow* RenderRequests::getWindow()
{
    return g_SFMLRenderer.get_sfWindow();
}

void RenderRequests::DrawInvoke(const std::function<void()>& delegate)
{
    Tasks.push_back(delegate);
}

void RenderRequests::DrawAll()
{
    if (Tasks.empty())
        return;

    for (auto& frameTask : Tasks)
    {
        frameTask();
    }
    Tasks.clear();
}

