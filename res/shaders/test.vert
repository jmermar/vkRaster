#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

struct Vertex {

	vec3 position;
	vec3 normal;
	vec2 uv;
	vec4 color;
}; 

struct Instance {
	mat4 transform;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

layout(buffer_reference, std430) readonly buffer InstanceBuffer{ 
	Instance instances[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 projView;
	VertexBuffer vertexBuffer;
	InstanceBuffer instanceBuffer;
} PushConstants;

void main()
{
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	Instance i = PushConstants.instanceBuffer.instances[gl_InstanceIndex];
	gl_Position = PushConstants.projView * i.transform * vec4(v.position, 1.0f);
	outColor = v.color.xyz;
	outUV.x = v.uv.x;
	outUV.y = v.uv.y;
}

