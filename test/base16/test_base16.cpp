/*
 *  test_base16.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements test logic for base16 functions.
 *
 *  Portability Issues:
 *      None.
 */

#include <random>
#include <chrono>
#include <string>
#include <cstdint>
#include <terra/stf/stf.h>
#include <terra/bases/base16.h>

using namespace Terra;

// The following are defined as macros so that errors will reveal
// the line number correctly for any failed test
#define VERIFY_BASE16_ENCODE(input, expected) \
    { \
        auto output = Base16::Encode(input); \
        STF_ASSERT_EQ(expected, output); \
    }

#define VERIFY_BASE16_DECODE(input, expected) \
    { \
        std::string s; \
        auto output = Base16::Decode(input); \
        std::copy(output.begin(), output.end(), std::back_inserter(s)); \
        STF_ASSERT_EQ(s, expected); \
    }

STF_TEST(Base16, EncodeTests)
{
    // Test vectors from RFC 4648
    VERIFY_BASE16_ENCODE("", "");
    VERIFY_BASE16_ENCODE("f", "66");
    VERIFY_BASE16_ENCODE("fo", "666F");
    VERIFY_BASE16_ENCODE("foo", "666F6F");
    VERIFY_BASE16_ENCODE("foob", "666F6F62");
    VERIFY_BASE16_ENCODE("fooba", "666F6F6261");
    VERIFY_BASE16_ENCODE("foobar", "666F6F626172");

    // Additional test vectors
    VERIFY_BASE16_ENCODE("The quick brown fox jumps over the lazy dog",
                         "54686520717569636B2062726F776E20666F78206A756D7073206"
                         "F76657220746865206C617A7920646F67");
    std::string str;
    str += char(255);
    VERIFY_BASE16_ENCODE(str, "FF");
    str += char(128);
    VERIFY_BASE16_ENCODE(str, "FF80");
}

STF_TEST(Base16, DecodeTests)
{
    // Test vectors from RFC 4648
    VERIFY_BASE16_DECODE("", "");
    VERIFY_BASE16_DECODE("66", "f");
    VERIFY_BASE16_DECODE("666F", "fo");
    VERIFY_BASE16_DECODE("666F6F", "foo");
    VERIFY_BASE16_DECODE("666F6F62", "foob");
    VERIFY_BASE16_DECODE("666F6F6261", "fooba");
    VERIFY_BASE16_DECODE("666F6F626172", "foobar");

    // Same tests with lowercase input
    VERIFY_BASE16_DECODE("666f", "fo");
    VERIFY_BASE16_DECODE("666f6f", "foo");
    VERIFY_BASE16_DECODE("666f6f62", "foob");
    VERIFY_BASE16_DECODE("666f6f6261", "fooba");
    VERIFY_BASE16_DECODE("666f6f626172", "foobar");

    // Same tests with pointless whitespace and invalid characters
    VERIFY_BASE16_DECODE("6 66.f", "fo");
    VERIFY_BASE16_DECODE("66 6 .f6f", "foo");
    VERIFY_BASE16_DECODE("666;f6 f' 62", "foob");
    VERIFY_BASE16_DECODE("666f 6 f6.2'61", "fooba");
    VERIFY_BASE16_DECODE("6. 66f#6f&62;61!72", "foobar");

    // Additional test vectors
    VERIFY_BASE16_DECODE("54686520717569636B2062726F776E20666F78206A756D7073206"
                         "F76657220746865206C617A7920646F67",
                         "The quick brown fox jumps over the lazy dog");
    VERIFY_BASE16_DECODE("54686520717569636b2062726f776e20666f78206a756d7073206"
                         "f76657220746865206c617a7920646f67",
                         "The quick brown fox jumps over the lazy dog");

    std::vector<std::uint8_t> decoded;
    decoded = Base16::Decode("FF");
    STF_ASSERT_EQ(std::size_t(1), decoded.size());
    STF_ASSERT_EQ(std::uint8_t(255), decoded[0]);

    decoded = Base16::Decode("FF80");
    STF_ASSERT_EQ(std::size_t(2), decoded.size());
    STF_ASSERT_EQ(std::uint8_t(128), decoded[1]);

    // Non-integral number of octets should fail
    decoded = Base16::Decode("FF80F");
    STF_ASSERT_TRUE(decoded.empty());
}

STF_TEST(Base16, RandomTest)
{
    std::vector<std::uint8_t> original;         // Original octets to encode
    std::string encoded;                        // Encoded string
    std::vector<std::uint8_t> decoded;          // Decoded octets

    // Initialize the PRNG to some pseudo-random binary data for testing
    std::random_device rd;                      // PRNG seed
    std::random_device::result_type seed_value; // Seed value

    try
    {
        // Get a seed (this may throw an exception without random device)
        seed_value = rd();
    }
    catch (...)
    {
        seed_value = static_cast<std::random_device::result_type>(
            std::chrono::system_clock::now().time_since_epoch().count());
    }
    std::default_random_engine generator(seed_value);
    std::uniform_int_distribution<unsigned> random_octet(0, 255);

    // Create a long random string of octets
    for (int i = 0; i < 50000; i++) original.push_back(random_octet(generator));

    // Encode the string
    encoded = Base16::Encode(original);

    // Decode the Base16-encoded string
    decoded = Base16::Decode(encoded);

    STF_ASSERT_EQ(original, decoded);
}

STF_TEST(Base16, VectorTest)
{
    // "foobar"
    std::vector<std::uint8_t> octets = {0x66, 0x6f, 0x6f, 0x62, 0x61, 0x72};

    VERIFY_BASE16_ENCODE(octets, "666F6F626172");
}

STF_TEST(Base16, ArrayTest)
{
    // "foobar"
    std::uint8_t octets[] = {0x66, 0x6f, 0x6f, 0x62, 0x61, 0x72};

    VERIFY_BASE16_ENCODE(octets, "666F6F626172");
}
