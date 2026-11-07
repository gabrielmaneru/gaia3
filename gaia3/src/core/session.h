#pragma once

#include <core/types.h>
#include <scene/level.h>
#include <system/resourceManager.h>

#include <string_view>

class Session
{
public:
	bool Init( std::string_view startLevel, std::string_view commonPath, std::string_view appPath );
	void Update();
	void Shutdown();

	bool   OpenLevel( std::string_view levelName );
	bool   SaveCurrentLevel();
	bool   SaveCurrentLevelAs( std::string_view fileName );

	Level* GetCurrentLevel() { return m_CurrentLevel.get(); }
	ResourceManager& GetResourceManager() { return m_ResourceManager; }

private:
	ResourceManager m_ResourceManager;
	shared<Level>   m_CurrentLevel = nullptr;
};
