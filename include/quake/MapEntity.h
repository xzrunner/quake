#pragma once

#include "quake/MapAttributes.h"

#include <vector>
#include <memory>

namespace pm3 { struct Brush; }

namespace quake
{

struct MapEntity
{
	std::vector<EntityAttribute> attributes;
	std::vector<pm3::Brush>      brushes;

	//size_t start_line;
	//size_t line_count;

}; // MapEntity

}