/*
 *  base32.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file defines functions to encode data as Base32 strings and
 *      decode those strings back to binary data (see IETF RFC 4648).
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

namespace Terra::Base32
{

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
std::string Encode(const std::string_view input);

/*
 *  Encode
 *
 *  Description:
 *      This function will encode the given span of octets into Base32.
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
std::string Encode(const std::span<const std::uint8_t> input);

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
std::vector<std::uint8_t> Decode(const std::string_view input);

} // namespace Terra::Base32
