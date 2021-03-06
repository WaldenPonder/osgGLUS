#version 430  compatibility

flat in uint g_id;
flat in vec4 v_linePt;
in vec3 g_color;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uint idTexture;
layout(location = 2) out vec4 depthTexture;
layout(location = 3) out vec4 linePtTexture;

void main()
{
   FragColor = vec4(g_color, 1);
   idTexture = g_id;
   depthTexture = vec4(vec3(gl_FragCoord.z), 1);
   linePtTexture = v_linePt;
}
