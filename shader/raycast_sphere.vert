#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;

//uniform mat4 osg_NormalMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform vec3 center;
//uniform mat4 modelViewMat;

//out vec3 normal;
out vec3 pos_;
out vec3 center_;

void main()
{
	//osg_NormalMatrix
   // normal      = (osg_ModelViewMatrix * vec4(aNormal, 1)).xyz;
	pos_ = (osg_ModelViewMatrix * vec4(aPos,1.0f)).xyz;
	center_ = (osg_ModelViewMatrix * vec4(0, 0, 0, 1.0f)).xyz;

    gl_Position = osg_ModelViewProjectionMatrix * vec4(aPos,1.0f);	
}




