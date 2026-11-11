#pragma once

#include <math/matrix.h>
#include <system/resource.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

// Note: A .glsli file in shaders/, shader code that .glsl files paste in with an #include line rather than a shader
// of its own
class ShaderInclude : public Resource
{
	RTTI_DECLARE_RESOURCE( ShaderInclude )
public:
	explicit ShaderInclude( std::string_view path, ResourceState state = ResourceState::NotLoaded );

	bool LoadInternal() override;
	bool UnloadInternal() override;

	const std::string& GetSource() const { return m_Source; }

private:
	std::string m_Source;
};

class Shader : public Resource
{
	RTTI_DECLARE_RESOURCE( Shader )
public:
	// Note: Gives the text of the include with that name, false when there is none
	using IncludeReader = std::function<bool( std::string_view includeName, std::string& outSource )>;

	explicit Shader( std::string_view path, ResourceState state = ResourceState::NotLoaded );

	// Note: A .glsl file holds both stages, each one starts after a "#type vertex" or "#type fragment" line
	static bool SplitStages( std::string_view source, std::string& outVertex, std::string& outFragment );

	// Note: Replaces each #include "name" line with that include's text, includes can include others too. Each one is
	// pasted only once, later #include lines for it are dropped. #line lines around each paste make compile errors
	// name the right line: the line's second number is the file's index in outSourceNames, 0 being source itself
	static bool ResolveIncludes( std::string_view source, const IncludeReader& readInclude, std::string& outSource, std::vector<std::string>& outSourceNames );

	bool LoadInternal() override;
	bool UnloadInternal() override;

	void Use() const;

	// Note: Values for uniforms the shader doesn't declare are ignored
	void SetInt( const char* name, int value ) const;
	void SetFloat( const char* name, float value ) const;
	void SetVec3( const char* name, const Vec3& value ) const;
	void SetMatrix44( const char* name, const Matrix44& value ) const;

private:
	u32 m_Program = 0;
};
