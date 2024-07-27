Base-N Encoder/Decoder Library
==============================

This library is used to transform strings between Base-N encoding format,
where N is any one of the implemented formats.  The following formats are
implemented:

* Base16
* Base32
* Base45
* Base58
* Base64

Each of the encoder / decoder routines are in a distinct namespace under the
Terra namespace.  For example, the functions is used to perform Base64 encoding
and decoding are in two functions in the `Base64` namespace:

```cpp
using namespace Terra;
void Base64::Encode(const std::string_view input, std::string &output);
void Base64::Decode(const std::string_view input, std::string &output);
```
