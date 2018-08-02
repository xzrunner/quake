#pragma once

#include "quake/MapAttributes.h"

#include <SM_Vector.h>
#include <SM_Plane.h>
#include <SM_Cube.h>
#include <halfedge/Polyhedron.h>

#include <vector>
#include <string>

namespace he { class Polyhedron; }

namespace quake
{

struct TexCoordSystem
{
	size_t   index;
	sm::vec3 x_axis;
	sm::vec3 y_axis;

}; // TexCoordSystem

struct BrushVertex
{
	BrushVertex(const sm::vec3& pos)
		: pos(pos) {}

	sm::vec3 pos;

}; // BrushVertex

using BrushVertexPtr = std::shared_ptr<BrushVertex>;

struct BrushFace
{
	std::string tex_name;

	sm::Plane plane;
	std::vector<BrushVertexPtr> vertices;

	// texcoords
	TexCoordSystem tc_sys;
	sm::vec2 offset;
	float    angle = 0;
	sm::vec2 scale;

	void SortVertices();

	void InitTexCoordSys();

	sm::vec2 CalcTexCoords(const sm::vec3& pos, float tex_w, float tex_h) const;

	bool AddVertex(const BrushVertexPtr& v);

}; // BrushFace

using BrushFacePtr = std::shared_ptr<BrushFace>;

struct MapBrush
{
	MapBrush(const std::vector<BrushFacePtr>& faces);

	void BuildVertices();

	void BuildGeometry();
	void RebuildGeometry(const sm::cube& world_bound);

	std::vector<BrushVertexPtr> vertices;
	std::vector<BrushFacePtr>   faces;

	he::PolyhedronPtr geometry = nullptr;

}; // MapBrush

struct MapEntity
{
	std::vector<EntityAttribute> attributes;
	std::vector<MapBrush> brushes;

	//size_t start_line;
	//size_t line_count;

}; // MapEntity

using MapEntityPtr = std::shared_ptr<MapEntity>;

}