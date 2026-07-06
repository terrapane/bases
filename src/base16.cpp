/*
 *  base16.cpp
 *
 *  Copyright (C) 2024, 2026
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements functions to encode data as Base16 strings and
 *      decode those strings back to binary data (see IETF RFC 4648).
 *
 *  Portability Issues:
 *      Requires C++20 or later.
 */

#include <cstdint>
#include <climits>
#include <array>
#include <vector>
#include <span>
#include <string_view>
#include <string>
#include <terra/bases/base16.h>

namespace Terra::Base16
{

namespace
{

// Define the table used for converting to Base16
const std::array<char, 16> Base16Table =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
    'D', 'E', 'F'
};

// Define an value to represent an invalid Base16 character
constexpr std::uint8_t InvalidBase16Character = 255;

// Function that will tell us the integer value for any given Base16 character
consteval uint8_t B16ToInt(std::uint8_t x) noexcept
{
    // NOLINTBEGIN(readability-avoid-nested-conditional-operator)
    return (x) == '0' ?  0 : (x) == '1' ?  1 : (x) == '2' ?  2 :
           (x) == '3' ?  3 : (x) == '4' ?  4 : (x) == '5' ?  5 :
           (x) == '6' ?  6 : (x) == '7' ?  7 : (x) == '8' ?  8 :
           (x) == '9' ?  9 : (x) == 'A' ? 10 : (x) == 'B' ? 11 :
           (x) == 'C' ? 12 : (x) == 'D' ? 13 : (x) == 'E' ? 14 :
           (x) == 'F' ? 15 : (x) == 'a' ? 10 : (x) == 'b' ? 11 :
           (x) == 'c' ? 12 : (x) == 'd' ? 13 : (x) == 'e' ? 14 :
           (x) == 'f' ? 15 : InvalidBase16Character;
    // NOLINTEND(readability-avoid-nested-conditional-operator)
}

// Reverse table generator function
consteval std::array<std::uint8_t, 256> GenerateReverseTable()
{
    std::array<std::uint8_t, 256> table{};

    for (std::size_t i = 0; i < 256; i++)
    {
        table[i] = B16ToInt(static_cast<std::uint8_t>(i));
    }

    return table;
}

// Define the table for converting from Base16 characters to integer values
const std::array<std::uint8_t, 256> Base16ReverseTable = GenerateReverseTable();

} // namespace

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given binary string into base16.
 *
 *  Parameters:
 *      input [in]
 *          Binary string to be encoded as base16.
 *
 *  Returns:
 *      The base16-encoded text string.
 *
 *  Comments:
 *      None.
 */
std::string Encode(std::string_view input)
{
    // This library assumes the width of char is 8 bits
    static_assert(CHAR_BIT == 8);

    return Encode(std::span<const std::uint8_t>{
        reinterpret_cast<const uint8_t *>(input.data()),
        input.size()});
}

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given span of octets into Base16.
 *
 *  Parameters:
 *      input [in]
 *          Span of octets to be encoded as Base16.
 *
 *  Returns:
 *      The Base16-encoded text string.
 *
 *  Comments:
 *      None.
 */
std::string Encode(std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string

    // Just return an empty string if the input is empty
    if (input.empty()) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(input.size() * 2);

    // Iterate over the input string
    for (const std::uint8_t octet : input)
    {
        // Write out the two hex characters representing this octet
        output += std::span(
            Base16Table)[static_cast<std::uint8_t>(octet >> 4U) & 0x0fU];
        output += std::span(Base16Table)[(octet      ) & 0x0fU];
    }

    return output;
}

/*
 *  Decode
 *
 *  Description:
 *      This function will decode the base16-encoded string.
 *
 *  Parameters:
 *      input [in]
 *          Base16-encoded string that is to be decoded.
 *
 *  Returns:
 *      The decoded octets, which will be empty if the input string was
 *      empty or if the input string was not a properly encoded string.
 *
 *  Comments:
 *      To allow for spacing, control characters, etc., any character that is
 *      not part of the character set is silently ignored.
 *
 *      For decoding purposes, the alphabet is treated case insensitively.
 */
std::vector<std::uint8_t> Decode(std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output octets
    std::uint_fast32_t group = 0;               // Current bit group
    std::uint_fast32_t group_size = 0;          // How many bits in group

    // Just return an empty octet vector if the input is empty
    if (input.empty()) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(input.size() / 2);

    // Iterate over the input string
    for (const char c : input)
    {
        // Determine if we have a valid Base16 character
        std::uint8_t octet =
            std::span(Base16ReverseTable)[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase16Character) continue;

        // Shift the group by 4 bits (no effect if group == 0)
        group <<= 4U;

        // Add these 4 bits to the group
        group |= octet & 0x0fU;

        // Increment the group size
        group_size += 4;

        // Do we have a full octet?
        if (group_size == 8)
        {
            // Append the octet to the output vector
            output.push_back(group & 0xffU);

            // Reset group data
            group_size = 0;
        }
    }

    // If there is a partial group (i.e., 4 bits remaining), that is an error
    if (group_size > 0) return {};

    return output;
}

} // namespace Terra::Base16
