#pragma once

#include <cu/cu_macro.h>
#include <painting2/Texture.h>

#include <map>
#include <memory>

namespace quake
{

class TextureManager
{
public:
	void Add(const std::string& name, std::unique_ptr<pt2::Texture>& tex);

private:
	std::map<std::string, std::unique_ptr<pt2::Texture>> m_name2tex;

	CU_SINGLETON_DECLARATION(TextureManager);

}; // TextureManager

}