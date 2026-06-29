#pragma once
#include "scene.h"

class Game : public Scene
{
public:
	
	void Init()override;
	void Update()override;
	void Uninit()override;
	void CreateGameObject(std::string str, Vector3 pos, Vector3 scale, Vector3 rot)override;
	void LoadObj();
	void SaveObj()override;
private:
	class Audio* m_BGM;
	bool isFirst;

	int m_checkPointNum = 0;

};

