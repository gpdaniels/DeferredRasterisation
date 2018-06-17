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

#include "Maths.hpp"

#include <cassert>
#include <cmath>

namespace DeferredRasterisation {
    Vector3::Vector3(float X, float Y, float Z)
        : Data { X, Y, Z } {
    }

    float& Vector3::operator[](unsigned int Index) {
        return this->Data[Index];
    }

    const float& Vector3::operator[](unsigned int Index) const {
        return const_cast<Vector3*>(this)->operator[](Index);
    }

    const float* Vector3::data(void) const {
        return this->Data;
    }

    Vector3 Vector3::Normalise(const Vector3& Vector) {
        const float Denominator = std::sqrt(Vector.Data[0] * Vector.Data[0] + Vector.Data[1] * Vector.Data[1] + Vector.Data[2] * Vector.Data[2]);
        assert(Denominator != 0);
        return Vector3(
            Vector.Data[0] / Denominator,
            Vector.Data[1] / Denominator,
            Vector.Data[2] / Denominator
        );
    }

    Vector3 Vector3::Cross(const Vector3& LHS, const Vector3& RHS) {
        return Vector3(
            LHS.Data[1] * RHS.Data[2] - LHS.Data[2] * RHS.Data[1],
            LHS.Data[2] * RHS.Data[0] - LHS.Data[0] * RHS.Data[2],
            LHS.Data[0] * RHS.Data[1] - LHS.Data[1] * RHS.Data[0]
        );
    }

    Vector3 operator+(const Vector3& LHS, const Vector3& RHS) {
        return Vector3 {
            LHS.Data[0] + RHS.Data[0],
            LHS.Data[1] + RHS.Data[1],
            LHS.Data[2] + RHS.Data[2]
        };
    }

    Vector3 operator-(const Vector3& LHS, const Vector3& RHS) {
        return Vector3 {
            LHS.Data[0] - RHS.Data[0],
            LHS.Data[1] - RHS.Data[1],
            LHS.Data[2] - RHS.Data[2]
        };
    }

    Matrix44::Matrix44(float Matrix00, float Matrix10, float Matrix20, float Matrix30,
                       float Matrix01, float Matrix11, float Matrix21, float Matrix31,
                       float Matrix02, float Matrix12, float Matrix22, float Matrix32,
                       float Matrix03, float Matrix13, float Matrix23, float Matrix33)
        : Data {
            { Matrix00, Matrix10, Matrix20, Matrix30 },
            { Matrix01, Matrix11, Matrix21, Matrix31 },
            { Matrix02, Matrix12, Matrix22, Matrix32 },
            { Matrix03, Matrix13, Matrix23, Matrix33 }
        } {

    }

    Matrix44 Matrix44::Frustum(float Left, float Right, float Bottom, float Top, float Near, float Far) {
        const float TwoNear = 2.0 * Near;
        const float Width = Right - Left;
        const float Height = Top - Bottom;
        const float Depth = Far - Near;
        return Matrix44(
            TwoNear / Width, 0.0, (Right + Left) / Width, 0.0,
            0.0, TwoNear / Height, (Top + Bottom) / Height, 0.0,
            0.0, 0.0, (-Far - Near) / Depth, (-TwoNear * Far) / Depth,
            0.0, 0.0, -1.0, 0.0
        );
    }

    Matrix44 Matrix44::Perspective(float FieldOfViewDegrees, float AspectRatio, float Near, float Far) {
        const float VerticalLimit = Near * std::tan(FieldOfViewDegrees * M_PI / 360.0);
        const float HorizontalLimit = VerticalLimit * AspectRatio;
        return Matrix44::Frustum(-HorizontalLimit, HorizontalLimit, -VerticalLimit, VerticalLimit, Near, Far);
    }

    Matrix44 Matrix44::View(const Vector3& Eye, const Vector3& Target, const Vector3& Up) {
        const Vector3 AxisZ = Vector3::Normalise(Eye - Target);
        const Vector3 AxisX = Vector3::Normalise(Vector3::Cross(Up, AxisZ));
        const Vector3 AxisY = Vector3::Cross(AxisZ, AxisX);
        return Matrix44(
            AxisX[0], AxisX[1], AxisX[2], -(AxisX[0] * Eye[0] + AxisX[1] * Eye[1] + AxisX[2] * Eye[2]),
            AxisY[0], AxisY[1], AxisY[2], -(AxisY[0] * Eye[0] + AxisY[1] * Eye[1] + AxisY[2] * Eye[2]),
            AxisZ[0], AxisZ[1], AxisZ[2], -(AxisZ[0] * Eye[0] + AxisZ[1] * Eye[1] + AxisZ[2] * Eye[2]),
            0, 0, 0, 1
        );
    }

    float& Matrix44::operator()(unsigned int X, unsigned int Y) {
        return this->Data[Y][X];
    }

    const float& Matrix44::operator()(unsigned int X, unsigned int Y) const {
        return const_cast<Matrix44*>(this)->operator()(X, Y);
    }

    const float* Matrix44::data(void) const {
        return &this->Data[0][0];
    }

    Matrix44 Matrix44::Invert(const Matrix44& Matrix) {
        Matrix44 Result(
             Matrix.Data[1][1] * Matrix.Data[2][2] * Matrix.Data[3][3] - Matrix.Data[1][1] * Matrix.Data[2][3] * Matrix.Data[3][2] - Matrix.Data[2][1] * Matrix.Data[1][2] * Matrix.Data[3][3] + Matrix.Data[2][1] * Matrix.Data[1][3] * Matrix.Data[3][2] + Matrix.Data[3][1] * Matrix.Data[1][2] * Matrix.Data[2][3] - Matrix.Data[3][1] * Matrix.Data[1][3] * Matrix.Data[2][2],
            -Matrix.Data[0][1] * Matrix.Data[2][2] * Matrix.Data[3][3] + Matrix.Data[0][1] * Matrix.Data[2][3] * Matrix.Data[3][2] + Matrix.Data[2][1] * Matrix.Data[0][2] * Matrix.Data[3][3] - Matrix.Data[2][1] * Matrix.Data[0][3] * Matrix.Data[3][2] - Matrix.Data[3][1] * Matrix.Data[0][2] * Matrix.Data[2][3] + Matrix.Data[3][1] * Matrix.Data[0][3] * Matrix.Data[2][2],
             Matrix.Data[0][1] * Matrix.Data[1][2] * Matrix.Data[3][3] - Matrix.Data[0][1] * Matrix.Data[1][3] * Matrix.Data[3][2] - Matrix.Data[1][1] * Matrix.Data[0][2] * Matrix.Data[3][3] + Matrix.Data[1][1] * Matrix.Data[0][3] * Matrix.Data[3][2] + Matrix.Data[3][1] * Matrix.Data[0][2] * Matrix.Data[1][3] - Matrix.Data[3][1] * Matrix.Data[0][3] * Matrix.Data[1][2],
            -Matrix.Data[0][1] * Matrix.Data[1][2] * Matrix.Data[2][3] + Matrix.Data[0][1] * Matrix.Data[1][3] * Matrix.Data[2][2] + Matrix.Data[1][1] * Matrix.Data[0][2] * Matrix.Data[2][3] - Matrix.Data[1][1] * Matrix.Data[0][3] * Matrix.Data[2][2] - Matrix.Data[2][1] * Matrix.Data[0][2] * Matrix.Data[1][3] + Matrix.Data[2][1] * Matrix.Data[0][3] * Matrix.Data[1][2],
            -Matrix.Data[1][0] * Matrix.Data[2][2] * Matrix.Data[3][3] + Matrix.Data[1][0] * Matrix.Data[2][3] * Matrix.Data[3][2] + Matrix.Data[2][0] * Matrix.Data[1][2] * Matrix.Data[3][3] - Matrix.Data[2][0] * Matrix.Data[1][3] * Matrix.Data[3][2] - Matrix.Data[3][0] * Matrix.Data[1][2] * Matrix.Data[2][3] + Matrix.Data[3][0] * Matrix.Data[1][3] * Matrix.Data[2][2],
             Matrix.Data[0][0] * Matrix.Data[2][2] * Matrix.Data[3][3] - Matrix.Data[0][0] * Matrix.Data[2][3] * Matrix.Data[3][2] - Matrix.Data[2][0] * Matrix.Data[0][2] * Matrix.Data[3][3] + Matrix.Data[2][0] * Matrix.Data[0][3] * Matrix.Data[3][2] + Matrix.Data[3][0] * Matrix.Data[0][2] * Matrix.Data[2][3] - Matrix.Data[3][0] * Matrix.Data[0][3] * Matrix.Data[2][2],
            -Matrix.Data[0][0] * Matrix.Data[1][2] * Matrix.Data[3][3] + Matrix.Data[0][0] * Matrix.Data[1][3] * Matrix.Data[3][2] + Matrix.Data[1][0] * Matrix.Data[0][2] * Matrix.Data[3][3] - Matrix.Data[1][0] * Matrix.Data[0][3] * Matrix.Data[3][2] - Matrix.Data[3][0] * Matrix.Data[0][2] * Matrix.Data[1][3] + Matrix.Data[3][0] * Matrix.Data[0][3] * Matrix.Data[1][2],
             Matrix.Data[0][0] * Matrix.Data[1][2] * Matrix.Data[2][3] - Matrix.Data[0][0] * Matrix.Data[1][3] * Matrix.Data[2][2] - Matrix.Data[1][0] * Matrix.Data[0][2] * Matrix.Data[2][3] + Matrix.Data[1][0] * Matrix.Data[0][3] * Matrix.Data[2][2] + Matrix.Data[2][0] * Matrix.Data[0][2] * Matrix.Data[1][3] - Matrix.Data[2][0] * Matrix.Data[0][3] * Matrix.Data[1][2],
             Matrix.Data[1][0] * Matrix.Data[2][1] * Matrix.Data[3][3] - Matrix.Data[1][0] * Matrix.Data[2][3] * Matrix.Data[3][1] - Matrix.Data[2][0] * Matrix.Data[1][1] * Matrix.Data[3][3] + Matrix.Data[2][0] * Matrix.Data[1][3] * Matrix.Data[3][1] + Matrix.Data[3][0] * Matrix.Data[1][1] * Matrix.Data[2][3] - Matrix.Data[3][0] * Matrix.Data[1][3] * Matrix.Data[2][1],
            -Matrix.Data[0][0] * Matrix.Data[2][1] * Matrix.Data[3][3] + Matrix.Data[0][0] * Matrix.Data[2][3] * Matrix.Data[3][1] + Matrix.Data[2][0] * Matrix.Data[0][1] * Matrix.Data[3][3] - Matrix.Data[2][0] * Matrix.Data[0][3] * Matrix.Data[3][1] - Matrix.Data[3][0] * Matrix.Data[0][1] * Matrix.Data[2][3] + Matrix.Data[3][0] * Matrix.Data[0][3] * Matrix.Data[2][1],
             Matrix.Data[0][0] * Matrix.Data[1][1] * Matrix.Data[3][3] - Matrix.Data[0][0] * Matrix.Data[1][3] * Matrix.Data[3][1] - Matrix.Data[1][0] * Matrix.Data[0][1] * Matrix.Data[3][3] + Matrix.Data[1][0] * Matrix.Data[0][3] * Matrix.Data[3][1] + Matrix.Data[3][0] * Matrix.Data[0][1] * Matrix.Data[1][3] - Matrix.Data[3][0] * Matrix.Data[0][3] * Matrix.Data[1][1],
            -Matrix.Data[0][0] * Matrix.Data[1][1] * Matrix.Data[2][3] + Matrix.Data[0][0] * Matrix.Data[1][3] * Matrix.Data[2][1] + Matrix.Data[1][0] * Matrix.Data[0][1] * Matrix.Data[2][3] - Matrix.Data[1][0] * Matrix.Data[0][3] * Matrix.Data[2][1] - Matrix.Data[2][0] * Matrix.Data[0][1] * Matrix.Data[1][3] + Matrix.Data[2][0] * Matrix.Data[0][3] * Matrix.Data[1][1],
            -Matrix.Data[1][0] * Matrix.Data[2][1] * Matrix.Data[3][2] + Matrix.Data[1][0] * Matrix.Data[2][2] * Matrix.Data[3][1] + Matrix.Data[2][0] * Matrix.Data[1][1] * Matrix.Data[3][2] - Matrix.Data[2][0] * Matrix.Data[1][2] * Matrix.Data[3][1] - Matrix.Data[3][0] * Matrix.Data[1][1] * Matrix.Data[2][2] + Matrix.Data[3][0] * Matrix.Data[1][2] * Matrix.Data[2][1],
             Matrix.Data[0][0] * Matrix.Data[2][1] * Matrix.Data[3][2] - Matrix.Data[0][0] * Matrix.Data[2][2] * Matrix.Data[3][1] - Matrix.Data[2][0] * Matrix.Data[0][1] * Matrix.Data[3][2] + Matrix.Data[2][0] * Matrix.Data[0][2] * Matrix.Data[3][1] + Matrix.Data[3][0] * Matrix.Data[0][1] * Matrix.Data[2][2] - Matrix.Data[3][0] * Matrix.Data[0][2] * Matrix.Data[2][1],
            -Matrix.Data[0][0] * Matrix.Data[1][1] * Matrix.Data[3][2] + Matrix.Data[0][0] * Matrix.Data[1][2] * Matrix.Data[3][1] + Matrix.Data[1][0] * Matrix.Data[0][1] * Matrix.Data[3][2] - Matrix.Data[1][0] * Matrix.Data[0][2] * Matrix.Data[3][1] - Matrix.Data[3][0] * Matrix.Data[0][1] * Matrix.Data[1][2] + Matrix.Data[3][0] * Matrix.Data[0][2] * Matrix.Data[1][1],
             Matrix.Data[0][0] * Matrix.Data[1][1] * Matrix.Data[2][2] - Matrix.Data[0][0] * Matrix.Data[1][2] * Matrix.Data[2][1] - Matrix.Data[1][0] * Matrix.Data[0][1] * Matrix.Data[2][2] + Matrix.Data[1][0] * Matrix.Data[0][2] * Matrix.Data[2][1] + Matrix.Data[2][0] * Matrix.Data[0][1] * Matrix.Data[1][2] - Matrix.Data[2][0] * Matrix.Data[0][2] * Matrix.Data[1][1]
        );

        const float Determinant = Matrix.Data[0][0] * Result.Data[0][0] + Matrix.Data[0][1] * Result.Data[1][0] + Matrix.Data[0][2] * Result.Data[2][0] + Matrix.Data[0][3] * Result.Data[3][0];
        assert(Determinant != 0);
        const float InverseDeterminant = 1.0 / Determinant;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                Result.Data[i][j] *= InverseDeterminant;
            }
        }

        return Result;
    }

    Matrix44 operator*(const Matrix44& LHS, const Matrix44& RHS) {
        Matrix44 Result;
        for (unsigned int RowLHS = 0; RowLHS < 4; ++RowLHS) {
            for (unsigned int ColumnRHS = 0; ColumnRHS < 4; ++ColumnRHS) {
                float Sum = 0;
                for (unsigned int ColumnLHSRowRHS = 0; ColumnLHSRowRHS < 4; ++ColumnLHSRowRHS) {
                    Sum += LHS.Data[RowLHS][ColumnLHSRowRHS] * RHS.Data[ColumnLHSRowRHS][ColumnRHS];
                }
                Result.Data[RowLHS][ColumnRHS] = Sum;
            }
        }
        return Result;
    }
}
