#define STRINGIFY(v) #v
#define XSTRINGIFY(v) STRINGIFY(v)

#if defined(__cplusplus)
    #if __cplusplus == 202101L 
        #pragma message("Compiling using C++23")
    #elif __cplusplus == 202002L 
        #pragma message("Compiling using C++20")
    #elif __cplusplus == 201703L 
        #pragma message("Compiling using C++17")
    #elif __cplusplus == 201402L 
        #pragma message("Compiling using C++14")
    #elif __cplusplus == 201103L 
        #pragma message("Compiling using C++11")
    #elif __cplusplus == 199711L 
        #pragma message("Compiling using C++98")
    #else 
        #pragma message("Compiling using unkown C++ version " XSTRINGIFY(__cplusplus))
    #endif
#elif defined(__STDC__)
    #if __STDC_VERSION__ == 202311L 
        #pragma message("Compiling using C23")
    #elif __STDC_VERSION__ == 201710L
        #pragma message("Compiling using C17")
    #elif __STDC_VERSION__ == 201112L
        #pragma message("Compiling using C11")
    #elif __STDC_VERSION__ == 199901L
        #pragma message("Compiling using C99")
    #elif __STDC_VERSION__ == 199409L
        #pragma message("Compiling using C90/C95")
    #else
        #pragma message("Compiling using C89/ISO")
    #endif
#else
    #pragma message("Compiling using unkown target")
#endif

#include "../mecs.h"
#include "../mecs_serialisation.h"

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string.h>
#if defined(__cplusplus)
#include <cstdint>
#endif

/* Constexpr definition. Prevents warnings for constant expression created by macros. */
#if defined(__cplusplus) && _cplusplus >= 201103L /* C++11 */
    #define MECS_CONST_EXPR_IF if constexpr
#elif defined(_MSC_VER)
    #define MECS_CONST_EXPR_IF __pragma(warning(suppress: 4127)) if
#else
    #define MECS_CONST_EXPR_IF if 
#endif

#define test(i_condition)                                                                           \
    MECS_CONST_EXPR_IF (!(i_condition))                                                             \
    {                                                                                               \
        printf("Test line %d failed: %s\n", __LINE__, STRINGIFY(i_condition));                      \
        assert(0);                                                                                  \
    }                                                                                               \

#define test_str(i_value, i_expected)                                                               \
    MECS_CONST_EXPR_IF (strcmp((i_value), (i_expected)) == 1)                                       \
    {                                                                                               \
        printf("Test line %d failed. Expected: %s, Got: %s\n", __LINE__, (i_expected), (i_value));  \
        assert(0);                                                                                  \
    }                                                                                               \

#define test_uint(i_value, i_expected)                                                              \
    MECS_CONST_EXPR_IF ((i_value) != (i_expected))                                                  \
    {                                                                                               \
        mecs_size_t value = (mecs_size_t)(i_value);                                                           \
        mecs_size_t expected = (mecs_size_t)(i_expected);                                                     \
        printf("Test line %d failed. Expected: %zu, Got: %zu\n", __LINE__, expected, value);        \
        assert(0);                                                                                  \
    }                                                                                               \

typedef struct 
{
    mecs_size_t address;
    mecs_size_t size;
    char freed;
} allocation_t;

#define MAX_ALLOCATIONS 1024
allocation_t g_memory_leak_allocations[MAX_ALLOCATIONS];
mecs_size_t g_memory_leak_total_allocations_made;
mecs_size_t g_memory_leak_total_allocated;
mecs_size_t g_memory_leak_total_freed;

void memory_leak_detector_init(void) 
{ 
    mecs_size_t i;
    for (i = 0; i < MAX_ALLOCATIONS; ++i)
    {
        g_memory_leak_allocations[i].address = 0;
        g_memory_leak_allocations[i].size = 0;
        g_memory_leak_allocations[i].freed = 1;
    }
    g_memory_leak_total_allocations_made = 0; 
    g_memory_leak_total_allocated = 0; 
    g_memory_leak_total_freed = 0;
}

void memory_leak_detector_shutdown(void) 
{ 
    mecs_size_t i;
    printf("--------------------------------------------------\n");
    for (i = 0; i < MAX_ALLOCATIONS; ++i)
    {
        if (g_memory_leak_allocations[i].address == 0)
        {
            break;
        }
        if (g_memory_leak_allocations[i].freed == 0)
        {
            printf("Memory leak detected. Address: %zx, Size %zu\n", g_memory_leak_allocations[i].address, g_memory_leak_allocations[i].size);
        }
    }

    printf("--------------------------------------------------\n");
    printf("Total allocations made: %zu.\n", g_memory_leak_total_allocations_made);
    printf("Total memory allocted:  %zu bytes.\n", g_memory_leak_total_allocated);
    printf("Total memory freed:     %zu bytes.\n", g_memory_leak_total_freed);
    if (g_memory_leak_total_allocated != g_memory_leak_total_freed) 
    { 
        printf("Total memory leaked:    %zu bytes.\n", g_memory_leak_total_allocated - g_memory_leak_total_freed);
        assert(0);
    }
    printf("--------------------------------------------------\n");
}

void* memory_leak_detector_realloc(void* i_ptr, mecs_size_t i_size) 
{ 
    mecs_size_t i;
    g_memory_leak_total_allocated += i_size; 

    if (i_ptr != NULL)
    {
        for (i = 0; i < MAX_ALLOCATIONS; ++i)
        {
            if (g_memory_leak_allocations[i].address == (mecs_size_t)i_ptr)
            {
                if (g_memory_leak_allocations[i].freed == 1)
                {
                    printf("Detected realloc of a freed address. Address: %zx, Size %zu\n", g_memory_leak_allocations[i].address, g_memory_leak_allocations[i].size);
                    assert(0);
                }
                g_memory_leak_allocations[i].freed = 1;
                g_memory_leak_total_freed += g_memory_leak_allocations[i].size; 
                break;
            }
        }
    }

    i_ptr = realloc(i_ptr, i_size);
    for (i = 0; i < MAX_ALLOCATIONS; ++i)
    {
        if (g_memory_leak_allocations[i].address == (mecs_size_t)i_ptr || g_memory_leak_allocations[i].address == 0)
        {
            g_memory_leak_allocations[i].address = (mecs_size_t)i_ptr;
            g_memory_leak_allocations[i].size = i_size;
            g_memory_leak_allocations[i].freed = 0;
            g_memory_leak_total_allocations_made += 1;
            return i_ptr;
        }
    }
    printf("Max allocations reached.\n");
    assert(0);
    return NULL;
}

void memory_leak_detector_free(void* i_ptr) 
{ 
    mecs_size_t i;
    for (i = 0; i < MAX_ALLOCATIONS; ++i)
    {
        if (g_memory_leak_allocations[i].address == (mecs_size_t)i_ptr)
        {
            if (g_memory_leak_allocations[i].freed == 1)
            {
                printf("Detected double free. Address: %zx, Size %zu\n", g_memory_leak_allocations[i].address, g_memory_leak_allocations[i].size);
                assert(0);
            }
            g_memory_leak_allocations[i].freed = 1;
            g_memory_leak_total_freed += g_memory_leak_allocations[i].size; 
            free(i_ptr); 
            return;
        }
    }
    printf("Allocation not found. Address: %zx\n", (mecs_size_t)i_ptr);
    assert(0);
}

typedef struct 
{
    mecs_uint32_t v; 
} test_comp_4;

typedef struct 
{
   mecs_uint64_t v; 
} test_comp_8;

typedef struct 
{
    mecs_uint32_t n; 
} test_comp_serialise_nested;

ARCHIVE(test_comp_serialise_nested, MECS_FALSE)
{
    mecs_uint32_t l = 0;
    archive_add(mecs_uint32_t, n, 0);
    archive_add_local(mecs_uint32_t, l, 0);
}

typedef struct 
{
    mecs_uint32_t v1; 
    mecs_uint32_t v2; 
    mecs_uint32_t v3; 
    test_comp_serialise_nested v4;
} test_comp_serialise;

ARCHIVE(test_comp_serialise, MECS_TRUE)
{
    archive_add(mecs_uint32_t, v1, 0);
    archive_add(mecs_uint32_t, v2, 0);
    archive_add(mecs_uint32_t, v3, 0);
    archive_add(test_comp_serialise_nested, v4, 0);
}

#if defined(__cplusplus)
namespace cpp
{

class test_comp_cpp 
{
public:
    test_comp_cpp() : 
        v(99) 
    {}

    ~test_comp_cpp() 
    { 
        v = 11;
    }

    std::uint64_t v;
};

class test_comp_cpp_inner_scope
{
    std::uint32_t v;
};

class test_comp_serialise_cpp 
{
public:
    std::uint32_t v;
};

ARCHIVE(test_comp_serialise_cpp, MECS_TRUE)
{
    archive_add(std::uint32_t, v, 0);
}

}
#endif

COMPONENT_DECLARE(test_comp_serialise);
#if defined(__cplusplus)
COMPONENT_DECLARE(cpp::test_comp_serialise_cpp);
#endif


COMPONENT_DECLARE(test_comp_4);
COMPONENT_DECLARE(test_comp_8);
#if defined(__cplusplus)
COMPONENT_DECLARE(cpp::test_comp_cpp);
namespace cpp { COMPONENT_DECLARE(test_comp_cpp_inner_scope); }
#endif

void test_registry_create(void) 
{
    registry_t* registry;

    registry = registry_create(4);
    COMPONENT_REGISTER(registry, test_comp_4);
    COMPONENT_REGISTER(registry, test_comp_8);

    test_uint(registry->components_len, 4);

    test_str(registry->components[0].type->name, "test_comp_4");
    test_uint(registry->components[0].type->size, 4);
    test_uint(registry->components[0].type->alignment, 4);

    test_str(registry->components[1].type->name, "test_comp_8");
    test_uint(registry->components[1].type->size, 8);
    test_uint(registry->components[1].type->alignment, 8);

    registry_destroy(registry);
    
    registry = registry_create(0);
    COMPONENT_REGISTER(registry, test_comp_4);
    COMPONENT_REGISTER(registry, test_comp_8);

    test_uint(registry->components_len, 2);

    test_str(registry->components[0].type->name, "test_comp_4");
    test_uint(registry->components[0].type->size, 4);
    test_uint(registry->components[0].type->alignment, 4);

    test_str(registry->components[1].type->name, "test_comp_8");
    test_uint(registry->components[1].type->size, 8);
    test_uint(registry->components[1].type->alignment, 8);

    registry_destroy(registry);
}

void test_entity_recycle(void) 
{
    registry_t* registry;
    entity_t entity0, entity1, entity2, entity3; 

    registry = registry_create(0);
    entity0 = entity_create(registry);
    entity1 = entity_create(registry);
    entity2 = entity_create(registry);
    entity3 = entity_create(registry);
    (void)entity3;

    entity_destroy(registry, entity2);
    entity_destroy(registry, entity1);
    entity_destroy(registry, entity0);
    entity0 = entity_create(registry);
    entity1 = entity_create(registry);
    entity2 = entity_create(registry);

    entity_destroy(registry, entity1);
    entity_destroy(registry, entity0);
    entity0 = entity_create(registry);
    entity1 = entity_create(registry);

    entity_destroy(registry, entity0);
    entity0 = entity_create(registry);

    test_uint(registry->entities_len, 4);
    test_uint(registry->entities_cap, 8);
    test_uint(entity_get_id(registry->entities[0]), 0);
    test_uint(entity_get_id(registry->entities[1]), 1);
    test_uint(entity_get_id(registry->entities[2]), 2);
    test_uint(entity_get_id(registry->entities[3]), 3);
    test_uint(entity_get_generation(registry->entities[0]), 3);
    test_uint(entity_get_generation(registry->entities[1]), 2);
    test_uint(entity_get_generation(registry->entities[2]), 1);
    test_uint(entity_get_generation(registry->entities[3]), 0);
    
    registry_destroy(registry);
}

void test_add_component(void)
{
    registry_t* registry;

    registry = registry_create(2);
    COMPONENT_REGISTER(registry, test_comp_4);
    COMPONENT_REGISTER(registry, test_comp_8);

    /* TODO: test */

    registry_destroy(registry);
}

void test_has_component(void)
{
    registry_t* registry;
    entity_t entity0, entity1; 

    registry = registry_create(2);
    COMPONENT_REGISTER(registry, test_comp_4);
    COMPONENT_REGISTER(registry, test_comp_8);

    entity0 = entity_create(registry);
    component_add(registry, entity0, test_comp_4);
    test_uint(component_has(registry, entity0, test_comp_4), MECS_TRUE);
    test_uint(component_has(registry, entity0, test_comp_8), MECS_FALSE);

    entity_destroy(registry, entity0);
    test_uint(component_has(registry, entity0, test_comp_4), MECS_FALSE);
    test_uint(component_has(registry, entity0, test_comp_8), MECS_FALSE);

    entity1 = entity_create(registry);
    component_add(registry, entity1, test_comp_4);
    test_uint(component_has(registry, entity0, test_comp_4), MECS_FALSE);
    test_uint(component_has(registry, entity0, test_comp_8), MECS_FALSE);
    test_uint(component_has(registry, entity1, test_comp_4), MECS_TRUE);
    test_uint(component_has(registry, entity1, test_comp_8), MECS_FALSE);

    registry_destroy(registry);
}

void test_query(void)
{
    registry_t* registry;
    entity_t entity0, entity1; 
    test_comp_4* comp4;
    test_comp_8* comp8;
    query_it_t query0, query1, query2, query3;
    mecs_size_t query0_count, query1_count, query2_count, query3_count;

    registry = registry_create(2);
    COMPONENT_REGISTER(registry, test_comp_4);
    COMPONENT_REGISTER(registry, test_comp_8);

    entity0 = entity_create(registry);
    comp4 = component_add(registry, entity0, test_comp_4);
    comp8 = component_add(registry, entity0, test_comp_8);
    comp4->v = 4;
    comp8->v = 8;

    entity1 = entity_create(registry);
    comp8 = component_add(registry, entity1, test_comp_8);
    comp8->v = 8;

    query0 = query_create();
    query_with(&query0, test_comp_4);
    query_with(&query0, test_comp_8);
    query0_count = 0;
    for(query_begin(registry, &query0); query_next(&query0);)
    {
        test_uint(query_entity_get(&query0), entity0);
        test_uint(query_component_has(&query0, test_comp_4, 0), MECS_TRUE);
        test_uint(query_component_has(&query0, test_comp_8, 1), MECS_TRUE);
        test_uint(query_component_get(&query0, test_comp_4, 0)->v, 4);
        test_uint(query_component_get(&query0, test_comp_8, 1)->v, 8);
        query0_count += 1;
    }
    test_uint(query0_count, 1);

    query1 = query_create();
    query_with(&query1, test_comp_8);
    query1_count = 0;
    for(query_begin(registry, &query1); query_next(&query1);)
    {
        test_uint(query_component_has(&query1, test_comp_8, 0), MECS_TRUE);
        test_uint(query_component_get(&query1, test_comp_8, 0)->v, 8);
        query1_count += 1;
    }
    test_uint(query1_count, 2);

    query2 = query_create();
    query_without(&query2, test_comp_4);
    query_with(&query2, test_comp_8);
    query2_count = 0;
    for(query_begin(registry, &query2); query_next(&query2);)
    {
        test_uint(query_entity_get(&query2), entity1);
        test_uint(query_component_has(&query2, test_comp_8, 1), MECS_TRUE);
        test_uint(query_component_get(&query2, test_comp_8, 1)->v, 8);
        query2_count += 1;
    }
    test_uint(query2_count, 1);

    query3 = query_create();
    query_optional(&query3, test_comp_4);
    query_with(&query3, test_comp_8);
    query3_count = 0;
    for(query_begin(registry, &query3); query_next(&query3);)
    {
        if(query_entity_get(&query3) == entity0) test_uint(query_component_has(&query3, test_comp_4, 0), MECS_TRUE);
        if(query_entity_get(&query3) == entity1) test_uint(query_component_has(&query3, test_comp_4, 0), MECS_FALSE);
        test_uint(query_component_has(&query3, test_comp_8, 1), MECS_TRUE);
        test_uint(query_component_get(&query3, test_comp_8, 1)->v, 8);
        query3_count += 1;
    }
    test_uint(query3_count, 2);

    registry_destroy(registry);
}

mecs_uint32_t g_test_destructor_count;

void init_test_comp4(void* io_comp)
{
    ((test_comp_4*)io_comp)->v = 99;
}

void destroy_test_comp4(void* io_comp)
{
    ((test_comp_4*)io_comp)->v = 11;
    g_test_destructor_count += 1;
}

void test_constructor_c(void)
{
    registry_t* registry;
    entity_t entity0, entity1;
    test_comp_4* comp0;
    test_comp_4* comp1;
    
    registry = registry_create(1);
    COMPONENT_REGISTER_LIFE_TIME_HOOKS(test_comp_4, &init_test_comp4, &destroy_test_comp4, NULL);
    COMPONENT_REGISTER(registry, test_comp_4);

    g_test_destructor_count = 0;
    entity0 = entity_create(registry);
    entity1 = entity_create(registry);

    comp0 = component_add(registry, entity0, test_comp_4);
    comp1 = component_add(registry, entity1, test_comp_4);
    test_uint(comp0->v, 99);
    test_uint(comp1->v, 99);
    comp1->v = 88;

    entity_destroy(registry, entity0);
    test_uint(comp0->v, 88);
    test_uint(g_test_destructor_count, 1);

    entity_destroy(registry, entity1);
    test_uint(comp0->v, 11);
    test_uint(g_test_destructor_count, 2);

    registry_destroy(registry);
}

#if defined(__cplusplus)
void test_constructor_cpp(void) 
{
    registry_t* registry = registry_create(1);
    COMPONENT_REGISTER(registry, cpp::test_comp_cpp);

    entity_t entity0 = entity_create(registry);
    entity_t entity1 = entity_create(registry);

    cpp::test_comp_cpp* comp0 = component_add(registry, entity0, cpp::test_comp_cpp);
    cpp::test_comp_cpp* comp1 = component_add(registry, entity1, cpp::test_comp_cpp);
    test_uint(comp0->v, 99);
    test_uint(comp1->v, 99);
    comp1->v = 88;

    entity_destroy(registry, entity0);
    test_uint(comp0->v, 88);
    test_uint(comp1->v, 11);

    entity_destroy(registry, entity1);
    test_uint(comp0->v, 11);
    test_uint(comp1->v, 11);

    using namespace cpp;
    COMPONENT_REGISTER(registry, test_comp_cpp_inner_scope);
    component_add(registry, entity0, test_comp_cpp_inner_scope);
    component_add(registry, entity1, test_comp_cpp_inner_scope);

    registry_destroy(registry);
}
#endif

void test_serialise(void)
{
    registry_t* registry0;
    registry_t* registry1;
    entity_t entity0;
    entity_t entity1;
    test_comp_serialise* comp0;
    test_comp_serialise* comp1;
    #if defined(__cplusplus)
    cpp::test_comp_serialise_cpp* comp2;
    #endif
    void* buffer;
    mecs_size_t buffer_size;

    /* Serialisation */
    {
        registry0 = registry_create(2);
        COMPONENT_REGISTER_SERIALISATION_HOOKS(test_comp_serialise);
        COMPONENT_REGISTER(registry0, test_comp_serialise);
        #if defined(__cplusplus)
        COMPONENT_REGISTER_SERIALISATION_HOOKS(cpp::test_comp_serialise_cpp);
        COMPONENT_REGISTER(registry0, cpp::test_comp_serialise_cpp);
        #endif

        entity0 = entity_create(registry0);
        comp0 = component_add(registry0, entity0, test_comp_serialise);
        comp0->v1 = 1;
        comp0->v2 = 2;
        comp0->v3 = 3;
        comp0->v4.n = 4;

        entity1 = entity_create(registry0);
        comp1 = component_add(registry0, entity1, test_comp_serialise);
        comp1->v1 = 5;
        comp1->v2 = 6;
        comp1->v3 = 7;
        comp1->v4.n = 8;

        #if defined(__cplusplus)
        comp2 = component_add(registry0, entity1, cpp::test_comp_serialise_cpp);
        comp2->v = 9;
        #endif

        serialise_registry_binary(registry0, &buffer, &buffer_size);
    }

    /* Deserialisation */
    {
        registry1 = registry_create(2);
        COMPONENT_REGISTER_SERIALISATION_HOOKS(test_comp_serialise);
        COMPONENT_REGISTER(registry1, test_comp_serialise);
        #if defined(__cplusplus)
        COMPONENT_REGISTER_SERIALISATION_HOOKS(cpp::test_comp_serialise_cpp);
        COMPONENT_REGISTER(registry1, cpp::test_comp_serialise_cpp);
        #endif

        deserialise_registry_binary(registry1, buffer, buffer_size);

        test(serialisation_is_trivial(test_comp_serialise));
        test(!serialisation_is_trivial(test_comp_serialise_nested));
        #if defined(__cplusplus)
        test(serialisation_is_trivial(cpp::test_comp_serialise_cpp));
        #endif

        test_uint(registry1->entities_len, 2);
        test_uint(entity_get_id(registry1->entities[0]), 0);
        test_uint(entity_get_id(registry1->entities[1]), 1);
        test_uint(entity_get_generation(registry1->entities[0]), 0);
        test_uint(entity_get_generation(registry1->entities[1]), 0);
        test_uint(component_has(registry1, entity0, test_comp_serialise), MECS_TRUE);
        test_uint(component_has(registry1, entity1, test_comp_serialise), MECS_TRUE);

        test_uint(component_get(registry1, entity0, test_comp_serialise)->v1, 1);
        test_uint(component_get(registry1, entity0, test_comp_serialise)->v2, 2);
        test_uint(component_get(registry1, entity0, test_comp_serialise)->v3, 3);
        test_uint(component_get(registry1, entity0, test_comp_serialise)->v4.n, 4);
        test_uint(component_get(registry1, entity1, test_comp_serialise)->v1, 5);
        test_uint(component_get(registry1, entity1, test_comp_serialise)->v2, 6);
        test_uint(component_get(registry1, entity1, test_comp_serialise)->v3, 7);
        test_uint(component_get(registry1, entity1, test_comp_serialise)->v4.n, 8);

        #if defined(__cplusplus)
        test_uint(component_has(registry1, entity1, cpp::test_comp_serialise_cpp), MECS_TRUE);
        test_uint(component_get(registry1, entity1, cpp::test_comp_serialise_cpp)->v, 9);
        #endif
    }

    memory_leak_detector_free(buffer);
    registry_destroy(registry0);
    registry_destroy(registry1);
}

int main(void) 
{
    memory_leak_detector_init();
    {
        test_registry_create();
        test_entity_recycle();
        test_has_component();
        test_query();
        test_constructor_c();
        #if defined(__cplusplus)
        test_constructor_cpp();
        #endif
        test_serialise();
    }
    memory_leak_detector_shutdown();
    printf("Shutdown");
    return 0;
}

#define mecs_realloc(io_data, i_size) memory_leak_detector_realloc((io_data), (i_size))
#define mecs_free(io_data) memory_leak_detector_free(io_data)
#define MECS_IMPLEMENTATION 
#include "../mecs.h"
#include "../mecs_serialisation.h"