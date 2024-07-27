/*
 *  base32.cpp
 *
 *  Copyright (C) 2024
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
#include <limits>
#include <climits>
#include <terra/bases/base32.h>

namespace Terra::Base32
{

// Define the table used for converting to Base32
static const char Base32Table[32] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '2', '3', '4', '5', '6', '7'
};

// Define the padding octet
static constexpr char Base32PaddingCharacter = '=';

// Define an value to represent an invalid Base32 character
static constexpr std::uint8_t InvalidBase32Character = 255;

// Use the C pre-processor to define a macro that will tell us the integer
// value for any given Base32 character
#define B32ToInt(x) ( \
    (x) == 'A' ?  0 : (x) == 'B' ?  1 : (x) == 'C' ?  2 : (x) == 'D' ?  3 : \
    (x) == 'E' ?  4 : (x) == 'F' ?  5 : (x) == 'G' ?  6 : (x) == 'H' ?  7 : \
    (x) == 'I' ?  8 : (x) == 'J' ?  9 : (x) == 'K' ? 10 : (x) == 'L' ? 11 : \
    (x) == 'M' ? 12 : (x) == 'N' ? 13 : (x) == 'O' ? 14 : (x) == 'P' ? 15 : \
    (x) == 'Q' ? 16 : (x) == 'R' ? 17 : (x) == 'S' ? 18 : (x) == 'T' ? 19 : \
    (x) == 'U' ? 20 : (x) == 'V' ? 21 : (x) == 'W' ? 22 : (x) == 'X' ? 23 : \
    (x) == 'Y' ? 24 : (x) == 'Z' ? 25 : (x) == 'a' ?  0 : (x) == 'b' ?  1 : \
    (x) == 'c' ?  2 : (x) == 'd' ?  3 : (x) == 'e' ?  4 : (x) == 'f' ?  5 : \
    (x) == 'g' ?  6 : (x) == 'h' ?  7 : (x) == 'i' ?  8 : (x) == 'j' ?  9 : \
    (x) == 'k' ? 10 : (x) == 'l' ? 11 : (x) == 'm' ? 12 : (x) == 'n' ? 13 : \
    (x) == 'o' ? 14 : (x) == 'p' ? 15 : (x) == 'q' ? 16 : (x) == 'r' ? 17 : \
    (x) == 's' ? 18 : (x) == 't' ? 19 : (x) == 'u' ? 20 : (x) == 'v' ? 21 : \
    (x) == 'w' ? 22 : (x) == 'x' ? 23 : (x) == 'y' ? 24 : (x) == 'z' ? 25 : \
    (x) == '2' ? 26 : (x) == '3' ? 27 : (x) == '4' ? 28 : (x) == '5' ? 29 : \
    (x) == '6' ? 30 : (x) == '7' ? 31 : InvalidBase32Character)

// Define the table for converting from Base32 characters to integer values
static const std::uint8_t Base32ReverseTable[256] =
{
    B32ToInt(0),   B32ToInt(1),   B32ToInt(2),   B32ToInt(3),   B32ToInt(4),
    B32ToInt(5),   B32ToInt(6),   B32ToInt(7),   B32ToInt(8),   B32ToInt(9),
    B32ToInt(10),  B32ToInt(11),  B32ToInt(12),  B32ToInt(13),  B32ToInt(14),
    B32ToInt(15),  B32ToInt(16),  B32ToInt(17),  B32ToInt(18),  B32ToInt(19),
    B32ToInt(20),  B32ToInt(21),  B32ToInt(22),  B32ToInt(23),  B32ToInt(24),
    B32ToInt(25),  B32ToInt(26),  B32ToInt(27),  B32ToInt(28),  B32ToInt(29),
    B32ToInt(30),  B32ToInt(31),  B32ToInt(32),  B32ToInt(33),  B32ToInt(34),
    B32ToInt(35),  B32ToInt(36),  B32ToInt(37),  B32ToInt(38),  B32ToInt(39),
    B32ToInt(40),  B32ToInt(41),  B32ToInt(42),  B32ToInt(43),  B32ToInt(44),
    B32ToInt(45),  B32ToInt(46),  B32ToInt(47),  B32ToInt(48),  B32ToInt(49),
    B32ToInt(50),  B32ToInt(51),  B32ToInt(52),  B32ToInt(53),  B32ToInt(54),
    B32ToInt(55),  B32ToInt(56),  B32ToInt(57),  B32ToInt(58),  B32ToInt(59),
    B32ToInt(60),  B32ToInt(61),  B32ToInt(62),  B32ToInt(63),  B32ToInt(64),
    B32ToInt(65),  B32ToInt(66),  B32ToInt(67),  B32ToInt(68),  B32ToInt(69),
    B32ToInt(70),  B32ToInt(71),  B32ToInt(72),  B32ToInt(73),  B32ToInt(74),
    B32ToInt(75),  B32ToInt(76),  B32ToInt(77),  B32ToInt(78),  B32ToInt(79),
    B32ToInt(80),  B32ToInt(81),  B32ToInt(82),  B32ToInt(83),  B32ToInt(84),
    B32ToInt(85),  B32ToInt(86),  B32ToInt(87),  B32ToInt(88),  B32ToInt(89),
    B32ToInt(90),  B32ToInt(91),  B32ToInt(92),  B32ToInt(93),  B32ToInt(94),
    B32ToInt(95),  B32ToInt(96),  B32ToInt(97),  B32ToInt(98),  B32ToInt(99),
    B32ToInt(100), B32ToInt(101), B32ToInt(102), B32ToInt(103), B32ToInt(104),
    B32ToInt(105), B32ToInt(106), B32ToInt(107), B32ToInt(108), B32ToInt(109),
    B32ToInt(110), B32ToInt(111), B32ToInt(112), B32ToInt(113), B32ToInt(114),
    B32ToInt(115), B32ToInt(116), B32ToInt(117), B32ToInt(118), B32ToInt(119),
    B32ToInt(120), B32ToInt(121), B32ToInt(122), B32ToInt(123), B32ToInt(124),
    B32ToInt(125), B32ToInt(126), B32ToInt(127), B32ToInt(128), B32ToInt(129),
    B32ToInt(130), B32ToInt(131), B32ToInt(132), B32ToInt(133), B32ToInt(134),
    B32ToInt(135), B32ToInt(136), B32ToInt(137), B32ToInt(138), B32ToInt(139),
    B32ToInt(140), B32ToInt(141), B32ToInt(142), B32ToInt(143), B32ToInt(144),
    B32ToInt(145), B32ToInt(146), B32ToInt(147), B32ToInt(148), B32ToInt(149),
    B32ToInt(150), B32ToInt(151), B32ToInt(152), B32ToInt(153), B32ToInt(154),
    B32ToInt(155), B32ToInt(156), B32ToInt(157), B32ToInt(158), B32ToInt(159),
    B32ToInt(160), B32ToInt(161), B32ToInt(162), B32ToInt(163), B32ToInt(164),
    B32ToInt(165), B32ToInt(166), B32ToInt(167), B32ToInt(168), B32ToInt(169),
    B32ToInt(170), B32ToInt(171), B32ToInt(172), B32ToInt(173), B32ToInt(174),
    B32ToInt(175), B32ToInt(176), B32ToInt(177), B32ToInt(178), B32ToInt(179),
    B32ToInt(180), B32ToInt(181), B32ToInt(182), B32ToInt(183), B32ToInt(184),
    B32ToInt(185), B32ToInt(186), B32ToInt(187), B32ToInt(188), B32ToInt(189),
    B32ToInt(190), B32ToInt(191), B32ToInt(192), B32ToInt(193), B32ToInt(194),
    B32ToInt(195), B32ToInt(196), B32ToInt(197), B32ToInt(198), B32ToInt(199),
    B32ToInt(200), B32ToInt(201), B32ToInt(202), B32ToInt(203), B32ToInt(204),
    B32ToInt(205), B32ToInt(206), B32ToInt(207), B32ToInt(208), B32ToInt(209),
    B32ToInt(210), B32ToInt(211), B32ToInt(212), B32ToInt(213), B32ToInt(214),
    B32ToInt(215), B32ToInt(216), B32ToInt(217), B32ToInt(218), B32ToInt(219),
    B32ToInt(220), B32ToInt(221), B32ToInt(222), B32ToInt(223), B32ToInt(224),
    B32ToInt(225), B32ToInt(226), B32ToInt(227), B32ToInt(228), B32ToInt(229),
    B32ToInt(230), B32ToInt(231), B32ToInt(232), B32ToInt(233), B32ToInt(234),
    B32ToInt(235), B32ToInt(236), B32ToInt(237), B32ToInt(238), B32ToInt(239),
    B32ToInt(240), B32ToInt(241), B32ToInt(242), B32ToInt(243), B32ToInt(244),
    B32ToInt(245), B32ToInt(246), B32ToInt(247), B32ToInt(248), B32ToInt(249),
    B32ToInt(250), B32ToInt(251), B32ToInt(252), B32ToInt(253), B32ToInt(254),
    B32ToInt(255)
};

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
std::string Encode(const std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string
    std::size_t group = 0;                      // Current bit group
    std::size_t group_size = 0;                 // How many bits in group
    std::size_t quantum = 0;                    // 5-bit groups outputted

    // Just return an empty string if the input is empty
    if (input.size() == 0) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(((input.size() / 5) + ((input.size() % 5 > 0) ? 1 : 0)) * 8);

    // Iterate over the input string
    for (const std::uint8_t octet : input)
    {
        // Shift the group 8 bits (no effect if group has no useful data bits)
        group <<= 8;

        // Add this octet to the group
        group |= static_cast<std::uint8_t>(octet);

        // Increment the group size to represents the number of data bits
        group_size += 8;

        do
        {
            // Convert the top most significant 5 bits using the Base32Table,
            // appending the Base32 character to the string
            output += Base32Table[(group >> (group_size - 5)) & 0x1f];

            // Note that 5 bits were outputted
            quantum++;

            // Reduce the group size to be 5 bits less
            group_size -= 5;

        } while (group_size >= 5);

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
        output += Base32Table[group & 0x1f];

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
std::vector<std::uint8_t> Decode(const std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output string
    std::uint_fast32_t group = 0;               // Current bit group
    std::uint_fast32_t group_size = 0;          // How many bits in group

    // Just return an empty string if the input is empty
    if (input.size() == 0) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(input.size() / 1.6);

    // Iterate over the input string
    for (const char c : input)
    {
        // Terminate the loop if we find a padding character
        if (c == Base32PaddingCharacter) break;

        // Determine if we have a valid Base32 character
        std::uint8_t octet = Base32ReverseTable[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase32Character) continue;

        // Shift the group by 5 bits (no effect if group == 0)
        group <<= 5;

        // Add these 5 bits to the group
        group |= (octet & 0x1f);

        // Increment the group size
        group_size += 5;

        // Do we have at least an octet in the group?
        if (group_size >= 8)
        {
            // Append the octet to the output string
            output.push_back((group >> (group_size - 8)) & 0xff);

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
