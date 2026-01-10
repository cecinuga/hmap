/* ========== ERROR HANDLING ========== */

typedef enum {
    DICT_OK = 0,              // Success
    DICT_ERR_NULL_ARG,        // NULL argument
    DICT_ERR_MIS_TYPE,        // try to update dict value mismatching type
    DICT_ERR_NOMEM,           // Memory allocation failed
    DICT_ERR_COLLISION,       // Hash collision
    DICT_ERR_NOT_FOUND,       // Key not found
    DICT_ERR_INVALID_CAPACITY // Capacity = 0 in dict_create
} DictError;

static _Thread_local DictError g_last_error;

// Ottiene l'ultimo errore (thread-safe)
DictError dict_last_error(void);

// Resetta l'ultimo errore a DICT_OK
void dict_clear_error(void);

// Converte codice errore in stringa leggibile
const char *dict_error_string(DictError err);

// Macro helper per settare errore e ritornare
#define SET_ERROR_AND_RETURN(err, retval) { g_last_error = (err); return (retval);}