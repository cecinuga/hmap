# dict --- Dictionary Library in C

`dict` is a lightweight and robust dictionary (hash table) library
written in C. It provides key--value storage with support for multiple
value types, collision resolution via **double hashing**, and explicit
memory ownership semantics.

The library is designed to be: - predictable - memory-safe (Valgrind
clean when used correctly) - suitable for low-level or embedded-style C
projects

------------------------------------------------------------------------

## ✨ Features

-   Hash table with **open addressing + double hashing**
-   Fixed-capacity dictionary (no implicit resizing)
-   Supported value types:
    -   `int`
    -   `double`
    -   `string` (deep-copied)
-   Explicit and consistent **error handling**
-   Clear **ownership rules**
-   No global state (except error handling)
-   Suitable for Valgrind / sanitizers

------------------------------------------------------------------------

## 🧠 Design Overview

### Data Model

Each dictionary entry consists of: - a **string key** (internally
copied) - a `DictValue` union tagged with a runtime type

``` c
typedef enum {
    DICT_TYPE_INT,
    DICT_TYPE_DOUBLE,
    DICT_TYPE_STRING
} DictType;

typedef struct {
    DictType type;
    union {
        int    i;
        double d;
        char  *s;
    };
} DictValue;
```

The dictionary itself uses a fixed-size array of entry pointers:

-   collisions are resolved using **double hashing**
-   no linked lists
-   no tombstones
-   lookup and insertion are `O(1)` average, `O(n)` worst-case

------------------------------------------------------------------------

## 🚀 Getting Started

### Create a dictionary

``` c
Dict *dict = dict_create(128);
if (!dict) {
    fprintf(stderr, "%s\n", dict_error_string(dict_last_error()));
    return 1;
}
```

------------------------------------------------------------------------

### Insert values

``` c
dict_put_int(dict, "age", 42);
dict_put_double(dict, "pi", 3.14159);
dict_put_string(dict, "name", "Mario");
```

All keys and values are **deep-copied** internally.

------------------------------------------------------------------------

### Retrieve values

``` c
DictValue v;
if (dict_get(dict, "age", &v)) {
    printf("%d\n", v.i);
}
```

⚠️ If the value type is `DICT_TYPE_STRING`, the caller **must free**
`v.s`.

------------------------------------------------------------------------

### Update existing values

``` c
dict_upd_int(dict, "age", 43);
dict_upd_string(dict, "name", "Luigi");
```

Type mismatch during update is reported as an error.

------------------------------------------------------------------------

### Remove values

``` c
DictValue v;
if (dict_take(dict, "age", &v)) {
    printf("%d\n", v.i);
}
```

-   The entry is removed from the dictionary
-   The value is deep-copied into `v`
-   Caller owns the copied value

------------------------------------------------------------------------

### Cleanup and destroy

``` c
dict_cleanup(dict);  // removes all entries, dictionary reusable
dict_destroy(dict);  // frees everything
```

------------------------------------------------------------------------

## ⚠️ Memory Ownership Rules

  Operation        Ownership
  ---------------- ----------------------------
  Insert (`put`)   Library owns internal data
  Get (`get`)      Caller owns copied value
  Take (`take`)    Caller owns copied value
  Destroy          Frees all internal memory

Failing to call `dict_destroy()` will result in memory leaks.

------------------------------------------------------------------------

## ❗ Error Handling

The library uses an explicit error system:

``` c
int err = dict_last_error();
fprintf(stderr, "%s\n", dict_error_string(err));
```

Errors are: - cleared at the beginning of each public API call - never
printed internally - never mixed with `errno`

------------------------------------------------------------------------

## 🧪 Assertions & Debugging

The implementation uses `assert()` extensively to enforce invariants
such as: - valid dictionary pointers - bounds-checked hash indices -
internal consistency

Compile with `-DNDEBUG` to disable assertions in production builds.

------------------------------------------------------------------------

## 📦 Build Example

``` bash
gcc -Wall -Wextra -g     dict.c dict_err.c hash.c     -o app
```

Valgrind-clean when used correctly:

``` bash
valgrind --leak-check=full ./app
```

------------------------------------------------------------------------

## 📌 Limitations

-   Fixed capacity (no resizing)
-   Keys must be null-terminated strings
-   Not thread-safe
-   No tombstone handling (removed entries free the slot)

These choices are intentional to keep the implementation simple and
predictable.

## 📌 Todo List
- 🔴 [dict.c] test collision handling
- 🔴 [dict.c] add tombstone handling
- 🔴 [dict.c] implement a proper resizing strategy for the hash table.
- 🔴 [hash.c] add support for custom hash functions provided by the user
- 🟠 [dict.c] create another Dict type where it stores only void* ptr in items.
- 🟢 [dict.c] @example summary not displayed in preview

------------------------------------------------------------------------

## 📄 License

MIT License

------------------------------------------------------------------------

## ✍️ Author

Matteo Marchetti