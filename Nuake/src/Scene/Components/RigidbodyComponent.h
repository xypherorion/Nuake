#pragma once
#include "TransformComponent.h"
#include "BaseComponent.h"
#include "src/Core/Core.h"

namespace Nuake {
	namespace Physics 
	{
		class RigidBody;
	};

	class RigidBodyComponent
	{
	public:
		float Mass;
		Ref<Physics::RigidBody> Rigidbody;

		Vector3 QueuedForce = Vector3();

		RigidBodyComponent();
		Ref<Physics::RigidBody> GetRigidBody() const;
		
		void SyncTransformComponent(TransformComponent* tc);
		void SyncWithTransform(TransformComponent* tc);

		void DrawShape(TransformComponent* tc);
		void DrawEditor();


		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL_LBL("Mass", Mass);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			Mass = j["Mass"];
			return true;
		}
	};
}
