#include "Bullet.h"
#include "Critter.h"
#include "Texture.h"
#include <iostream>

Game::Bullet::Bullet(::Renderer::spriteSheetTexture* p_texture) : m_texture(p_texture)
{
    m_texture->render = false;
}

void Game::Bullet::Shoot(int p_damage, float p_speed, int initalX, int initialY, int angle, Critter* target, int p_effect, float p_blastRadius)
{
    m_damage = p_damage;
    m_speed = p_speed;
    m_blastRadius = p_blastRadius;
    m_texture->setPosition(initalX, initialY);
	m_texture->m_angle = angle;
	m_target = target;
	m_texture->render = true;
	isActive = true;

    //0 = nothing, 1 = aoe, 2 = slow
    effect = p_effect;
}

void Game::Bullet::Update(float deltatime)
{
        if (!isActive || m_target == nullptr)
            return;

        // Bullet center
        float bulletX =
            m_texture->m_pos_scale.x;

        float bulletY =
            m_texture->m_pos_scale.y;

        // Critter center
        SDL_FRect targetRect = m_target->getTexture()->m_pos_scale;

        float targetX = targetRect.x;
        float targetY = targetRect.y;

        float dx = targetX - bulletX;
        float dy = targetY - bulletY;

        float distance = std::sqrt(dx * dx + dy * dy);

        // Avoid division by zero
        if (distance <= 1.0f)
        {
            Hit();
            return;
        }

        // Normalize direction
        float dirX = dx / distance;
        float dirY = dy / distance;

        // Move bullet
        float moveAmount = m_speed * deltatime;

        float newX = m_texture->m_pos_scale.x + dirX * moveAmount;
        float newY = m_texture->m_pos_scale.y + dirY * moveAmount;

        m_texture->setPosition(
            newX,
            newY
        );

        // Rotate bullet toward target
        constexpr float PI = 3.14159265358979323846f;

        m_texture->m_angle =
            std::atan2(dy, dx) * 180.0f / PI;

        // If your bullet sprite originally points UP:
        m_texture->m_angle += 90.0f;

        // Hit once close enough
        if (distance <= moveAmount + 5.0f)
        {
            Hit();
        }
}

void Game::Bullet::Hit()
{
    m_texture->render = false;

    if (effect == 2) //slow
    {
        m_target->takeDamage(m_damage);
        m_target->halfSpeed();
    }
    else if (effect == 1) //aoe
    {
        // No direct damage here: the splash around the impact tile covers
        // the target too, and the game loop applies it once via the
        // HitRequest below.
        hitRequest = new HitRequest(m_target->getRow(), m_target->getCol(), m_damage, m_blastRadius);
    }
    else
    {
        m_target->takeDamage(m_damage);
    }

	isActive = false;
	m_target = nullptr;
}

void Game::Bullet::DeleteHitRequest()
{
    if (hitRequest)
        delete hitRequest;

    hitRequest = nullptr;
}

Game::Bullet::~Bullet()
{
    DeleteHitRequest();
}
