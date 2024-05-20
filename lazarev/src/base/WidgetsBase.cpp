#include "SFMLRenderer.h"
#include "WidgetsBase.h"

WidgetsBase::WidgetsBase(const std::string& path)
{
    loadImageFromFile(path);
}

void WidgetsBase::loadImageFromFile(const std::string& path)
{
    SM_ASSERT(m_texture.loadFromFile(path), "::WidgetsBase() Couldn't load image from the given file");
    m_texture.generateMipmap();
    m_sprite.setTexture(m_texture);
}

bool WidgetsBase::is_hovered()
{
    auto* pWnd = m_SFMLRenderer.get_sfWindow();
    auto* pView = m_SFMLRenderer.get_sfView();

    if (!pWnd || !pView)
        return false;

    sf::Vector2f mouseWorldPos = pWnd->mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(*pWnd)), *pView);
    return m_sprite.getGlobalBounds().contains(mouseWorldPos);
}

sf::Texture&     WidgetsBase::GetTexture()   { return m_texture; }
sf::Sprite&      WidgetsBase::GetSprite()    { return m_sprite; }

void WidgetsBase::SetPosition(const sf::Vector2f& pos) { m_sprite.setPosition(pos); }


