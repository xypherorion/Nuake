#pragma once

#include "../Core/Physics/CharacterController.h"
class CharacterControllerComponent
{
public:
	Ref < Physics::CharacterController>  CharacterController;

	float Height = 1.0f;
	float Radius = 0.2f;
	float Mass = 25.0f;

	CharacterControllerComponent() 
	{
		
	}

	json Serialize() {
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Height);
		SERIALIZE_VAL(Radius);
		SERIALIZE_VAL(Mass);
		END_SERIALIZE();
	}

	bool Deserialize(const std::string str) {
		BEGIN_DESERIALIZE();
		Height = j["Height"];
		Radius = j["Radius"];
		Mass = j["Mass"];
		return true;
	}
	void SyncWithTransform(TransformComponent& tc)
	{
		btVector3 pos = CharacterController->m_motionTransform.getOrigin();
		glm::vec3 finalPos = glm::vec3(pos.x(), pos.y(), pos.z());

		std::string msg = "moved to: " + std::to_string(finalPos.x) + " " + std::to_string(finalPos.y) + "\n";
		printf(msg.c_str());
		tc.Translation = finalPos;
	}
};