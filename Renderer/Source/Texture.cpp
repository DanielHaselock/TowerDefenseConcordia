#include "Texture.h"
#include "SDL_image.h"

Renderer::Texture::Texture(int x, int y, int w, int h)
{
    scale.x = x;
    scale.y = y;
    scale.w = w;
    scale.h = h;
}

Renderer::Texture::Texture(Texture* p_otherText)//TODO
{
    //scale.x = p_otherText->scale.x;
    //scale.y = p_otherText->scale.y;
    //scale.w = p_otherText->scale.w;
    //scale.h = p_otherText->scale.h;

    //m_texture = 

}

void Renderer::Texture::loadTexture(std::string p_path, SDL_Renderer* p_renderer, Math::Vec3<int> p_colorToIgnore)
{
    if (p_renderer == nullptr)
    {
        return;
    }

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(p_path.c_str());
    if (loadedSurface == nullptr)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", p_path.c_str(), IMG_GetError());
    }
    else
    {
        if(p_colorToIgnore != Math::Vec3<int>::error)
            SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, p_colorToIgnore.X(), p_colorToIgnore.Y(), p_colorToIgnore.Z()));

        m_texture = SDL_CreateTextureFromSurface(p_renderer, loadedSurface);
        if (m_texture == nullptr)
        {
            printf("Unable to create texture from %s! SDL Error: %s\n", p_path.c_str(), SDL_GetError());
        }

        scale.w = loadedSurface->w;
        scale.h = loadedSurface->h;

        SDL_FreeSurface(loadedSurface);
    }
}

void Renderer::Texture::useSpriteSheets(int numb, int distance_x, int distance_y, int numb_row_x, int numb_row_y, int width, int height)
{
    int tempX = 0, tempY = 0;
    int numbX = 0, numbY = 0;

    // Walks the sheet left-to-right in distance_x steps, wrapping to a new row (advancing by
    // distance_y) once numb_row_x sprites have been placed on the current row.
    for (int i = 0; i < numb; ++i)
    {
        SDL_Rect* rect = new SDL_Rect{ 0,0, distance_x,distance_y };

        if (numbX < numb_row_x)
        {
            numbX++;
            rect->x = tempX;
            rect->y = tempY;
            tempX += distance_x;
        }
        else if (numbY < numb_row_y)
        {
            numbY++;
            numbX = 0;
            tempY += distance_y;
            tempX = 0;


            numbX++;
            rect->x = tempX;
            rect->y = tempY;
            tempX += distance_x;
        }

        spriteSheetTexture* sprite = new spriteSheetTexture();
        sprite->m_spriteSheetRect = rect;
        sprite->setScale(width, height);
        m_spriteSheets.emplace_back(sprite);
    }
}

Renderer::spriteSheetTexture* Renderer::Texture::useSpecificSpriteSheet(int pixel_x, int pixel_y, int size_x, int size_y, const Math::Vec4<int>& scaled_position)
{
    SDL_Rect* rect = new SDL_Rect{ pixel_x,pixel_y, size_x,size_y };
    spriteSheetTexture* sprite = new spriteSheetTexture();
    sprite->m_spriteSheetRect = rect;
    sprite->setPosition(scaled_position.X(), scaled_position.Y());
    sprite->setScale(scaled_position.Z(), scaled_position.W());
    m_spriteSheets.emplace_back(sprite);

    return sprite;
}

void Renderer::Texture::Render(SDL_Renderer* p_renderer)
{
    // With no sprites defined, draw the whole loaded image into `scale`; otherwise draw each
    // sprite individually (rotated by its own angle) and skip any sprite currently hidden.
    if(m_spriteSheets.size() == 0)
        SDL_RenderCopy(p_renderer, m_texture, NULL, &scale);
    else
    {
        for (int i = 0; i < m_spriteSheets.size(); ++i)
        {
            if (m_spriteSheets[i]->render)
            {
                SDL_RenderCopyExF(
                    p_renderer,
                    m_texture,
                    m_spriteSheets[i]->m_spriteSheetRect,
                    &m_spriteSheets[i]->m_pos_scale,
                    m_spriteSheets[i]->m_angle,
                    nullptr,              
                    SDL_FLIP_NONE
                );
            }
        }
    }
}

void Renderer::Texture::setRender(bool p_render)
{
    m_shouldRender = p_render;

    if (m_spriteSheets.size() > 0)
    {
        for (Renderer::spriteSheetTexture* text : m_spriteSheets)
        {
            text->render = p_render;
        }
    }
}

void Renderer::Texture::destroyTexture()
{
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
}

void Renderer::Texture::setScale(int w, int h)
{
    scale.w = w;
    scale.h = h;
}

void Renderer::Texture::setScale(Math::Vec4<int> p_scale)
{
    scale.x = p_scale.X();
    scale.y = p_scale.Y();
    scale.w = p_scale.Z();
    scale.h = p_scale.W();
}

void Renderer::Texture::setPosition(int x, int y)
{
    scale.x = x;
    scale.y = y;
}

SDL_Rect Renderer::Texture::getScale()
{
    return scale;
}

void Renderer::Texture::setAlpha(float p_alpha)
{
    SDL_SetTextureAlphaMod(m_texture, p_alpha);
}

void Renderer::Texture::setBlendMode(SDL_BlendMode p_blending)
{
    SDL_SetTextureBlendMode(m_texture, p_blending);
}

void Renderer::Texture::setColor(Math::Vec3<int> p_color)
{
    SDL_SetTextureColorMod(m_texture, p_color.X(), p_color.Y(), p_color.Z());
}

std::vector<Renderer::spriteSheetTexture*> Renderer::Texture::getSpriteSheet()
{
    return m_spriteSheets;
}

void Renderer::Texture::setSpecificSpriteRender(int p_spriteNumb, bool p_render)
{
    if (m_spriteSheets.size() == 0 || p_spriteNumb >= m_spriteSheets.size())
        return;

    m_spriteSheets[p_spriteNumb]->render = p_render;
}

void Renderer::Texture::deactivateAllSpriteRender()
{
    if (m_spriteSheets.size() == 0)
        return;

    for (spriteSheetTexture* sprite : m_spriteSheets)
    {
        sprite->render = false;
    }
}

Renderer::Texture::~Texture()
{
    destroyTexture();

    for (auto it : m_spriteSheets)
    {
        delete it;
    }

    m_spriteSheets.clear();
}

void Renderer::spriteSheetTexture::setPosition(float x, float y)
{
    m_pos_scale.x = x;
    m_pos_scale.y = y;
}

void Renderer::spriteSheetTexture::setScale(int w, int h)
{
    m_pos_scale.w = w;
    m_pos_scale.h = h;
}

Renderer::spriteSheetTexture::~spriteSheetTexture()
{
    delete(m_spriteSheetRect);
}
