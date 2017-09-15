#include <stdlib.h>

#ifndef VECTOR_T_CLEANUP

#ifndef VECTOR_T_STRUCT
#error VECTOR_T_STRUCT must be defined before including the vector template.
#endif

#ifndef VECTOR_T_ELEMENT_T
#error VECTOR_T_ELEMENT_T must be defined before including the vector template.
#endif

#ifndef VECTOR_T_ALLOC_F
#define VECTOR_T_ALLOC_F malloc
#endif

#ifndef VECTOR_T_REALLOC_F
#define VECTOR_T_REALLOC_F realloc
#endif

#ifndef VECTOR_T_FREE_F
#define VECTOR_T_FREE_F free
#endif

#ifndef VECTOR_T_COPY_ELEMENT_F
#define VECTOR_T_COPY_ELEMENT_F(D, S) (*(D) = *(S))
#endif

#define VECTOR_T struct VECTOR_T_STRUCT

#ifdef VECTOR_T_DESTROY_ELEMENT_F
#define VECTOR_T_DESTROY_LAST_ELEMENTS(VECTOR, START_INDEX)                \
do {                                                                       \
    VECTOR_T *_vector__1 = VECTOR;                                         \
    size_t _vector__2 = START_INDEX;                                       \
    VECTOR_T *self = _vector__1;                                           \
    size_t start_index = _vector__2;                                       \
    size_t i;                                                              \
    for (i = start_index; i < self->size; i++) {                           \
        VECTOR_T_DESTROY_ELEMENT_F(&self->pool[i]);                        \
    }                                                                      \
} while(0)
#else
#define VECTOR_T_DESTROY_LAST_ELEMENTS(VECTOR, START_INDEX) do {} while(0)
#endif

#define VECTOR_T_D_STRUCT                                                  \
struct VECTOR_T_STRUCT {                                                   \
    size_t capacity;                                                       \
    size_t size;                                                           \
    VECTOR_T_ELEMENT_T *pool;                                              \
}


#define VECTOR_T_D_CONSTRUCT(CONSTRUCT)                                    \
void CONSTRUCT(VECTOR_T* self, size_t initial_capacity, int* error)

#define VECTOR_T_D_DESTRUCT(DESTRUCT)                                      \
void DESTRUCT(VECTOR_T* self)

#define VECTOR_T_D_SET_SIZE(SET_SIZE)                                      \
void SET_SIZE(VECTOR_T* self, size_t new_size, int* error)

#define VECTOR_T_D_GET_REF(GET_REF)                                        \
VECTOR_T_ELEMENT_T* GET_REF(VECTOR_T* self, size_t i, int* error)

#define VECTOR_T_D_GET_REF_UNCHECKED(GET_REF_UNCHECKED)                    \
VECTOR_T_ELEMENT_T* GET_REF_UNCHECKED(VECTOR_T* self, size_t i)

#define VECTOR_T_D_SET(SET)                                                \
void SET(VECTOR_T* self, size_t i, VECTOR_T_ELEMENT_T* elem, int* error)

#define VECTOR_T_D_PUSH(PUSH)                                              \
void PUSH(VECTOR_T* self, VECTOR_T_ELEMENT_T* elem, int* error)


#define VECTOR_T_I_SET(SET)                                                \
VECTOR_T_D_SET(SET) {                                                      \
    if (i >= self->size) {                                                 \
        *error = 1;                                                        \
        return;                                                            \
    }                                                                      \
    VECTOR_T_COPY_ELEMENT_F(self->pool+i, elem);                           \
}


#define VECTOR_T_I_GET_REF(GET_REF)                                        \
VECTOR_T_D_GET_REF(GET_REF) {                                              \
    if (i >= self->size) {                                                 \
        *error = 1;                                                        \
        return NULL;                                                       \
    }                                                                      \
    return self->pool+i;                                                   \
}

#define VECTOR_T_I_GET_REF_UNCHECKED(GET_REF_UNCHECKED)                    \
VECTOR_T_D_GET_REF_UNCHECKED(GET_REF_UNCHECKED) {                          \
    return self->pool+i;                                                   \
}
#define VECTOR_T_GROW(VECTOR, ERROR)                                       \
do { int *_vector__1 = ERROR; VECTOR_T *_vector__2 = VECTOR;               \
    int* error = _vector__1;                                               \
    VECTOR_T* self = _vector__2;                                           \
    size_t elem_size = sizeof(VECTOR_T_ELEMENT_T);                         \
    size_t capacity = self->capacity * 2;                                  \
    VECTOR_T_ELEMENT_T *new_pool;                                          \
                                                                           \
    if (capacity <= self->capacity) {                                      \
        *error = 1;                                                        \
        break;                                                             \
    }                                                                      \
                                                                           \
    new_pool = VECTOR_T_REALLOC_F(self->pool, elem_size * capacity);       \
    if (new_pool == NULL) {                                                \
        *error = 2;                                                        \
        break;                                                             \
    }                                                                      \
                                                                           \
    self->capacity = capacity;                                             \
    self->pool = new_pool;                                                 \
} while(0)


#define VECTOR_T_I_PUSH(PUSH)                                              \
VECTOR_T_D_PUSH(PUSH) {                                                    \
    if (self->size >= self->capacity) {                                    \
        VECTOR_T_GROW(self, error);                                        \
    }                                                                      \
    if (*error) {                                                          \
        *error = (*error *100) + 1;                                        \
        return;                                                            \
    }                                                                      \
    VECTOR_T_COPY_ELEMENT_F(self->pool + self->size, elem);                  \
    self->size++;                                                          \
}


#define VECTOR_T_I_CONSTRUCT(CONSTRUCT)                                    \
VECTOR_T_D_CONSTRUCT(CONSTRUCT) {                                          \
    self->pool =                                                           \
        VECTOR_T_ALLOC_F(sizeof(VECTOR_T_ELEMENT_T) * initial_capacity);   \
    if (self->pool == NULL) {                                              \
        *error = 1;                                                        \
        return;                                                            \
    }                                                                      \
    self->capacity = initial_capacity;                                     \
    self->size = 0;                                                        \
}


#define VECTOR_T_I_SET_SIZE(SET_SIZE)                                      \
VECTOR_T_D_SET_SIZE(SET_SIZE) {                                            \
    if (new_size < self->size) {                                           \
        VECTOR_T_DESTROY_LAST_ELEMENTS(self, new_size);                    \
    }                                                                      \
    else if (new_size > self->capacity) {                                  \
        *error = 1;                                                        \
    }                                                                      \
    self->size = new_size;                                                 \
}


#define VECTOR_T_I_DESTRUCT(DESTRUCT)                                      \
VECTOR_T_D_DESTRUCT(DESTRUCT) {                                            \
    VECTOR_T_DESTROY_LAST_ELEMENTS(self, 0);                               \
    VECTOR_T_FREE_F(self->pool);                                           \
}

#else/*VECTOR_T_CLEANUP*/
#undef VECTOR_T
#undef VECTOR_T_ALLOC_F
#undef VECTOR_T_CLEANUP
#undef VECTOR_T_COPY_ELEMENT_F
#undef VECTOR_T_DESTROY_ELEMENT_F
#undef VECTOR_T_DESTROY_LAST_ELEMENTS
#undef VECTOR_T_D_CONSTRUCT
#undef VECTOR_T_D_SET_SIZE
#undef VECTOR_T_D_DESTRUCT
#undef VECTOR_T_D_GET_REF
#undef VECTOR_T_D_PUSH
#undef VECTOR_T_D_SET
#undef VECTOR_T_D_STRUCT
#undef VECTOR_T_ELEMENT_T
#undef VECTOR_T_FREE_F
#undef VECTOR_T_GROW
#undef VECTOR_T_I_CONSTRUCT
#undef VECTOR_T_I_SET_SIZE
#undef VECTOR_T_I_DESTRUCT
#undef VECTOR_T_I_GET_REF
#undef VECTOR_T_I_PUSH
#undef VECTOR_T_I_SET
#undef VECTOR_T_REALLOC_F
#undef VECTOR_T_STRUCT
#endif/*VECTOR_T_CLEANUP*/
