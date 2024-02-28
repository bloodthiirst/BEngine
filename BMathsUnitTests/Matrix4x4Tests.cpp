#include "pch.h"
#include "CppUnitTest.h"
#include <Maths/Vector4.h>
#include <Maths/Matrix4x4.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace BEngineMathsUnitTests
{
    TEST_CLASS ( Matrix4x4Tests )
    {
    private:

        Matrix4x4 GetSimpleInput ()
        {
            Matrix4x4 input = Matrix4x4 ();
            input.values[0] = 1;
            input.values[1] = 1;
            input.values[2] = 1;
            input.values[3] = -1;

            input.values[4] = 1;
            input.values[5] = 1;
            input.values[6] = -1;
            input.values[7] = 1;

            input.values[8] = 1;
            input.values[9] = -1;
            input.values[10] = 1;
            input.values[11] = 1;

            input.values[12] = -1;
            input.values[13] = 1;
            input.values[14] = 1;
            input.values[15] = 1;

            return input;
        }

        Matrix4x4 GetFloatingInput ()
        {
            Matrix4x4 input = Matrix4x4 ();
            input.values[0] = 1.2;
            input.values[1] = 1.5;
            input.values[2] = 9;
            input.values[3] = -5;

            input.values[4] = 7;
            input.values[5] = 99;
            input.values[6] = -10;
            input.values[7] = 16;

            input.values[8] = 18;
            input.values[9] = -1.1;
            input.values[10] = 1.33;
            input.values[11] = 1.8;

            input.values[12] = -1.8;
            input.values[13] = 18.9;
            input.values[14] = 1.558;
            input.values[15] = -11.5;

            return input;
        }

    public:

        TEST_METHOD ( PerpectiveMatrix )
        {
            Matrix4x4 input = Matrix4x4::Perspective ( 90, 1000, 1, 16.0f / 9.0f );

            Matrix4x4 inverse = Matrix4x4::Inverse ( input );

            Matrix4x4 result = inverse * input;
            Matrix4x4 expected = Matrix4x4::Identity ();

            bool approxEq = Matrix4x4::ApproxEqual ( result, expected, 0.00001f );
            Assert::IsTrue ( approxEq );
        }

        TEST_METHOD ( RotationMatrix )
        {
            Vector3 input = Vector3::Right ();
            Quaternion rot = Quaternion::AxisRotation ( 90, Vector3::Up() );
            Matrix4x4 mat = Matrix4x4::Rotate(rot);

            Vector3 output = mat * input;

            Vector3 expected = Vector3::Backward();

            bool result = Vector3::EqualsApprox(expected ,output);

            Assert::IsTrue ( result == true );
        }

        TEST_METHOD( DoubleRotationMatrix )
        {
            Vector3 input = Vector3(0,0,0);
            
            Vector3 translate = Vector3(0,0,-1);
            Quaternion rot = Quaternion::AxisRotation( 90, Vector3::Up() );
            
            Matrix4x4 tra_mat = Matrix4x4::Translate( translate );
            Matrix4x4 rot_mat = Matrix4x4::Rotate( rot );

            Vector3 translated = (tra_mat) * input;
            Vector3 rotated = (rot_mat) * translated;
        }

        TEST_METHOD( MultiplyMatrix )
        {
            Vector3 input = Vector3( 0, 1, 0 );
            Matrix4x4 mat = Matrix4x4::Translate( Vector3::Right() );

            Vector3 output = (mat * mat) * input;

            Vector3 expected = Vector3( 2, 1, 0 );

            bool result = expected == output;

            Assert::IsTrue( result == true );
        }

        TEST_METHOD ( TranslationMatrix )
        {          
            Vector3 input = Vector3 ( 0, 1, 0 );
            Matrix4x4 mat = Matrix4x4::Translate ( Vector3::One () );

            Vector3 output = mat * input;
            
            Vector3 expected = Vector3 ( 1, 2, 1 );

            bool result = expected == output;

            Assert::IsTrue ( result == true );
        }

        TEST_METHOD ( EqualityOperator )
        {
            Matrix4x4 input = GetSimpleInput ();

            Matrix4x4 equal = GetSimpleInput ();

            bool result = equal == input;

            Assert::IsTrue ( result == true );
        }

        TEST_METHOD ( InverseSimple )
        {
            Matrix4x4 input = GetSimpleInput ();

            Matrix4x4 inverse = Matrix4x4::Inverse ( input );

            Matrix4x4 result = inverse * input;

            Matrix4x4 expected = Matrix4x4::Identity ();

            bool approxEq = Matrix4x4::ApproxEqual ( expected, result, 0.000001f );

            Assert::IsTrue ( approxEq );
        }

        TEST_METHOD ( InverseFloating )
        {
            Matrix4x4 input = GetFloatingInput ();

            Matrix4x4 inverse = Matrix4x4::Inverse ( input );

            Matrix4x4 result = inverse * input;

            Matrix4x4 expected = Matrix4x4::Identity ();

            bool approxEq = Matrix4x4::ApproxEqual ( expected, result, 0.000001f );

            Assert::IsTrue ( approxEq );
        }

        TEST_METHOD ( Determinant )
        {
            Matrix4x4 input = GetSimpleInput ();

            float result = Matrix4x4::Determinant ( input );

            // correct answer : -16
            float expected = -16;

            Assert::AreEqual ( expected, result );
        }
    };
}
