#pragma once
#include "Memory.h"
struct RenderNode {
	char pad_01[0x10]; // 0x00(0x10)
	unsigned __int64 rnd_flags; // 0x10(0x08)
	char pad_02[0x14]; // 0x18(0x14)
	unsigned int silhouettes_param; // 0x2C(0x04)
	char pad_03[0x8]; // 0x30(0x8)
	float m_fWSMaxViewDist; //0x38(0x4)
};
struct HealthBar {
	char pad_01[0x10]; // 0x00(0x10)
	unsigned int current_hp; // 0x10(0x04)
	unsigned int regenerable_max_hp; // 0x14(0x04)
	unsigned int current_max_hp; // 0x18(0x04)
};
struct EntityNameStruct {
	char name[100];
};
enum class EntityType : int
{
	Unknown,

	LocalPlayer,
	EnemyPlayer,
	FriendlyPlayer,
	DeadPlayer,

	Butcher,
	Spider,
	Assassin,
	Scrapbeak,
	Rotjaw,
	Hellborn,

	CashRegister,
	GoldCashRegister,

	Pouch,
	Poster,
	WorkbenchUpgrade,
	WorkbenchOil,
	Trait,

	ExtractionPoint,
	Clue,

	BearTrap,
	TripMine,
	DarksightDynamite,

	OilBarrel,
	GunpowderBarrel,
	BioBarrel,

	AmmoSwapBox,
	SpitzerBullets,
	PoisonBullets,
	HighVelocityBullets,
	IncendiaryBullets,
	DumDumBullets,
	ExplosiveBullets,
	FMJBullets,
	PoisonBolts,
	ExplosiveBolts,
	IncendiaryShells,
	PennyShotShells,
	FlechetteShells,
	SlugShells,

	SupplyBox,
	AmmoCrate,
	SpecialAmmoCrate,
	CompactAmmoPackage,
	MediumAmmoPackage,
	LongAmmoPackage,
	ShotgunAmmoPackage,
	Medkit,

	Event,
};
struct Matrix4x4 {
	float m[4][4];

	Matrix4x4() {
		memset(m, 0, sizeof(m));
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	}

	Vector3 TransformPoint(const Vector3& point) const {
		Vector3 result;
		result.x = point.x * m[0][0] + point.y * m[1][0] + point.z * m[2][0] + m[3][0];
		result.y = point.x * m[0][1] + point.y * m[1][1] + point.z * m[2][1] + m[3][1];
		result.z = point.x * m[0][2] + point.y * m[1][2] + point.z * m[2][2] + m[3][2];
		return result;
	}
};

class WorldEntity
{
private:
	uint64_t Class = 0x0;
	uint64_t ClassAddress = 0x0;
	const uint64_t PosOffset = 0x134;
	const uint64_t StringBufferOffset = 0x10;
	const uint64_t ClassPointerOffset = 0x18;
	const uint64_t SlotsPointerOffset = 0xA8;
	const uint64_t SlotOffset = 0x0;
	const uint64_t RenderNodePointerOffset = 0xA0;
	RenderNode Node;
	EntityNameStruct EntityName;
	EntityNameStruct ClassName;

	uint64_t EntityNamePointer = 0x0;
	uint64_t ClassNamePointer = 0x0;
	uint64_t ClassPointer = 0x0;
	uint64_t SlotsPointer = 0x0;
	uint64_t Slot = 0x0;
	uint64_t RenderNodePointer = 0x0;
	Vector3 Position;
	EntityType Type;

	const uint64_t HpOffset1 = 0x198;
	const uint64_t HpOffset2 = 0x20;
	const uint64_t HpOffset3 = 0xD0;
	const uint64_t HpOffset4 = 0x78;
	const uint64_t HpOffset5 = 0x58;
	uint64_t HpPointer1 = 0x0;
	uint64_t HpPointer2 = 0x0;
	uint64_t HpPointer3 = 0x0;
	uint64_t HpPointer4 = 0x0;
	uint64_t HpPointer5 = 0x0;
	HealthBar Health;

	const uint64_t CharacterInstanceOffset = 0x88;
	const uint64_t BoneArrayOffset = 0xD00;
	const uint64_t SkeletonPoseOffset = 0x1260;
	const uint64_t ModelJointsOffset = 0x8;
	const uint64_t BoneArraySizeOffset = 0xA0;
	const uint64_t BoneStructSize = 0x1C;
	const uint64_t WorldMatrixOffset = 0x160;
	static const int MAX_BONES = 15;
	uint32_t BoneCount = 0;
	int BoneIndex[MAX_BONES] = { 0 };
	Vector3 HeadPosition;

	const uint64_t InternalFlagsOffset = 0x8;
	uint32_t InternalFlags = 0x0;
	bool Hidden = false;

	bool Valid = true;
	std::unordered_map<EntityType, std::wstring> Names = {
		{EntityType::Unknown, L"???"},

		{EntityType::LocalPlayer, L"Me, Myself and I"},
		{EntityType::EnemyPlayer, L"Hunter"},
		{EntityType::FriendlyPlayer, L"Teammate"},
		{EntityType::DeadPlayer, L"Dead"},

		{EntityType::Butcher, L"Butcher"},
		{EntityType::Assassin, L"Assassin"},
		{EntityType::Scrapbeak, L"Screapbeak"},
		{EntityType::Spider, L"Spider"},
		{EntityType::Rotjaw, L"Rotjaw"},
		{EntityType::Hellborn, L"Hellborn"},

		{EntityType::CashRegister, L"Cash Register"},
		{EntityType::GoldCashRegister, L"GOLD CASH REGISTER (STONKS)"},

		{EntityType::Pouch, L"Pouch"},
		{EntityType::Poster, L"Poster"},
		{EntityType::WorkbenchUpgrade, L"Blueprint"},
		{EntityType::WorkbenchOil, L"Gun Oil"},
		{EntityType::Trait, L"Trait"},

		{EntityType::ExtractionPoint, L"Extraction"},
		{EntityType::Clue, L"Clue"},

		{EntityType::BearTrap, L"Bear Trap"},
		{EntityType::TripMine, L"Trip Mine"},
		{EntityType::DarksightDynamite, L"C4"},

		{EntityType::OilBarrel, L"Oil Barrel"},
		{EntityType::GunpowderBarrel, L"Barrel"},
		{EntityType::BioBarrel, L"Bio Barrel"},

		{EntityType::AmmoSwapBox, L"AmmoSwap Box"},
		{EntityType::SpitzerBullets, L"Spitzer Bullets"},
		{EntityType::PoisonBullets, L"Poison Bullets"},
		{EntityType::HighVelocityBullets, L"High Velocity Bullets"},
		{EntityType::IncendiaryBullets, L"Incendiary Bullets"},
		{EntityType::DumDumBullets, L"Dum Dum Bullets"},
		{EntityType::ExplosiveBullets, L"Explosive Bullets"},
		{EntityType::FMJBullets, L"FMJ Bullets"},
		{EntityType::PoisonBolts, L"Poison Bolts"},
		{EntityType::ExplosiveBolts, L"Explosive Bolts"},
		{EntityType::IncendiaryShells, L"Incendiary Shells"},
		{EntityType::PennyShotShells, L"Penny Shot Shells"},
		{EntityType::FlechetteShells, L"Flechette Shells"},
		{EntityType::SlugShells, L"Slug Shells"},

		{EntityType::SupplyBox, L"Supply Box"},
		{EntityType::AmmoCrate, L"Ammo Crate"},
		{EntityType::SpecialAmmoCrate, L"Special Ammo Crate"},
		{EntityType::CompactAmmoPackage, L"Compact Ammo Package"},
		{EntityType::MediumAmmoPackage, L"Medium Ammo Package"},
		{EntityType::LongAmmoPackage, L"Long Ammo Package"},
		{EntityType::ShotgunAmmoPackage, L"Shotgun Ammo Package"},
		{EntityType::Medkit, L"Medkit"},

		{EntityType::Event, L"Event"},
	};
public:
	WorldEntity( uint64_t classptr);
	void SetUp(VMMDLL_SCATTER_HANDLE handle);
	void SetUp1(VMMDLL_SCATTER_HANDLE handle);
	void SetUp2(VMMDLL_SCATTER_HANDLE handle);
	void SetUp3(VMMDLL_SCATTER_HANDLE handle);
	void SetUp4(VMMDLL_SCATTER_HANDLE handle);
	EntityNameStruct GetEntityName() { return EntityName; }
	EntityNameStruct GetEntityClassName() { return ClassName; }
	Vector3 GetPosition() { return Position; }
	RenderNode GetRenderNode() { return Node; }
	HealthBar GetHealth() { return Health; }
	uint32_t GetInternalFlags() { return InternalFlags; }
	void SetType(EntityType type) { Type = type; }
	EntityType GetType() { return Type; }
	void WriteNode(VMMDLL_SCATTER_HANDLE handle, int colour, bool show);
	void UpdatePosition(VMMDLL_SCATTER_HANDLE handle);
	void UpdateNode(VMMDLL_SCATTER_HANDLE handle);
	void UpdateHealth(VMMDLL_SCATTER_HANDLE handle);
	void UpdateBones();
	void UpdateExtraction(VMMDLL_SCATTER_HANDLE handle);
	void UpdateClass(VMMDLL_SCATTER_HANDLE handle);
	uint64_t GetClass() { return ClassAddress; }
	bool IsHidden() { return Hidden; }
	void SetHidden(bool isHidden) { Hidden = isHidden; }
	bool GetValid() { return Valid; }
	void SetValid(bool valid) { Valid = valid; }
	std::wstring GetName() { return Names[Type]; };
	bool IsLocalPlayer();
	Vector3 GetHeadPosition() const { return HeadPosition; }

	uint64_t SpecCountOffset1 = 0x198;
	uint64_t SpecCountOffset2 = 0x20;
	uint64_t SpecCountOffset3 = 0xd0;
	uint64_t SpecCountOffset4 = 0x100;
	uint64_t SpecCountOffset5 = 0x10;
	uint64_t SpecCountPointer1 = 0x0;
	uint64_t SpecCountPointer2 = 0x0;
	uint64_t SpecCountPointer3 = 0x0;
	uint64_t SpecCountPointer4 = 0x0;
	int SpecCount = 0;

	static const uint32_t HIDDEN_FLAG = 0x8;
};
