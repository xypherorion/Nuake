#pragma once
#include "Shaders/Shader.h"
#include "../Scene/Components/Components.h"
#include "../Core/Core.h"

namespace Nuake
{
	struct Light 
	{
		TransformComponent transform;
		LightComponent light;
	};

	class Renderer 
	{
	public:
		static Shader* m_Shader;
		static Shader* m_ShadowmapShader;
		static Shader* m_SkyboxShader;
		static Shader* m_BRDShader;
		static Shader* m_GBufferShader;
		static Shader* m_DeferredShader;
		static Shader* m_ProceduralSkyShader;
		static Shader* m_DebugShader;

		static void Init();

		// Drawing states
		static void BeginDraw(Ref<Camera> camera);
		static void EndDraw();

		// Lights
		static std::vector<Light> m_Lights;
		static void RegisterLight(TransformComponent transform, LightComponent light, Ref<Camera> cam);
		static void RegisterDeferredLight(TransformComponent transform, LightComponent light, Camera* cam);


		static void SubmitMesh(Ref<Mesh> mesh);

		// Debug
		static void DrawDebugLine(glm::vec3 start, glm::vec3 end, glm::vec4 color);
		static void DrawCube(TransformComponent transform, glm::vec4 color);
		static void DrawSphere(TransformComponent transform, glm::vec4 color);
		static void DrawQuad(Matrix4 transform);
	};
}
