#pragma once

#include <cu/cu_macro.h>
#include <unirender2/typedef.h>

#include <map>
#include <memory>

namespace quake
{

class TextureManager
{
public:
	void Add(const std::string& name, ur2::TexturePtr& tex);

    ur2::TexturePtr Query(const std::string& name) const;

private:
	std::map<std::string, ur2::TexturePtr> m_name2tex;

	CU_SINGLETON_DECLARATION(TextureManager);

}; // TextureManager

}