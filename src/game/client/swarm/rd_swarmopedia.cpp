#include "cbase.h"
#include "rd_swarmopedia.h"
#include "asw_util_shared.h"
#include "rd_workshop.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "asw_weapon_shared.h"
#include "ammodef.h"
#include "asw_ammo_drop_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define SWARMOPEDIA_PATH "resource/swarmopedia.txt"

using namespace RD_Swarmopedia;

template<typename T>
T *Helpers::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	T *p = new T();

	if ( p->ReadFromFile( pszPath, pKV ) )
	{
		return p;
	}

	delete p;

	return NULL;
}

template<typename T>
void Helpers::CopyAddVector( CUtlVectorAutoPurge<T *> &dst, const CUtlVectorAutoPurge<T *> &src )
{
	dst.EnsureCapacity( dst.Count() + src.Count() );

	FOR_EACH_VEC( src, i )
	{
		dst.AddToTail( new T( *src[i] ) );
	}
}

template<typename T>
void Helpers::CopyVector( CUtlVectorAutoPurge<T *> &dst, const CUtlVectorAutoPurge<T *> &src )
{
	Assert( dst.Count() == 0 );

	CopyAddVector( dst, src );
}

void Helpers::CopyVector( CUtlStringList &dst, const CUtlStringList &src )
{
	Assert( dst.Count() == 0 );

	dst.EnsureCapacity( src.Count() );
	FOR_EACH_VEC( src, i )
	{
		dst.CopyAndAddToTail( src[i] );
	}
}

template<typename T>
void Helpers::AddMerge( CUtlVectorAutoPurge<T *> &dst, const char *pszPath, KeyValues *pKV )
{
	if ( T *p = ReadFromFile<T>( pszPath, pKV ) )
	{
		AddMerge( dst, p );
	}
}

template<typename T>
void Helpers::AddMerge( CUtlVectorAutoPurge<T *> &dst, T *pElement )
{
	FOR_EACH_VEC( dst, i )
	{
		if ( dst[i]->IsSame( pElement ) )
		{
			dst[i]->Merge( pElement );

			delete pElement;

			return;
		}
	}

	dst.AddToTail( pElement );
}

template<typename T>
void Helpers::CopyUniqueVector( CUtlVectorAutoPurge<T *> &dst, const CUtlVectorAutoPurge<T *> &src )
{
	FOR_EACH_VEC( src, i )
	{
		AddMerge( dst, new T( *src[i] ) );
	}
}

Collection::Collection( const Collection &copy )
{
	Helpers::CopyVector( Aliens, copy.Aliens );
	Helpers::CopyVector( Weapons, copy.Weapons );
}

void Collection::ReadFromFiles( Subset subset )
{
	ReadSubset = subset;
	UTIL_RD_LoadAllKeyValues( SWARMOPEDIA_PATH, "GAME", "Swarmopedia", &ReadHelper, this );
}

void Collection::ReadHelper( const char *pszPath, KeyValues *pKV, void *pUserData )
{
	static_cast< Collection * >( pUserData )->ReadFromFile( pszPath, pKV );
}

void Collection::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	FOR_EACH_SUBKEY( pKV, pEntry )
	{
		if ( FStrEq( pEntry->GetName(), "ALIEN" ) )
		{
			if ( int( ReadSubset ) & int( Subset::Aliens ) )
			{
				Helpers::AddMerge( Aliens, pszPath, pEntry );
			}
		}
		else if ( FStrEq( pEntry->GetName(), "WEAPON" ) )
		{
			if ( int( ReadSubset ) & int( Subset::Weapons ) )
			{
				if ( Weapon *pWeapon = Helpers::ReadFromFile<Weapon>( pszPath, pEntry ) )
				{
					if ( int( ReadSubset ) & int( pWeapon->Extra ? Subset::ExtraWeapons : Subset::RegularWeapons ) )
					{
						Helpers::AddMerge( Weapons, pWeapon );
					}
					else
					{
						delete pWeapon;
					}
				}
			}
		}
		else
		{
			Warning( "Swarmopedia: unexpected entry type '%s' in %s\n", pEntry->GetName(), pszPath );
		}
	}
}

Alien::Alien( const Alien &copy ) :
	ID{ copy.ID },
	Name{ copy.Name },
	Icon{ copy.Icon }
{
	Helpers::CopyVector( Requirements, copy.Requirements );
	Helpers::CopyVector( GlobalStats, copy.GlobalStats );
	Helpers::CopyVector( Display, copy.Display );
	Helpers::CopyVector( Abilities, copy.Abilities );
	Helpers::CopyVector( Content, copy.Content );
	Sources = copy.Sources;
}

float Alien::GetOverallRequirementProgress() const
{
	float flTotal = 0.0f;

	FOR_EACH_VEC( Requirements, i )
	{
		flTotal += Requirements[i]->GetProgress();
	}

	return flTotal / Requirements.Count();
}

bool Alien::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	ID = pKV->GetString( "ID" );
	if ( ID.IsEmpty() )
	{
		Warning( "Swarmopedia: ALIEN entry missing ID in %s\n", pszPath );
		return false;
	}

	Sources.AddToTail( g_ReactiveDropWorkshop.AddonForFileSystemPath( pszPath ) );

	bool bSeenID = false;
	FOR_EACH_SUBKEY( pKV, pSubKey )
	{
		const char *szName = pSubKey->GetName();
		if ( FStrEq( szName, "ID" ) )
		{
			if ( bSeenID )
			{
				Warning( "Swarmopedia: ALIEN entry '%s' has multiple ID fields in %s\n", ID.Get(), pszPath );
				return false;
			}

			bSeenID = true;
		}
		else if ( FStrEq( szName, "Name" ) )
		{
			if ( Name.IsEmpty() )
			{
				Name = pSubKey->GetString();
			}
		}
		else if ( FStrEq( szName, "Icon" ) )
		{
			if ( Icon.IsEmpty() )
			{
				Icon = pSubKey->GetString();
			}
		}
		else if ( FStrEq( szName, "SteamStatLoreRequirement" ) )
		{
			Helpers::AddMerge( Requirements, pszPath, pSubKey );
		}
		else if ( FStrEq( szName, "GlobalStat" ) )
		{
			Helpers::AddMerge( GlobalStats, pszPath, pSubKey );
		}
		else if ( FStrEq( szName, "Display" ) )
		{
			Helpers::AddMerge( Display, pszPath, pSubKey );
		}
		else if ( FStrEq( szName, "Ability" ) )
		{
			Ability *a = new Ability();
			a->Caption = pSubKey->GetString();
			Helpers::AddMerge( Abilities, a );
		}
		else if ( FStrEq( szName, "Paragraph" ) )
		{
			Helpers::AddMerge( Content, pszPath, pSubKey );
		}
		else if ( FStrEq( szName, "AppearsIn" ) )
		{
			// TODO: decide whether we want to use this; the data was very incomplete last time we tried
		}
		else
		{
			Warning( "Swarmopedia: Unhandled key '%s' in ALIEN '%s' in %s\n", szName, ID.Get(), pszPath );
		}
	}

	return true;
}

bool Alien::IsSame( const Alien *pAlien ) const
{
	return ID == pAlien->ID;
}

void Alien::Merge( const Alien *pAlien )
{
	Assert( ID == pAlien->ID );

	// first value for Name/Icon wins.
	if ( Name.IsEmpty() )
	{
		Name = pAlien->Name;
	}
	if ( Icon.IsEmpty() )
	{
		Icon = pAlien->Icon;
	}

	// everything else gets merged.
	Helpers::CopyUniqueVector( Requirements, pAlien->Requirements );
	Helpers::CopyUniqueVector( GlobalStats, pAlien->GlobalStats );
	Helpers::CopyUniqueVector( Display, pAlien->Display );
	Helpers::CopyUniqueVector( Abilities, pAlien->Abilities );
	Helpers::CopyUniqueVector( Content, pAlien->Content );
}

Requirement::Requirement( const Requirement &copy ) :
	Type{ copy.Type },
	Caption{ copy.Caption },
	MinValue{ copy.MinValue }
{
	Helpers::CopyVector( StatNames, copy.StatNames );
}

float Requirement::GetProgress() const
{
	if ( Type == Type_t::SteamStat )
	{
		if ( MinValue <= 0 )
		{
			return 1.0f;
		}

		int iStatTotal{};
		FOR_EACH_VEC( StatNames, i )
		{
			int iStat{};
			if ( !SteamUserStats() || !SteamUserStats()->GetStat( StatNames[i], &iStat ) )
			{
				DevWarning( "Failed to retrieve stat '%s' for Swarmopedia\n", StatNames[i] );
			}

			iStatTotal += iStat;
		}

		return clamp( ( float )iStatTotal / ( float )MinValue, 0.0f, 1.0f );
	}

	Assert( !"Unhandled Swarmopedia requirement type" );
	return 0.0f;
}

bool Requirement::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	Type = Type_t::SteamStat;

	Caption = pKV->GetString( "Caption" );
	MinValue = pKV->GetInt( "MinValue" );

	FOR_EACH_VALUE( pKV, pValue )
	{
		const char *szName = pValue->GetName();
		if ( FStrEq( szName, "Caption" ) || FStrEq( szName, "MinValue" ) )
		{
			// Already handled.
		}
		else if ( FStrEq( szName, "StatName" ) )
		{
			StatNames.CopyAndAddToTail( pValue->GetString() );
		}
		else
		{
			DevWarning( "Unhandled field in Swarmopedia '%s' in %s\n", pKV->GetName(), pszPath );
		}
	}

	return true;
}

bool Requirement::IsSame( const Requirement *pRequirement ) const
{
	return false;
}

void Requirement::Merge( const Requirement *pRequirement )
{
	Assert( !"RD_Swarmopedia::Requirement::Merge should not have been called." );
}

Ability::Ability( const Ability &copy ) :
	Caption{ copy.Caption }
{
}

bool Ability::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	Assert( FStrEq( pKV->GetName(), "Ability" ) );

	Caption = pKV->GetString();

	return true;
}

bool Ability::IsSame( const Ability *pAbility ) const
{
	return Caption == pAbility->Caption;
}

void Ability::Merge( const Ability *pAbility )
{
	Assert( Caption == pAbility->Caption );
}

GlobalStat::GlobalStat( const GlobalStat &copy ) :
	StatName{ copy.StatName },
	Caption{ copy.Caption }
{
}

bool GlobalStat::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	StatName = pKV->GetString( "StatName" );
	if ( StatName.IsEmpty() )
	{
		Warning( "Swarmopedia: GlobalStat missing StatName in %s\n", pszPath );
		return false;
	}

	Caption = pKV->GetString( "Caption" );

	return true;
}

bool GlobalStat::IsSame( const GlobalStat *pGlobalStat ) const
{
	return StatName == pGlobalStat->StatName;
}

void GlobalStat::Merge( const GlobalStat *pGlobalStat )
{
	Assert( StatName == pGlobalStat->StatName );

	if ( Caption.IsEmpty() )
	{
		Caption = pGlobalStat->Caption;
	}
}

Display::Display( const Display &copy ) :
	Caption{ copy.Caption },
	LightingState{ copy.LightingState }
{
	Helpers::CopyVector( Models, copy.Models );
}

static Vector ColorToVector( Color c )
{
	return Vector{ c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f };
}

static void FixLightCoordSystem( Vector &vec )
{
	// The Swarmopedia camera faces east, but everything else in the game faces north.
	// Do a trivial coordinate system swap:
	float flTemp = vec.x;
	vec.x = vec.y;
	vec.y = -flTemp;
}

static void SetupLightAttenuation( const char *pszPath, LightDesc_t &light, KeyValues *pKV )
{
	bool bOldAtten = pKV->FindKey( "Constant" ) || pKV->FindKey( "Linear" ) || pKV->FindKey( "Quadratic" );
	bool bNewAtten = pKV->FindKey( "ZeroPercent" ) || pKV->FindKey( "FiftyPercent" );

	if ( bOldAtten && bNewAtten )
	{
		Warning( "Swarmopedia: cannot define both CLQ light attenuation and percentage light attenuation for the same light in %s\n", pszPath );
		return;
	}

	if ( bOldAtten )
	{
		light.SetupOldStyleAttenuation( pKV->GetFloat( "Quadratic" ), pKV->GetFloat( "Linear" ), pKV->GetFloat( "Constant" ) );
	}
	else if ( bNewAtten )
	{
		light.SetupNewStyleAttenuation( pKV->GetFloat( "FiftyPercent" ), pKV->GetFloat( "ZeroPercent" ) );
	}
}

const MaterialLightingState_t &SwarmopediaDefaultLightingState()
{
	static MaterialLightingState_t s_State{};
	static bool s_bInit = false;
	if ( !s_bInit )
	{
		s_bInit = true;

		s_State.m_vecAmbientCube[0].Init( 24.0f / 255.0f, 24.0f / 255.0f, 24.0f / 255.0f );
		s_State.m_vecAmbientCube[1] = s_State.m_vecAmbientCube[0];
		s_State.m_vecAmbientCube[2] = s_State.m_vecAmbientCube[0];
		s_State.m_vecAmbientCube[3] = s_State.m_vecAmbientCube[0];
		s_State.m_vecAmbientCube[4] = s_State.m_vecAmbientCube[0];
		s_State.m_vecAmbientCube[5] = s_State.m_vecAmbientCube[0];
		s_State.m_vecLightingOrigin.Init();

		s_State.m_nLocalLightCount = 3;
		s_State.m_pLocalLightDesc[0].InitPoint( Vector{ -128.0f, -64.0f, 96.0f }, Vector{ 3.0f, 3.0f, 3.0f } );
		s_State.m_pLocalLightDesc[1].InitPoint( Vector{ 0.0f, 256.0f, 48.0f }, Vector{ 1.5f, 1.5f, 96.0f / 255.0f } );
		s_State.m_pLocalLightDesc[2].InitDirectional( Vector{ -128, 192, 16 }.Normalized(), Vector{ 5.0f, 5.0f, 5.0f } );
	}

	return s_State;
}

bool Display::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	Caption = pKV->GetString( "Caption" );

	FOR_EACH_SUBKEY( pKV, pSubKey )
	{
		const char *szName = pSubKey->GetName();
		if ( FStrEq( szName, "Model" ) )
		{
			Helpers::AddMerge( Models, pszPath, pSubKey );
		}
		else if ( FStrEq( szName, "LightingPreset" ) )
		{
			if ( pSubKey->GetInt() == 1 )
			{
				LightingState = SwarmopediaDefaultLightingState();
			}
			else
			{
				Warning( "Swarmopedia: Unknown lighting preset value (only 1 is currently supported) in %s\n", pszPath );
			}
		}
		else if ( FStrEq( szName, "LightingOriginX" ) )
		{
			LightingState.m_vecLightingOrigin.x = pSubKey->GetFloat();
		}
		else if ( FStrEq( szName, "LightingOriginY" ) )
		{
			LightingState.m_vecLightingOrigin.y = pSubKey->GetFloat();
		}
		else if ( FStrEq( szName, "LightingOriginZ" ) )
		{
			LightingState.m_vecLightingOrigin.z = pSubKey->GetFloat();
		}
		else if ( FStrEq( szName, "Ambient" ) )
		{
			Vector ambient = ColorToVector( pSubKey->GetColor() );
			LightingState.m_vecAmbientCube[0] = ambient;
			LightingState.m_vecAmbientCube[1] = ambient;
			LightingState.m_vecAmbientCube[2] = ambient;
			LightingState.m_vecAmbientCube[3] = ambient;
			LightingState.m_vecAmbientCube[4] = ambient;
			LightingState.m_vecAmbientCube[5] = ambient;
		}
		else if ( FStrEq( szName, "AmbientFar" ) )
		{
			LightingState.m_vecAmbientCube[0] = ColorToVector( pSubKey->GetColor() );
		}
		else if ( FStrEq( szName, "AmbientNear" ) )
		{
			LightingState.m_vecAmbientCube[1] = ColorToVector( pSubKey->GetColor() );
		}
		else if ( FStrEq( szName, "AmbientLeft" ) )
		{
			LightingState.m_vecAmbientCube[2] = ColorToVector( pSubKey->GetColor() );
		}
		else if ( FStrEq( szName, "AmbientRight" ) )
		{
			LightingState.m_vecAmbientCube[3] = ColorToVector( pSubKey->GetColor() );
		}
		else if ( FStrEq( szName, "AmbientTop" ) )
		{
			LightingState.m_vecAmbientCube[4] = ColorToVector( pSubKey->GetColor() );
		}
		else if ( FStrEq( szName, "AmbientBottom" ) )
		{
			LightingState.m_vecAmbientCube[5] = ColorToVector( pSubKey->GetColor() );
		}
		else if ( FStrEq( szName, "LightPoint" ) )
		{
			if ( LightingState.m_nLocalLightCount >= MATERIAL_MAX_LIGHT_COUNT )
			{
				Warning( "Swarmopedia: too many lights (max %d per Display) in %s\n", MATERIAL_MAX_LIGHT_COUNT, pszPath );
				continue;
			}

			Vector vecPos{ pSubKey->GetFloat( "X" ), pSubKey->GetFloat( "Y" ), pSubKey->GetFloat( "Z" ) };
			Color color = pSubKey->GetColor( "Color", Color{ 255, 255, 255, 255 } );

			FixLightCoordSystem( vecPos );

			int iLight = LightingState.m_nLocalLightCount++;
			LightingState.m_pLocalLightDesc[iLight].InitPoint( vecPos, ColorToVector( color ) * pSubKey->GetFloat( "Boost", 1.0f ) );
			SetupLightAttenuation( pszPath, LightingState.m_pLocalLightDesc[iLight], pSubKey );
		}
		else if ( FStrEq( szName, "LightDirectional" ) )
		{
			if ( LightingState.m_nLocalLightCount >= MATERIAL_MAX_LIGHT_COUNT )
			{
				Warning( "Swarmopedia: too many lights (max %d per Display) in %s\n", MATERIAL_MAX_LIGHT_COUNT, pszPath );
				continue;
			}

			Vector vecDir{ pSubKey->GetFloat( "X" ), pSubKey->GetFloat( "Y" ), pSubKey->GetFloat( "Z" ) };
			Color color = pSubKey->GetColor( "Color", Color{ 255, 255, 255, 255 } );

			FixLightCoordSystem( vecDir );
			VectorNormalizeFast( vecDir );

			int iLight = LightingState.m_nLocalLightCount++;
			LightingState.m_pLocalLightDesc[iLight].InitDirectional( vecDir, ColorToVector( color ) * pSubKey->GetFloat( "Boost", 1.0f ) );
		}
		else if ( FStrEq( szName, "LightSpot" ) )
		{
			if ( LightingState.m_nLocalLightCount >= MATERIAL_MAX_LIGHT_COUNT )
			{
				Warning( "Swarmopedia: too many lights (max %d per Display) in %s\n", MATERIAL_MAX_LIGHT_COUNT, pszPath );
				continue;
			}

			Vector vecPos{ pSubKey->GetFloat( "X" ), pSubKey->GetFloat( "Y" ), pSubKey->GetFloat( "Z" ) };
			Color color = pSubKey->GetColor( "Color", Color{ 255, 255, 255, 255 } );
			Vector vecPointAt{ pSubKey->GetFloat( "TargetX" ), pSubKey->GetFloat( "TargetY" ), pSubKey->GetFloat( "TargetZ" ) };
			float flInnerCone = DEG2RAD( pSubKey->GetFloat( "InnerCone", 45.0f ) );
			float flOuterCone = DEG2RAD( pSubKey->GetFloat( "OuterCone", 60.0f ) );

			FixLightCoordSystem( vecPos );
			FixLightCoordSystem( vecPointAt );

			int iLight = LightingState.m_nLocalLightCount++;
			LightingState.m_pLocalLightDesc[iLight].InitSpot( vecPos, ColorToVector( color ) * pSubKey->GetFloat( "Boost", 1.0f ), vecPointAt, flInnerCone, flOuterCone );
			SetupLightAttenuation( pszPath, LightingState.m_pLocalLightDesc[iLight], pSubKey );
		}
	}

	if ( Models.Count() == 0 )
	{
		Warning( "Swarmopedia: Display contains no valid Model in %s\n", pszPath );

		return false;
	}

	return true;
}

bool Display::IsSame( const Display *pDisplay ) const
{
	return false;
}

void Display::Merge( const Display *pDisplay )
{
	Assert( !"RD_Swarmopedia::Display::Merge should not have been called." );
}

Model::Model( const Model &copy ) :
	ModelName{ copy.ModelName },
	Animation{ copy.Animation },
	Skin{ copy.Skin },
	Color{ copy.Color },
	Pitch{ copy.Pitch },
	Yaw{ copy.Yaw },
	Roll{ copy.Roll },
	X{ copy.X },
	Y{ copy.Y },
	Z{ copy.Z },
	Scale{ copy.Scale }
{
	BodyGroups.EnsureCapacity( copy.BodyGroups.Count() );

	FOR_EACH_MAP_FAST( copy.BodyGroups, i )
	{
		BodyGroups.Insert( copy.BodyGroups.Key( i ), copy.BodyGroups.Element( i ) );
	}
}

bool Model::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	ModelName = pKV->GetString( "ModelName" );
	if ( ModelName.IsEmpty() )
	{
		Warning( "Swarmopedia: Missing ModelName for Model in %s\n", pszPath );
		return false;
	}

	Animation = pKV->GetString( "Animation" );
	if ( Animation.IsEmpty() )
	{
		Warning( "Swarmopedia: Missing Animation for Model in %s\n", pszPath );
		return false;
	}

	Skin = pKV->GetInt( "Skin", Skin );
	Color = pKV->GetColor( "Color", Color );
	Pitch = pKV->GetFloat( "Pitch", Pitch );
	Yaw = pKV->GetFloat( "Yaw", Yaw );
	Roll = pKV->GetFloat( "Roll", Roll );
	X = pKV->GetFloat( "X", X );
	Y = pKV->GetFloat( "Y", Y );
	Z = pKV->GetFloat( "Z", Z );
	Scale = pKV->GetFloat( "Scale", Scale );

	FOR_EACH_SUBKEY( pKV, pSubKey )
	{
		if ( FStrEq( pSubKey->GetName(), "BodyGroup" ) )
		{
			BodyGroups.Insert( pSubKey->GetInt( "Group" ), pSubKey->GetInt( "Value" ) );
		}
		else if ( FStrEq( pSubKey->GetName(), "BodyGroups" ) )
		{
			FOR_EACH_VALUE( pSubKey, pGroup )
			{
				BodyGroups.Insert( atoi( pGroup->GetName() ), pGroup->GetInt() );
			}
		}
	}

	return true;
}

bool Model::IsSame( const Model *pModel ) const
{
	return false;
}

void Model::Merge( const Model *pModel )
{
	Assert( !"RD_Swarmopedia::Model::Merge should not have been called." );
}

Content::Content( const Content &copy ) :
	Type{ copy.Type },
	Text{ copy.Text },
	Color{ copy.Color }
{
}

bool Content::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	if ( FStrEq( pKV->GetName(), "Paragraph" ) )
	{
		Type = Type_t::Paragraph;
		if ( pKV->GetDataType() == KeyValues::TYPE_NONE )
		{
			Text = pKV->GetString( "Text" );
			Color = pKV->GetColor( "Color", Color );
		}
		else
		{
			Text = pKV->GetString();
		}

		return true;
	}

	return false;
}

bool Content::IsSame( const Content *pContent ) const
{
	return false;
}

void Content::Merge( const Content *pContent )
{
	Assert( !"RD_Swarmopedia::Content::Merge should not have been called." );
}

Weapon::Weapon( const Weapon &copy ) :
	ClassName{ copy.ClassName },
	EquipIndex{ copy.EquipIndex },
	Name{ copy.Name },
	Icon{ copy.Icon },
	RequiredClass{ copy.RequiredClass },
	RequiredLevel{ copy.RequiredLevel },
	Builtin{ copy.Builtin },
	Extra{ copy.Extra },
	Unique{ copy.Unique },
	Hidden{ copy.Hidden }
{
	Helpers::CopyVector( GlobalStats, copy.GlobalStats );
	Helpers::CopyVector( Display, copy.Display );
	Helpers::CopyVector( Abilities, copy.Abilities );
	Helpers::CopyVector( Content, copy.Content );
	Helpers::CopyVector( Facts, copy.Facts );
	Sources = copy.Sources;
}

static void PostProcessBuiltin( WeaponFact *pFact, CASW_WeaponInfo *pWeaponInfo, bool bIsSecondary )
{
	if ( !pFact->UseWeaponInfo )
	{
		return;
	}

	if ( pFact->Type == WeaponFact::Type_T::DamagePerShot || pFact->Type == WeaponFact::Type_T::Ammo )
	{
		const char *szSuffix = NULL;
		int iDamageType = GetAmmoDef()->DamageType( bIsSecondary ? pWeaponInfo->iAmmo2Type : pWeaponInfo->iAmmoType );
		switch ( iDamageType )
		{
		case 0:
			break;
		case DMG_BULLET:
			szSuffix = "_bullet";
			break;
		case DMG_SLASH:
			szSuffix = "_slash";
			break;
		case DMG_BURN:
			szSuffix = "_burn";
			break;
		case DMG_BLAST:
			szSuffix = "_blast";
			break;
		case DMG_SHOCK:
			szSuffix = "_shock";
			break;
		case DMG_SONIC:
			// special case; untyped or non-damaging
			break;
		case DMG_ENERGYBEAM:
			szSuffix = "_beam";
			break;
		case DMG_NERVEGAS:
			szSuffix = "_gas";
			break;
		case DMG_DISSOLVE:
			szSuffix = "_dissolve";
			break;
		case DMG_BULLET | DMG_BUCKSHOT:
			szSuffix = "_buckshot";
			break;
		default:
			Warning( "Swarmopedia: unhandled damage type %d (%s)\n", iDamageType, bIsSecondary ? pWeaponInfo->szAmmo2 : pWeaponInfo->szAmmo1 );
			DebuggerBreakIfDebugging();
			break;
		}

		if ( szSuffix && pFact->Caption.IsEmpty() )
		{
			pFact->Caption = pFact->Type == WeaponFact::Type_T::DamagePerShot ? "#rd_weapon_fact_damage_per_shot" : "#rd_weapon_fact_ammo";
			pFact->Caption += szSuffix;
		}

		if ( szSuffix && pFact->Icon.IsEmpty() )
		{
			pFact->Icon = pFact->Type == WeaponFact::Type_T::DamagePerShot ? "swarm/swarmopedia/fact/damage" : "swarm/swarmopedia/fact/ammo";
			pFact->Icon += szSuffix;
		}
	}

	switch ( pFact->Type )
	{
	case WeaponFact::Type_T::Generic:
		break;
	case WeaponFact::Type_T::Numeric:
		break;
	case WeaponFact::Type_T::HammerUnits:
		break;
	case WeaponFact::Type_T::ShotgunPellets:
		pFact->Base += pWeaponInfo->m_iNumPellets;
		break;
	case WeaponFact::Type_T::DamagePerShot:
		pFact->Base += pWeaponInfo->m_flBaseDamage;
		break;
	case WeaponFact::Type_T::LargeAlienDamageScale:
		break;
	case WeaponFact::Type_T::BulletSpread:
		break;
	case WeaponFact::Type_T::Piercing:
		break;
	case WeaponFact::Type_T::FireRate:
		//pFact->Base += bIsSecondary ? pWeaponInfo->m_flSecondaryFireRate : pWeaponInfo->m_flFireRate;
		pFact->Base += pWeaponInfo->m_flFireRate;
		break;
	case WeaponFact::Type_T::Ammo:
		pFact->Base += bIsSecondary ? pWeaponInfo->iDefaultClip2 : pWeaponInfo->iDefaultClip1;

		if ( pFact->ClipSize == 0 )
		{
			pFact->ClipSize = bIsSecondary ? pWeaponInfo->iMaxClip2 : pWeaponInfo->iMaxClip1;
		}

		// TODO: figure out where "AR2" ammo type has its max carry ignored normally
		if ( pFact->ClipSize == WEAPON_NOCLIP || bIsSecondary || pWeaponInfo->iAmmoType <= 1 )
		{
			pFact->ClipSize = 0;
		}
		else
		{
			Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex( pWeaponInfo->iAmmoType );
			if ( pAmmo->pMaxCarry == USE_CVAR )
			{
				pFact->CVar = pAmmo->pMaxCarryCVar->GetName();
			}
			else
			{
				Assert( pAmmo->pMaxCarry != INFINITE_AMMO );
				pFact->Base += pAmmo->pMaxCarry;
			}
		}

		break;
	case WeaponFact::Type_T::Secondary:
		pFact->Caption += pWeaponInfo->szAltFireText;

		FOR_EACH_VEC( pFact->Facts, i )
		{
			PostProcessBuiltin( pFact->Facts[i], pWeaponInfo, true );
		}
		break;
	case WeaponFact::Type_T::Deployed:
		FOR_EACH_VEC( pFact->Facts, i )
		{
			PostProcessBuiltin( pFact->Facts[i], pWeaponInfo, bIsSecondary );
		}
		break;
	case WeaponFact::Type_T::RequirementLevel:
		break;
	case WeaponFact::Type_T::RequirementClass:
		break;
	}
}

bool Weapon::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	ClassName = pKV->GetString( "ClassName" );
	if ( ClassName.IsEmpty() )
	{
		Warning( "Swarmopedia: WEAPON entry missing ClassName in %s\n", pszPath );
		DebuggerBreakIfDebugging();
		return false;
	}

	Sources.AddToTail( g_ReactiveDropWorkshop.AddonForFileSystemPath( pszPath ) );

	Builtin = pKV->GetBool( "Builtin" );
	if ( Builtin )
	{
		CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( ClassName );
		Assert( pWeaponInfo && pWeaponInfo->szClassName[0] != '\0' );
		if ( !pWeaponInfo || pWeaponInfo->szClassName[0] == '\0' )
		{
			Warning( "Swarmopedia: no data for builtin weapon %s in %s\n", ClassName.Get(), pszPath );
			DebuggerBreakIfDebugging();
			return false;
		}

		Icon = pWeaponInfo->szEquipIcon;
		RequiredLevel = GetWeaponLevelRequirement( ClassName ) + 1;

		if ( pWeaponInfo->m_bSapper )
		{
			RequiredClass = MARINE_CLASS_NCO;
		}
		else if ( pWeaponInfo->m_bSpecialWeapons )
		{
			RequiredClass = MARINE_CLASS_SPECIAL_WEAPONS;
		}
		else if ( pWeaponInfo->m_bFirstAid )
		{
			RequiredClass = MARINE_CLASS_MEDIC;
		}
		else if ( pWeaponInfo->m_bTech )
		{
			RequiredClass = MARINE_CLASS_TECH;
		}

		Extra = pWeaponInfo->m_bExtra;
		Unique = pWeaponInfo->m_bUnique;

		if ( Extra )
		{
			EquipIndex = g_ASWEquipmentList.GetExtraIndex( ClassName );
			Hidden = EquipIndex == -1 || !g_ASWEquipmentList.GetExtra( EquipIndex )->m_bSelectableInBriefing;
		}
		else
		{
			EquipIndex = g_ASWEquipmentList.GetRegularIndex( ClassName );
			Hidden = EquipIndex == -1 || !g_ASWEquipmentList.GetRegular( EquipIndex )->m_bSelectableInBriefing;
		}

		Assert( EquipIndex != -1 );

		if ( RequiredLevel )
		{
			Helpers::AddMerge( Facts, "INTERNAL", KeyValues::AutoDeleteInline( new KeyValues( "RequirementLevel", "Base", RequiredLevel ) ) );
		}

		if ( RequiredClass != MARINE_CLASS_UNDEFINED )
		{
			Helpers::AddMerge( Facts, "INTERNAL", KeyValues::AutoDeleteInline( new KeyValues( "RequirementClass", "Class", ClassToString( RequiredClass ) ) ) );
		}

		if ( Unique )
		{
			Helpers::AddMerge( Facts, "INTERNAL", KeyValues::AutoDeleteInline( new KeyValues( "Generic", "Icon", "swarm/swarmopedia/fact/unique", "Caption", "#rd_weapon_fact_unique" ) ) );
		}

		Name = pWeaponInfo->szPrintName;

		RD_Swarmopedia::Display *pDisplay = new RD_Swarmopedia::Display{};
		Display.AddToTail( pDisplay );
		pDisplay->Caption = pWeaponInfo->szEquipLongName;

		pDisplay->LightingState = SwarmopediaDefaultLightingState();

		int i = pDisplay->Models.AddToTail( new Model{} );
		if ( pWeaponInfo->szDisplayModel[0] )
		{
			pDisplay->Models[i]->ModelName = pWeaponInfo->szDisplayModel;
			if ( pWeaponInfo->szDisplayModel2[0] )
			{
				int j = pDisplay->Models.AddToTail( new Model{} );
				pDisplay->Models[j]->ModelName = pWeaponInfo->szDisplayModel2;
			}
		}
		else
		{
			pDisplay->Models[i]->ModelName = pWeaponInfo->szWorldModel;
		}

		if ( pWeaponInfo->m_iDisplayModelSkin > 0 )
		{
			pDisplay->Models[i]->Skin = pWeaponInfo->m_iDisplayModelSkin;
		}
		else
		{
			pDisplay->Models[i]->Skin = pWeaponInfo->m_iPlayerModelSkin;
		}

		if ( KeyValues *pTransform = pKV->FindKey( "BuiltinModelTransform" ) )
		{
			FOR_EACH_VEC( pDisplay->Models, j )
			{
				Model *pModel = pDisplay->Models[j];
				pModel->Color = pTransform->GetColor( "Color", pModel->Color );
				pModel->Pitch = pTransform->GetFloat( "Pitch", pModel->Pitch );
				pModel->Yaw = pTransform->GetFloat( "Yaw", pModel->Yaw );
				pModel->Roll = pTransform->GetFloat( "Roll", pModel->Roll );
				pModel->X = pTransform->GetFloat( "X", pModel->X );
				pModel->Y = pTransform->GetFloat( "Y", pModel->Y );
				pModel->Z = pTransform->GetFloat( "Z", pModel->Z );
				pModel->Scale = pTransform->GetFloat( "Scale", pModel->Scale );
			}
		}

		if ( pWeaponInfo->szAttributesText[0] != '\0' )
		{
			Ability *a = new Ability();
			a->Caption = pWeaponInfo->szAttributesText;
			Helpers::AddMerge( Abilities, a );
		}

		RD_Swarmopedia::Content *pContent = new RD_Swarmopedia::Content{};
		Content.AddToTail( pContent );
		pContent->Text = pWeaponInfo->szEquipDescription1;
		pContent->Color = Color{ 255, 255, 255, 255 };
	}

	FOR_EACH_SUBKEY( pKV, pSubKey )
	{
		const char *szName = pSubKey->GetName();
		if ( FStrEq( szName, "GlobalStat" ) )
		{
			Helpers::AddMerge( GlobalStats, pszPath, pSubKey );
		}
		else if ( FStrEq( szName, "Display" ) )
		{
			Helpers::AddMerge( Display, pszPath, pSubKey );
		}
		else if ( FStrEq( szName, "Ability" ) )
		{
			Ability *a = new Ability();
			a->Caption = pSubKey->GetString();
			Helpers::AddMerge( Abilities, a );
		}
		else if ( FStrEq( szName, "Paragraph" ) )
		{
			Helpers::AddMerge( Content, pszPath, pSubKey );
		}
	}

	if ( KeyValues *pFacts = pKV->FindKey( "Facts" ) )
	{
		FOR_EACH_SUBKEY( pFacts, pFact )
		{
			Helpers::AddMerge( Facts, pszPath, pFact );
		}
	}

	if ( Builtin )
	{
		CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( ClassName );
		Assert( pWeaponInfo && pWeaponInfo->szClassName[0] != '\0' );

		bool bWantAmmoFacts = !Extra && pWeaponInfo->iAmmoType > 1;

		FOR_EACH_VEC( Facts, i )
		{
			PostProcessBuiltin( Facts[i], pWeaponInfo, false );

			if ( Facts[i]->Type == WeaponFact::Type_T::Ammo && bWantAmmoFacts )
			{
				bWantAmmoFacts = false;

				ASSERT_INVARIANT( DEFAULT_AMMO_DROP_UNITS == 100 );
				int iUnitCost = CASW_Ammo_Drop_Shared::GetAmmoUnitCost( pWeaponInfo->iAmmoType );
				int iClipsPerRefill = CASW_Ammo_Drop_Shared::GetAmmoClipsToGive( pWeaponInfo->iAmmoType );

				int iNext = i;
				if ( iUnitCost > DEFAULT_AMMO_DROP_UNITS )
				{
					Facts.InsertAfter( iNext++, Helpers::ReadFromFile<WeaponFact>( "INTERNAL", KeyValues::AutoDeleteInline( new KeyValues( "Generic", "Icon", "swarm/swarmopedia/fact/generic_ammo_refill", "Caption", "#rd_weapon_fact_generic_ammo_refill_none" ) ) ) );
				}
				else
				{
					KeyValues::AutoDelete pFact( "Numeric" );
					pFact->SetString( "Icon", "swarm/swarmopedia/fact/generic_ammo_refill" );
					pFact->SetString( "Caption", "#rd_weapon_fact_generic_ammo_refill_cost" );
					pFact->SetInt( "Base", iUnitCost );
					Facts.InsertAfter( iNext++, Helpers::ReadFromFile<WeaponFact>( "INTERNAL", pFact ) );

					if ( iClipsPerRefill != 1 )
					{
						pFact->SetString( "Icon", "swarm/swarmopedia/fact/generic_ammo_refill_clips" );
						pFact->SetString( "Caption", "#rd_weapon_fact_generic_ammo_refill_clips" );
						pFact->SetInt( "Base", iClipsPerRefill );
						Facts.InsertAfter( iNext++, Helpers::ReadFromFile<WeaponFact>( "INTERNAL", pFact ) );
					}
				}

				if ( Facts[i]->UseWeaponInfo && Facts[i]->ClipSize )
				{
					KeyValues::AutoDelete pFact( "Numeric" );
					pFact->SetString( "Icon", "swarm/swarmopedia/fact/reload" );
					pFact->SetString( "Caption", "#rd_weapon_fact_reload" );
					pFact->SetInt( "Precision", 2 );
					pFact->SetString( "Skill", "ASW_MARINE_SKILL_RELOADING" );
					pFact->SetString( "SubSkill", "ASW_MARINE_SUBSKILL_RELOADING_SPEED_SCALE" );
					pFact->SetFloat( "SkillMultiplier", pWeaponInfo->m_flDisplayReloadTime > 0 ? pWeaponInfo->m_flDisplayReloadTime : pWeaponInfo->flReloadTime );
					Facts.InsertAfter( iNext++, Helpers::ReadFromFile<WeaponFact>( "INTERNAL", pFact ) );
				}
			}
		}
	}

	return true;
}

bool Weapon::IsSame( const Weapon *pWeapon ) const
{
	return ClassName == pWeapon->ClassName;
}

void Weapon::Merge( const Weapon *pWeapon )
{
	Assert( !"TODO" );
}

WeaponFact::WeaponFact( const WeaponFact &copy ) :
	Type{ copy.Type },
	Icon{ copy.Icon },
	Caption{ copy.Caption },
	RequireCVar{ copy.RequireCVar },
	RequireValue{ copy.RequireValue },
	HaveRequireValue{ copy.HaveRequireValue },
	UseWeaponInfo{ copy.UseWeaponInfo },
	Precision{ copy.Precision },
	Base{ copy.Base },
	MinimumValue{ copy.MinimumValue },
	MaximumValue{ copy.MaximumValue },
	CVar{ copy.CVar },
	BaseMultiplier{ copy.BaseMultiplier },
	Skill{ copy.Skill },
	SubSkill{ copy.SubSkill },
	SkillMultiplier{ copy.SkillMultiplier },
	ShowReciprocal{ copy.ShowReciprocal },
	Flattened{ copy.Flattened },
	SkillValueIsClipSize{ copy.SkillValueIsClipSize },
	ClipSize{ copy.ClipSize },
	Class{ copy.Class }
{
	Helpers::CopyVector( BaseMultiplierCVar, copy.BaseMultiplierCVar );
	Helpers::CopyVector( BaseDivisorCVar, copy.BaseDivisorCVar );
	Helpers::CopyVector( SkillMultiplierCVar, copy.SkillMultiplierCVar );
	Helpers::CopyVector( SkillDivisorCVar, copy.SkillDivisorCVar );
	Helpers::CopyVector( Facts, copy.Facts );
}

bool WeaponFact::ReadFromFile( const char *pszPath, KeyValues *pKV )
{
	const char *szName = pKV->GetName();
	if ( FStrEq( szName, "Generic" ) )
	{
		Type = Type_T::Generic;
	}
	else if ( FStrEq( szName, "Numeric" ) )
	{
		Type = Type_T::Numeric;
	}
	else if ( FStrEq( szName, "HammerUnits" ) )
	{
		Type = Type_T::HammerUnits;
	}
	else if ( FStrEq( szName, "ShotgunPellets" ) )
	{
		Type = Type_T::ShotgunPellets;
		Precision = 0;
	}
	else if ( FStrEq( szName, "DamagePerShot" ) )
	{
		Type = Type_T::DamagePerShot;
		Precision = 0;
	}
	else if ( FStrEq( szName, "LargeAlienDamageScale" ) )
	{
		Type = Type_T::LargeAlienDamageScale;
		Precision = 1;
	}
	else if ( FStrEq( szName, "BulletSpread" ) )
	{
		Type = Type_T::BulletSpread;
		Precision = 0;
	}
	else if ( FStrEq( szName, "Piercing" ) )
	{
		Type = Type_T::Piercing;
	}
	else if ( FStrEq( szName, "FireRate" ) )
	{
		Type = Type_T::FireRate;
		Precision = 2;
	}
	else if ( FStrEq( szName, "Ammo" ) )
	{
		Type = Type_T::Ammo;
		Precision = 0;
	}
	else if ( FStrEq( szName, "Secondary" ) )
	{
		Type = Type_T::Secondary;

		FOR_EACH_SUBKEY( pKV, pFact )
		{
			Helpers::AddMerge( Facts, pszPath, pFact );
		}

		return true;
	}
	else if ( FStrEq( szName, "Deployed" ) )
	{
		Type = Type_T::Deployed;

		FOR_EACH_SUBKEY( pKV, pFact )
		{
			Helpers::AddMerge( Facts, pszPath, pFact );
		}

		return true;
	}
	else if ( FStrEq( szName, "RequirementLevel" ) )
	{
		Type = Type_T::RequirementLevel;
		Precision = 0;
	}
	else if ( FStrEq( szName, "RequirementClass" ) )
	{
		Type = Type_T::RequirementClass;
	}
	else
	{
		Warning( "Swarmopedia: unhandled weapon fact type %s in %s\n", szName, pszPath );
		DebuggerBreakIfDebugging();
		return false;
	}

	Icon = pKV->GetString( "Icon" );
	Caption = pKV->GetString( "Caption" );
	RequireCVar = pKV->GetString( "RequireCVar" );
	if ( const char *szRequired = pKV->GetString( "RequireValue", NULL ) )
	{
		RequireValue = szRequired;
		HaveRequireValue = true;
	}

	UseWeaponInfo = pKV->GetBool( "UseWeaponInfo", true );
	Precision = pKV->GetInt( "Precision", Precision );

	Base = pKV->GetFloat( "Base" );
	MinimumValue = pKV->GetFloat( "MinimumValue", -FLT_MAX );
	MaximumValue = pKV->GetFloat( "MaximumValue", FLT_MAX );
	CVar = pKV->GetString( "CVar" );

	if ( const char *szSkill = pKV->GetString( "Skill", NULL ) )
	{
		Skill = SkillFromString( szSkill );
		if ( Skill == ASW_MARINE_SKILL_INVALID )
		{
			Warning( "Swarmopedia: Invalid skill %s in weapon fact in %s\n", szSkill, pszPath );
			DebuggerBreakIfDebugging();
		}
	}

	if ( const char *szSubSkill = pKV->GetString( "SubSkill", NULL ) )
	{
		if ( Skill == ASW_MARINE_SKILL_INVALID )
		{
			Warning( "Swarmopedia: Cannot define subskill %s without valid skill in weapon fact in %s\n", szSubSkill, pszPath );
			DebuggerBreakIfDebugging();
		}
		else
		{
			SubSkill = SubSkillFromString( szSubSkill, Skill );
			if ( SubSkill == -1 )
			{
				Warning( "Swarmopedia: Invalid subskill %s for skill %s in weapon fact in %s\n", szSubSkill, SkillToString( Skill ), pszPath );
				DebuggerBreakIfDebugging();
			}
		}
	}

	ShowReciprocal = pKV->GetBool( "ShowReciprocal", false );
	Flattened = pKV->GetBool( "Flattened", false );
	SkillValueIsClipSize = pKV->GetBool( "SkillValueIsClipSize", false );
	ClipSize = pKV->GetInt( "ClipSize" );

	if ( const char *szClass = pKV->GetString( "Class", NULL ) )
	{
		Class = ClassFromString( szClass );
		if ( Class == MARINE_CLASS_UNDEFINED )
		{
			Warning( "Swarmopedia: Invalid class %s in weapon fact in %s\n", szClass, pszPath );
			DebuggerBreakIfDebugging();
		}
	}

	FOR_EACH_VALUE( pKV, pValue )
	{
		szName = pValue->GetName();
		if ( FStrEq( szName, "Icon" ) ||
			FStrEq( szName, "Caption" ) ||
			FStrEq( szName, "RequireCVar" ) ||
			FStrEq( szName, "RequireValue" ) ||
			FStrEq( szName, "UseWeaponInfo" ) ||
			FStrEq( szName, "Precision" ) ||
			FStrEq( szName, "Base" ) ||
			FStrEq( szName, "MinimumValue" ) ||
			FStrEq( szName, "MaximumValue" ) ||
			FStrEq( szName, "CVar" ) ||
			FStrEq( szName, "Skill" ) ||
			FStrEq( szName, "SubSkill" ) ||
			FStrEq( szName, "ShowReciprocal" ) ||
			FStrEq( szName, "Flattened" ) ||
			FStrEq( szName, "SkillValueIsClipSize" ) ||
			FStrEq( szName, "ClipSize" ) ||
			FStrEq( szName, "Class" ) )
		{
			// handled
			continue;
		}

		if ( FStrEq( szName, "BaseMultiplier" ) )
		{
			BaseMultiplier *= pValue->GetFloat();
		}
		else if ( FStrEq( szName, "BaseDivisor" ) )
		{
			BaseMultiplier /= pValue->GetFloat();
		}
		else if ( FStrEq( szName, "BaseMultiplierCVar" ) )
		{
			BaseMultiplierCVar.CopyAndAddToTail( pValue->GetString() );
		}
		else if ( FStrEq( szName, "BaseDivisorCVar" ) )
		{
			BaseDivisorCVar.CopyAndAddToTail( pValue->GetString() );
		}
		else if ( FStrEq( szName, "SkillMultiplier" ) )
		{
			SkillMultiplier *= pValue->GetFloat();
		}
		else if ( FStrEq( szName, "SkillDivisor" ) )
		{
			SkillMultiplier /= pValue->GetFloat();
		}
		else if ( FStrEq( szName, "SkillMultiplierCVar" ) )
		{
			SkillMultiplierCVar.CopyAndAddToTail( pValue->GetString() );
		}
		else if ( FStrEq( szName, "SkillDivisorCVar" ) )
		{
			SkillDivisorCVar.CopyAndAddToTail( pValue->GetString() );
		}
		else
		{
			Warning( "Swarmopedia: unhandled weapon fact key %s in %s\n", szName, pszPath );
			DebuggerBreakIfDebugging();
		}
	}

	return true;
}

bool WeaponFact::IsSame( const WeaponFact *pWeaponFact ) const
{
	return false;
}

void WeaponFact::Merge( const WeaponFact *pWeaponFact )
{
	Assert( !"RD_Swarmopedia::WeaponFact::Merge should not have been called." );
}
