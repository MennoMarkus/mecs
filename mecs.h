#ifndef MECS_H
#define MECS_H

/* TODO: Turns these std defines into library options. */

#include <stdint.h>
#include <limits.h>
typedef uint8_t mecs_uint8_t;
typedef uint16_t mecs_uint16_t;
typedef uint32_t mecs_uint32_t;
#define MECS_CHAR_BIT CHAR_BIT

typedef char mecs_bool_t;
#define MECS_TRUE 1
#define MECS_FALSE 0

#include <assert.h>
#define mecs_assert(i_condition)                assert(i_condition)

#include <stdlib.h>
#define mecs_free(i_ptr)                        free(i_ptr)
#define mecs_malloc(i_size)                     malloc(i_size)
#define mecs_calloc(i_len, i_size)              calloc((i_len), (i_size))
#define mecs_realloc(i_ptr, i_size)             realloc((i_ptr), (i_size))

#define mecs_malloc_type(i_type)                (i_type*)mecs_malloc(sizeof(i_type))
#define mecs_calloc_type(i_type)                (i_type*)mecs_calloc(1, sizeof(i_type))
#define mecs_calloc_arr(i_len, i_type)          (i_type*)mecs_calloc((i_len), sizeof(i_type))
#define mecs_realloc_arr(i_ptr, i_len, i_type)  (i_type*)mecs_realloc((i_ptr), (i_len) * sizeof(i_type))

#include <string.h>
#define mecs_memset(i_ptr, i_value, i_size)     memset((i_ptr), (i_value), (i_size))

#if defined(__cplusplus)
    #define mecs_alignof(i_type)                static_cast<size_t>(alignof(i_type))
#elif defined(_MSC_VER)
    #define mecs_alignof(i_type)                (size_t)__alignof(i_type)
#elif defined(__GNUC__)
    #define mecs_alignof(i_type)                (size_t)__alignof__(i_type)
#else
    #define mecs_alignof(i_type)                (((size_t)&( (struct{ mecs_uint8_t b; i_type t; }* )0 )->t)
#endif


/* TODO: Expose defines to user as library options. /

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
typedef size_t mecs_component_t;
#define MECS_COMPONENT_INVALID ((mecs_component_t)-1)

typedef mecs_entity_t mecs_sparse_t;
typedef mecs_entity_t mecs_dense_t;
#define MECS_SPARSE_INVALID ((mecs_sparse_t)-1) 
#define MECS_PAGE_LEN_SPARSE (4096 / sizeof(mecs_sparse_t)) /* Default chosen to equal as many entities as fit in the common page size (4kb). */
#define MECS_PAGE_LEN_DENSE 512

typedef struct
{ 
    mecs_sparse_t block[MECS_PAGE_LEN_SPARSE]; 
} mecs_sparse_block_t;

typedef struct 
{
    char const* name;
    size_t size;
    size_t alignment;
    
    void* ctor;
    void* dtor;

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
    size_t components_len;
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

/* Uncached query. Quick to construct and lives on the stack with now references by the registry but potentially slower to iterate. */
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


#define MECS_COMPONENT_IDENT(i_type)                            mecs__component_##i_type##_id
#define MECS_COMPONENT_DECLARE(i_type)                          mecs_component_t MECS_COMPONENT_IDENT(i_type)
#define MECS_COMPONENT_REGISTER(io_registry, i_type)            MECS_COMPONENT_IDENT(i_type) = mecs_component_register_impl((io_registry), #i_type, sizeof(i_type), mecs_alignof(i_type) )
#define mecs_component_add(io_registry, i_entity, i_type)       ((i_type*)mecs_component_add_impl((io_registry), (i_entity), MECS_COMPONENT_IDENT(i_type)))
#define mecs_component_remove(io_registry, i_entity, i_type)    mecs_component_remove_impl((io_registry), (i_entity), MECS_COMPONENT_IDENT(i_type))
#define mecs_component_has(i_registry, i_entity, i_type)        mecs_component_has_impl((i_registry), (i_entity), MECS_COMPONENT_IDENT(i_type))
#define mecs_component_get(io_registry, i_entity, i_type)       ((i_type*)mecs_component_get_impl((io_registry), (i_entity), MECS_COMPONENT_IDENT(i_type)))

mecs_component_t        mecs_component_register_impl(mecs_registry_t* io_registry, char const* name, size_t size, size_t alignment);
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

mecs_registry_t*        mecs_registry_create(size_t i_component_count_reserve);
void                    mecs_registry_destroy(mecs_registry_t* io_registry);

#define mecs_query_with(io_query_it, i_type)                    mecs_query_with_impl((io_query_it), MECS_COMPONENT_IDENT(i_type))
#define mecs_query_without(io_query_it, i_type)                 mecs_query_without_impl((io_query_it), MECS_COMPONENT_IDENT(i_type))
#define mecs_query_optional(io_query_it, i_type)                mecs_query_optional_impl((io_query_it), MECS_COMPONENT_IDENT(i_type))
#define mecs_query_component_has(io_query_it, i_type, i_index)  mecs_query_component_has_impl((io_query_it), MECS_COMPONENT_IDENT(i_type), i_index)
#define mecs_query_component_get(io_query_it, i_type, i_index)  ((i_type*)mecs_query_component_get_impl((io_query_it), MECS_COMPONENT_IDENT(i_type), i_index))

mecs_query_it_t         mecs_query_create();
void                    mecs_query_with_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component);
void                    mecs_query_without_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component);
void                    mecs_query_optional_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component);
void                    mecs_query_begin(mecs_registry_t* io_registry, mecs_query_it_t* io_query_it);
mecs_bool_t             mecs_query_next(mecs_query_it_t* io_query_it);
mecs_entity_t           mecs_query_entity_get(mecs_query_it_t* io_query_it);
mecs_bool_t             mecs_query_component_has_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component, size_t i_index);
void*                   mecs_query_component_get_impl(mecs_query_it_t* io_query_it, mecs_component_t i_component, size_t i_index);



#ifdef MECS_IMPLEMENTATION

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

mecs_registry_t* mecs_registry_create(size_t i_component_count_reserve) 
{
    mecs_registry_t* registry;
    registry = mecs_calloc_type(mecs_registry_t);
    if (registry == NULL)
    {
        return NULL;
    }

    /* Reserve space for the components we will register. We allow growing beyond this memory but will always try to use as little memory as possible. 
       This is because registring new components should be an infrequent operation, likely only taking place during init. */
    registry->components_len = 0;
    registry->components_cap = i_component_count_reserve;
    if (registry->components_cap != 0)
    {
        registry->components = mecs_calloc_arr(registry->components_cap, mecs_component_store_t);
        if (registry->components == NULL)
        {
            mecs_free(registry);
            mecs_assert(MECS_FALSE);
            return NULL;
        }
    }

    /* Reserve space for entities to prevent frequent growing of the array when the first entities get added. */
    registry->next_free_entity = 0;
    registry->entities_len = 0;
    registry->entities_cap = 8; 
    registry->entities = mecs_calloc_arr(registry->entities_cap, mecs_entity_t);
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
    mecs_assert(io_registry != NULL);

    for (i = 0; i < io_registry->components_len; ++i)
    {
        /* TODO: Deallocate sparse set. */
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

mecs_component_t mecs_component_register_impl(mecs_registry_t* io_registry, char const* name, size_t size, size_t alignment) 
{
    mecs_component_store_t* components_grown;
    size_t new_capacity;
    mecs_component_t component;
    mecs_assert(io_registry != NULL);

    if (io_registry->components_len >= io_registry->components_cap)
    {
        /* Grow array by 1 to guarantee minimal memory usage. */
        new_capacity = io_registry->components_cap + 1;
        components_grown = mecs_realloc_arr(io_registry->components, new_capacity, mecs_component_store_t);
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

    io_registry->components[component].name = name;
    io_registry->components[component].size = size;
    io_registry->components[component].alignment = alignment;

    io_registry->components[component].sparse = NULL;
    io_registry->components[component].sparse_len = 0;
    io_registry->components[component].dense = NULL;
    io_registry->components[component].entities_count = 0;
    io_registry->components[component].components = NULL;
    io_registry->components[component].components_len = 0;
    io_registry->components_len += 1;
    return component;
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

        memcpy(entity_component_elem, last_entity_component_elem, component_store->size);
        *entity_dense_elem = *last_entity_dense_elem;
        *last_entity_sparse_elem = mecs_entity_compose(mecs_entity_get_generation(*last_entity_sparse_elem), entity_dense_index); /* Override the new dense index of the last entity. */
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
        sparse_grown = mecs_realloc_arr(i_component_store->sparse, page_index + 1, mecs_sparse_block_t*);
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
        dense_grown = mecs_realloc_arr(i_component_store->dense, dense_grown_size, mecs_dense_t);
        if (dense_grown == NULL)
        {
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        memset(dense_grown + dense_grown_offset, 0xFF, MECS_PAGE_LEN_DENSE * sizeof(mecs_dense_t)); /* Initialise all entires to invalid entity. */
        i_component_store->dense = dense_grown;

        /* Allocate a new component page. */
        components_page = mecs_calloc(MECS_PAGE_LEN_DENSE, i_component_store->size);
        if (components_page == NULL)
        {
            mecs_assert(MECS_FALSE);
            return NULL;
        }
        
        /* Grow the array of component pages so we can hold the new page. */
        components_grown = mecs_realloc_arr(i_component_store->components, i_component_store->components_len + 1, void*);
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

            entities_grown = mecs_realloc_arr(io_registry->entities, new_capacity, mecs_entity_t);
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
    size_t i;
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

mecs_query_it_t mecs_query_create()
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
#endif /* MECS_H */
