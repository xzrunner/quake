#include "quake/TextureManager.h"

namespace quake
{

CU_SINGLETON_DEFINITION(TextureManager);

TextureManager::TextureManager()
{
}

void TextureManager::Add(const std::string& name, ur::TexturePtr& tex)
{
	m_name2tex.insert({ name, tex });
}

ur::TexturePtr TextureManager::Query(const std::string& name) const
{
	auto itr = m_name2tex.find(name);
	return itr == m_name2tex.end() ? nullptr : itr->second;
}

}