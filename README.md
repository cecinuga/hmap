# Dict Library – API Contract

## Overview
`dict` is a simple C library implementing a key → value dictionary using hashing.

The library is designed as a **public, fail-safe API**:
- No intentional undefined behavior
- No segmentation faults as a response to invalid input
- All errors are reported explicitly via return values

This document defines the **official API contract** of the library.

---

## Design Philosophy

### Public, Defensive API
The library is intended to be used by external and potentially untrusted code.

As a result:
- Functions validate their inputs
- Invalid inputs result in a clean failure
- The library never returns incorrect data silently

### Dual-mode Development
- In **debug builds**, internal invariants may be checked with `assert`
- In **release builds**, errors are still handled gracefully

The observable behavior of the API does not change between builds.

---

## Memory Ownership Rules

### Dict
- Allocated with `dict_create`
- Freed with `dict_destroy`
- The caller owns the `Dict*` handle

### Keys (`key`)
- Keys passed to the library are **copied internally**
- The caller retains ownership of the original key string
- The caller may free or reuse the key after insertion

### Values (`DictValue`)
- Values are allocated and freed internally by the library
- Values returned to the caller are **copies**
- The caller must not free internal dictionary memory

---

## Error Handling

- No public API function returns `void`
- Success and failure are always explicit
- Standard convention:
  - `1` → success
  - `0` → failure

On failure:
- Output parameters are not modified
- Dictionary state remains unchanged

---

## Collision Policy

- Hash collisions are possible
- If collision handling is not implemented:
  - Insertions fail cleanly
  - The function returns `0`
- The library **never returns a value associated with a different key**

Failing is always preferable to returning incorrect data.

---

## API Reference

### dict_create

**Description**  
Creates a new dictionary with a fixed capacity.

**Parameters**
- `capacity` > 0

**Returns**
- Pointer to a valid `Dict` on success
- `NULL` on allocation failure

**Ownership**
- Caller owns the returned dictionary

---

### dict_put_*

**Description**  
Inserts a key → value pair into the dictionary.

**Parameters**
- `dict` must not be `NULL`
- `key` must not be `NULL` and must be a null-terminated string

**Behavior**
- The key is copied internally
- The value is copied internally
- If a collision occurs, insertion fails

**Returns**
- `1` on successful insertion
- `0` on failure (collision, invalid input, allocation failure)

**Ownership**
- The caller retains ownership of `key`

---

### dict_get

**Description**  
Retrieves the value associated with a key without removing it.

**Parameters**
- `dict`, `key`, `out` must not be `NULL`

**Behavior**
- If the key exists, `out` is written
- If the key does not exist, `out` is not modified

**Returns**
- `1` if the key was found
- `0` if the key was not found or on error

---

### dict_take

**Description**  
Retrieves and removes the value associated with a key.

**Parameters**
- `dict`, `key`, `out` must not be `NULL`

**Behavior**
- If the key exists:
  - The value is copied into `out`
  - The entry is removed from the dictionary
- If the key does not exist:
  - No state change occurs

**Returns**
- `1` if the element was removed
- `0` if the key was not found or on error

---

### dict_cleanup

**Description**  
Removes all entries from the dictionary.

**Behavior**
- Frees all internal entries
- The dictionary remains valid and reusable

---

### dict_destroy

**Description**  
Destroys the dictionary and releases all associated resources.

**Behavior**
- Frees all internal memory
- After this call, the dictionary pointer is invalid

---

## Guarantees

- No function returns incorrect data
- No function intentionally causes undefined behavior
- All ownership rules are explicit and stable

---

## Notes

This contract defines the **semantic behavior** of the API.
Any implementation must adhere to this contract to be considered correct.



Todo:
- 🔴 high impact features or improvements or bug fixes should be prioritized.
- 🟠 medium-high general improvements to code quality, usability, performance and testing.
- 🟡 medium features, improvement or refactoring.
- 🟢 low urgency features, minor tweaks, documentation updates.
- 🔵 funny little things.

- 🔴 [dict.c] review entire file logic, ask to llm what he thinks.
- 🔴 [dict.c, Dict] implement a proper resizing strategy for the hash table when load factor > 0.7.
- 🔴 [dict.c, Dict] change the 'put' logic, it should handle occupancy properly.
- 🟠 [dict.c, Dict] create another Dict type where it stores only void* ptr in items.