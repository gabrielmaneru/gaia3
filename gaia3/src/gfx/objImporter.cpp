#include "gaia3_pch.h"
#include "objImporter.h"

#include <cctype>
#include <charconv>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace
{
bool ParseVec2( std::istringstream& stream, Vec2& outValue )
{
	if( !( stream >> outValue.x ) )
		return false;

	// The second coordinate of a "vt" line is optional.
	if( !( stream >> outValue.y ) )
		outValue.y = 0.0f;

	return true;
}

bool ParseVec3( std::istringstream& stream, Vec3& outValue )
{
	if( !( stream >> outValue.x ) )
		return false;

	if( !( stream >> outValue.y ) )
		return false;

	if( !( stream >> outValue.z ) )
		return false;

	return true;
}

struct FaceVertex
{
	int position = -1;
	int uv       = -1;
	int normal   = -1;

	struct Hasher
	{
		u64 operator()( const FaceVertex& vertex ) const
		{
			u64 hash = std::hash<int>{}( vertex.position );
			hash     = hash * 31 + std::hash<int>{}( vertex.uv );
			hash     = hash * 31 + std::hash<int>{}( vertex.normal );
			return hash;
		}
	};
	bool operator==( const FaceVertex& vertex ) const = default;
};

bool ParseFaceVertex( std::string_view token, u64 positionCount, u64 uvCount, u64 normalCount, FaceVertex& outVertex )
{
	auto resolveIndex = []( std::string_view text, u64 count, int& outIndex ) -> bool {
		const char* begin = text.data();
		const char* end   = text.data() + text.size();

		int index = 0;

		const auto [parsedEnd, errorCode] = std::from_chars( begin, end, index );
		if( errorCode != std::errc() || parsedEnd != end || index < 1 )
			return false;

		// Converts 1-based index to a zero-based
		outIndex = index - 1;

		return static_cast<u64>( outIndex ) < count;
	};

	// Parse Position
	const u64 firstSlash = token.find( '/' );
	if( !resolveIndex( token.substr( 0, firstSlash ), positionCount, outVertex.position ) )
		return false;
	if( firstSlash == std::string_view::npos )
		return true;
	const std::string_view afterPosition = token.substr( firstSlash + 1 );

	// Parse UV
	const u64              secondSlash = afterPosition.find( '/' );
	const std::string_view uvText      = afterPosition.substr( 0, secondSlash );
	if( !uvText.empty() && !resolveIndex( uvText, uvCount, outVertex.uv ) )
		return false;
	if( secondSlash == std::string_view::npos )
		return true;

	// Parse Normal
	return resolveIndex( afterPosition.substr( secondSlash + 1 ), normalCount, outVertex.normal );
}

// Note: Names can hold spaces, like "g wall left", so they are the whole rest of the line
std::string ReadRestOfLine( std::istringstream& stream )
{
	std::string rest;
	std::getline( stream >> std::ws, rest );

	while( !rest.empty() && std::isspace( static_cast<unsigned char>( rest.back() ) ) )
	{
		rest.pop_back();
	}

	return rest;
}

// Note: Faces of one object or group, kept per material in the order each material first appears
struct MeshFaces
{
	std::string                                              name;
	std::vector<std::string>                                 slotOrder;
	std::unordered_map<std::string, std::vector<FaceVertex>> facesBySlot;
};

u64 FindOrAddMesh( std::vector<MeshFaces>& meshes, const std::string& name )
{
	for( u64 i = 0; i < meshes.size(); ++i )
	{
		if( meshes[i].name == name )
			return i;
	}

	meshes.push_back( MeshFaces{ name } );
	return meshes.size() - 1;
}
} // namespace

bool ObjImporter::Import( std::string_view path, ModelData& outData )
{
	outData = ModelData();

	std::ifstream inputFile{ std::string( path ) };
	if( !inputFile.is_open() )
	{
		GAIA_ERROR( "ObjImporter: failed to open {}", path );
		return false;
	}

	// Load the file into temporary vectors
	std::vector<Vec3> tempPositions;
	std::vector<Vec2> tempUVs;
	std::vector<Vec3> tempNormals;

	// Note: The current mesh is looked up on its first face, so object names without faces add no mesh
	std::vector<MeshFaces> meshes;
	std::string            currentMesh;
	u64                    currentMeshIndex = 0;
	bool                   bMeshKnown       = false;
	std::string            currentSlot;

	std::string line;
	u64         lineNumber = 0;
	while( std::getline( inputFile, line ) )
	{
		++lineNumber;

		std::istringstream lineStream( line );
		std::string        keyword;
		lineStream >> keyword;

		bool bLineValid = true;
		if( keyword == "v" )
		{
			Vec3 position;
			bLineValid = ParseVec3( lineStream, position );
			tempPositions.push_back( position );
		}
		else if( keyword == "vn" )
		{
			Vec3 normal;
			bLineValid = ParseVec3( lineStream, normal );
			tempNormals.push_back( normal );
		}
		else if( keyword == "vt" )
		{
			Vec2 uv;
			bLineValid = ParseVec2( lineStream, uv );
			tempUVs.push_back( uv );
		}
		else if( keyword == "usemtl" )
		{
			// Note: A usemtl line without a name goes back to the default material
			currentSlot.clear();
			lineStream >> currentSlot;
		}
		else if( keyword == "o" || keyword == "g" )
		{
			currentMesh = ReadRestOfLine( lineStream );
			bMeshKnown  = false;
		}
		else if( keyword == "mtllib" )
		{
			std::string library;
			while( lineStream >> library )
			{
				outData.materialLibraries.push_back( library );
			}
		}
		else if( keyword == "f" )
		{
			std::vector<FaceVertex> polygon;

			std::string token;
			while( bLineValid && lineStream >> token )
			{
				FaceVertex vertex;
				bLineValid = ParseFaceVertex( token, tempPositions.size(), tempUVs.size(), tempNormals.size(), vertex );
				polygon.push_back( vertex );
			}

			bLineValid = bLineValid && polygon.size() >= 3;
			if( bLineValid )
			{
				if( !bMeshKnown )
				{
					currentMeshIndex = FindOrAddMesh( meshes, currentMesh );
					bMeshKnown       = true;
				}

				MeshFaces& mesh               = meshes[currentMeshIndex];
				const auto [slotIt, bNewSlot] = mesh.facesBySlot.try_emplace( currentSlot );
				if( bNewSlot )
				{
					mesh.slotOrder.push_back( currentSlot );
				}

				// Triangulate non-triangular polygons
				std::vector<FaceVertex>& slotFaces = slotIt->second;
				for( u64 i = 1; i + 1 < polygon.size(); ++i )
				{
					slotFaces.push_back( polygon[0] );
					slotFaces.push_back( polygon[i] );
					slotFaces.push_back( polygon[i + 1] );
				}
			}
		}

		if( !bLineValid )
		{
			GAIA_ERROR( "ObjImporter: invalid data at {}:{}", path, lineNumber );
			return false;
		}
	}

	if( meshes.empty() )
	{
		GAIA_ERROR( "ObjImporter: no faces found in {}", path );
		return false;
	}

	if( tempNormals.empty() )
	{
		GAIA_WARN( "ObjImporter: {} has no normals, using zero normals", path );
	}

	// Note: build the vertex buffer and indices, sharing corners with the same position, UV and normal
	std::unordered_map<FaceVertex, u32, FaceVertex::Hasher> vertexLookup;
	for( MeshFaces& mesh : meshes )
	{
		MeshData meshData;
		meshData.name = mesh.name;

		for( const std::string& slot : mesh.slotOrder )
		{
			SubMeshData subMesh;
			subMesh.indexOffset  = static_cast<u32>( outData.indices.size() );
			subMesh.materialSlot = slot;

			for( const FaceVertex& vertex : mesh.facesBySlot[slot] )
			{
				const auto [lookupIt, bInserted] = vertexLookup.try_emplace( vertex, static_cast<u32>( outData.positions.size() ) );
				if( bInserted )
				{
					outData.positions.push_back( tempPositions[vertex.position] );
					outData.uvs.push_back( vertex.uv >= 0 ? tempUVs[vertex.uv] : Vec2() );
					outData.normals.push_back( vertex.normal >= 0 ? tempNormals[vertex.normal] : Vec3() );
				}

				outData.indices.push_back( lookupIt->second );
			}

			subMesh.indexCount = static_cast<u32>( outData.indices.size() ) - subMesh.indexOffset;
			meshData.subMeshes.push_back( subMesh );
		}

		outData.meshes.push_back( meshData );
	}

	return true;
}
