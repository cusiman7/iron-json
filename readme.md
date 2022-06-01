# Iron JSON

A C++ JSON library.

## Project Goals

1. **Efficient**:
  * Developer Time - Simple. Easy to use. Quick to build.
  * CPU Time - Fast to parse. Fast to write. 
  * Memory Used - No uncessary allocations.

2. **Correct**
  * Full JSON compliance - [RFC8259](https://datatracker.ietf.org/doc/html/rfc8259)
  * UTF-8 Validation

3. **First Class**
 * 

## Include It

```
#include <iron/json>
using fe::json;
```

## Parse JSON

```
json j = json::parse("{"key": true, "key2": [null, "hi", 123]}");
```

## Write JSON

```
json j = {
    {"key", true},
    { nullptr, "hi", 123"}
};
```

## Status

Not nearly there yet. It's still early days.

