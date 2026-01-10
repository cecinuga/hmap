#include "dict_err.h"
#include "dict.h"

/* ========== ERROR HANDLING IMPLEMENTATION ========== */

// Thread-local storage per l'ultimo errore

_Thread_local DictError g_last_error = DICT_OK;

DictError dict_last_error(void) {
    return g_last_error;
}

void dict_clear_error(void) {
    g_last_error = DICT_OK;
}

const char *dict_error_string(DictError err) {
    switch (err) {
        case DICT_OK:
            return "Success";
        case DICT_ERR_NULL_ARG:
            return "NULL argument provided";
        case DICT_ERR_NOMEM:
            return "Memory allocation failed";
        case DICT_ERR_COLLISION:
            return "Hash collision occurred";
        case DICT_ERR_NOT_FOUND:
            return "Key not found";
        case DICT_ERR_INVALID_CAPACITY:
            return "Invalid capacity (must be > 0)";
        default:
            return "Unknown error";
    }
}