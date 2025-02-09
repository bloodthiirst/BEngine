#pragma once

#include "../Typedefs/Typedefs.h"
#include "../Containers/DArray.h"
#include <Windows.h>

struct TestResult
{
    bool success;
    char *function;
    char *file;
    size_t line;
    char *message;
};

#define EVALUATE_NO_MESSAGE(predicate)         \
    if (!(predicate))                          \
    {                                          \
        TestResult failed_return = {};         \
        failed_return.function = __FUNCTION__; \
        failed_return.file = __FILE__;         \
        failed_return.line = __LINE__;         \
        failed_return.success = false;         \
        failed_return.message = nullptr;       \
        return failed_return;                  \
    }\

#define EVALUATE_WTIH_MESSAGE(predicate, eval_message) \
    if (!(predicate))                                  \
    {                                                  \
        TestResult failed_return = {};                 \
        failed_return.function = __FUNCTION__;         \
        failed_return.file = __FILE__;                 \
        failed_return.line = __LINE__;                 \
        failed_return.success = false;                 \
        failed_return.message = eval_message;          \
        return failed_return;                          \
    }                                                  \

#define GET_EVALUATE_MACRO(_1, _2, NAME, ...) NAME
#define EVALUATE(...) GET_EVALUATE_MACRO(__VA_ARGS__, EVALUATE_WTIH_MESSAGE, EVALUATE_NO_MESSAGE, )(__VA_ARGS__)

#define TEST_DECLARATION(name)  \
    static TestResult name()

#define TEST_END(...)                           \
    {                                           \
        TestResult success_return = {};         \
        success_return.function = __FUNCTION__; \
        success_return.file = __FILE__;         \
        success_return.line = __LINE__;         \
        success_return.success = true;          \
        success_return.message = nullptr;       \
        return success_return;                  \
    }

using TestCallback = Func<TestResult>;

struct BTest
{
    static inline DArray<TestCallback> all_tests = {};

    static void Init()
    {
        CoreContext::DefaultContext();

        Allocator alloc = HeapAllocator::Create();
        DArray<TestCallback>::Create(10, &all_tests, alloc);
    }

    static void Append(TestCallback test)
    {
        DArray<TestCallback>::Add(&all_tests, test);
    }

    static void AppendAll(DArray<TestCallback> tests)
    {
        for (size_t i = 0; i < tests.size; ++i)
        {
            DArray<TestCallback>::Add(&all_tests, tests.data[i]);
        }

        DArray<TestCallback>::Destroy(&tests);
    }

    static void RunAll()
    {
        LARGE_INTEGER freq = {};
        QueryPerformanceFrequency(&freq);

        for (size_t i = 0; i < all_tests.size; ++i)
        {
            TestCallback fncPtr = all_tests.data[i];

            LARGE_INTEGER before = {}; 
            QueryPerformanceCounter(&before);

            TestResult result = fncPtr();
            
            LARGE_INTEGER after = {};
            QueryPerformanceCounter(&after);

            int64_t ticks = after.QuadPart - before.QuadPart;
            double ms = (double) ( ( (ticks) * 1000) / freq.QuadPart);

            if (!result.success)
            {
                // Set console color to red
                printf("\033[0;31m");
                printf("[ FAILED  ] ");
            }
            else
            {
                // Set console color to green
                printf("\033[0;32m"); 
                printf("[ SUCCESS ] ");
            }

            // set console color to normal (white)
            printf("\033[0m");
            printf(" => ");
            printf("Name : %s , ", result.function);       
            printf("Time : %f ms , %lld ticks", ms , ticks);

            if (!result.success)
            {
                printf(" , ");
                printf("File : %s(%zu,0)", result.file , result.line);
                
                if(result.message != nullptr)
                {
                    printf(" , ");
                    printf("Message : %s", result.message);
                }
            }

            printf("\n");
        }
    }
};