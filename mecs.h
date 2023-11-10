/*
MECS - v1 - Menno Markus 2023 - public domain
Single header entity component system compatible with C89/C++98 and higher.

Usage: 
Before you include this file add the following define in *one* C or C++ file to dfine the implementation.
#define MECS_IMPLEMENTATION
#include "mecs.h"

--------------------------------------------------
Documentation
--------------------------------------------------
Table:
    1.) Functions
        1.1) The registry
        1.2) Components
        1.3) Entities
        1.4) Queries
    2.) Compile time options
    3.) Standard library compile time options

1.) FUNCTIONS

1.1) THE REGISTRY

    registry_create
    registry_destroy
        registry_t* registry_create(mecs_component_size_t i_component_count_reserve)
        void registry_destroy(registry_t* io_registry)

1.2) COMPONENTS

    COMPONENT_DECLARE
    COMPONENT_REGISTER
        void COMPONENT_DECLARE(T)
        void COMPONENT_REGISTER(registry_t* io_registry, T)

    COMPONENT_REGISTER_LIFE_TIME_HOOKS
        void COMPONENT_REGISTER_LIFE_TIME_HOOKS(registry_t* io_registry, T, mecs_ctor_func_t i_ctor_func_ptr, mecs_dtor_func_t i_dtor_func_ptr, mecs_move_and_dtor_func_t i_move_and_dtor_func_ptr)
        void mecs_ctor_func_t(void* io_data)
        void mecs_dtor_func_t(void* io_data)
        void mecs_move_and_dtor_func_t(void* io_src_to_move, void* io_dst_to_destruct)

    component_add
    component_remove
        T* component_add(registry_t* io_registry, entity_t i_entity, T)
        void component_remove(registry_t* io_registry, entity_t i_entity, T)

    component_has
        bool component_has(i_registry, entity_t i_entity, T)

    component_get
        T* component_get(registry_t* io_registry, entity_t i_entity, T)

1.3) ENTITIES
    entity_get_id
    entity_get_generation
        entity_id_t entity_get_id(entity_t i_entity)
        entity_gen_t entity_get_generation(entity_t i_entity)

    entity_create
    entity_destroy
        entity_t entity_create(registry_t* io_registry)
        void entity_destroy(registry_t* io_registry, entity_t i_entity)

    entity_is_destroyed
        bool entity_is_destroyed(registry_t* io_registry, entity_t i_entity)

1.4) QUERIES
    query_create
        query_it_t query_create()

    query_with
        void query_with(query_it_t* io_query_it, T)

    query_without
        void query_without(query_it_t* io_query_it, T)

    query_optional
        void query_optional(query_it_t* io_query_it, T)

    query_begin
    query_next
        void query_begin(registry_t* io_registry, query_it_t* io_query_it)
        bool query_next(mecs_query_it_t* io_query_it)

    query_entity_get
        entity_t query_entity_get(query_it_t* io_query_it)

    query_component_has
        bool query_component_has(query_it_t* io_query_it, T, size_t i_index)

    query_component_get
        T* query_component_get(query_it_t* io_query_it, T, size_t i_index)

2.) COMPILE TIME OPTIONS

    #define MECS_PAGE_LEN_SPARSE
        Must be defined globally.

        Define how many entities should be helt per page of the sparse array.
        Defaults to 4096 / sizeof mecs_entity_t (= 1024) items.

    #define MECS_PAGE_LEN_DENSE
        Must be defined globally.

        Define how many components should be helt per page of the dense array.
        Defaults to 512 items.

    #define MECS_NO_SHORT_NAMES
        Must be defined globally.
        
        Short names remove the mecs_ prefix on common public api
        types/functions. Allows to disable this behaviour. Default undefined.

    #define MECS_NO_DEFAULT_REGISTER_CPP_LIFETIME
        Must be defined globally.

        Allows to disable the automatic usage of C++ constructors/destructors
        with components. Default undefined.

3.) STANDARD LIBRARY COMPILE TIME OPTIONS

    #define mecs_uint8_t 
    #define mecs_uint16_t 
    #define mecs_uint32_t
        Must be defined globally.

        Default implementation always provided through either "stdint.h",
        "cstdint" or by using unsigned char, unsigned short, unsigned long
        respectively for these types. If you define one, you most define all
        three. Can be used to provide your own implementation for the fixed size
        data types mecs uses on platforms that don't support these through the
        C/C++ standard library or have weird sizes. .

    #define mecs_bool_t 
    #define MECS_TRUE
    #define MECS_FALSE
        Must be defined globally.

        Default implementation always provided through "stdbool.h", C++ build
        in bool type or by using unsigned char with values 1 and 0. If you
        define one, you most define all three. Can be used to provide your own
        implementation for booleans. 

    #define mecs_alignof
        Must be defined globally.

        Defaults to implementation provided either by the C/C++ standard
        library, compiler build ins or portable C/C++ implementation. Used to
        return the alignment of a type and used in combination with
        mecs_realloc_aligned. Can be used to provide your own implementation
        that on compilers/platforms that have build in support. 
        
    #define MECS_CHAR_BIT
        Must be defined by the file containing #define MECS_IMPLEMENTATION.

        Defaults to "limits.h" or "climits". Can be used to provide your own
        implementation for how many bits are in a byte on platforms that don't
        support this through the C/C++ standard library.

    #define mecs_assert(i_condition)
        Must be defined by the file containing #define MECS_IMPLEMENTATION.

        Defaults to "assert.h" or "cassert". Mecs heavily depend on asserts for
        error checking. Can be used to provide your own assert implementation.

    #define mecs_realloc(io_data, i_size) 
    #define mecs_free(io_data)
        Must be defined by the file containing #define MECS_IMPLEMENTATION.

        Defaults to standard library realloc and free. If you define one, you
        most define the other. Can be used to provide your own custom allocator.

    #define mecs_realloc_aligned(io_data, i_size, i_alignment) 
    #define mecs_free_aligned(io_data)
        Must be defined by the file containing #define MECS_IMPLEMENTATION.

        Defaults to implementations build on mecs_realloc and mecs_free. If you
        define one, you most define the other. Used to support user components
        that require a specific alignment, but the libary does not enforce that
        the returned allocation is actually aligned. Can be used to provide your
        own custom allocator.  

*/
#ifndef MECS_H
#define MECS_H

/*
V1, 10-Nov-2023, Initial version released.
*/
#define MECS_VERSION 1

#if !defined(MECS_NO_SHORT_NAMES)
#define component_t                             mecs_component_t                                            
#define COMPONENT_DECLARE                       MECS_COMPONENT_DECLARE                                                  
#define COMPONENT_REGISTER                      MECS_COMPONENT_REGISTER                                                   
#define COMPONENT_REGISTER_LIFE_TIME_HOOKS      MECS_COMPONENT_REGISTER_LIFE_TIME_HOOKS                                                                                
#define component_add                           mecs_component_add                                                              
#define component_remove                        mecs_component_remove                                                                 
#define component_has                           mecs_component_has                                                              
#define component_get                           mecs_component_get                                                              

#define entity_t                                mecs_entity_t                                         
#define entity_id_t                             mecs_entity_id_t                                            
#define entity_gen_t                            mecs_entity_gen_t                                             
#define entity_get_id                           mecs_entity_get_id
#define entity_get_generation                   mecs_entity_get_generation                                                        
#define entity_create                           mecs_entity_create                                        
#define entity_destroy                          mecs_entity_destroy                                          
#define entity_is_destroyed                     mecs_entity_is_destroyed                                                    

#define registry_t                              mecs_registry_t
#define registry_create                         mecs_registry_create                                            
#define registry_destroy                        mecs_registry_destroy                                              

#define query_it_t                              mecs_query_it_t
#define query_with                              mecs_query_with                                  
#define query_without                           mecs_query_without                                        
#define query_optional                          mecs_query_optional                                          
#define query_create                            mecs_query_create
#define query_begin                             mecs_query_begin
#define query_next                              mecs_query_next
#define query_entity_get                        mecs_query_entity_get
#define query_component_has                     mecs_query_component_has                                                    
#define query_component_get                     mecs_query_component_get                                                     
#endif

/* --------------------------------------------------
Definition of compiler and platform agnostic standard library types/functions.
-------------------------------------------------- */

/* Provide custom or default implementation of stdint.h fixed size types. */
#if defined(mecs_uint8_t) || defined(mecs_uint16_t) || defined(mecs_uint32_t)
    #if !defined(mecs_uint8_t) || !defined(mecs_uint16_t) || !defined(mecs_uint32_t)
        #error "You must define all of mecs_uint8_t, mecs_uint16_t and mecs_uint32_t."
    #endif
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
    #include <stdint.h>
    typedef uint8_t         mecs_uint8_t;
    typedef uint16_t        mecs_uint16_t;
    typedef uint32_t        mecs_uint32_t;
#elif defined(_cplusplus) && _cplusplus >= 201103L /* C++11 */
    #include <cstdint>
    typedef std::uint8_t    mecs_uint8_t;
    typedef std::uint16_t   mecs_uint16_t;
    typedef std::uint32_t   mecs_uint32_t;
#else
    typedef unsigned char   mecs_uint8_t;
    typedef unsigned short  mecs_uint16_t;
    typedef unsigned long   mecs_uint32_t;
#endif

/* Provide custom or default implementation for booleans. */
#if defined(mecs_bool_t) || defined(MECS_TRUE) || defined(MECS_FALSE)
    #if !defined(mecs_bool_t) || !defined(MECS_TRUE) || !defined(MECS_FALSE)
        #error "You must define all of mecs_bool_t, MECS_TRUE and MECS_FALSE."
    #endif
#elif defined(_cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) /* C++ or C99 */
    typedef bool            mecs_bool_t;
    #define MECS_TRUE       true
    #define MECS_FALSE      false
#else
    typedef unsigned char   mecs_bool_t;
    #define MECS_TRUE       1
    #define MECS_FALSE      0
#endif

/* Alignment. Provide custom implementation of alignof or default implementation which attempts to grab the most accurate implementation. */
#if !defined(mecs_alignof)
    #if defined(__cplusplus) && _cplusplus >= 201103L /* C++11 */
        #define mecs_alignof(T)     ((size_t)alignof(T))
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L /* C11 */
        #define mecs_alignof(T)     ((size_t)_Alignof(T))
    #elif defined(_MSC_VER)
        #define mecs_alignof(T)     ((size_t)__alignof(T))
    #elif defined(__GNUC__)
        #define mecs_alignof(T)     ((size_t)__alignof__(T))
    #elif defined(__cplusplus)
        /* Portable alignment implementation by Martin Buchholz ( https://wambold.com/Martin/writings/alignof.html ) */
        /* 1) Attempt to find the alignment by rounding to the nearest power of 2. */
        namespace mecs_alignof_nearest_power_of_two {
            template<typename T>
            struct alignof_t { 
                enum { 
                    size = sizeof(T), 
                    value = size ^ (size & (size - 1))
                }; 
            };
        }
        /* 2) Attempt to find the alignment by adding bytes onto the type until the size of the struct doubles. */
        namespace mecs_alignof_size_doubled {
            template<typename T> struct alignof_t;

            template<int size_diff>
            struct helper_t {
                template<typename T> 
                struct val_t { enum { value = size_diff }; };
            };

            template<>
            struct helper_t<0>
            {
                template<typename T> 
                struct val_t { enum { value = alignof_t<T>::value }; };
            };

            template<typename T>
            struct alignof_t
            {
                struct type_plus_one_byte_t { T x; char c; };
                enum { 
                    diff = sizeof(type_plus_one_byte_t) - sizeof(T), 
                    value = helper_t<diff>::template val_t<type_plus_one_byte_t>::value 
                };
            };
        }
        /* Pick the larger of the two reported alignments as neither works in all cases. */
        template<typename T>
        struct mecs_alignof_t {
            enum { 
                one = mecs_alignof_nearest_power_of_two::alignof_t<T>::value,
                two = mecs_alignof_size_doubled::alignof_t<T>::value,
                value = one < two ? one : two
            };
        };

        #define mecs_alignof(T)                static_cast<size_t>(mecs_alignof_t<T>::value)
    #else
        #define mecs_alignof(T)                (((size_t)&( (struct{ mecs_uint8_t b; T t; }* )0 )->t)
    #endif
#endif

/* --------------------------------------------------
Definition of the core library.
-------------------------------------------------- */

/* An entity consists out of a generation in the bottom bits and an id in the top bits. 
   Ids may be reused so the generation can be used to check if an entity has been destroyed. */
typedef mecs_uint32_t mecs_entity_t;
typedef mecs_uint16_t mecs_entity_id_t;
typedef mecs_uint16_t mecs_entity_gen_t;
typedef mecs_entity_id_t mecs_entity_size_t;
#define MECS_ENTITY_ID_BITCOUNT 16
#define MECS_ENTITY_ID_INVALID ((mecs_entity_id_t)-1)
#define MECS_ENTITY_INVALID ((mecs_entity_t)-1)
#define MECS_ENTITY_GENERATION_INVALID ((mecs_entity_gen_t)-1)

/* Each component has a unique index assigned to it by the registry. */
typedef mecs_uint32_t mecs_component_t;
typedef mecs_component_t mecs_component_size_t;
#define MECS_COMPONENT_INVALID ((mecs_component_t)-1)

typedef mecs_entity_t mecs_sparse_t;
typedef mecs_entity_t mecs_dense_t;
#define MECS_SPARSE_INVALID ((mecs_sparse_t)-1) 

#if !defined(MECS_PAGE_LEN_SPARSE)
    #define MECS_PAGE_LEN_SPARSE (4096 / sizeof(mecs_sparse_t)) /* Default chosen to equal as many entities as fit in the common page size (4kb). */
#endif
#if !defined(MECS_PAGE_LEN_DENSE)
    #define MECS_PAGE_LEN_DENSE 512
#endif

/* Hooks for callbacks regarding the lifetime of a component. In C++ we automatically register the constructor and destructor. */
typedef void(*mecs_ctor_func_t)(void* io_data);
typedef void(*mecs_dtor_func_t)(void* io_data);
typedef void(*mecs_move_and_dtor_func_t)(void* io_src_to_move, void* io_dst_to_destruct);

#if !defined(MECS_NO_DEFAULT_REGISTER_CPP_LIFETIME) && defined(__cplusplus)
    template<typename T> void mecs_ctor_cpp_impl(void* io_data);
    template<typename T> void mecs_dtor_cpp_impl(void* io_data);
    template<typename T> void mecs_move_and_dtor_cpp_impl(void* io_src_to_move, void* io_dst_to_destruct);
#endif

typedef struct
{ 
    mecs_sparse_t block[MECS_PAGE_LEN_SPARSE]; 
} mecs_sparse_block_t;

typedef struct 
{
    char const* name;
    size_t size;
    size_t alignment;
    
    mecs_ctor_func_t ctor_func;
    mecs_dtor_func_t dtor_func;
    mecs_move_and_dtor_func_t move_and_dtor_func;

    /* Sparse-set mapping entity id to components. 
       Example:

       entity:      id_2 -----------v
       sparse:      [0xff] [0002] [0000] [0xff] [0001]
                       v-----------/
       dense:       [id_2]   [id_4]   [id_1] 
       components:  [comp_2] [comp_4] [comp_2] 
    */
    mecs_sparse_block_t** sparse;   /* Array of pointers to blocks sized MECS_PAGE_LEN_SPARSE elements containing the generation and dense index for each entity id. */
    mecs_dense_t* dense;            /* Array entities for each components. Size is entities_count and capacity matches components_len * MECS_PAGE_LEN_DENSE. */
    void** components;              /* Array of pointers to blocks sized MECS_PAGE_LEN_DENSE elements containing each component. */
    mecs_entity_size_t sparse_len;
    mecs_entity_size_t entities_count;
    mecs_entity_size_t components_len;
} mecs_component_store_t;

typedef struct 
{
    /* Array of component types. Capacity will always try to equal length to minimize memory, but can be used to reserve memory. 
       This is because registring new components should be an infrequent operation, likely only happening during init. */
    mecs_component_store_t* components;
    mecs_component_size_t components_len;
    size_t components_cap;

    /* Array of both alive entities and destroyed entities. Destroyed entities form an implicit linked list within the array. 
       Destroyed entity ids will be reused first when creating new entities. The generation will be incremented and can therefore be used to check if an entity is alive.
       Example:
       
       [gen_0|id_0] [gen_1|id_4] [gen_0|id_2] [gen_0|id_3] [gen_2|id_5] [gen_4|0xff] [gen_8|id_6] 
       next: id_1 ---^        `-----------------------------^        `---^
    */
    mecs_entity_t next_free_entity;
    mecs_entity_t* entities;
    mecs_entity_size_t entities_len;
    mecs_entity_size_t entities_cap;
} mecs_registry_t;

typedef mecs_uint8_t mecs_query_type_t; 
#define MECS_QUERY_TYPE_WITH     0
#define MECS_QUERY_TYPE_WITHOUT  1
#define MECS_QUERY_TYPE_OPTIONAL 2
#define MECS_QUERY_MAX_LEN 15

typedef struct 
{
    mecs_query_type_t type;
    mecs_component_t component;
} mecs_query_arg_t;

/* Uncached query. Quick to construct and lives on the stack with no references by the registry but potentially slower to iterate. */
typedef struct 
{
    /* Points into the dense array of the component store that is the base of this iterator. 
       The query argument with the smallest number of entities we have to iterate is used as the base. */
    mecs_entity_t* current;   
    mecs_entity_t* end;
    mecs_component_store_t* component_stores; 

    /* Evaluating a query only touches the sparse arrays, but chache the dense index as we likely need it to access the component data. */
    mecs_sparse_t sparse_elements[MECS_QUERY_MAX_LEN]; 

    size_t args_len;
    mecs_query_arg_t args[MECS_QUERY_MAX_LEN];
} mecs_query_it_t;


#define MECS_COMPONENT_IDENT(T)                                                            mecs__component_##T##_id
#define MECS_COMPONENT_DECLARE(T)                                                          mecs_component_t MECS_COMPONENT_IDENT(T)
#if !defined(MECS_NO_DEFAULT_REGISTER_CPP_LIFETIME) && defined(__cplusplus)
    #define MECS_COMPONENT_REGISTER(io_registry, T)                                        MECS_COMPONENT_IDENT(T) = mecs_component_register_impl((io_registry), #T, sizeof(T), mecs_alignof(T), &mecs_ctor_cpp_impl<T>, &mecs_dtor_cpp_impl<T>, &mecs_move_and_dtor_cpp_impl<T> )
#else
    #define MECS_COMPONENT_REGISTER(io_registry, T)                                        MECS_COMPONENT_IDENT(T) = mecs_component_register_impl((io_registry), #T, sizeof(T), mecs_alignof(T), NULL, NULL, NULL )
#endif

#define MECS_COMPONENT_REGISTER_LIFE_TIME_HOOKS(io_registry, T, i_ctor_func_ptr, i_dtor_func_ptr, i_move_and_dtor_func_ptr) \
    mecs_component_register_life_time_hooks_impl((io_registry), MECS_COMPONENT_IDENT(T), (i_ctor_func_ptr), (i_dtor_func_ptr), (i_move_and_dtor_func_ptr))

#define mecs_component_add(io_registry, i_entity, T)                                       ((T*)mecs_component_add_impl((io_registry), (i_entity), MECS_COMPONENT_IDENT(T)))
#define mecs_component_remove(io_registry, i_entity, T)                                    mecs_component_remove_impl((io_registry), (i_entity), MECS_COMPONENT_IDENT(T))
#define mecs_component_has(i_registry, i_entity, T)                                        mecs_component_has_impl((i_registry), (i_entity), MECS_COMPONENT_IDENT(T))
#define mecs_component_get(io_registry, i_entity, T)                                       ((T*)mecs_component_get_impl((io_registry), (i_entity), MECS_COMPONENT_IDENT(T)))

mecs_component_t        mecs_component_register_impl(mecs_registry_t* io_registry, char const* name, size_t size, size_t alignment, mecs_ctor_func_t i_ctor, mecs_dtor_func_t i_dtor, mecs_move_and_dtor_func_t i_move_and_dtor);
void                    mecs_component_register_life_time_hooks_impl(mecs_registry_t* io_registry, mecs_component_t i_component, mecs_ctor_func_t i_ctor, mecs_dtor_func_t i_dtor, mecs_move_and_dtor_func_t i_move_and_dtor);

void*                   mecs_component_add_impl(mecs_registry_t* io_registry, mecs_entity_t i_entity, mecs_component_t i_component);
void                    mecs_component_remove_impl(mecs_registry_t* io_registry, mecs_entity_t i_entity, mecs_component_t i_component);
mecs_bool_t             mecs_component_has_impl(mecs_registry_t const* i_registry, mecs_entity_t i_entity, mecs_component_t i_component);
void*                   mecs_component_get_impl(mecs_registry_t* io_registry, mecs_entity_t i_entity, mecs_component_t i_component);

mecs_sparse_t*          mecs_component_get_sparse_element(mecs_component_store_t* i_component_store, mecs_entity_t i_entity);
mecs_dense_t*           mecs_component_get_dense_element(mecs_component_store_t* i_component_store, mecs_entity_size_t i_index);
void*                   mecs_component_get_component_element(mecs_component_store_t* i_component_store, mecs_entity_size_t i_index);
mecs_dense_t*           mecs_component_get_last_dense_element(mecs_component_store_t* i_component_store);
void*                   mecs_component_get_last_component_element(mecs_component_store_t* i_component_store);
mecs_bool_t             mecs_component_has_sparse_element(mecs_component_store_t const* i_component_store, mecs_entity_t i_entity);
mecs_sparse_t*          mecs_component_add_sparse_element(mecs_component_store_t* i_component_store, mecs_entity_t i_entity);
void*                   mecs_component_add_dense_element(mecs_component_store_t* i_component_store);

mecs_entity_t           mecs_entity_compose(mecs_entity_gen_t i_generation, mecs_entity_id_t i_id);
mecs_entity_id_t        mecs_entity_get_id(mecs_entity_t i_entity);
mecs_entity_gen_t       mecs_entity_get_generation(mecs_entity_t i_entity);
mecs_entity_t           mecs_entity_create(mecs_registry_t* io_registry);
mecs_bool_t             mecs_entity_destroy(mecs_registry_t* io_registry, mecs_entity_t i_entity);
mecs_bool_t             mecs_entity_is_destroyed(mecs_registry_t* io_registry, mecs_entity_t i_entity);

mecs_registry_t*        mecs_registry_create(mecs_component_size_t i_component_count_reserve);
void                    mecs_registry_destroy(mecs_registry_t* io_registry);

#define mecs_query_with(io_query_it, T)                    mecs_query_with_impl((io_query_it), MECS_COMPONENT_IDENT(T))
#define mecs_query_without(io_query_it, T)                 mecs_query_without_impl((io_query_it), MECS_COMPONENT_IDENT(T))
#define mecs_query_optional(io_query_it, T)                mecs_query_optional_impl((io_query_it), MECS_COMPONENT_IDENT(T))
#define mecs_query_component_has(io_query_it, T, i_index)  mecs_query_component_has_impl((io_query_it), MECS_COMPONENT_IDENT(T), i_index)
#define mecs_query_component_get(io_query_it, T, i_index)  ((T*)mecs_query_component_get_impl((io_query_it), MECS_COMPONENT_IDENT(T), i_index))

mecs_query_it_t         mecs_query_create(void);
void                    mecs_query_with_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component);
void                    mecs_query_without_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component);
void                    mecs_query_optional_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component);
void                    mecs_query_begin(mecs_registry_t* io_registry, mecs_query_it_t* io_query_it);
mecs_bool_t             mecs_query_next(mecs_query_it_t* io_query_it);
mecs_entity_t           mecs_query_entity_get(mecs_query_it_t* io_query_it);
mecs_bool_t             mecs_query_component_has_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component, size_t i_index);
void*                   mecs_query_component_get_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component, size_t i_index);

#endif /* MECS_H */

#ifdef MECS_IMPLEMENTATION

/* --------------------------------------------------
Implementaton of compiler and platform agnostic standard library types/functions.
-------------------------------------------------- */

/* Provide custom or default implementation for the number of bits in a char. */
#if !defined(MECS_CHAR_BIT)
    #if defined(_cplusplus)
        #include <climits>
    #else
        #include <limits.h>
    #endif
    #define MECS_CHAR_BIT CHAR_BIT
#endif

/* Provide custom or default implementation for asserts which this library heavily depends on for error checking. */
#if !defined(mecs_assert)
    #if defined(_cplusplus)
        #include <cassert>
    #else
        #include <assert.h>
    #endif
    #define mecs_assert(i_condition) assert(i_condition)
#endif

/* Provide custom or default implementation for memset. */
#if !defined(mecs_memset)
    #if defined(__cplusplus)
        #include <cstring>
        #define mecs_memset(i_ptr, i_value, i_size)     std::memset((i_ptr), (i_value), (i_size))
    #else
        #include <string.h>
        #define mecs_memset(i_ptr, i_value, i_size)     memset((i_ptr), (i_value), (i_size))
    #endif
#endif

/* Memory management. Provide custom allocator implementation or default implementation build on the C standard library. */
#if defined(mecs_realloc) && !defined(mecs_free) || !defined(mecs_realloc) && defined(mecs_free)
    #error "You must define both mecs_realloc and mecs_free."
#endif
#if defined(mecs_realloc_aligned) && !defined(mecs_free_aligned) || !defined(mecs_realloc_aligned) && defined(mecs_free_aligned)
    #error "You must define both mecs_realloc_aligned and mecs_free_aligned."
#endif

#if !defined(mecs_realloc) && !defined(mecs_free)
    #if defined(_cplusplus)
        #include <cstdlib>
    #else
        #include <stdlib.h>
    #endif
    #define mecs_realloc(io_data, i_size)                       realloc((io_data), (i_size))
    #define mecs_free(io_data)                                  free(io_data)
#endif

#if !defined(mecs_realloc_aligned) && !defined(mecs_free_aligned)
    #define mecs_realloc_aligned(io_data, i_size, i_alignment)  mecs_realloc_aligned_impl((io_data), (i_size), (i_alignment))
    #define mecs_free_aligned(io_data)                          mecs_free_aligned_impl(io_data)
#endif

    void* mecs_realloc_aligned_impl(void* io_data, size_t i_size, size_t i_alignment)
    {
        size_t offset;
        mecs_uint8_t* pointer;
        mecs_uint8_t* return_pointer;
        size_t offset_from_prev_align;
        size_t offset_to_next_align;
        mecs_assert(i_alignment < 256); /* Maximum supported alignment. */

        pointer = ((mecs_uint8_t*)io_data);
        if (pointer != NULL)
        {
            offset = *(((mecs_uint8_t*)io_data) - 1);
            pointer = (pointer - offset);
        }

        /* Realloc can return any address but our desired alignement can be at most i_alignment bytes away. */
        pointer = (mecs_uint8_t*)mecs_realloc(pointer, i_size + i_alignment); 
        mecs_assert(pointer != NULL);
        
        /* Round to the next aligned address. 
        Store the offset from our allocation to this address in the byte before it so we can retreive the orginal address later. */
        offset_from_prev_align = (size_t)pointer % i_alignment;
        offset_to_next_align = i_alignment - offset_from_prev_align;
        return_pointer = pointer + offset_to_next_align;

        *(mecs_uint8_t*)(return_pointer - 1) = (mecs_uint8_t)offset_to_next_align;

        return (void*)return_pointer;
    }

    void mecs_free_aligned_impl(void* io_data)
    {
        size_t offset;
        offset = *(((mecs_uint8_t*)io_data) - 1);
        mecs_free(((mecs_uint8_t*)io_data) - offset);
    }

/* Allocation helper functions. */
#define mecs_malloc_aligned(i_size, i_alignment)           mecs_realloc_aligned(NULL, (i_size), (i_alignment))
#define mecs_malloc_type(T)                                (T*)mecs_realloc(NULL, sizeof(T))
#define mecs_malloc_arr(T, i_len)                          (T*)mecs_realloc(NULL, (i_len) * sizeof(T))
#define mecs_realloc_arr(T, i_ptr, i_len)                  (T*)mecs_realloc((i_ptr), (i_len) * sizeof(T))

/* Implement hooks for C++ constructor/destructor. */
#if !defined(MECS_NO_DEFAULT_REGISTER_CPP_LIFETIME) && defined(__cplusplus)
    #include <new>

    template<typename T>
    inline void mecs_ctor_cpp_impl(void* io_data)
    {
        ::new(io_data) T;
    }

    template<typename T>
    inline void mecs_dtor_cpp_impl(void* io_data)
    {
        ((T*)io_data)->~T();
    }

    template<typename T>
    inline void mecs_move_and_dtor_cpp_impl(void* io_src_to_move, void* io_dst_to_destruct)
    {
        T* dst = (T*)io_dst_to_destruct; 
        T* src = (T*)io_src_to_move; 
        *dst = static_cast<T&&>(*src);
        src->~T();
    }
#endif

/* --------------------------------------------------
Implementation of the core library.
-------------------------------------------------- */

mecs_entity_t mecs_entity_compose(mecs_entity_gen_t i_generation, mecs_entity_id_t i_id)
{
    mecs_assert(i_id == mecs_entity_get_id(i_id));
    return ((mecs_entity_t)(i_generation << MECS_ENTITY_ID_BITCOUNT)) | ((mecs_entity_t)i_id);
}

mecs_entity_id_t mecs_entity_get_id(mecs_entity_t i_entity)
{
    /* Convert bitcount into a mask to prevent needing a second constant that can get out of sync. Should get optimized into a compile time constant by the compiler. */
    mecs_entity_t const mask = ((mecs_entity_t)-1) >> ((sizeof(mecs_entity_t) * MECS_CHAR_BIT) - MECS_ENTITY_ID_BITCOUNT);
    return (mecs_entity_id_t)(i_entity & mask);
}

mecs_entity_gen_t mecs_entity_get_generation(mecs_entity_t i_entity)
{
    return (mecs_entity_gen_t)(i_entity >> MECS_ENTITY_ID_BITCOUNT);
}

mecs_registry_t* mecs_registry_create(mecs_component_size_t i_component_count_reserve) 
{
    mecs_registry_t* registry;
    registry = mecs_malloc_type(mecs_registry_t);
    if (registry == NULL)
    {
        return NULL;
    }

    /* Reserve space for the components we will register. We allow growing beyond this memory but will always try to use as little memory as possible. 
       This is because registring new components should be an infrequent operation, likely only taking place during init. */
    registry->components_len = 0;
    registry->components_cap = i_component_count_reserve;
    registry->components = NULL;
    if (registry->components_cap != 0)
    {
        registry->components = mecs_malloc_arr(mecs_component_store_t, registry->components_cap);
        if (registry->components == NULL)
        {
            mecs_free(registry);
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        mecs_memset(registry->components, 0x00, sizeof(mecs_component_store_t) * registry->components_cap);
    }

    /* Reserve space for entities to prevent frequent growing of the array when the first entities get added. */
    registry->next_free_entity = 0;
    registry->entities_len = 0;
    registry->entities_cap = 8; 
    registry->entities = mecs_malloc_arr(mecs_entity_t, registry->entities_cap);
    if (registry->entities == NULL)
    {
        if (registry->components != NULL)
        {
            mecs_free(registry->components);
        }
        mecs_free(registry);
        mecs_assert(MECS_FALSE);
        return NULL;
    }

    registry->next_free_entity = MECS_ENTITY_ID_INVALID;

    return registry;
}

void mecs_registry_destroy(mecs_registry_t* io_registry) 
{
    size_t i;
    size_t block_idx;
    size_t block_offset;
    size_t component_idx;
    void* component;
    mecs_component_store_t* component_store;
    mecs_assert(io_registry != NULL);

    for (i = 0; i < io_registry->components_len; ++i)
    {
        component_store = &io_registry->components[i];

        /* Free sparse */
        for (block_idx = 0; block_idx < component_store->sparse_len; ++block_idx)
        {
            if (component_store->sparse[block_idx] != NULL)
            {
                mecs_free(component_store->sparse[block_idx]);
            }
        }
        if (component_store->sparse != NULL)
        {
            mecs_free(component_store->sparse);
        }

        /* Free components */
        component_idx = 0;
        for (block_idx = 0; block_idx < component_store->components_len; ++block_idx)
        {
            block_offset = 0;
            while (component_idx < component_store->entities_count && block_offset < MECS_PAGE_LEN_DENSE)
            {
                if (component_store->dtor_func != NULL)
                {
                    component = (void*)(((char*)component_store->components[block_idx]) + (block_offset * component_store->size));
                    component_store->dtor_func(component);
                }
                component_idx += 1;
                block_offset += 1;
            }
            mecs_free_aligned(component_store->components[block_idx]);
        }
        if (component_store->components != NULL)
        {
            mecs_free(component_store->components);
        }

        /* Free dense */
        if (component_store->dense != NULL)
        {
            mecs_free(component_store->dense);
        }
    }

    if (io_registry->components_cap != 0)
    {
        mecs_memset(io_registry->components, 0xCC, sizeof(mecs_component_store_t) * io_registry->components_cap);
        mecs_free(io_registry->components);
    }

    if (io_registry->entities_cap != 0)
    {
        mecs_memset(io_registry->entities, 0xCC, sizeof(mecs_entity_t) * io_registry->entities_cap);
        mecs_free(io_registry->entities);
    }

    mecs_memset(io_registry, 0xCC, sizeof(mecs_registry_t));
    mecs_free(io_registry);
}

mecs_component_t mecs_component_register_impl(mecs_registry_t* io_registry, char const* i_name, size_t i_size, size_t i_alignment, mecs_ctor_func_t i_ctor, mecs_dtor_func_t i_dtor, mecs_move_and_dtor_func_t i_move_and_dtor) 
{
    mecs_component_store_t* components_grown;
    size_t new_capacity;
    mecs_component_t component;
    mecs_assert(io_registry != NULL);

    if (io_registry->components_len >= io_registry->components_cap)
    {
        /* Grow array by 1 to guarantee minimal memory usage. */
        new_capacity = io_registry->components_cap + 1;
        components_grown = mecs_realloc_arr(mecs_component_store_t, io_registry->components, new_capacity);
        if (components_grown == NULL)
        {
            mecs_assert(MECS_FALSE);
            return MECS_COMPONENT_INVALID;
        }
        mecs_memset(components_grown + io_registry->components_cap, 0x00, sizeof(mecs_component_store_t));
        io_registry->components = components_grown;
        io_registry->components_cap = new_capacity;
    }

    component = io_registry->components_len;
    if (io_registry->components[component].name != NULL)
    {
        mecs_assert(MECS_FALSE);
        return component; /* Already registered. */
    }

    io_registry->components[component].name = i_name;
    io_registry->components[component].size = i_size;
    io_registry->components[component].alignment = i_alignment;
    io_registry->components[component].ctor_func = i_ctor;
    io_registry->components[component].dtor_func = i_dtor;
    io_registry->components[component].move_and_dtor_func = i_move_and_dtor;

    io_registry->components[component].sparse = NULL;
    io_registry->components[component].sparse_len = 0;
    io_registry->components[component].dense = NULL;
    io_registry->components[component].entities_count = 0;
    io_registry->components[component].components = NULL;
    io_registry->components[component].components_len = 0;
    io_registry->components_len += 1;
    return component;
}

void mecs_component_register_life_time_hooks_impl(mecs_registry_t* io_registry, mecs_component_t i_component, mecs_ctor_func_t i_ctor, mecs_dtor_func_t i_dtor, mecs_move_and_dtor_func_t i_move_and_dtor)
{
    mecs_assert(io_registry);
    io_registry->components[i_component].ctor_func = i_ctor;
    io_registry->components[i_component].dtor_func = i_dtor;
    io_registry->components[i_component].move_and_dtor_func = i_move_and_dtor;
}

void* mecs_component_add_impl(mecs_registry_t* io_registry, mecs_entity_t i_entity, mecs_component_t i_component)
{
    mecs_component_store_t* component_store; 
    mecs_sparse_t* sparse_elem;
    mecs_dense_t* dense_elem; 
    void* component_elem;
    mecs_assert(io_registry != NULL);
    mecs_assert(i_component < io_registry->components_len);

    component_store = &io_registry->components[i_component];
    sparse_elem = mecs_component_add_sparse_element(component_store, i_entity);
    component_elem = mecs_component_add_dense_element(component_store); /* Allocating a new dense elements will grow both the components array and dense array to match. */
    dense_elem = mecs_component_get_last_dense_element(component_store);

    if (component_store->ctor_func != NULL)
    {
        component_store->ctor_func(component_elem);
    }
    
    *sparse_elem = mecs_entity_compose(mecs_entity_get_generation(i_entity), component_store->entities_count - 1); /* Build sparse element out of version and dense index. */
    *dense_elem  = i_entity;
    return component_elem;
}

void mecs_component_remove_impl(mecs_registry_t* io_registry, mecs_entity_t i_entity, mecs_component_t i_component)
{
    mecs_component_store_t* component_store; 

    mecs_sparse_t* entity_sparse_elem; 
    mecs_entity_size_t entity_dense_index;
    mecs_dense_t* entity_dense_elem; 
    void* entity_component_elem;

    mecs_sparse_t* last_entity_sparse_elem; 
    mecs_dense_t* last_entity_dense_elem; 
    void* last_entity_component_elem;
    mecs_assert(io_registry != NULL);

    component_store = &io_registry->components[i_component];

    entity_sparse_elem = mecs_component_get_sparse_element(component_store, i_entity);
    entity_dense_index = mecs_entity_get_id(*entity_sparse_elem); /* Get the dense index from the entity version - dense index pair. */
    entity_dense_elem = mecs_component_get_dense_element(component_store, entity_dense_index);
    entity_component_elem = mecs_component_get_component_element(component_store, entity_dense_index);

    if (component_store->entities_count != 1)
    {
        /* Move the last component in place of the component we want to remove. */
        last_entity_dense_elem = mecs_component_get_last_dense_element(component_store);
        last_entity_sparse_elem = mecs_component_get_sparse_element(component_store, *last_entity_dense_elem);
        last_entity_component_elem = mecs_component_get_last_component_element(component_store);

        if (component_store->move_and_dtor_func != NULL)
        {
            component_store->move_and_dtor_func(last_entity_component_elem, entity_component_elem);
        }
        else
        {
            if (component_store->dtor_func != NULL)
            {
                component_store->dtor_func(entity_component_elem);
            }
            memcpy(entity_component_elem, last_entity_component_elem, component_store->size);
        }

        *entity_dense_elem = *last_entity_dense_elem;
        *last_entity_sparse_elem = mecs_entity_compose(mecs_entity_get_generation(*last_entity_sparse_elem), entity_dense_index); /* Override the new dense index of the last entity. */
    }
    else 
    {
        if (component_store->dtor_func != NULL)
        {
            component_store->dtor_func(entity_component_elem);
        }
    }

    /* Destroy the entry associated with this entity. */
    *entity_sparse_elem = MECS_SPARSE_INVALID;
    component_store->entities_count -= 1;
}

mecs_bool_t mecs_component_has_impl(mecs_registry_t const* i_registry, mecs_entity_t i_entity, mecs_component_t i_component)
{
    mecs_assert(i_registry != NULL);
    return mecs_component_has_sparse_element(&i_registry->components[i_component], i_entity);
}

void* mecs_component_get_impl(mecs_registry_t* io_registry, mecs_entity_t i_entity, mecs_component_t i_component)
{
    mecs_component_store_t* component_store; 
    mecs_sparse_t* sparse_elem;
    mecs_entity_size_t dense_index;
    mecs_assert(io_registry != NULL);

    component_store = &io_registry->components[i_component];
    sparse_elem = mecs_component_get_sparse_element(component_store, i_entity);
    dense_index = mecs_entity_get_id(*sparse_elem);
    return mecs_component_get_component_element(component_store, dense_index);
}

mecs_sparse_t* mecs_component_get_sparse_element(mecs_component_store_t* i_component_store, mecs_entity_t i_entity)
{
    mecs_entity_size_t page_index;
    mecs_entity_size_t page_offset;
    mecs_assert(i_component_store != NULL);

    page_index = mecs_entity_get_id(i_entity) / MECS_PAGE_LEN_SPARSE;
    page_offset = mecs_entity_get_id(i_entity) % MECS_PAGE_LEN_SPARSE;
    return &i_component_store->sparse[page_index]->block[page_offset];
}

mecs_dense_t* mecs_component_get_dense_element(mecs_component_store_t* i_component_store, mecs_entity_size_t i_index)
{
    mecs_assert(i_component_store != NULL);
    return &i_component_store->dense[i_index];
}

void* mecs_component_get_component_element(mecs_component_store_t* i_component_store, mecs_entity_size_t i_index)
{
    mecs_entity_size_t page_index;
    mecs_entity_size_t page_offset;
    void* component_page;
    void* component;
    mecs_assert(i_component_store != NULL);

    page_index = i_index / MECS_PAGE_LEN_DENSE;
    page_offset = i_index % MECS_PAGE_LEN_DENSE;
    component_page = i_component_store->components[page_index];
    component = (void*)(((char*)component_page) + (page_offset * i_component_store->size));
    return component;
}

mecs_dense_t* mecs_component_get_last_dense_element(mecs_component_store_t* i_component_store)
{
    return &i_component_store->dense[i_component_store->entities_count - 1];
}

void* mecs_component_get_last_component_element(mecs_component_store_t* i_component_store)
{
    mecs_entity_size_t page_index;
    mecs_entity_size_t page_offset;
    void* component_page;
    void* component;
    mecs_assert(i_component_store != NULL);

    page_index = i_component_store->components_len - 1;
    page_offset = (i_component_store->entities_count - 1) % MECS_PAGE_LEN_SPARSE;
    component_page = i_component_store->components[page_index];
    component = (void*)(((char*)component_page) + (page_offset * i_component_store->size));
    return component;
}

mecs_bool_t mecs_component_has_sparse_element(mecs_component_store_t const* i_component_store, mecs_entity_t i_entity)
{
    mecs_entity_size_t page_index;
    mecs_entity_size_t page_offset;
    mecs_sparse_t sparse_elem;
    mecs_assert(i_component_store != NULL);

    page_index = mecs_entity_get_id(i_entity) / MECS_PAGE_LEN_SPARSE;
    if (page_index >= i_component_store->sparse_len)
    {
        return MECS_FALSE;
    }

    /* If there is no entity sparse_elem will have MECS_ENTITY_GENERATION_INVALID, else make sure the entity stored here has the same generation. 
       Prevents accessing the dense array to check. */ 
    page_offset = mecs_entity_get_id(i_entity) % MECS_PAGE_LEN_SPARSE;
    sparse_elem = i_component_store->sparse[page_index]->block[page_offset];
    if (mecs_entity_get_generation(sparse_elem) != mecs_entity_get_generation(i_entity)) 
    {
        return MECS_FALSE;
    }

    return MECS_TRUE;
}

mecs_sparse_t* mecs_component_add_sparse_element(mecs_component_store_t* i_component_store, mecs_entity_t i_entity)
{
    mecs_entity_size_t page_index;
    mecs_entity_size_t page_offset;
    mecs_sparse_block_t** sparse_grown;
    mecs_entity_size_t sparse_grown_count;
    mecs_sparse_block_t* sparse_page;
    mecs_assert(i_component_store != NULL);

    page_index = mecs_entity_get_id(i_entity) / MECS_PAGE_LEN_SPARSE;
    page_offset = mecs_entity_get_id(i_entity) % MECS_PAGE_LEN_SPARSE;
    
    /* Get the page in the array of sparse pages that holds this entity. */
    if (page_index >= i_component_store->sparse_len)
    {
        /* Grow the array of sparse pages so we can hold the page for this entity. Don't allocate the page for this entity yet. */
        sparse_grown = mecs_realloc_arr(mecs_sparse_block_t*, i_component_store->sparse, page_index + 1);
        if (sparse_grown == NULL)
        {
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        sparse_grown_count = (page_index  - i_component_store->sparse_len) + 1;
        memset(sparse_grown + i_component_store->sparse_len, 0x00, sparse_grown_count  * sizeof(mecs_sparse_block_t*)); /* Initialise all entires to NULL, an empty page. */
        i_component_store->sparse = sparse_grown;
        i_component_store->sparse_len = page_index + 1;
    }
    sparse_page = i_component_store->sparse[page_index];

    /* Allocate a new sparse page if this page is empty. */
    if (sparse_page == NULL)
    {
        sparse_page = mecs_malloc_type(mecs_sparse_block_t);
        if (sparse_page == NULL)
        {
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        memset(sparse_page, 0xFF, sizeof(mecs_sparse_block_t)); /* Initialise all entires MECS_SPARSE_INVALID, indicates there is no component for this entity. */
        i_component_store->sparse[page_index] = sparse_page;
    }

    return &sparse_page->block[page_offset];
}

void* mecs_component_add_dense_element(mecs_component_store_t* i_component_store)
{
    mecs_entity_size_t components_page_offset;
    mecs_entity_size_t components_page_index;
    mecs_entity_size_t dense_grown_offset;
    mecs_entity_size_t dense_grown_size;
    mecs_dense_t* dense_grown;

    void** components_grown;
    void* components_page;
    void* component;
    mecs_assert(i_component_store != NULL);

    components_page_index = i_component_store->entities_count / MECS_PAGE_LEN_SPARSE;
    components_page_offset =i_component_store->entities_count % MECS_PAGE_LEN_SPARSE;

    /* Allocate a new page for the component if required. */
    if (components_page_index >= i_component_store->components_len)
    {
        /* Grow the dense array to match the entries in the components array after allocating a new page. */
        dense_grown_offset = i_component_store->components_len * MECS_PAGE_LEN_DENSE;
        dense_grown_size = (i_component_store->components_len + 1) * MECS_PAGE_LEN_DENSE;
        dense_grown = mecs_realloc_arr(mecs_dense_t, i_component_store->dense, dense_grown_size);
        if (dense_grown == NULL)
        {
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        memset(dense_grown + dense_grown_offset, 0xFF, MECS_PAGE_LEN_DENSE * sizeof(mecs_dense_t)); /* Initialise all entires to invalid entity. */
        i_component_store->dense = dense_grown;

        /* Allocate a new component page. */
        components_page = mecs_malloc_aligned(MECS_PAGE_LEN_DENSE * i_component_store->size, i_component_store->alignment);
        if (components_page == NULL)
        {
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        
        /* Grow the array of component pages so we can hold the new page. */
        components_grown = mecs_realloc_arr(void*, i_component_store->components, i_component_store->components_len + 1);
        if (components_grown == NULL)
        {
            mecs_free(components_page);
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        i_component_store->components = components_grown;
        i_component_store->components[i_component_store->components_len] = components_page;
        i_component_store->components_len += 1;
    }
    components_page = i_component_store->components[components_page_index];

    component = (void*)(((char*)components_page) + (components_page_offset * i_component_store->size));
    i_component_store->entities_count += 1;
    return component;
}

mecs_entity_t mecs_entity_create(mecs_registry_t* io_registry)
{
    mecs_entity_id_t free_entity_id;
    mecs_entity_gen_t free_entity_gen;
    mecs_entity_t* entities_grown;
    mecs_entity_size_t new_capacity;
    mecs_entity_t entity;
    mecs_assert(io_registry != NULL);
    
    free_entity_id = mecs_entity_get_id(io_registry->next_free_entity);
    if (free_entity_id == MECS_ENTITY_ID_INVALID)
    {
        /* There is no entity id we can re-use so create a new entity id. This potenially involves allocating more memory. */

        if (io_registry->entities_len >= io_registry->entities_cap)
        {
            /* Double array capcity to guarantee O(1) amortized. */
            if (io_registry->entities_cap <= ((mecs_entity_size_t)-1) - io_registry->entities_cap)
            {
                new_capacity = io_registry->entities_cap * 2;
            }
            else 
            {
                new_capacity = (mecs_entity_size_t)-1; /* Ensure we don't overflow and utalise the full range of available ids. */
            }

            entities_grown = mecs_realloc_arr(mecs_entity_t, io_registry->entities, new_capacity);
            if (entities_grown == NULL)
            {
                mecs_assert(MECS_FALSE);
                return MECS_ENTITY_INVALID;
            }
            mecs_memset(entities_grown + io_registry->entities_cap, 0x00, sizeof(mecs_entity_t));
            io_registry->entities = entities_grown;
            io_registry->entities_cap = new_capacity;
        }

        entity = (mecs_entity_t)io_registry->entities_len;
        io_registry->entities[(mecs_entity_id_t)entity] = entity; /* Genaration is 0 so can use as index directly. */
        io_registry->entities_len += 1;
        return entity;
    }
    else 
    {
        /* Re-use an entity id by popping an entry of the implicit linked list of destroyed entities. */
        free_entity_gen = mecs_entity_get_generation(io_registry->entities[free_entity_id]);
        io_registry->next_free_entity = mecs_entity_get_id(io_registry->entities[free_entity_id]);
        entity = mecs_entity_compose(free_entity_gen, free_entity_id);

        io_registry->entities[free_entity_id] = entity;
        return entity;
    }
}

mecs_bool_t mecs_entity_destroy(mecs_registry_t* io_registry, mecs_entity_t i_entity)
{
    mecs_component_size_t i;
    mecs_entity_id_t destroyed_id;
    mecs_entity_gen_t destroyed_gen;
    mecs_entity_id_t next_free_entity_id;
    mecs_assert(io_registry != NULL);

    if (mecs_entity_is_destroyed(io_registry, i_entity)) 
    {
        mecs_assert(MECS_FALSE);
        return MECS_FALSE;
    }

    /* Remove all components. */
    for (i = 0; i < io_registry->components_len; ++i)
    {
        if (mecs_component_has_impl(io_registry, i_entity, i))
        {
            mecs_component_remove_impl(io_registry, i_entity, i);
        }
    }

    destroyed_id = mecs_entity_get_id(i_entity);
    destroyed_gen = mecs_entity_get_generation(i_entity);
    next_free_entity_id = mecs_entity_get_id(io_registry->next_free_entity);

    /* Increase generation so any entities still using this id become invalid as the generation won't match. */
    destroyed_gen += 1;

    /* Have the entity we're destroying point to the next free entity and update the head of our list to point to the entity we're destroying. */
    io_registry->entities[destroyed_id] = mecs_entity_compose(destroyed_gen, next_free_entity_id);
    io_registry->next_free_entity = destroyed_id;
    return MECS_TRUE;
}

mecs_bool_t mecs_entity_is_destroyed(mecs_registry_t* io_registry, mecs_entity_t i_entity)
{
    mecs_entity_id_t entity_id;
    mecs_entity_gen_t entity_gen;
    mecs_entity_gen_t current_gen;
    mecs_assert(io_registry != NULL);

    entity_id = mecs_entity_get_id(i_entity);
    mecs_assert(entity_id < io_registry->entities_len);

    entity_gen = mecs_entity_get_generation(i_entity);
    current_gen = mecs_entity_get_generation(io_registry->entities[entity_id]);
    if (entity_gen != current_gen)
    {
        return MECS_TRUE;
    }
    return MECS_FALSE;
}

mecs_query_it_t mecs_query_create(void)
{
    mecs_query_it_t query;
    query.current = NULL;
    query.end = NULL;
    query.component_stores = NULL;
    query.args_len = 0;
    return query;
}

void mecs_query_with_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component)
{
    mecs_assert(io_query_it != NULL);
    mecs_assert(io_query_it->args_len != MECS_QUERY_MAX_LEN);
    io_query_it->args[io_query_it->args_len].type = MECS_QUERY_TYPE_WITH;
    io_query_it->args[io_query_it->args_len].component = i_component;
    io_query_it->args_len += 1;
}

void mecs_query_without_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component)
{
    mecs_assert(io_query_it != NULL);
    mecs_assert(io_query_it->args_len != MECS_QUERY_MAX_LEN);
    io_query_it->args[io_query_it->args_len].type = MECS_QUERY_TYPE_WITHOUT;
    io_query_it->args[io_query_it->args_len].component = i_component;
    io_query_it->args_len += 1;
}

void mecs_query_optional_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component)
{
    mecs_assert(io_query_it != NULL);
    mecs_assert(io_query_it->args_len != MECS_QUERY_MAX_LEN);
    io_query_it->args[io_query_it->args_len].type = MECS_QUERY_TYPE_OPTIONAL;
    io_query_it->args[io_query_it->args_len].component = i_component;
    io_query_it->args_len += 1;
}

void mecs_query_begin(mecs_registry_t* io_registry, mecs_query_it_t* io_query_it)
{
    size_t arg_idx;
    mecs_query_type_t type;
    mecs_component_t component;
    mecs_entity_size_t entities_count;

    mecs_entity_size_t smallest_entities_count;
    mecs_component_store_t* smallest_component_store;
    mecs_assert(io_registry != NULL);
    mecs_assert(io_query_it != NULL);

    /* Find the component store with the smallest number of entities to become the iterator base. */
    smallest_entities_count = (mecs_entity_size_t)-1;
    smallest_component_store = NULL;
    for (arg_idx = 0; arg_idx < io_query_it->args_len; ++arg_idx)
    {
        /* Only with query arguments can form the base for the iterator as they are the only type that narrows down the set entities we have it iterate to a single array. */
        type = io_query_it->args[arg_idx].type;
        component = io_query_it->args[arg_idx].component;
        if (type == MECS_QUERY_TYPE_WITH)
        {
            entities_count = io_registry->components[component].entities_count;
            if (entities_count < smallest_entities_count)
            {
                smallest_entities_count = entities_count;
                smallest_component_store = &io_registry->components[component];
            }
        }
    }

    if (smallest_component_store == NULL)
    {
        /* This query matches all entities. We need at least 1 query arg to form the base of iteration. */
        mecs_assert(MECS_FALSE);
        return;
    }

    io_query_it->current = smallest_component_store->dense;
    io_query_it->end = smallest_component_store->dense + smallest_component_store->entities_count;
    io_query_it->component_stores = io_registry->components;
}

mecs_bool_t mecs_query_next(mecs_query_it_t* io_query_it)
{
    size_t arg_idx;
    mecs_query_type_t type;
    mecs_component_t component;
    mecs_component_store_t* component_store;
    mecs_bool_t has_component;

    while(io_query_it->current < io_query_it->end)
    {
        for (arg_idx = 0; arg_idx < io_query_it->args_len; ++arg_idx)
        {
            /* Check if the entity matches the query argument. */
            type = io_query_it->args[arg_idx].type;
            component = io_query_it->args[arg_idx].component;
            component_store = &io_query_it->component_stores[component];
            has_component = mecs_component_has_sparse_element(component_store, *io_query_it->current);

            if (has_component)
            {
                io_query_it->sparse_elements[arg_idx] = *mecs_component_get_sparse_element(component_store, *io_query_it->current);
            }
            else
            {
                io_query_it->sparse_elements[arg_idx] = MECS_SPARSE_INVALID;
            }

            if((type == MECS_QUERY_TYPE_WITH && has_component) ||
               (type == MECS_QUERY_TYPE_WITHOUT && !has_component) ||
               (type == MECS_QUERY_TYPE_OPTIONAL)
            )
            {
                continue;
            }
            goto l_next_entity; /* Query arg does not match the current entity. Move on to the next entity. */ 
        }

        io_query_it->current += 1;           
        return MECS_TRUE; /* All query args match the current entity. Return this entity to the caller. */

        l_next_entity:;
        io_query_it->current += 1;           
    }
    return MECS_FALSE;
}

mecs_entity_t mecs_query_entity_get(mecs_query_it_t* io_query_it)
{
    return *(io_query_it->current - 1);
}

mecs_bool_t mecs_query_component_has_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component, size_t i_index)
{
    mecs_assert(io_query_it->args[i_index].component == i_component);
    return io_query_it->sparse_elements[i_index] != MECS_SPARSE_INVALID;
}

void* mecs_query_component_get_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component, size_t i_index)
{
    mecs_assert(io_query_it->args[i_index].component == i_component);
    return mecs_component_get_component_element(&io_query_it->component_stores[i_component], mecs_entity_get_id(io_query_it->sparse_elements[i_index]));
}

#endif /* MECS_IMPLEMENTATION */
