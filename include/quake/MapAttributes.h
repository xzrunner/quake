#pragma once

#include <string>
#include <map>
#include <vector>

namespace quake
{

typedef std::string AttributeName;
typedef std::string AttributeValue;

namespace AttributeNames
{
	extern const AttributeName Classname;
	extern const AttributeName Origin;
	extern const AttributeName Wad;
	extern const AttributeName Textures;
	extern const AttributeName Mods;
	extern const AttributeName GameEngineParameterSpecs;
	extern const AttributeName Spawnflags;
	extern const AttributeName EntityDefinitions;
	extern const AttributeName Angle;
	extern const AttributeName Angles;
	extern const AttributeName Mangle;
	extern const AttributeName Target;
	extern const AttributeName Targetname;
	extern const AttributeName Killtarget;
	extern const AttributeName GroupType;
	extern const AttributeName LayerId;
	extern const AttributeName LayerName;
	extern const AttributeName Layer;
	extern const AttributeName GroupId;
	extern const AttributeName GroupName;
	extern const AttributeName Group;
	extern const AttributeName Message;
}

namespace AttributeValues
{
	extern const AttributeValue WorldspawnClassname;
	extern const AttributeValue NoClassname;
	extern const AttributeValue LayerClassname;
	extern const AttributeValue GroupClassname;
	extern const AttributeValue GroupTypeLayer;
	extern const AttributeValue GroupTypeGroup;
}

std::string NumberedAttributePrefix(const std::string& name);
bool IsNumberedAttribute(const std::string& prefix, const AttributeName& name);

//typedef std::vector<EntityAttribute> List;

struct EntityAttribute
{
	EntityAttribute(const AttributeName& name, const AttributeValue& val)
		: name(name), val(val)
	{
	}

	AttributeName  name;
	AttributeValue val;

}; // EntityAttribute

bool IsLayer(const std::string& classname, const std::vector<EntityAttribute>& attributes);
bool IsGroup(const std::string& classname, const std::vector<EntityAttribute>& attributes);
bool IsWorldspawn(const std::string& classname, const std::vector<EntityAttribute>& attributes);
const AttributeValue& FindAttribute(const std::vector<EntityAttribute>& attributes, const AttributeName& name, const AttributeValue& default_value = "");

struct ExtraAttribute
{
	typedef enum {
		Type_String,
		Type_Integer
	} Type;

	ExtraAttribute(Type type, const std::string& name, const std::string& value, size_t line, size_t column)
		: type(type), name(name), value(value), line(line), column(column) {}

	template <typename T>
	T IntValue() const
	{
		assert(type == Type_Integer);
		return static_cast<T>(std::atoi(value.c_str()));
	}

	Type type;

	std::string name;
	std::string value;

	size_t line;
	size_t column;

}; // ExtraAttribute

}