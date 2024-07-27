/*
 *  base64.cpp
 *
 *  Copyright (C) 2024
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
#include <climits>
#include <terra/bases/base64.h>

namespace Terra::Base64
{

// Define the table used for converting to Base64
static const char Base64Table[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

// Define the padding octet
static constexpr char Base64PaddingCharacter = '=';

// Define an value to represent an invalid Base64 character
static constexpr std::uint8_t InvalidBase64Character = 255;

// Use the C pre-processor to define a macro that will tell us the integer
// value for any given Base64 character
#define B64ToInt(x) ( \
    (x) == 'A' ?  0 : (x) == 'B' ?  1 : (x) == 'C' ?  2 : (x) == 'D' ?  3 : \
    (x) == 'E' ?  4 : (x) == 'F' ?  5 : (x) == 'G' ?  6 : (x) == 'H' ?  7 : \
    (x) == 'I' ?  8 : (x) == 'J' ?  9 : (x) == 'K' ? 10 : (x) == 'L' ? 11 : \
    (x) == 'M' ? 12 : (x) == 'N' ? 13 : (x) == 'O' ? 14 : (x) == 'P' ? 15 : \
    (x) == 'Q' ? 16 : (x) == 'R' ? 17 : (x) == 'S' ? 18 : (x) == 'T' ? 19 : \
    (x) == 'U' ? 20 : (x) == 'V' ? 21 : (x) == 'W' ? 22 : (x) == 'X' ? 23 : \
    (x) == 'Y' ? 24 : (x) == 'Z' ? 25 : (x) == 'a' ? 26 : (x) == 'b' ? 27 : \
    (x) == 'c' ? 28 : (x) == 'd' ? 29 : (x) == 'e' ? 30 : (x) == 'f' ? 31 : \
    (x) == 'g' ? 32 : (x) == 'h' ? 33 : (x) == 'i' ? 34 : (x) == 'j' ? 35 : \
    (x) == 'k' ? 36 : (x) == 'l' ? 37 : (x) == 'm' ? 38 : (x) == 'n' ? 39 : \
    (x) == 'o' ? 40 : (x) == 'p' ? 41 : (x) == 'q' ? 42 : (x) == 'r' ? 43 : \
    (x) == 's' ? 44 : (x) == 't' ? 45 : (x) == 'u' ? 46 : (x) == 'v' ? 47 : \
    (x) == 'w' ? 48 : (x) == 'x' ? 49 : (x) == 'y' ? 50 : (x) == 'z' ? 51 : \
    (x) == '0' ? 52 : (x) == '1' ? 53 : (x) == '2' ? 54 : (x) == '3' ? 55 : \
    (x) == '4' ? 56 : (x) == '5' ? 57 : (x) == '6' ? 58 : (x) == '7' ? 59 : \
    (x) == '8' ? 60 : (x) == '9' ? 61 : (x) == '+' ? 62 : (x) == '/' ? 63 : \
    InvalidBase64Character)

// Define the table for converting from Base64 characters to integer values
static const std::uint8_t Base64ReverseTable[256] =
{
    B64ToInt(0),   B64ToInt(1),   B64ToInt(2),   B64ToInt(3),   B64ToInt(4),
    B64ToInt(5),   B64ToInt(6),   B64ToInt(7),   B64ToInt(8),   B64ToInt(9),
    B64ToInt(10),  B64ToInt(11),  B64ToInt(12),  B64ToInt(13),  B64ToInt(14),
    B64ToInt(15),  B64ToInt(16),  B64ToInt(17),  B64ToInt(18),  B64ToInt(19),
    B64ToInt(20),  B64ToInt(21),  B64ToInt(22),  B64ToInt(23),  B64ToInt(24),
    B64ToInt(25),  B64ToInt(26),  B64ToInt(27),  B64ToInt(28),  B64ToInt(29),
    B64ToInt(30),  B64ToInt(31),  B64ToInt(32),  B64ToInt(33),  B64ToInt(34),
    B64ToInt(35),  B64ToInt(36),  B64ToInt(37),  B64ToInt(38),  B64ToInt(39),
    B64ToInt(40),  B64ToInt(41),  B64ToInt(42),  B64ToInt(43),  B64ToInt(44),
    B64ToInt(45),  B64ToInt(46),  B64ToInt(47),  B64ToInt(48),  B64ToInt(49),
    B64ToInt(50),  B64ToInt(51),  B64ToInt(52),  B64ToInt(53),  B64ToInt(54),
    B64ToInt(55),  B64ToInt(56),  B64ToInt(57),  B64ToInt(58),  B64ToInt(59),
    B64ToInt(60),  B64ToInt(61),  B64ToInt(62),  B64ToInt(63),  B64ToInt(64),
    B64ToInt(65),  B64ToInt(66),  B64ToInt(67),  B64ToInt(68),  B64ToInt(69),
    B64ToInt(70),  B64ToInt(71),  B64ToInt(72),  B64ToInt(73),  B64ToInt(74),
    B64ToInt(75),  B64ToInt(76),  B64ToInt(77),  B64ToInt(78),  B64ToInt(79),
    B64ToInt(80),  B64ToInt(81),  B64ToInt(82),  B64ToInt(83),  B64ToInt(84),
    B64ToInt(85),  B64ToInt(86),  B64ToInt(87),  B64ToInt(88),  B64ToInt(89),
    B64ToInt(90),  B64ToInt(91),  B64ToInt(92),  B64ToInt(93),  B64ToInt(94),
    B64ToInt(95),  B64ToInt(96),  B64ToInt(97),  B64ToInt(98),  B64ToInt(99),
    B64ToInt(100), B64ToInt(101), B64ToInt(102), B64ToInt(103), B64ToInt(104),
    B64ToInt(105), B64ToInt(106), B64ToInt(107), B64ToInt(108), B64ToInt(109),
    B64ToInt(110), B64ToInt(111), B64ToInt(112), B64ToInt(113), B64ToInt(114),
    B64ToInt(115), B64ToInt(116), B64ToInt(117), B64ToInt(118), B64ToInt(119),
    B64ToInt(120), B64ToInt(121), B64ToInt(122), B64ToInt(123), B64ToInt(124),
    B64ToInt(125), B64ToInt(126), B64ToInt(127), B64ToInt(128), B64ToInt(129),
    B64ToInt(130), B64ToInt(131), B64ToInt(132), B64ToInt(133), B64ToInt(134),
    B64ToInt(135), B64ToInt(136), B64ToInt(137), B64ToInt(138), B64ToInt(139),
    B64ToInt(140), B64ToInt(141), B64ToInt(142), B64ToInt(143), B64ToInt(144),
    B64ToInt(145), B64ToInt(146), B64ToInt(147), B64ToInt(148), B64ToInt(149),
    B64ToInt(150), B64ToInt(151), B64ToInt(152), B64ToInt(153), B64ToInt(154),
    B64ToInt(155), B64ToInt(156), B64ToInt(157), B64ToInt(158), B64ToInt(159),
    B64ToInt(160), B64ToInt(161), B64ToInt(162), B64ToInt(163), B64ToInt(164),
    B64ToInt(165), B64ToInt(166), B64ToInt(167), B64ToInt(168), B64ToInt(169),
    B64ToInt(170), B64ToInt(171), B64ToInt(172), B64ToInt(173), B64ToInt(174),
    B64ToInt(175), B64ToInt(176), B64ToInt(177), B64ToInt(178), B64ToInt(179),
    B64ToInt(180), B64ToInt(181), B64ToInt(182), B64ToInt(183), B64ToInt(184),
    B64ToInt(185), B64ToInt(186), B64ToInt(187), B64ToInt(188), B64ToInt(189),
    B64ToInt(190), B64ToInt(191), B64ToInt(192), B64ToInt(193), B64ToInt(194),
    B64ToInt(195), B64ToInt(196), B64ToInt(197), B64ToInt(198), B64ToInt(199),
    B64ToInt(200), B64ToInt(201), B64ToInt(202), B64ToInt(203), B64ToInt(204),
    B64ToInt(205), B64ToInt(206), B64ToInt(207), B64ToInt(208), B64ToInt(209),
    B64ToInt(210), B64ToInt(211), B64ToInt(212), B64ToInt(213), B64ToInt(214),
    B64ToInt(215), B64ToInt(216), B64ToInt(217), B64ToInt(218), B64ToInt(219),
    B64ToInt(220), B64ToInt(221), B64ToInt(222), B64ToInt(223), B64ToInt(224),
    B64ToInt(225), B64ToInt(226), B64ToInt(227), B64ToInt(228), B64ToInt(229),
    B64ToInt(230), B64ToInt(231), B64ToInt(232), B64ToInt(233), B64ToInt(234),
    B64ToInt(235), B64ToInt(236), B64ToInt(237), B64ToInt(238), B64ToInt(239),
    B64ToInt(240), B64ToInt(241), B64ToInt(242), B64ToInt(243), B64ToInt(244),
    B64ToInt(245), B64ToInt(246), B64ToInt(247), B64ToInt(248), B64ToInt(249),
    B64ToInt(250), B64ToInt(251), B64ToInt(252), B64ToInt(253), B64ToInt(254),
    B64ToInt(255)
};

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
std::string Encode(const std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string
    std::size_t group = 0;                      // Group of 24 bits
    std::size_t group_size = 0;                 // How many bits in group

    // Just return an empty string if the input is empty
    if (input.size() == 0) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve((((4 * input.size()) / 3) + 3) & ~std::size_t(3));

    // Iterate over the input string to form 24-bit groups
    for (const std::uint8_t octet : input)
    {
        // Shift the group 8 bits (no effect if group == 0)
        group <<= 8;

        // Add this octet to the group
        group |= octet;

        // Increment the group size to represents the number of data bits
        group_size += 8;

        // Check if the group is full
        if (group_size == 24)
        {
            // Convert 6 bits at a time using the Base64Table, appending Base64
            // characters to the string for each of the 6 bits
            output += Base64Table[(group >> 18) & 0x3f];
            output += Base64Table[(group >> 12) & 0x3f];
            output += Base64Table[(group >> 6 ) & 0x3f];
            output += Base64Table[(group      ) & 0x3f];

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
        output += Base64Table[(group >> 18) & 0x3f];
        output += Base64Table[(group >> 12) & 0x3f];
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
            output += Base64Table[(group >> 6) & 0x3f];
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
std::vector<std::uint8_t> Decode(const std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output octets
    std::uint_fast32_t group = 0;               // Group of 24 bits
    std::uint_fast32_t group_size = 0;          // How many bits in group

    // Just return an empty string if the input is empty
    if (input.size() == 0) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(((3 * input.size()) / 4) + 1);

    // Iterate over the input span
    for (const char c : input)
    {
        // Terminate the loop if we find a padding character
        if (c == Base64PaddingCharacter) break;

        // Determine if we have a valid Base64 character
        std::uint8_t octet = Base64ReverseTable[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase64Character) continue;

        // Shift the group by 6 bits (no effect if group == 0)
        group <<= 6;

        // Add these 6 bits to the group
        group |= (octet & 0x3f);

        // Increment the group size to represents the number of data bits
        group_size += 6;

        // Check if the group is full
        if (group_size == 24)
        {
            // Append the octets to the output string
            output.push_back((group >> 16) & 0xff);
            output.push_back((group >>  8) & 0xff);
            output.push_back((group      ) & 0xff);

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

        // Append the octets to the output string
        output.push_back((group >> 16) & 0xff);
        if (group_size >= 16)
        {
            output.push_back((group >> 8) & 0xff);
            if (group_size == 24) output.push_back((group) & 0xff);
        }
    }

    return output;
}

} // namespace Terra::Base64
