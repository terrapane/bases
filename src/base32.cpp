/*
 *  base32.cpp
 *
 *  Copyright (C) 2024, 2026
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements functions to encode data as Base32 strings and
 *      decode those strings back to binary data (see IETF RFC 4648).
 *
 *  Portability Issues:
 *      Requires C++20 or later.
 */

#include <cstdint>
#include <cstddef>
#include <limits>
#include <climits>
#include <array>
#include <vector>
#include <span>
#include <string>
#include <string_view>
#include <terra/bases/base32.h>

namespace Terra::Base32
{

namespace
{

// Define the table used for converting to Base32
const std::array<char, 32> Base32Table =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '2', '3', '4', '5', '6', '7'
};

// Define the padding octet
constexpr char Base32PaddingCharacter = '=';

// Define an value to represent an invalid Base32 character
constexpr std::uint8_t InvalidBase32Character = 255;

// Function that will tell us the integer value for any given Base32 character
consteval std::uint8_t B32ToInt(std::uint8_t x) noexcept
{
    // NOLINTBEGIN(readability-avoid-nested-conditional-operator)
    return (x) == 'A' ?  0 : (x) == 'B' ?  1 : (x) == 'C' ?  2 :
           (x) == 'D' ?  3 : (x) == 'E' ?  4 : (x) == 'F' ?  5 :
           (x) == 'G' ?  6 : (x) == 'H' ?  7 : (x) == 'I' ?  8 :
           (x) == 'J' ?  9 : (x) == 'K' ? 10 : (x) == 'L' ? 11 :
           (x) == 'M' ? 12 : (x) == 'N' ? 13 : (x) == 'O' ? 14 :
           (x) == 'P' ? 15 : (x) == 'Q' ? 16 : (x) == 'R' ? 17 :
           (x) == 'S' ? 18 : (x) == 'T' ? 19 : (x) == 'U' ? 20 :
           (x) == 'V' ? 21 : (x) == 'W' ? 22 : (x) == 'X' ? 23 :
           (x) == 'Y' ? 24 : (x) == 'Z' ? 25 : (x) == 'a' ?  0 :
           (x) == 'b' ?  1 : (x) == 'c' ?  2 : (x) == 'd' ?  3 :
           (x) == 'e' ?  4 : (x) == 'f' ?  5 : (x) == 'g' ?  6 :
           (x) == 'h' ?  7 : (x) == 'i' ?  8 : (x) == 'j' ?  9 :
           (x) == 'k' ? 10 : (x) == 'l' ? 11 : (x) == 'm' ? 12 :
           (x) == 'n' ? 13 : (x) == 'o' ? 14 : (x) == 'p' ? 15 :
           (x) == 'q' ? 16 : (x) == 'r' ? 17 : (x) == 's' ? 18 :
           (x) == 't' ? 19 : (x) == 'u' ? 20 : (x) == 'v' ? 21 :
           (x) == 'w' ? 22 : (x) == 'x' ? 23 : (x) == 'y' ? 24 :
           (x) == 'z' ? 25 : (x) == '2' ? 26 : (x) == '3' ? 27 :
           (x) == '4' ? 28 : (x) == '5' ? 29 : (x) == '6' ? 30 :
           (x) == '7' ? 31 : InvalidBase32Character;
    // NOLINTEND(readability-avoid-nested-conditional-operator)
}

// Reverse table generator function
consteval std::array<std::uint8_t, 256> GenerateReverseTable()
{
    std::array<std::uint8_t, 256> table{};

    for (std::size_t i = 0; i < 256; i++)
    {
        table[i] = B32ToInt(static_cast<std::uint8_t>(i));
    }

    return table;
}

// Define the table for converting from Base32 characters to integer values
const std::array<std::uint8_t, 256> Base32ReverseTable = GenerateReverseTable();

} // namespace

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given binary string into Base32.
 *
 *  Parameters:
 *      input [in]
 *          Binary string to be encoded as Base32.
 *
 *  Returns:
 *      The Base32-encoded text string.
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
 *      This function will encode the span of octets into Base32.
 *
 *  Parameters:
 *      input [in]
 *          Span of octets to be encoded as Base32.
 *
 *  Returns:
 *      The Base32-encoded text string.
 *
 *  Comments:
 *      None.
 */
std::string Encode(std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string
    std::size_t group = 0;                      // Current bit group
    std::size_t group_size = 0;                 // How many bits in group
    std::size_t quantum = 0;                    // 5-bit groups outputted

    // Just return an empty string if the input is empty
    if (input.empty()) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(((input.size() / 5) + ((input.size() % 5 > 0) ? 1 : 0)) * 8);

    // Iterate over the input string
    for (const std::uint8_t octet : input)
    {
        // Shift the group 8 bits (no effect if group has no useful data bits)
        group <<= 8U;

        // Add this octet to the group
        group |= static_cast<std::uint8_t>(octet);

        // Increment the group size to represents the number of data bits
        group_size += 8;

        while (group_size >= 5)
        {
            // Convert the top most significant 5 bits using the Base32Table,
            // appending the Base32 character to the string
            output +=
                std::span(Base32Table)[(group >> (group_size - 5)) & 0x1fU];

            // Note that 5 bits were outputted
            quantum++;

            // Reduce the group size to be 5 bits less
            group_size -= 5;

        }

        // Reset quantum if all 40 bits of the current group were written
        if (quantum == 8) quantum = 0;
    }

    // Do we have a partial group to consider?
    if (group_size > 0)
    {
        // Shift the group so that there is an integral number of 5-bits
        group <<= 5 - (group_size % 5);

        // Convert the residual 5 bits using the Base32Table, appending the
        // Base32 character to the string
        output += std::span(Base32Table)[group & 0x1fU];

        // Note that 5 bits were outputted
        quantum++;

        // Add padding characters as required
        if (quantum < 8) output.append(8 - quantum, Base32PaddingCharacter);
    }

    return output;
}

/*
 *  Decode
 *
 *  Description:
 *      This function will decode the Base32-encoded string.
 *
 *  Parameters:
 *      input [in]
 *          Base32-encoded string that is to be decoded.
 *
 *  Returns:
 *      The decoded octets, which will be empty if the input string was
 *      empty or if the input string was not a properly encoded string.
 *
 *  Comments:
 *      The '=' padding character(s) may be missing from the end of the string,
 *      since some implementations fail to add those.  Decoding will cease once
 *      padding characters are encountered and any residual data in the input
 *      string is ignored and not counted as an error.
 *
 *      To allow for spacing, control characters, etc., any character that is
 *      not part of the character set is silently ignored.
 *
 *      For decoding purposes, the alphabet is treated case insensitively.
 */
std::vector<std::uint8_t> Decode(std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output string
    std::uint_fast32_t group = 0;               // Current bit group
    std::uint_fast32_t group_size = 0;          // How many bits in group

    // Just return an empty octet vector if the input is empty
    if (input.empty()) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(
        static_cast<std::size_t>(static_cast<double>(input.size()) / 1.6));

    // Iterate over the input string
    for (const char c : input)
    {
        // Terminate the loop if we find a padding character
        if (c == Base32PaddingCharacter) break;

        // Determine if we have a valid Base32 character
        std::uint8_t octet =
            std::span(Base32ReverseTable)[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase32Character) continue;

        // Shift the group by 5 bits (no effect if group == 0)
        group <<= 5U;

        // Add these 5 bits to the group
        group |= octet & 0x1fU;

        // Increment the group size
        group_size += 5;

        // Do we have at least an octet in the group?
        if (group_size >= 8)
        {
            // Append the octet to the output vector
            output.push_back((group >> (group_size - 8)) & 0xffU);

            // Adjust the group size value
            group_size -= 8;
        }
    }

    // Do we have a partial group to consider?
    if (group_size > 0)
    {
        // Create a bit mask of all ones
        std::uint_fast32_t mask = std::numeric_limits<uint_fast32_t>::max();

        // Shift the mask by the number of bits in the residual group
        mask <<= group_size;

        // What is remaining should only be padding bits having value 0; verify
        if ((group & (~mask)) != 0) return {};
    }

    return output;
}

} // namespace Terra::Base32
