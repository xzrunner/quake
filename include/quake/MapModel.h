#pragma once

#include "quake/MapAttributes.h"

#include <SM_Vector.h>
#include <SM_Plane.h>

#include <vector>
#include <string>

namespace quake
{

struct TexCoordSystem
{
	size_t   index;
	sm::vec3 x_axis;
	sm::vec3 y_axis;

}; // TexCoordSystem

struct BrushFace
{
	std::string tex_name;

	sm::Plane             plane;
	std::vector<sm::vec3> vertices;

	// texcoords
	TexCoordSystem tc_sys;
	sm::vec2 offset;
	float    angle = 0;
	sm::vec2 scale;

	void SortVertices();

	void InitTexCoordSys();

	sm::vec2 CalcTexCoords(const sm::vec3& pos, float tex_w, float tex_h) const;

	void AddVertex(const sm::vec3& v);

}; // BrushFace

struct MapBrush
{
	MapBrush(const std::vector<BrushFace>& faces)
		: faces(faces) {}

	void BuildVertices();

	std::vector<BrushFace> faces;

}; // MapBrush

struct MapEntity
{
	std::vector<EntityAttribute> attributes;
	std::vector<MapBrush> brushes;

	size_t start_line;
	size_t line_count;

}; // MapEntity

}