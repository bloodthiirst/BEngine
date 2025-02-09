#include <Testing/BTest.h>
#include "ContainerUtilsTests.h"
#include "DArrayTests.h"
#include "QueueTests.h"
#include "FreeListTests.h"
#include "HMapTests.h"
#include "StringTests.h"
#include "MinHeapTests.h"
#include "SlotArrayTests.h"
#include "DeferTests.h"

TEST_DECLARATION(Wrong)
{
    EVALUATE( 1 == 2)
    TEST_END()
}

TEST_DECLARATION(WrongWithMessage)
{
    EVALUATE( 1 == 2 , "Testing messages for errors")
    TEST_END()
}

int main(int argc , char** argv)
{
    BTest::Init();
    
    BTest::AppendAll(Tests::HMapTests::GetAll());
    BTest::AppendAll(Tests::ContainerUtilsTests::GetAll());   
    BTest::AppendAll(Tests::DArrayTests::GetAll());
    BTest::AppendAll(Tests::QueueTests::GetAll());
    BTest::AppendAll(Tests::FreeListTests::GetAll());
    BTest::AppendAll(Tests::StringTests::GetAll());
    BTest::AppendAll(Tests::MinHeapTests::GetAll());
    BTest::AppendAll(Tests::DeferTests::GetAll());
    BTest::AppendAll(Tests::SlotArrayTests::GetAll());

    BTest::RunAll();
}