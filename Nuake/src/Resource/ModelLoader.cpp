#include "src/Resource/ModelLoader.h"

#include "src/Core/FileSystem.h"
#include "src/Core/Logger.h"

#include "src/Core/String.h"

#include "src/Resource/SkinnedModel.h"
#include "src/Resource/Model.h"
#include "src/Resource/SkeletalAnimation.h"

namespace Nuake
{
	ModelLoader::ModelLoader() {}
	ModelLoader::~ModelLoader() {}

	Ref<Model> ModelLoader::LoadModel(const std::string& path, bool absolute)
	{
		m_Meshes.clear();
		Ref<Model> model = CreateRef<Model>(path);

		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);

		auto importFlags = 
			aiProcess_Triangulate | 
			aiProcess_GenSmoothNormals |
			aiProcess_FixInfacingNormals | 
			aiProcess_CalcTangentSpace;

		modelDir = absolute ? path + "/../" : FileSystem::Root + path + "/../";
		const std::string filePath = absolute ? path : FileSystem::Root + path;
		const aiScene* scene = importer.ReadFile(filePath, importFlags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::string assimpErrorMsg = std::string(importer.GetErrorString());
			std::string logMsg = "[Failed to load model] - " + assimpErrorMsg;
			Logger::Log(logMsg, "model", WARNING);

			return model;
		}

		ProcessNode(scene->mRootNode, scene);

		for (const auto& mesh : m_Meshes)
		{
			model->AddMesh(mesh);
		}

		return model;
	}

	Ref<SkinnedModel> ModelLoader::LoadSkinnedModel(const std::string& path, bool absolute)
	{
		m_SkinnedMeshes.clear();
		Ref<SkinnedModel> model = CreateRef<SkinnedModel>(path);

		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);

		auto importFlags =
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FixInfacingNormals |
			aiProcess_CalcTangentSpace;

		modelDir = absolute ? path + "/../" : FileSystem::Root + path + "/../";
		const std::string filePath = absolute ? path : FileSystem::Root + path;
		const aiScene* scene = importer.ReadFile(filePath, importFlags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::string assimpErrorMsg = std::string(importer.GetErrorString());
			std::string logMsg = "Failed to load model: " + assimpErrorMsg;
			Logger::Log(logMsg, "model", WARNING);

			return model;
		}

		ProcessSkinnedNode(scene->mRootNode, scene);

		if (scene->HasAnimations())
		{
			SkeletalAnimation animation;

			for (uint32_t i = 0; i < scene->mNumAnimations; i++)
			{
				aiAnimation* aiAnim = scene->mAnimations[i];
				const float duration = aiAnim->mDuration;
				const float ticksPerSecond = aiAnim->mTicksPerSecond;

				// Read data
				SkeletonNode rootSkeletonNode;
				ProcessAnimationNode(rootSkeletonNode, scene->mRootNode);

				model->SetSkeletonRootNode(rootSkeletonNode);
			}
		}

		for (const auto& mesh : m_SkinnedMeshes)
		{
			model->AddMesh(mesh);
		}

		return model;
	}

	void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene));
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	void ModelLoader::ProcessSkinnedNode(aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_SkinnedMeshes.push_back(ProcessSkinnedMesh(mesh, scene));
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessSkinnedNode(node->mChildren[i], scene);
		}
	}

	Ref<SkinnedMesh> ModelLoader::ProcessSkinnedMesh(aiMesh* node, const aiScene* scene)
	{
		auto& vertices = ProcessSkinnedVertices(node);
		auto& indices = ProcessIndices(node);
		auto& material = ProcessMaterials(scene, node);
		auto& bones = std::vector<Bone>();
		auto& bonesMap = std::unordered_map<std::string, Bone>();

		if (node->HasBones())
		{
			uint32_t boneCounter = 0;
			for (uint32_t i = 0; i < node->mNumBones; i++)
			{
				aiBone* bone = node->mBones[i];
				const std::string& boneName = bone->mName.C_Str();

				int32_t boneId = -1;
				if (bonesMap.find(boneName) == bonesMap.end())
				{
					boneId = boneCounter;
					boneCounter++;

					Bone newBone = Bone(boneName, boneId);
					newBone.Offset = ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
					bones.push_back(newBone);

					bonesMap[boneName] = newBone;
				}
				else
				{
					boneId = bonesMap[boneName].Id;
				}
				
				assert(boneId != -1);

				const uint32_t numWeight = bone->mNumWeights;
				for (uint32_t j = 0; j < numWeight; j++)
				{
					aiVertexWeight vertexWeight = bone->mWeights[j];
					const uint32_t vertexWeightVertexId = vertexWeight.mVertexId;

					SetVertexBoneData(vertices[vertexWeightVertexId], boneId, vertexWeight.mWeight);
					
					//BoneVertexWeight boneVertexWeight
					//{
					//	vertexWeightVertexId,
					//	vertexWeight.mWeight
					//};
					//
					//Bone& newBone = bones[boneId];
					//newBone.VertexWeights.push_back(std::move(boneVertexWeight));
				}
			}
		}
		else
		{
			Logger::Log("Using skinned mesh imported while the model has no bones!", "model loader", WARNING);
		}

		// Fill in the bones in the vertices
		Ref<SkinnedMesh> mesh = CreateRef<SkinnedMesh>();
		mesh->AddSurface(vertices, indices, bones);
		mesh->SetMaterial(material);
		return mesh;
	}

	Ref<Mesh> ModelLoader::ProcessMesh(aiMesh* node, const aiScene* scene)
	{
		auto vertices = ProcessVertices(node);
		auto indices = ProcessIndices(node);
		auto material = ProcessMaterials(scene, node);

		Ref<Mesh> mesh = CreateRef<Mesh>();
		mesh->AddSurface(vertices, indices);
		mesh->SetMaterial(material);

		return mesh;
	}

	std::vector<Vertex> ModelLoader::ProcessVertices(aiMesh* mesh)
	{
		auto vertices = std::vector<Vertex>();
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			Vector3 current;

			// Position
			current.x = mesh->mVertices[i].x;
			current.y = mesh->mVertices[i].z;
			current.z = mesh->mVertices[i].y;
			vertex.position = current;

			// Normals
			current.x = mesh->mNormals[i].x;
			current.y = mesh->mNormals[i].z;
			current.z = mesh->mNormals[i].y;
			vertex.normal = current;

			// Tangents
			if (mesh->mTangents)
			{
				current.x = mesh->mTangents[i].x;
				current.y = mesh->mTangents[i].z;
				current.z = mesh->mTangents[i].y;
				vertex.tangent = current;
			}

			if (mesh->mBitangents)
			{
				current.x = mesh->mBitangents[i].x;
				current.y = mesh->mBitangents[i].z;
				current.z = mesh->mBitangents[i].y;
				vertex.bitangent = current;
			}

			vertex.uv = glm::vec2(0.0f, 0.0f);

			// Does it contain UVs?
			if (mesh->mTextureCoords[0]) 
			{
				float u = mesh->mTextureCoords[0][i].x;
				float v = mesh->mTextureCoords[0][i].y;
				vertex.uv = Vector2(u, v);
			}

			vertices.push_back(vertex);
		}

		return vertices;
	}

	std::vector<SkinnedVertex> ModelLoader::ProcessSkinnedVertices(aiMesh* mesh)
	{
		auto vertices = std::vector<SkinnedVertex>();
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			SkinnedVertex vertex;
			for (int w = 0; w < MAX_BONE_INFLUENCE; w++)
			{
				vertex.boneIDs[w] = -1;
				vertex.weights[w] = 0.0f;
			}
			
			Vector3 current;

			// Position
			current.x = mesh->mVertices[i].x;
			current.y = mesh->mVertices[i].y;
			current.z = mesh->mVertices[i].z;
			vertex.position = current;

			// Normals
			current.x = mesh->mNormals[i].x;
			current.y = mesh->mNormals[i].y;
			current.z = mesh->mNormals[i].z;
			vertex.normal = current;

			// Tangents
			if (mesh->mTangents)
			{
				current.x = mesh->mTangents[i].x;
				current.y = mesh->mTangents[i].y;
				current.z = mesh->mTangents[i].z;
				vertex.tangent = current;
			}

			if (mesh->mBitangents)
			{
				current.x = mesh->mBitangents[i].x;
				current.y = mesh->mBitangents[i].y;
				current.z = mesh->mBitangents[i].z;
				vertex.bitangent = current;
			}

			vertex.uv = glm::vec2(0.0f, 0.0f);

			// Does it contain UVs?
			if (mesh->mTextureCoords[0])
			{
				float u = mesh->mTextureCoords[0][i].x;
				float v = mesh->mTextureCoords[0][i].y;
				vertex.uv = Vector2(u, v);
			}

			vertices.push_back(vertex);
		}

		return vertices;
	}

	void ModelLoader::SetVertexBoneData(SkinnedVertex& vertex, int boneID, float weight)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			if (vertex.boneIDs[i] < 0)
			{
				vertex.weights[i] = weight;
				vertex.boneIDs[i] = boneID;
				break;
			}
		}
	}

	void ModelLoader::ProcessAnimationNode(SkeletonNode& dest, const aiNode* src)
	{
		assert(src);

		dest.Name = src->mName.data;
		dest.Transform = ConvertMatrixToGLMFormat(src->mTransformation);
		dest.ChildrenCount = src->mNumChildren;

		for (uint32_t i = 0; i < dest.ChildrenCount; i++)
		{
			SkeletonNode newNode;
			ProcessAnimationNode(newNode, src->mChildren[i]);
			dest.Children.push_back(std::move(newNode));
		}
	}

	std::vector<uint32_t> ModelLoader::ProcessIndices(aiMesh* mesh)
	{
		auto indices = std::vector<uint32_t>();
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		return indices;
	}

	Ref<Material> ModelLoader::ProcessMaterials(const aiScene* scene, aiMesh* mesh)
	{
		if (mesh->mMaterialIndex < 0)
			return nullptr;

		aiMaterial* materialNode = scene->mMaterials[mesh->mMaterialIndex];

		Ref<Material> material = CreateRef<Material>();

		aiString str;
		if (materialNode->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			materialNode->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
			material->SetAlbedo(albedoTexture);
		}

		if (materialNode->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			materialNode->GetTexture(aiTextureType_NORMALS, 0, &str);
			Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
			material->SetNormal(albedoTexture);
		}

		if (materialNode->GetTextureCount(aiTextureType_METALNESS) > 0)
		{
			materialNode->GetTexture(aiTextureType_METALNESS, 0, &str);
			Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
			material->SetMetalness(albedoTexture);
		}

		if (materialNode->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
		{
			materialNode->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str);
			Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
			material->SetAO(albedoTexture);
		}

		if (materialNode->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
		{
			materialNode->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
			Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
			material->SetRoughness(albedoTexture);
		}

		if (materialNode->GetTextureCount(aiTextureType_DISPLACEMENT) > 0)
		{
			materialNode->GetTexture(aiTextureType_DISPLACEMENT, 0, &str);
			Ref<Texture> albedoTexture = ProcessTextures(scene, str.C_Str());
			material->SetDisplacement(albedoTexture);
		}

		return material;
	}

	Ref<Texture> ModelLoader::ProcessTextures(const aiScene* scene, const std::string& path)
	{
		// Load embedded textures, a texture is embedded is the path
		// starts with a '*'.
		if (String::BeginsWith(path, "*"))
		{
			uint32_t textureIndex = std::atoi(String::Split(path, '*')[1].c_str());
			const aiTexture* aitexture = scene->GetEmbeddedTexture(path.c_str());

			Vector2 textureSize = Vector2(aitexture->mWidth, aitexture->mHeight);
			auto texture = CreateRef<Texture>(textureSize, (unsigned char*)aitexture->pcData, textureSize.x);
			return texture;
		}
		
		std::string texturePath = modelDir + path;
		if (!FileSystem::FileExists(texturePath, true))
		{
			std::string textureNotFoundmsg = "Texture file couldn't be found: " + texturePath;
			Logger::Log(textureNotFoundmsg, "model", Nuake::LOG_TYPE::WARNING);

			texturePath = "resources/Textures/default/Default.png";
		}

		return TextureManager::Get()->GetTexture(texturePath);
	}
}