#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier: require

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;
layout (location = 3) in vec3 vColor;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;


//push constants block
layout( push_constant ) uniform constants
{	
	mat4 projView;
} PushConstants;

void main()
{
	gl_Position = PushConstants.projView * vec4(vPosition, 1.0f);
	outColor = vColor;
	outUV = vUV;
}

