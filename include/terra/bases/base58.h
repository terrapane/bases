/*
 *  base58.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file defines functions to encode and decode data to and from
 *      Base58 (as used by Bitcoin).
 *
 *  Portability Issues:
 *      Requires C++20 or later.
 */

#pragma once

#include <string>
#include <string_view>
#include <span>
#include <cstdint>
#include <vector>

namespace Terra::Base58
{

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
std::string Encode(const std::string_view input);

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given span of octets into Base58.
 *
 *  Parameters:
 *      input [in]
 *          Span of octets to be encoded as Base58.
 *
 *  Returns:
 *      The Base58-encoded text string.
 *
 *  Comments:
 *      None.
 */
std::string Encode(const std::span<const std::uint8_t> input);

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
std::vector<std::uint8_t> Decode(const std::string_view input);

} // namespace Terra::Base58
