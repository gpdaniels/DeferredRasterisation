/*
The MIT License

Copyright (c) 2017 Geoffrey Daniels. http://gpdaniels.com/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE
*/

#pragma once
#ifndef RAYMARCH_MATHS_HPP
#define RAYMARCH_MATHS_HPP

namespace DeferredRasterisation {
    /// @brief  Vector3 is a simple three dimensional vector structure.
    class Vector3 {
    private:
        /// @brief  Vector data.
        float Data[3];

    public:
        /// @brief  Default empty constructor.
        Vector3(void) = default;

        /// @brief  Value constructor that sets all dimensions of the vector.
        Vector3(float X, float Y, float Z);

    public:
        /// @brief  Vector element accessor, allowing modification.
        /// @return Reference to the indexed element.
        float& operator[](unsigned int Index);

        /// @brief  Vector element accessor.
        /// @return Constant reference to the indexed element.
        const float& operator[](unsigned int Index) const;

    public:
        /// @brief  Vector data accessor.
        /// @return Pointer to constant internal data.
        const float* data(void) const;

    public:
        /// @brief  Static normalising function.
        /// @param  Vector - The vector to normalise.
        /// @return A normalised vector.
        static Vector3 Normalise(const Vector3& Vector);

        /// @brief  Static vector cross product function.
        /// @param  LHS - The left hand side vector.
        /// @param  RHS - The right hand side vector.
        /// @return The vector cross product.
        static Vector3 Cross(const Vector3& LHS, const Vector3& RHS);

    public:
        /// @brief  Friend addition operator.
        /// @param  LHS - The left hand side vector.
        /// @param  RHS - The right hand side vector.
        /// @return The vector result of adding the elements of each vector together.
         friend Vector3 operator+(const Vector3& LHS, const Vector3& RHS);

         /// @brief  Friend subtraction operator.
         /// @param  LHS - The left hand side vector.
         /// @param  RHS - The right hand side vector.
         /// @return The vector result of subtracting the elements of each vector from one another.
         friend Vector3 operator-(const Vector3& LHS, const Vector3& RHS);
    };

    /// @brief  Matrix44 is a simple four by four matrix structure.
    class Matrix44 {
    private:
        /// @brief  Matrix data.
        float Data[4][4];

    public:
        /// @brief  Default empty constructor.
        Matrix44(void) = default;

        /// @brief  Value constructor that sets all values of the matrix.
        Matrix44(float Matrix00, float Matrix01, float Matrix02, float Matrix03,
                 float Matrix10, float Matrix11, float Matrix12, float Matrix13,
                 float Matrix20, float Matrix21, float Matrix22, float Matrix23,
                 float Matrix30, float Matrix31, float Matrix32, float Matrix33);

    public:
        /// @brief  Static frustum matrix function.
        /// @param  Left - The left distance.
        /// @param  Right - The right distance.
        /// @param  Bottom - The bottom distance.
        /// @param  Top - The top distance.
        /// @param  Near - The near distance.
        /// @param  Far - The far distance.
        /// @return A frustum matrix.
        static Matrix44 Frustum(float Left, float Right, float Bottom, float Top, float Near, float Far);

        /// @brief  Static perspective matrix function.
        /// @param  FieldOfViewDegrees - The vertical field of view in degrees.
        /// @param  AspectRatio - The aspect ratio between vertical and horizontal.
        /// @param  Near - The near distance.
        /// @param  Far - The far distance.
        /// @return A perspective matrix.
        static Matrix44 Perspective(float FieldOfViewDegrees, float AspectRatio, float Near, float Far);

        /// @brief  Static view matrix function.
        /// @param  Eye - The location of the eye position.
        /// @param  Target - The location of the target to look at.
        /// @param  Up - The direction that should be considered up.
        /// @return A view matrix.
        static Matrix44 View(const Vector3& Eye, const Vector3& Target, const Vector3& Up);

    public:
        /// @brief  Matrix element accessor, allowing modification.
        /// @return Reference to the indexed element.
        float& operator()(unsigned int X, unsigned int Y);

        /// @brief  Matrix element accessor.
        /// @return Constant reference to the indexed element.
        const float& operator()(unsigned int X, unsigned int Y) const;

    public:
        /// @brief  Matrix data accessor.
        /// @return Pointer to constant internal data.
        const float* data(void) const;

    public:
        /// @brief  Static inverting function.
        /// @param  Matrix - The matrix to invert.
        /// @return An inverse matrix.
        static Matrix44 Invert(const Matrix44& Matrix);

    public:
        /// @brief  Friend multiplication operator.
        /// @param  LHS - The left hand side matrix.
        /// @param  RHS - The right hand side matrix.
        /// @return The matrix result of multiplying the two matrices together.
        friend Matrix44 operator*(const Matrix44& LHS, const Matrix44& RHS);
    };
}

#endif // RAYMARCH_MATHS_HPP
