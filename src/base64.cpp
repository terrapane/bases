/*
 *  base64.cpp
 *
 *  Copyright (C) 2024, 2026
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements functions to encode data as Base64 strings and
 *      decode those strings back to binary data (see IETF RFC 4648).
 *
 *  Portability Issues:
 *      Requires C++20 or later.
 */

#include <cstdint>
#include <cstddef>
#include <climits>
#include <array>
#include <vector>
#include <span>
#include <string_view>
#include <string>
#include <terra/bases/base64.h>

namespace Terra::Base64
{

namespace
{

// Define the table used for converting to Base64
const std::array<char, 64> Base64Table =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

// Define the padding octet
constexpr char Base64PaddingCharacter = '=';

// Define an value to represent an invalid Base64 character
constexpr std::uint8_t InvalidBase64Character = 255;

// Function that will tell us the integer value for any given Base64 character
consteval std::uint8_t B64ToInt(std::uint8_t x) noexcept
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
           (x) == 'Y' ? 24 : (x) == 'Z' ? 25 : (x) == 'a' ? 26 :
           (x) == 'b' ? 27 : (x) == 'c' ? 28 : (x) == 'd' ? 29 :
           (x) == 'e' ? 30 : (x) == 'f' ? 31 : (x) == 'g' ? 32 :
           (x) == 'h' ? 33 : (x) == 'i' ? 34 : (x) == 'j' ? 35 :
           (x) == 'k' ? 36 : (x) == 'l' ? 37 : (x) == 'm' ? 38 :
           (x) == 'n' ? 39 : (x) == 'o' ? 40 : (x) == 'p' ? 41 :
           (x) == 'q' ? 42 : (x) == 'r' ? 43 : (x) == 's' ? 44 :
           (x) == 't' ? 45 : (x) == 'u' ? 46 : (x) == 'v' ? 47 :
           (x) == 'w' ? 48 : (x) == 'x' ? 49 : (x) == 'y' ? 50 :
           (x) == 'z' ? 51 : (x) == '0' ? 52 : (x) == '1' ? 53 :
           (x) == '2' ? 54 : (x) == '3' ? 55 : (x) == '4' ? 56 :
           (x) == '5' ? 57 : (x) == '6' ? 58 : (x) == '7' ? 59 :
           (x) == '8' ? 60 : (x) == '9' ? 61 : (x) == '+' ? 62 :
           (x) == '/' ? 63 : InvalidBase64Character;
    // NOLINTEND(readability-avoid-nested-conditional-operator)
}

// Reverse table generator function
consteval std::array<std::uint8_t, 256> GenerateReverseTable()
{
    std::array<std::uint8_t, 256> table{};

    for (std::size_t i = 0; i < 256; i++)
    {
        table[i] = B64ToInt(static_cast<std::uint8_t>(i));
    }

    return table;
}

// Define the table for converting from Base64 characters to integer values
const std::array<std::uint8_t, 256> Base64ReverseTable = GenerateReverseTable();

} // namespace

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given string into Base64.
 *
 *  Parameters:
 *      input [in]
 *          String to be encoded as Base64.
 *
 *  Returns:
 *      The Base64-encoded text string.
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
 *      This function will encode the span of octets into Base64.
 *
 *  Parameters:
 *      input [in]
 *          Span of octets to be encoded as Base64.
 *
 *  Returns:
 *      The Base64-encoded text string.
 *
 *  Comments:
 *      None.
 */
std::string Encode(std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string
    std::size_t group = 0;                      // Group of 24 bits
    std::size_t group_size = 0;                 // How many bits in group

    // Define a span over the Base64Table
    const auto table_span = std::span(Base64Table);

    // Just return an empty string if the input is empty
    if (input.empty()) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve((((4 * input.size()) / 3) + 3) &
                   ~static_cast<std::size_t>(3));

    // Iterate over the input string to form 24-bit groups
    for (const std::uint8_t octet : input)
    {
        // Shift the group 8 bits (no effect if group == 0)
        group <<= 8U;

        // Add this octet to the group
        group |= octet;

        // Increment the group size to represents the number of data bits
        group_size += 8;

        // Check if the group is full
        if (group_size == 24)
        {
            // Convert 6 bits at a time using the Base64Table, appending Base64
            // characters to the string for each of the 6 bits
            output += table_span[(group >> 18U) & 0x3fU];
            output += table_span[(group >> 12U) & 0x3fU];
            output += table_span[(group >>  6U) & 0x3fU];
            output += table_span[(group       ) & 0x3fU];

            // Reset group data
            group_size = 0;
            group = 0;
        }
    }

    // Do we have a partial group to consider?
    if (group_size > 0)
    {
        // Shift the group variable so we have a full 24 bits of data
        group <<= (24 - group_size);

        // Convert 6 bits at a time using the Base64Table
        output += table_span[(group >> 18U) & 0x3fU];
        output += table_span[(group >> 12U) & 0x3fU];
        if (group_size == 8)
        {
            // We have only one residual octet, so we append two padding octets
            output += Base64PaddingCharacter;
            output += Base64PaddingCharacter;
        }
        else
        {
            // We have two residual octets, so we have an additional 6 bits
            // to output and then one padding octet
            output += table_span[(group >> 6U) & 0x3fU];
            output += Base64PaddingCharacter;
        }
    }

    return output;
}

/*
 *  Decode
 *
 *  Description:
 *      This function will decode the Base64-encoded string.
 *
 *  Parameters:
 *      input [in]
 *          Base64-encoded string that is to be decoded.
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
 */
std::vector<std::uint8_t> Decode(std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output octets
    std::uint_fast32_t group = 0;               // Group of 24 bits
    std::uint_fast32_t group_size = 0;          // How many bits in group

    // Just return an empty octet vector if the input is empty
    if (input.empty()) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(((3 * input.size()) / 4) + 1);

    // Iterate over the input string
    for (const char c : input)
    {
        // Terminate the loop if we find a padding character
        if (c == Base64PaddingCharacter) break;

        // Determine if we have a valid Base64 character
        std::uint8_t octet =
            std::span(Base64ReverseTable)[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase64Character) continue;

        // Shift the group by 6 bits (no effect if group == 0)
        group <<= 6U;

        // Add these 6 bits to the group
        group |= octet & 0x3fU;

        // Increment the group size to represents the number of data bits
        group_size += 6;

        // Check if the group is full
        if (group_size == 24)
        {
            // Append the octets to the output vector
            output.push_back((group >> 16U) & 0xffU);
            output.push_back((group >>  8U) & 0xffU);
            output.push_back((group       ) & 0xffU);

            // Reset group data
            group_size = 0;
            group = 0;
        }
    }

    // Do we have a partial group to consider?
    if (group_size > 0)
    {
        // Shift all bits in the group left, padding the group with zeros
        group <<= (24 - group_size);

        // Append the octets to the output vector
        output.push_back((group >> 16U) & 0xffU);
        if (group_size >= 16)
        {
            output.push_back((group >> 8U) & 0xffU);
            if (group_size == 24) output.push_back((group) & 0xffU);
        }
    }

    return output;
}

} // namespace Terra::Base64
