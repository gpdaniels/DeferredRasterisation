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
#ifndef RAYMARCH_RENDERER_HPP
#define RAYMARCH_RENDERER_HPP

#include "Volume.hpp"

#include "Maths.hpp"
#include "GameState.hpp"

#include <GL/glew.h>

#include <array>

namespace DeferredRasterisation {
    /// @brief  Renderer configures and runs OpenGL to draw a scene.
	class Renderer {
	private:
        /// @brief  The width of the OpenGL viewport.
        std::size_t ScreenWidth;

        /// @brief  The height of the OpenGL viewport.
        std::size_t ScreenHeight;

	private:
        /// @brief  Stage 1 vertex shader.
        GLuint VertexShader1;

        /// @brief  Stage 1 fragment shader.
        GLuint FragmentShader1;

        /// @brief  Stage 2 vertex shader.
        GLuint VertexShader2;

        /// @brief  Stage 2 fragment shader.
        GLuint FragmentShader2;

        /// @brief  The combined vertex and fragment shaders for stage 1.
        GLuint ShaderProgram1;

        /// @brief  The combined vertex and fragment shaders for stage 2.
        GLuint ShaderProgram2;

    private:
        /// @brief  Projection matrix.
        Matrix44 Projection;

        /// @brief  View matrix.
        Matrix44 View;

        /// @brief  Model matrix.
        Matrix44 Model;

    private:
        /// @brief  Number of subdivisions to break each voxel down into.
        GLfloat Subdivisions;

    private:
        /// @brief  The texture used to access the intermediate position data for stage 1.
        GLuint TexturePosition1;

        /// @brief  The texture used to access the intermediate position data for stage 1.
        GLuint TextureNormal1;

        /// @brief  The texture used to access the intermediate position data for stage 1.
        GLuint TextureColour1;

        /// @brief  The texture used to access the intermediate position data for stage 1.
        GLuint TextureDepth1;

        /// @brief  The framebuffer used to store the intermediate render from stage 1.
        GLuint FrameBuffer1;

        /// @brief  The texture used to access the intermediate position data for stage 2.
        GLuint TexturePosition2;

        /// @brief  The texture used to access the intermediate position data for stage 2.
        GLuint TextureNormal2;

        /// @brief  The texture used to access the intermediate position data for stage 2.
        GLuint TextureColour2;

        /// @brief  The framebuffer used to store the intermediate render from stage 2.
        GLuint FrameBuffer2;

    private:
        /// @brief  Shader uniform for the pre-multiplied model, view, and projection matrices.
        GLint ShaderUniformModelViewProjection;

        /// @brief  Shader uniform for the model matrix.
        GLint ShaderUniformModel;

        /// @brief  Shader uniform for the position texture input.
        GLint ShaderUniformPosition;

        /// @brief  Shader uniform for the normal texture input.
        GLint ShaderUniformNormal;

        /// @brief  Shader uniform for the colour texture input.
        GLint ShaderUniformColour;

    private:
        /// @brief  Shader uniform for the inverse pre-multiplied view and projection matrices.
        GLint ShaderUniformViewProjectionInverse;

        /// @brief  Shader uniform for the render direction.
        GLint ShaderUniformEvaluationDirection;

        /// @brief  Shader uniform for the voxel size.
        GLint ShaderUniformVoxelSize;

        /// @brief  Shader uniform for dimensions of a pixel on the screen.
        GLint ShaderUniformPixelDimensions;

        /// @brief  Shader uniform for a boolean flag that is set on the last render.
        GLint ShaderUniformLast;

        /// @brief  Shader uniform for the offset of the curret scene.
        GLint ShaderUniformSceneOffset;

    private:
        /// @brief  Vertex buffer to hold voxel data.
        GLuint VertexBuffer;

        /// @brief  Vertex array to hold vertex buffer.
        GLuint VertexArray;

	public:
        /// @brief  Constructor that specifies the size of the renderer viewport.
        Renderer(std::size_t ScreenWidth, std::size_t ScreenHeight);

        /// @brief  Render the gamestate to the current OpenGL window.
        /// @param  State - the state of the game.
        void Render(const GameState& State);
	};
}

#endif // RAYMARCH_RENDERER_HPP
