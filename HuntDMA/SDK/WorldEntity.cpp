#include "pch.h"
#include "WorldEntity.h"
#include "Globals.h"

WorldEntity::WorldEntity( uint64_t entity)
{
	this->Class = entity;
	this->ClassAddress = entity;
	
}
void WorldEntity::SetUp(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, this->Class + SpecCountOffset1, &SpecCountPointer1, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->Class + HpOffset1, &HpPointer1, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->Class + PosOffset, &Position, sizeof(Vector3));
	TargetProcess.AddScatterReadRequest(handle, this->Class + StringBufferOffset, &EntityNamePointer, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->Class + ClassPointerOffset, &ClassPointer, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->Class + TypeNameOffset1, &TypeNamePointer1, sizeof(uint64_t));
	if (Class != 0)
		TargetProcess.AddScatterReadRequest(handle, this->Class + SlotsPointerOffset, &SlotsPointer, sizeof(uint64_t));
	else
		SlotsPointer = 0;
}
void WorldEntity::SetUp1(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, this->SpecCountPointer1 + SpecCountOffset2, &SpecCountPointer2, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->HpPointer1 + HpOffset2, &HpPointer2, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle,this->EntityNamePointer, &EntityName, sizeof(EntityNameStruct));
	TargetProcess.AddScatterReadRequest(handle, this->ClassPointer + StringBufferOffset, &ClassNamePointer, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->TypeNamePointer1 + TypeNameOffset2, &TypeNamePointer2, sizeof(uint64_t));
	if (SlotsPointer != 0)
		TargetProcess.AddScatterReadRequest(handle, this->SlotsPointer, &Slot, sizeof(uint64_t));
	else
		Slot = 0;
}
void WorldEntity::SetUp2(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, this->SpecCountPointer2 + SpecCountOffset3, &SpecCountPointer3, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->HpPointer2 + HpOffset3, &HpPointer3, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->TypeNamePointer2, &TypeName, sizeof(EntityNameStruct));
	if (Slot != 0)
		TargetProcess.AddScatterReadRequest(handle, this->Slot + RenderNodePointerOffset, &RenderNodePointer, sizeof(uint64_t));
	else
		RenderNodePointer = 0;
	TargetProcess.AddScatterReadRequest( handle, this->ClassNamePointer,&ClassName, sizeof(EntityNameStruct));

}
void WorldEntity::SetUp3(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, this->SpecCountPointer3 + SpecCountOffset4, &SpecCountPointer4, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, this->HpPointer3 + HpOffset4, &HpPointer4, sizeof(uint64_t));
	ClassName.name[99] = '\0';
	EntityName.name[99] = '\0';
	TypeName.name[99] = '\0';
	std::string typeName(TypeName.name);
	if (typeName != "")
		CompactTypeName = typeName.substr(typeName.find_last_of("./") + 1);
	
	TargetProcess.AddScatterReadRequest(handle, RenderNodePointer, &Node, sizeof(RenderNode));
}
void WorldEntity::SetUp4(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, this->HpPointer4 + HpOffset5, &HpPointer5, sizeof(uint64_t));
}

void WorldEntity::WriteNode(VMMDLL_SCATTER_HANDLE handle, int colour, bool show)
{
	uint32_t convertedcolour = 0;
	if(colour == 0)
		convertedcolour = 0xFF0000FF; // Outline Red
	else if (colour == 1)
		convertedcolour = 0x0000FFFF; // Outline Blue
	else if (colour == 2)
		convertedcolour = 0xFFFF00FF; // Outline Yellow
	else if (colour == 3)
		convertedcolour = 0xFFA500FF; // Outline Orange
	else if (colour == 4)
		convertedcolour = 0x00FFFFFF; // Outline Cyan
	else if (colour == 5)
		convertedcolour = 0xFF00FFFF; // Outline Magenta
	else if (colour == 6)
		convertedcolour = 0xFFFFFFFF; // Outline White
	else if (colour == 7)
		convertedcolour = 0xFF000000; // Filled Red
	else if (colour == 8)
		convertedcolour = 0x0000FF00; // Filled Blue
	else if (colour == 9)
		convertedcolour = 0xFFFF0000; // Filled Yellow
	else if (colour == 10)
		convertedcolour = 0xFFA50000; // Filled Orange
	else if (colour == 11)
		convertedcolour = 0x00FFFF00; // Filled Cyan
	else if (colour == 12)
		convertedcolour = 0xFF00FF00; // Filled Magenta
	else if (colour == 13)
		convertedcolour = 0xFFFFFF00; // Filled White
	if (RenderNodePointer != 0)
	{
	//	if(Node.rnd_flags == 284558360584 || Node.rnd_flags == 9680453640)
		uint64_t none = 0x200000008;
		uint64_t allmap = 0x80018;
		uint64_t limit_distance = 0x80008;
		float maxdistance = 5000;
		if (!show)
		{
			TargetProcess.AddScatterWriteRequest(handle, RenderNodePointer + 0x10, &none, sizeof(uint64_t));
			return;
		}
		TargetProcess.AddScatterWriteRequest(handle, RenderNodePointer + 0x10, &allmap, sizeof(uint64_t)); // change render flag to max distance, allows us to use chams at further distances as long as the model isn't culled.
		TargetProcess.AddScatterWriteRequest(handle, RenderNodePointer + 0x2C, &convertedcolour, sizeof(uint32_t));
		TargetProcess.AddScatterWriteRequest(handle, RenderNodePointer + 0x38, &maxdistance, sizeof(float));
	}
}

void WorldEntity::UpdatePosition(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, this->Class + PosOffset, &Position, sizeof(Vector3));
}

void WorldEntity::UpdateNode(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, RenderNodePointer, &Node, sizeof(RenderNode));
}

void WorldEntity::UpdateHealth(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, HpPointer5, &Health, sizeof(HealthBar));
}

void WorldEntity::UpdateBones() // Currently doesn't work
{
	// Get character instance pointer and bones
	uint64_t charInstancePointer = TargetProcess.Read<uint64_t>(Slot + CharacterInstanceOffset);
	if (!charInstancePointer) return;

	uint64_t bonePtr0 = TargetProcess.Read<uint64_t>(charInstancePointer + BoneArrayOffset + 0x20);
	uint64_t bonePtr1 = TargetProcess.Read<uint64_t>(charInstancePointer + BoneArrayOffset + 0x38);
	if (!bonePtr0 || !bonePtr1) return;

	uint64_t boneArray = bonePtr0 + ((bonePtr1 + 3) & 0xFFFFFFFFFFFFFFFC);
	uint64_t skeletonPose = TargetProcess.Read<uint64_t>(charInstancePointer + SkeletonPoseOffset); // problem here is a bit that it seems not all hunter skins have default skeleton pose anymore
	if (!skeletonPose) return;

	// Read bone mapping
	std::map<std::string, int> boneMap;
	uint64_t modelJoints = TargetProcess.Read<uint64_t>(skeletonPose + ModelJointsOffset);
	uint32_t boneArraySize = TargetProcess.Read<uint32_t>(skeletonPose + BoneArraySizeOffset);
	BoneCount = boneArraySize;

	// Cache all bone names and indices
	for (uint32_t i = 0; i < boneArraySize; i++)
	{
		auto boneName = TargetProcess.Read<EntityNameStruct>(modelJoints + (i * 0x38));
		boneMap[std::string(boneName.name)] = i;
	}

	if (!boneMap.empty())
	{
		// Map bone indices
		const char* boneNames[] = {
			"head", "neck", "pelvis", "R_upperarm", "R_forearm", "R_hand", "R_thigh", "R_calf", "R_foot", "L_upperarm", "L_forearm", "L_hand", "L_thigh", "L_calf", "L_foot"
		};

		for (int i = 0; i < MAX_BONES; i++) {
			auto it = boneMap.find(boneNames[i]);
			if (it != boneMap.end()) {
				BoneIndex[i] = it->second;
			}
		}

		// Read head bone position if available
		if (BoneIndex[0] != 0) {
			// Read bone transform
			Vector4 boneRotation;
			Vector3 bonePosition;

			// Read bone quaternion and position
			boneRotation = TargetProcess.Read<Vector4>(boneArray + (BoneIndex[0] * BoneStructSize));

			// Read world matrix
			Matrix4x4 worldMatrix;
			worldMatrix = TargetProcess.Read<Matrix4x4>(Class + WorldMatrixOffset);

			// Calculate head position after scatter read completes
			// Note: The actual calculation will be done after ExecuteReadScatter
			Vector3 localHeadPos = Vector4::Mult(boneRotation, Vector3(0, 0, 0));
			HeadPosition = worldMatrix.TransformPoint(localHeadPos);
			LOG_INFO("Headpos:[%f,%f,%f]", HeadPosition.x, HeadPosition.y, HeadPosition.z);
			HeadPosition = Position;
			HeadPosition.z = HeadPosition.z + 1.7f;
			LOG_INFO("FIXED Headpos:[%f,%f,%f]", HeadPosition.x, HeadPosition.y, HeadPosition.z);
			LOG_INFO("================");
		}
	}
}

void WorldEntity::UpdateExtraction(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, Class + InternalFlagsOffset, &InternalFlags, sizeof(uint32_t));
}

void WorldEntity::UpdateClass(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, Class, &ClassAddress, sizeof(uint64_t));
}

bool WorldEntity::IsLocalPlayer()
{
	return EnvironmentInstance->GetLocalPlayerPointer() == GetClass();
}