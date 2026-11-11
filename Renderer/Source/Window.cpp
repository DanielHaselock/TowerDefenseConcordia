#include "Window.h"
#include <algorithm>

void Renderer::Window::createWindowRenderer()
{
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    m_renderer.createRenderer(m_window);
}

bool Renderer::Window::pollEvents()
{
    bool a = false;

    m_mouse.setStateChanged(false);
    m_mouse.setJustPressed(false);
    m_mouse.setJustReleased(false);

    keysPressedDown.clear();
    keysPressedUp.clear();

    while (SDL_PollEvent(&m_event) != 0)
    {
        if (m_event.type != 0)
        {
            handleEvent(m_event);
            a = true;
        }
    }
    return a;
}

void Renderer::Window::render()
{
    m_renderer.clearRenderer();

    m_renderer.setColor();

    for (::Renderer::AbstractRenderable* render : m_renderTargets)
    {
        if (render->shouldRender())
        {
            // IRenderable has no "draw color" concept, so rectangles need their fill color set on
            // the renderer before Render() is called; dynamic_cast tells us which concrete type this is.
            ::Renderer::Texture* texture = dynamic_cast<::Renderer::Texture*>(render);
            ::Renderer::Rectangle* rectangle = dynamic_cast<::Renderer::Rectangle*>(render);
            if (texture)
            {
                texture->Render(m_renderer.getRenderer());
            }
            else if (rectangle)
            {
                m_renderer.setColor(rectangle->getColor().R(), rectangle->getColor().G(), rectangle->getColor().B(), rectangle->getColor().A());

                rectangle->Render(m_renderer.getRenderer());
            }
            else
            {
                // PriceLabel, NumberLabel and any future self-colored
                // renderable sets its own draw color inside Render(), so no
                // setup is needed here.
                render->Render(m_renderer.getRenderer());
            }
        }
    }

    m_renderer.setColor();

    m_renderer.refreshRenderer();
}

void Renderer::Window::handleEvent(SDL_Event p_event)
{
    switch (p_event.type)
    {
    case SDL_QUIT:
        setQuit(true);
        break;

    case SDL_KEYDOWN:
        handleKeyboardDown(p_event.key);
        break;
    case SDL_KEYUP:
        handleKeyboardUp(p_event.key);
        break;

    case SDL_MOUSEMOTION:
        int x, y;
        SDL_GetMouseState(&x, &y);
        m_mouse.setPosition(x, y);
        m_mouse.setType(p_event.type);
        break;

    case SDL_MOUSEBUTTONDOWN:
        m_mouse.setPosition(p_event.button.x, p_event.button.y);
        m_mouse.setButton(p_event.button.button);
        m_mouse.setPressed(true);
        m_mouse.setType(p_event.type);
        m_mouse.setStateChanged(true);
        m_mouse.setJustPressed(true);
        break;

    case SDL_MOUSEBUTTONUP:
        m_mouse.setPosition(p_event.button.x, p_event.button.y);
        m_mouse.setButton(p_event.button.button);
        m_mouse.setPressed(false);
        m_mouse.setType(p_event.type);
        m_mouse.setStateChanged(true);
        m_mouse.setJustReleased(true);
        break;

    default:
        break;
    }


}

void Renderer::Window::handleKeyboardDown(SDL_KeyboardEvent p_event)
{
    SDL_Keycode key = p_event.keysym.sym;

    if (key == SDLK_ESCAPE)
    {
        setQuit(true);
    }
    
    keysPressedDown.push_back(key);
}

void Renderer::Window::handleKeyboardUp(SDL_KeyboardEvent p_event)
{
    SDL_Keycode key = p_event.keysym.sym;
    keysPressedUp.push_back(key);
}

void Renderer::Window::quit()
{
    ClearSession();
    m_renderer.quit();
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Renderer::Window::ClearSession()
{
    freeRects();
    freePriceLabels();
    freeNumberLabels();
    freeTextures();
    freeRenderTargets();
}

bool Renderer::Window::getQuit()
{
    return m_quit;
}

void Renderer::Window::setQuit(bool p_quit)
{
    m_quit = p_quit;
}

::Renderer::Texture* Renderer::Window::addTexture(std::string p_path, Math::Vec4<int> p_scale, Math::Vec3<int> p_colorToIgnore)
{
    ::Renderer::Texture* newTexture = new ::Renderer::Texture(0,0, WINDOW_WIDTH, WINDOW_HEIGHT);
    newTexture->loadTexture(p_path, m_renderer.getRenderer(), p_colorToIgnore);

    if(p_scale.GetMagnitude() > 0)
        newTexture->setScale(p_scale);

    newTexture->setBlendMode(SDL_BLENDMODE_BLEND); //alpha
    

    m_Textures.emplace_back(newTexture);
    m_renderTargets.emplace_back(newTexture);

    return newTexture;
}

void Renderer::Window::selectTexture(int numb, bool p_render)
{
    if (numb >= m_Textures.size())
    {
        return;
    }

    m_Textures[numb]->setRender(p_render);
}


::Renderer::Rectangle* Renderer::Window::addRect(Math::Vec4<int> p_scale, Math::Vec4<int> p_color)
{
    ::Renderer::Rectangle* rect = new ::Renderer::Rectangle(p_scale);
    rect->setColor(p_color.X(), p_color.Y(), p_color.Z(), p_color.W());
    m_recs.emplace_back(rect);
    m_renderTargets.emplace_back(rect);

    return rect;
}

::Renderer::PriceLabel* Renderer::Window::addPriceLabel(int x, int y, int value)
{
    ::Renderer::PriceLabel* label = new ::Renderer::PriceLabel(x, y, value);
    m_priceLabels.emplace_back(label);
    m_renderTargets.emplace_back(label);

    return label;
}

void Renderer::Window::showRect(int numb)
{
    if (numb >= m_recs.size())
    {
        return;
    }
    m_recs[numb]->setRender(true);
}

void Renderer::Window::resetRenderTargets()
{
    for (::Renderer::AbstractRenderable* render : m_renderTargets)
    {
        render->setRender(false);
    }
}

void Renderer::Window::attachRenderable(::Renderer::AbstractRenderable* renderable)
{
    if (renderable != nullptr)
    {
        m_renderTargets.emplace_back(renderable);
    }
}

void Renderer::Window::detachRenderable(::Renderer::AbstractRenderable* renderable)
{
    m_renderTargets.erase(
        std::remove(m_renderTargets.begin(), m_renderTargets.end(), renderable),
        m_renderTargets.end());
}

void Renderer::Window::freeTextures()
{
    for (auto text : m_Textures)
    {
        delete text;
    }

    m_Textures.clear();
}

void Renderer::Window::freeRects()
{
    for (auto recs : m_recs)
    {
        delete recs;
    }

    m_recs.clear();
}

void Renderer::Window::freePriceLabels()
{
    for (auto label : m_priceLabels)
    {
        delete label;
    }

    m_priceLabels.clear();
}

::Renderer::NumberLabel* Renderer::Window::addNumberLabel(int x, int y, int value, Math::Vec3<int> color)
{
    ::Renderer::NumberLabel* label = new ::Renderer::NumberLabel(x, y, value, color);
    m_numberLabels.emplace_back(label);
    m_renderTargets.emplace_back(label);

    return label;
}

::Renderer::TextLabel* Renderer::Window::addTextLabel(int x, int y, std::string text, int scale, SDL_Color color)
{
    ::Renderer::TextLabel* label = new ::Renderer::TextLabel(x, y, std::move(text), scale, color);
    m_Textures.emplace_back(label);
    m_renderTargets.emplace_back(label);
    return label;
}

::Renderer::StyledButton* Renderer::Window::addStyledButton(int x, int y, int w, int h, std::string label, ::Renderer::ButtonVariant variant)
{
    ::Renderer::StyledButton* button = new ::Renderer::StyledButton(x, y, w, h, std::move(label), variant);
    m_Textures.emplace_back(button);
    m_renderTargets.emplace_back(button);
    return button;
}

::Renderer::InspectPanel* Renderer::Window::addInspectPanel(int x, int y, int w, int h, std::string title, SDL_Color accent,
    std::vector<::Renderer::InspectPanel::Row> rows, SDL_Point decimalPointOffset)
{
    ::Renderer::InspectPanel* panel = new ::Renderer::InspectPanel(x, y, w, h, std::move(title), accent, std::move(rows), decimalPointOffset);
    m_Textures.emplace_back(panel);
    m_renderTargets.emplace_back(panel);
    return panel;
}

::Renderer::OutcomeOverlay* Renderer::Window::addOutcomeOverlay(int x, int y, int w, int h, bool won)
{
    ::Renderer::OutcomeOverlay* overlay = new ::Renderer::OutcomeOverlay(x, y, w, h, won);
    m_Textures.emplace_back(overlay);
    m_renderTargets.emplace_back(overlay);
    return overlay;
}

::Renderer::SelectionHighlight* Renderer::Window::addSelectionHighlight(int x, int y, int w, int h)
{
    ::Renderer::SelectionHighlight* highlight = new ::Renderer::SelectionHighlight(x, y, w, h);
    m_Textures.emplace_back(highlight);
    m_renderTargets.emplace_back(highlight);
    return highlight;
}

void Renderer::Window::freeNumberLabels()
{
    for (auto label : m_numberLabels)
    {
        delete label;
    }

    m_numberLabels.clear();
}

void Renderer::Window::freeRenderTargets()
{
    m_renderTargets.clear(); //vectors are deleted already
}
