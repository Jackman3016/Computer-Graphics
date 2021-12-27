//菲涅尔方程
float3 FresnelSchlick(float cosTheta, float3 F0) {
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
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}
