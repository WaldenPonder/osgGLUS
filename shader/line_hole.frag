#version 430 

flat in vec3 g_id;
flat in vec4 v_linePt;
in vec3 g_color;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 idTexture;
layout(location = 2) out vec4 depthTexture;
layout(location = 3) out vec4 startPtTexture;
//layout(location = 4) out vec4 endPtTexture;

void main()
{
   FragColor = vec4(g_color, 1);
   idTexture = vec4(g_id, 1);
   depthTexture = vec4(vec3(gl_FragCoord.z), 1);
   startPtTexture = v_linePt;
}
