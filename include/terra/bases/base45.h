/*
 *  base45.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file defines functions to encode data as Base45 strings and
 *      decode those strings back to binary data (see IETF RFC 9285).
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

namespace Terra::Base45
{

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
std::string Encode(const std::string_view input);

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
std::string Encode(const std::string_view input);

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
std::string Encode(const std::span<const std::uint8_t> input);

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
std::vector<std::uint8_t> Decode(const std::string_view input);

} // namespace Terra::Base45
