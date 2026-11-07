#include "gaia3_pch.h"
#include "level.h"

#include <filesystem>
#include <json/json.h>

RTTI_DEFINE_RESOURCE( Level,
                      RTTI_RESOURCE_FOLDER( "levels" ),
                      RTTI_RESOURCE_EXTENSIONS( ".level", ".lvl" ),
                      RTTI_PROPERTY( m_Entities ) )

namespace
{
// Note: Turns "Crate (1)" into "Crate", any name without a number at the end stays the same
std::string_view RemoveCloneNumber( std::string_view name )
{
	const size_t numberStart = name.rfind( " (" );
	if( numberStart == std::string_view::npos || numberStart == 0 || name.back() != ')' )
		return name;

	const std::string_view digits   = name.substr( numberStart + 2, name.size() - numberStart - 3 );
	const bool             isNumber = !digits.empty() && std::all_of( digits.begin(), digits.end(), []( char character ) {
		return character >= '0' && character <= '9';
	} );

	return isNumber ? name.substr( 0, numberStart ) : name;
}
} // namespace

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
		GAIA_ERROR( "Level {}: failed to open {}", GetName(), GetPath() );
		return false;
	}

	Json::CharReaderBuilder readerBuilder;
	Json::Value             levelJson;
	std::string             parseErrors;
	if( !Json::parseFromStream( readerBuilder, inputFile, &levelJson, &parseErrors ) )
	{
		GAIA_ERROR( "Level {}: invalid JSON: {}", GetName(), parseErrors );
		return false;
	}

	if( !rtti::Deserialize( *this, levelJson ) )
	{
		GAIA_ERROR( "Level {}: invalid level data", GetName() );
		return false;
	}

	const Json::Value& entitiesJson = levelJson["m_Entities"];
	if( !entitiesJson.isArray() )
	{
		GAIA_ERROR( "Level {}: missing entity list", GetName() );
		return false;
	}

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
		{
			GAIA_ERROR( "Level {}: failed to restore the components of entity {}", GetName(), entity->GetName() );
			return false;
		}
	}

	GAIA_INFO( "Level {}: {} entities", GetName(), m_Entities.size() );
	for( const unique<Entity>& entity : m_Entities )
	{
		if( entity != nullptr )
			GAIA_INFO( "  Entity {}: {} components", entity->GetName(), entity->GetComponents().size() );
	}

	return true;
}

bool Level::UnloadInternal()
{
	ClearEntities();
	return true;
}

bool Level::SaveAsInternal( std::string_view path )
{
	std::filesystem::path filePath( path );
	if( filePath.empty() )
	{
		GAIA_ERROR( "Level: cannot save without a path" );
		return false;
	}

	std::error_code ec;
	std::filesystem::create_directories( filePath.parent_path(), ec );

	Json::Value levelJson;

	if( !rtti::Serialize( *this, levelJson ) )
	{
		GAIA_ERROR( "Level {}: failed to serialize level data", GetName() );
		return false;
	}

	Json::Value& entitiesJson = levelJson["m_Entities"];
	if( !entitiesJson.isArray() )
	{
		GAIA_ERROR( "Level {}: missing entity list", GetName() );
		return false;
	}

	for( Json::ArrayIndex i = 0; i < entitiesJson.size(); ++i )
	{
		if( i >= m_Entities.size() || m_Entities[i] == nullptr )
			continue;

		Json::Value serializedComponents;
		if( !m_Entities[i]->SerializeComponents( serializedComponents ) )
		{
			GAIA_ERROR( "Level {}: failed to serialize the components of entity {}", GetName(), m_Entities[i]->GetName() );
			return false;
		}

		entitiesJson[i]["m_SerializedComponents"] = std::move( serializedComponents );
	}

	std::ofstream outputFile( filePath );
	if( !outputFile.is_open() )
	{
		GAIA_ERROR( "Level: failed to open {} for writing", path );
		return false;
	}

	outputFile << levelJson;
	if( !outputFile.good() )
	{
		GAIA_ERROR( "Level: failed to write {}", path );
		return false;
	}

	return true;
}

Entity* Level::CreateEntity( std::string_view entityName )
{
	unique<Entity> entity    = new_unique<Entity>( m_NextEntityId++, std::string( entityName ), this );
	Entity*        entityPtr = entity.get();
	m_Entities.push_back( std::move( entity ) );
	return entityPtr;
}

Entity* Level::CloneEntity( u64 id )
{
	const Entity* source = FindEntity( id );
	if( source == nullptr )
	{
		GAIA_ERROR( "Level {}: cannot clone unknown entity {}", GetName(), id );
		return nullptr;
	}

	Entity* clone = CreateEntity( MakeCloneName( source->GetName() ) );

	// Note: The transform goes first because components read it when they are added
	clone->SetTransform( source->GetTransform() );

	Json::Value components;
	if( !source->SerializeComponents( components ) || !clone->DeserializeComponents( components ) )
	{
		GAIA_ERROR( "Level {}: failed to clone the components of entity {}", GetName(), source->GetName() );
		RemoveEntity( clone->GetId() );
		return nullptr;
	}

	return clone;
}

std::string Level::MakeCloneName( std::string_view sourceName ) const
{
	auto isNameTaken = [this]( std::string_view name ) {
		return std::any_of( m_Entities.begin(), m_Entities.end(), [name]( const unique<Entity>& entity ) {
			return entity != nullptr && entity->GetName() == name;
		} );
	};

	const std::string baseName( RemoveCloneNumber( sourceName ) );

	// Note: Names can repeat, entities are told apart by their id, the number only helps to read the Hierarchy
	u64         cloneNumber = 1;
	std::string cloneName   = baseName + " (1)";
	while( isNameTaken( cloneName ) )
	{
		++cloneNumber;
		cloneName = baseName + " (" + std::to_string( cloneNumber ) + ")";
	}

	return cloneName;
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
