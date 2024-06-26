#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define MAX_NUM_LIGHTS_PER_TILE 1024

// output write
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;
layout(location = 3) in vec3 worldPos;
layout(location = 4) in flat uint material;
layout(location = 5) in mat3 TBN;

layout(binding = 2) uniform sampler2D textures[];

struct Material {
    vec4 color;
    uint texColor;
    uint texRoughMet;
    uint texNormal;
    uint pad;
};

struct LightPoint {
    vec4 posAndIntensity;
    vec3 color;
    float radius;
};

layout(binding = 1, std430) readonly buffer Materials { Material materials[]; }
materials[];

layout(binding = 1, std430) readonly buffer Lights { LightPoint lights[]; }
lights[];

layout(binding = 1) buffer LightIndex { int lightIndex[]; }
lightIndex[];

layout(push_constant) uniform constants {
    mat4 projView;

    vec4 cameraPosition;

    uint lightsBind;
    uint drawParamsBind;
    uint materialsBind;
    uint lightIndexBind;

    uint numberOfTilesX;
};

#define RECIPROCAL_PI 0.3183098861837907
#define RECIPROCAL_2PI 0.15915494309189535

vec3 rgb2lin(vec3 rgb) {  // sRGB to linear approximation
    return pow(rgb, vec3(2.2));
}

vec3 lin2rgb(vec3 lin) {  // linear to sRGB approximation
    return pow(lin, vec3(1.0 / 2.2));
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float NoH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float b = (NoH2 * (alpha2 - 1.0) + 1.0);
    return alpha2 * RECIPROCAL_PI / (b * b);
}

float G1_GGX_Schlick(float NoV, float roughness) {
    float alpha = roughness * roughness;
    float k = alpha / 2.0;
    return max(NoV, 0.001) / (NoV * (1.0 - k) + k);
}

float G_Smith(float NoV, float NoL, float roughness) {
    return G1_GGX_Schlick(NoL, roughness) * G1_GGX_Schlick(NoV, roughness);
}

float fresnelSchlick90(float cosTheta, float F0, float F90) {
    return F0 + (F90 - F0) * pow(1.0 - cosTheta, 5.0);
}

float disneyDiffuseFactor(float NoV, float NoL, float VoH, float roughness) {
    float alpha = roughness * roughness;
    float F90 = 0.5 + 2.0 * alpha * VoH * VoH;
    float F_in = fresnelSchlick90(NoL, 1.0, F90);
    float F_out = fresnelSchlick90(NoV, 1.0, F90);
    return F_in * F_out;
}

vec3 brdfMicrofacet(in vec3 L, in vec3 V, in vec3 N, in float metallic,
                    in float roughness, in vec3 baseColor,
                    in float reflectance) {
    vec3 H = normalize(V + L);

    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);

    vec3 f0 = vec3(0.16 * (reflectance * reflectance));
    f0 = mix(f0, baseColor, metallic);

    vec3 F = fresnelSchlick(VoH, f0);
    float D = D_GGX(NoH, roughness);
    float G = G_Smith(NoV, NoL, roughness);

    vec3 spec = (F * D * G) / (4.0 * max(NoV, 0.001) * max(NoL, 0.001));

    vec3 rhoD = baseColor;

    // optionally
    rhoD *= vec3(1.0) - F;
    // rhoD *= disneyDiffuseFactor(NoV, NoL, VoH, roughness);

    rhoD *= (1.0 - metallic);

    vec3 diff = rhoD * RECIPROCAL_PI;

    return diff + spec;
}

void main() {
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * numberOfTilesX + tileID.x;

    Material m = materials[materialsBind].materials[material];

    vec3 baseCol = texture(textures[m.texColor], uv).xyz;
    float roughness = texture(textures[m.texRoughMet], uv).g;
    float metallic = texture(textures[m.texRoughMet], uv).r;

    vec3 n = texture(textures[m.texNormal], uv).rgb;
    n = n * 2.0 - 1.0;
    n = normalize(TBN * n);

    vec3 viewDir = normalize(cameraPosition.xyz - worldPos);

    vec3 radiance = vec3(0);
    uint offset = index * MAX_NUM_LIGHTS_PER_TILE;
    for (int i = 0; i < MAX_NUM_LIGHTS_PER_TILE &&
                    lightIndex[lightIndexBind].lightIndex[offset + i] != -1;
         i++) {
        LightPoint light =
            lights[lightsBind]
                .lights[lightIndex[lightIndexBind].lightIndex[offset + i]];

        float dist = length(light.posAndIntensity.xyz - worldPos);

        vec3 lightDir = normalize(light.posAndIntensity.xyz - worldPos);

        float reflectance = 0.5;
        float attenuation = smoothstep(light.radius, 0, dist);
        float irradiPerp = light.posAndIntensity.w * attenuation;

        float irradiance = max(dot(lightDir, n), 0.0) * irradiPerp;

        if (irradiance > 0.0) {
            vec3 brdf = brdfMicrofacet(lightDir, viewDir, n, metallic,
                                       roughness, baseCol, reflectance);
            radiance += brdf * irradiance * light.color;
        }
    }

    outColor.rgb = radiance;
    outColor.a = 1.0;
}
