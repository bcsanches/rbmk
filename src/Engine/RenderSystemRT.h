#pragma once

#include <memory>

#include "RenderSystem.h"

#include <SDL3/SDL.h>

namespace rbmk
{

	class RenderSystemRT : public RenderSystem
	{
		public:
			RenderSystemRT() = default;

			void Init(int width, int height);

		private:
			SDL_Window		*m_pWindow = nullptr;
			SDL_Renderer	*m_pRenderer = nullptr;
			SDL_Texture		*m_pTexture = nullptr;

			std::unique_ptr<uint32_t[]> m_pPixels;
	};
}

