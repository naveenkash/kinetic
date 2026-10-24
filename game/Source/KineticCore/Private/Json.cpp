#include "Kinetic/Json.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>

namespace Kinetic
{
namespace
{
constexpr double LargeNumber = 1e15;
constexpr int MaxDecimals = 9;
} // namespace

void FJsonWriter::BeforeValue()
{
	if (bAfterKey)
	{
		bAfterKey = false;
		return;
	}
	if (!NeedsComma.empty())
	{
		if (NeedsComma.back())
		{
			Out += ',';
		}
		NeedsComma.back() = true;
	}
}

FJsonWriter& FJsonWriter::BeginObject()
{
	BeforeValue();
	Out += '{';
	NeedsComma.push_back(false);
	return *this;
}

FJsonWriter& FJsonWriter::EndObject()
{
	Out += '}';
	NeedsComma.pop_back();
	return *this;
}

FJsonWriter& FJsonWriter::BeginArray()
{
	BeforeValue();
	Out += '[';
	NeedsComma.push_back(false);
	return *this;
}

FJsonWriter& FJsonWriter::EndArray()
{
	Out += ']';
	NeedsComma.pop_back();
	return *this;
}

FJsonWriter& FJsonWriter::Key(std::string_view Name)
{
	String(Name);
	Out += ':';
	bAfterKey = true;
	return *this;
}

namespace
{
// Length of the valid UTF-8 sequence starting at Bytes[I], or 0 if it is not one.
size_t Utf8Length(std::string_view Bytes, size_t I)
{
	const auto At = [&](size_t K) { return static_cast<unsigned char>(Bytes[K]); };
	const unsigned char Lead = At(I);
	size_t Length = 0;
	uint32_t Point = 0;
	if (Lead < 0x80)
	{
		return 1;
	}
	if (Lead >= 0xc2 && Lead <= 0xdf)
	{
		Length = 2;
		Point = Lead & 0x1f;
	}
	else if (Lead >= 0xe0 && Lead <= 0xef)
	{
		Length = 3;
		Point = Lead & 0x0f;
	}
	else if (Lead >= 0xf0 && Lead <= 0xf4)
	{
		Length = 4;
		Point = Lead & 0x07;
	}
	else
	{
		return 0;
	}
	if (I + Length > Bytes.size())
	{
		return 0;
	}
	for (size_t K = 1; K < Length; ++K)
	{
		if ((At(I + K) & 0xc0) != 0x80)
		{
			return 0;
		}
		Point = (Point << 6) | (At(I + K) & 0x3f);
	}
	const uint32_t Least[] = {0, 0, 0x80, 0x800, 0x10000};
	const bool bSurrogate = Point >= 0xd800 && Point <= 0xdfff;
	return Point < Least[Length] || bSurrogate || Point > 0x10ffff ? 0 : Length;
}
} // namespace

// Strings are always valid UTF-8 JSON that Postgres jsonb accepts: invalid bytes and NUL become
// U+FFFD, control characters are escaped, and U+2028 and U+2029 are escaped for JavaScript.
FJsonWriter& FJsonWriter::String(std::string_view Value)
{
	static constexpr const char* Replacement = "\xef\xbf\xbd";
	BeforeValue();
	Out += '"';
	for (size_t I = 0; I < Value.size();)
	{
		const char C = Value[I];
		const size_t Length = Utf8Length(Value, I);
		if (Length == 0 || C == '\0')
		{
			Out += Replacement;
			I += Length == 0 ? 1 : Length;
			continue;
		}
		if (Length == 3 && Value.substr(I, 2) == "\xe2\x80" && (Value[I + 2] == '\xa8' || Value[I + 2] == '\xa9'))
		{
			Out += Value[I + 2] == '\xa8' ? "\\u2028" : "\\u2029";
			I += 3;
			continue;
		}
		switch (C)
		{
		case '"':
			Out += "\\\"";
			break;
		case '\\':
			Out += "\\\\";
			break;
		case '\n':
			Out += "\\n";
			break;
		case '\r':
			Out += "\\r";
			break;
		case '\t':
			Out += "\\t";
			break;
		default:
			if (static_cast<unsigned char>(C) < 0x20)
			{
				char Escape[8];
				std::snprintf(Escape, sizeof(Escape), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(C)));
				Out += Escape;
			}
			else
			{
				Out.append(Value.substr(I, Length));
			}
		}
		I += Length;
	}
	Out += '"';
	return *this;
}

FJsonWriter& FJsonWriter::Integer(int64_t Value)
{
	BeforeValue();
	Out += std::to_string(Value);
	return *this;
}

FJsonWriter& FJsonWriter::Number(double Value, int Decimals)
{
	if (!std::isfinite(Value))
	{
		return Null();
	}
	BeforeValue();
	// to_chars rounds the exact binary value once and ignores the locale. Past 1e15 the
	// decimals carry nothing a double can hold, so the shortest exact form is written instead.
	char Buffer[400];
	const std::to_chars_result Result =
	    std::abs(Value) >= LargeNumber
	        ? std::to_chars(Buffer, Buffer + sizeof(Buffer), Value)
	        : std::to_chars(Buffer, Buffer + sizeof(Buffer), Value, std::chars_format::fixed,
	                        std::clamp(Decimals, 0, MaxDecimals));
	std::string Text(Buffer, Result.ptr);
	if (Text.find('.') != std::string::npos && Text.find('e') == std::string::npos)
	{
		Text.erase(Text.find_last_not_of('0') + 1);
		if (Text.back() == '.')
		{
			Text.pop_back();
		}
	}
	Out += Text == "-0" ? "0" : Text;
	return *this;
}

FJsonWriter& FJsonWriter::Bool(bool Value)
{
	BeforeValue();
	Out += Value ? "true" : "false";
	return *this;
}

FJsonWriter& FJsonWriter::Null()
{
	BeforeValue();
	Out += "null";
	return *this;
}
} // namespace Kinetic
