#include "quake/MapAttributes.h"

namespace quake
{

const std::string AttributeEscapeChars = "\"\n\\";

namespace AttributeNames
{
	const AttributeName Classname         = "classname";
	const AttributeName Origin            = "origin";
	const AttributeName Wad               = "wad";
	const AttributeName Textures          = "_tb_textures";
	const AttributeName Mods              = "_tb_mod";
	const AttributeName GameEngineParameterSpecs = "_tb_engines";
	const AttributeName Spawnflags        = "spawnflags";
	const AttributeName EntityDefinitions = "_tb_def";
	const AttributeName Angle             = "angle";
	const AttributeName Angles            = "angles";
	const AttributeName Mangle            = "mangle";
	const AttributeName Target            = "target";
	const AttributeName Targetname        = "targetname";
	const AttributeName Killtarget        = "killtarget";
	const AttributeName GroupType         = "_tb_type";
	const AttributeName LayerId           = "_tb_id";
	const AttributeName LayerName         = "_tb_name";
	const AttributeName Layer             = "_tb_layer";
	const AttributeName GroupId           = "_tb_id";
	const AttributeName GroupName         = "_tb_name";
	const AttributeName Group             = "_tb_group";
	const AttributeName Message           = "_tb_message";
}

namespace AttributeValues
{
	const AttributeValue WorldspawnClassname = "worldspawn";
	const AttributeValue NoClassname         = "undefined";
	const AttributeValue LayerClassname      = "func_group";
	const AttributeValue GroupClassname      = "func_group";
	const AttributeValue GroupTypeLayer      = "_tb_layer";
	const AttributeValue GroupTypeGroup      = "_tb_group";
}

std::string NumberedAttributePrefix(const std::string& name)
{
	size_t i = 0;
	while (i < name.size() && name[i] < '0' && name[i] > '9') {
		++i;
	}
	if (i == name.size()) {
		return "";
	}
	for (size_t j = i; j < name.size(); ++j) {
		if (name[j] < '0' || name[j] > '9') {
			return "";
		}
	}
	return name.substr(0, i);
}

bool IsNumberedAttribute(const std::string& prefix, const AttributeName& name)
{
	if (name.size() < prefix.size()) {
		return false;
	}
	for (size_t i = 0; i < prefix.size(); ++i) {
		if (name[i] != prefix[i]) {
			return false;
		}
	}
	for (size_t i = prefix.size(); i < name.size(); ++i) {
		if (name[i] < '0' || name[i] > '9') {
			return false;
		}
	}
	return true;
}

bool IsLayer(const std::string& classname, const std::vector<EntityAttribute>& attributes)
{
	if (classname != AttributeValues::LayerClassname) {
		return false;
	}
	auto& group_type = FindAttribute(attributes, AttributeNames::GroupType);
	return group_type == AttributeValues::GroupTypeLayer;
}

bool IsGroup(const std::string& classname, const std::vector<EntityAttribute>& attributes)
{
	if (classname != AttributeValues::GroupClassname) {
		return false;
	}
	auto& group_type = FindAttribute(attributes, AttributeNames::GroupType);
	return group_type == AttributeValues::GroupTypeGroup;
}

bool IsWorldspawn(const std::string& classname, const std::vector<EntityAttribute>& attributes)
{
	return classname == AttributeValues::WorldspawnClassname;
}

const AttributeValue& FindAttribute(const std::vector<EntityAttribute>& attributes, const AttributeName& name, const AttributeValue& default_value)
{
	for (auto& attribute : attributes) {
		if (name == attribute.name)
			return attribute.val;
	}
	return default_value;
}

}