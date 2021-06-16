import "Scripts/Engine" for Engine
import "Scripts/Math" for Vector3

class Scene {
	foreign static GetEntityID(name)


	static GetEntity(name) {
		var entId = Scene.GetEntityID(name)
		var ent = Entity.new(entId)
		return ent
	}

	foreign static EntityHasComponent(id, name)
	static EntityGetComponent(id, component) {
		if(this.EntityHasComponent(id, component) == false) {
			Engine.Log("Tried getting a non-existent component of type: %(component) on entity with id: %(id)")
			return
		}

		if (component == "Light") {
			return Light.new(id)
		} else if (component == "CharacterController") {
			return CharacterController.new(id)
		} else if (component == "Camera") {
			return Camera.new(id)
		} else if (component == "Transform") {
			return TransformComponent.new(id)
		}

	}

    //
	// Components
	//
	// Transform
	foreign static GetTranslation_(e)
	//foreign static SetTranslation_(e, x, y, z)
	//foreign static SetRotation_(e, x, y, z)
	//foreign static SetScale_(e, x, y, z)

	// Light
	foreign static GetLightIntensity_(e)  // returns a float
	foreign static SetLightIntensity_(e, intensity)
	//foreign static SetLightIsVolumetric_(e, bool)
	//foreign static SetLightSyncDirectionWithSky_(e, bool)
	//foreign static SetLightColor_(e, r, g, b)
	//foreign static GetLightColor_(e)

	// Camera
	foreign static SetCameraDirection_(e, x, y, z)
	foreign static GetCameraDirection_(e) // returns a list x,y,z
	foreign static GetCameraRight_(e)     // returns a list x,y,z
	//foreign static SetcameraFov(e, fov)
	//foreign static GetCameraFov(e)        // returns a float

	// Character controller
	foreign static MoveAndSlide_(e, x, y, z)
	foreign static IsCharacterControllerOnGround_(e)
	//foreign static IsOnGround_(e)
	

}

class Entity {
	construct new(id) {
		_entityId = id
	}

	HasComponent(component) {
		return Scene.EntityHasComponent(_entityId, component)
	}

	GetComponent(component) {
		return Scene.EntityGetComponent(_entityId, component)
	}

	// Foreign engine functions
/* 
	// Transform
	foreign static SetTranslation_(e, x, y, z)
	foreign static SetRotation_(e, x, y, z)
	foreign static SetScale_(e, x, y, z)

	// Character controller
	foreign static SetVelocity(e, x, y, z)
	foreign static SetStepHeight(e, x, y, z)
	foreign static IsOnGround(e)
	*/
	// Light
	//
	/* 
	foreign static SetLightIsVolumetric_(e, bool)
	foreign static SetLightSyncDirectionWithSky_(e, bool)
	foreign static SetLightColor_(e, r, g, b)

	// Camera
	foreign static SetCameraFov_(e, fov)
	foreign static SetCameraType(e, type)
	foreign static SetCameraDirection_(e, x, y, z)
	*/
}

class TransformComponent {
	construct new(id) {
		_entityId = id
	}

	GetTranslation() {
		var result = Scene.GetTranslation_(_entityId)
		return Vector3.new(result[0], result[1], result[2])
	}


}

class Light {
	construct new(id) {
		_entityId = id
	}


	SetIntensity(intensity) {
		Scene.SetLightIntensity_(_entityId, intensity)
	}

	GetIntensity() {
		return Scene.GetLightIntensity_(_entityId)
	}

	
	/*
	SetType(type) {
		this.SetType_(_entityId, type)
	}

	SetColor(color) {
		this.SetColor_(_entityId, color.r, color.g, color.b, color.a)
	}*/
	
}

class CharacterController {
	construct new(id) {
		_entityId = id
	}

	MoveAndSlide(vel) {
		Scene.MoveAndSlide_(_entityId, vel.x, vel.y, vel.z)
	}

	IsOnGround() {
		return Scene.IsCharacterControllerOnGround_(_entityId)
	}
}

class Camera {
	construct new(id) {
		_entityId = id
	}

	SetDirection(dir) {
		Scene.SetCameraDirection_(_entityId, dir.x, dir.y, dir.z)
	}

	GetDirection() {
		var dir = Scene.GetCameraDirection_(_entityId)
		return Vector3.new(dir[0], dir[1], dir[2])
	}

	GetRight() {
		var dir = Scene.GetCameraRight_(_entityId)
		return Vector3.new(dir[0], dir[1], dir[2])
	}

}
