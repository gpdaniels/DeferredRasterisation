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

#include "ShaderSource.hpp"

namespace DeferredRasterisation {
    const std::string ShaderSource::VertexShaderSource1 = R"(
        #version 330

        // Uniform parameters.
        uniform mat4 ModelViewProjectionMatrix;
        uniform mat4 ModelMatrix;

        // Input data from vertex buffer.
        layout(location=0) in vec3 InputPosition;
        layout(location=1) in vec3 InputNormal;
        layout(location=2) in vec3 InputColour;

        // Output data to fragment shader.
        out vec3 VertexPosition;
        out vec3 VertexNormal;
        out vec3 VertexColour;

        // Main function copies inputs to outputs, transforming positions using the provided matrices.
        void main() {
            VertexPosition = (ModelMatrix * vec4(InputPosition, 1.0)).xyz;
            VertexNormal = (ModelMatrix * vec4(InputNormal, 0.0)).xyz;
            VertexColour = InputColour;
            gl_Position = ModelViewProjectionMatrix * vec4(InputPosition, 1.0);
        }
    )";

    const std::string ShaderSource::FragmentShaderSource1 = R"(
        #version 330

        // Input data from vertex shader.
        in vec3 VertexPosition;
        in vec3 VertexNormal;
        in vec3 VertexColour;

        // Output data to framebuffer textures.
        layout(location=0) out vec4 FragmentPosition;
        layout(location=1) out vec4 FragmentNormal;
        layout(location=2) out vec4 FragmentColour;

        // Main function copies inputs to outputs, ensuring normals and colours are normalised.
        void main() {
            FragmentPosition = vec4(VertexPosition, 1.0);
            FragmentNormal = vec4(normalize(VertexNormal), 0.0);
            FragmentColour = vec4(normalize(VertexColour), 1.0);
        }
    )";

    const std::string ShaderSource::VertexShaderSource2 = R"(
        #version 330

        // Output data to fragment shader.
        out vec3 VertexPosition;

        // Fullscreen 2D single triangle shader. Only uses vertex ID to generate vertex positions.
        void main() {
            float X = -1.0 + float((gl_VertexID & 1) << 2);
            float Y = -1.0 + float((gl_VertexID & 2) << 1);
            VertexPosition = vec3(X, Y, 1);
            gl_Position = vec4(X, Y, 1, 1);
        }
    )";

    const std::string ShaderSource::FragmentShaderSource2 = R"(
        #version 330

        // Input from bound textures.
        uniform sampler2D PositionSampler;
        uniform sampler2D NormalSampler;
        uniform sampler2D ColourSampler;

        // Uniform parameters.
        uniform mat4 ViewProjectionInverseMatrix;
        uniform vec2 EvaluationDirection;
        uniform float VoxelSize;
        uniform vec2 PixelDimensions;
        uniform bool LastRender;
        uniform vec3 SceneOffset;

        // Output data to framebuffer textures.
        layout(location=0) out vec4 FragmentPosition;
        layout(location=1) out vec4 FramgmentNormal;
        layout(location=2) out vec4 FramgmentColour;

        // Number of neighbours to tap.
        const int TapNeighbourhoodSize = 16;

        // Input data from vertex shader.
        in vec3 VertexPosition;

        // Convert HSL colours into RGB colours.
        vec3 HSL2RGB(in vec3 HSL) {
            vec3 RGB = clamp(abs(mod(HSL.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
            return HSL.z + HSL.y * (RGB - 0.5) * (1.0 - abs(2.0 * HSL.z - 1.0));
        }

        // Hash function to create "random" data from a seed.
        float Hash(float Seed) {
            return fract(sin(Seed) * 43758.5453);
        }

        // Noise function, using hash function to create 3D "random" noise.
        // The noise function returns a value in the range -1.0f -> 1.0f.
        float Noise(vec3 Seed) {
            vec3 FloorSeed = floor(Seed);
            vec3 FractSeed = fract(Seed);

            FractSeed = FractSeed * FractSeed * (3.0 - 2.0 * FractSeed);

            float BaseSeed = FloorSeed.x + FloorSeed.y * 57.0 + 113.0 * FloorSeed.z;

            return mix(mix(mix(Hash(BaseSeed + 0.0), Hash(BaseSeed + 1.0), FractSeed.x),
                       mix(Hash(BaseSeed + 57.0), Hash(BaseSeed + 58.0), FractSeed.x), FractSeed.y),
                       mix(mix(Hash(BaseSeed + 113.0), Hash(BaseSeed + 114.0), FractSeed.x),
                       mix(Hash(BaseSeed + 170.0), Hash(BaseSeed + 171.0), FractSeed.x), FractSeed.y), FractSeed.z);
        }

        // AABB intersection function to determine if the ray passes through the box and at what depth.
        vec2 RayBoxIntersect(in vec3 RayOrigin, in vec3 RayDirection, in vec3 BoxMin, in vec3 BoxMax) {

            vec3 InverseRayDirection = 1.0 / RayDirection;
            vec3 OriginToBoxMinimumVector = InverseRayDirection * (BoxMin - RayOrigin);
            vec3 OriginToBoxMaximumVector = InverseRayDirection * (BoxMax - RayOrigin);

            vec3 MaximumVector = max(OriginToBoxMaximumVector, OriginToBoxMinimumVector);
            vec3 MinimumVector = min(OriginToBoxMaximumVector, OriginToBoxMinimumVector);

            vec2 MaxOfMin = max(MinimumVector.xx, MinimumVector.yz);
            vec2 MinOfMax = min(MaximumVector.xx, MaximumVector.yz);

            float BackIntersectionDepth = min(MinOfMax.x, MinOfMax.y);
            float IntersectionDepth = max(MaxOfMin.x, MaxOfMin.y);

            // Hit if IntersectionDepth < BackIntersectionDepth;
            return vec2(IntersectionDepth, BackIntersectionDepth);
        }

        // Hemisphere lighting function.
        vec3 HemisphereLighting(vec3 Normal) {
            const vec3 LightPosition = vec3(0.1, -1.0, 0.0);
            float NdotL = dot(Normal, LightPosition) * 0.5 + 0.5;
            return mix(vec3(0.886, 0.757, 0.337), vec3(0.518, 0.169, 0.0), NdotL);
        }

        // Main deferred rasterisation shader function.
        void main() {
            vec4 EyePosition = ViewProjectionInverseMatrix * vec4(0.0, 0.0, -1.0, 1.0);
            EyePosition.xyz /= EyePosition.w;

            vec4 ScreenPosition = ViewProjectionInverseMatrix * vec4(VertexPosition, 1.0);
            ScreenPosition.xyz /= ScreenPosition.w;

            vec3 EyeVector = normalize(ScreenPosition.xyz - EyePosition.xyz);

            // Get the central data coordinate.
            vec2 DataCoordinate = VertexPosition.xy * 0.5 + 0.5;

            // Prepare output variables.
            vec3 OutputPosition = vec3(0.0, 0.0, 0.0);
            vec3 OutputNormal = vec3(0.0, 0.0, 0.0);
            vec3 OutputColour = vec3(0.0, 0.0, 0.0);

            // Save the best intersection depth.
            float BestDepth = 9999999.0;

            for (int CurrentTap = -TapNeighbourhoodSize; CurrentTap <= TapNeighbourhoodSize; ++CurrentTap) {

                vec2 TappedCoordinate = DataCoordinate + vec2(CurrentTap) * EvaluationDirection * PixelDimensions;
                vec3 Position = texture(PositionSampler, TappedCoordinate).xyz;

                // The intersect test returns a two channel result, the x component is the front intersection and the y component the back.
                vec2 IntersectionDepths = RayBoxIntersect(EyePosition.xyz, EyeVector, Position - VoxelSize, Position + VoxelSize);

                // If the front intersection depth is less than the back then the object was hit.
                if (IntersectionDepths.x <= IntersectionDepths.y) {
                    if (IntersectionDepths.x <= BestDepth) {
                        BestDepth = IntersectionDepths.x;
                        OutputPosition = Position;
                        OutputNormal = texture(NormalSampler, TappedCoordinate).xyz;
                        OutputColour = texture(ColourSampler, TappedCoordinate).xyz;
                    }
                }
            }

            if (LastRender && BestDepth < 9999999.0) {
                // Last pass output colour in the first channel.
                FragmentPosition = vec4(mix(HemisphereLighting(OutputNormal), HSL2RGB(OutputColour), vec3(0.5, 0.5, 0.5)), 1.0);
                // Add some colour noise based on position.
                FragmentPosition = mix(vec4(Noise((OutputPosition + SceneOffset) * 100.0)), FragmentPosition, vec4(0.9, 0.9, 0.9, 1.0));
                // TODO: Should output gl_FragDepth
            } else {
                FragmentPosition = vec4(OutputPosition, 0.0);
                FramgmentNormal = vec4(OutputNormal, 0.0);
                FramgmentColour = vec4(OutputColour, 0.0);
            }
        }
    )";
}
