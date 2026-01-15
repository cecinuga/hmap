# dict — typed hash table in C

`dict` is a fixed-capacity, type-tagged dictionary implemented in C. It stores deep-copied `int`, `double`, or `string` values, probes with double hashing, and surfaces every failure through an explicit error channel so callers stay in control.

------------------------------------------------------------------------

## ✨ Highlights

- Open addressing with **double hashing** (`djb2` primary + `FNV-1a` secondary)
- Configurable capacity via `dict_create(capacity)` (the default test harness uses 701 slots)
- Strong ownership rules – keys and values are copied, strings returned by `dict_get` / `dict_take` belong to the caller
- Thread-local `DictError` state exposed through `dict_last_error()` + `dict_error_string()`
- Deterministic test-suite driven development (`main.c` runs `run_tests()`)

------------------------------------------------------------------------

## 🧠 Architecture at a glance

### Core data structures

```c
typedef struct {
    DictType type;   // DICT_TYPE_INT / DOUBLE / STRING
    union { int i; double d; char *s; };
} DictValue;

typedef enum { CELL_FREE, CELL_OCCUPIED, CELL_TOMBSTONE } DictCellState;

typedef struct {
    char *key;          // heap copy of the user key
    DictValue *value;   // heap-allocated payload
    DictCellState state;
} DictEntry;

typedef struct {
    uint32_t size;
    uint32_t capacity;
    DoubleHashFunction hfn;   // defaults to double_hash
    DictEntry *entries;        // contiguous probe table
} Dict;
```

### Probe lifecycle

1. `double_hash(key, i, capacity)` mixes `djb2` and `FNV-1a` to produce the `i`th probe.
2. Insertions walk until a `CELL_FREE` slot is found. Existing matching keys trigger `DICT_ERR_ALR_INSERTED`.
3. Removals (`dict_take`) free key/value buffers and mark the slot as a **tombstone** (`CELL_TOMBSTONE`) so lookups keep probing. Tombstones are not recycled yet, so repeated insert/delete cycles will exhaust the table.

------------------------------------------------------------------------

## 📚 Public API quick reference

| Function | Description |
| --- | --- |
| `Dict *dict_create(uint32_t capacity)` | Allocate a dictionary with a fixed number of slots. |
| `int dict_put_int/double/string(Dict*, char *key, T val)` | Insert a new value (fails if the key already exists or the table is full). |
| `int dict_upd_int/double/string(Dict*, char *key, T val)` | Update an existing entry, enforcing the original type. |
| `int dict_get(Dict*, char *key, DictValue *out)` | Deep-copy the stored value into `out`. Caller must free `out->s` for strings. |
| `int dict_take(Dict*, char *key, DictValue *out)` | Copy the value into `out` and remove the entry (slot becomes a tombstone). |
| `void dict_destroy(Dict*)` | Free every entry, the probe table, and the `Dict` itself. |

Every public function clears the error state on entry and stores the root cause in `g_last_error` before returning `0`/`NULL` on failure.

------------------------------------------------------------------------

## 🧪 Usage example

```c
#include "dict_public.h"

int main(void) {
    Dict *dict = dict_create(128);
    if (!dict) {
        fprintf(stderr, "dict_create failed: %s\n",
                dict_error_string(dict_last_error()));
        return 1;
    }

    dict_put_int(dict, "age", 42);
    dict_put_string(dict, "name", "Mario");

    dict_upd_int(dict, "age", 43);

    DictValue v;
    if (dict_get(dict, "name", &v)) {
        printf("Hi %s!\n", v.s);
        free(v.s); // caller owns the copy
    }

    if (dict_take(dict, "age", &v)) {
        printf("Removed age=%d\n", v.i);
    }

    dict_destroy(dict);
    return 0;
}
```

------------------------------------------------------------------------

## 🛠️ Building & running

Use the provided `Makefile` to compile every source file under `src/` into `build/app` (which currently runs the test harness):

```bash
make app
./build/app
```

Compilation defaults to `-g -O0 -Wall -Wextra`. Define `DICT_TESTING` to expose internal helpers (already enabled in `test.c`).

### Repository layout

```
.
├── Makefile           # build rules (outputs into build/)
├── src/
│   ├── dict.c         # public API + internal helpers
│   ├── dict_err.*     # thread-local error system
│   ├── dict_struct.h  # shared structs & constants
│   ├── hash.*         # djb2, FNV-1a, bad hash helpers
│   ├── utils.*        # misc helpers (string_to_ascii_long)
│   ├── test.*         # deterministic stress tests
│   └── main.c         # runs run_tests()
└── build/             # generated .o files + app
```

------------------------------------------------------------------------

## ✅ Test suite

`run_tests()` exercises the dictionary under several scenarios:

- **collision_test** – ensures that each slot can be probed exactly once when inserting `DICT_CAP` sequential keys.
- **succ/fail_full_dict_test** – validate the reported `DICT_ERR_DICT_FULL` boundary.
- **succ/fail_put_full_test** – double-check insert error handling when the table is saturated.
- **succ/fail_take_all_test** – verify `dict_take` semantics and the tombstone behavior.

All tests print a `[+] Success …` / `[!] Failed …` banner to stdout/stderr.

------------------------------------------------------------------------

## 🚨 Error handling contract

`dict_err.h` defines the following `DictError` codes:

- `DICT_OK` – last call succeeded
- `DICT_ERR_NULL_ARG` – NULL pointer provided to API
- `DICT_ERR_MIS_TYPE` – attempting to update with a different type
- `DICT_ERR_NOMEM` – allocation failure
- `DICT_ERR_ALR_INSERTED` – duplicate key insertion
- `DICT_ERR_NOT_FOUND` – lookup/take miss
- `DICT_ERR_DICT_FULL` – table is saturated (including tombstones)
- `DICT_ERR_INVALID_CAPACITY` – `dict_create(0)`
- `DICT_ERR_GENERIC` – internal invariant violation

Call `dict_last_error()` immediately after a failure and pass the result to `dict_error_string()` for human-readable diagnostics.

------------------------------------------------------------------------

## ⚠️ Limitations & roadmap

- Capacity is fixed for the lifetime of the dictionary (no resizing yet).
- Tombstoned slots are not recycled, so sequences of `dict_take` operations still consume capacity.
- Not thread-safe; callers must add their own synchronization.
- Hash function selection is currently hard-coded to `double_hash`.

Planned tasks (see in-code TODOs): resizing, user-provided hash functions, and an alternative `Dict` storing raw `void *` items.

------------------------------------------------------------------------

## 📌 Todo List
- 🔴 [dict.c] implement a proper resizing strategy for the hash table.
- 🔴 [hash.c] add support for custom hash functions provided by the user
- 🟠 [dict.c] add thread-safety mechanisms (e.g., mutexes)
- 🟢 [dict.c] @example summary not displayed in preview

------------------------------------------------------------------------

## 📄 License

MIT License

------------------------------------------------------------------------

## ✍️ Author

Matteo Marchetti