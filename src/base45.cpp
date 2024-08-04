/*
 *  base45.cpp
 *
 *  Copyright (C) 2024
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
#include <climits>
#include <terra/bases/base45.h>

namespace Terra::Base45
{

// Define the table used for converting to Base45
static const char Base45Table[45] =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
    'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%',
    '*', '+', '-', '.', '/', ':'
};

// Define an value to represent an invalid Base45 character
static constexpr std::uint8_t InvalidBase45Character = 255;

// Use the C pre-processor to define a macro that will tell us the integer
// value for any given Base45 character
#define B45ToInt(x) ( \
    (x) == '0' ?  0 : (x) == '1' ?  1 : (x) == '2' ?  2 : (x) == '3' ?  3 : \
    (x) == '4' ?  4 : (x) == '5' ?  5 : (x) == '6' ?  6 : (x) == '7' ?  7 : \
    (x) == '8' ?  8 : (x) == '9' ?  9 : (x) == 'A' ? 10 : (x) == 'B' ? 11 : \
    (x) == 'C' ? 12 : (x) == 'D' ? 13 : (x) == 'E' ? 14 : (x) == 'F' ? 15 : \
    (x) == 'G' ? 16 : (x) == 'H' ? 17 : (x) == 'I' ? 18 : (x) == 'J' ? 19 : \
    (x) == 'K' ? 20 : (x) == 'L' ? 21 : (x) == 'M' ? 22 : (x) == 'N' ? 23 : \
    (x) == 'O' ? 24 : (x) == 'P' ? 25 : (x) == 'Q' ? 26 : (x) == 'R' ? 27 : \
    (x) == 'S' ? 28 : (x) == 'T' ? 29 : (x) == 'U' ? 30 : (x) == 'V' ? 31 : \
    (x) == 'W' ? 32 : (x) == 'X' ? 33 : (x) == 'Y' ? 34 : (x) == 'Z' ? 35 : \
    (x) == ' ' ? 36 : (x) == '$' ? 37 : (x) == '%' ? 38 : (x) == '*' ? 39 : \
    (x) == '+' ? 40 : (x) == '-' ? 41 : (x) == '.' ? 42 : (x) == '/' ? 43 : \
    (x) == ':' ? 44 : InvalidBase45Character)

// Define the table for converting from Base45 characters to integer values
static const std::uint8_t Base45ReverseTable[256] =
{
    B45ToInt(0),   B45ToInt(1),   B45ToInt(2),   B45ToInt(3),   B45ToInt(4),
    B45ToInt(5),   B45ToInt(6),   B45ToInt(7),   B45ToInt(8),   B45ToInt(9),
    B45ToInt(10),  B45ToInt(11),  B45ToInt(12),  B45ToInt(13),  B45ToInt(14),
    B45ToInt(15),  B45ToInt(16),  B45ToInt(17),  B45ToInt(18),  B45ToInt(19),
    B45ToInt(20),  B45ToInt(21),  B45ToInt(22),  B45ToInt(23),  B45ToInt(24),
    B45ToInt(25),  B45ToInt(26),  B45ToInt(27),  B45ToInt(28),  B45ToInt(29),
    B45ToInt(30),  B45ToInt(31),  B45ToInt(32),  B45ToInt(33),  B45ToInt(34),
    B45ToInt(35),  B45ToInt(36),  B45ToInt(37),  B45ToInt(38),  B45ToInt(39),
    B45ToInt(40),  B45ToInt(41),  B45ToInt(42),  B45ToInt(43),  B45ToInt(44),
    B45ToInt(45),  B45ToInt(46),  B45ToInt(47),  B45ToInt(48),  B45ToInt(49),
    B45ToInt(50),  B45ToInt(51),  B45ToInt(52),  B45ToInt(53),  B45ToInt(54),
    B45ToInt(55),  B45ToInt(56),  B45ToInt(57),  B45ToInt(58),  B45ToInt(59),
    B45ToInt(60),  B45ToInt(61),  B45ToInt(62),  B45ToInt(63),  B45ToInt(64),
    B45ToInt(65),  B45ToInt(66),  B45ToInt(67),  B45ToInt(68),  B45ToInt(69),
    B45ToInt(70),  B45ToInt(71),  B45ToInt(72),  B45ToInt(73),  B45ToInt(74),
    B45ToInt(75),  B45ToInt(76),  B45ToInt(77),  B45ToInt(78),  B45ToInt(79),
    B45ToInt(80),  B45ToInt(81),  B45ToInt(82),  B45ToInt(83),  B45ToInt(84),
    B45ToInt(85),  B45ToInt(86),  B45ToInt(87),  B45ToInt(88),  B45ToInt(89),
    B45ToInt(90),  B45ToInt(91),  B45ToInt(92),  B45ToInt(93),  B45ToInt(94),
    B45ToInt(95),  B45ToInt(96),  B45ToInt(97),  B45ToInt(98),  B45ToInt(99),
    B45ToInt(100), B45ToInt(101), B45ToInt(102), B45ToInt(103), B45ToInt(104),
    B45ToInt(105), B45ToInt(106), B45ToInt(107), B45ToInt(108), B45ToInt(109),
    B45ToInt(110), B45ToInt(111), B45ToInt(112), B45ToInt(113), B45ToInt(114),
    B45ToInt(115), B45ToInt(116), B45ToInt(117), B45ToInt(118), B45ToInt(119),
    B45ToInt(120), B45ToInt(121), B45ToInt(122), B45ToInt(123), B45ToInt(124),
    B45ToInt(125), B45ToInt(126), B45ToInt(127), B45ToInt(128), B45ToInt(129),
    B45ToInt(130), B45ToInt(131), B45ToInt(132), B45ToInt(133), B45ToInt(134),
    B45ToInt(135), B45ToInt(136), B45ToInt(137), B45ToInt(138), B45ToInt(139),
    B45ToInt(140), B45ToInt(141), B45ToInt(142), B45ToInt(143), B45ToInt(144),
    B45ToInt(145), B45ToInt(146), B45ToInt(147), B45ToInt(148), B45ToInt(149),
    B45ToInt(150), B45ToInt(151), B45ToInt(152), B45ToInt(153), B45ToInt(154),
    B45ToInt(155), B45ToInt(156), B45ToInt(157), B45ToInt(158), B45ToInt(159),
    B45ToInt(160), B45ToInt(161), B45ToInt(162), B45ToInt(163), B45ToInt(164),
    B45ToInt(165), B45ToInt(166), B45ToInt(167), B45ToInt(168), B45ToInt(169),
    B45ToInt(170), B45ToInt(171), B45ToInt(172), B45ToInt(173), B45ToInt(174),
    B45ToInt(175), B45ToInt(176), B45ToInt(177), B45ToInt(178), B45ToInt(179),
    B45ToInt(180), B45ToInt(181), B45ToInt(182), B45ToInt(183), B45ToInt(184),
    B45ToInt(185), B45ToInt(186), B45ToInt(187), B45ToInt(188), B45ToInt(189),
    B45ToInt(190), B45ToInt(191), B45ToInt(192), B45ToInt(193), B45ToInt(194),
    B45ToInt(195), B45ToInt(196), B45ToInt(197), B45ToInt(198), B45ToInt(199),
    B45ToInt(200), B45ToInt(201), B45ToInt(202), B45ToInt(203), B45ToInt(204),
    B45ToInt(205), B45ToInt(206), B45ToInt(207), B45ToInt(208), B45ToInt(209),
    B45ToInt(210), B45ToInt(211), B45ToInt(212), B45ToInt(213), B45ToInt(214),
    B45ToInt(215), B45ToInt(216), B45ToInt(217), B45ToInt(218), B45ToInt(219),
    B45ToInt(220), B45ToInt(221), B45ToInt(222), B45ToInt(223), B45ToInt(224),
    B45ToInt(225), B45ToInt(226), B45ToInt(227), B45ToInt(228), B45ToInt(229),
    B45ToInt(230), B45ToInt(231), B45ToInt(232), B45ToInt(233), B45ToInt(234),
    B45ToInt(235), B45ToInt(236), B45ToInt(237), B45ToInt(238), B45ToInt(239),
    B45ToInt(240), B45ToInt(241), B45ToInt(242), B45ToInt(243), B45ToInt(244),
    B45ToInt(245), B45ToInt(246), B45ToInt(247), B45ToInt(248), B45ToInt(249),
    B45ToInt(250), B45ToInt(251), B45ToInt(252), B45ToInt(253), B45ToInt(254),
    B45ToInt(255)
};

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
std::string Encode(const std::span<const std::uint8_t> input)
{
    std::string output;                         // Output string
    std::size_t group = 0;                      // Group of 16 bits
    std::size_t group_size = 0;                 // How many octets in group

    // Just return an empty string if the input is empty
    if (input.empty()) return {};

    // Reserve space for output
    if ((input.size() & 1) == 0)
    {
        // Even size = input size + (input size / 2)
        output.reserve(input.size() + (input.size() >> 1));
    }
    else
    {
        // Even size = input size + (input size / 2) + 1
        output.reserve(input.size() + (input.size() >> 1) + 1);
    }

    // Iterate over the input string to form 16-bit groups
    for (const uint8_t octet : input)
    {
        // Shift the group 8 bits (no effect if group == 0)
        group <<= 8;

        // Add this octet to the group
        group |= static_cast<std::uint8_t>(octet);

        // Increment the group size to represents the number of data bits
        group_size++;

        // Check if the group is full
        if (group_size == 2)
        {
            // Convert one group at a time using the Base45Table, appending
            // Base45 characters to the string for each group
            output += Base45Table[(group       ) % 45];
            output += Base45Table[(group /   45) % 45];
            output += Base45Table[(group / 2025) % 45];

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
        output += Base45Table[(group     ) % 45];
        output += Base45Table[(group / 45) % 45];
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
std::vector<std::uint8_t> Decode(const std::string_view input)
{
    std::vector<std::uint8_t> output;           // Output string
    std::uint_fast32_t group = 0;               // Group of 24 bits
    std::uint_fast32_t group_size = 0;          // How many octets in group

    // Just return an empty string if the input is empty
    if (input.empty()) return {};

    // Reserve space for output (= input size / 1.5)
    output.reserve(
        static_cast<std::size_t>(static_cast<double>(input.size()) / 1.5));

    // Iterate over the input string
    for (const char c : input)
    {
        // Determine if we have a valid Base45 character
        std::uint8_t octet = Base45ReverseTable[static_cast<std::uint8_t>(c)];

        // Skip over any invalid character in the input
        if (octet == InvalidBase45Character) continue;

        // Shift the group 8 bits (no effect if group == 0)
        group <<= 8;

        // Add this octet to the group
        group |= octet;

        // Increment the group size to represents the number of octets
        group_size++;

        // Check if the group is full
        if (group_size == 3)
        {
            // Compute the 16-bit value represented by this group
            std::uint_fast16_t octet_pair = ((group >> 16) & 0xff) +
                                            ((group >>  8) & 0xff) * 45 +
                                            ((group      ) & 0xff) * 2025;

            // Append the octets to the output string
            output.push_back((octet_pair >> 8) & 0xff);
            output.push_back((octet_pair     ) & 0xff);

            // Reset group data
            group_size = 0;
            group = 0;
        }
    }

    // Do we have a partial group to consider?
    if (group_size > 0)
    {
        // Anything other than exactly two octets would indicate a
        // string length error
        if (group_size != 2) return {};

        // Compute the octet value to convert
        output.push_back((((group >> 8) & 0xff) +
                          ((group     ) & 0xff) * 45) & 0xff);
    }

    return output;
}

} // namespace Terra::Base45
