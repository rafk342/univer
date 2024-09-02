#include <iostream>
#include <thread>
#include <functional>
#include <print>

#include "base/SFMLRenderer.h"

//void* operator new(size_t sz) noexcept
//{
//	static size_t count = 0;
//	count++;
//	std::cout << " i : " << count << " alloc : " << sz << std::endl;
//	return malloc(sz);
//}

int main()
{
    SFMLRenderer* renderer = SFMLRenderer::Create();
    renderer->Init();
    renderer->OnRender();
    SFMLRenderer::Destroy();
    return 0;
}


