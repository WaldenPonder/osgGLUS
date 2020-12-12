#version 430 

flat in vec3 v_id;
in vec3 v_color;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 idTexture;

void main()
{
   FragColor = vec4(v_color, 1);
   idTexture = vec4(v_id, 1);
}
