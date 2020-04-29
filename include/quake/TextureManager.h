#pragma once

#include <cu/cu_macro.h>
#include <unirender/typedef.h>

#include <map>
#include <memory>

namespace quake
{

class TextureManager
{
public:
	void Add(const std::string& name, ur::TexturePtr& tex);

    ur::TexturePtr Query(const std::string& name) const;

private:
	std::map<std::string, ur::TexturePtr> m_name2tex;

	CU_SINGLETON_DECLARATION(TextureManager);

}; // TextureManager

}