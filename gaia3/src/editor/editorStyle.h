#pragma once

#include <core/types.h>

#include <imgui.h>

// Note: The editor's colours, for editors that draw their own widgets in the shared style. Needs ImGui, so only
// editor code includes it
namespace EditorStyle
{
constexpr ImVec4 RgbColor( u32 hex, float alpha = 1.0f )
{
	return ImVec4( ( ( hex >> 16 ) & 0xFF ) / 255.0f, ( ( hex >> 8 ) & 0xFF ) / 255.0f, ( hex & 0xFF ) / 255.0f, alpha );
}

constexpr ImVec4 kColorPanel         = RgbColor( 0x333333 );
constexpr ImVec4 kColorStrip         = RgbColor( 0x232323 );
constexpr ImVec4 kColorField         = RgbColor( 0x262626 );
constexpr ImVec4 kColorFieldHovered  = RgbColor( 0x2E2E2E );
constexpr ImVec4 kColorControl       = RgbColor( 0x454545 );
constexpr ImVec4 kColorControlHover  = RgbColor( 0x525252 );
constexpr ImVec4 kColorControlActive = RgbColor( 0x5E5E5E );
constexpr ImVec4 kColorBorder        = RgbColor( 0x1A1A1A );
constexpr ImVec4 kColorAccent        = RgbColor( 0x2C5D87 );
constexpr ImVec4 kColorAccentHover   = RgbColor( 0x3A76AB );
constexpr ImVec4 kColorAccentBright  = RgbColor( 0x4C9BE0 );
constexpr ImVec4 kColorText          = RgbColor( 0xD4D4D4 );
constexpr ImVec4 kColorTextDisabled  = RgbColor( 0x7E7E7E );
constexpr ImVec4 kColorInvisible     = RgbColor( 0x000000, 0.0f );

// Note: Right clicking empty space of a window, not one of its items
constexpr ImGuiPopupFlags kEmptySpaceContextMenu = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;

// Note: Collapsing headers share their colour with selected rows, which should be the only blue in the panels, so
// headers are drawn grey between these two calls
inline void PushGreyHeaders()
{
	ImGui::PushStyleColor( ImGuiCol_Header, kColorControl );
	ImGui::PushStyleColor( ImGuiCol_HeaderHovered, kColorControlHover );
	ImGui::PushStyleColor( ImGuiCol_HeaderActive, kColorControlActive );
}

inline void PopGreyHeaders()
{
	ImGui::PopStyleColor( 3 );
}
} // namespace EditorStyle
