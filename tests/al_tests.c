#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <stdio.h>
#include <stdbool.h>
#include "arraylist.h"
#include "foreach.h"

/******************************************
 *                  ITEMS                 *
 ******************************************/
arraylist_t *global_list;
arraylist_t *list;
arraylist_t *deletelist;

typedef struct {
    char* name;
    int32_t id;
    double gpa;
}student_t;

typedef struct{
    int i;
    float f;
    long double ld;
    char c1:4;
    char c2:4;
    short s;
    void *some_data;
}test_item_t;

/******************************************
 *              HELPER FUNCS              *
 ******************************************/
void test_item_t_free_func(void *argptr){
    test_item_t* ptr = (test_item_t*) argptr;

    if(ptr)
        free(ptr->some_data);
    else
        cr_log_warn("%s\n", "Pointer was NULL");
}

void setup(void) {
    cr_log_warn("Setting up test");
    deletelist = new_al(sizeof(test_item_t));
}

void teardown(void) {
    //printf("teardown function\n");
    cr_log_error("Tearing down");
    delete_al(deletelist, test_item_t_free_func);
}

/******************************************
 *                  FOR-EACH TESTS                 *
 ******************************************/

arraylist_t *global_list9;
arraylist_t *global_list8;

void* multi_thread_for_each_5_helper(void* ptr)
{
    foreach(student_t,value,global_list9)
    {
        //printf("value->name: %s\n", value->name);
        value->id = 1000;
    }
    return NULL;
}

void* multi_thread_for_each_6_helper(void* ptr)
{
    foreach(student_t,value,global_list8)
    {
        //printf("value->name: %s\n", value->name);
        value->id = 1000;
    }
    return NULL;
}

Test(al_suite, multithread_test_5, .timeout = 5)
{
    global_list9 = new_al(sizeof(student_t));

    student_t *s = calloc(1,sizeof(student_t));
    s->name = "a";
    insert_al(global_list9,s);
    s->name = "b";
    insert_al(global_list9,s);
    s->name = "c";
    insert_al(global_list9,s);
    s->name = "d";
    insert_al(global_list9,s);

    pthread_t threads[100];
    int i;

    for(i = 0  ; i < 100 ; i++)
    {
        int* y = malloc(sizeof(int));
        *y = i;
        pthread_create(&threads[i],NULL,multi_thread_for_each_5_helper,(void*)y);
    }

    for(int i = 0 ; i < 100 ; i++)
    {
        pthread_join(threads[i],NULL);
    }

    // void* p = calloc(1,global_list9->item_size);
    // for(int i = 0 ; i < global_list9->length; i++)
    // {
    //     //student_t *test = calloc(1,sizeof(student_t));
    //     p = get_index_al(global_list9,i);
    //     test = p;
    //     //printf("result: %d, i: %d, name: %s \n", (int)test->id, i, test->name);
    //     //cr_assert(test->id == 1000, "result: %d", (int)test->id);
    // }
}

void* test_inserting(void *index)
{
    student_t *s = calloc(1,sizeof(student_t));
    s->name = "new";
    //printf("@@@@ insert!!\n");
    insert_al(global_list8,s);
    return NULL;
}

// Test(al_suite, multithread_foreach_test_6, .timeout = 5)
// {
//     global_list8 = new_al(sizeof(student_t));

//     student_t *s = calloc(1,sizeof(student_t));

//     pthread_t threads[100];
//     int i;

//     for(i = 0  ; i < 500 ; i++)
//     {
//         s->name = "a";
//         insert_al(global_list8,s);
//         s->name = "b";
//         insert_al(global_list8,s);
//     }

//     int* y ;
//     for(i = 0  ; i < 50 ; i+=2)
//     {
//         y = malloc(sizeof(int));
//         *y = i;
//         pthread_create(&threads[i],NULL,test_inserting,(void*)y);
//         pthread_create(&threads[i+1],NULL,multi_thread_for_each_6_helper,(void*)y);

//     }

//     for(int i = 0 ; i < 100 ; i++)
//     {
//         pthread_join(threads[i],NULL);
//     }

//     void* p = calloc(1,global_list8->item_size);

//     printf("length: %d \n", (int)global_list8->length);

//     for(int i = 0 ; i < global_list8->length; i++)
//     {
//         student_t *test = calloc(1,sizeof(student_t));
//         p = get_index_al(global_list8,i);
//         test = p;
//         printf("result: %d, i: %d, name: %s \n", (int)test->id, i, test->name);
//         //cr_assert(test->id == 1000, "result: %d", (int)test->id);
//     }
// }

int numbers(void* data){
    ((item_t*)data)->num = 999;
    return 0;
}

item_t* child1; item_t* child2; item_t* child3; item_t* child4; item_t* child5;
item_t* child6;

void* test_whileFE_inserting(void *index)
{
    insert_al(list,child5);
    insert_al(list,child6);
    insert_al(list,child5);
    insert_al(list,child6);
    return NULL;
}

void* test_runFE(void *index)
{
    apply(list, numbers);
    return NULL;
}

Test(al_suite, deletefl, .timeout=2, .init=setup, .fini=teardown){

    test_item_t *t = calloc(1, sizeof(test_item_t));
    t->some_data = calloc(1, sizeof(char));

    insert_al(deletelist, t);
}

Test(al_suite, deletefl2, .timeout=2, .init=setup, .fini=teardown){

    test_item_t *t = calloc(1, sizeof(test_item_t));
    //t->some_data = calloc(1, sizeof(char));

    insert_al(deletelist, t);
}


Test(al_suite, foreach_updateValue, .timeout=2){
    list = new_al(sizeof(item_t));

    pthread_t threads[2];

    child1 = malloc(sizeof(item_t));
    child2 = malloc(sizeof(item_t));
    child3 = malloc(sizeof(item_t));
    child4 = malloc(sizeof(item_t));
    child5 = malloc(sizeof(item_t));
    child6 = malloc(sizeof(item_t));
    child1->name = "child1";
    child2->name = "child2";
    child3->name = "child3";
    child4->name = "child4";
    child5->name = "child5";
    child6->name = "child6";
    child1->num = 99;
    child2->num = 98;
    child3->num = 97;
    child4->num = 96;
    child5->num = 95;
    child6->num = 94;

    insert_al(list,child1);
    insert_al(list,child2);
    insert_al(list,child3);
    insert_al(list,child4);
    //insert_al(list,child5);

    int* y = malloc(sizeof(int));
    pthread_create(&threads[0],NULL,test_runFE,(void*)y);
    pthread_create(&threads[1],NULL,test_whileFE_inserting,(void*)y);

    for(int i = 0 ; i < 2 ; i++)
    {
        pthread_join(threads[i],NULL);
    }

    item_t* childtemp = get_index_al(list, 0);
    cr_assert(childtemp->num == 999, "Error. The data is not updated.");

    // for(int i=0; i<list->length; i++){
    //     item_t* childtemp = get_index_al(list, i);
    //     //printf("childtemp->num: %d\n", childtemp->num);
    // }

    printf("\n\n");

    pthread_create(&threads[0],NULL,test_runFE,(void*)y);
    pthread_join(threads[0],NULL);
    //}
    // for(int i=0; i<list->length; i++){
    //     item_t* childtemp = get_index_al(list, i);
    //     //printf("childtemp->num: %d\n", childtemp->num);
    // }

    //printf("list->length: %d\n", (int)list->length);
}


// /******************************************
//  *                  TESTS                 *
//  ******************************************/



Test(al_suite, 0_creation, .timeout=2){
    arraylist_t *locallist = new_al(sizeof(test_item_t));

    cr_assert_not_null(locallist, "List returned was NULL");
}

Test(al_suite, 1_deletion, .timeout=2){
    arraylist_t *locallist = new_al(sizeof(test_item_t));

    cr_assert_not_null(locallist, "List returned was NULL");

    delete_al(locallist, test_item_t_free_func);

    cr_assert(true, "Delete completed without crashing");
}

Test(al_suite, 2_insertion, .timeout=2, .init=setup, .fini=teardown){
    cr_assert(true, "I win");
}

Test(al_suite, 3_removal, .timeout=2, .init=setup, .fini=teardown){
}


Test(al_suite, test5 , .timeout = 2)
{
    arraylist_t *locallist1 = new_al(sizeof(student_t));
    arraylist_t *locallist2 = new_al(sizeof(student_t));

    student_t *student = calloc(1,sizeof(student_t));
    student->gpa = 4.0;

    for(int i = 0 ; i < 100 ; i++)
    {
        cr_assert(i == insert_al(locallist1,student));
        cr_assert(locallist1->length == i + 1);
        cr_assert(i == insert_al(locallist2,student));
        cr_assert(locallist2->length == i + 1);
    }

    for(int i = 0 ; i < 100 ; i++)
    {
        //printf("%d\n", i);
        cr_assert(remove_data_al(locallist1,student) == true);
        cr_assert(remove_data_al(locallist2,student) == true);
    }

    cr_assert(locallist1->capacity == 4);
    cr_assert(locallist1->length == 0);
    cr_assert(locallist2->capacity == 4);
    cr_assert(locallist2->length == 0);
}

arraylist_t *al;


typedef struct {
    char* name;
    int32_t id;
    double gpa;
}student_tt;

Test(al_suite, test6 , .timeout = 2)
{
    student_tt* matt = malloc(sizeof(student_tt));
    student_tt* matt2 = malloc(sizeof(student_tt));
    student_tt* matt3 = malloc(sizeof(student_tt));
    student_tt* matt4 = malloc(sizeof(student_tt));
    student_tt* matt5 = malloc(sizeof(student_tt));

    matt->name = "matt";
    matt->id = 1;
    matt -> gpa = 50.0;

    matt2->name = "matt2";
    matt2->id = 1;
    matt2 -> gpa = 50.0;

    matt3->name = "matt3";
    matt3->id = 1;
    matt3 -> gpa = 50.0;

    matt4->name = "matt4";
    matt4->id = 1;
    matt4 -> gpa = 50.0;

    matt5->name = "matt5";
    matt5->id = 1;
    matt5 -> gpa = 50.0;

    al = new_al(sizeof(student_tt));
    insert_al(al,matt);
    insert_al(al,matt2);
    insert_al(al,matt3);
    insert_al(al,matt4);
    insert_al(al,matt5);

    remove_data_al(al,matt4);
    remove_data_al(al,matt2);

    free(matt);
    free(matt2);
    free(matt3);
    free(matt4);
    free(matt5);

    cr_assert(al->capacity == 4);
    cr_assert(al->length == 3);
}



/**********************************************************************************/

