#include "pch.h"
#include "CppUnitTest.h"
#include <Maths/Quaternion.h>
#include <Maths/Vector3.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace Tests
{
    TEST_CLASS ( QuaternionTests )
    {
    public:

        TEST_METHOD ( RotateDiagonal90DegreesUp )
        {
            Vector3 right = Vector3 ( 1, 1, 1 );
            right = Vector3::Normalize ( right );

            Vector3 up = Vector3::Up ();

            Quaternion rotation = Quaternion::AxisRotation ( 90, up );

            Vector3 result = rotation * right;

            Vector3 expected = Vector3::Backward ();

            bool isTrue = result == expected;
            
            Assert::IsTrue ( true );
        }


        TEST_METHOD ( RotateRight90DegreesUp )
        {
            Vector3 right = Vector3::Right ();

            Vector3 up = Vector3::Up ();

            Quaternion rotation = Quaternion::AxisRotation ( 90, up );

            Vector3 result = rotation * right;

            Vector3 expected = Vector3::Backward ();

            bool isTrue = result == expected;

            Assert::IsTrue ( true );
        }


    };
}
