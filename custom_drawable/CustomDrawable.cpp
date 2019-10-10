#include "pch.h"
#include "CustomDrawable.h"
#include <osg/GLExtensions>
#include <iostream>
#include <assert.h>
using namespace std;

const char* vert_src =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"uniform mat4 u_mat;"
"out vec3 color;\n"
"void main()\n"
"{"
	"color = aColor;"
	"gl_Position =  u_mat * vec4(aPos, 1.0);"
"}";

const char* frag_src =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 color;"
"void main()\n"
"{\n"
	"FragColor = vec4(color, 1);\n"
"}";

CustomDrawable::CustomDrawable()
{
	arr_ = new osg::Vec3Array;
	arr_->push_back(osg::Vec3(0.5f, 0.5f, 0.0f));    arr_->push_back(osg::Vec3(1, 0, 0));
	arr_->push_back(osg::Vec3(0.5f, -0.5f, 0.0f));   arr_->push_back(osg::Vec3(0, 1, 0));
	arr_->push_back(osg::Vec3(-0.5f, -0.5f, 0.0f));  arr_->push_back(osg::Vec3(0, 0, 1));
}

void CustomDrawable::drawImplementation(osg::RenderInfo& info) const
{
	const char* version = (const char*)glGetString(GL_VERSION);
	//cout << "AAA " << version << endl;

	osg::GLExtensions& ext = *info.getState()->get<osg::GLExtensions>();

	complie(info);
	osg::Camera* camera = info.getView()->getCamera();

	//---------------------draw-----------------------------
	//glClearColor(.3f, .7f, .5f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int loc = ext.glGetUniformLocation(program_, "u_mat");
	osg::Matrixf mat = camera->getViewMatrix() * camera->getProjectionMatrix();
	ext.glUniformMatrix4fv(loc, 1, GL_FALSE, mat.ptr());

	ext.glUseProgram(program_);
	ext.glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

osg::BoundingBox CustomDrawable::computeBoundingBox() const
{
	osg::BoundingBox bb;

	if (arr_)
	{
		for (auto& p : arr_->asVector())
		{
			bb.expandBy(p);
		}
	}

	return bb;
}

void CustomDrawable::complie(osg::RenderInfo& info) const
{
	if (!dirty_) return;
	dirty_ = false;

	osg::GLExtensions& ext =  *info.getState()->get<osg::GLExtensions>();

	//---------------shader-----------------------------------
	int vert_shader = ext.glCreateShader(GL_VERTEX_SHADER);
	{
		ext.glShaderSource(vert_shader, 1, &vert_src, NULL);
		ext.glCompileShader(vert_shader);
		int success;
		char infoLog[512];
		ext.glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			ext.glGetShaderInfoLog(vert_shader, 512, NULL, infoLog);

			cout << infoLog << "  GL_VERTEX_SHADER  ERROR\n";
			return;
		}
	}

	int frag_shader = ext.glCreateShader(GL_FRAGMENT_SHADER);
	{
		ext.glShaderSource(frag_shader, 1, &frag_src, NULL);
		ext.glCompileShader(frag_shader);
		int success;
		char infoLog[512];

		ext.glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			ext.glGetShaderInfoLog(frag_shader, 512, NULL, infoLog);

			cout << infoLog << "   GL_FRAGMENT_SHADER  ERROR\n";
			return;
		}
	}

	program_ = ext.glCreateProgram();
	ext.glAttachShader(program_, vert_shader);
	ext.glAttachShader(program_, frag_shader);
	ext.glLinkProgram(program_);

	int success;
	char infoLog[512];
	ext.glGetProgramiv(program_, GL_LINK_STATUS, &success);
	if (!success)
	{
		ext.glGetProgramInfoLog(program_, 512, NULL, infoLog);
		cout << infoLog << "GL_LINK_STATUS  \n";
	}

	ext.glDeleteShader(vert_shader);
	ext.glDeleteShader(frag_shader);

	//------------------VAO VBO--------------------------------	
	ext.glGenVertexArrays(1, &vao_);
	ext.glGenBuffers(1, &vbo_);
	
	ext.glBindVertexArray(vao_);
	ext.glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_);
	ext.glBufferData(GL_ARRAY_BUFFER_ARB, arr_->getTotalDataSize(), arr_->getDataPointer(), GL_STATIC_DRAW_ARB);
	int loc = ext.glGetAttribLocation(program_, "aPos");
	ext.glEnableVertexAttribArray(loc);
	ext.glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 2 * arr_->getElementSize(), (void*)0);
	
	assert(arr_->getElementSize() == 3 * sizeof(FLOAT));

	loc = ext.glGetAttribLocation(program_, "aColor");
	ext.glEnableVertexAttribArray(loc);
	ext.glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 2 * arr_->getElementSize(), (void*)(arr_->getElementSize()));
}
