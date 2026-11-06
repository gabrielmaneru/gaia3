#pragma once

#include <system/rtti.h>
#include <system/type_id.h>

#include <string>
#include <string_view>
#include <vector>

enum class ResourceState
{
	NotLoaded,
	Loading,
	Loaded,
	New
};

class Resource
{
	Resource( const Resource& ) = delete;

public:
	Resource( std::string_view path, ResourceState state = ResourceState::NotLoaded );
	virtual ~Resource() = default;

	bool Load();
	bool Unload();
	bool SaveAs( std::string_view path );

	ResourceState GetState() const { return m_State; }
	bool          IsLoaded() const { return m_State == ResourceState::Loaded; }
	bool          IsNew() const { return m_State == ResourceState::New; }

	std::string_view GetName() const { return m_Name; }
	std::string_view GetPath() const { return m_Path; }
	void             SetPath( std::string_view path );

	struct TypeDescriptor
	{
		using CreateFunc = shared<Resource> ( * )( std::string_view );
		u64                      TypeId;
		std::vector<std::string> Extensions;
		CreateFunc               Create;
		bool                     IsValid() const { return TypeId != 0 && Create != nullptr && !Extensions.empty(); }
	};
	static const TypeDescriptor* FindResourceTypeByExtension( std::string_view extension );
	static bool                  RegisterResourceType( const TypeDescriptor& descriptor );

protected:
	virtual bool LoadInternal() { return true; }
	virtual bool UnloadInternal() { return true; }
	virtual bool SaveAsInternal( std::string_view path ) { return true; }

	ResourceState m_State = ResourceState::NotLoaded;
	std::string   m_Name;
	std::string   m_Path;

private:
	static std::vector<Resource::TypeDescriptor>& GetTypeDescriptorsRef();
};

#define RTTI_DECLARE_RESOURCE( ... ) RTTI_DECLARE_TYPE( __VA_ARGS__ )

#define REGISTER_RESOURCE_TYPE( TypeName, FactoryFn, ... )                               \
	namespace                                                                            \
	{                                                                                    \
	const bool s_##TypeName##ResourceRegistered = Resource::RegisterResourceType(        \
	    Resource::TypeDescriptor{ GetTypeId<TypeName>(), { __VA_ARGS__ }, FactoryFn } ); \
	}

#define RTTI_RESOURCE_EXTENSIONS( ... ) __VA_ARGS__

#define RTTI_DEFINE_RESOURCE( TypeName, Extensions, ... )                \
	RTTI_DEFINE_TYPE( TypeName, __VA_ARGS__ )                            \
	namespace                                                            \
	{                                                                    \
	shared<Resource> Create##TypeName##Resource( std::string_view path ) \
	{                                                                    \
		return new_shared<TypeName>( path, ResourceState::NotLoaded );   \
	}                                                                    \
	}                                                                    \
	REGISTER_RESOURCE_TYPE( TypeName, &Create##TypeName##Resource, Extensions )
