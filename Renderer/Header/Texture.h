#pragma once
#include "SDL.h"
#include <string>
#include "AbstractRenderable.h"
#include "Vector4.h"
#include "Vector3.h"
#include <vector>
namespace Renderer
{

	/**
	 * @class spriteSheetTexture
	 * @brief One named sub-rectangle ("sprite") cut out of a parent Texture's sprite sheet.
	 *
	 * Stores the source rectangle within the sheet image (m_spriteSheetRect)
	 * and the destination position/size/rotation (m_pos_scale, m_angle) used
	 * to draw it. A Texture owns a list of these and draws each one whose
	 * render flag is true. Created via Texture::useSpriteSheets() or
	 * Texture::useSpecificSpriteSheet(), and owned/destroyed by that Texture.
	 */
	class spriteSheetTexture
	{
	public:
		SDL_Rect* m_spriteSheetRect = nullptr; ///< Source rectangle (in sheet-image pixels) for this sprite. Owned by this object.
		SDL_FRect m_pos_scale{ 0 }; ///< Destination rectangle (x, y, w, h) where the sprite is drawn on screen.
		double m_angle = 0.0; ///< Rotation angle, in degrees, applied when rendering this sprite.
		bool render = true; ///< Whether this sprite is currently drawn by Texture::Render().

		/// Moves this sprite's destination rectangle to (x, y), keeping its current size.
		void setPosition(float x, float y);
		/// Resizes this sprite's destination rectangle to (w, h), keeping its current position.
		void setScale(int w, int h);


		/// Frees the owned source rectangle (m_spriteSheetRect).
		~spriteSheetTexture();
	};


	/**
	 * @class Texture
	 * @brief An SDL texture loaded from an image file, optionally sliced into a sprite sheet.
	 *
	 * A Texture can be drawn either as a single image filling its scale
	 * rectangle, or - once useSpriteSheets()/useSpecificSpriteSheet() has
	 * been called - as a collection of independently positioned/rotated
	 * spriteSheetTexture entries (e.g. menu icons, critters, turrets all
	 * sharing one sheet image). It owns the underlying SDL_Texture and
	 * every spriteSheetTexture it creates, freeing them in its destructor.
	 */
	class Texture : public AbstractRenderable
	{
	public:
		/// Creates a texture with the given destination rectangle (x, y, w, h); no image is loaded yet.
		Texture(int x, int y, int w, int h);

		/// Intended to copy-construct from another Texture; currently unimplemented (no-op).
		Texture(Texture* p_otherText);

		/**
		 * @brief Loads an image file from disk and uploads it as this texture's SDL_Texture.
		 *
		 * On success, also updates the destination rectangle's width/height to
		 * match the loaded image's pixel size.
		 * @param p_path Path to the image file.
		 * @param p_renderer Renderer used to create the GPU texture.
		 * @param p_colorToIgnore If not Math::Vec3<int>::error, this color is treated as transparent (color key).
		 */
		void loadTexture(std::string p_path, SDL_Renderer* p_renderer, Math::Vec3<int> p_colorToIgnore = Math::Vec3<int>::error);

		/**
		 * @brief Slices this texture's sheet image into a grid of equally-sized spriteSheetTexture entries.
		 *
		 * @param numb Total number of sprites to generate.
		 * @param distance_x Width in pixels of each cell in the sheet.
		 * @param distance_y Height in pixels of each cell in the sheet.
		 * @param numb_row_x Number of sprites per row before wrapping to the next row.
		 * @param numb_row_y Number of rows to generate.
		 * @param width Destination render width for each generated sprite.
		 * @param height Destination render height for each generated sprite.
		 */
		void useSpriteSheets(int numb, int distance_x, int distance_y, int numb_row_x, int numb_row_y, int width, int height);

		/**
		 * @brief Creates a single spriteSheetTexture for one explicit region of the sheet image.
		 *
		 * Unlike useSpriteSheets(), this carves out exactly one sprite at an
		 * arbitrary pixel location, which is how most sprites (menu icons,
		 * turrets, critters, bullets) are created throughout this module.
		 * @param pixel_x Left edge, in pixels, of the source region within the sheet.
		 * @param pixel_y Top edge, in pixels, of the source region within the sheet.
		 * @param size_x Width, in pixels, of the source region within the sheet.
		 * @param size_y Height, in pixels, of the source region within the sheet.
		 * @param scaled_position Destination rectangle (x, y, w, h) on screen.
		 * @return Pointer to the newly created spriteSheetTexture, owned by this Texture.
		 */
		spriteSheetTexture* useSpecificSpriteSheet(int pixel_x, int pixel_y, int size_x, int size_y, const Math::Vec4<int>& scaled_position);

		/// Draws either the whole texture (if no sprite sheet was set up) or every sprite whose render flag is true.
		void Render(SDL_Renderer* p_renderer) override;

		/// Enables or disables rendering of this texture, and of every one of its sprite-sheet entries.
		void setRender(bool p_render) override;

		/// Destroys the underlying SDL_Texture and clears the handle.
		void destroyTexture();

		/// Resizes the whole-texture destination rectangle to (w, h).
		void setScale(int w, int h);

		/// Sets the whole-texture destination rectangle from (x, y, w, h).
		void setScale(Math::Vec4<int> scale);

		/// Moves the whole-texture destination rectangle to (x, y).
		void setPosition(int x, int y);

		/// Returns the whole-texture destination rectangle (x, y, w, h).
		SDL_Rect getScale();

		/// Sets the texture's alpha modulation (transparency).
		void setAlpha(float p_alpha);

		/// Sets the SDL blend mode used when rendering this texture (e.g. alpha blending).
		void setBlendMode(SDL_BlendMode p_blending);

		/// Applies a color modulation (tint) to this texture.
		void setColor(Math::Vec3<int> p_color); //modulation

		/// Returns the list of spriteSheetTexture entries carved out of this texture.
		std::vector<spriteSheetTexture*> getSpriteSheet();

		/// Enables or disables rendering of a single sprite-sheet entry by index.
		void setSpecificSpriteRender(int p_spriteNumb, bool p_render);

		/// Disables rendering for every sprite-sheet entry belonging to this texture.
		void deactivateAllSpriteRender();

		/// Returns the underlying native SDL_Texture handle for low-level Renderer drawing.
		SDL_Texture* getRawTexture() const { return m_texture; }

		/// Destroys the underlying SDL_Texture and deletes every owned spriteSheetTexture.
		~Texture();

	private:
		SDL_Texture* m_texture = NULL; ///< Owned native SDL texture handle (the raw image data on the GPU).

		SDL_Rect scale{ 0 }; ///< Destination rectangle used when this texture is drawn as a single whole image.

		std::vector<spriteSheetTexture*> m_spriteSheets; ///< Sprites carved out of this texture's sheet image, owned by this Texture.
	};

}