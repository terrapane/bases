/*
 *  test_base64.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements test logic to encode and decode strings to
 *      and from Base64.
 *
 *  Portability Issues:
 *      None.
 */

#include <random>
#include <chrono>
#include <string>
#include <cstdint>
#include <vector>
#include <terra/stf/stf.h>
#include <terra/bases/base64.h>

using namespace Terra;

// The following are defined as macros so that errors will reveal
// the line number correctly for any failed test
#define VERIFY_BASE64_ENCODE(input, expected) \
    { \
        auto output = Base64::Encode(input); \
        STF_ASSERT_EQ(expected, output); \
    }

#define VERIFY_BASE64_DECODE(input, expected) \
    { \
        std::string s; \
        auto output = Base64::Decode(input); \
        std::copy(output.begin(), output.end(), std::back_inserter(s)); \
        STF_ASSERT_EQ(s, expected); \
    }

STF_TEST(Base64, EncodeTests)
{
    // Test vectors from RFC 4648
    VERIFY_BASE64_ENCODE("", "");
    VERIFY_BASE64_ENCODE("f", "Zg==");
    VERIFY_BASE64_ENCODE("fo", "Zm8=");
    VERIFY_BASE64_ENCODE("foo", "Zm9v");
    VERIFY_BASE64_ENCODE("foob", "Zm9vYg==");
    VERIFY_BASE64_ENCODE("fooba", "Zm9vYmE=");
    VERIFY_BASE64_ENCODE("foobar", "Zm9vYmFy");

    // Additional test vectors
    VERIFY_BASE64_ENCODE("Hello, World!\n", "SGVsbG8sIFdvcmxkIQo=");
    VERIFY_BASE64_ENCODE(std::string(1, '\0'), "AA==");
    VERIFY_BASE64_ENCODE(std::string(2, '\0'), "AAA=");
    VERIFY_BASE64_ENCODE(std::string(3, '\0'), "AAAA");
}

STF_TEST(Base64, DecodeTests)
{
    // Test vectors from RFC 4648
    VERIFY_BASE64_DECODE("", "");
    VERIFY_BASE64_DECODE("Zg==", "f");
    VERIFY_BASE64_DECODE("Zg", "f");            // Padding missing test
    VERIFY_BASE64_DECODE("Zm8=", "fo");
    VERIFY_BASE64_DECODE("Zm8", "fo");          // Padding missing test
    VERIFY_BASE64_DECODE("Zm9v", "foo");
    VERIFY_BASE64_DECODE("Zm9vYg==", "foob");
    VERIFY_BASE64_DECODE("Zm9vYg", "foob");     // Padding missing test
    VERIFY_BASE64_DECODE("Zm9vYmE=", "fooba");
    VERIFY_BASE64_DECODE("Zm9vYmFy", "foobar");

    // Additional test vectors
    VERIFY_BASE64_DECODE("SGVsbG8sIFdvcmxkIQo=", "Hello, World!\n");
    VERIFY_BASE64_DECODE("SGVsbG8s\nIFdvcmxkIQo=", "Hello, World!\n");
    VERIFY_BASE64_DECODE(R"multiline(
                                SGVsbG8s
                                IFdv
                                cmxkIQo=
                         )multiline",
                        "Hello, World!\n");
    VERIFY_BASE64_DECODE("AA==", std::string(1, '\0'));
    VERIFY_BASE64_DECODE("AAA=", std::string(2, '\0'));
    VERIFY_BASE64_DECODE("AAAA", std::string(3, '\0'));
    std::string binary_string;
    std::uint8_t binary_data[6] = {0x25, 0x59, 0x00, 0xEB, 0x67, 0xE6};
    for (auto c : binary_data) binary_string += c;
    VERIFY_BASE64_DECODE("JVkA62fm", binary_string);

    // Verify a longer multi-line Base64-encoded string decodes properly
    VERIFY_BASE64_DECODE(R"multiline(
SWYgSSBoYWQgYSB3b3JsZCBvZiBteSBvd24sIGV2ZXJ5dGhpbmcgd291bGQgYmUgbm9uc2Vuc2
UuIE5vdGhpbmcgd291bGQgYmUgd2hhdCBpdCBpcywgYmVjYXVzZSBldmVyeXRoaW5nIHdvdWxk
IGJlIHdoYXQgaXQgaXNuJ3QuIEFuZCBjb250cmFyeSB3aXNlLCB3aGF0IGlzLCBpdCB3b3VsZG
4ndCBiZS4gQW5kIHdoYXQgaXQgd291bGRuJ3QgYmUsIGl0IHdvdWxkLiBZb3Ugc2VlPw==
                         )multiline",
                         "If I had a world of my own, everything would be "
                         "nonsense. Nothing would be what it is, because "
                         "everything would be what it isn't. And contrary "
                         "wise, what is, it wouldn't be. And what it "
                         "wouldn't be, it would. You see?");
}

STF_TEST(Base64, RandomTest)
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

    // Create a long random vector of octets
    for(int i = 0; i < 50000; i++) original.push_back(random_octet(generator));

    // Encode the vector
    encoded = Base64::Encode(original);

    // Decode the vector
    decoded = Base64::Decode(encoded);

    STF_ASSERT_EQ(original, decoded);
}

STF_TEST(Base64, VectorTest)
{
    std::vector<std::uint8_t> octets = {0x25, 0x59, 0x00, 0xEB, 0x67, 0xE6};

    VERIFY_BASE64_ENCODE(octets, "JVkA62fm");
}

STF_TEST(Base64, ArrayTest)
{
    std::uint8_t octets[] = {0x25, 0x59, 0x00, 0xEB, 0x67, 0xE6};

    VERIFY_BASE64_ENCODE(octets, "JVkA62fm");
}
