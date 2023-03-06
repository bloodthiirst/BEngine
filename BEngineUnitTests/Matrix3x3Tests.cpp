#include "pch.h"
#include "CppUnitTest.h"
#include "../BEngine/Core/Maths/Vector4.h"
#include "../BEngine/Core/Maths/Matrix3x3.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace BEngineMathsUnitTests
{
    TEST_CLASS ( Matrix3x3Tests )
    {
    private:
        Matrix3x3 GetSimpleInput ()
        {
            Matrix3x3 input = {};
            input.values[0] = 3;
            input.values[1] = 4;
            input.values[2] = 6;

            input.values[3] = 1;
            input.values[4] = 8;
            input.values[5] = 5;

            input.values[6] = 12;
            input.values[7] = 5.5;
            input.values[8] = 1;

            return input;
        }
    public:

        TEST_METHOD ( Determinant )
        {
            Matrix3x3 input = GetSimpleInput ();

            float result = Matrix3x3::Determinant ( input );

            // correct answer 
            float expected = -365.5;

            Assert::IsTrue ( result == expected );
        }
    };
}
