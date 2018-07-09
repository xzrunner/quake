#include "quake/Palette.h"
#include "quake/ColorMap.h"

#include <boost/filesystem.hpp>

namespace quake
{

Palette::Palette()
	: m_size(0)
	, m_data(nullptr)
{
}

Palette::~Palette()
{
	if (m_data) {
		delete[] m_data;
	}
}

void Palette::LoadFromFile(const std::string& filepath)
{
	auto ext = boost::filesystem::extension(filepath);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
	if (ext == ".lmp")
	{
		std::ifstream fin(filepath, std::ios::binary | std::ios::ate);
		m_size = static_cast<size_t>(fin.tellg());
		fin.seekg(0, std::ios::beg);

		if (m_data) {
			delete[] m_data;
		}
		m_data = new unsigned char[m_size];
		fin.read(reinterpret_cast<char*>(m_data), m_size);
	}
}

void Palette::IndexedToRgb(const unsigned char* indexed, size_t size,
	                       unsigned char* rgb) const
{
	if (m_data)
	{
		for (size_t i = 0; i < size; ++i)
		{
			auto index = static_cast<size_t>(indexed[i]);
			assert(index < m_size);
			for (size_t j = 0; j < 3; ++j)
			{
				auto c = m_data[index * 3 + j];
				rgb[i * 3 + j] = c;
			}
		}
	}
	else
	{
		const int sz = sizeof(COLOR_MAP);
		for (size_t i = 0; i < size; ++i)
		{
			auto index = static_cast<size_t>(indexed[i]);
			assert(index < 256);
			for (size_t j = 0; j < 3; ++j) {
				rgb[i * 3 + j] = COLOR_MAP[index][j];
			}
		}
	}
}

}