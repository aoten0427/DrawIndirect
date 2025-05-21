/*
	@file	SceneManager.h
	@brief	シーンマネージャクラス
*/
#pragma once
#include "IScene.h"

// 前方宣言
class CommonResources;

class SceneManager
{
private:
	std::unique_ptr<IScene> m_currentScene;
	CommonResources* m_commonResources;


public:
	SceneManager();
	~SceneManager();

	void Initialize(CommonResources* resources);
	void Update(float elapsedTime);
	void Render();
	void Finalize();


private:
	void ChangeScene(IScene::SceneID sceneID);
	void CreateScene(IScene::SceneID sceneID);
	void DeleteScene();
};
