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

#include "Renderer.hpp"
#include "ShaderSource.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstring>
#include <iostream>

// When debugging check for OpenGL errors after every usage.
#ifdef _DEBUG
    #define CHECK_GL(Expression) Expression; \
    { \
        GLenum EC; \
        while ((EC = glGetError()) != GL_NO_ERROR) { \
            std::cout << "OpenGL error [" << EC << "] on line [" << __LINE__ << "]: " << gluErrorString(EC) << std::endl; \
            asm("int3"); \
        } \
    }
#else
    #define CHECK_GL(Expression) Expression;
#endif

namespace DeferredRasterisation {
    // Constructor that initialises the renderer at the provided size.
    Renderer::Renderer(std::size_t ScreenWidth, std::size_t ScreenHeight)
        : ScreenWidth(ScreenWidth)
        , ScreenHeight(ScreenHeight) {

        // Create the WebGL context.
        CHECK_GL(glViewport(0, 0, this->ScreenWidth, this->ScreenHeight));

        ///////////////////////////////////////////////////////////////////////////
        /// Create the shaders.                                                  //
        ///////////////////////////////////////////////////////////////////////////

        // Create the shader fragments.

        this->VertexShader1 = CHECK_GL(glCreateShader(GL_VERTEX_SHADER));
        const char* VertexShaderSource1 = ShaderSource::VertexShaderSource1.c_str();
        CHECK_GL(glShaderSource(this->VertexShader1, 1, &VertexShaderSource1, nullptr));
        CHECK_GL(glCompileShader(this->VertexShader1));

        this->FragmentShader1 = CHECK_GL(glCreateShader(GL_FRAGMENT_SHADER));
        const char* FragmentShaderSource1 = ShaderSource::FragmentShaderSource1.c_str();
        CHECK_GL(glShaderSource(this->FragmentShader1, 1, &FragmentShaderSource1, nullptr));
        CHECK_GL(glCompileShader(this->FragmentShader1));

        this->VertexShader2 = CHECK_GL(glCreateShader(GL_VERTEX_SHADER));
        const char* VertexShaderSource2 = ShaderSource::VertexShaderSource2.c_str();
        CHECK_GL(glShaderSource(this->VertexShader2, 1, &VertexShaderSource2, nullptr));
        CHECK_GL(glCompileShader(this->VertexShader2));

        this->FragmentShader2 = CHECK_GL(glCreateShader(GL_FRAGMENT_SHADER));
        const char* FragmentShaderSource2 = ShaderSource::FragmentShaderSource2.c_str();
        CHECK_GL(glShaderSource(this->FragmentShader2, 1, &FragmentShaderSource2, nullptr));
        CHECK_GL(glCompileShader(this->FragmentShader2));

        // Set up the programs.
        this->ShaderProgram1 = CHECK_GL(glCreateProgram());
        CHECK_GL(glAttachShader(this->ShaderProgram1, this->VertexShader1));
        CHECK_GL(glAttachShader(this->ShaderProgram1, this->FragmentShader1));
        CHECK_GL(glBindAttribLocation(this->ShaderProgram1, 0, "InputPosition"));
        CHECK_GL(glBindAttribLocation(this->ShaderProgram1, 1, "InputNormal"));
        CHECK_GL(glBindAttribLocation(this->ShaderProgram1, 2, "InputColour"));
        CHECK_GL(glBindFragDataLocation(this->ShaderProgram1, 0, "FragmentPosition"));
        CHECK_GL(glBindFragDataLocation(this->ShaderProgram1, 1, "FragmentNormal"));
        CHECK_GL(glBindFragDataLocation(this->ShaderProgram1, 2, "FragmentColour"));
        CHECK_GL(glLinkProgram(this->ShaderProgram1));

        this->ShaderProgram2 = CHECK_GL(glCreateProgram());
        CHECK_GL(glAttachShader(this->ShaderProgram2, this->VertexShader2));
        CHECK_GL(glAttachShader(this->ShaderProgram2, this->FragmentShader2));
        CHECK_GL(glBindFragDataLocation(this->ShaderProgram2, 0, "FragmentPosition"));
        CHECK_GL(glBindFragDataLocation(this->ShaderProgram2, 1, "FragmentNormal"));
        CHECK_GL(glBindFragDataLocation(this->ShaderProgram2, 2, "FragmentColour"));
        CHECK_GL(glLinkProgram(this->ShaderProgram2));

        // Catch any errors.
        GLint ErrorCode;
        CHECK_GL(glGetShaderiv(this->VertexShader1, GL_COMPILE_STATUS, &ErrorCode));
        if (ErrorCode == GL_FALSE) {
            char InfoLogBuffer[1024];
            CHECK_GL(glGetShaderInfoLog(this->VertexShader1, 1024, NULL, InfoLogBuffer));
            std::cerr << "The stage 1 vertex shader failed to compile with the error:" << std::endl << InfoLogBuffer << std::endl;
        }

        CHECK_GL(glGetShaderiv(this->FragmentShader1, GL_COMPILE_STATUS, &ErrorCode));
        if (ErrorCode == GL_FALSE) {
            char InfoLogBuffer[1024];
            CHECK_GL(glGetShaderInfoLog(this->FragmentShader1, 1024, NULL, InfoLogBuffer));
            std::cerr << "The stage 1 fragment shader failed to compile with the error:" << std::endl << InfoLogBuffer << std::endl;
        }

        CHECK_GL(glGetShaderiv(this->VertexShader2, GL_COMPILE_STATUS, &ErrorCode));
        if (ErrorCode == GL_FALSE) {
            char InfoLogBuffer[1024];
            CHECK_GL(glGetShaderInfoLog(this->VertexShader2, 1024, NULL, InfoLogBuffer));
            std::cerr << "The stage 2 vertex shader failed to compile with the error:" << std::endl << InfoLogBuffer << std::endl;
        }

        CHECK_GL(glGetShaderiv(this->FragmentShader2, GL_COMPILE_STATUS, &ErrorCode));
        if (ErrorCode == GL_FALSE) {
            char InfoLogBuffer[1024];
            CHECK_GL(glGetShaderInfoLog(this->FragmentShader2, 1024, NULL, InfoLogBuffer));
            std::cerr << "The stage 2 fragment shader failed to compile with the error:" << std::endl << InfoLogBuffer << std::endl;
        }

        CHECK_GL(glGetProgramiv(this->ShaderProgram1, GL_LINK_STATUS, &ErrorCode));
        if (ErrorCode == GL_FALSE) {
            char InfoLogBuffer[1024];
            CHECK_GL(glGetProgramInfoLog(this->ShaderProgram1, 1024, NULL, InfoLogBuffer));
            std::cerr << "The stage 1 shader program failed to compile with the error:" << std::endl << InfoLogBuffer << std::endl;
        }

        CHECK_GL(glGetProgramiv(this->ShaderProgram2, GL_LINK_STATUS, &ErrorCode));
        if (ErrorCode == GL_FALSE) {
            char InfoLogBuffer[1024];
            CHECK_GL(glGetProgramInfoLog(this->ShaderProgram2, 1024, NULL, InfoLogBuffer));
            std::cerr << "The stage 2 shader program failed to compile with the error:" << std::endl << InfoLogBuffer << std::endl;
        }

        ///////////////////////////////////////////////////////////////////////////
        /// Configure the program, uniforms, framebuffers, and texture.          //
        ///////////////////////////////////////////////////////////////////////////

        this->Projection = Matrix44::Perspective(45.0, static_cast<float>(ScreenWidth) / static_cast<float>(ScreenHeight), 0.01, 1000.0);

        this->View = Matrix44::View(Vector3(-0.25, -0.25, -0.25), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

        this->Model = Matrix44(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );

        this->Subdivisions = 100.0;

        CHECK_GL(glUseProgram(this->ShaderProgram1));

        // Create a framebuffer texture for stage 1.
        CHECK_GL(glGenTextures(1, &this->TexturePosition1));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TexturePosition1));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr));

        CHECK_GL(glGenTextures(1, &this->TextureNormal1));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureNormal1));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr));

        CHECK_GL(glGenTextures(1, &this->TextureColour1));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureColour1));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr));

        CHECK_GL(glGenTextures(1, &this->TextureDepth1));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureDepth1));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, ScreenWidth, ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));

        // Create a framebuffer to store the intermediate stage 1 data.
        CHECK_GL(glGenFramebuffers(1, &this->FrameBuffer1));
        CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, this->FrameBuffer1));

        CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->TexturePosition1, 0));
        CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->TextureNormal1, 0));
        CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->TextureColour1, 0));
        CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->TextureDepth1, 0));

        GLenum DrawBuffers1[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, DrawBuffers1);

        CHECK_GL(GLenum Status1 = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (Status1 != GL_FRAMEBUFFER_COMPLETE) {
            std::abort();
        }

        // Don't think this is required.
        CHECK_GL(glUseProgram(this->ShaderProgram2));

        // Create a framebuffer texture for stage 2.
        CHECK_GL(glGenTextures(1, &this->TexturePosition2));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TexturePosition2));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr));

        CHECK_GL(glGenTextures(1, &this->TextureNormal2));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureNormal2));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr));

        CHECK_GL(glGenTextures(1, &this->TextureColour2));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureColour2));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, nullptr));

        // Create a framebuffer to store the intermediate stage 2 data.
        CHECK_GL(glGenFramebuffers(1, &this->FrameBuffer2));
        CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, this->FrameBuffer2));

        CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->TexturePosition2, 0));
        CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->TextureNormal2, 0));
        CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->TextureColour2, 0));

        GLenum DrawBuffers2[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, DrawBuffers2);

        CHECK_GL(GLenum Status2 = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (Status2 != GL_FRAMEBUFFER_COMPLETE) {
            std::abort();
        }

        ///////////////////////////////////////////////////////////////////////////
        /// Uniforms and texture.                                                //
        ///////////////////////////////////////////////////////////////////////////

        // Don't think this is required.
        CHECK_GL(glUseProgram(this->ShaderProgram1));

        // Stage 1.
        this->ShaderUniformModelViewProjection      = CHECK_GL(glGetUniformLocation(this->ShaderProgram1, "ModelViewProjectionMatrix"));
        this->ShaderUniformModel                    = CHECK_GL(glGetUniformLocation(this->ShaderProgram1, "ModelMatrix"));

        this->ShaderUniformPosition                 = CHECK_GL(glGetAttribLocation(this->ShaderProgram1, "InputPosition"));
        this->ShaderUniformNormal                   = CHECK_GL(glGetAttribLocation(this->ShaderProgram1, "InputNormal"));
        this->ShaderUniformColour                   = CHECK_GL(glGetAttribLocation(this->ShaderProgram1, "InputColour"));

        // Don't think this is required.
        CHECK_GL(glUseProgram(this->ShaderProgram2));

        // Stage 2.
        this->ShaderUniformViewProjectionInverse    = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "ViewProjectionInverseMatrix"));
        this->ShaderUniformEvaluationDirection      = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "EvaluationDirection"));
        this->ShaderUniformVoxelSize                = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "VoxelSize"));
        this->ShaderUniformPixelDimensions          = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "PixelDimensions"));
        this->ShaderUniformLast                     = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "LastRender"));
        this->ShaderUniformSceneOffset              = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "SceneOffset"));

        // Set the samplers.
        const GLint ShaderUniformSamplerPosition2   = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "PositionSampler"));
        CHECK_GL(glUniform1i(ShaderUniformSamplerPosition2, 0));
        const GLint ShaderUniformSamplerNormal2     = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "NormalSampler"));
        CHECK_GL(glUniform1i(ShaderUniformSamplerNormal2, 1));
        const GLint ShaderUniformSamplerColour2     = CHECK_GL(glGetUniformLocation(this->ShaderProgram2, "ColourSampler"));
        CHECK_GL(glUniform1i(ShaderUniformSamplerColour2, 2));

        // Configure OpenGL.
        CHECK_GL(glEnable(GL_DEPTH_TEST));
        CHECK_GL(glDepthFunc(GL_LESS));
        CHECK_GL(glPointSize(1.0f));
        CHECK_GL(glEnable(GL_BLEND));
        CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        // Clear the colour buffer.
        CHECK_GL(glClearColor(0.0, 0.0, 0.0, 0.0));
        CHECK_GL(glClearDepth(0.0));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        CHECK_GL(glGenVertexArrays(1, &this->VertexArray));
        CHECK_GL(glBindVertexArray(this->VertexArray));

        CHECK_GL(glGenBuffers(1, &this->VertexBuffer));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, this->VertexBuffer));

        CHECK_GL(glVertexAttribPointer(this->ShaderUniformPosition, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*) (0 * sizeof(GLfloat))));
        CHECK_GL(glVertexAttribPointer(this->ShaderUniformNormal, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat))));
        CHECK_GL(glVertexAttribPointer(this->ShaderUniformColour, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*) (6 * sizeof(GLfloat))));

        CHECK_GL(glEnableVertexAttribArray(this->ShaderUniformPosition));
        CHECK_GL(glEnableVertexAttribArray(this->ShaderUniformNormal));
        CHECK_GL(glEnableVertexAttribArray(this->ShaderUniformColour));
    }

    void Renderer::Render(const GameState& State) {
        // Move camera.
        this->View = Matrix44::View(Vector3(State.GetCameraPosition()[0], State.GetCameraPosition()[1], State.GetCameraPosition()[2]), Vector3(State.GetCameraTarget()[0], State.GetCameraTarget()[1], State.GetCameraTarget()[2]), Vector3(0.0, 1.0, 0.0));

        // Enable depth testing for the first pass.
        CHECK_GL(glEnable(GL_DEPTH_TEST));
        CHECK_GL(glDisable(GL_BLEND));

        // First pass, render depth-tested points into the first buffer.
        CHECK_GL(glUseProgram(this->ShaderProgram1));
        CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, this->FrameBuffer1));

        // Set clearing parameters.
        CHECK_GL(glClearColor(0.9, 0.9, 0.9, 1.0));
        CHECK_GL(glClearDepth(1.0));

        // Clear the framebuffer.
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // Set the matrices.
        Matrix44 ViewProjection = this->Projection * this->View;
        Matrix44 ModelViewProjection = ViewProjection * this->Model;
        Matrix44 ViewProjectionInverse = Matrix44::Invert(ViewProjection);

        CHECK_GL(glUniformMatrix4fv(this->ShaderUniformModelViewProjection, 1, GL_TRUE, ModelViewProjection.data()));
        CHECK_GL(glUniformMatrix4fv(this->ShaderUniformModel, 1, GL_TRUE, this->Model.data()));

        // Allocate a vertex array and fill it from the voxel volume.
        static std::vector<std::array<GLfloat, 9> > map;

        // Clear the old map, from the previous frame.
        map.clear();

        // Ensure enough memory is reserved for the entire scene.
        map.reserve(State.GetScene().GetSizeZ() * State.GetScene().GetSizeY() * State.GetScene().GetSizeX());

        // Brute force copy voxels to the new map.
        for (std::size_t z = 0; z < State.GetScene().GetSizeZ(); ++z) {
            for (std::size_t y = 0; y < State.GetScene().GetSizeY(); ++y) {
                for (std::size_t x = 0; x < State.GetScene().GetSizeX(); ++x) {
                    const Voxel& v = State.GetScene().operator ()(x, y, z);

                    // Ignore see through voxels.
                    if (v.Alpha > 0) {

                        float Hue = float(v.Hue - uint(4)) / 11.0f;
                        float Saturation = float(v.Saturation) / 3.0f;
                        float Light = float(v.Light) / 15.0f;

                        map.push_back({static_cast<float>(x) / 100.0f, static_cast<float>(y) / 100.0f, static_cast<float>(z) / 100.0f, 1, 0, 0, static_cast<float>(Hue), static_cast<float>(Saturation), static_cast<float>(Light)});
                    }
                }
            }
        }

        // Upload the map.
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, this->VertexBuffer));
        CHECK_GL(glBufferData(GL_ARRAY_BUFFER, map.size() * 9 * sizeof(GLfloat), map.data(), GL_STATIC_DRAW));

        // Initial draw.
        CHECK_GL(glBindVertexArray(this->VertexArray));
        CHECK_GL(glDrawArrays(GL_POINTS, 0, map.size()));

        // Disable depth testing for ping pong passes.
        CHECK_GL(glDisable(GL_DEPTH_TEST));
        CHECK_GL(glDisable(GL_BLEND));

        // Second to the penultimate pass.
        CHECK_GL(glUseProgram(this->ShaderProgram2));

        // Second through N-1 pass, ping-pong render both buffers in turn, spreading the points across the faces of their respective cubes
        // N.B. limit should always be odd?
        for (int i = 0; i < 3; ++i) {
            if (i % 2 == 0) {
                CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, this->FrameBuffer2));

                CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

                CHECK_GL(glActiveTexture(GL_TEXTURE0));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TexturePosition1));

                CHECK_GL(glActiveTexture(GL_TEXTURE1));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureNormal1));

                CHECK_GL(glActiveTexture(GL_TEXTURE2));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureColour1));
            }
            else {
                CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, this->FrameBuffer1));

                CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

                CHECK_GL(glActiveTexture(GL_TEXTURE0));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TexturePosition2));

                CHECK_GL(glActiveTexture(GL_TEXTURE1));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureNormal2));

                CHECK_GL(glActiveTexture(GL_TEXTURE2));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureColour2));
            }

            const GLfloat Direction[2] = { static_cast<float>((i + 1) % 2), static_cast<float>(i % 2) };
            CHECK_GL(glUniform2fv(this->ShaderUniformEvaluationDirection, 1, Direction));

            CHECK_GL(glUniform1f(this->ShaderUniformVoxelSize, 0.5 / (this->Subdivisions)));

            const GLfloat PixelDimensions[2] = { 1.0f / static_cast<float>(ScreenWidth), 1.0f / static_cast<float>(ScreenHeight) };
            CHECK_GL(glUniform2fv(this->ShaderUniformPixelDimensions, 1, PixelDimensions));

            CHECK_GL(glUniformMatrix4fv(this->ShaderUniformViewProjectionInverse, 1, GL_TRUE, ViewProjectionInverse.data()));

            CHECK_GL(glUniform1i(this->ShaderUniformLast, false));

            const GLfloat SceneOffset[3] = { static_cast<float>(State.GetSceneOffset()[0]) / 100.0f, static_cast<float>(State.GetSceneOffset()[1]) / 100.0f, static_cast<float>(State.GetSceneOffset()[2]) / 100.0f };
            CHECK_GL(glUniform3fv(this->ShaderUniformSceneOffset, 1, SceneOffset));

            // Drawing just using one triangle now.
            CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 3));
        }

        // Enable alpha blending for the final pass.
        CHECK_GL(glDisable(GL_DEPTH_TEST));
        CHECK_GL(glEnable(GL_BLEND));

        // Final pass, composite the last buffer to the screen, performing lighting in the process.

        // Bind output framebuffer.
        CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        // Set clearing parameters.
        CHECK_GL(glClearColor(0.306, 0.267, 0.698, 0.0));
        CHECK_GL(glClearDepth(1.0));

        // Clear the framebuffer.
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // Textures
        CHECK_GL(glActiveTexture(GL_TEXTURE0));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TexturePosition2));

        CHECK_GL(glActiveTexture(GL_TEXTURE1));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureNormal2));

        CHECK_GL(glActiveTexture(GL_TEXTURE2));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->TextureColour2));

        const GLfloat Direction[2] = { 0, 1 };
        CHECK_GL(glUniform2fv(this->ShaderUniformEvaluationDirection, 1, Direction));

        CHECK_GL(glUniform1f(this->ShaderUniformVoxelSize, 0.5 / (this->Subdivisions)));

        const GLfloat PixelDimensions[2] = { 1.0f / static_cast<float>(ScreenWidth), 1.0f / static_cast<float>(ScreenHeight) };
        CHECK_GL(glUniform2fv(this->ShaderUniformPixelDimensions, 1, PixelDimensions));

        CHECK_GL(glUniformMatrix4fv(this->ShaderUniformViewProjectionInverse, 1, GL_TRUE, ViewProjectionInverse.data()));

        CHECK_GL(glUniform1i(this->ShaderUniformLast, true));

        const GLfloat SceneOffset[3] = { static_cast<float>(State.GetSceneOffset()[0]) / 100.0f, static_cast<float>(State.GetSceneOffset()[1]) / 100.0f, static_cast<float>(State.GetSceneOffset()[2]) / 100.0f };
        CHECK_GL(glUniform3fv(this->ShaderUniformSceneOffset, 1, SceneOffset));

        // Drawing just using one triangle now.
        CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 3));
    }
}
