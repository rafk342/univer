#include "ImageButton.h"

bool ImageButton::ButtonBehavior()
{
    RenderRequests::InvokeWidgetUpdate([this]
        {
            RenderRequests::getWindow()->draw(m_sprite);
        });

    bool is_hovered_on_this_frame = is_hovered();

    if (is_hovered_on_this_frame) {
        m_sprite.setColor(hovered_color);
    } else {
        m_sprite.setColor(unhovered_color);
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !was_pressed)
    {
        if (is_hovered_on_this_frame)
        {
            m_sprite.setTextureRect(ActiveSpriteRect);
            was_pressed_over_the_button = true;
        }
        else
        {
            was_pressed_over_the_button = false;
        }
        was_pressed = true;
    }
    else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && was_pressed)
    {
        m_sprite.setTextureRect(InactiveSpriteRect);
        was_pressed = false;

        if (was_pressed_over_the_button && is_hovered_on_this_frame)
        {
            was_pressed_over_the_button = false;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

void ImageButton::SetInactiveImageRectSprite(const sf::IntRect& rect)
{
    m_sprite.setTextureRect(rect); 
    InactiveSpriteRect = rect;
}

void ImageButton::SetActiveImageRectSprite(const sf::IntRect& rect)
{
    ActiveSpriteRect = rect;
}

void ImageButton::SetHoveredTint(sf::Color col)
{
    hovered_color = col;
}

void ImageButton::SetUnhoveredTint(sf::Color col)
{
    unhovered_color = col;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TwoStatesButton::ButtonBehavior()
{
    RenderRequests::InvokeWidgetUpdate([this]
        {
            RenderRequests::getWindow()->draw(m_sprite);
        });

    bool is_hovered_on_this_frame = is_hovered();

    if (is_hovered_on_this_frame) {
        m_sprite.setColor(hovered_color);
    } else {
        m_sprite.setColor(unhovered_color);
    }

    if (current_state) {
        m_sprite.setTextureRect(ActiveSpriteRect);
    } else {
        m_sprite.setTextureRect(InactiveSpriteRect);
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !was_pressed)
    {
        if (is_hovered_on_this_frame)
        {
            was_pressed_over_the_button = true;
        }
        else
        {
            was_pressed_over_the_button = false;
        }
        was_pressed = true;
    }
    else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && was_pressed)
    {
        was_pressed = false;

        if (was_pressed_over_the_button && is_hovered_on_this_frame)
        {
            was_pressed_over_the_button = false;
            current_state = !current_state;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

void TwoStatesButton::SetTrueStateSpriteRect(const sf::IntRect& rect)
{
    SetActiveImageRectSprite(rect);
}

void TwoStatesButton::SetFalseStateSpriteRect(const sf::IntRect& rect)
{
    SetInactiveImageRectSprite(rect);
}


