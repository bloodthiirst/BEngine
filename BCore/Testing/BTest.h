#pragma once

#include "../Typedefs/Typedefs.h"
#include "../Containers/DArray.h"

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
    }

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
    }

#define GET_EVALUATE_MACRO(_1, _2, NAME, ...) NAME
#define EVALUATE(...) GET_EVALUATE_MACRO(__VA_ARGS__, EVALUATE_WTIH_MESSAGE, EVALUATE_NO_MESSAGE, )(__VA_ARGS__)

#define TEST_DECLARATION(name) \
    static TestResult name() \

#define TEST_BODY(...)                                \
    {                                                 \
        {__VA_ARGS__} TestResult default_return = {}; \
        default_return.function = __FUNCTION__;       \
        default_return.file = __FILE__;               \
        default_return.line = __LINE__;               \
        default_return.success = true;                \
        return default_return;                        \
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
        for (size_t i = 0; i < all_tests.size; ++i)
        {
            TestCallback fncPtr = all_tests.data[i];
            TestResult result = fncPtr();

            if (!result.success)
            {
                printf("\033[0;31m");
                printf("[ FAILED  ] ");
            }
            else
            {
                printf("\033[0;32m"); 
                printf("[ SUCCESS ] ");
            }

            printf("\033[0m");
            printf(" => ");
            printf("Name : %s", result.function);

            if (!result.success)
            {
                printf(" , ");
                printf("File : %s , ", result.file);
                printf("Line : %zu", result.line);

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