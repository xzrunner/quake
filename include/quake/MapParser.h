// Code from https://github.com/kduske/TrenchBroom

#pragma once

#include "quake/MapModel.h"

#include <lexer/Tokenizer.h>
#include <lexer/Parser.h>
#include <SM_Vector.h>

#include <vector>
#include <set>

namespace quake
{

namespace MapToken
{
	typedef unsigned int Type;
	static const Type Integer       = 1 <<  0; // integer number
	static const Type Decimal       = 1 <<  1; // decimal number
	static const Type String        = 1 <<  2; // string
	static const Type OParenthesis  = 1 <<  3; // opening parenthesis: (
	static const Type CParenthesis  = 1 <<  4; // closing parenthesis: )
	static const Type OBrace        = 1 <<  5; // opening brace: {
	static const Type CBrace        = 1 <<  6; // closing brace: }
	static const Type OBracket      = 1 <<  7; // opening bracket: [
	static const Type CBracket      = 1 <<  8; // closing bracket: ]
	static const Type Comment       = 1 <<  9; // line comment starting with ///
	static const Type Eof           = 1 << 10; // end of file
	static const Type Eol           = 1 << 11; // end of line
}

namespace MapFormat
{
	typedef size_t Type;
	static const Type Unknown  = 1 << 0;
	static const Type Standard = 1 << 1;
	static const Type Quake2   = 1 << 2;
	static const Type Valve    = 1 << 3;
	static const Type Hexen2   = 1 << 4;
}

class MapTokenizer : public lexer::Tokenizer<MapToken::Type>
{
public:
	MapTokenizer(const std::string& str);

	void SetSkipEol(bool skip_eol);

protected:
	virtual Token EmitToken() override;

	static const std::string& NumberDelim();

private:
	bool m_skip_eol;

}; // MapTokenizer

struct ExtraAttribute;

class MapParser : public lexer::Parser<MapToken::Type>
{
public:
	MapParser(const std::string& str);
	virtual ~MapParser() override;

	void Parse();

	const MapEntity* GetWorldEntity() const;
	auto& GetAllEntities() const { return m_entities; }

	void UpdateFaceTextures();

protected:
	void ParseEntities(MapFormat::Type format);
	void ParseBrushes(MapFormat::Type format);
	void ParseBrushFaces(MapFormat::Type format);

	void Reset();
private:
	void SetFormat(MapFormat::Type format);

	void ParseEntity();
	void ParseEntityAttribute(std::vector<EntityAttribute>& attributes,
		std::set<AttributeName>& names);
	void ParseBrush();
	void ParseFace();

	sm::vec3 ParseVector();
	void ParseExtraAttributes(std::map<std::string, ExtraAttribute>& attributes);

	std::map<MapToken::Type, std::string> TokenNames() const override;

private:
	enum EntityType
	{
		ENTITY_LAYER,
		ENTITY_GROUP,
		ENTITY_WORLDSPAWN,
		ENTITY_DEFAULT
	};

	void BeginEntity(size_t line, const std::vector<EntityAttribute>& attributes,
		const std::map<std::string, ExtraAttribute>& extra_attributes);
	void EndEntity(size_t start_line, size_t line_count);
	void BeginBrush(size_t line);
	void EndBrush(size_t start_line, size_t line_count,
		const std::map<std::string, ExtraAttribute>& extra_attributes);

	EntityType GetEntityType(const std::vector<EntityAttribute>& attributes) const;

private:
	MapTokenizer    m_tokenizer;
	MapFormat::Type m_format;

	std::vector<std::unique_ptr<MapEntity>> m_entities;
	int m_world_entry_idx = -1;

	MapEntity* m_curr_entity = nullptr;
	std::vector<MapFace> m_curr_faces;

	typedef MapTokenizer::Token Token;

}; // MapParser

}