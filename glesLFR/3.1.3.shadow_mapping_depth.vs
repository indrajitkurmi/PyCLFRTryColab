#version 310 es
#extension GL_EXT_shader_io_blocks : enable
precision highp float;
precision highp int;
precision mediump image2DArray;


layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}