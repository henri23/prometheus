#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;

void main() {
    // Simple color interpolation based on screen position
    vec2 uv = gl_FragCoord.xy / vec2(800.0, 600.0); // Normalize to [0,1]

    // Interpolate between catppuccin-style colors
    vec3 color1 = vec3(0.137, 0.176, 0.227); // Dark blue #24283B
    vec3 color2 = vec3(0.243, 0.145, 0.384); // Purple #3E2562
    vec3 color3 = vec3(0.384, 0.145, 0.192); // Pink #622531

    // Create smooth gradient based on position
    vec3 final_color = mix(color1, color2, uv.x);
    final_color = mix(final_color, color3, uv.y * 0.5);

    out_colour = vec4(final_color, 1.0);
}
