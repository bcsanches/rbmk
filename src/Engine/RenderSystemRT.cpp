#include "RenderSystemRT.h"

#include <stdexcept>


namespace rbmk
{
	void RenderSystemRT::Init(int width, int height) 
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());

			throw std::runtime_error("Failed to initialize SDL");
		}

		if (!SDL_CreateWindowAndRenderer("RBMK", width, width, 0, &m_pWindow, &m_pRenderer))
		{
			SDL_Log("SDL_CreateWindowAndRenderer() failed: %s", SDL_GetError());
			
			throw std::runtime_error("Failed to create window and renderer");
		}

		m_pTexture = SDL_CreateTexture(
			m_pRenderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height
		);

		if (!m_pTexture)
		{
			SDL_Log("SDL_CreateTexture() failed: %s", SDL_GetError());
			
			throw std::runtime_error("Failed to create texture");
		}

		SDL_SetWindowRelativeMouseMode(m_pWindow, true);

		m_pPixels = std::make_unique<uint32_t[]>(width * height);
	}
}