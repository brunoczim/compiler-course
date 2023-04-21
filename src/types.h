#ifndef TYPES_H_
#define TYPES_H_ 1

enum datatype {
    DATATYPE_INTE,
    DATATYPE_CARA,
    DATATYPE_REAL
};

enum semantic_type {
    SEMANTIC_INT,
    SEMANTIC_CHAR,
    SEMANTIC_FLOAT,
    SEMANTIC_BOOL
};

enum inference_status {
    INFERENCE_ERROR,
    INFERENCE_UNKNOWN,
    INFERENCE_OK
};

struct datatype_list {
    size_t length;
    enum datatype *types;
};

struct function_datatype {
    enum datatype return_type;
    struct datatype_list parameter_types;
};

enum semantic_type datatype_to_semantic_type(enum datatype datatype);

enum datatype semantic_type_to_datatype(enum semantic_type semantic_type);

void free_datatype_list(struct datatype_list datatype_list);

void free_function_datatype(struct function_datatype function_datatype);

char const *semantic_type_to_str(enum semantic_type semantic_type);

int semantic_type_equiv(enum semantic_type type_a, enum semantic_type type_b);

int semantic_type_unify(
    enum semantic_type type_a,
    enum semantic_type type_b,
    enum semantic_type *type_out
);


#endif
