#define PI 3.14

/*************************************************************************/
// 物体表面材质
struct Material
{
	float4 DiffuseAlbedo;
	float Roughness;
	float metallic;
};

struct DirectionalLight
{
	float3 Strength;
	float3 Direction;
};

//菲涅尔方程
float3 fresnelSchlick(float cosTheta, float3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//正态分布函数
float DistributionGGX(float3 N, float3 H, float roughness) {//N->Normal; H->给定向量; roughness->粗糙度
	float a = roughness * roughness; 
	float a2 = a * a;//光照在几何遮蔽函数和法线分布函数中采用粗糙度的平方会让光照看起来更加自然。
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return nom / denom;
}

//几何函数
float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	float nom = NdotV;
	float denom = NdotV * (1 - k) + k;
	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

//*****************************Compute Cook-torrance**********************//
float3 CookTorranceBRDF(
	float3 radiance,
	float3 N, 
	float3 H, 
	float3 V,
	float3 L,
	float3 F0,
	Material mat
)
{
	float NDF = DistributionGGX(N, H, mat.Roughness);
	float G = GeometrySmith(N, V, L, mat.Roughness);
	float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	float3 kS = F;
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - mat.metallic;

	float3 nominator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
	float3 specular = nominator / denominator;

	// outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);
	return  (kD * mat.DiffuseAlbedo.rgb / PI + specular) * radiance * NdotL;
}


float3 ComputeDirectionalLight(
	DirectionalLight L,
	Material mat,
	float3 normal,
	float3 toEye,
	float3 F0)
{
	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -L.Direction;

	// Scale light down by Lambert's cosine law.
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = L.Strength * ndotl;

	float3 Half = normalize(toEye + lightVec);

	return  CookTorranceBRDF(lightStrength, normal, Half, toEye, lightVec, F0, mat);
}
