/*
 *  test_base45.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements test logic for Base45 functions.
 *
 *  Portability Issues:
 *      None.
 */

#include <random>
#include <chrono>
#include <string>
#include <cstdint>
#include <terra/stf/stf.h>
#include <terra/bases/base45.h>

using namespace Terra;

// The following are defined as macros so that errors will reveal
// the line number correctly for any failed test
#define VERIFY_BASE45_ENCODE(input, expected) \
    { \
        auto output = Base45::Encode(input); \
        STF_ASSERT_EQ(expected, output); \
    }

#define VERIFY_BASE45_DECODE(input, expected) \
    { \
        std::string s; \
        auto output = Base45::Decode(input); \
        std::copy(output.begin(), output.end(), std::back_inserter(s)); \
        STF_ASSERT_EQ(s, expected); \
    }

STF_TEST(Base45, EncodeTests)
{
    // Test vectors from RFC 9285
    VERIFY_BASE45_ENCODE("AB", "BB8");
    VERIFY_BASE45_ENCODE("Hello!!", "%69 VD92EX0");
    VERIFY_BASE45_ENCODE("base-45", "UJCLQE7W581");
    VERIFY_BASE45_ENCODE("ietf!", "QED8WEX0");

    // Other known values
    VERIFY_BASE45_ENCODE("", "");
    VERIFY_BASE45_ENCODE("Hello", "%69 VDL2");
    VERIFY_BASE45_ENCODE("The quick brown fox jumps over the lazy dog",
                         "8UADZCKFEOEDJOD2KC54EM-DX.CH8FSKDQ$D.OE44E5$CS44+"
                         "8DK44OEC3EFGVCD2");
    VERIFY_BASE45_ENCODE(std::string(1, '\0'), "00");
    VERIFY_BASE45_ENCODE(std::string(2, '\0'), "000");
    VERIFY_BASE45_ENCODE(std::string(3, '\0'), "00000");
}

STF_TEST(Base45, DecodeTests)
{
    // Test vectors from RFC 9285
    VERIFY_BASE45_DECODE("BB8", "AB");
    VERIFY_BASE45_DECODE("%69 VD92EX0", "Hello!!");
    VERIFY_BASE45_DECODE("UJCLQE7W581", "base-45");
    VERIFY_BASE45_DECODE("QED8WEX0", "ietf!");

    // Other known values
    VERIFY_BASE45_DECODE("", "");
    VERIFY_BASE45_DECODE("%69 VDL2", "Hello");
    VERIFY_BASE45_DECODE("8UADZCKFEOEDJOD2KC54EM-DX.CH8FSKDQ$D.OE44E5$CS44+"
                         "8DK44OEC3EFGVCD2",
                         "The quick brown fox jumps over the lazy dog");
    VERIFY_BASE45_DECODE("00", std::string(1, '\0'));
    VERIFY_BASE45_DECODE("000", std::string(2, '\0'));
    VERIFY_BASE45_DECODE("00000", std::string(3, '\0'));
}

STF_TEST(Base45, RandomTest)
{
    std::vector<std::uint8_t> original;         // Original octets
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

    // Encode the vector
    encoded = Base45::Encode(original);

    // Decode the vector
    decoded = Base45::Decode(encoded);

    STF_ASSERT_EQ(original, decoded);
}

STF_TEST(Base45, OneOctetInputs)
{
    for (std::size_t i = 0; i < 256; i++)
    {
        std::string encoded{};
        std::string decoded{};

        // Form string
        std::string original(1, char(i));

        // Ensure the string is 1 character long
        STF_ASSERT_EQ(std::size_t(1), original.length());

        // Encode
        encoded = Base45::Encode(original);

        // Decode
        VERIFY_BASE45_DECODE(encoded, original);
    }
}

STF_TEST(Base45, TwoOctetInputs)
{
    for (std::size_t i = 0; i < 256; i++)
    {
        for (std::size_t j = 0; j < 256; j++)
        {
            std::string encoded{};
            std::string decoded{};

            // Form string
            std::string original;
            original += char(i);
            original += char(j);

            // Ensure the string is 2 characters long
            STF_ASSERT_EQ(std::size_t(2), original.length());

            // Encode the string
            encoded = Base45::Encode(original);

            // Decode the string
            VERIFY_BASE45_DECODE(encoded, original);
        }
    }
}

STF_TEST(Base45, VectorTest)
{
    // "Hello!!"
    std::vector<std::uint8_t> octets =
        {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21, 0x21};

    VERIFY_BASE45_ENCODE(octets, "%69 VD92EX0");
}

STF_TEST(Base45, ArrayTest)
{
    // "Hello!!"
    std::uint8_t octets[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21, 0x21};

    VERIFY_BASE45_ENCODE(octets, "%69 VD92EX0");
}
