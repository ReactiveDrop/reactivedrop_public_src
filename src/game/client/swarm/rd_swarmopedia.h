#pragma once

#include "steam/steam_api.h"
#include "asw_marine_skills.h"

namespace RD_Swarmopedia
{
	struct Alien;
	struct Requirement;
	struct Ability;
	struct GlobalStat;
	struct Display;
	struct Model;
	struct Content;

	struct Weapon;
	struct WeaponFact;

	struct Helpers
	{
		Helpers() = delete;

		template<typename T>
		static T *ReadFromFile( const char *, KeyValues * );
		template<typename T>
		static void CopyAddVector( CUtlVectorAutoPurge<T *> &, const CUtlVectorAutoPurge<T *> & );
		template<typename T>
		static void CopyVector( CUtlVectorAutoPurge<T *> &, const CUtlVectorAutoPurge<T *> & );
		static void CopyVector( CUtlStringList &, const CUtlStringList & );
		template<typename T>
		static void AddMerge( CUtlVectorAutoPurge<T *> &, const char *, KeyValues * );
		template<typename T>
		static void AddMerge( CUtlVectorAutoPurge<T *> &, T * );
		template<typename T>
		static void CopyUniqueVector( CUtlVectorAutoPurge<T *> &, const CUtlVectorAutoPurge<T *> & );
	};

	enum class Subset
	{
		Aliens = 1 << 0,
		RegularWeapons = 1 << 1,
		ExtraWeapons = 1 << 2,
		Weapons = RegularWeapons | ExtraWeapons,
		All = Aliens | Weapons,
	};

	struct Collection
	{
		Collection() = default;
		Collection( const Collection &copy );

		CUtlVectorAutoPurge<Alien *> Aliens{};
		CUtlVectorAutoPurge<Weapon *> Weapons{};

		void ReadFromFiles( Subset subset = Subset::All );
	private:
		friend struct Helpers;
		static void ReadHelper( const char *, KeyValues *, void * );
		void ReadFromFile( const char *, KeyValues * );

		Subset ReadSubset{};
	};

	struct Alien
	{
		Alien() = default;
		Alien( const Alien &copy );

		CUtlString ID{};
		CUtlString Name{};
		CUtlString Icon{};
		CUtlVectorAutoPurge<Requirement *> Requirements{};
		CUtlVectorAutoPurge<GlobalStat *> GlobalStats{};
		CUtlVectorAutoPurge<Display *> Display{};
		CUtlVectorAutoPurge<Ability *> Abilities{};
		CUtlVectorAutoPurge<Content *> Content{};
		CUtlVector<PublishedFileId_t> Sources{};

		float GetOverallRequirementProgress() const;

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const Alien * ) const;
		void Merge( const Alien * );
	};

	struct Requirement
	{
		Requirement() = default;
		Requirement( const Requirement &copy );

		enum class Type_t
		{
			SteamStat,
		} Type{ Type_t::SteamStat };

		CUtlString Caption{};
		CUtlStringList StatNames{};
		int MinValue{ 0 };

		float GetProgress() const;

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const Requirement * ) const;
		void Merge( const Requirement * );
	};

	struct Ability
	{
		Ability() = default;
		Ability( const Ability &copy );

		CUtlString Caption{};

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const Ability * ) const;
		void Merge( const Ability * );
	};

	struct GlobalStat
	{
		GlobalStat() = default;
		GlobalStat( const GlobalStat &copy );

		CUtlString StatName{};
		CUtlString Caption{};

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const GlobalStat * ) const;
		void Merge( const GlobalStat * );
	};

	struct Display
	{
		Display() = default;
		Display( const Display &copy );

		CUtlString Caption{};
		CUtlVectorAutoPurge<Model *> Models{};
		MaterialLightingState_t LightingState
		{
			{
				{ 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },
			},
			{ 0.0f, 0.0f, 0.0f },
			0,
			{},
		};

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const Display * ) const;
		void Merge( const Display * );
	};

	struct Model
	{
		Model() = default;
		Model( const Model &copy );

		CUtlString ModelName{};
		CUtlString Animation{};
		int Skin{ -1 };
		Color Color{ 255, 255, 255, 255 };
		float Pitch{ 0.0f }, Yaw{ 0.0f }, Roll{ 0.0f };
		float X{ 0.0f }, Y{ 0.0f }, Z{ 0.0f };
		float Scale{ 1.0f };
		CUtlMap<int, int> BodyGroups{ DefLessFunc( int ) };

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const Model * ) const;
		void Merge( const Model * );
	};

	struct Content
	{
		Content() = default;
		Content( const Content &copy );

		enum class Type_t
		{
			Paragraph,
		} Type{ Type_t::Paragraph };

		CUtlString Text{};
		Color Color{ 224, 224, 224, 255 };

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const Content * ) const;
		void Merge( const Content * );
	};

	struct Weapon
	{
		Weapon() = default;
		Weapon( const Weapon &copy );

		CUtlString ClassName{};
		int EquipIndex{ -1 };
		CUtlString Name{};
		CUtlString Icon{};
		ASW_Marine_Class RequiredClass{ MARINE_CLASS_UNDEFINED };
		int RequiredLevel{ 0 };
		bool Builtin{ false };
		bool Extra{ false };
		bool Unique{ false };
		bool Hidden{ false };
		CUtlVectorAutoPurge<GlobalStat *> GlobalStats{};
		CUtlVectorAutoPurge<Display *> Display{};
		CUtlVectorAutoPurge<Ability *> Abilities{};
		CUtlVectorAutoPurge<Content *> Content{};
		CUtlVectorAutoPurge<WeaponFact *> Facts{};
		CUtlVector<PublishedFileId_t> Sources{};

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const Weapon *) const;
		void Merge( const Weapon * );
	};

	struct WeaponFact
	{
		WeaponFact() = default;
		WeaponFact( const WeaponFact &copy );

		enum class Type_T
		{
			Generic,
			Numeric,
			HammerUnits,
			ShotgunPellets,
			DamagePerShot,
			LargeAlienDamageScale,
			BulletSpread,
			Piercing,
			FireRate,
			Ammo,
			Secondary,
			Deployed,
			RequirementLevel,
			RequirementClass,
		} Type{ Type_T::Generic };

		// Generic, etc.
		CUtlString Icon{};
		CUtlString Caption{};
		CUtlString RequireCVar{};
		CUtlString RequireValue{};
		bool HaveRequireValue{ false };
		bool UseWeaponInfo{ true };

		// Numeric, etc.
		int Precision{ 0 };
		float Base{ 0.0f };
		float MinimumValue{ -FLT_MAX };
		float MaximumValue{ FLT_MAX };
		CUtlString CVar{};
		float BaseMultiplier{ 1.0f };
		CUtlStringList BaseMultiplierCVar{};
		CUtlStringList BaseDivisorCVar{};

		ASW_Skill Skill{ ASW_MARINE_SKILL_INVALID };
		int SubSkill{ 0 };
		float SkillMultiplier{ 1.0f };
		CUtlStringList SkillMultiplierCVar{};
		CUtlStringList SkillDivisorCVar{};
		bool ShowReciprocal{ false };

		// BulletSpread
		bool Flattened{ false };

		// Ammo
		bool SkillValueIsClipSize{ false };
		int ClipSize{ 0 };

		// RequirementClass
		ASW_Marine_Class Class{ MARINE_CLASS_UNDEFINED };

		// Secondary
		CUtlVectorAutoPurge<WeaponFact *> Facts{};

	private:
		friend struct Helpers;
		bool ReadFromFile( const char *, KeyValues * );
		bool IsSame( const WeaponFact * ) const;
		void Merge( const WeaponFact * );
	};
}
