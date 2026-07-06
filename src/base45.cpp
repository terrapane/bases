/*
 *  base45.cpp
 *
 *  Copyright (C) 2024, 2026
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements functions to encode data as Base45 strings and
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
#include <terra/bases/base45.h>

namespace Terra::Base45
{

namespace
{

// Define the table used for converting to Base45
const std::array<char, 45> Base45Table =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
    'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%',
    '*', '+', '-', '.', '/', ':'
};

// Define an value to represent an invalid Base45 character
constexpr std::uint8_t InvalidBase45Character = 255;

// Function that will tell us the integer value for any given Base45 character
consteval std::uint8_t B45ToInt(std::uint8_t x) noexcept
{
    // NOLINTBEGIN(readability-avoid-nested-conditional-operator)
    return (x) == '0' ?  0 : (x) == '1' ?  1 : (x) == '2' ?  2 :
           (x) == '3' ?  3 : (x) == '4' ?  4 : (x) == '5' ?  5 :
           (x) == '6' ?  6 : (x) == '7' ?  7 : (x) == '8' ?  8 :
           (x) == '9' ?  9 : (x) == 'A' ? 10 : (x) == 'B' ? 11 :
           (x) == 'C' ? 12 : (x) == 'D' ? 13 : (x) == 'E' ? 14 :
           (x) == 'F' ? 15 : (x) == 'G' ? 16 : (x) == 'H' ? 17 :
           (x) == 'I' ? 18 : (x) == 'J' ? 19 : (x) == 'K' ? 20 :
           (x) == 'L' ? 21 : (x) == 'M' ? 22 : (x) == 'N' ? 23 :
           (x) == 'O' ? 24 : (x) == 'P' ? 25 : (x) == 'Q' ? 26 :
           (x) == 'R' ? 27 : (x) == 'S' ? 28 : (x) == 'T' ? 29 :
           (x) == 'U' ? 30 : (x) == 'V' ? 31 : (x) == 'W' ? 32 :
           (x) == 'X' ? 33 : (x) == 'Y' ? 34 : (x) == 'Z' ? 35 :
           (x) == ' ' ? 36 : (x) == '$' ? 37 : (x) == '%' ? 38 :
           (x) == '*' ? 39 : (x) == '+' ? 40 : (x) == '-' ? 41 :
           (x) == '.' ? 42 : (x) == '/' ? 43 : (x) == ':' ? 44 :
           InvalidBase45Character;
    // NOLINTEND(readability-avoid-nested-conditional-operator)
}

// Reverse table generator function
consteval std::array<std::uint8_t, 256> GenerateReverseTable()
{
    std::array<std::uint8_t, 256> table{};

    for (std::size_t i = 0; i < 256; i++)
    {
        table[i] = B45ToInt(static_cast<std::uint8_t>(i));
    }

    return table;
}

// Define the table for converting from Base45 characters to integer values
const std::array<std::uint8_t, 256> Base45ReverseTable = GenerateReverseTable();

} // namespace

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given binary string into Base45.
 *
 *  Parameters:
 *      input [in]
 *          Binary string to be encoded as Base45.
 *
 *  Returns:
 *      The Base45-encoded text string.
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
 *      This function will encode the span of octets into Base45.
 *
 *  Parameters:
 *      input [in]
 *          Span of octets to be encoded as Base45.
 *
 *  Returns:
 *      The Base45-encoded text string.
 *
 *  Comments:
 *      None.
 */
std::string Encode(std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string
    std::size_t group = 0;                      // Group of 16 bits
    std::size_t group_size = 0;                 // How many octets in group

    // Define a span over the Base45Table
    const auto table_span = std::span(Base45Table);

    // Just return an empty string if the input is empty
    if (input.empty()) return {};

    // Reserve space for output
    if ((input.size() & 1U) == 0)
    {
        // Even size = input size + (input size / 2)
        output.reserve(input.size() + (input.size() >> 1U));
    }
    else
    {
        // Even size = input size + (input size / 2) + 1
        output.reserve(input.size() + (input.size() >> 1U) + 1);
    }

    // Iterate over the input string to form 16-bit groups
    for (const uint8_t octet : input)
    {
        // Shift the group 8 bits (no effect if group == 0)
        group <<= 8U;

        // Add this octet to the group
        group |= static_cast<std::uint8_t>(octet);

        // Increment the group size to represents the number of data bits
        group_size++;

        // Check if the group is full
        if (group_size == 2)
        {
            // Convert one group at a time using the Base45Table, appending
            // Base45 characters to the string for each group
            output += table_span[(group       ) % 45];
            output += table_span[(group /   45) % 45];
            output += table_span[(group / 2025) % 45];

            // Reset group data
            group_size = 0;
            group = 0;
        }
    }

    // Do we have a partial group to consider?
    if (group_size > 0)
    {
        // Convert the last group using the Base45Table, appending Base45
        // characters to the string
        output += table_span[(group     ) % 45];
        output += table_span[(group / 45) % 45];
    }

    return output;
}

/*
 *  Decode
 *
 *  Description:
 *      This function will decode the Base45-encoded string.
 *
 *  Parameters:
 *      input [in]
 *          Base45-encoded string that is to be decoded.
 *
 *  Returns:
 *      The decoded octets, which will be empty if the input string was
 *      empty or if the input string was not a properly encoded string.
 *
 *  Comments:
 *      To allow for whitespace and multi-line input, any character that is not
 *      part of the Base45 character set is silently ignored.
 *
 *      The alphabet is treated case sensitively as required by RFC 9285.
 *      Lowercase characters are ignored.
 */
std::vector<std::uint8_t> Decode(std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output string
    std::uint_fast32_t group = 0;               // Group of 24 bits
    std::uint_fast32_t group_size = 0;          // How many octets in group

    // Just return an empty octet vector if the input is empty
    if (input.empty()) return {};

    // Reserve space for output (= input size / 1.5)
    output.reserve(
        static_cast<std::size_t>(static_cast<double>(input.size()) / 1.5));

    // Iterate over the input string
    for (const char c : input)
    {
        // Determine if we have a valid Base45 character
        std::uint8_t octet =
            std::span(Base45ReverseTable)[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase45Character) continue;

        // Shift the group 8 bits (no effect if group == 0)
        group <<= 8U;

        // Add this octet to the group
        group |= octet;

        // Increment the group size to represents the number of octets
        group_size++;

        // Check if the group is full
        if (group_size == 3)
        {
            // Compute the 16-bit value represented by this group
            std::uint_fast16_t octet_pair =
                ((((group >> 16U) & 0xffU)       ) +
                 (((group >>  8U) & 0xffU) *   45) +
                 (((group       ) & 0xffU) * 2025)) & 0xffffU;

            // Append the octets to the output vector
            output.push_back((octet_pair >> 8U) & 0xffU);
            output.push_back((octet_pair      ) & 0xffU);

            // Reset group data
            group_size = 0;
            group = 0;
        }
    }

    // Do we have a partial group to consider?
    if (group_size > 0)
    {
        // Anything other than two octets would indicate a string length error
        if (group_size != 2) return {};

        output.push_back(  ((group >> 8U) & 0xffU) +
                         ((((group      ) & 0xffU) * 45) & 0xffU));
    }

    return output;
}

} // namespace Terra::Base45
