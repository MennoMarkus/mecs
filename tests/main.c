#define MECS_IMPLEMENTATION 
#include "../mecs.h"

#include <stdint.h>
#include <stdio.h>

#define STRINGIFY(v) #v
#define test(i_condition) if (!(i_condition)) { printf("Test line %d failed: %s\n", __LINE__, STRINGIFY(i_condition)); assert(0); }
#define test_str(i_value, i_expected) if (strcmp((i_value), (i_expected)) == 1) { printf("Test line %d failed. Expected: %s, Got: %s\n", __LINE__, (i_expected), (i_value)); assert(0); }
#define test_uint(i_value, i_expected) if ((i_value) != (i_expected)) { printf("Test line %d failed. Expected: %zu, Got: %zu\n", __LINE__, (size_t)(i_expected), (size_t)(i_value)); assert(0); }

typedef struct 
{
   uint32_t v; 
} test_comp_4;

typedef struct 
{
   uint64_t v; 
} test_comp_8;

#if defined(__cplusplus)
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

    uint64_t v;
};
#endif

MECS_COMPONENT_DECLARE(test_comp_4);
MECS_COMPONENT_DECLARE(test_comp_8);
#if defined(__cplusplus)
MECS_COMPONENT_DECLARE(test_comp_cpp);
#endif

void test_registry_create(void) 
{
    mecs_registry_t* registry;

    registry = mecs_registry_create(4);
    MECS_COMPONENT_REGISTER(registry, test_comp_4);
    MECS_COMPONENT_REGISTER(registry, test_comp_8);

    test_uint(registry->components_len, 2);
    test_uint(registry->components_cap, 4);

    test_str(registry->components[0].name, "test_comp_4");
    test_uint(registry->components[0].size, 4);
    test_uint(registry->components[0].alignment, 4);

    test_str(registry->components[1].name, "test_comp_8");
    test_uint(registry->components[1].size, 8);
    test_uint(registry->components[1].alignment, 8);

    mecs_registry_destroy(registry);
    
    registry = mecs_registry_create(0);
    MECS_COMPONENT_REGISTER(registry, test_comp_4);
    MECS_COMPONENT_REGISTER(registry, test_comp_8);

    test_uint(registry->components_len, 2);
    test_uint(registry->components_cap, 2);

    test_str(registry->components[0].name, "test_comp_4");
    test_uint(registry->components[0].size, 4);
    test_uint(registry->components[0].alignment, 4);

    test_str(registry->components[1].name, "test_comp_8");
    test_uint(registry->components[1].size, 8);
    test_uint(registry->components[1].alignment, 8);

    mecs_registry_destroy(registry);
}

void test_entity_recycle(void) 
{
    mecs_registry_t* registry;
    mecs_entity_t entity0, entity1, entity2, entity3; 

    registry = mecs_registry_create(0);
    entity0 = mecs_entity_create(registry);
    entity1 = mecs_entity_create(registry);
    entity2 = mecs_entity_create(registry);
    entity3 = mecs_entity_create(registry);
    (void)entity3;

    mecs_entity_destroy(registry, entity2);
    mecs_entity_destroy(registry, entity1);
    mecs_entity_destroy(registry, entity0);
    entity0 = mecs_entity_create(registry);
    entity1 = mecs_entity_create(registry);
    entity2 = mecs_entity_create(registry);

    mecs_entity_destroy(registry, entity1);
    mecs_entity_destroy(registry, entity0);
    entity0 = mecs_entity_create(registry);
    entity1 = mecs_entity_create(registry);

    mecs_entity_destroy(registry, entity0);
    entity0 = mecs_entity_create(registry);

    test_uint(registry->entities_len, 4);
    test_uint(registry->entities_cap, 8);
    test_uint(mecs_entity_get_id(registry->entities[0]), 0);
    test_uint(mecs_entity_get_id(registry->entities[1]), 1);
    test_uint(mecs_entity_get_id(registry->entities[2]), 2);
    test_uint(mecs_entity_get_id(registry->entities[3]), 3);
    test_uint(mecs_entity_get_generation(registry->entities[0]), 3);
    test_uint(mecs_entity_get_generation(registry->entities[1]), 2);
    test_uint(mecs_entity_get_generation(registry->entities[2]), 1);
    test_uint(mecs_entity_get_generation(registry->entities[3]), 0);
    
    mecs_registry_destroy(registry);
}

void test_add_component(void)
{
    mecs_registry_t* registry;

    registry = mecs_registry_create(2);
    MECS_COMPONENT_REGISTER(registry, test_comp_4);
    MECS_COMPONENT_REGISTER(registry, test_comp_8);

    /* TODO: test */

    mecs_registry_destroy(registry);
}

void test_has_component(void)
{
    mecs_registry_t* registry;
    mecs_entity_t entity0, entity1; 

    registry = mecs_registry_create(2);
    MECS_COMPONENT_REGISTER(registry, test_comp_4);
    MECS_COMPONENT_REGISTER(registry, test_comp_8);

    entity0 = mecs_entity_create(registry);
    mecs_component_add(registry, entity0, test_comp_4);
    test_uint(mecs_component_has(registry, entity0, test_comp_4), MECS_TRUE);
    test_uint(mecs_component_has(registry, entity0, test_comp_8), MECS_FALSE);

    mecs_entity_destroy(registry, entity0);
    test_uint(mecs_component_has(registry, entity0, test_comp_4), MECS_FALSE);
    test_uint(mecs_component_has(registry, entity0, test_comp_8), MECS_FALSE);

    entity1 = mecs_entity_create(registry);
    mecs_component_add(registry, entity1, test_comp_4);
    test_uint(mecs_component_has(registry, entity0, test_comp_4), MECS_FALSE);
    test_uint(mecs_component_has(registry, entity0, test_comp_8), MECS_FALSE);
    test_uint(mecs_component_has(registry, entity1, test_comp_4), MECS_TRUE);
    test_uint(mecs_component_has(registry, entity1, test_comp_8), MECS_FALSE);

    mecs_registry_destroy(registry);
}

void test_query(void)
{
    mecs_registry_t* registry;
    mecs_entity_t entity0, entity1; 
    test_comp_4* comp4;
    test_comp_8* comp8;
    mecs_query_it_t query0, query1, query2, query3;
    size_t query0_count, query1_count, query2_count, query3_count;

    registry = mecs_registry_create(2);
    MECS_COMPONENT_REGISTER(registry, test_comp_4);
    MECS_COMPONENT_REGISTER(registry, test_comp_8);

    entity0 = mecs_entity_create(registry);
    comp4 = mecs_component_add(registry, entity0, test_comp_4);
    comp8 = mecs_component_add(registry, entity0, test_comp_8);
    comp4->v = 4;
    comp8->v = 8;

    entity1 = mecs_entity_create(registry);
    comp8 = mecs_component_add(registry, entity1, test_comp_8);
    comp8->v = 8;

    query0 = mecs_query_create();
    mecs_query_with(&query0, test_comp_4);
    mecs_query_with(&query0, test_comp_8);
    query0_count = 0;
    for(mecs_query_begin(registry, &query0); mecs_query_next(&query0);)
    {
        test_uint(mecs_query_entity_get(&query0), entity0);
        test_uint(mecs_query_component_has(&query0, test_comp_4, 0), MECS_TRUE);
        test_uint(mecs_query_component_has(&query0, test_comp_8, 1), MECS_TRUE);
        test_uint(mecs_query_component_get(&query0, test_comp_4, 0)->v, 4);
        test_uint(mecs_query_component_get(&query0, test_comp_8, 1)->v, 8);
        query0_count += 1;
    }
    test_uint(query0_count, 1);

    query1 = mecs_query_create();
    mecs_query_with(&query1, test_comp_8);
    query1_count = 0;
    for(mecs_query_begin(registry, &query1); mecs_query_next(&query1);)
    {
        test_uint(mecs_query_component_has(&query1, test_comp_8, 0), MECS_TRUE);
        test_uint(mecs_query_component_get(&query1, test_comp_8, 0)->v, 8);
        query1_count += 1;
    }
    test_uint(query1_count, 2);

    query2 = mecs_query_create();
    mecs_query_without(&query2, test_comp_4);
    mecs_query_with(&query2, test_comp_8);
    query2_count = 0;
    for(mecs_query_begin(registry, &query2); mecs_query_next(&query2);)
    {
        test_uint(mecs_query_entity_get(&query2), entity1);
        test_uint(mecs_query_component_has(&query2, test_comp_8, 1), MECS_TRUE);
        test_uint(mecs_query_component_get(&query2, test_comp_8, 1)->v, 8);
        query2_count += 1;
    }
    test_uint(query2_count, 1);

    query3 = mecs_query_create();
    mecs_query_optional(&query3, test_comp_4);
    mecs_query_with(&query3, test_comp_8);
    query3_count = 0;
    for(mecs_query_begin(registry, &query3); mecs_query_next(&query3);)
    {
        if(mecs_query_entity_get(&query3) == entity0) test_uint(mecs_query_component_has(&query3, test_comp_4, 0), MECS_TRUE);
        if(mecs_query_entity_get(&query3) == entity1) test_uint(mecs_query_component_has(&query3, test_comp_4, 0), MECS_FALSE);
        test_uint(mecs_query_component_has(&query3, test_comp_8, 1), MECS_TRUE);
        test_uint(mecs_query_component_get(&query3, test_comp_8, 1)->v, 8);
        query3_count += 1;
    }
    test_uint(query3_count, 2);

    mecs_registry_destroy(registry);
}

uint64_t g_test_destructor_count;

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
    mecs_registry_t* registry;
    mecs_entity_t entity0, entity1;
    test_comp_4* comp0;
    test_comp_4* comp1;
    
    registry = mecs_registry_create(1);
    MECS_COMPONENT_REGISTER(registry, test_comp_4);
    MECS_COMPONENT_REGISTER_CTOR(registry, test_comp_4, &init_test_comp4);
    MECS_COMPONENT_REGISTER_DTOR(registry, test_comp_4, &destroy_test_comp4);
    /* MECS_COMPONENT_REGISTER_MOVE_AND_DTOR(registry, test_comp_4, &overide_test_comp4); */

    g_test_destructor_count = 0;
    entity0 = mecs_entity_create(registry);
    entity1 = mecs_entity_create(registry);

    comp0 = mecs_component_add(registry, entity0, test_comp_4);
    comp1 = mecs_component_add(registry, entity1, test_comp_4);
    test_uint(comp0->v, 99);
    test_uint(comp1->v, 99);
    comp1->v = 88;

    mecs_entity_destroy(registry, entity0);
    test_uint(comp0->v, 88);
    test_uint(g_test_destructor_count, 1);

    mecs_entity_destroy(registry, entity1);
    test_uint(comp0->v, 11);
    test_uint(g_test_destructor_count, 2);

    mecs_registry_destroy(registry);
}

#if defined(__cplusplus)
void test_constructor_cpp(void) 
{
    mecs_registry_t* registry = mecs_registry_create(1);
    MECS_COMPONENT_REGISTER(registry, test_comp_cpp);

    mecs_entity_t entity0 = mecs_entity_create(registry);
    mecs_entity_t entity1 = mecs_entity_create(registry);

    test_comp_cpp* comp0 = mecs_component_add(registry, entity0, test_comp_cpp);
    test_comp_cpp* comp1 = mecs_component_add(registry, entity1, test_comp_cpp);
    test_uint(comp0->v, 99);
    test_uint(comp1->v, 99);
    comp1->v = 88;

    mecs_entity_destroy(registry, entity0);
    test_uint(comp0->v, 88);
    test_uint(comp1->v, 11);

    mecs_entity_destroy(registry, entity1);
    test_uint(comp0->v, 11);
    test_uint(comp1->v, 11);

    mecs_registry_destroy(registry);
}
#endif

/* TODO: Scratch test space. Remove... */

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int velocity_x;
    int velocity_y;
    int acceleration_x;
    int acceleration_y;
    size_t mass;
} Physics;

typedef struct {
    char r, g, b;
} Colour;

MECS_COMPONENT_DECLARE(Position);
MECS_COMPONENT_DECLARE(Physics);
MECS_COMPONENT_DECLARE(Colour);

int main(void) {
    mecs_registry_t* registry;
    size_t i;
    mecs_entity_t entity0, entity1, entity2, entity3; 
    Position* position0;
    Position* position2; 

    test_registry_create();
    test_entity_recycle();
    test_has_component();
    test_query();
    test_constructor_c();
    #if defined(__cplusplus)
        test_constructor_cpp();
    #endif

    registry = mecs_registry_create(1);
    MECS_COMPONENT_REGISTER(registry, Position);
    MECS_COMPONENT_REGISTER(registry, Physics);
    MECS_COMPONENT_REGISTER(registry, Colour);

    entity0 = mecs_entity_create(registry);
    entity1 = mecs_entity_create(registry);
    entity2 = mecs_entity_create(registry);
    entity3 = mecs_entity_create(registry);
    mecs_entity_destroy(registry, entity1);
    mecs_entity_destroy(registry, entity2);
    mecs_entity_destroy(registry, entity3);
    
    (void)entity0; 
    (void)entity1; 
    (void)entity2; 
    (void)entity3; 
    
    position0 = mecs_component_add(registry, entity0, Position);
    position2 = mecs_component_add(registry, entity2, Position);
    position0->x = 1;
    position0->y = 2;
    position2->x = 3;
    position2->y = 4;

    if (mecs_component_has(registry, entity0, Position))
    {
        position0 = mecs_component_get(registry, entity0, Position);
        position0->x = 10;
        position0->y = 20;
        mecs_component_remove(registry, entity0, Position);
    }
    if (mecs_component_has(registry, entity2, Position))
    {
        position2 = mecs_component_get(registry, entity2, Position);
        position2->x = 30;
        position2->y = 40;
        mecs_component_remove(registry, entity2, Position);
    }

    for (i = 0; i < registry->components_len; ++i)
    {
        printf("Size: %3zu, Alignment: %3zu, Name: %s\n", registry->components[i].size, registry->components[i].alignment, registry->components[i].name);
    }
    printf("\n");

    for (i = 0; i < registry->entities_len; ++i)
    {
        printf("Entity ID: %5u, Generation: %3u\n", mecs_entity_get_id(registry->entities[i]), mecs_entity_get_generation(registry->entities[i]));
    }
    printf("Next free entity: %u\n", mecs_entity_get_id(registry->next_free_entity));
    printf("\n");

    mecs_registry_destroy(registry);
    printf("Shutdown");
    return 0;
}
