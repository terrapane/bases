/*
 *  test_base32.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements test logic for Base32 functions.
 *
 *  Portability Issues:
 *      None.
 */

#include <random>
#include <chrono>
#include <string>
#include <cstdint>
#include <terra/stf/stf.h>
#include <terra/bases/base32.h>

using namespace Terra;

// The following are defined as macros so that errors will reveal
// the line number correctly for any failed test
#define VERIFY_BASE32_ENCODE(input, expected) \
    { \
        auto output = Base32::Encode(input); \
        STF_ASSERT_EQ(expected, output); \
    }

#define VERIFY_BASE32_DECODE(input, expected) \
    { \
        std::string s; \
        auto output = Base32::Decode(input); \
        std::copy(output.begin(), output.end(), std::back_inserter(s)); \
        STF_ASSERT_EQ(s, expected); \
    }

STF_TEST(Base32, EncodeTests)
{
    // Test vectors from RFC 4648
    VERIFY_BASE32_ENCODE("", "");
    VERIFY_BASE32_ENCODE("f", "MY======");
    VERIFY_BASE32_ENCODE("fo", "MZXQ====");
    VERIFY_BASE32_ENCODE("foo", "MZXW6===");
    VERIFY_BASE32_ENCODE("foob", "MZXW6YQ=");
    VERIFY_BASE32_ENCODE("fooba", "MZXW6YTB");
    VERIFY_BASE32_ENCODE("foobar", "MZXW6YTBOI======");

    // Additional test vectors
    VERIFY_BASE32_ENCODE("The quick brown fox jumps over the lazy dog",
                         "KRUGKIDROVUWG2ZAMJZG653OEBTG66BANJ2W24DTEBXXMZLSEB2GQ"
                         "ZJANRQXU6JAMRXWO===");
}

STF_TEST(Base32, DecodeTests)
{
    // Test vectors from RFC 4648
    VERIFY_BASE32_DECODE("", "");
    VERIFY_BASE32_DECODE("MY======", "f");
    VERIFY_BASE32_DECODE("MZXQ====", "fo");
    VERIFY_BASE32_DECODE("MZXW6===", "foo");
    VERIFY_BASE32_DECODE("MZXW6YQ=", "foob");
    VERIFY_BASE32_DECODE("MZXW6YTB", "fooba");
    VERIFY_BASE32_DECODE("MZXW6YTBOI======", "foobar");

    // Same tests with lowercase input
    VERIFY_BASE32_DECODE("my======", "f");
    VERIFY_BASE32_DECODE("mzxq====", "fo");
    VERIFY_BASE32_DECODE("mzxw6===", "foo");
    VERIFY_BASE32_DECODE("mzxw6yq=", "foob");
    VERIFY_BASE32_DECODE("mzxw6ytb", "fooba");
    VERIFY_BASE32_DECODE("mzxw6ytboi======", "foobar");

    // Test vectors from RFC 4648 without the padding octets
    VERIFY_BASE32_DECODE("MY", "f");
    VERIFY_BASE32_DECODE("MZXQ", "fo");
    VERIFY_BASE32_DECODE("MZXW6", "foo");
    VERIFY_BASE32_DECODE("MZXW6YQ", "foob");
    VERIFY_BASE32_DECODE("MZXW6YTBOI", "foobar");

    // Additional test vectors
    VERIFY_BASE32_DECODE("KRUGKIDROVUWG2ZAMJZG653OEBTG66BANJ2W24DTEBXXMZLSEB2GQ"
                         "ZJANRQXU6JAMRXWO===",
                         "The quick brown fox jumps over the lazy dog");
    VERIFY_BASE32_DECODE("krugkidrovuwg2zamjzg653oebtg66banj2w24dtebxxmzlseb2gq"
                         "zjanrqxu6jamrxwo===",
                         "The quick brown fox jumps over the lazy dog");
}

STF_TEST(Base32, RandomTest)
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
    encoded = Base32::Encode(original);

    // Decode the Base32-encoded string
    decoded = Base32::Decode(encoded);

    STF_ASSERT_EQ(original, decoded);
}

STF_TEST(Base32, VectorTest)
{
    // "foobar"
    std::vector<std::uint8_t> octets = {0x66, 0x6f, 0x6f, 0x62, 0x61, 0x72};

    VERIFY_BASE32_ENCODE(octets, "MZXW6YTBOI======");
}

STF_TEST(Base32, ArrayTest)
{
    // "foobar"
    std::uint8_t octets[] = {0x66, 0x6f, 0x6f, 0x62, 0x61, 0x72};

    VERIFY_BASE32_ENCODE(octets, "MZXW6YTBOI======");
}
