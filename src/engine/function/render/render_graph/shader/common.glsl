#extension GL_EXT_nonuniform_qualifier : enable

#define BindlessDescriptorSet 0

#define BindlessUniformBinding 0
#define BindlessStorageBinding 1
#define BindlessSamplerBinding 2

#define GetLayoutVariableName(Name) u##Name##Register

// Access a specific resource
#define GetResource(Name, Index) \
    GetLayoutVariableName(Name)[Index]

layout(set = BindlessDescriptorSet, binding = BindlessSamplerBinding)
    uniform sampler2D texture2Ds[];

layout(set = BindlessDescriptorSet, binding = BindlessSamplerBinding)
    uniform sampler3D texture3Ds[];

#define Handle uint

const float gamma = 2.2;
const float exposure = 1.0;

vec3 linearToSrgb(vec3 v)
{
    return pow(v, vec3(1.0 / gamma));
}

vec3 srgbToLinear(vec3 v)
{
    return pow(v, vec3(gamma));
}

bool selectPixel(int i, int j, vec4 GL_FragCoord)
{
    return GL_FragCoord.x > i && GL_FragCoord.y > j && GL_FragCoord.x <= i + 1 && GL_FragCoord.y <= j + 1;
}
