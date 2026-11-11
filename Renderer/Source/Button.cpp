#include "Button.h"

Renderer::Button::Button()
{
}

Renderer::Button::Button(Texture* texture)
{
    m_currentTexture = texture;
    m_listTextures.emplace_back(texture);
}

Renderer::Button::~Button()
{
    m_listTextures.clear();
}

void Renderer::Button::setPosition(int x, int y)
{
    m_x = x;
    m_y = y;

    if (m_useSpriteSheet)
    {
        for (spriteSheetTexture* text : m_currentTexture->getSpriteSheet())
        {
            text->setPosition(m_x, m_y);
        }
    }
    else
    {
        for (Renderer::Texture* text : m_listTextures)
        {
            text->setPosition(m_x, m_y);
        }
    }
}

void Renderer::Button::setScale(int w, int h)
{
    m_w = w;
    m_h = h;

    if (m_useSpriteSheet)
    {
        for (spriteSheetTexture* text : m_currentTexture->getSpriteSheet())
        {
            text->setScale(m_w, m_h);
        }
    }
    else
    {
        for (Renderer::Texture* text : m_listTextures)
        {
            text->setScale(m_w, m_h);
        }
    }
}

void Renderer::Button::handlePress()
{
    //Get mouse position
    int x, y;
    SDL_GetMouseState(&x, &y);
        
    bool inside = false;

    if (m_useSpriteSheet)
        inside = checkSpriteSheetTexture(x, y);
    else
        inside = checkRegularTexture(x,y);

    changebutton(inside);
}

void Renderer::Button::changebutton(bool inside)
{
    if (inside)
    {
        m_pressed = true;
    }
}

void Renderer::Button::changeTexture()
{
    if (m_useSpriteSheet)
    {
        m_currentTexture->deactivateAllSpriteRender();
        m_currentTexture->setSpecificSpriteRender(mCurrentSprite, true);
    }
    else
    {
        if (mCurrentSprite >= m_listTextures.size())
            return;

        for (Texture* text : m_listTextures)
        {
            text->setRender(false);
        }
   
        m_listTextures[mCurrentSprite]->setRender(true);
        setTexture(m_listTextures[mCurrentSprite]);
    }
}

void Renderer::Button::addTexture(Texture* p_newTexture)
{
    m_listTextures.emplace_back(p_newTexture);

    if(!m_currentTexture)
        m_currentTexture = p_newTexture;

    if (m_useSpriteSheet)
    {
        for (spriteSheetTexture* text : m_currentTexture->getSpriteSheet())
        {
            text->setPosition(m_x, m_y);
            text->setScale(m_w, m_h);
        }
    }
    else
    {
        p_newTexture->setPosition(m_x, m_y);
        p_newTexture->setScale(m_w, m_h);
    }
}

bool Renderer::Button::checkSpriteSheetTexture(int x, int y)
{
    //Check if mouse is in button
    bool inside = true;

    //Mouse is left of the button
    if (x < m_currentTexture->getSpriteSheet()[mCurrentSprite]->m_pos_scale.x)
    {
        inside = false;
    }
    //Mouse is right of the button
    else if (x > m_currentTexture->getSpriteSheet()[mCurrentSprite]->m_pos_scale.x + m_currentTexture->getSpriteSheet()[mCurrentSprite]->m_pos_scale.w)
    {
        inside = false;
    }
    //Mouse above the button
    else if (y < m_currentTexture->getSpriteSheet()[mCurrentSprite]->m_pos_scale.y)
    {
        inside = false;
    }
    //Mouse below the button
    else if (y > m_currentTexture->getSpriteSheet()[mCurrentSprite]->m_pos_scale.y + m_currentTexture->getSpriteSheet()[mCurrentSprite]->m_pos_scale.h)
    {
        inside = false;
    }
    return inside;
}

bool Renderer::Button::checkRegularTexture(int x, int y)
{
    //Check if mouse is in button
    bool inside = true;

    //Mouse is left of the button
    if (x < m_currentTexture->getScale().x)
    {
        inside = false;
    }
    //Mouse is right of the button
    else if (x > m_currentTexture->getScale().x + m_currentTexture->getScale().w)
    {
        inside = false;
    }
    //Mouse above the button
    else if (y < m_currentTexture->getScale().y)
    {
        inside = false;
    }
    //Mouse below the button
    else if (y > m_currentTexture->getScale().y + m_currentTexture->getScale().h)
    {
        inside = false;
    }
    return inside;
}

void Renderer::Button::setTexture(Texture* p_text)
{
    if(p_text)
        m_currentTexture = p_text;
}

void Renderer::Button::setUseSpriteSheet(bool p_sprite)
{
    m_useSpriteSheet = p_sprite;
}

void Renderer::Button::setButtonActive(bool p_active)
{
    m_active = p_active;

    m_currentTexture->setRender(p_active);

    if (m_useSpriteSheet)
    {
        m_currentTexture->setSpecificSpriteRender(mCurrentSprite, true);
    }
    else
    {
        m_currentTexture->setRender(false);
        m_currentTexture = m_listTextures[mCurrentSprite];
        m_currentTexture->setRender(p_active);
    }
}

Renderer::Texture* Renderer::Button::getCurrentTexture()
{
    return m_currentTexture;
}
