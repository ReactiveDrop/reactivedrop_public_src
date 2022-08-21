#include "cbase.h"
#include "rd_swarmopedia.h"
#include "asw_util_shared.h"
#include "rd_workshop.h"

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
}

void Collection::ReadFromFiles()
{
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
			Helpers::AddMerge( Aliens, pszPath, pEntry );
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
	FOR_EACH_VEC( copy.StatNames, i )
	{
		StatNames.CopyAndAddToTail( copy.StatNames[i] );
	}
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
