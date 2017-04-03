#pragma once
#include "IScene.h"
class PlayerSceneExample :
	public IScene
{
public:
	PlayerSceneExample();
	virtual ~PlayerSceneExample();

	virtual void Tick() override;

	virtual void Init() override;

	virtual void Destroy() override;

};

