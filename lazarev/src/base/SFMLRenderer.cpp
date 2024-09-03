#include "SFMLRenderer.h"
#include "base/RenderRequests.h"

SFMLRenderer* SFMLRenderer::Create()
{
	if (!self)
		self = new SFMLRenderer();

	return self;
}

void SFMLRenderer::Destroy()
{
	if (self)
	{
		delete self;
		self = nullptr;
	}
}

SFMLRenderer* SFMLRenderer::Get()
{
	return self;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void SFMLRenderer::Init()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	m_Window = std::make_unique<sf::RenderWindow>(sf::VideoMode(1920, 1080), "Wnd", sf::Style::Default, settings);
	m_Window->setFramerateLimit(frameLimit);
	
	m_view.setSize(sf::Vector2f(m_Window->getSize()));
	m_view.setCenter(sf::Vector2f(m_Window->getSize()) / 2.f);
	m_view.zoom(1.5);

	m_Window->setView(m_view);

	SM_ASSERT(m_font.loadFromFile("c:\\Windows\\Fonts\\calibri.ttf"), "::SFMLRenderer() -> Failed to load font");
}


void SFMLRenderer::OnRender()
{
	Scheme m_scheme;

	sf::Time prevTime;
	sf::Time currTime;

	sf::Vector2f prev_mouse_pos{};
	sf::Vector2f curr_mouse_pos{};
	

	while (m_Window->isOpen())
	{
		curr_mouse_pos = sf::Vector2f(sf::Mouse::getPosition());
		currTime = m_Clock.getElapsedTime();
		
		m_frameTime = currTime.asSeconds() - prevTime.asSeconds();
		m_fps = 1.f / m_frameTime;
		delta_mouse = curr_mouse_pos - prev_mouse_pos;

		if (m_Window->hasFocus())
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				m_view.move(-(delta_mouse));
		}
		handleEvents();
		
		{
			m_scheme.DrawScheme();
		}
		m_Window->setView(m_view);
		m_Window->clear(sf::Color(200, 200, 200));
		RenderRequests::DrawAll();
		//auto start = std::chrono::high_resolution_clock::now();
		m_Window->display();
		//std::println("3 : {:.10f}\n", std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start).count());

		prev_mouse_pos = curr_mouse_pos;
		prevTime = currTime;
	}
}


void SFMLRenderer::handleEvents()
{
	while (m_Window->pollEvent(m_event))
	{
		if (m_event.type == sf::Event::Closed)
			m_Window->close();

		if (m_event.type == sf::Event::Resized)
		{
			m_view.setSize(sf::Vector2f(m_Window->getSize()));
			m_view.setCenter(sf::Vector2f(m_Window->getSize()) / 2.0f);
		}
		
		if (m_Window->hasFocus())
		{
			if (m_event.type == sf::Event::MouseWheelScrolled)
			{
				if (m_event.mouseWheelScroll.delta > 0)
					m_view.zoom(0.95);

				else if (m_event.mouseWheelScroll.delta < 0)
					m_view.zoom(1.05f);
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


sf::Event*			SFMLRenderer::get_sfEvents()	{ return &m_event;}
sf::View*			SFMLRenderer::get_sfView()		{ return &m_view;}
sf::RenderWindow*	SFMLRenderer::get_sfWindow()	{ return m_Window.get();}
sf::Font&			SFMLRenderer::get_font()		{ return m_font;}
sf::Vector2f		SFMLRenderer::GetDeltaMouse()	{ return delta_mouse; }

sf::Vector2f SFMLRenderer::GetWorldMousePos()
{
	return m_Window->mapPixelToCoords(sf::Mouse::getPosition(*m_Window), m_view);
}

