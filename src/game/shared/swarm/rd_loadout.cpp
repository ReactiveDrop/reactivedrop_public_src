#include "cbase.h"
#include "rd_loadout.h"
#include "fmtstr.h"
#ifdef CLIENT_DLL
#include "ConfigManager.h"
#include "asw_util_shared.h"
#include "c_asw_player.h"
#include "c_asw_game_resource.h"
#include "c_asw_marine_resource.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
// current loadout equip slots
ConVar asw_default_primary[ASW_NUM_MARINE_PROFILES + 1]
{
	{ "asw_default_primary_0", "-1", FCVAR_ARCHIVE, "Default primary equip for Sarge" },
	{ "asw_default_primary_1", "-1", FCVAR_ARCHIVE, "Default primary equip for Wildcat" },
	{ "asw_default_primary_2", "-1", FCVAR_ARCHIVE, "Default primary equip for Faith" },
	{ "asw_default_primary_3", "-1", FCVAR_ARCHIVE, "Default primary equip for Crash" },
	{ "asw_default_primary_4", "-1", FCVAR_ARCHIVE, "Default primary equip for Jaeger" },
	{ "asw_default_primary_5", "-1", FCVAR_ARCHIVE, "Default primary equip for Wolfe" },
	{ "asw_default_primary_6", "-1", FCVAR_ARCHIVE, "Default primary equip for Bastille" },
	{ "asw_default_primary_7", "-1", FCVAR_ARCHIVE, "Default primary equip for Vegas" },
	// we don't have Flynn, but this convar has existed for a while so keep it as FCVAR_ARCHIVE in case anyone is really attached to the number they put there
	{ "asw_default_primary_8", "-1", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Default primary equip for Flynn" },
};
ConVar asw_default_secondary[ASW_NUM_MARINE_PROFILES + 1]
{
	{ "asw_default_secondary_0", "-1", FCVAR_ARCHIVE, "Default secondary equip for Sarge" },
	{ "asw_default_secondary_1", "-1", FCVAR_ARCHIVE, "Default secondary equip for Wildcat" },
	{ "asw_default_secondary_2", "-1", FCVAR_ARCHIVE, "Default secondary equip for Faith" },
	{ "asw_default_secondary_3", "-1", FCVAR_ARCHIVE, "Default secondary equip for Crash" },
	{ "asw_default_secondary_4", "-1", FCVAR_ARCHIVE, "Default secondary equip for Jaeger" },
	{ "asw_default_secondary_5", "-1", FCVAR_ARCHIVE, "Default secondary equip for Wolfe" },
	{ "asw_default_secondary_6", "-1", FCVAR_ARCHIVE, "Default secondary equip for Bastille" },
	{ "asw_default_secondary_7", "-1", FCVAR_ARCHIVE, "Default secondary equip for Vegas" },
	{ "asw_default_secondary_8", "-1", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Default secondary equip for Flynn" },
};
ConVar asw_default_extra[ASW_NUM_MARINE_PROFILES + 1]
{
	{ "asw_default_extra_0", "-1", FCVAR_ARCHIVE, "Default extra equip for Sarge" },
	{ "asw_default_extra_1", "-1", FCVAR_ARCHIVE, "Default extra equip for Wildcat" },
	{ "asw_default_extra_2", "-1", FCVAR_ARCHIVE, "Default extra equip for Faith" },
	{ "asw_default_extra_3", "-1", FCVAR_ARCHIVE, "Default extra equip for Crash" },
	{ "asw_default_extra_4", "-1", FCVAR_ARCHIVE, "Default extra equip for Jaeger" },
	{ "asw_default_extra_5", "-1", FCVAR_ARCHIVE, "Default extra equip for Wolfe" },
	{ "asw_default_extra_6", "-1", FCVAR_ARCHIVE, "Default extra equip for Bastille" },
	{ "asw_default_extra_7", "-1", FCVAR_ARCHIVE, "Default extra equip for Vegas" },
	{ "asw_default_extra_8", "-1", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Default extra equip for Flynn" },
};
extern ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
extern ConVar rd_equipped_marine[ASW_NUM_MARINE_PROFILES];
ConVar rd_equipped_weapon_primary[ASW_NUM_MARINE_PROFILES]
{
	{ "rd_equipped_weapon_primary0", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Sarge" },
	{ "rd_equipped_weapon_primary1", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Wildcat" },
	{ "rd_equipped_weapon_primary2", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Faith" },
	{ "rd_equipped_weapon_primary3", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Crash" },
	{ "rd_equipped_weapon_primary4", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Jaeger" },
	{ "rd_equipped_weapon_primary5", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Wolfe" },
	{ "rd_equipped_weapon_primary6", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Bastille" },
	{ "rd_equipped_weapon_primary7", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current primary weapon for Vegas" },
};
ConVar rd_equipped_weapon_secondary[ASW_NUM_MARINE_PROFILES]
{
	{ "rd_equipped_weapon_secondary0", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Sarge" },
	{ "rd_equipped_weapon_secondary1", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Wildcat" },
	{ "rd_equipped_weapon_secondary2", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Faith" },
	{ "rd_equipped_weapon_secondary3", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Crash" },
	{ "rd_equipped_weapon_secondary4", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Jaeger" },
	{ "rd_equipped_weapon_secondary5", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Wolfe" },
	{ "rd_equipped_weapon_secondary6", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Bastille" },
	{ "rd_equipped_weapon_secondary7", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current secondary weapon for Vegas" },
};
ConVar rd_equipped_weapon_extra[ASW_NUM_MARINE_PROFILES]
{
	{ "rd_equipped_weapon_extra0", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Sarge" },
	{ "rd_equipped_weapon_extra1", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Wildcat" },
	{ "rd_equipped_weapon_extra2", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Faith" },
	{ "rd_equipped_weapon_extra3", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Crash" },
	{ "rd_equipped_weapon_extra4", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Jaeger" },
	{ "rd_equipped_weapon_extra5", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Wolfe" },
	{ "rd_equipped_weapon_extra6", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Bastille" },
	{ "rd_equipped_weapon_extra7", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of current extra weapon for Vegas" },
};

// preferences
ConVar rd_loadout_auto_update( "rd_loadout_auto_update", "1", FCVAR_ARCHIVE, "Should the current loadout be updated when an item is selected during briefing?" );
ConVar rd_loadout_load_medals( "rd_loadout_load_medals", "1", FCVAR_ARCHIVE, "Should medals be included when loading a saved loadout?" );
ConVar rd_loadout_track_last_used( "rd_loadout_track_last_used", "1", FCVAR_NONE, "Should loadouts remember when they were last loaded?" );

// commands
CON_COMMAND( rd_loadout_save, "Save the current loadout with the specified name." )
{
	if ( args.ArgC() != 2 )
	{
		Warning( "rd_loadout_save command malformed\n" );
		return;
	}

	if ( ReactiveDropLoadout::Save( args[1] ) )
		Msg( "Replaced saved loadout '%s'.\n", args[1] );
	else
		Msg( "New loadout saved as '%s'.\n", args[1] );
}

CON_COMMAND( rd_loadout_load, "Restore a loadout with the specified name." )
{
	if ( args.ArgC() != 2 )
	{
		Warning( "rd_loadout_load command malformed\n" );
		return;
	}

	if ( ReactiveDropLoadout::Load( args[1] ) )
		Msg( "Loadout '%s' loaded.\n", args[1] );
	else
		Msg( "No loadout named '%s'.\n", args[1] );
}

CON_COMMAND( rd_loadout_delete, "Remove a saved loadout with the specified name." )
{
	if ( ReactiveDropLoadout::Delete( args[1] ) )
		Msg( "Loadout '%s' deleted.\n", args[1] );
	else
		Msg( "No loadout named '%s'.\n", args[1] );
}
#endif

// implementation
namespace ReactiveDropLoadout
{
	const LoadoutData_t DefaultLoadout
	{
		{
			k_SteamItemInstanceIDInvalid,
			k_SteamItemInstanceIDInvalid,
			k_SteamItemInstanceIDInvalid,
		},
		{
			{ ASW_EQUIP_RIFLE, ASW_EQUIP_VINDICATOR, ASW_EQUIP_FLARES }, // Sarge
			{ ASW_EQUIP_AUTOGUN, ASW_EQUIP_RIFLE, ASW_EQUIP_FLARES }, // Wildcat
			{ ASW_EQUIP_RIFLE, ASW_EQUIP_HEAL_GRENADE, ASW_EQUIP_FLARES }, // Faith
			{ ASW_EQUIP_PRIFLE, ASW_EQUIP_AMMO_SATCHEL, ASW_EQUIP_WELDER }, // Crash
			{ ASW_EQUIP_VINDICATOR, ASW_EQUIP_SENTRY, ASW_EQUIP_MEDKIT }, // Jaeger
			{ ASW_EQUIP_AUTOGUN, ASW_EQUIP_SENTRY, ASW_EQUIP_MEDKIT }, // Wolfe
			{ ASW_EQUIP_RIFLE, ASW_EQUIP_HEAL_GRENADE, ASW_EQUIP_FLARES }, // Bastille
			{ ASW_EQUIP_PRIFLE, ASW_EQUIP_PISTOL, ASW_EQUIP_MEDKIT }, // Vegas
		},
		{
			true, true, true, true,
			true, true, true, true,
		},
	};

	static void WriteItemID( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName, SteamItemInstanceID_t id )
	{
		if ( id == 0 || id == k_SteamItemInstanceIDInvalid )
			return;

		if ( bBinary )
			pKV->SetUint64( szShortName, id );
		else
			pKV->SetString( szLongName, CFmtStr{ "%llu", id } );
	}
	static SteamItemInstanceID_t ReadItemID( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName )
	{
		if ( bBinary )
			return pKV->GetUint64( szShortName, k_SteamItemInstanceIDInvalid );

		SteamItemInstanceID_t id = strtoull( pKV->GetString( szLongName ), NULL, 10 );
		if ( id == 0 )
			return k_SteamItemInstanceIDInvalid;

		return id;
	}
	static void WriteRegularID( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName, ASW_Equip_Regular id )
	{
		Assert( id >= 0 && id < ASW_NUM_EQUIP_REGULAR );

		if ( bBinary )
			pKV->SetInt( szShortName, id );
		else
			pKV->SetString( szLongName, g_ASWEquipmentList.GetRegular( id )->m_szEquipClass );
	}
	static ASW_Equip_Regular ReadRegularID( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName )
	{
		int id;
		if ( bBinary )
			id = pKV->GetInt( szShortName, ASW_EQUIP_RIFLE );
		else
			id = g_ASWEquipmentList.GetRegularIndex( pKV->GetString( szLongName ) );

		if ( id < 0 || id >= ASW_NUM_EQUIP_REGULAR )
			return ASW_EQUIP_RIFLE;

		return ASW_Equip_Regular( id );
	}
	static void WriteExtraID( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName, ASW_Equip_Extra id )
	{
		Assert( id >= 0 && id < ASW_NUM_EQUIP_EXTRA );

		if ( bBinary )
			pKV->SetInt( szShortName, id );
		else
			pKV->SetString( szLongName, g_ASWEquipmentList.GetExtra( id )->m_szEquipClass );
	}
	static ASW_Equip_Extra ReadExtraID( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName )
	{
		int id;
		if ( bBinary )
			id = pKV->GetInt( szShortName, ASW_EQUIP_MEDKIT );
		else
			id = g_ASWEquipmentList.GetExtraIndex( pKV->GetString( szLongName ) );

		if ( id < 0 || id >= ASW_NUM_EQUIP_EXTRA )
			return ASW_EQUIP_MEDKIT;

		return ASW_Equip_Extra( id );
	}

	void LoadoutMarineData_t::ToKeyValues( KeyValues *pKV, bool bBinary ) const
	{
		WriteRegularID( pKV, bBinary, "p", "Primary", Primary );
		WriteRegularID( pKV, bBinary, "s", "Secondary", Secondary );
		WriteExtraID( pKV, bBinary, "e", "Extra", Extra );
		WriteItemID( pKV, bBinary, "mi", "Items/Suit", Suit );
		WriteItemID( pKV, bBinary, "pi", "Items/Primary", PrimaryItem );
		WriteItemID( pKV, bBinary, "si", "Items/Secondary", SecondaryItem );
		WriteItemID( pKV, bBinary, "ei", "Items/Extra", ExtraItem );
	}
	void LoadoutMarineData_t::FromKeyValues( KeyValues *pKV, bool bBinary )
	{
		Primary = ReadRegularID( pKV, bBinary, "p", "Primary" );
		Secondary = ReadRegularID( pKV, bBinary, "s", "Secondary" );
		Extra = ReadExtraID( pKV, bBinary, "e", "Extra" );
		Suit = ReadItemID( pKV, bBinary, "mi", "Items/Suit" );
		PrimaryItem = ReadItemID( pKV, bBinary, "pi", "Items/Primary" );
		SecondaryItem = ReadItemID( pKV, bBinary, "si", "Items/Secondary" );
		ExtraItem = ReadItemID( pKV, bBinary, "ei", "Items/Extra" );
	}

	static void WriteMarine( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName, const LoadoutMarineData_t &marine, bool bMarineIncluded )
	{
		if ( bMarineIncluded )
		{
			marine.ToKeyValues( pKV->FindKey( bBinary ? szShortName : szLongName, true ), bBinary );
		}
	}
	static void ReadMarine( KeyValues *pKV, bool bBinary, const char *szShortName, const char *szLongName, LoadoutMarineData_t &marine, bool &bMarineIncluded )
	{
		KeyValues *pMarine = pKV->FindKey( bBinary ? szShortName : szLongName );
		if ( pMarine )
		{
			bMarineIncluded = true;
			marine.FromKeyValues( pMarine, bBinary );
		}
		else
		{
			bMarineIncluded = false;
		}
	}

	void LoadoutData_t::ToKeyValues( KeyValues *pKV, bool bBinary ) const
	{
		// this is technically affected by the Y2K38 problem, but it'll just convert silently between negative and >2B,
		// so it's more of a Y2106 problem and I'll be 112 years old then so I probably won't remember writing this comment
		// if you're playing AS:RD in 2106 and the last modified time on your new loadout suddenly says 1970,
		// well, you've probably got more important problems
		if ( LastModified )
			pKV->SetInt( bBinary ? "mt" : "LastModified", LastModified );
		if ( LastUsed )
			pKV->SetInt( bBinary ? "at" : "LastUsed", LastUsed );

		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
			WriteItemID( pKV, bBinary, CFmtStr{ "m%d", i }, CFmtStr{ "Medals/Slot%d", i + 1 }, Medals[i] );

		WriteMarine( pKV, bBinary, "0", "Marines/Sarge", Marines[ASW_MARINE_PROFILE_SARGE], MarineIncluded[ASW_MARINE_PROFILE_SARGE] );
		WriteMarine( pKV, bBinary, "1", "Marines/Wildcat", Marines[ASW_MARINE_PROFILE_WILDCAT], MarineIncluded[ASW_MARINE_PROFILE_WILDCAT] );
		WriteMarine( pKV, bBinary, "2", "Marines/Faith", Marines[ASW_MARINE_PROFILE_FAITH], MarineIncluded[ASW_MARINE_PROFILE_FAITH] );
		WriteMarine( pKV, bBinary, "3", "Marines/Crash", Marines[ASW_MARINE_PROFILE_CRASH], MarineIncluded[ASW_MARINE_PROFILE_CRASH] );
		WriteMarine( pKV, bBinary, "4", "Marines/Jaeger", Marines[ASW_MARINE_PROFILE_JAEGER], MarineIncluded[ASW_MARINE_PROFILE_JAEGER] );
		WriteMarine( pKV, bBinary, "5", "Marines/Wolfe", Marines[ASW_MARINE_PROFILE_WOLFE], MarineIncluded[ASW_MARINE_PROFILE_WOLFE] );
		WriteMarine( pKV, bBinary, "6", "Marines/Bastille", Marines[ASW_MARINE_PROFILE_BASTILLE], MarineIncluded[ASW_MARINE_PROFILE_BASTILLE] );
		WriteMarine( pKV, bBinary, "7", "Marines/Vegas", Marines[ASW_MARINE_PROFILE_VEGAS], MarineIncluded[ASW_MARINE_PROFILE_VEGAS] );
	}
	void LoadoutData_t::FromKeyValues( KeyValues *pKV, bool bBinary )
	{
		LastModified = pKV->GetInt( bBinary ? "mt" : "LastModified" );
		LastUsed = pKV->GetInt( bBinary ? "at" : "LastUsed" );

		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
			Medals[i] = ReadItemID( pKV, bBinary, CFmtStr{ "m%d", i }, CFmtStr{ "Medals/Slot%d", i + 1 } );

		ReadMarine( pKV, bBinary, "0", "Marines/Sarge", Marines[ASW_MARINE_PROFILE_SARGE], MarineIncluded[ASW_MARINE_PROFILE_SARGE] );
		ReadMarine( pKV, bBinary, "1", "Marines/Wildcat", Marines[ASW_MARINE_PROFILE_WILDCAT], MarineIncluded[ASW_MARINE_PROFILE_WILDCAT] );
		ReadMarine( pKV, bBinary, "2", "Marines/Faith", Marines[ASW_MARINE_PROFILE_FAITH], MarineIncluded[ASW_MARINE_PROFILE_FAITH] );
		ReadMarine( pKV, bBinary, "3", "Marines/Crash", Marines[ASW_MARINE_PROFILE_CRASH], MarineIncluded[ASW_MARINE_PROFILE_CRASH] );
		ReadMarine( pKV, bBinary, "4", "Marines/Jaeger", Marines[ASW_MARINE_PROFILE_JAEGER], MarineIncluded[ASW_MARINE_PROFILE_JAEGER] );
		ReadMarine( pKV, bBinary, "5", "Marines/Wolfe", Marines[ASW_MARINE_PROFILE_WOLFE], MarineIncluded[ASW_MARINE_PROFILE_WOLFE] );
		ReadMarine( pKV, bBinary, "6", "Marines/Bastille", Marines[ASW_MARINE_PROFILE_BASTILLE], MarineIncluded[ASW_MARINE_PROFILE_BASTILLE] );
		ReadMarine( pKV, bBinary, "7", "Marines/Vegas", Marines[ASW_MARINE_PROFILE_VEGAS], MarineIncluded[ASW_MARINE_PROFILE_VEGAS] );
	}

#ifdef CLIENT_DLL
	static CUtlDict<LoadoutData_t> s_Loadouts;
	static bool s_bLoadoutsInitialized = false;

	static void InitLoadouts()
	{
		if ( s_bLoadoutsInitialized )
			return;

		KeyValues::AutoDelete pKV{ "SavedLoadouts" };
		bool bLoaded;
		ConVarRef cl_cloud_settings{ "cl_cloud_settings" };
		ISteamRemoteStorage *pSteamRemoteStorage = SteamRemoteStorage();
		if ( ( cl_cloud_settings.GetInt() & STEAMREMOTESTORAGE_CLOUD_SAVED_LOADOUTS ) && pSteamRemoteStorage && pSteamRemoteStorage->FileExists( "reactivedrop/cfg/saved_loadouts.txt" ) )
		{
			CUtlBuffer buf;
			int32 iSize = pSteamRemoteStorage->GetFileSize( "reactivedrop/cfg/saved_loadouts.txt" );
			pSteamRemoteStorage->FileRead( "reactivedrop/cfg/saved_loadouts.txt", buf.AccessForDirectRead( iSize ), iSize );
			buf.SeekPut( CUtlBuffer::SEEK_HEAD, iSize );
			filesystem->WriteFile( "cfg/saved_loadouts.txt", "MOD", buf );
			buf.SetBufferType( true, true );
			bLoaded = UTIL_RD_LoadKeyValues( pKV, "cfg/saved_loadouts.txt", buf );
		}
		else
		{
			bLoaded = UTIL_RD_LoadKeyValuesFromFile( pKV, g_pFullFileSystem, "cfg/saved_loadouts.txt", "MOD" );
		}

		if ( bLoaded )
		{
			Assert( pKV->GetInt( "Version" ) == 1 );
			KeyValues *pLoadouts = pKV->FindKey( "Loadouts" );
			if ( pLoadouts )
			{
				FOR_EACH_SUBKEY( pLoadouts, pLoadout )
				{
					int i = s_Loadouts.Insert( pLoadout->GetName() );
					s_Loadouts[i].FromKeyValues( pLoadout, false );
				}
			}
		}

		s_bLoadoutsInitialized = true;
	}
	static void WriteLoadouts()
	{
		Assert( s_bLoadoutsInitialized );

		DevMsg( "Writing %d loadouts to file\n", s_Loadouts.Count() );

		KeyValues::AutoDelete pKV{ "SavedLoadouts" };
		pKV->SetInt( "Version", 1 );

		KeyValues *pLoadouts = pKV->FindKey( "Loadouts", true );
		for ( int i = 0; i < s_Loadouts.Count(); i++ )
		{
			s_Loadouts[i].ToKeyValues( pLoadouts->FindKey( s_Loadouts.GetElementName( i ), true ), false );
		}

		pKV->SaveToFile( g_pFullFileSystem, "cfg/saved_loadouts.txt", "MOD" );

		ConVarRef cl_cloud_settings{ "cl_cloud_settings" };
		ISteamRemoteStorage *pSteamRemoteStorage = SteamRemoteStorage();
		if ( ( cl_cloud_settings.GetInt() & STEAMREMOTESTORAGE_CLOUD_SAVED_LOADOUTS ) && pSteamRemoteStorage )
		{
			CUtlBuffer buf;
			pKV->RecursiveSaveToFile( buf, 0 );
			pSteamRemoteStorage->FileWrite( "reactivedrop/cfg/saved_loadouts.txt", buf.Base(), buf.TellPut() );
		}
	}
	static void ReplaceItemIDHelper( bool &bAnyChanged, SteamItemInstanceID_t &id, SteamItemInstanceID_t oldID, SteamItemInstanceID_t newID, const char *szLoadoutName, const char *szSlotName )
	{
		if ( oldID == id )
		{
			id = newID;
			DevMsg( "Replacing item instance %llu with %llu in saved loadout '%s' slot '%s'\n", oldID, newID, szLoadoutName, szSlotName );
			bAnyChanged = true;
		}
	}

	void LoadoutMarineData_t::CopyToLive( ASW_Marine_Profile id ) const
	{
		asw_default_primary[id].SetValue( Primary );
		asw_default_secondary[id].SetValue( Secondary );
		asw_default_extra[id].SetValue( Extra );
		rd_equipped_marine[id].SetValue( CFmtStr{ "%llu", Suit } );
		rd_equipped_weapon_primary[id].SetValue( CFmtStr{ "%llu", PrimaryItem } );
		rd_equipped_weapon_secondary[id].SetValue( CFmtStr{ "%llu", SecondaryItem } );
		rd_equipped_weapon_extra[id].SetValue( CFmtStr{ "%llu", ExtraItem } );

		C_ASW_Game_Resource *pGameResource = ASWGameResource();
		C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
		if ( pGameResource && pPlayer )
		{
			for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
			{
				C_ASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( pMR && pMR->GetCommander() == pPlayer )
				{
					pPlayer->SendRosterSelectCommand( "cl_loadouta", pMR->GetProfileIndex() );
				}
			}
		}
	}
	void LoadoutMarineData_t::CopyFromLive( ASW_Marine_Profile id )
	{
		Primary = ASW_Equip_Regular( asw_default_primary[id].GetInt() );
		Secondary = ASW_Equip_Regular( asw_default_secondary[id].GetInt() );
		Extra = ASW_Equip_Extra( asw_default_extra[id].GetInt() );
		if ( Primary < 0 || Primary >= ASW_NUM_EQUIP_REGULAR )
			Primary = DefaultLoadout.Marines[id].Primary;
		if ( Secondary < 0 || Secondary >= ASW_NUM_EQUIP_REGULAR )
			Secondary = DefaultLoadout.Marines[id].Secondary;
		if ( Extra < 0 || Extra >= ASW_NUM_EQUIP_EXTRA )
			Extra = DefaultLoadout.Marines[id].Extra;

		Suit = strtoull( rd_equipped_marine[id].GetString(), NULL, 10 );
		PrimaryItem = strtoull( rd_equipped_weapon_primary[id].GetString(), NULL, 10 );
		SecondaryItem = strtoull( rd_equipped_weapon_secondary[id].GetString(), NULL, 10 );
		ExtraItem = strtoull( rd_equipped_weapon_extra[id].GetString(), NULL, 10 );

		if ( Suit == 0 )
			Suit = k_SteamItemInstanceIDInvalid;
		if ( PrimaryItem == 0 )
			PrimaryItem = k_SteamItemInstanceIDInvalid;
		if ( SecondaryItem == 0 )
			SecondaryItem = k_SteamItemInstanceIDInvalid;
		if ( ExtraItem == 0 )
			ExtraItem = k_SteamItemInstanceIDInvalid;
	}
	bool LoadoutMarineData_t::ReplaceItemID( SteamItemInstanceID_t oldID, SteamItemInstanceID_t newID, const char *szDebugLoadoutName, const char *szDebugMarineName )
	{
		bool bAnyChanged = false;

		ReplaceItemIDHelper( bAnyChanged, Suit, oldID, newID, szDebugLoadoutName, CFmtStr{ "%s's suit", szDebugMarineName } );
		ReplaceItemIDHelper( bAnyChanged, PrimaryItem, oldID, newID, szDebugLoadoutName, CFmtStr{ "%s's primary weapon", szDebugMarineName } );
		ReplaceItemIDHelper( bAnyChanged, SecondaryItem, oldID, newID, szDebugLoadoutName, CFmtStr{ "%s's secondary weapon", szDebugMarineName } );
		ReplaceItemIDHelper( bAnyChanged, ExtraItem, oldID, newID, szDebugLoadoutName, CFmtStr{ "%s's extra weapon", szDebugMarineName } );

		return bAnyChanged;
	}

	void LoadoutData_t::CopyToLive()
	{
		if ( rd_loadout_load_medals.GetBool() )
		{
			for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
			{
				rd_equipped_medal[i].SetValue( CFmtStr{ "%llu", Medals[i] } );
			}
		}

		for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
		{
			if ( MarineIncluded[i] )
				Marines[i].CopyToLive( ASW_Marine_Profile( i ) );
		}

		ISteamUtils *pUtils = SteamUtils();
		if ( rd_loadout_track_last_used.GetInt() > 0 && pUtils && LastUsed <= pUtils->GetServerRealTime() - rd_loadout_track_last_used.GetInt() )
		{
			LastUsed = pUtils->GetServerRealTime();
			WriteLoadouts();
		}

		engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
	}
	void LoadoutData_t::CopyFromLive()
	{
		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
		{
			Medals[i] = strtoull( rd_equipped_medal[i].GetString(), NULL, 10 );
			if ( Medals[i] == 0 )
				Medals[i] = k_SteamItemInstanceIDInvalid;
		}

		for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
		{
			Marines[i].CopyFromLive( ASW_Marine_Profile( i ) );
			MarineIncluded[i] = true;
		}

		ISteamUtils *pUtils = SteamUtils();
		if ( pUtils )
			LastModified = pUtils->GetServerRealTime();
	}
	bool LoadoutData_t::ReplaceItemID( SteamItemInstanceID_t oldID, SteamItemInstanceID_t newID, const char *szDebugLoadoutName )
	{
		bool bAnyChanged = false;

		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
			ReplaceItemIDHelper( bAnyChanged, Medals[i], oldID, newID, szDebugLoadoutName, CFmtStr{ "medal %d", i + 1 } );

		bAnyChanged = Marines[ASW_MARINE_PROFILE_SARGE].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Sarge" ) || bAnyChanged;
		bAnyChanged = Marines[ASW_MARINE_PROFILE_WILDCAT].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Wildcat" ) || bAnyChanged;
		bAnyChanged = Marines[ASW_MARINE_PROFILE_FAITH].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Faith" ) || bAnyChanged;
		bAnyChanged = Marines[ASW_MARINE_PROFILE_CRASH].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Crash" ) || bAnyChanged;
		bAnyChanged = Marines[ASW_MARINE_PROFILE_JAEGER].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Jaeger" ) || bAnyChanged;
		bAnyChanged = Marines[ASW_MARINE_PROFILE_WOLFE].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Wolfe" ) || bAnyChanged;
		bAnyChanged = Marines[ASW_MARINE_PROFILE_BASTILLE].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Bastille" ) || bAnyChanged;
		bAnyChanged = Marines[ASW_MARINE_PROFILE_VEGAS].ReplaceItemID( oldID, newID, szDebugLoadoutName, "Vegas" ) || bAnyChanged;

		return bAnyChanged;
	}

	void List( CUtlStringList &names )
	{
		InitLoadouts();

		for ( int i = 0; i < s_Loadouts.Count(); i++ )
		{
			names.CopyAndAddToTail( s_Loadouts.GetElementName( i ) );
		}
	}
	bool Save( const char *szName )
	{
		InitLoadouts();

		bool bFound = true;
		int i = s_Loadouts.Find( szName );
		if ( !s_Loadouts.IsValidIndex( i ) )
		{
			bFound = false;
			i = s_Loadouts.Insert( szName );
		}

		s_Loadouts[i].CopyFromLive();
		WriteLoadouts();

		return bFound;
	}
	bool Load( const char *szName )
	{
		InitLoadouts();

		int i = s_Loadouts.Find( szName );
		if ( s_Loadouts.IsValidIndex( i ) )
		{
			s_Loadouts[i].CopyToLive();
			return true;
		}

		return false;
	}
	bool Delete( const char *szName )
	{
		InitLoadouts();

		int i = s_Loadouts.Find( szName );
		if ( s_Loadouts.IsValidIndex( i ) )
		{
			s_Loadouts.RemoveAt( i );
			WriteLoadouts();
			return true;
		}

		return false;
	}
	static void ReplaceItemIDCVarHelper( bool &bAnyChanged, ConVar *pConVar, SteamItemInstanceID_t oldID, const char *szNewValue )
	{
		if ( strtoull( pConVar->GetString(), NULL, 10 ) == oldID )
		{
			pConVar->SetValue( szNewValue );
			DevMsg( "Replacing item instance %llu with %s in %s\n", oldID, szNewValue, pConVar->GetName() );
			bAnyChanged = true;
		}
	}
	void ReplaceItemID( SteamItemInstanceID_t oldID, SteamItemInstanceID_t newID )
	{
		InitLoadouts();

		bool bAnyChanged = false;
		CFmtStr szNewValue{ "%llu", newID };

		// static equip slots (medals, marines)
		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
		{
			ReplaceItemIDCVarHelper( bAnyChanged, &rd_equipped_medal[i], oldID, szNewValue );
		}
		for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
		{
			ReplaceItemIDCVarHelper( bAnyChanged, &rd_equipped_marine[i], oldID, szNewValue );
		}

		// current loadout (weapons)
		for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
		{
			ReplaceItemIDCVarHelper( bAnyChanged, &rd_equipped_weapon_primary[i], oldID, szNewValue );
			ReplaceItemIDCVarHelper( bAnyChanged, &rd_equipped_weapon_secondary[i], oldID, szNewValue );
			ReplaceItemIDCVarHelper( bAnyChanged, &rd_equipped_weapon_extra[i], oldID, szNewValue );
		}

		// saved loadouts
		for ( int i = 0; i < s_Loadouts.Count(); i++ )
		{
			if ( s_Loadouts[i].ReplaceItemID( oldID, newID, s_Loadouts.GetElementName( i ) ) )
			{
				bAnyChanged = true;
			}
		}

		if ( bAnyChanged )
		{
			engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
			WriteLoadouts();
		}
		else
		{
			DevMsg( "Item instance %llu replaced with %llu but is not equipped; nothing to do.\n", oldID, newID );
		}
	}
#endif
}
