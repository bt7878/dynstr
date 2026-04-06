# dynstr

A lightweight, efficient dynamic string library for C with Small String Optimization (SSO).

## Features

- **Small String Optimization (SSO):** Strings up to 15 characters are stored directly in the `dynstr_t` structure,
  avoiding heap allocations.
- **Dynamic Resizing:** Automatically manages memory for larger strings.
- **Rich API:** Support for creation, cloning, appending, inserting, removing, and more.
- **CMake Integration:** Easy to include in your project.
- **Unit Tested:** Comprehensive test suite using CMocka.

## Structure

The `dynstr_t` structure is defined as follows:

```c
typedef struct {
    union {
        char *ptr;
        char buf[16]; // 15 chars + null terminator
    };

    size_t len;
    size_t cap;
} dynstr_t;
```

## Building

The project uses CMake. To build the static library:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Running Tests

To run the unit tests:

```bash
ctest --output-on-failure
```

## Usage Example

```c
#include "dynstr.h"
#include <stdio.h>

int main() {
    dynstr_t s;
    
    // Create a dynamic string
    if (!dynstr_create(&s, "Hello")) {
        return 1;
    }

    // Append to the string
    dynstr_append(&s, " World!");

    // Print the string
    printf("%s\n", dynstr_cstr(&s)); // Output: Hello World!
    printf("Length: %zu, Capacity: %zu\n", dynstr_len(&s), dynstr_cap(&s));

    // Insert into the string
    dynstr_insert(&s, 5, ", C"); // Output: Hello, C World!
    printf("%s\n", dynstr_cstr(&s));

    // Substring
    dynstr_t sub;
    dynstr_substr(&sub, &s, 0, 5); // Output: Hello
    printf("Substring: %s\n", dynstr_cstr(&sub));
    
    // Cleanup
    dynstr_destroy(&s);
    dynstr_destroy(&sub);

    return 0;
}
```

## API Reference

### Initialization & Cleanup

- `bool dynstr_create(dynstr_t *out, const char *str)`: Initialize a new `dynstr_t`.
- `void dynstr_destroy(const dynstr_t *dynstr)`: Free resources.
- `bool dynstr_clone(dynstr_t *out, const dynstr_t *dynstr)`: Deep copy of a string.
- `void dynstr_clear(dynstr_t *dynstr)`: Clear content without freeing memory.

### Accessors

- `const char *dynstr_cstr(const dynstr_t *dynstr)`: Get the C-string representation.
- `size_t dynstr_len(const dynstr_t *dynstr)`: Get the current string length.
- `size_t dynstr_cap(const dynstr_t *dynstr)`: Get the current capacity.
- `char *dynstr_at(dynstr_t *dynstr, size_t i)`: Get a pointer to the character at index `i`.

### Modifiers

- `bool dynstr_reserve(dynstr_t *dynstr, size_t cap)`: Reserve capacity.
- `bool dynstr_shrink_to_fit(dynstr_t *dynstr)`: Shrink capacity to match length.
- `bool dynstr_append(dynstr_t *dynstr, const char *str)`: Append a string.
- `bool dynstr_push(dynstr_t *dynstr, char c)`: Push a single character.
- `void dynstr_pop(dynstr_t *dynstr)`: Remove the last character.
- `bool dynstr_insert(dynstr_t *dynstr, size_t pos, const char *str)`: Insert a string at `pos`.
- `bool dynstr_remove(dynstr_t *dynstr, size_t pos, size_t len)`: Remove `len` characters from `pos`.

### Utilities

- `bool dynstr_substr(dynstr_t *out, const dynstr_t *dynstr, size_t pos, size_t len)`: Extract a substring.
