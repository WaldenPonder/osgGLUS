#version 430  compatibility

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int idTexture;
layout(location = 2) out vec4 depthTexture;
layout(location = 3) out vec4 linePtTexture;

in vec4 v_color;
flat in int v_id;


//bgfx shaderlib.sh
vec4 packFloatToRgba(float _value)
{
	const vec4 shift = vec4(256 * 256 * 256, 256 * 256, 256, 1.0);
	const vec4 mask = vec4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
	vec4 comp = fract(_value * shift);
	comp -= comp.xxyz * mask;
	return comp;
}

void main()
{
	idTexture = v_id;
	//idTexture = 0; //面的ID无效，为0
	depthTexture = packFloatToRgba(gl_FragCoord.z);
	FragColor = v_color;
	linePtTexture = vec4(0);
	
	//FragColor.a = 0;
	//FragColor = vec4(1,1,0,1);
}
