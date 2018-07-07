#include "quake/MapParser.h"
#include "quake/MapAttributes.h"
#include "quake/TextureManager.h"

#include <lexer/Exception.h>

#include <set>
#include <algorithm>

#include <assert.h>

#define SwitchFallthrough() [[fallthrough]]

namespace
{

const std::string NoTextureName = "__TB_empty";

const float SCALE = 0.01f;

}

namespace quake
{

//////////////////////////////////////////////////////////////////////////
// class MapTokenizer
//////////////////////////////////////////////////////////////////////////

MapTokenizer::MapTokenizer(const std::string& str)
	: lexer::Tokenizer<MapToken::Type>(str, "\"", '\\')
	, m_skip_eol(true)
{
}

void MapTokenizer::SetSkipEol(bool skip_eol)
{
	m_skip_eol = skip_eol;
}

lexer::Tokenizer<MapToken::Type>::Token MapTokenizer::EmitToken()
{
    while (!Eof())
	{
        size_t start_line   = Line();
        size_t start_column = Column();
        const char* c = CurPos();
        switch (*c)
		{
            case '/':
                Advance();
                if (CurChar() == '/')
				{
                    Advance();
                    if (CurChar() == '/') {
                        Advance();
                        return Token(MapToken::Comment, c, c+3, Offset(c), start_line, start_column);
                    }
                    DiscardUntil("\n\r");
                }
                break;
            case '{':
                Advance();
                return Token(MapToken::OBrace, c, c+1, Offset(c), start_line, start_column);
            case '}':
                Advance();
                return Token(MapToken::CBrace, c, c+1, Offset(c), start_line, start_column);
            case '(':
                Advance();
                return Token(MapToken::OParenthesis, c, c+1, Offset(c), start_line, start_column);
            case ')':
                Advance();
                return Token(MapToken::CParenthesis, c, c+1, Offset(c), start_line, start_column);
            case '[':
                Advance();
                return Token(MapToken::OBracket, c, c+1, Offset(c), start_line, start_column);
            case ']':
                Advance();
                return Token(MapToken::CBracket, c, c+1, Offset(c), start_line, start_column);
            case '"': { // quoted string
                Advance();
                c = CurPos();
                const char* e = ReadQuotedString('"', "\n}");
                return Token(MapToken::String, c, e, Offset(c), start_line, start_column);
            }
            case '\n':
                if (!m_skip_eol) {
                    Advance();
                    return Token(MapToken::Eol, c, c+1, Offset(c), start_line, start_column);
                }
//                SwitchFallthrough();
            case '\r':
            case ' ':
            case '\t':
                DiscardWhile(Whitespace());
                break;
            default: { // whitespace, integer, decimal or word
                const char* e = ReadInteger(NumberDelim());
				if (e != nullptr) {
					return Token(MapToken::Integer, c, e, Offset(c), start_line, start_column);
				}

                e = ReadDecimal(NumberDelim());
				if (e != nullptr) {
					return Token(MapToken::Decimal, c, e, Offset(c), start_line, start_column);
				}

                e = ReadUntil(Whitespace());
				if (e == nullptr) {
					throw lexer::ParserException(start_line, start_column, "Unexpected character: " + std::string(c, 1));
				}
                return Token(MapToken::String, c, e, Offset(c), start_line, start_column);
            }
        }
    }
    return Token(MapToken::Eof, nullptr, nullptr, Length(), Line(), Column());
}

const std::string& MapTokenizer::NumberDelim()
{
	static const std::string number_delim(Whitespace() + ")");
	return number_delim;
}

//////////////////////////////////////////////////////////////////////////
// class MapParser
//////////////////////////////////////////////////////////////////////////

MapParser::MapParser(const std::string& str)
	: m_tokenizer(MapTokenizer(str))
	, m_format(MapFormat::Unknown)
{
}

MapParser::~MapParser()
{
}

void MapParser::Parse()
{
	ParseEntities(MapFormat::Quake2);
}

const MapEntity* MapParser::GetWorldEntity() const
{
	return m_world_entry_idx >= 0 ?
		m_entities[m_world_entry_idx].get() : nullptr;
}

void MapParser::UpdateFaceTextures()
{
	std::set<std::string> err;

	auto tex_mgr = TextureManager::Instance();
	for (auto& e : m_entities) {
		for (auto& b : e->brushes) {
			for (auto& f : b.faces) {
				tex_mgr->Query(f.tex_name);
			}
		}
	}
}

void MapParser::ParseEntities(MapFormat::Type format)
{
	SetFormat(format);

	Token token = m_tokenizer.PeekToken();
	while (token.GetType() != MapToken::Eof)
	{
		Expect(MapToken::OBrace, token);
		ParseEntity();
		token = m_tokenizer.PeekToken();
	}

	// set m_world_entry_idx
	for (int i = 0, n = m_entities.size(); i < n; ++i) {
		if (GetEntityType(m_entities[i]->attributes) == ENTITY_WORLDSPAWN) {
			m_world_entry_idx = i;
			break;
		}
	}
	assert(m_world_entry_idx >= 0);
}

void MapParser::ParseBrushes(MapFormat::Type format)
{
	SetFormat(format);

	Token token = m_tokenizer.PeekToken();
	while (token.GetType() != MapToken::Eof)
	{
		Expect(MapToken::OBrace, token);
		ParseBrush();
		token = m_tokenizer.PeekToken();
	}
}

void MapParser::ParseBrushFaces(MapFormat::Type format)
{
	SetFormat(format);

	Token token = m_tokenizer.PeekToken();
	while (token.GetType() != MapToken::Eof)
	{
		Expect(MapToken::OParenthesis, token);
		ParseFace();
		token = m_tokenizer.PeekToken();
	}
}

void MapParser::Reset()
{
	m_tokenizer.Reset();
}

void MapParser::SetFormat(MapFormat::Type format)
{
	assert(format != MapFormat::Unknown);
	m_format = format;
}

void MapParser::ParseEntity()
{
	Token token = m_tokenizer.NextToken();
	if (token.GetType() == MapToken::Eof) {
		return;
	}

	Expect(MapToken::OBrace, token);

	bool begin_entity_called = false;

	std::vector<EntityAttribute> attributes;
	std::set<AttributeName> attribute_names;

	std::map<std::string, ExtraAttribute> extra_attributes;
	size_t start_line = token.Line();

	token = m_tokenizer.PeekToken();
	while (token.GetType() != MapToken::Eof)
	{
		switch (token.GetType())
		{
		case MapToken::Comment:
			m_tokenizer.NextToken();
			ParseExtraAttributes(extra_attributes);
			break;
		case MapToken::String:
			ParseEntityAttribute(attributes, attribute_names);
			break;
		case MapToken::OBrace:
			if (!begin_entity_called) {
				BeginEntity(start_line, attributes, extra_attributes);
				begin_entity_called = true;
			}
			ParseBrush();
			break;
		case MapToken::CBrace:
			m_tokenizer.NextToken();
			if (!begin_entity_called) {
				BeginEntity(start_line, attributes, extra_attributes);
			}
			EndEntity(start_line, token.Line() - start_line);
			return;
		default:
			Expect(MapToken::Comment | MapToken::String | MapToken::OBrace | MapToken::CBrace, token);
		}

		token = m_tokenizer.PeekToken();
	}
}

void MapParser::ParseEntityAttribute(std::vector<EntityAttribute>& attributes,
	                                 std::set<AttributeName>& names)
{
    Token token = m_tokenizer.NextToken();
    assert(token.GetType() == MapToken::String);
    auto name = token.Data();

    auto line   = token.Line();
    auto column = token.Column();

    Expect(MapToken::String, token = m_tokenizer.NextToken());
    auto value = token.Data();

    if (names.count(name) == 0) {
        attributes.push_back(EntityAttribute(name, value));
        names.insert(name);
    } else {
//        status.warn(line, column, "Ignoring duplicate entity property '" + name + "'");
    }
}

void MapParser::ParseBrush()
{
    Token token = m_tokenizer.NextToken();
	if (token.GetType() == MapToken::Eof) {
		return;
	}

    Expect(MapToken::OBrace | MapToken::CBrace, token);
	if (token.GetType() == MapToken::CBrace) {
		return;
	}

    bool begin_brush_called = false;
	std::map<std::string, ExtraAttribute> extra_attributes;
    auto start_line = token.Line();

    token = m_tokenizer.PeekToken();
    while (token.GetType() != MapToken::Eof)
	{
        switch (token.GetType())
		{
        case MapToken::Comment:
            m_tokenizer.NextToken();
            ParseExtraAttributes(extra_attributes);
            break;
        case MapToken::OParenthesis:
            if (!begin_brush_called) {
                BeginBrush(start_line);
                begin_brush_called = true;
            }
            ParseFace();
            break;
        case MapToken::CBrace:
            m_tokenizer.NextToken();
			if (!begin_brush_called) {
				BeginBrush(start_line);
			}
            EndBrush(start_line, token.Line() - start_line, extra_attributes);
            return;
        default:
            Expect(MapToken::OParenthesis | MapToken::CParenthesis, token);
        }

        token = m_tokenizer.PeekToken();
    }
}

void MapParser::ParseFace()
{
	// todo
    sm::vec3 tex_axis_x, tex_axis_y;

    Token token = m_tokenizer.NextToken();
	if (token.GetType() == MapToken::Eof) {
		return;
	}

    const size_t line = token.Line();
    const size_t column = token.Column();

    Expect(MapToken::OParenthesis, token);
    const sm::vec3 p1 = ParseVector();
    Expect(MapToken::CParenthesis, token = m_tokenizer.NextToken());
    Expect(MapToken::OParenthesis, token = m_tokenizer.NextToken());
    const sm::vec3 p2 = ParseVector();
    Expect(MapToken::CParenthesis, token = m_tokenizer.NextToken());
    Expect(MapToken::OParenthesis, token = m_tokenizer.NextToken());
    const sm::vec3 p3 = ParseVector();
    Expect(MapToken::CParenthesis, token = m_tokenizer.NextToken());

    // texture names can contain braces etc, so we just read everything until the next opening bracket or number
    std::string texture_name = m_tokenizer.ReadAnyString(MapTokenizer::Whitespace());
	if (texture_name == NoTextureName) {
		texture_name = "";
	}

	MapFace face;
	face.vertices[0] = p1 * SCALE;
	face.vertices[1] = p2 * SCALE;
	face.vertices[2] = p3 * SCALE;
	face.tex_name = texture_name;
	std::transform(face.tex_name.begin(), face.tex_name.end(), face.tex_name.begin(), ::tolower);
    if (m_format == MapFormat::Valve)
	{
        Expect(MapToken::OBracket, m_tokenizer.NextToken());
        tex_axis_x = ParseVector();
        Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
		face.offset.x = token.ToFloat<float>();
        Expect(MapToken::CBracket, m_tokenizer.NextToken());

        Expect(MapToken::OBracket, m_tokenizer.NextToken());
        tex_axis_y = ParseVector();
        Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
		face.offset.y = token.ToFloat<float>();
        Expect(MapToken::CBracket, m_tokenizer.NextToken());
    }
	else
	{
        Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
		face.offset.x = token.ToFloat<float>();
        Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
		face.offset.y = token.ToFloat<float>();
    }

    Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
	face.angle = token.ToFloat<float>();
    Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
	face.scale.x = token.ToFloat<float>();
    Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
	face.scale.y = token.ToFloat<float>();

    // We'll be pretty lenient when parsing additional face attributes.
    if (!Check(MapToken::OParenthesis | MapToken::CBrace | MapToken::Eof, m_tokenizer.PeekToken()))
	{
        // There's more stuff - let's examine it!
        Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
        // It could be a Hexen 2 face attribute or Quake 2 content and surface flags and surface values

        if (Check(MapToken::Integer, m_tokenizer.PeekToken()))
		{
            // If there's more stuff, then it's a Quake 2 surface flags!
            const int surfaceContents = token.ToInteger<int>();
            token = m_tokenizer.NextToken(); // already checked it!
            const int surfaceFlags = token.ToInteger<int>();
            Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
            const float surfaceValue = token.ToFloat<float>();

    //        if (m_format == MapFormat::Quake2) {
				//face.setSurfaceContents(surfaceContents);
				//face.setSurfaceFlags(surfaceFlags);
				//face.setSurfaceValue(surfaceValue);
    //        }
        }
		else
		{
            // Noone seems to know what the extra face attribute in Hexen 2 maps does, so we discard it
            // const int hexenValue = token.ToInteger<int>();
        }
    }

    const sm::vec3 normal = (p3 - p1).Cross(p2 - p1).Normalized();
	if (fabs(normal.x) < FLT_EPSILON &&
		fabs(normal.y) < FLT_EPSILON &&
		fabs(normal.z) < FLT_EPSILON) {
//		status.error(line, column, "Skipping face: face points are colinear");
	} else {
		m_curr_faces.push_back(face);
	}
}

sm::vec3 MapParser::ParseVector()
{
	Token token;
	sm::vec3 vec;
	for (size_t i = 0; i < 3; i++) {
		Expect(MapToken::Integer | MapToken::Decimal, token = m_tokenizer.NextToken());
		vec[i] = token.ToFloat<float>();
	}
	return vec;
}

void MapParser::ParseExtraAttributes(std::map<std::string, ExtraAttribute>& attributes)
{
    Token token = m_tokenizer.NextToken();
    Expect(MapToken::String | MapToken::Eol | MapToken::Eof, token);
    while (token.GetType() != MapToken::Eol && token.GetType() != MapToken::Eof)
	{
        auto name = token.Data();
        Expect(MapToken::String | MapToken::Integer, token = m_tokenizer.NextToken());
        auto value = token.Data();
        auto type = token.GetType() == MapToken::String ? ExtraAttribute::Type_String : ExtraAttribute::Type_Integer;
        attributes.insert(std::make_pair(name, ExtraAttribute(type, name, value, token.Line(), token.Column())));
        Expect(MapToken::String | MapToken::Eol | MapToken::Eof, token = m_tokenizer.NextToken());
    }
}

std::map<MapToken::Type, std::string> MapParser::TokenNames() const
{
	using namespace MapToken;

	std::map<MapToken::Type, std::string> names;
	names[Integer]      = "integer";
	names[Decimal]      = "decimal";
	names[String]       = "string";
	names[OParenthesis] = "'('";
	names[CParenthesis] = "')'";
	names[OBrace]       = "'{'";
	names[CBrace]       = "'}'";
	names[OBracket]     = "'['";
	names[CBracket]     = "']'";
	names[Comment]      = "comment";
	names[Eof]          = "end of file";
	return names;
}

void MapParser::BeginEntity(size_t line, const std::vector<EntityAttribute>& attributes,
	                        const std::map<std::string, ExtraAttribute>& extra_attributes)
{
	auto entity = std::make_unique<MapEntity>();
	m_curr_entity = entity.get();
	m_entities.push_back(std::move(entity));

	m_curr_entity->attributes = attributes;
}

void MapParser::EndEntity(size_t start_line, size_t line_count)
{
	m_curr_entity->start_line = start_line;
	m_curr_entity->line_count = line_count;
	m_curr_entity = nullptr;
}

void MapParser::BeginBrush(size_t line)
{
	assert(m_curr_faces.empty());
}

void MapParser::EndBrush(size_t start_line, size_t line_count,
	                     const std::map<std::string, ExtraAttribute>& extra_attributes)
{
	m_curr_entity->brushes.emplace_back(m_curr_faces);
	m_curr_faces.clear();
}

MapParser::EntityType MapParser::GetEntityType(const std::vector<EntityAttribute>& attributes) const
{
	auto& classname = FindAttribute(attributes, AttributeNames::Classname);
	if (IsLayer(classname, attributes))
		return ENTITY_LAYER;
	if (IsGroup(classname, attributes))
		return ENTITY_GROUP;
	if (IsWorldspawn(classname, attributes))
		return ENTITY_WORLDSPAWN;
	return ENTITY_DEFAULT;
}

}