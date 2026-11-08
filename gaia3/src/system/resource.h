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

	// Note: The resource's name inside a file that holds several, like one material of a .mtl library. Empty for
	// files that hold a single resource, which are named after the file
	std::string_view GetEntry() const { return m_Entry; }
	void             SetEntry( std::string_view entry );

	struct TypeDescriptor
	{
		using CreateFunc      = shared<Resource> ( * )( std::string_view path, std::string_view entry );
		using ListEntriesFunc = std::vector<std::string> ( * )( std::string_view path );
		u64                      TypeId;
		std::string              Folder;
		std::vector<std::string> Extensions;
		CreateFunc               Create;
		ListEntriesFunc          ListEntries = nullptr;
		bool                     IsValid() const { return TypeId != 0 && !Folder.empty() && Create != nullptr && !Extensions.empty(); }
	};
	static const TypeDescriptor* FindResourceTypeByExtension( std::string_view extension );
	static const TypeDescriptor* FindResourceTypeById( u64 typeId );
	static bool                  RegisterResourceType( const TypeDescriptor& descriptor );

protected:
	virtual bool LoadInternal() { return true; }
	virtual bool UnloadInternal() { return true; }
	virtual bool SaveAsInternal( std::string_view path ) { return true; }

	ResourceState m_State = ResourceState::NotLoaded;
	std::string   m_Name;
	std::string   m_Path;
	std::string   m_Entry;

private:
	static std::vector<Resource::TypeDescriptor>& GetTypeDescriptorsRef();
};

// Note: A type with a static ListEntries( path ) reads several resources from one file, the others read one
template <typename T>
constexpr Resource::TypeDescriptor::ListEntriesFunc GetListEntriesFunc()
{
	if constexpr( requires { T::ListEntries( std::string_view{} ); } )
		return &T::ListEntries;
	else
		return nullptr;
}

#define RTTI_DECLARE_RESOURCE( ... ) RTTI_DECLARE_TYPE( __VA_ARGS__ )

// Note: The folder of a type is where it saves, and files inside it are named without it
#define RTTI_RESOURCE_FOLDER( Folder ) Folder

#define RTTI_RESOURCE_EXTENSIONS( ... ) __VA_ARGS__

#define RTTI_DEFINE_RESOURCE( TypeName, Folder, Extensions, ... )                                                                                 \
	RTTI_DEFINE_TYPE( TypeName, __VA_ARGS__ )                                                                                                     \
	namespace                                                                                                                                     \
	{                                                                                                                                             \
	shared<Resource> Create##TypeName##Resource( std::string_view path, std::string_view entry )                                                  \
	{                                                                                                                                             \
		shared<TypeName> resource = new_shared<TypeName>( path, ResourceState::NotLoaded );                                                       \
		resource->SetEntry( entry );                                                                                                              \
		return resource;                                                                                                                          \
	}                                                                                                                                             \
	const bool s_##TypeName##ResourceRegistered = Resource::RegisterResourceType(                                                                 \
	    Resource::TypeDescriptor{ GetTypeId<TypeName>(), Folder, { Extensions }, &Create##TypeName##Resource, GetListEntriesFunc<TypeName>() } ); \
	}
