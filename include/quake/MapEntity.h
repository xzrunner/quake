#pragma once

#include "quake/MapAttributes.h"

#include <polymesh3/typedef.h>

#include <vector>
#include <memory>

namespace quake
{

struct MapEntity
{
	std::vector<EntityAttribute>  attributes;
	std::vector<pm3::PolytopePtr> brushes;

	//size_t start_line;
	//size_t line_count;

}; // MapEntity

}