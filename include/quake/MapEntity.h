#pragma once

#include "quake/MapAttributes.h"

#include <vector>
#include <memory>

namespace pm3 { class Polytope; }

namespace quake
{

struct MapEntity
{
	std::vector<EntityAttribute> attributes;
	std::vector<std::shared_ptr<pm3::Polytope>> brushes;

	//size_t start_line;
	//size_t line_count;

}; // MapEntity

}