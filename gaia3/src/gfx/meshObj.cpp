#include "gaia3_pch.h"
#include "meshObj.h"

#include <charconv>
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
} // namespace

MeshObj::MeshObj( std::string_view path )
{
	std::ifstream inputFile( path.data() );
	if( !inputFile.is_open() )
	{
		GAIA_ERROR( "MeshObj: failed to open {}", path );
		return;
	}

	// Load the file into temporary vectors
	std::vector<Vec3>       tempPositions;
	std::vector<Vec2>       tempUVs;
	std::vector<Vec3>       tempNormals;
	std::vector<FaceVertex> tempIndices;

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
				// Triangulate non-triangular polygons
				for( u64 i = 1; i + 1 < polygon.size(); ++i )
				{
					tempIndices.push_back( polygon[0] );
					tempIndices.push_back( polygon[i] );
					tempIndices.push_back( polygon[i + 1] );
				}
			}
		}

		if( !bLineValid )
		{
			GAIA_ERROR( "MeshObj: invalid data at {}:{}", path, lineNumber );
			return;
		}
	}

	if( tempIndices.empty() )
	{
		GAIA_ERROR( "MeshObj: no faces found in {}", path );
		return;
	}

	if( tempNormals.empty() )
	{
		GAIA_WARN( "MeshObj: {} has no normals, using zero normals", path );
	}

	// Note: build the vertex buffer and indices, sharing corners with the same position, UV and normal
	std::unordered_map<FaceVertex, u64, FaceVertex::Hasher> vertexLookup;
	for( const FaceVertex& vertex : tempIndices )
	{
		const auto [lookupIt, bInserted] = vertexLookup.try_emplace( vertex, vertexBuffer.positions.size() );
		if( bInserted )
		{
			vertexBuffer.positions.push_back( tempPositions[vertex.position] );
			vertexBuffer.uvs.push_back( vertex.uv >= 0 ? tempUVs[vertex.uv] : Vec2() );
			vertexBuffer.normals.push_back( vertex.normal >= 0 ? tempNormals[vertex.normal] : Vec3() );
		}

		indices.push_back( lookupIt->second );
	}

	bIsValid = true;
}
