#include "RenderWorld.h"

#include "Model.h"

namespace rbmk
{
	RenderEntity::RenderEntity(std::unique_ptr<Model> &&model):
		m_upModel{ std::move(model) },
		m_v3Origin{ 0, 0, 0 },
		m_v3Axis{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1}}
	{				
		//empty
	}

	RenderWorld::RenderWorld()
	{
		//empty
	}

	void RenderWorld::AddLocalModel(std::unique_ptr<Model> model)
	{
		m_vecLocalModels.push_back(std::move(model));
	}
}
