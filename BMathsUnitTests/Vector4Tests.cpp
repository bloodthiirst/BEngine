#include "pch.h"
#include "CppUnitTest.h"
#include <Maths/Vector4.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace Tests
{
	TEST_CLASS(Vector4Tests)
	{
	public:
		
		TEST_METHOD(Multiply)
		{
            Vector4 input = Vector4 ( 1, 2, 3, 4 );
            
            Vector4 result = input * 2.0f;

            Vector4 expected = Vector4 ( 2, 4, 6, 8 );

            Assert::IsTrue ( result == expected );
		}
	};
}
