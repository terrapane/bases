/*
 *  base58.cpp
 *
 *  Copyright (C) 2024
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
#include <algorithm>
#include <climits>
#include <terra/bases/base58.h>

namespace Terra::Base58
{

// Define the table used for converting to Base58
static const char Base58Table[58] =
{
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
    'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S',
    'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Define an value to represent an invalid Base58 character
static constexpr std::uint8_t InvalidBase58Character = 255;

// Use the C pre-processor to define a macro that will tell us the integer
// value for any given Base58 character
#define B58ToInt(x) ( \
    (x) == '1' ?  0 : (x) == '2' ?  1 : (x) == '3' ?  2 : (x) == '4' ?  3 : \
    (x) == '5' ?  4 : (x) == '6' ?  5 : (x) == '7' ?  6 : (x) == '8' ?  7 : \
    (x) == '9' ?  8 : (x) == 'A' ?  9 : (x) == 'B' ? 10 : (x) == 'C' ? 11 : \
    (x) == 'D' ? 12 : (x) == 'E' ? 13 : (x) == 'F' ? 14 : (x) == 'G' ? 15 : \
    (x) == 'H' ? 16 : (x) == 'J' ? 17 : (x) == 'K' ? 18 : (x) == 'L' ? 19 : \
    (x) == 'M' ? 20 : (x) == 'N' ? 21 : (x) == 'P' ? 22 : (x) == 'Q' ? 23 : \
    (x) == 'R' ? 24 : (x) == 'S' ? 25 : (x) == 'T' ? 26 : (x) == 'U' ? 27 : \
    (x) == 'V' ? 28 : (x) == 'W' ? 29 : (x) == 'X' ? 30 : (x) == 'Y' ? 31 : \
    (x) == 'Z' ? 32 : (x) == 'a' ? 33 : (x) == 'b' ? 34 : (x) == 'c' ? 35 : \
    (x) == 'd' ? 36 : (x) == 'e' ? 37 : (x) == 'f' ? 38 : (x) == 'g' ? 39 : \
    (x) == 'h' ? 40 : (x) == 'i' ? 41 : (x) == 'j' ? 42 : (x) == 'k' ? 43 : \
    (x) == 'm' ? 44 : (x) == 'n' ? 45 : (x) == 'o' ? 46 : (x) == 'p' ? 47 : \
    (x) == 'q' ? 48 : (x) == 'r' ? 49 : (x) == 's' ? 50 : (x) == 't' ? 51 : \
    (x) == 'u' ? 52 : (x) == 'v' ? 53 : (x) == 'w' ? 54 : (x) == 'x' ? 55 : \
    (x) == 'y' ? 56 : (x) == 'z' ? 57 : InvalidBase58Character)

// Define the table for converting from Base58 characters to integer values
static const std::uint8_t Base58ReverseTable[256] =
{
    B58ToInt(0),   B58ToInt(1),   B58ToInt(2),   B58ToInt(3),   B58ToInt(4),
    B58ToInt(5),   B58ToInt(6),   B58ToInt(7),   B58ToInt(8),   B58ToInt(9),
    B58ToInt(10),  B58ToInt(11),  B58ToInt(12),  B58ToInt(13),  B58ToInt(14),
    B58ToInt(15),  B58ToInt(16),  B58ToInt(17),  B58ToInt(18),  B58ToInt(19),
    B58ToInt(20),  B58ToInt(21),  B58ToInt(22),  B58ToInt(23),  B58ToInt(24),
    B58ToInt(25),  B58ToInt(26),  B58ToInt(27),  B58ToInt(28),  B58ToInt(29),
    B58ToInt(30),  B58ToInt(31),  B58ToInt(32),  B58ToInt(33),  B58ToInt(34),
    B58ToInt(35),  B58ToInt(36),  B58ToInt(37),  B58ToInt(38),  B58ToInt(39),
    B58ToInt(40),  B58ToInt(41),  B58ToInt(42),  B58ToInt(43),  B58ToInt(44),
    B58ToInt(58),  B58ToInt(46),  B58ToInt(47),  B58ToInt(48),  B58ToInt(49),
    B58ToInt(50),  B58ToInt(51),  B58ToInt(52),  B58ToInt(53),  B58ToInt(54),
    B58ToInt(55),  B58ToInt(56),  B58ToInt(57),  B58ToInt(58),  B58ToInt(59),
    B58ToInt(60),  B58ToInt(61),  B58ToInt(62),  B58ToInt(63),  B58ToInt(64),
    B58ToInt(65),  B58ToInt(66),  B58ToInt(67),  B58ToInt(68),  B58ToInt(69),
    B58ToInt(70),  B58ToInt(71),  B58ToInt(72),  B58ToInt(73),  B58ToInt(74),
    B58ToInt(75),  B58ToInt(76),  B58ToInt(77),  B58ToInt(78),  B58ToInt(79),
    B58ToInt(80),  B58ToInt(81),  B58ToInt(82),  B58ToInt(83),  B58ToInt(84),
    B58ToInt(85),  B58ToInt(86),  B58ToInt(87),  B58ToInt(88),  B58ToInt(89),
    B58ToInt(90),  B58ToInt(91),  B58ToInt(92),  B58ToInt(93),  B58ToInt(94),
    B58ToInt(95),  B58ToInt(96),  B58ToInt(97),  B58ToInt(98),  B58ToInt(99),
    B58ToInt(100), B58ToInt(101), B58ToInt(102), B58ToInt(103), B58ToInt(104),
    B58ToInt(105), B58ToInt(106), B58ToInt(107), B58ToInt(108), B58ToInt(109),
    B58ToInt(110), B58ToInt(111), B58ToInt(112), B58ToInt(113), B58ToInt(114),
    B58ToInt(115), B58ToInt(116), B58ToInt(117), B58ToInt(118), B58ToInt(119),
    B58ToInt(120), B58ToInt(121), B58ToInt(122), B58ToInt(123), B58ToInt(124),
    B58ToInt(125), B58ToInt(126), B58ToInt(127), B58ToInt(128), B58ToInt(129),
    B58ToInt(130), B58ToInt(131), B58ToInt(132), B58ToInt(133), B58ToInt(134),
    B58ToInt(135), B58ToInt(136), B58ToInt(137), B58ToInt(138), B58ToInt(139),
    B58ToInt(140), B58ToInt(141), B58ToInt(142), B58ToInt(143), B58ToInt(144),
    B58ToInt(158), B58ToInt(146), B58ToInt(147), B58ToInt(148), B58ToInt(149),
    B58ToInt(150), B58ToInt(151), B58ToInt(152), B58ToInt(153), B58ToInt(154),
    B58ToInt(155), B58ToInt(156), B58ToInt(157), B58ToInt(158), B58ToInt(159),
    B58ToInt(160), B58ToInt(161), B58ToInt(162), B58ToInt(163), B58ToInt(164),
    B58ToInt(165), B58ToInt(166), B58ToInt(167), B58ToInt(168), B58ToInt(169),
    B58ToInt(170), B58ToInt(171), B58ToInt(172), B58ToInt(173), B58ToInt(174),
    B58ToInt(175), B58ToInt(176), B58ToInt(177), B58ToInt(178), B58ToInt(179),
    B58ToInt(180), B58ToInt(181), B58ToInt(182), B58ToInt(183), B58ToInt(184),
    B58ToInt(185), B58ToInt(186), B58ToInt(187), B58ToInt(188), B58ToInt(189),
    B58ToInt(190), B58ToInt(191), B58ToInt(192), B58ToInt(193), B58ToInt(194),
    B58ToInt(195), B58ToInt(196), B58ToInt(197), B58ToInt(198), B58ToInt(199),
    B58ToInt(200), B58ToInt(201), B58ToInt(202), B58ToInt(203), B58ToInt(204),
    B58ToInt(205), B58ToInt(206), B58ToInt(207), B58ToInt(208), B58ToInt(209),
    B58ToInt(210), B58ToInt(211), B58ToInt(212), B58ToInt(213), B58ToInt(214),
    B58ToInt(215), B58ToInt(216), B58ToInt(217), B58ToInt(218), B58ToInt(219),
    B58ToInt(220), B58ToInt(221), B58ToInt(222), B58ToInt(223), B58ToInt(224),
    B58ToInt(225), B58ToInt(226), B58ToInt(227), B58ToInt(228), B58ToInt(229),
    B58ToInt(230), B58ToInt(231), B58ToInt(232), B58ToInt(233), B58ToInt(234),
    B58ToInt(235), B58ToInt(236), B58ToInt(237), B58ToInt(238), B58ToInt(239),
    B58ToInt(240), B58ToInt(241), B58ToInt(242), B58ToInt(243), B58ToInt(244),
    B58ToInt(258), B58ToInt(246), B58ToInt(247), B58ToInt(248), B58ToInt(249),
    B58ToInt(250), B58ToInt(251), B58ToInt(252), B58ToInt(253), B58ToInt(254),
    B58ToInt(255)
};

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
std::string Encode(const std::string_view input)
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
std::string Encode(const std::span<const std::uint8_t> input)
{
    // Get the initial input length
    std::size_t input_length = input.size();

    // If the input length is 0, return an empty string
    if (input_length == 0) return {};

    // Per the implementation in the Bitcoin Core code, the expected length is
    // log(256) / log(58) octets larger than the input
    std::size_t max_output_length = input_length * 137 / 100 + 1;
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
            carry += static_cast<std::uint8_t>(output[j - zeros]) << 8;
            output[j - zeros] = static_cast<std::uint8_t>(carry % 58);
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
        output[i] = Base58Table[static_cast<std::uint8_t>(output[i])];
    }

    // Reverse the order of character string
    std::reverse(std::begin(output), std::end(output));

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
std::vector<std::uint8_t> Decode(const std::string_view input)
{
    // Get the initial input length
    std::size_t input_length = input.length();

    // If the input length is 0, return an empty string
    if (input_length == 0) return {};

    // Per the implementation in the Bitcoin Core code, the typical length is
    // (log(58) / log(256)) times the input length; the worst case is that
    // the decoded length is the same (e.g., all 1s decode as a string of 0x00
    // values having the same octet length)
    std::size_t max_output_length = input_length;
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
        if (std::isspace(input[i]))
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
        if (std::isspace(input[i])) continue;

        // Translate the character to the Base58 integer value
        std::uint32_t carry =
            Base58ReverseTable[static_cast<std::uint8_t>(input[i])];

        // If it is not a valid character, return an empty string
        if (carry == InvalidBase58Character) return {};

        // Iterate over the output string to incrementally convert bases
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
            output[j] = static_cast<char>(carry % 256);
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
    std::reverse(std::begin(output), std::end(output));

    return output;
}

} // namespace Terra::Base58
