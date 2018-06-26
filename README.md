
# Deferred Rasterisation #
-------------

This repository aims to demonstrate a GLSL deferred renderer that expands GL_POINTS into cube voxels.

| Coloured blocks:  | Moving in the volume: |
|--|--|
|![Image of coloured blocks in the volume](https://raw.githubusercontent.com/gpdaniels/DeferredRasterisation/master/Data/SampleSmall.png "Image of coloured blocks in the volume") |![Animation of moving in the volume](https://raw.githubusercontent.com/gpdaniels/DeferredRasterisation/master/Data/SampleSmall.gif "Animation of moving in the volume") |


## Scene ##

On load a voxel volume is generated consisting of:
- A grey bottom floor layer.
- Two green "grass" layers.
- A number of blue columns that are randomly placed.
- A set of six coloured blocks.
- A red sphere.

The whole scene is volumetric and can be changed very easily in the code.

## Controls ##

Use the arrow keys to move around.

There is a day/night cycle that occurs about once a minute.

## Inspriation ##

This project was inspired by the deferred rasterisation example here:
https://github.com/swiftcoder/isosurface

Which itself was inspired by the rendering technology in [VoxelQuest](https://www.voxelquest.com/).

## License ##

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
THE SOFTWARE.
