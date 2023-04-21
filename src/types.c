#include <stdlib.h>
#include "types.h"
#include "panic.h"

enum semantic_type datatype_to_semantic_type(enum datatype datatype)
{
    switch (datatype) {
        case DATATYPE_INTE:
            return SEMANTIC_INT;
        case DATATYPE_CARA:
            return SEMANTIC_CHAR;
        case DATATYPE_REAL:
            return SEMANTIC_FLOAT;
    }
    panic("unimplemented datatype %i's conversion to semantic type", datatype);
}

enum datatype semantic_type_to_datatype(enum semantic_type semantic_type)
{
    switch (semantic_type) {
        case SEMANTIC_CHAR:
            return DATATYPE_CARA;
        case SEMANTIC_BOOL:
        case SEMANTIC_INT:
            return DATATYPE_INTE;
        case SEMANTIC_FLOAT:
            return DATATYPE_REAL;
    }
    panic(
        "unimplemented semantic_type %i's conversion to datatype",
        semantic_type
    );
}

void free_datatype_list(struct datatype_list datatype_list)
{
    free(datatype_list.types);
}

void free_function_datatype(struct function_datatype function_datatype)
{
    free_datatype_list(function_datatype.parameter_types);
}

char const *semantic_type_to_str(enum semantic_type semantic_type)
{
    switch (semantic_type) {
        case SEMANTIC_INT: return "inte";
        case SEMANTIC_CHAR: return "cara";
        case SEMANTIC_FLOAT: return "real";
        case SEMANTIC_BOOL: return "<bool>";
    }
    panic("unimplemented semantic_type %i's rendering", semantic_type);
}

int semantic_type_equiv(enum semantic_type type_a, enum semantic_type type_b)
{
    return type_a == type_b
        || (type_a == SEMANTIC_INT && type_b == SEMANTIC_CHAR)
        || (type_a == SEMANTIC_CHAR && type_b == SEMANTIC_INT);
}

int semantic_type_unify(
    enum semantic_type type_a,
    enum semantic_type type_b,
    enum semantic_type *type_out
)
{
    if (type_a == type_b) {
        *type_out = type_a;
        return 1;
    }

    if (
        (type_a == SEMANTIC_INT && type_b == SEMANTIC_CHAR)
        || (type_a == SEMANTIC_CHAR && type_b == SEMANTIC_INT)
    ) {
        *type_out = SEMANTIC_INT;
        return 1;
    }

    return 0;
}
