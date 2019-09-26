#version 430 
uniform int u_selected_index;

flat in float v_index_start;
flat in float v_index_end;

out vec4 FragColor;

void main()
{
   if( u_selected_index > v_index_start && u_selected_index < v_index_end ) FragColor = vec4(1,1.0,0.0,1);
    else FragColor = vec4(1,.0,0.0,1);
}