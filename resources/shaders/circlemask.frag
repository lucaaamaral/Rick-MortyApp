#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};

layout(binding = 1) uniform sampler2D source;

void main() {
    vec2 uv = qt_TexCoord0 - vec2(0.5);
    float dist = length(uv);
    float alpha = 1.0 - smoothstep(0.47, 0.5, dist);
    vec4 tex = texture(source, qt_TexCoord0);
    fragColor = tex * alpha * qt_Opacity;
}
