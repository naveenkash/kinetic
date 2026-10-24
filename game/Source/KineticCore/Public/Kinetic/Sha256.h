// SHA-256 (FIPS 180-4), for the match report's raw_events_sha256: the digest of the raw event
// stream the server uploads, so the backend can tell the report and the events belong together.
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace Kinetic
{
class FSha256
{
public:
	FSha256();
	void Update(std::string_view Bytes);
	std::array<uint8_t, 32> Finish();

	static std::string HexOf(std::string_view Bytes);  // Lowercase hex digest in one call

private:
	void Block(const uint8_t* Data);

	std::array<uint32_t, 8> State;
	std::array<uint8_t, 64> Pending{};
	size_t PendingBytes = 0;
	uint64_t TotalBytes = 0;
};
} // namespace Kinetic
