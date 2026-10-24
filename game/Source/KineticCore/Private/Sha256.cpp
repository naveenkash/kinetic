#include "Kinetic/Sha256.h"

#include <cstdio>

namespace Kinetic
{
namespace
{
constexpr std::array<uint32_t, 64> RoundConstants{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

uint32_t Rotate(uint32_t X, int N) { return (X >> N) | (X << (32 - N)); }
} // namespace

FSha256::FSha256()
    : State{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19}
{
}

void FSha256::Block(const uint8_t* Data)
{
	std::array<uint32_t, 64> W;
	for (int I = 0; I < 16; ++I)
	{
		W[I] = (uint32_t(Data[4 * I]) << 24) | (uint32_t(Data[4 * I + 1]) << 16) | (uint32_t(Data[4 * I + 2]) << 8) |
		       uint32_t(Data[4 * I + 3]);
	}
	for (int I = 16; I < 64; ++I)
	{
		const uint32_t S0 = Rotate(W[I - 15], 7) ^ Rotate(W[I - 15], 18) ^ (W[I - 15] >> 3);
		const uint32_t S1 = Rotate(W[I - 2], 17) ^ Rotate(W[I - 2], 19) ^ (W[I - 2] >> 10);
		W[I] = W[I - 16] + S0 + W[I - 7] + S1;
	}
	uint32_t A = State[0], B = State[1], C = State[2], D = State[3];
	uint32_t E = State[4], F = State[5], G = State[6], H = State[7];
	for (int I = 0; I < 64; ++I)
	{
		const uint32_t S1 = Rotate(E, 6) ^ Rotate(E, 11) ^ Rotate(E, 25);
		const uint32_t Choose = (E & F) ^ (~E & G);
		const uint32_t T1 = H + S1 + Choose + RoundConstants[I] + W[I];
		const uint32_t S0 = Rotate(A, 2) ^ Rotate(A, 13) ^ Rotate(A, 22);
		const uint32_t Majority = (A & B) ^ (A & C) ^ (B & C);
		const uint32_t T2 = S0 + Majority;
		H = G;
		G = F;
		F = E;
		E = D + T1;
		D = C;
		C = B;
		B = A;
		A = T1 + T2;
	}
	State[0] += A;
	State[1] += B;
	State[2] += C;
	State[3] += D;
	State[4] += E;
	State[5] += F;
	State[6] += G;
	State[7] += H;
}

void FSha256::Update(std::string_view Bytes)
{
	TotalBytes += Bytes.size();
	for (const char C : Bytes)
	{
		Pending[PendingBytes++] = static_cast<uint8_t>(C);
		if (PendingBytes == Pending.size())
		{
			Block(Pending.data());
			PendingBytes = 0;
		}
	}
}

std::array<uint8_t, 32> FSha256::Finish()
{
	const uint64_t Bits = TotalBytes * 8;
	Update(std::string_view("\x80", 1));
	while (PendingBytes != 56)
	{
		Update(std::string_view("\0", 1));
	}
	char Length[8];
	for (int I = 0; I < 8; ++I)
	{
		Length[I] = static_cast<char>((Bits >> (56 - 8 * I)) & 0xff);
	}
	Update(std::string_view(Length, 8));
	std::array<uint8_t, 32> Digest;
	for (int I = 0; I < 8; ++I)
	{
		for (int K = 0; K < 4; ++K)
		{
			Digest[4 * I + K] = static_cast<uint8_t>((State[I] >> (24 - 8 * K)) & 0xff);
		}
	}
	return Digest;
}

std::string FSha256::HexOf(std::string_view Bytes)
{
	FSha256 Hash;
	Hash.Update(Bytes);
	std::string Hex;
	for (const uint8_t Byte : Hash.Finish())
	{
		char Pair[3];
		std::snprintf(Pair, sizeof(Pair), "%02x", Byte);
		Hex += Pair;
	}
	return Hex;
}
} // namespace Kinetic
