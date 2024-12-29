#include <Testing/BTest.h>
#include "ContainerUtilsTests.h"
#include "DArrayTests.h"
#include "FreeListTests.h"
#include "HMapTests.h"
#include "StringTests.h"

TEST_DECLARATION(Wrong)
TEST_BODY({
    EVALUATE( 1 == 2)
})

int main(int argc , char** argv)
{
    BTest::Init();
    
    BTest::Append(Wrong);
    BTest::AppendAll(Tests::ContainerUtilsTests::GetAll());   
    BTest::AppendAll(Tests::DArrayTests::GetAll());
    BTest::AppendAll(Tests::FreeListTests::GetAll());
    BTest::AppendAll(Tests::HMapTests::GetAll());
    BTest::AppendAll(Tests::StringTests::GetAll());
    
    BTest::RunAll();
}