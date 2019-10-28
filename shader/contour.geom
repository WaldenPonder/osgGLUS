#version 430 core
layout(lines) in;
layout(line_strip, max_vertices = 2) out;
//in int vertexID;
in VS_OUT {
    int vertexID;
} gs_in[];

void main() 
{   

/*     if(gs_in[0].vertexID % 2 != 0)
    {
        EndPrimitive();
        return;
    } */
    //if(gs_in[0].vertexID % 3 == 0)
    {
        gl_Position = gl_in[0].gl_Position; 
        EmitVertex(); 
        
        //gl_Position = gl_in[0].gl_Position; 
        EmitVertex();
        //EndPrimitive();
    }

    //if((gs_in[0].vertexID + 1) % 3 != 0)
    {
        //gl_Position = gl_in[1].gl_Position;
     //   EmitVertex();
    }
    
}