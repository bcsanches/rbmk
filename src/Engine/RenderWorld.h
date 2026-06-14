#pragma once

#include <vector>
#include <memory>

#include <Math/rbVec3.h>

namespace rbmk
{
	class Model;

	class RenderEntity
	{
		private:
			std::unique_ptr<Model>	m_upModel;

			Math::Vec3				m_v3Origin;
			Math::Vec3				m_v3Axis[3];

		public:
			RenderEntity(std::unique_ptr<Model> &&model);
	};

	class RenderWorld
	{
		public:
			RenderWorld();

			void AddLocalModel(std::unique_ptr<Model> model);

		private:
			std::vector<std::unique_ptr<Model>> m_vecLocalModels;
	};
}


