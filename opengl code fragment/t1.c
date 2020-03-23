//https://www.cnblogs.com/hellobb/p/8891374.html

glGenBuffers(2, textureInstancingPBO);
glBindBuffer(GL_PIXEL_UNPACK_BUFFER, textureInstancingPBO[0]); <br><br>//GL_STREAM_DRAW_ARB means that we will change data every frame
glBufferData(GL_PIXEL_UNPACK_BUFFER, INSTANCES_DATA_SIZE, 0, GL_STREAM_DRAW_ARB);
glBindBuffer(GL_PIXEL_UNPACK_BUFFER, textureInstancingPBO[1]);
glBufferData(GL_PIXEL_UNPACK_BUFFER, INSTANCES_DATA_SIZE, 0, GL_STREAM_DRAW_ARB);
glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);<br>
//create texture where we will store instances data on gpu
glGenTextures(1, textureInstancingDataTex);
glBindTexture(GL_TEXTURE_2D, textureInstancingDataTex);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT); //in each line we store NUM_INSTANCES_PER_LINE object's data. 128 in our case<br>
//for each object we store PER_INSTANCE_DATA_VECTORS data-vectors. 2 in our case
//GL_RGBA32F, we have float32 data
//complex_mesh_instances_data source data of instances, if we are not going to update data in the texture
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, NUM_INSTANCES_PER_LINE * PER_INSTANCE_DATA_VECTORS, MAX_INSTANCES / NUM_INSTANCES_PER_LINE, 0, GL_RGBA, GL_FLOAT, &complex_mesh_instances_data[0]);
glBindTexture(GL_TEXTURE_2D, 0);

///////////////////////////////////////////////////////////////
glBindTexture(GL_TEXTURE_2D, textureInstancingDataTex);
glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, textureInstancingPBO[current_frame_index]); // copy pixels from PBO to texture object
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NUM_INSTANCES_PER_LINE * PER_INSTANCE_DATA_VECTORS, MAX_INSTANCES / NUM_INSTANCES_PER_LINE, GL_RGBA, GL_FLOAT, 0); // bind PBO to update pixel values
glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, textureInstancingPBO[next_frame_index]);<br>
//http://www.songho.ca/opengl/gl_pbo.html
// Note that glMapBufferARB() causes sync issue.
// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
// until GPU to finish its job. To avoid waiting (idle), you can call
// first glBufferDataARB() with NULL pointer before glMapBufferARB().
// If you do that, the previous data in PBO will be discarded and
// glMapBufferARB() returns a new allocated pointer immediately
// even if GPU is still working with the previous data.
glBufferData(GL_PIXEL_UNPACK_BUFFER, INSTANCES_DATA_SIZE, 0, GL_STREAM_DRAW_ARB);
gpu_data = (float*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY_ARB);
if (gpu_data)
{
　　memcpy(gpu_data, complex_mesh_instances_data[0], INSTANCES_DATA_SIZE); // update data
　　glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); //release pointer to mapping buffer
}

///////////////////////////////////////////////////////////////
//使用texture instancing渲染：
//bind texture with instances data
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, textureInstancingDataTex);
glBindSampler(0, Sampler_nearest);
glBindVertexArray(geometry_vao_id); //what geometry to render
tex_instancing_shader.bind(); //with what shader<br>
//tell shader texture with data located, what name it has
static GLint location = glGetUniformLocation(tex_instancing_shader.programm_id, s_texture_0);
if (location >= 0)
　　glUniform1i(location, 0); //render group of objects
glDrawElementsInstanced(GL_TRIANGLES, BOX_NUM_INDICES, GL_UNSIGNED_INT, NULL, CURRENT_NUM_INSTANCES);


///////////////////////////////////////////////////////////////
//顶点着色器示例：
version 150 core
in vec3 s_pos;
in vec3 s_normal;
in vec2 s_uv;
uniform mat4 ModelViewProjectionMatrix;
 
uniform sampler2D s_texture_0;
out vec2 uv;
out vec3 instance_color;
void main()
{
　　const vec2 texel_size = vec2(1.0 / 256.0, 1.0 / 16.0);
　　const int objects_per_row = 128;
　　const vec2 half_texel = vec2(0.5, 0.5);<br>
　　//calc texture coordinates - where our instance data located
　　//gl_InstanceID % objects_per_row - index of object in the line
　　//multiple by 2 as each object has 2 vectors of data
　　//gl_InstanceID / objects_per_row - in what line our data located
　　//multiple by texel_size gieves us 0..1 uv to sample from texture from interer texel id
　　vec2 texel_uv = (vec2((gl_InstanceID % objects_per_row) * 2, floor(gl_InstanceID / objects_per_row)) + half_texel) * texel_size;
　　vec4 instance_pos = textureLod(s_texture_0, texel_uv, 0);
　　instance_color = textureLod(s_texture_0, texel_uv + vec2(texel_size.x, 0.0), 0).xyz;
　　uv = s_uv;
　　gl_Position = ModelViewProjectionMatrix * vec4(s_pos + instance_pos.xyz, 1.0);
}

