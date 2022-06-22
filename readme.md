# Iron JSON

A C++ JSON library with a focus on performance *and* ease of use.

![build & test](https://github.com/cusiman7/iron-json/actions/workflows/ci.yml/badge.svg)

### Performance
  * Parse at 300 MB/s
  * Custom arena allocator
  * Custom number parsing
  * Fast to build

### Easy to Use
  * One header and one cpp file
  * First class `json` type
  * Automatic array/object deduction
  * `<iostream>` support

### Correct
  * Full JSON compliance - [RFC8259](https://datatracker.ietf.org/doc/html/rfc8259)
  * UTF-8 Validation

### First Class
 * Works like standard library containers

## Examples

Working examples available in `examples/`

### Parse JSON

```
#include <iron/json.h>
using fe::json;

json j = json::parse("{"key": true, "key2": [null, "hi", 123]}").value();
if (j["key"].get<bool>().value()) {
   ...
}
```

## Write JSON

```
#include <iron/json.h>
using fe::json;

json j = json::doc();
json image = json::object(doc.arena());
image["Width"] = 800;
image["Height"] = 600;
image["Title"] = "View from 15th Floor";
image["Thumbnail"] = {
  { "URL", "http://www.example.com/image/481989943" },
  { "Height", 125 },
  { "Width", 100 }
};
image["Animated"] = false;
image["IDs"] = { 116, 943, 234, 38793 };
j["Image"] = std::move(image);

std::string s = j.dump();
// or
std::cout << j;
```

### out
```
{
  "Image": {
    "Width":  800,
    "Height": 600,
    "Title":  "View from 15th Floor",
    "Thumbnail": {
      "Url": "http://www.example.com/image/481989943",
      "Height": 125,
      "Width": 100
    },
    "Animated" : false,
    "IDs": [116, 943, 234, 38793]
  }
}
```

## Status

Not nearly there yet. It's still early days.

