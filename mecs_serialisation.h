
/*
MECS - v1 - Menno Markus 2023 - public domain
Serialisation addon for mecs entity component system compatible with C89/C++98 and higher.

Usage: 
Include this file after including mecs.h to use it. 
Ensure to also add the following define in *one* C or C++ file to define the implementation.
#define MECS_IMPLEMENTATION
#include "mecs.h"
#include "mecs_serialisation.h"

--------------------------------------------------
Documentation
--------------------------------------------------
Table:
    1.) Functions
    2.) Compile time options

1.) FUNCTIONS

2.) COMPILE TIME OPTIONS

    #define MECS_NO_SHORT_NAMES
        Must be defined globally.
        
        Short names remove the mecs_ prefix on common public api
        types/functions. Allows to disable this behaviour. Default undefined.

    #define MECS_SERIALISATION_VERSION_LATEST
        Must be defined globally.

        Define the current latest version number to use. If undefined will
        default to 0 but throw a warning.

    #define MECS_SERIALISATION_NO_SUPPORT_MECS_TYPES
        Must be defined globally.

        Allows to disable support for serialising types defined by mecs.

    #define MECS_SERIALISATION_NO_SUPPORT_STD_TYPES
        Must be defined globally.

        Allows to disable support for serialising types defined by the standard
        library.

*/
#ifndef MECS_SERIALISATION_H
#define MECS_SERIALISATION_H

#if !defined(MECS_NO_SHORT_NAMES)
#define serialiser_t                            mecs_serialiser_t
#define deserialiser_t                          mecs_deserialiser_t
#define serialise_registry                      mecs_serialise_registry
#define deserialise_registry                    mecs_deserialise_registry
#define serialise_component_store               mecs_serialise_component_store
#define deserialise_component_store             mecs_deserialise_component_store

#define SERIALISATION_IS_TRIVIAL_DECLARE        MECS_SERIALISATION_IS_TRIVIAL_DECLARE
#define serialisation_is_trivial                mecs_serialisation_is_trivial

#define ARCHIVE_DECLARE                         MECS_ARCHIVE_DECLARE
#define ARCHIVE_DEFINE                          MECS_ARCHIVE_DEFINE
#define ARCHIVE                                 MECS_ARCHIVE
#define SERIALISE_DECLARE                       MECS_SERIALISE_DECLARE
#define SERIALISE_DEFINE                        MECS_SERIALISE_DEFINE
#define SERIALISE                               MECS_SERIALISE
#define DESERIALISE_DECLARE                     MECS_DESERIALISE_DECLARE
#define DESERIALISE_DEFINE                      MECS_DESERIALISE_DEFINE
#define DESERIALISE                             MECS_DESERIALISE
#define archive_add                             mecs_archive_add
#define archive_rem                             mecs_archive_rem
#define archive_add_local                       mecs_archive_add_local
#define archive_rem_local                       mecs_archive_rem_local
#define ARCHIVE_CORE_TYPE                       MECS_ARCHIVE_CORE_TYPE

#define COMPONENT_REGISTER_SERIALISATION_HOOKS  MECS_COMPONENT_REGISTER_SERIALISATION_HOOKS
#define COMPONENT_REGISTER_SERIALISE_HOOK       MECS_COMPONENT_REGISTER_SERIALISE_HOOK
#define COMPONENT_REGISTER_DESERIALISE_HOOK     MECS_COMPONENT_REGISTER_DESERIALISE_HOOK
#define serialiser_binary_t                     mecs_serialiser_binary_t
#define deserialiser_binary_t                   mecs_deserialiser_binary_t
#define serialise_registry_binary               mecs_serialise_registry_binary
#define deserialise_registry_binary             mecs_deserialise_registry_binary
#define serialiser_binary_create                mecs_serialiser_binary_create
#define deserialiser_binary_create              mecs_deserialiser_binary_create
#endif

#ifndef MECS_SERIALISATION_VERSION_LATEST
    #pragma message("Warning: MECS_SERIALISATION_VERSION_LATEST has not been defined. Assuming lastest version equals 0.")
    #define MECS_SERIALISATION_VERSION_LATEST 0
#endif

/* --------------------------------------------------
Core serialisation
-------------------------------------------------- */
typedef struct mecs_serialiser_t mecs_serialiser_t;
typedef struct mecs_deserialiser_t mecs_deserialiser_t;

typedef void(*mecs_serialiser_object_begin_func_t)(mecs_serialiser_t* io_serialiser);
typedef void(*mecs_serialiser_object_end_func_t)(mecs_serialiser_t* io_serialiser);
typedef void(*mecs_serialiser_list_begin_func_t)(mecs_serialiser_t* io_serialiser, size_t i_length);
typedef void(*mecs_serialiser_list_end_func_t)(mecs_serialiser_t* io_serialiser);
typedef void(*mecs_serialiser_map_begin_func_t)(mecs_serialiser_t* io_serialiser, size_t i_length);
typedef void(*mecs_serialiser_map_end_func_t)(mecs_serialiser_t* io_serialiser);
typedef void(*mecs_serialiser_write_func_t)(mecs_serialiser_t* io_serialiser, void const* i_data, size_t i_size);

struct mecs_serialiser_t
{
    void* serialiser_data;
    mecs_uint32_t version;
    mecs_bool_t allow_binary;
    mecs_bool_t allow_out_of_order;
    mecs_bool_t is_versioned;

    mecs_serialiser_object_begin_func_t object_begin_func;
    mecs_serialiser_object_end_func_t object_end_func;
    mecs_serialiser_list_begin_func_t list_begin_func;
    mecs_serialiser_list_end_func_t list_end_func;
    mecs_serialiser_map_begin_func_t map_begin_func;
    mecs_serialiser_map_end_func_t map_end_func;
    mecs_serialiser_write_func_t write_func;
};

typedef void(*mecs_deserialiser_object_begin_func_t)(mecs_deserialiser_t* io_deserialiser);
typedef void(*mecs_deserialiser_object_end_func_t)(mecs_deserialiser_t* io_deserialiser);
typedef void(*mecs_deserialiser_list_begin_func_t)(mecs_deserialiser_t* io_deserialiser, size_t* o_length);
typedef void(*mecs_deserialiser_list_end_func_t)(mecs_deserialiser_t* io_deserialiser);
typedef void(*mecs_deserialiser_map_begin_func_t)(mecs_deserialiser_t* io_deserialiser, size_t* o_length);
typedef void(*mecs_deserialiser_map_end_func_t)(mecs_deserialiser_t* io_deserialiser);
typedef void(*mecs_deserialiser_read_func_t)(mecs_deserialiser_t* io_deserialiser, void* o_data, size_t i_size);

struct mecs_deserialiser_t
{
    void* serialiser_data;
    mecs_uint32_t version;
    mecs_bool_t allow_binary;
    mecs_bool_t allow_out_of_order;
    mecs_bool_t is_versioned;

    mecs_deserialiser_object_begin_func_t object_begin_func;
    mecs_deserialiser_object_end_func_t object_end_func;
    mecs_deserialiser_list_begin_func_t list_begin_func;
    mecs_deserialiser_list_end_func_t list_end_func;
    mecs_deserialiser_map_begin_func_t map_begin_func;
    mecs_deserialiser_map_end_func_t map_end_func;
    mecs_deserialiser_read_func_t read_func;
};

typedef union 
{
    mecs_serialiser_t* serialiser;
    mecs_deserialiser_t* deserialiser;
} mecs_serialisation_archive_t;

#if defined(__cplusplus)
    #define MECS_FUNC_NAME_SERIALISE(T_type) mecs__serialise
    #define MECS_FUNC_NAME_DESERIALISE(T_type) mecs__deserialise
    #define MECS_FUNC_NAME_ARCHIVE(T_type) mecs__archive

    template<class T_type>
    struct mecs_serialisation_is_trivial_impl;

    #define mecs_serialisation_is_trivial(T_type)                                                                           \
        mecs_serialisation_is_trivial_impl<T_type>::v                                                                       \

    #define MECS_SERIALISATION_IS_TRIVIAL_DECLARE(T_type, i_is_trivial)                                                     \
        template<>                                                                                                          \
        struct mecs_serialisation_is_trivial_impl<T_type> { enum { v = i_is_trivial }; }                                    \

    /* Argument dependent lookup allows to invoke a function in a different namespace from outside of that namespace 
       based on the function argument passed. This helper allows grabbing a function pointer to the serialise and 
       deserialise methods from outside of their namespace. */
    template<class T_type>
    struct mecs_serialisation_adl_helper 
    {
        static void serialise_func(mecs_serialiser_t* io_archive, T_type* io_data) 
        { 
            MECS_FUNC_NAME_SERIALISE(T_type)(io_archive, io_data); 
        }

        static void deserialise_func(mecs_deserialiser_t* io_archive, T_type* io_data) 
        { 
            MECS_FUNC_NAME_DESERIALISE(T_type)(io_archive, io_data); 
        }
    };

    #define MECS_FUNC_PTR_SERIALISE(T_type)                                                                                 \
        &mecs_serialisation_adl_helper<T_type>::serialise_func                                                              \

    #define MECS_FUNC_PTR_DESERIALISE(T_type)                                                                               \
        &mecs_serialisation_adl_helper<T_type>::deserialise_func                                                            \

#else 
    #define MECS_FUNC_NAME_SERIALISE(T_type) mecs__serialise_##T_type
    #define MECS_FUNC_NAME_DESERIALISE(T_type) mecs__deserialise_##T_type
    #define MECS_FUNC_NAME_ARCHIVE(T_type) mecs__archive_##T_type

    #define mecs_serialisation_is_trivial(T_type)                                                                           \
        mecs__is_trivial_##T_type                                                                                           \

    #define MECS_SERIALISATION_IS_TRIVIAL_DECLARE(T_type, i_is_trivial)                                                     \
        mecs_bool_t mecs__is_trivial_##T_type = i_is_trivial                                                                \

    #define MECS_FUNC_PTR_SERIALISE(T_type)                                                                                 \
        &MECS_FUNC_NAME_SERIALISE(T_type)                                                                                   \

    #define MECS_FUNC_PTR_DESERIALISE(T_type)                                                                               \
        &MECS_FUNC_NAME_DESERIALISE(T_type)                                                                                 \

#endif

/* Archive macros allow to define both serialise and deserialise methods as one function. */
#define MECS_ARCHIVE_DECLARE(T_type, i_is_trivial)                                                                          \
    MECS_SERIALISATION_IS_TRIVIAL_DECLARE(T_type, i_is_trivial);                                                            \
    void MECS_FUNC_NAME_SERIALISE(T_type)(mecs_serialiser_t* io_archive, T_type* io_data);                                  \
    void MECS_FUNC_NAME_DESERIALISE(T_type)(mecs_deserialiser_t* io_archive, T_type* io_data);                              \
    void MECS_FUNC_NAME_ARCHIVE(T_type)(mecs_serialisation_archive_t io_archive, T_type* io_data, mecs_bool_t is_writer)    \

#define MECS_ARCHIVE_DEFINE(T_type)                                                                                         \
    void MECS_FUNC_NAME_SERIALISE(T_type)(mecs_serialiser_t* io_serialiser, T_type* io_data)                                \
    {                                                                                                                       \
        mecs_serialisation_archive_t archive;                                                                               \
        archive.serialiser = io_serialiser;                                                                                 \
        MECS_FUNC_NAME_ARCHIVE(T_type)(archive, io_data, MECS_TRUE);                                                        \
    }                                                                                                                       \
                                                                                                                            \
    void MECS_FUNC_NAME_DESERIALISE(T_type)(mecs_deserialiser_t* io_deserialiser, T_type* io_data)                          \
    {                                                                                                                       \
        mecs_serialisation_archive_t archive;                                                                               \
        archive.deserialiser = io_deserialiser;                                                                             \
        MECS_FUNC_NAME_ARCHIVE(T_type)(archive, io_data, MECS_FALSE);                                                       \
    }                                                                                                                       \
                                                                                                                            \
    void MECS_FUNC_NAME_ARCHIVE(T_type)(mecs_serialisation_archive_t io_archive, T_type* io_data, mecs_bool_t i_is_writer)  \

#define MECS_ARCHIVE(T_type, i_is_trivial)                                                                                  \
    MECS_ARCHIVE_DECLARE(T_type, i_is_trivial);                                                                             \
    MECS_ARCHIVE_DEFINE(T_type)                                                                                             \


/* Serialise/deserialise macros allow to define both serialise and deserialise methods as seprate functions. */
#define MECS_SERIALISE_DECLARE(T_type)                                                                                      \
    void MECS_FUNC_NAME_SERIALISE(T_type)(mecs_serialiser_t* io_serialiser, T_type* io_data)                                \

#define MECS_DESERIALISE_DECLARE(T_type)                                                                                    \
    void MECS_FUNC_NAME_DESERIALISE(T_type)(mecs_deserialiser_t* io_deserialiser, T_type* io_data)                          \

#define MECS_SERIALISE_DEFINE(T_type)                                                                                       \
    void MECS_FUNC_NAME_SERIALISE(T_type)(mecs_serialiser_t* io_serialiser, T_type* io_data)                                \

#define MECS_DESERIALISE_DEFINE(T_type)                                                                                     \
    void MECS_FUNC_NAME_DESERIALISE(T_type)(mecs_deserialiser_t* io_deserialiser, T_type* io_data)                          \

#define MECS_SERIALISE(T_type)                                                                                              \
    MECS_SERIALISE_DECLARE(T_type);                                                                                         \
    MECS_SERIALISE_DEFINE(T_type)                                                                                           \

#define MECS_DESERIALISE(T_type)                                                                                            \
    MECS_DESERIALISE_DECLARE(T_type);                                                                                       \
    MECS_DESERIALISE_DEFINE(T_type)                                                                                         \


/* Helper methods for serialising and deserialising values. */
#define mecs_archive_add(T_fieldType, i_fieldName, i_versionAdded)                                                          \
    if (i_is_writer)                                                                                                        \
    {                                                                                                                       \
        if (io_archive.serialiser->version >= i_versionAdded)                                                               \
        {                                                                                                                   \
            MECS_FUNC_NAME_SERIALISE(T_fieldType)(io_archive.serialiser, &(io_data->i_fieldName));                          \
        }                                                                                                                   \
    }                                                                                                                       \
    else                                                                                                                    \
    {                                                                                                                       \
        if (io_archive.deserialiser->version >= i_versionAdded)                                                             \
        {                                                                                                                   \
            MECS_FUNC_NAME_DESERIALISE(T_fieldType)(io_archive.deserialiser, &(io_data->i_fieldName));                      \
        }                                                                                                                   \
    }                                                                                                                       \

#define mecs_archive_rem(T_fieldType, i_fieldName, i_versionAdded, i_versionRemoved)                                        \
    if (i_is_writer)                                                                                                        \
    {                                                                                                                       \
        if (io_archive.serialiser->version >= i_versionAdded && io_archive.serialiser->version < i_versionRemoved)          \
        {                                                                                                                   \
            T_fieldType temp;                                                                                               \
            MECS_FUNC_NAME_SERIALISE(T_fieldType)(io_archive.serialiser, &temp);                                            \
        }                                                                                                                   \
    }                                                                                                                       \
    else                                                                                                                    \
    {                                                                                                                       \
        if (io_archive.deserialiser->version >= i_versionAdded && io_archive.deserialiser->version < i_versionRemoved)      \
        {                                                                                                                   \
            T_fieldType temp;                                                                                               \
            MECS_FUNC_NAME_DESERIALISE(T_fieldType)(io_archive.deserialiser, &temp);                                        \
        }                                                                                                                   \
    }                                                                                                                       \

#define mecs_archive_add_local(T_type, i_name, i_versionAdded)                                                              \
    if (i_is_writer)                                                                                                        \
    {                                                                                                                       \
        if (io_archive.serialiser->version >= i_versionAdded)                                                               \
        {                                                                                                                   \
            MECS_FUNC_NAME_SERIALISE(T_type)(io_archive.serialiser, &(i_name));                                             \
        }                                                                                                                   \
    }                                                                                                                       \
    else                                                                                                                    \
    {                                                                                                                       \
        if (io_archive.deserialiser->version >= i_versionAdded)                                                             \
        {                                                                                                                   \
            MECS_FUNC_NAME_DESERIALISE(T_type)(io_archive.deserialiser, &(i_name));                                         \
        }                                                                                                                   \
    }                                                                                                                       \

#define mecs_archive_rem_local(T_type, i_name, i_versionAdded, i_versionRemoved)                                            \
    if (i_is_writer)                                                                                                        \
    {                                                                                                                       \
        if (io_archive.serialiser->version >= i_versionAdded && io_archive.serialiser->version < i_versionRemoved)          \
        {                                                                                                                   \
            T_type temp;                                                                                                    \
            MECS_FUNC_NAME_SERIALISE(T_type)(io_archive.serialiser, &(i_name));                                             \
        }                                                                                                                   \
    }                                                                                                                       \
    else                                                                                                                    \
    {                                                                                                                       \
        if (io_archive.deserialiser->version >= i_versionAdded && io_archive.deserialiser->version < i_versionRemoved)      \
        {                                                                                                                   \
            T_type temp;                                                                                                    \
            MECS_FUNC_NAME_DESERIALISE(T_type)(io_archive.deserialiser, &(i_name));                                         \
        }                                                                                                                   \
    }                                                                                                                       \

/* --------------------------------------------------
Core serialisation types
-------------------------------------------------- */
#define MECS_ARCHIVE_CORE_TYPE(T_type)                                                                                      \
    MECS_ARCHIVE(T_type, MECS_TRUE)                                                                                         \
    {                                                                                                                       \
        if (i_is_writer)                                                                                                    \
        {                                                                                                                   \
            io_archive.serialiser->write_func(io_archive.serialiser, io_data, sizeof(*io_data));                            \
        }                                                                                                                   \
        else                                                                                                                \
        {                                                                                                                   \
            io_archive.deserialiser->read_func(io_archive.deserialiser, io_data, sizeof(*io_data));                         \
        }                                                                                                                   \
    }                                                                                                                       \

#if !defined(MECS_SERIALISATION_NO_SUPPORT_MECS_TYPES)
    #if !defined(__cplusplus) || defined(MECS_SERIALISATION_NO_SUPPORT_STD_TYPES) /* If we are compiling C++ and want to support std types skip these to prevent redefinition. */
        MECS_ARCHIVE_CORE_TYPE(mecs_bool_t)
        MECS_ARCHIVE_CORE_TYPE(mecs_uint8_t)
        MECS_ARCHIVE_CORE_TYPE(mecs_uint16_t)
        MECS_ARCHIVE_CORE_TYPE(mecs_uint32_t)
        MECS_ARCHIVE_CORE_TYPE(mecs_uint64_t)
    #endif
#endif

#if !defined(MECS_SERIALISATION_NO_SUPPORT_STD_TYPES)
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
        #include <stdint.h>
        MECS_ARCHIVE_CORE_TYPE(bool)
        MECS_ARCHIVE_CORE_TYPE(int8_t)
        MECS_ARCHIVE_CORE_TYPE(uint8_t)
        MECS_ARCHIVE_CORE_TYPE(int16_t)
        MECS_ARCHIVE_CORE_TYPE(uint16_t)
        MECS_ARCHIVE_CORE_TYPE(int32_t)
        MECS_ARCHIVE_CORE_TYPE(uint32_t)
        MECS_ARCHIVE_CORE_TYPE(int64_t)
        MECS_ARCHIVE_CORE_TYPE(uint64_t)
    #elif defined(__cplusplus) && __cplusplus >= 201103L /* C++11 */
        #include <cstdint>
        MECS_ARCHIVE_CORE_TYPE(bool)
        MECS_ARCHIVE_CORE_TYPE(std::int8_t)
        MECS_ARCHIVE_CORE_TYPE(std::uint8_t)
        MECS_ARCHIVE_CORE_TYPE(std::int16_t)
        MECS_ARCHIVE_CORE_TYPE(std::uint16_t)
        MECS_ARCHIVE_CORE_TYPE(std::int32_t)
        MECS_ARCHIVE_CORE_TYPE(std::uint32_t)
        MECS_ARCHIVE_CORE_TYPE(std::int64_t)
        MECS_ARCHIVE_CORE_TYPE(std::uint64_t)
    #endif
#endif

typedef struct mecs_registry_t mecs_registry_t;
typedef struct mecs_component_store_t mecs_component_store_t;

#define MECS_COMPONENT_REGISTER_SERIALISATION_HOOKS(T_component) mecs_component_register_serialisation_hooks_impl(      \
    mecs_component_get_type_ptr(T_component),                                                                           \
    (mecs_serialise_func_t)(MECS_FUNC_PTR_SERIALISE(T_component)),                                                      \
    (mecs_deserialise_func_t)(MECS_FUNC_PTR_DESERIALISE(T_component)),                                                  \
    mecs_serialisation_is_trivial(T_component))                                                                         \

#define MECS_COMPONENT_REGISTER_SERIALISE_HOOK(T_component) mecs_component_register_serialise_hook_impl(                \
    mecs_component_get_type_ptr(T_component),                                                                           \
    (mecs_serialise_func_t)(MECS_FUNC_PTR_SERIALISE(T_component)),                                                      \
    mecs_serialisation_is_trivial(T_component))                                                                         \

#define MECS_COMPONENT_REGISTER_DESERIALISE_HOOK(T_component) mecs_component_register_deserialise_hook_impl(            \
    mecs_component_get_type_ptr(T_component),                                                                           \
    (mecs_deserialise_func_t)(MECS_FUNC_PTR_DESERIALISE(T_component)),                                                  \
    mecs_serialisation_is_trivial(T_component))                                                                         \

void mecs_component_register_serialisation_hooks_impl(mecs_component_type_t* o_type, mecs_serialise_func_t i_serialise, mecs_deserialise_func_t i_deserialise, mecs_bool_t i_is_trivial);
void mecs_component_register_serialise_hook_impl(mecs_component_type_t* o_type, mecs_serialise_func_t i_serialise, mecs_bool_t i_is_trivial);
void mecs_component_register_deserialise_hook_impl(mecs_component_type_t* o_type, mecs_deserialise_func_t i_deserialise, mecs_bool_t i_is_trivial);

void mecs_serialise_registry(mecs_serialiser_t* io_serialiser, mecs_registry_t const* i_registry);
void mecs_deserialise_registry(mecs_deserialiser_t* io_deserialiser, mecs_registry_t*o_registry);
void mecs_serialise_component_store(mecs_serialiser_t* io_serialiser, mecs_component_store_t* i_component_store);
void mecs_deserialise_component_store(mecs_deserialiser_t* io_deserialiser, mecs_component_store_t* i_component_store);


/* --------------------------------------------------
Unversioned binary serialisation
-------------------------------------------------- */

typedef struct
{
    mecs_serialiser_t base;
    void* data;
    size_t capacity;
    size_t size;
} mecs_serialiser_binary_t;

typedef struct
{
    mecs_deserialiser_t base;
    void* data;
    size_t size;
    size_t position;
} mecs_deserialiser_binary_t;

void mecs_serialise_registry_binary(mecs_registry_t const* i_registry, void** o_data, size_t* o_size);
void mecs_deserialise_registry_binary(mecs_registry_t* o_registry, void* i_data, size_t i_size);

void mecs_serialiser_binary_create(mecs_serialiser_binary_t* o_serialiser);
void mecs_deserialiser_binary_create(mecs_deserialiser_binary_t* o_deserialiser, void* i_data, size_t i_size);

void mecs_serialiser_binary_list_begin_func(mecs_serialiser_t* io_serialiser, size_t i_length);
void mecs_serialiser_binary_map_begin_func(mecs_serialiser_t* io_serialiser, size_t i_length);
void mecs_serialiser_binary_write_func(mecs_serialiser_t* io_serialiser, void const* i_data, size_t i_size);
void mecs_deserialiser_binary_list_begin_func(mecs_deserialiser_t* io_deserialiser, size_t* o_length);
void mecs_deserialiser_binary_map_begin_func(mecs_deserialiser_t* io_deserialiser, size_t* o_length);
void mecs_deserialiser_binary_read_func(mecs_deserialiser_t* io_deserialiser, void* o_data, size_t i_size);

#endif /* MECS_SERIALISATION_H */

#ifdef MECS_IMPLEMENTATION

/* --------------------------------------------------
Core serialisation
-------------------------------------------------- */
void mecs_component_register_serialisation_hooks_impl(mecs_component_type_t* o_type, mecs_serialise_func_t i_serialise, mecs_deserialise_func_t i_deserialise, mecs_bool_t i_is_trivial)
{
    mecs_assert(o_type);
    o_type->serialise_func = i_serialise;
    o_type->deserialise_func = i_deserialise;
    o_type->is_trivial = i_is_trivial;
}

void mecs_component_register_serialise_hook_impl(mecs_component_type_t* o_type, mecs_serialise_func_t i_serialise, mecs_bool_t i_is_trivial)
{
    mecs_assert(o_type);
    o_type->serialise_func = i_serialise;
    o_type->is_trivial = i_is_trivial;
}

void mecs_component_register_deserialise_hook_impl(mecs_component_type_t* o_type, mecs_deserialise_func_t i_deserialise, mecs_bool_t i_is_trivial)
{
    mecs_assert(o_type);
    o_type->deserialise_func = i_deserialise;
    o_type->is_trivial = i_is_trivial;
}

#define mecs_object_begin(io_serialiser)            if ((io_serialiser)->object_begin_func) (io_serialiser)->object_begin_func(io_serialiser)
#define mecs_object_end(io_serialiser)              if ((io_serialiser)->object_end_func) (io_serialiser)->object_end_func(io_serialiser)
#define mecs_list_begin(io_serialiser, io_length)   if ((io_serialiser)->list_begin_func) (io_serialiser)->list_begin_func(io_serialiser, io_length)
#define mecs_list_end(io_serialiser)                if ((io_serialiser)->list_end_func) (io_serialiser)->list_end_func(io_serialiser)
#define mecs_map_begin(io_serialiser, io_length)    if ((io_serialiser)->map_begin_func) (io_serialiser)->map_begin_func(io_serialiser, io_length)
#define mecs_map_end(io_serialiser)                 if ((io_serialiser)->map_end_func) (io_serialiser)->map_end_func(io_serialiser)
#define mecs_write(io_serialiser, i_data, i_size)   if ((io_serialiser)->write_func) (io_serialiser)->write_func((io_serialiser), (i_data), (i_size))
#define mecs_read(io_serialiser, o_data, i_size)    if ((io_serialiser)->read_func) (io_serialiser)->read_func((io_serialiser), (o_data), (i_size))


void mecs_serialise_registry(mecs_serialiser_t* io_serialiser, mecs_registry_t const* i_registry)
{
    mecs_component_size_t i;
    mecs_component_size_t component_id;
    mecs_component_store_t* component_store;
    mecs_assert(io_serialiser != NULL);
    mecs_assert(i_registry != NULL);

    /* Serialise free and allocated entities. */
    mecs_list_begin(io_serialiser, i_registry->entities_len);
    {
        if (io_serialiser->allow_binary)
        {
            mecs_write(io_serialiser, i_registry->entities, i_registry->entities_len * sizeof(mecs_entity_t));
        }
        else
        {
            for (i = 0; i < i_registry->entities_len; ++i)
            {
                mecs_write(io_serialiser, &i_registry->entities[i], sizeof(mecs_entity_t));
            }
        }
    }
    mecs_list_end(io_serialiser);
    mecs_write(io_serialiser, &i_registry->next_free_entity, sizeof(mecs_entity_t));

    /* Serialise component stores. */
    mecs_map_begin(io_serialiser, i_registry->valid_components_count);
    for (i = 0; i < i_registry->components_len; ++i)
    {
        if (i_registry->components[i].type == NULL)
        {
            /* Not a valid component, nothing to serialise. */
            continue;
        }

        component_id = i;
        mecs_write(io_serialiser, &component_id, sizeof(component_id));

        component_store = &i_registry->components[component_id];
        mecs_serialise_component_store(io_serialiser, component_store);
    }
    mecs_map_end(io_serialiser);
}

void mecs_deserialise_registry(mecs_deserialiser_t* io_deserialiser, mecs_registry_t* o_registry)
{
    size_t entities_len;
    size_t valid_components_count;
    mecs_entity_t* entity;
    mecs_component_size_t i;
    mecs_component_size_t component_id;
    mecs_component_store_t* component_store;
    mecs_assert(io_deserialiser != NULL);
    mecs_assert(o_registry != NULL);

    entities_len = 0;
    valid_components_count = 0;
    component_id = 0;

    /* Deserialise free and allocated entities. */
    mecs_list_begin(io_deserialiser, &entities_len);
    {
        /* Ensure enough memory to deserialise entities. */
        mecs_assert(entities_len < MECS_ENTITY_ID_INVALID);
        entity = mecs_entity_create_array(o_registry, (mecs_entity_size_t)entities_len);
        mecs_assert(entity != NULL);

        if (io_deserialiser->allow_binary)
        {
            mecs_read(io_deserialiser, o_registry->entities, entities_len * sizeof(mecs_entity_t));
        }
        else
        {
            for (i = 0; i < entities_len; ++i)
            {
                mecs_read(io_deserialiser, &o_registry->entities[i], sizeof(mecs_entity_t));
            }
        }
    }
    mecs_list_end(io_deserialiser);
    mecs_read(io_deserialiser, &o_registry->next_free_entity, sizeof(mecs_entity_t));

    /* Deserialise component stores. */
    mecs_map_begin(io_deserialiser, &valid_components_count);
    for (i = 0; i < valid_components_count; ++i)
    {
        mecs_read(io_deserialiser, &component_id, sizeof(component_id));
        /* TODO: Clear component stores not in deserialised data. */

        if (io_deserialiser->allow_out_of_order)
        {
            /* TODO */
        }
        else
        {
            mecs_assert(component_id < o_registry->components_len);
            component_store = &o_registry->components[component_id];
            mecs_deserialise_component_store(io_deserialiser, component_store);
        }
    }
    mecs_map_end(io_deserialiser);
}

void mecs_serialise_component_store(mecs_serialiser_t* io_serialiser, mecs_component_store_t* i_component_store)
{
    mecs_entity_size_t i;
    mecs_entity_size_t page_index;
    mecs_entity_size_t page_offset;
    mecs_entity_size_t page_len;
    void* page;
    void* component;
    mecs_assert(io_serialiser != NULL);
    mecs_assert(i_component_store != NULL);
    mecs_assert(i_component_store->type->serialise_func != NULL);

    mecs_object_begin(io_serialiser);
    {
        mecs_list_begin(io_serialiser, i_component_store->entities_count);
        {
            /* Serialise all entities. Each index maps to an entry in the components list but its faster to not interleave the data. */
            if (io_serialiser->allow_binary)
            {
                mecs_write(io_serialiser, i_component_store->dense, i_component_store->entities_count * sizeof(mecs_dense_t));
            }
            else 
            {
                for (i = 0; i < i_component_store->entities_count; ++i)
                {
                    mecs_write(io_serialiser, &i_component_store->dense[i], sizeof(mecs_dense_t));
                }
            }
        }
        mecs_list_end(io_serialiser);
        mecs_list_begin(io_serialiser, i_component_store->entities_count);
        {
            /* Serialise all components. */
            if (io_serialiser->allow_binary && i_component_store->type->is_trivial)
            {
                /* Serialise each component page as a single binary blob. */
                for (page_index = 0; page_index < i_component_store->components_len - 1; ++page_index)
                {
                    page = i_component_store->components[page_index];
                    page_len = MECS_PAGE_LEN_DENSE;
                    mecs_write(io_serialiser, page, i_component_store->type->size * page_len);
                }
                if (i_component_store->components_len > 0)
                {
                    page = i_component_store->components[i_component_store->components_len - 1];
                    page_len = i_component_store->entities_count % MECS_PAGE_LEN_DENSE;
                    mecs_write(io_serialiser, page, i_component_store->type->size * page_len);
                }
            }
            else 
            {
                /* Serialise each component individually. */
                for (i = 0; i < i_component_store->entities_count; ++i)
                {
                    page_index = i / MECS_PAGE_LEN_DENSE;
                    page_offset = i % MECS_PAGE_LEN_DENSE;
                    page = i_component_store->components[page_index];
                    component = (void*)(((char*)page) + (page_offset * i_component_store->type->size));
                    i_component_store->type->serialise_func(io_serialiser, component);
                }
            }
        }
        mecs_list_end(io_serialiser);
    }
    mecs_object_end(io_serialiser);
}

void mecs_deserialise_component_store(mecs_deserialiser_t* io_deserialiser, mecs_component_store_t* o_component_store)
{
    size_t entities_count;
    mecs_entity_size_t i;
    mecs_dense_t entity;
    mecs_sparse_t* sparse_elem;

    size_t components_count;
    mecs_entity_size_t page_count;
    mecs_entity_size_t page_index;
    mecs_entity_size_t page_offset;
    mecs_entity_size_t page_len;
    void* page;
    void* component;
    mecs_assert(io_deserialiser != NULL);
    mecs_assert(o_component_store != NULL);
    mecs_assert(o_component_store->type->deserialise_func != NULL);

    entities_count = 0;
    entity = MECS_ENTITY_INVALID;
    components_count = 0;
    mecs_object_begin(io_deserialiser);
    {
        mecs_list_begin(io_deserialiser, &entities_count);
        {
            /* Ensure enough memory to deserialise entities and components. */
            mecs_assert(entities_count < MECS_ENTITY_ID_INVALID);
            mecs_component_add_dense_elements(o_component_store, (mecs_entity_size_t)entities_count);
            mecs_assert(o_component_store->entities_count >= entities_count);

            /* Deserialise all entities. */
            if (io_deserialiser->allow_binary)
            {
                mecs_read(io_deserialiser, o_component_store->dense, o_component_store->entities_count * sizeof(mecs_dense_t));

                /* Need to reconstruct the sparse set as only the dense arrays are serialised. */
                for (i = 0; i < entities_count; ++i)
                {
                    entity = o_component_store->dense[i];
                    sparse_elem = mecs_component_add_sparse_element(o_component_store, entity);
                    *sparse_elem = mecs_entity_compose(mecs_entity_get_generation(entity), i); /* Build sparse element out of version and dense index. */
                }
            }
            else 
            {
                for (i = 0; i < entities_count; ++i)
                {
                    mecs_read(io_deserialiser, &entity, sizeof(mecs_dense_t));
                    o_component_store->dense[i] = entity;
                    sparse_elem = mecs_component_add_sparse_element(o_component_store, entity);
                    *sparse_elem = mecs_entity_compose(mecs_entity_get_generation(entity), i); /* Build sparse element out of version and dense index. */
                }
            }

        }
        mecs_list_end(io_deserialiser);
        mecs_list_begin(io_deserialiser, &components_count);
        {
            mecs_assert(components_count == entities_count);

            /* Deserialise all components. */
            if (io_deserialiser->allow_binary && o_component_store->type->is_trivial)
            {
                /* Deserialise components as single binary blobs into pages. */
                page_count = (mecs_entity_size_t)components_count / MECS_PAGE_LEN_DENSE;
                for (page_index = 0; page_index < page_count; ++page_index)
                {
                    page = o_component_store->components[page_index];
                    page_len = MECS_PAGE_LEN_DENSE;
                    mecs_read(io_deserialiser, page, o_component_store->type->size * page_len);
                }
                if (o_component_store->components_len > 0)
                {
                    page = o_component_store->components[page_count];
                    page_len = components_count % MECS_PAGE_LEN_DENSE;
                    mecs_read(io_deserialiser, page, o_component_store->type->size * page_len);
                }
            }
            else 
            {
                /* Deserialise each component individually. */
                for (i = 0; i < components_count; ++i)
                {
                    page_index = i / MECS_PAGE_LEN_DENSE;
                    page_offset = i % MECS_PAGE_LEN_DENSE;
                    page = o_component_store->components[page_index];
                    component = (void*)(((char*)page) + (page_offset * o_component_store->type->size));
                    o_component_store->type->deserialise_func(io_deserialiser, component);
                }
            }
        }
        mecs_list_end(io_deserialiser);
    }
    mecs_object_end(io_deserialiser);
}

/* --------------------------------------------------
Unversioned binary serialisation
-------------------------------------------------- */

void mecs_serialise_registry_binary(mecs_registry_t const* i_registry, void** o_data, size_t* o_size) 
{
    mecs_serialiser_binary_t serialiser;
    mecs_assert(o_data != NULL);

    mecs_serialiser_binary_create(&serialiser);
    mecs_serialise_registry(&serialiser.base, i_registry);

    /* Shrink to fit. */
    *o_size = serialiser.size;
    *o_data = mecs_realloc(serialiser.data, serialiser.size);
    if (*o_data == NULL)
    {
        *o_size = 0;
        mecs_assert(MECS_FALSE);
        return;
    }
}

void mecs_deserialise_registry_binary(mecs_registry_t* o_registry, void* i_data, size_t i_size)
{
    mecs_deserialiser_binary_t deserialiser;
    mecs_assert(i_data != NULL);

    mecs_deserialiser_binary_create(&deserialiser, i_data, i_size);
    mecs_deserialise_registry(&deserialiser.base, o_registry);
}

void mecs_serialiser_binary_create(mecs_serialiser_binary_t* o_serialiser)
{
    mecs_assert(o_serialiser != NULL);

    o_serialiser->base.serialiser_data = o_serialiser;
    o_serialiser->base.version = MECS_SERIALISATION_VERSION_LATEST;
    o_serialiser->base.allow_binary = MECS_TRUE;
    o_serialiser->base.allow_out_of_order = MECS_FALSE;
    o_serialiser->base.is_versioned = MECS_FALSE;

    o_serialiser->base.object_begin_func = NULL;
    o_serialiser->base.object_end_func = NULL;
    o_serialiser->base.list_begin_func = &mecs_serialiser_binary_list_begin_func;
    o_serialiser->base.list_end_func = NULL;
    o_serialiser->base.map_begin_func = &mecs_serialiser_binary_map_begin_func;
    o_serialiser->base.map_end_func = NULL;
    o_serialiser->base.write_func = &mecs_serialiser_binary_write_func;

    o_serialiser->data = NULL;
    o_serialiser->capacity = 0;
    o_serialiser->size = 0;
}

void mecs_deserialiser_binary_create(mecs_deserialiser_binary_t* o_deserialiser, void* i_data, size_t i_size)
{
    mecs_assert(o_deserialiser != NULL);

    o_deserialiser->base.serialiser_data = o_deserialiser;
    o_deserialiser->base.version = MECS_SERIALISATION_VERSION_LATEST;
    o_deserialiser->base.allow_binary = MECS_TRUE;
    o_deserialiser->base.allow_out_of_order = MECS_FALSE;
    o_deserialiser->base.is_versioned = MECS_FALSE;

    o_deserialiser->base.object_begin_func = NULL;
    o_deserialiser->base.object_end_func = NULL;
    o_deserialiser->base.list_begin_func = &mecs_deserialiser_binary_list_begin_func;
    o_deserialiser->base.list_end_func = NULL;
    o_deserialiser->base.map_begin_func = &mecs_deserialiser_binary_map_begin_func;
    o_deserialiser->base.map_end_func = NULL;
    o_deserialiser->base.read_func = &mecs_deserialiser_binary_read_func;

    o_deserialiser->data = i_data;
    o_deserialiser->size = i_size;
    o_deserialiser->position = 0;
}

void mecs_serialiser_binary_list_begin_func(mecs_serialiser_t* io_serialiser, size_t i_length)
{
    mecs_assert(io_serialiser != NULL);
    mecs_serialiser_binary_write_func(io_serialiser, &i_length, sizeof(i_length));
}

void mecs_serialiser_binary_map_begin_func(mecs_serialiser_t* io_serialiser, size_t i_length)
{
    mecs_assert(io_serialiser != NULL);
    mecs_serialiser_binary_write_func(io_serialiser, &i_length, sizeof(i_length));
}

void mecs_serialiser_binary_write_func(mecs_serialiser_t* io_serialiser, void const* i_data, size_t i_size)
{
    mecs_serialiser_binary_t* serialiser;
    void* data_grown;
    size_t capacity_grown;
    mecs_assert(io_serialiser != NULL);

    serialiser = (mecs_serialiser_binary_t*)io_serialiser->serialiser_data;
    if (serialiser->capacity - serialiser->size < i_size)
    {
        capacity_grown = serialiser->capacity;
        if (capacity_grown == 0)
        {
            capacity_grown = 64;
        }

        /* Double buffer capacity until the data fits.
           Ensure we don't overflow and utalise the full range of available bytes. */
        while (capacity_grown - serialiser->size < i_size)
        {
            if (capacity_grown <= ((size_t)-1) - capacity_grown)
            {
                capacity_grown = capacity_grown * 2;
            }
            else
            {
                capacity_grown = (size_t)-1;
                break;
            }
        }

        /* If we were unable to allocate enough capacity, abort. */
        if (capacity_grown - serialiser->size < i_size)
        {
            mecs_assert(MECS_FALSE);
            return;
        }

        data_grown = mecs_realloc(serialiser->data, capacity_grown);
        if (data_grown == NULL)
        {
            mecs_assert(MECS_FALSE);
            return;
        }
        serialiser->data = data_grown;
        serialiser->capacity = capacity_grown;

    } 

    memcpy((char*)serialiser->data + serialiser->size, i_data, i_size);
    serialiser->size += i_size;
}

void mecs_deserialiser_binary_list_begin_func(mecs_deserialiser_t* io_deserialiser, size_t* o_length)
{
    mecs_assert(io_deserialiser != NULL);
    mecs_deserialiser_binary_read_func(io_deserialiser, o_length, sizeof(*o_length));
}

void mecs_deserialiser_binary_map_begin_func(mecs_deserialiser_t* io_deserialiser, size_t* o_length)
{
    mecs_assert(io_deserialiser != NULL);
    mecs_deserialiser_binary_read_func(io_deserialiser, o_length, sizeof(*o_length));
}

void mecs_deserialiser_binary_read_func(mecs_deserialiser_t* io_deserialiser, void* o_data, size_t i_size)
{
    mecs_deserialiser_binary_t* deserialiser;
    mecs_assert(io_deserialiser != NULL);
    mecs_assert(o_data != NULL);

    deserialiser = (mecs_deserialiser_binary_t*)io_deserialiser->serialiser_data;
    if (deserialiser->size - deserialiser->position < i_size)
    {
        mecs_assert(MECS_FALSE);
        return;
    }

    memcpy(o_data, (char*)deserialiser->data + deserialiser->position, i_size);
    deserialiser->position += i_size;
}

#endif /* MECS_IMPLEMENTATION */