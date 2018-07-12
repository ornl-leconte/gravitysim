# gravitysim

3D gravity simulator

## Requirements

OpenGL, GLM, GLEW, GLFW

Optionally:

OpenCL (for faster physics computation)

CUDA (for faster physics computation) (use `cmake -DENABLE_CUDA=ON`)


## Maths

Here are some articles I used to implement rendering techniques, physics, etc:

  - [GPU-Based Ray-Casting of Quadratic Surfaces](http://reality.cs.ucl.ac.uk/projects/quadrics/pbg06.pdf)


## Running Notes

On some platforms other than apple, you may need to force OpenGL context:

For example, on MESA open source Linux stuff, use: ` export MESA_GL_VERSION_OVERRIDE=3.3 `

