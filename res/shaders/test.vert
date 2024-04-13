#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier: require

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;
layout (location = 3) in vec3 vColor;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

struct Vertex {

	vec3 position;
	vec3 normal;
	vec2 uv;
	vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

struct Instance {
	mat4 transform;
};

layout(std430, set = 0, binding = 1) readonly buffer InstanceBuffer{ 
	Instance instances[];
} storages[];

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 projView;
	uint instanceId;
} PushConstants;

void main()
{
	Instance instance = storages[PushConstants.instanceId].instances[gl_InstanceIndex];
	gl_Position = PushConstants.projView * instance.transform * vec4(vPosition, 1.0f);
	outColor = vColor;
	outUV = vUV;
}

