#include "gaia3_pch.h"
#include "session.h"

bool Session::Init( std::string_view startLevel, std::string_view commonPath, std::string_view appPath )
{
	if( !m_ResourceManager.Init( commonPath, appPath ) )
		return false;

	if( shared<Level> levelResource = m_ResourceManager.FindResource<Level>( startLevel, true ) )
	{
		m_CurrentLevel = levelResource;
	}
	else
	{
		m_CurrentLevel = new_shared<Level>();
	}

	return true;
}

void Session::Update()
{
	// check for reloading and next level
}

void Session::Shutdown()
{
	m_CurrentLevel.reset();
	m_ResourceManager.Shutdown();
}

bool Session::SaveCurrentLevelAs( std::string_view fileName )
{
	if( m_CurrentLevel == nullptr )
		return false;

	return m_ResourceManager.SaveResourceAs<Level>( m_CurrentLevel, fileName );
}
