#pragma once

#include "quake/MapAttributes.h"

#include <SM_Vector.h>

#include <vector>
#include <string>

namespace quake
{

struct MapFace
{
	std::string tex_name;

	sm::vec3 vertices[3];

	sm::vec2 offset;
	float    angle;
	sm::vec2 scale;

}; // MapFace

struct MapBrush
{
	MapBrush(const std::vector<MapFace>& faces)
		: faces(faces) {}

	std::vector<MapFace> faces;

}; // MapBrush

struct MapEntity
{
	std::vector<EntityAttribute> attributes;
	std::vector<MapBrush> brushes;

	size_t start_line;
	size_t line_count;

}; // MapEntity

}