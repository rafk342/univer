#include "RenderRequests.h"

std::deque<std::function<void()>> RenderRequests::Tasks;

sf::RenderWindow* RenderRequests::getWindow()
{
    return m_SFMLRenderer.get_sfWindow();
}

void RenderRequests::InvokeWidgetUpdate(const std::function<void()>& delegate)
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

