#include "gaia3_pch.h"
#include "level.h"

#include <filesystem>
#include <json/json.h>

RTTI_DEFINE_RESOURCE( Level,
                      RTTI_RESOURCE_EXTENSIONS( ".level", ".lvl" ),
                      RTTI_PROPERTY( m_Entities ) )

Level::Level()
    : Resource( "", ResourceState::New )
{
}

Level::Level( std::string_view path, ResourceState state )
    : Resource( path, state )
{
}

Level::~Level()
{
	ClearEntities();
}

void Level::Update()
{
	for( auto& pEntity : m_Entities )
	{
		pEntity->Update();
	}
}

bool Level::LoadInternal()
{
	m_ComponentFactory.Clear();
	m_NextEntityId = 1;

	std::ifstream inputFile( GetPath().data() );
	if( !inputFile.is_open() )
	{
		return false;
	}

	Json::CharReaderBuilder readerBuilder;
	Json::Value             levelJson;
	std::string             parseErrors;
	if( !Json::parseFromStream( readerBuilder, inputFile, &levelJson, &parseErrors ) )
	{
		return false;
	}

	if( !rtti::Deserialize( *this, levelJson ) )
	{
		return false;
	}

	const Json::Value& entitiesJson = levelJson["m_Entities"];
	if( !entitiesJson.isArray() )
		return false;

	for( size_t i = 0; i < m_Entities.size(); ++i )
	{
		const unique<Entity>& entity = m_Entities[i];
		if( entity == nullptr )
			continue;

		entity->m_Id = m_NextEntityId++;

		entity->AttachToLevel( this );

		if( i >= entitiesJson.size() )
			continue;

		const Json::Value& entityJson               = entitiesJson[static_cast<Json::ArrayIndex>( i )];
		const Json::Value& serializedComponentsJson = entityJson["m_SerializedComponents"];
		if( !serializedComponentsJson.isArray() )
			continue;

		if( !entity->DeserializeComponents( serializedComponentsJson ) )
			return false;
	}

	return true;
}

bool Level::SaveAsInternal( std::string_view path )
{
	std::filesystem::path filePath( path );
	if( filePath.empty() )
		return false;

	std::error_code ec;
	std::filesystem::create_directories( filePath.parent_path(), ec );

	Json::Value levelJson;

	if( !rtti::Serialize( *this, levelJson ) )
		return false;

	Json::Value& entitiesJson = levelJson["m_Entities"];
	if( !entitiesJson.isArray() )
		return false;

	for( Json::ArrayIndex i = 0; i < entitiesJson.size(); ++i )
	{
		if( i >= m_Entities.size() || m_Entities[i] == nullptr )
			continue;

		Json::Value serializedComponents;
		if( !m_Entities[i]->SerializeComponents( serializedComponents ) )
			return false;

		entitiesJson[i]["m_SerializedComponents"] = std::move( serializedComponents );
	}

	std::ofstream outputFile( filePath );
	if( !outputFile.is_open() )
		return false;

	outputFile << levelJson;
	if( !outputFile.good() )
		return false;

	return true;
}

Entity* Level::CreateEntity( std::string_view entityName )
{
	unique<Entity> entity    = new_unique<Entity>( m_NextEntityId++, std::string( entityName ), this );
	Entity*        entityPtr = entity.get();
	m_Entities.push_back( std::move( entity ) );
	return entityPtr;
}

bool Level::RemoveEntity( u64 id )
{
	for( auto it = m_Entities.begin(); it != m_Entities.end(); ++it )
	{
		if( *it != nullptr && ( *it )->GetId() == id )
		{
			m_Entities.erase( it );
			return true;
		}
	}

	return false;
}

void Level::ClearEntities()
{
	m_Entities.clear();
	m_ComponentFactory.Clear();
	m_NextEntityId = 1;
}

Entity* Level::FindEntity( u64 id )
{
	for( const auto& entity : m_Entities )
	{
		if( entity != nullptr && entity->GetId() == id )
			return entity.get();
	}

	return nullptr;
}

const Entity* Level::FindEntity( u64 id ) const
{
	for( const auto& entity : m_Entities )
	{
		if( entity != nullptr && entity->GetId() == id )
			return entity.get();
	}

	return nullptr;
}
