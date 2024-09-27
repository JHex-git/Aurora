#version 330 core
layout (lines) in;
layout (line_strip, max_vertices=16) out;

in vec3 vPosition[];

uniform mat4 uViewProjection;

void main()
{
    vec4 position_min = vec4(vPosition[0], 1.0);
    gl_Position = uViewProjection * position_min;
    EmitVertex();
    
    vec4 position_max = vec4(vPosition[1], 1.0);
    gl_Position = uViewProjection * vec4(position_max.x, position_min.yzw);
    EmitVertex();

    gl_Position = uViewProjection * vec4(position_max.x, position_min.y, position_max.zw);
    EmitVertex();
    
    gl_Position = uViewProjection * vec4(position_min.x, position_min.y, position_max.zw);
    EmitVertex();
    
    gl_Position = uViewProjection * position_min;
    EmitVertex();

    gl_Position = uViewProjection * vec4(position_min.x, position_max.y, position_min.zw);
    EmitVertex();
    
    gl_Position = uViewProjection * vec4(position_max.x, position_max.y, position_min.zw);
    EmitVertex();

    gl_Position = uViewProjection * position_max;
    EmitVertex();
    
    gl_Position = uViewProjection * vec4(position_min.x, position_max.y, position_max.zw);
    EmitVertex();
    
    gl_Position = uViewProjection * vec4(position_min.x, position_max.y, position_min.zw);
    EmitVertex();
    EndPrimitive();

    gl_Position = uViewProjection * vec4(position_max.x, position_min.yzw);
    EmitVertex();
    gl_Position = uViewProjection * vec4(position_max.xy, position_min.zw);
    EmitVertex();
    EndPrimitive();

    gl_Position = uViewProjection * vec4(position_max.x, position_min.y, position_max.zw);
    EmitVertex();
    gl_Position = uViewProjection * position_max;
    EmitVertex();
    EndPrimitive();

    gl_Position = uViewProjection * vec4(position_min.xy, position_max.zw);
    EmitVertex();
    gl_Position = uViewProjection * vec4(position_min.x, position_max.yzw);
    EmitVertex();
    EndPrimitive();
}