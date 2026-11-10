#pragma once

#include <math/matrix.h>
#include <system/resource.h>

#include <string>
#include <string_view>

class Shader : public Resource
{
	RTTI_DECLARE_RESOURCE( Shader )
public:
	explicit Shader( std::string_view path, ResourceState state = ResourceState::NotLoaded );

	// Note: A .glsl file holds both stages, each one starts after a "#type vertex" or "#type fragment" line
	static bool SplitStages( std::string_view source, std::string& outVertex, std::string& outFragment );

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
