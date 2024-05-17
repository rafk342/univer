#include <iostream>
#include <thread>
#include <functional>
#include <print>

#include "base/SFMLRenderer.h"

int main()
{
    auto* renderer = SFMLRenderer::Create();
    renderer->Init();
    renderer->OnRender();
    SFMLRenderer::Destroy();
    return 0;
}
