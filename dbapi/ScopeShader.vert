#version 440

layout(location = 0) in vec4 vertices;
layout(location = 0) out vec2 coords;

//layout(std140, binding = 0) uniform buf {
//    float t;
//    float y_dir;
//};

void main()
{
//    gl_Position = vertices;
//    coords = vertices.xy;
//    coords.y *= 1;

    coords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(coords * 2.0 - 1.0, 0.0, 1.0);
}
