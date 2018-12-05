#version 450 core

layout(location = 0) in vec2 coord;
out vec4 fColor;

const float X_SCALE = 1.0;
const float Y_SCALE = 1.0;

void main()
{
	gl_Position = vec4((coord.x * 2.0 - 1.0) * X_SCALE, (coord.y + 0.0) * Y_SCALE, 0, 1);
	fColor = vec4(abs(coord.y * Y_SCALE), 1.0 - abs(coord.y * Y_SCALE), 0, 1);
}