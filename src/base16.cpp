/*
 *  base16.cpp
 *
 *  Copyright (C) 2024
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
#include <terra/bases/base16.h>

namespace Terra::Base16
{

// Define the table used for converting to Base16
static const char Base16Table[16] =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
    'D', 'E', 'F'
};

// Define an value to represent an invalid Base16 character
static constexpr std::uint8_t InvalidBase16Character = 255;

// Use the C pre-processor to define a macro that will tell us the integer
// value for any given Base16 character
#define B16ToInt(x) ( \
    (x) == '0' ?  0 : (x) == '1' ?  1 : (x) == '2' ?  2 : (x) == '3' ?  3 : \
    (x) == '4' ?  4 : (x) == '5' ?  5 : (x) == '6' ?  6 : (x) == '7' ?  7 : \
    (x) == '8' ?  8 : (x) == '9' ?  9 : (x) == 'A' ? 10 : (x) == 'B' ? 11 : \
    (x) == 'C' ? 12 : (x) == 'D' ? 13 : (x) == 'E' ? 14 : (x) == 'F' ? 15 : \
    (x) == 'a' ? 10 : (x) == 'b' ? 11 : (x) == 'c' ? 12 : (x) == 'd' ? 13 : \
    (x) == 'e' ? 14 : (x) == 'f' ? 15 : InvalidBase16Character)

// Define the table for converting from Base16 characters to integer values
static const std::uint8_t Base16ReverseTable[256] =
{
    B16ToInt(0),   B16ToInt(1),   B16ToInt(2),   B16ToInt(3),   B16ToInt(4),
    B16ToInt(5),   B16ToInt(6),   B16ToInt(7),   B16ToInt(8),   B16ToInt(9),
    B16ToInt(10),  B16ToInt(11),  B16ToInt(12),  B16ToInt(13),  B16ToInt(14),
    B16ToInt(15),  B16ToInt(16),  B16ToInt(17),  B16ToInt(18),  B16ToInt(19),
    B16ToInt(20),  B16ToInt(21),  B16ToInt(22),  B16ToInt(23),  B16ToInt(24),
    B16ToInt(25),  B16ToInt(26),  B16ToInt(27),  B16ToInt(28),  B16ToInt(29),
    B16ToInt(30),  B16ToInt(31),  B16ToInt(32),  B16ToInt(33),  B16ToInt(34),
    B16ToInt(35),  B16ToInt(36),  B16ToInt(37),  B16ToInt(38),  B16ToInt(39),
    B16ToInt(40),  B16ToInt(41),  B16ToInt(42),  B16ToInt(43),  B16ToInt(44),
    B16ToInt(45),  B16ToInt(46),  B16ToInt(47),  B16ToInt(48),  B16ToInt(49),
    B16ToInt(50),  B16ToInt(51),  B16ToInt(52),  B16ToInt(53),  B16ToInt(54),
    B16ToInt(55),  B16ToInt(56),  B16ToInt(57),  B16ToInt(58),  B16ToInt(59),
    B16ToInt(60),  B16ToInt(61),  B16ToInt(62),  B16ToInt(63),  B16ToInt(64),
    B16ToInt(65),  B16ToInt(66),  B16ToInt(67),  B16ToInt(68),  B16ToInt(69),
    B16ToInt(70),  B16ToInt(71),  B16ToInt(72),  B16ToInt(73),  B16ToInt(74),
    B16ToInt(75),  B16ToInt(76),  B16ToInt(77),  B16ToInt(78),  B16ToInt(79),
    B16ToInt(80),  B16ToInt(81),  B16ToInt(82),  B16ToInt(83),  B16ToInt(84),
    B16ToInt(85),  B16ToInt(86),  B16ToInt(87),  B16ToInt(88),  B16ToInt(89),
    B16ToInt(90),  B16ToInt(91),  B16ToInt(92),  B16ToInt(93),  B16ToInt(94),
    B16ToInt(95),  B16ToInt(96),  B16ToInt(97),  B16ToInt(98),  B16ToInt(99),
    B16ToInt(100), B16ToInt(101), B16ToInt(102), B16ToInt(103), B16ToInt(104),
    B16ToInt(105), B16ToInt(106), B16ToInt(107), B16ToInt(108), B16ToInt(109),
    B16ToInt(110), B16ToInt(111), B16ToInt(112), B16ToInt(113), B16ToInt(114),
    B16ToInt(115), B16ToInt(116), B16ToInt(117), B16ToInt(118), B16ToInt(119),
    B16ToInt(120), B16ToInt(121), B16ToInt(122), B16ToInt(123), B16ToInt(124),
    B16ToInt(125), B16ToInt(126), B16ToInt(127), B16ToInt(128), B16ToInt(129),
    B16ToInt(130), B16ToInt(131), B16ToInt(132), B16ToInt(133), B16ToInt(134),
    B16ToInt(135), B16ToInt(136), B16ToInt(137), B16ToInt(138), B16ToInt(139),
    B16ToInt(140), B16ToInt(141), B16ToInt(142), B16ToInt(143), B16ToInt(144),
    B16ToInt(145), B16ToInt(146), B16ToInt(147), B16ToInt(148), B16ToInt(149),
    B16ToInt(150), B16ToInt(151), B16ToInt(152), B16ToInt(153), B16ToInt(154),
    B16ToInt(155), B16ToInt(156), B16ToInt(157), B16ToInt(158), B16ToInt(159),
    B16ToInt(160), B16ToInt(161), B16ToInt(162), B16ToInt(163), B16ToInt(164),
    B16ToInt(165), B16ToInt(166), B16ToInt(167), B16ToInt(168), B16ToInt(169),
    B16ToInt(170), B16ToInt(171), B16ToInt(172), B16ToInt(173), B16ToInt(174),
    B16ToInt(175), B16ToInt(176), B16ToInt(177), B16ToInt(178), B16ToInt(179),
    B16ToInt(180), B16ToInt(181), B16ToInt(182), B16ToInt(183), B16ToInt(184),
    B16ToInt(185), B16ToInt(186), B16ToInt(187), B16ToInt(188), B16ToInt(189),
    B16ToInt(190), B16ToInt(191), B16ToInt(192), B16ToInt(193), B16ToInt(194),
    B16ToInt(195), B16ToInt(196), B16ToInt(197), B16ToInt(198), B16ToInt(199),
    B16ToInt(200), B16ToInt(201), B16ToInt(202), B16ToInt(203), B16ToInt(204),
    B16ToInt(205), B16ToInt(206), B16ToInt(207), B16ToInt(208), B16ToInt(209),
    B16ToInt(210), B16ToInt(211), B16ToInt(212), B16ToInt(213), B16ToInt(214),
    B16ToInt(215), B16ToInt(216), B16ToInt(217), B16ToInt(218), B16ToInt(219),
    B16ToInt(220), B16ToInt(221), B16ToInt(222), B16ToInt(223), B16ToInt(224),
    B16ToInt(225), B16ToInt(226), B16ToInt(227), B16ToInt(228), B16ToInt(229),
    B16ToInt(230), B16ToInt(231), B16ToInt(232), B16ToInt(233), B16ToInt(234),
    B16ToInt(235), B16ToInt(236), B16ToInt(237), B16ToInt(238), B16ToInt(239),
    B16ToInt(240), B16ToInt(241), B16ToInt(242), B16ToInt(243), B16ToInt(244),
    B16ToInt(245), B16ToInt(246), B16ToInt(247), B16ToInt(248), B16ToInt(249),
    B16ToInt(250), B16ToInt(251), B16ToInt(252), B16ToInt(253), B16ToInt(254),
    B16ToInt(255)
};

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
std::string Encode(const std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string

    // Just return an empty string if the input is empty
    if (input.size() == 0) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(input.size() * 2);

    // Iterate over the input string
    for (const std::uint8_t octet : input)
    {
        // Write out the two hex characters representing this octet
        output += Base16Table[(octet >> 4) & 0x0f];
        output += Base16Table[(octet     ) & 0x0f];
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
std::vector<std::uint8_t> Decode(const std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output octets
    std::uint_fast32_t group = 0;               // Current bit group
    std::uint_fast32_t group_size = 0;          // How many bits in group

    // Just return an empty string if the input is empty
    if (input.size() == 0) return {};

    // Estimate the size of the output to avoid repetitive buffer resizing
    output.reserve(input.size() / 2);

    // Iterate over the input string
    for (const char c : input)
    {
        // Determine if we have a valid Base16 character
        std::uint8_t octet = Base16ReverseTable[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase16Character) continue;

        // Shift the group by 4 bits (no effect if group == 0)
        group <<= 4;

        // Add these 4 bits to the group
        group |= (octet & 0x0f);

        // Increment the group size
        group_size += 4;

        // Do we have a full octet?
        if (group_size == 8)
        {
            // Append the octet to the output string
            output.push_back(group & 0xff);

            // Reset group data
            group_size = 0;
        }
    }

    // If there is a partial group (i.e., 4 bits remaining), that is an error
    if (group_size > 0) return {};

    return output;
}

} // namespace Terra::Base16
