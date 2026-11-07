#include "gaia3_pch.h"
#include "session.h"

bool Session::Init( std::string_view startLevel, std::string_view commonPath, std::string_view appPath )
{
	if( !m_ResourceManager.Init( commonPath, appPath ) )
		return false;

	// Note: An empty start level is a valid choice that means starting from a new level
	if( startLevel.empty() )
	{
		m_CurrentLevel = new_shared<Level>();
	}
	else if( shared<Level> levelResource = m_ResourceManager.FindResource<Level>( startLevel, true ) )
	{
		m_CurrentLevel = levelResource;
	}
	else
	{
		GAIA_WARN( "Session: start level {} is not available, using a new level", startLevel );
		m_CurrentLevel = new_shared<Level>();
	}

	return true;
}

void Session::Update()
{
	m_CurrentLevel->Update();
}

void Session::Shutdown()
{
	m_CurrentLevel.reset();
	m_ResourceManager.Shutdown();
}

void Session::NewLevel()
{
	if( m_CurrentLevel != nullptr )
	{
		m_CurrentLevel->Unload();
	}

	m_CurrentLevel = new_shared<Level>();
}

bool Session::OpenLevel( std::string_view levelName )
{
	// Note: The new level loads first, so a failed load keeps the current level running
	shared<Level> level = m_ResourceManager.FindResource<Level>( levelName, true );
	if( level == nullptr )
	{
		GAIA_ERROR( "Session: failed to open level {}", levelName );
		return false;
	}

	if( level == m_CurrentLevel )
		return true;

	// Note: A level loaded from disk stays in the ResourceManager, so unloading is what removes it from the scene
	if( m_CurrentLevel != nullptr )
	{
		m_CurrentLevel->Unload();
	}

	m_CurrentLevel = level;
	return true;
}

bool Session::SaveCurrentLevel()
{
	if( m_CurrentLevel == nullptr || m_CurrentLevel->IsNew() )
	{
		GAIA_ERROR( "Session: the current level has no file yet, save it with a name first" );
		return false;
	}

	const std::string path( m_CurrentLevel->GetPath() );
	return m_CurrentLevel->SaveAs( path );
}

bool Session::SaveCurrentLevelAs( std::string_view fileName )
{
	if( m_CurrentLevel == nullptr )
		return false;

	return m_ResourceManager.SaveResourceAs<Level>( m_CurrentLevel, fileName );
}
