/*
 *  test_base58.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements test logic for Base58 encode/decode functions.
 *
 *  Portability Issues:
 *      None.
 */

#include <random>
#include <chrono>
#include <string>
#include <cstdint>
#include <terra/stf/stf.h>
#include <terra/bases/base58.h>

using namespace Terra;

// The following are defined as macros so that errors will reveal
// the line number correctly for any failed test
#define VERIFY_BASE58_ENCODE(input, expected) \
    { \
        auto output = Base58::Encode(input); \
        STF_ASSERT_EQ(std::string(expected), output); \
    }

#define VERIFY_BASE58_DECODE(input, expected) \
    { \
        std::string s; \
        auto output = Base58::Decode(input); \
        std::copy(output.begin(), output.end(), std::back_inserter(s)); \
        STF_ASSERT_EQ(s, expected); \
    }

STF_TEST(Base58, EncodeTests)
{
    VERIFY_BASE58_ENCODE("", "");
    VERIFY_BASE58_ENCODE("A", "28");
    VERIFY_BASE58_ENCODE("AB", "5y3");
    VERIFY_BASE58_ENCODE("H", "2F");
    VERIFY_BASE58_ENCODE("HH", "6W3");
    VERIFY_BASE58_ENCODE("HHH", "RHB5");
    std::string ff_20(20, static_cast<char>(0xff));
    VERIFY_BASE58_ENCODE(ff_20, "4ZrjxJnU1LA5xSyrWMNuXTvSYKwt");
    std::string hex_0000010203;
    hex_0000010203.append(2, 0x00);
    hex_0000010203.append(1, 0x01);
    hex_0000010203.append(1, 0x02);
    hex_0000010203.append(1, 0x03);
    VERIFY_BASE58_ENCODE(hex_0000010203, "11Ldp");

    // Tests from IETF draft-msporny-base58-03
    VERIFY_BASE58_ENCODE("Hello World!", "2NEpo7TZRRrLZSi2U");
    VERIFY_BASE58_ENCODE("The quick brown fox jumps over the lazy dog.",
                         "USm3fpXnKG5EUBx2ndxBDMPVciP5hGey2Jh4NDv6gmeo1LkMeiKrL"
                         "JUUBk6Z");
    std::string hex_0000287fb4cd;
    hex_0000287fb4cd.append(2, 0x00);
    hex_0000287fb4cd.append(1, 0x28);
    hex_0000287fb4cd.append(1, 0x7f);
    hex_0000287fb4cd.append(1, static_cast<char>(0xb4));
    hex_0000287fb4cd.append(1, static_cast<char>(0xcd));
    VERIFY_BASE58_ENCODE(hex_0000287fb4cd, "11233QC4");
}

STF_TEST(Base58, DecodeTests)
{
    VERIFY_BASE58_DECODE("", "");
    VERIFY_BASE58_DECODE("28", "A");
    VERIFY_BASE58_DECODE("5y3", "AB");
    VERIFY_BASE58_DECODE("2F", "H");
    VERIFY_BASE58_DECODE("6W3", "HH");
    VERIFY_BASE58_DECODE("RHB5", "HHH");
    std::string ff_20(20, static_cast<char>(0xff));
    VERIFY_BASE58_DECODE("4ZrjxJnU1LA5xSyrWMNuXTvSYKwt", ff_20);
    std::string hex_0000010203;
    hex_0000010203.append(2, 0x00);
    hex_0000010203.append(1, 0x01);
    hex_0000010203.append(1, 0x02);
    hex_0000010203.append(1, 0x03);
    VERIFY_BASE58_DECODE("11Ldp", hex_0000010203);

    // Tests from IETF draft-msporny-base58-03
    VERIFY_BASE58_DECODE("2NEpo7TZRRrLZSi2U", "Hello World!");
    VERIFY_BASE58_DECODE("USm3fpXnKG5EUBx2ndxBDMPVciP5hGey2Jh4NDv6gmeo1LkMeiKrL"
                         "JUUBk6Z",
                         "The quick brown fox jumps over the lazy dog.");
    std::string hex_0000287fb4cd;
    hex_0000287fb4cd.append(2, 0x00);
    hex_0000287fb4cd.append(1, 0x28);
    hex_0000287fb4cd.append(1, 0x7f);
    hex_0000287fb4cd.append(1, static_cast<char>(0xb4));
    hex_0000287fb4cd.append(1, static_cast<char>(0xcd));
    VERIFY_BASE58_DECODE("11233QC4", hex_0000287fb4cd);
}

// The decoder will ignore whitespace, regardless of where it is, so replicate
// the above tests with various whitespace elements sprinkled throughout
STF_TEST(Base58, DecodeTestsWithWhitespace)
{
    VERIFY_BASE58_DECODE("    \n \t   \n\r  ", "");
    VERIFY_BASE58_DECODE(" 2\n 8  ", "A");
    VERIFY_BASE58_DECODE("5 y3\t ", "AB");
    VERIFY_BASE58_DECODE("  2F  ", "H");
    VERIFY_BASE58_DECODE("6 W\n3  ", "HH");
    VERIFY_BASE58_DECODE("   RHB5   ", "HHH");
    std::string ff_20(20, static_cast<char>(0xff));
    VERIFY_BASE58_DECODE("4Zr  jxJnU1LA 5 x Sy  rWMNuXTvSYKwt", ff_20);
    std::string hex_0000010203;
    hex_0000010203.append(2, 0x00);
    hex_0000010203.append(1, 0x01);
    hex_0000010203.append(1, 0x02);
    hex_0000010203.append(1, 0x03);
    VERIFY_BASE58_DECODE("    1 1Ldp", hex_0000010203);

    // Tests from IETF draft-msporny-base58-03
    VERIFY_BASE58_DECODE(" 2 N E po7TZ  RRrLZSi2U", "Hello World!");
    VERIFY_BASE58_DECODE("   USm3fpXnKG5EUBx2ndxBDMPVciP5hGey2Jh4NDv6gmeo1LkMei"
                         "KrLJUUBk6Z  ",
                         "The quick brown fox jumps over the lazy dog.");
    std::string hex_0000287fb4cd;
    hex_0000287fb4cd.append(2, 0x00);
    hex_0000287fb4cd.append(1, 0x28);
    hex_0000287fb4cd.append(1, 0x7f);
    hex_0000287fb4cd.append(1, static_cast<char>(0xb4));
    hex_0000287fb4cd.append(1, static_cast<char>(0xcd));
    VERIFY_BASE58_DECODE("\n\n11233QC4\n\n", hex_0000287fb4cd);
}

STF_TEST(Base58, VectorTest)
{
    std::vector<std::uint8_t> hex_0000287fb4cd;
    hex_0000287fb4cd.push_back(0x00);
    hex_0000287fb4cd.push_back(0x00);
    hex_0000287fb4cd.push_back(0x28);
    hex_0000287fb4cd.push_back(0x7f);
    hex_0000287fb4cd.push_back(0xb4);
    hex_0000287fb4cd.push_back(0xcd);
    VERIFY_BASE58_ENCODE(hex_0000287fb4cd, "11233QC4");
}

STF_TEST(Base58, ArrayTest)
{
    std::uint8_t octets[] = {0x00, 0x00, 0x28, 0x7f, 0xb4, 0xcd};

    VERIFY_BASE58_ENCODE(octets, "11233QC4");
}
