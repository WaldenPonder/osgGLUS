#version 430 

flat in vec3 v_color;
out vec4 FragColor;

void main()
{
   FragColor = vec4(v_color, 1.);
/*
   if( u_selected_index > v_index_start && u_selected_index < v_index_end ) FragColor = vec4(1,1.0,0.0,1);
    else FragColor = vec4(1,.0,0.0,1);
	*/
}