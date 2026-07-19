/*
 *  base58.cpp
 *
 *  Copyright (C) 2024, 2026
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements functions to encode and decode data to and from
 *      Base58 (as used by Bitcoin).
 *
 *  Portability Issues:
 *      Requires C++20 or later.
 */

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <climits>
#include <cctype>
#include <array>
#include <vector>
#include <span>
#include <string_view>
#include <string>

#include <terra/bases/base58.h>

namespace Terra::Base58
{

namespace
{

// Define the table used for converting to Base58
const std::array<char, 58> Base58Table =
{
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
    'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S',
    'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Define an value to represent an invalid Base58 character
constexpr std::uint8_t InvalidBase58Character = 255;

// Function that will tell us the integer value for any given Base58 character
consteval std::uint8_t B58ToInt(std::uint8_t x) noexcept
{
    // NOLINTBEGIN(readability-avoid-nested-conditional-operator)
    return (x) == '1' ?  0 : (x) == '2' ?  1 : (x) == '3' ?  2 :
           (x) == '4' ?  3 : (x) == '5' ?  4 : (x) == '6' ?  5 :
           (x) == '7' ?  6 : (x) == '8' ?  7 : (x) == '9' ?  8 :
           (x) == 'A' ?  9 : (x) == 'B' ? 10 : (x) == 'C' ? 11 :
           (x) == 'D' ? 12 : (x) == 'E' ? 13 : (x) == 'F' ? 14 :
           (x) == 'G' ? 15 : (x) == 'H' ? 16 : (x) == 'J' ? 17 :
           (x) == 'K' ? 18 : (x) == 'L' ? 19 : (x) == 'M' ? 20 :
           (x) == 'N' ? 21 : (x) == 'P' ? 22 : (x) == 'Q' ? 23 :
           (x) == 'R' ? 24 : (x) == 'S' ? 25 : (x) == 'T' ? 26 :
           (x) == 'U' ? 27 : (x) == 'V' ? 28 : (x) == 'W' ? 29 :
           (x) == 'X' ? 30 : (x) == 'Y' ? 31 : (x) == 'Z' ? 32 :
           (x) == 'a' ? 33 : (x) == 'b' ? 34 : (x) == 'c' ? 35 :
           (x) == 'd' ? 36 : (x) == 'e' ? 37 : (x) == 'f' ? 38 :
           (x) == 'g' ? 39 : (x) == 'h' ? 40 : (x) == 'i' ? 41 :
           (x) == 'j' ? 42 : (x) == 'k' ? 43 : (x) == 'm' ? 44 :
           (x) == 'n' ? 45 : (x) == 'o' ? 46 : (x) == 'p' ? 47 :
           (x) == 'q' ? 48 : (x) == 'r' ? 49 : (x) == 's' ? 50 :
           (x) == 't' ? 51 : (x) == 'u' ? 52 : (x) == 'v' ? 53 :
           (x) == 'w' ? 54 : (x) == 'x' ? 55 : (x) == 'y' ? 56 :
           (x) == 'z' ? 57 : InvalidBase58Character;
    // NOLINTEND(readability-avoid-nested-conditional-operator)
}

// Reverse table generator function
consteval std::array<std::uint8_t, 256> GenerateReverseTable()
{
    std::array<std::uint8_t, 256> table{};

    for (std::size_t i = 0; i < 256; i++)
    {
        table[i] = B58ToInt(static_cast<std::uint8_t>(i));
    }

    return table;
}

// Define the table for converting from Base58 characters to integer values
const std::array<std::uint8_t, 256> Base58ReverseTable = GenerateReverseTable();

} // namespace

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given string into Base58.
 *
 *  Parameters:
 *      input [in]
 *          String to be encoded as Base58.
 *
 *  Returns:
 *      The Base58-encoded text string.
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
 *      This function will encode the given binary string into Base58.
 *
 *  Parameters:
 *      input [in]
 *          Binary string to be encoded as Base58.
 *
 *  Returns:
 *      The Base58-encoded text string.
 *
 *  Comments:
 *      None.
 */
std::string Encode(std::span<const std::uint8_t> input)
{
    // Get the initial input length
    const std::size_t input_length = input.size();

    // If the input length is 0, return an empty string
    if (input_length == 0) return {};

    // Per the implementation in the Bitcoin Core code, the expected length is
    // log(256) / log(58) octets larger than the input
    const std::size_t max_output_length = (input_length * 137 / 100) + 1;
    std::string output(max_output_length, 0);

    // Initialize the count of leading zeros
    std::size_t zeros = 0;

    // Count the leading zeros
    for(std::size_t i = 0; (i < input_length) && (input[i] == 0); i++) zeros++;

    std::size_t output_length = zeros;

    // Iterate over the input string
    for (std::size_t i = zeros; i < input_length; i++)
    {
        std::uint32_t carry = static_cast<std::uint8_t>(input[i]);

        // Iterate over the output string to incrementally convert bases
        for (std::size_t j = zeros; j < max_output_length; j++)
        {
            // At the end of the output buffer?
            if (j == output_length)
            {
                // Break if there are no more carry bits
                if (carry == 0) break;

                // Increment the output length
                output_length++;
            }
            carry += static_cast<std::uint32_t>(
                static_cast<std::uint8_t>(output[j - zeros]) << 8U);
            output[j - zeros] =
                static_cast<char>(static_cast<std::uint8_t>(carry % 58));
            carry /= 58;
        }

        // If there are remaining carry bits, this is an error
        if (carry > 0) return {};
    }

    // Truncate the output string to the correct size (counted 0s at the end)
    output.resize(output_length);

    // Perform Base58 character substitution
    for (std::size_t i = 0; i <= output_length; i++)
    {
        output[i] =
            std::span(Base58Table)[static_cast<std::uint8_t>(output[i])];
    }

    // Reverse the order of character string
    std::ranges::reverse(output);

    return output;
}

/*
 *  Decode
 *
 *  Description:
 *      This function will decode the Base58-encoded string.
 *
 *  Parameters:
 *      input [in]
 *          Base58-encoded string that is to be decoded.
 *
 *  Returns:
 *      The decoded octets, which will be empty if the input string was
 *      empty or if the input string was not a properly encoded string.
 *
 *  Comments:
 *      To allow for whitespace and multi-line input, any whitespace character
 *      is silently ignored (including spaces, tabs, new lines, etc).
 */
std::vector<std::uint8_t> Decode(std::string_view input)
{
    // Get the initial input length
    const std::size_t input_length = input.length();

    // If the input length is 0, return an empty octet vector
    if (input_length == 0) return {};

    // Per the implementation in the Bitcoin Core code, the typical length is
    // (log(58) / log(256)) times the input length; the worst case is that
    // the decoded length is the same (e.g., all 1s decode as a string of 0x00
    // values having the same octet length)
    const std::size_t max_output_length = input_length;
    std::vector<std::uint8_t> output(max_output_length, 0);

    // Initialize the count of leading zeros
    std::size_t zeros = 0;

    // Initialize the location of the digits beyond leading zeros
    std::size_t digits_start = 0;

    // Count leading zeros (skipping over whitespace)
    for(std::size_t i = 0; i < input_length; i++)
    {
        // Zeros are encoded as '1'
        if (input[i] == '1')
        {
            zeros++;
            digits_start++;
            continue;
        }

        // Skip over whitespace
        if (std::isspace(input[i]) != 0)
        {
            digits_start++;
            continue;
        }

        // Break out of the loop when the first digit found
        break;
    }

    // Start with an assumed output length of 0
    std::size_t output_length = 0;

    // Iterate over the Base58 input string, ignoring whitespace
    for (std::size_t i = digits_start; i < input_length; i++)
    {
        // Skip over whitespace
        if (std::isspace(input[i]) != 0) continue;

        // Translate the character to the Base58 integer value
        std::uint32_t carry =
            std::span(Base58ReverseTable)[static_cast<std::uint8_t>(input[i])];

        // If it is not a valid character, return an empty string
        if (carry == InvalidBase58Character) return {};

        // Iterate over the output vector to incrementally convert bases
        for (std::size_t j = 0; j < max_output_length; j++)
        {
            // At the end of the output buffer?
            if (j == output_length)
            {
                // Break if there are no more carry bits
                if (carry == 0) break;

                // Increment the output length
                output_length++;
            }
            carry += 58 * static_cast<std::uint8_t>(output[j]);
            output[j] = static_cast<std::uint8_t>(carry % 256);
            carry /= 256;
        }

        // If there are remaining carry bits, this is an error
        if (carry > 0) return {};
    }

    // Adjust the output length
    output.resize(output_length);

    // Append the count of zeros
    if (zeros > 0) output.resize(output.size() + zeros, 0);

    // Reverse the order of the binary output
    std::ranges::reverse(output);

    return output;
}

} // namespace Terra::Base58
