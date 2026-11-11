#include "RendererMap.h"
#include <iostream>

Renderer::RendererMap::RendererMap(TileType p_type, spriteSheetTexture* p_texture, spriteSheetTexture* p_infectedtexture, Texture* p_main_texture, int textureNumb, int p_cellNumber) : m_type(p_type), m_texture(p_texture), m_infectedtexture(p_infectedtexture), m_mainTextureNumb(textureNumb), m_mainTexture(p_main_texture), m_cellNumber(p_cellNumber)
{
	// Pre-creates all 9 turret sprites (3 colors x 3 upgrade levels) up front, all hidden;
	// PlaceTurret()/UpgradeTurret() just toggle which one of these is rendered.
	//Green
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(255, 192, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(255, 128, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(255, 64, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));

	//red
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(319, 192, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(319, 128, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(319, 64, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));

	//blue
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(191, 192, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(191, 128, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));
	m_turretTextures.emplace_back(m_mainTexture->useSpecificSpriteSheet(191, 64, 64, 64, Math::Vec4<int>(m_texture->m_pos_scale.x, m_texture->m_pos_scale.y, m_texture->m_pos_scale.w, m_texture->m_pos_scale.h)));


	for (auto text : m_turretTextures)
	{
		text->render = false;
	}

	m_infectedtexture->render = false;
}

void Renderer::RendererMap::PlaceTurret(int turretType)
{
	if (m_type == TileType::scenery)
	{
		m_turretTextures[turretType]->render = true;
	}
}

::Renderer::spriteSheetTexture* Renderer::RendererMap::UpgradeTurret()
{
	// Finds the currently visible turret sprite and swaps to the next entry in m_turretTextures.
	// Relies on turret sprites being grouped per color in increasing level order, and on the
	// caller never upgrading an already-maxed turret (i+1 would go out of bounds at the last level).
	for(int i = 0; i < m_turretTextures.size(); ++i)
	{
		if (m_turretTextures[i]->render == true)
		{
			m_turretTextures[i]->render = false;
			m_turretTextures[i + 1]->render = true;
			return m_turretTextures[i + 1];
		}
	}

	return nullptr;
}

void Renderer::RendererMap::Update(float deltatime)
{
	if (isInfected)
	{
		currInfectedTime += deltatime;

		if (currInfectedTime >= maxInfectedTime)
		{
			isInfected = false;
			currInfectedTime = 0;

			m_texture->render = true;
			m_infectedtexture->render = false;
		}
	}

}

void Renderer::RendererMap::infect(int damage)
{
	isInfected = true;
	infectedDamage = damage;
	currInfectedTime = 0;

	m_texture->render = false;
	m_infectedtexture->render = true;
}

void Renderer::RendererMap::checkSelectTile(int x, int y)
{
	SDL_FPoint mouse;
	mouse.x = x;
	mouse.y = y;

	const SDL_FRect& tileRect = m_texture->m_pos_scale;
	if (mouse.x >= tileRect.x && mouse.x < tileRect.x + tileRect.w &&
		mouse.y >= tileRect.y && mouse.y < tileRect.y + tileRect.h)
	{

		for (auto turretText : m_turretTextures)
		{
			//There is a turret
			if (turretText->render == true)
			{
				clickRequest = new TileClickRequest(m_cellNumber);
			}
		}
	}

}
