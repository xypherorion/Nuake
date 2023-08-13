#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;
layout(location = 5) in ivec4 BoneIDs;
layout(location = 6) in vec4 Weights;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

const int MAX_BONES = 100;
const int MAX_BONES_INFLUENCE = 4;
uniform mat4 u_FinalBonesMatrice[MAX_BONES];

out vec3 FragPos;
out mat3 TBN;
out vec2 UV;

void main()
{
    vec3 T = normalize((u_Model * vec4(Tangent, 0.0f)).xyz);
    vec3 N = normalize((u_Model * vec4(Normal, 0.0f)).xyz);
    vec3 B = normalize((u_Model * vec4(Bitangent, 0.0f)).xyz);
    TBN = mat3(T, B, N);

    UV = UVPosition;

    vec4 totalPosition = vec4(0.0f);
    for (int i = 0; i < MAX_BONES_INFLUENCE; i++)
    {
        if (BonesIDs[i] == -1)
        {
            continue;
        }

        if (BonesIDs[i] >= MAX_BONES)
        {
            totalPosition = vec4(VertexPosition, 1.0f);
        }

        vec4 localPositon = u_FinalBonesMatrice[BoneIDs[i]] * vec4(VertexPosition, 1.0f);
        totalPosition += localPosition * Weights[i];
        // vec3 localNormal = mat3(u_FinalBonesMatrice[BoneIDs[i]]) * Normal;
    }

    gl_Position = u_Projection * u_View * u_Model * vec4(totalPosition, 1.0f);
}

#shader fragment
#version 460 core

layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gMaterial;
layout(location = 3) out int gEntityID;

in vec3 FragPos;
in vec2 UV;
in mat3 TBN;

// Material
uniform sampler2D m_Albedo;
uniform sampler2D m_Metalness;
uniform sampler2D m_Roughness;
uniform sampler2D m_AO;
uniform sampler2D m_Normal;
uniform sampler2D m_Displacement;
uniform int u_EntityID;

layout(std140, binding = 32) uniform u_MaterialUniform
{
    int u_HasAlbedo;
    vec3 m_AlbedoColor;     //  16 byte
    int u_HasMetalness;     //  32 byte
    float u_MetalnessValue; //  36 byte
    int u_HasRoughness;     //  40 byte
    float u_RoughnessValue; //  44 byte
    int u_HasAO;            //  48 byte
    float u_AOValue;        //  52 byte
    int u_HasNormal;        //  56 byte
    int u_HasDisplacement;  //  60 byte
    int u_Unlit;
};

void main()
{
    // Normal
    vec3 normal = vec3(0.5, 0.5, 1.0);
    if (u_HasNormal == 1)
        normal = texture(m_Normal, UV).rgb;
    normal = normal * 2.0 - 1.0;
    normal = TBN * normalize(normal);
    gNormal = vec4(normal / 2.0 + 0.5, 1.0);

    // Albedo
    gAlbedo = vec4(m_AlbedoColor, 1.0);
    if (u_HasAlbedo == 1)
    {
        vec4 albedoSample = texture(m_Albedo, UV);
        gAlbedo.rgb = albedoSample.rgb * m_AlbedoColor.rgb;
        gAlbedo.a = albedoSample.a;

        if (albedoSample.a < 0.1f)
        {
            discard;
        }
    }

    // Material
    float finalMetalness = u_MetalnessValue;
    if (u_HasMetalness == 1)
        finalMetalness = texture(m_Metalness, UV).r;

    float finalAO = u_AOValue;
    if (u_HasAO == 1)
        finalAO = texture(m_AO, UV).r;

    float finalRoughness = u_RoughnessValue;
    if (u_HasRoughness == 1)
        finalRoughness = texture(m_Roughness, UV).r;

    gMaterial = vec4(0, 0, 0, 1);
    gMaterial.r = finalMetalness;
    gMaterial.g = finalAO;
    gMaterial.b = finalRoughness;

    gEntityID = int(u_EntityID);
}