// A minimal JSON writer for the match report and the raw event stream. Output is
// deterministic: keys stay in the order written, and numbers have a fixed format, so the same
// match always produces the same bytes (and the same raw_events_sha256).
#pragma once

#include "Kinetic/Api.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Kinetic
{
class KINETICCORE_API FJsonWriter
{
public:
	FJsonWriter& BeginObject();
	FJsonWriter& EndObject();
	FJsonWriter& BeginArray();
	FJsonWriter& EndArray();
	FJsonWriter& Key(std::string_view Name);

	FJsonWriter& String(std::string_view Value);  // Invalid UTF-8 and NUL become U+FFFD
	FJsonWriter& Integer(int64_t Value);
	// Rounded to Decimals places (at most 9), trailing zeros trimmed: 2.5, 184200, 0.001. From
	// 1e15 up, the shortest exact form. Locale-independent. Non-finite values are written as null (JSON
	// has no NaN).
	FJsonWriter& Number(double Value, int Decimals = 3);
	FJsonWriter& Bool(bool Value);
	FJsonWriter& Null();

	const std::string& Text() const { return Out; }

private:
	void BeforeValue();

	std::string Out;
	std::vector<bool> NeedsComma;  // One per open object or array
	bool bAfterKey = false;
};
} // namespace Kinetic
