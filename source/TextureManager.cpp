#include "quake/TextureManager.h"

namespace quake
{

CU_SINGLETON_DEFINITION(TextureManager);

TextureManager::TextureManager()
{
}

void TextureManager::Add(const std::string& name, std::unique_ptr<pt2::Texture>& tex)
{
	m_name2tex.insert({ name, std::move(tex) });
}

}