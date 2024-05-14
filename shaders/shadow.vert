layout (location=0) in vec3 pos;

layout(std430, buffer_reference) readonly buffer PerFrame {
    mat4 light;
};

layout(std430, buffer_reference) readonly buffer PerObject {
    mat4 model;
};

layout(push_constant) uniform constants
{
    PerFrame perFrame;
    PerObject perObject;
} pc;

void main() {
    mat4 light = pc.perFrame.light;
    mat4 model = pc.perObject.model;
    gl_Position = light * model * vec4(pos, 1.0);
}