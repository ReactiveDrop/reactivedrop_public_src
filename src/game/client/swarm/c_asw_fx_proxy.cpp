//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_asw_marine.h"
#include "c_asw_physics_prop_statue.h"
#include "c_asw_mesh_emitter_entity.h"
#include "c_asw_egg.h"
#include "c_asw_clientragdoll.h"
#include "c_asw_weapon.h"
#include "c_asw_sentry_base.h"
#include "c_asw_sentry_top.h"

#include "ProxyEntity.h"
#include "functionproxy.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterialSystem.h"
#include <KeyValues.h>

#include "imaterialproxydict.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


PRECACHE_REGISTER_BEGIN( GLOBAL, ASW_Model_FX )
	PRECACHE( MATERIAL, "effects/TiledFire/fire_tiled_precache" )
	PRECACHE( MATERIAL, "effects/model_layer_shock_1_precache" )
	PRECACHE( MATERIAL, "effects/model_layer_ice_1_precache" )
	PRECACHE( MATERIAL, "effects/model_layer_shockfire_1_precache" )
	PRECACHE( MATERIAL, "effects/model_layer_shockice_1_precache" )
	PRECACHE( MATERIAL, "effects/model_layer_icefire_1_precache" )
	PRECACHE( MATERIAL, "effects/model_layer_ohgod_1_precache" )
PRECACHE_REGISTER_END()

//-----------------------------------------------------------------------------
// Material proxy for changing the material of aliens
//-----------------------------------------------------------------------------
class CASW_Model_FX_Proxy : public CEntityMaterialProxy
{
public:
	CASW_Model_FX_Proxy( void );
	virtual				~CASW_Model_FX_Proxy( void );
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void * );
	virtual void OnBind( C_BaseEntity *pEnt );
	void UpdateEffects( bool bShockBig, bool bOnFire, float flFrozen );
	void TextureTransform( float flSpeed = 0, float flScale = 6.0f );
	virtual IMaterial *	GetMaterial();

private:
	ITexture*	m_pFXTexture;

	// "$detailscale" "5"
	// "$detailblendfactor" 1.0
	// "$detailblendmode" 6
	IMaterialVar		*m_pDetailMaterial;
	IMaterialVar		*m_pDetailScale;
	IMaterialVar		*m_pDetailBlendFactor;
	IMaterialVar		*m_pDetailBlendMode;
	IMaterialVar		*m_pTextureScrollVar;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CASW_Model_FX_Proxy::CASW_Model_FX_Proxy( void )
{
	m_pFXTexture = NULL;
	m_pDetailMaterial = NULL;
	m_pDetailScale = NULL;
	m_pDetailBlendFactor = NULL;
	m_pDetailBlendMode = NULL;
	m_pTextureScrollVar = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CASW_Model_FX_Proxy::~CASW_Model_FX_Proxy( void )
{
}

bool CASW_Model_FX_Proxy::Init( IMaterial *pMaterial, KeyValues* pKeyValues )
{
	Assert( pMaterial );

	// Need to get the material var
	bool bDetail;
	m_pDetailMaterial = pMaterial->FindVar( "$detail", &bDetail );

	bool bScale;
	m_pDetailScale = pMaterial->FindVar( "$detailscale", &bScale );

	bool bBlendFact;
	m_pDetailBlendFactor = pMaterial->FindVar( "$detailblendfactor", &bBlendFact );

	bool bBlendMode;
	m_pDetailBlendMode = pMaterial->FindVar( "$detailblendmode", &bBlendMode );

	char const* pScrollVarName = pKeyValues->GetString( "texturescrollvar" );
	if( !pScrollVarName )
		return false;

	bool bScrollVar;
	m_pTextureScrollVar = pMaterial->FindVar( "$detailtexturetransform", &bScrollVar );

	return ( bDetail && bScale && bBlendFact && bBlendMode && bScrollVar );
}

void CASW_Model_FX_Proxy::OnBind( void *pC_BaseEntity )
{
	if ( !pC_BaseEntity )
	{
		m_pDetailBlendFactor->SetFloatValue( 0.0f );
	}

	CEntityMaterialProxy::OnBind( pC_BaseEntity );
}

void CASW_Model_FX_Proxy::OnBind( C_BaseEntity *pEnt )
{
	if ( !pEnt )
		return;

	bool bShockBig = false;
	bool bOnFire = false;
	float flFrozen = 0;

	if ( pEnt->IsInhabitableNPC() )
	{
		C_ASW_Inhabitable_NPC *pNPC = assert_cast< C_ASW_Inhabitable_NPC * >( pEnt );

		bShockBig = pNPC->m_bElectroStunned;
		bOnFire = pNPC->m_bOnFire;
		flFrozen = pNPC->GetMoveType() == MOVETYPE_NONE ? 0.0f : pNPC->GetFrozenAmount();
		UpdateEffects( bShockBig, bOnFire, flFrozen );
		return;
	}

	if ( pEnt->Classify() == CLASS_ASW_EGG )
	{
		C_ASW_Egg* pEgg = assert_cast<C_ASW_Egg*>(pEnt);

		bOnFire = pEgg->m_bOnFire;
		flFrozen = pEgg->GetFrozenAmount();
		UpdateEffects(false, bOnFire, flFrozen);
		return;
	}

	C_ASW_ClientRagdoll *pRagDoll = dynamic_cast< C_ASW_ClientRagdoll * >( pEnt );
	if ( pRagDoll )
	{
		bShockBig = pRagDoll->m_bElectroShock;
		bOnFire = !!( pRagDoll->GetFlags() & FL_ONFIRE );
		UpdateEffects( bShockBig, bOnFire, 0.0f );
		return;
	}

	C_ASW_Mesh_Emitter *pGib = dynamic_cast< C_ASW_Mesh_Emitter * >( pEnt );
	if ( pGib && pGib->m_bFrozen )
	{
		m_pFXTexture = materials->FindTexture( "effects/model_layer_ice_1", TEXTURE_GROUP_MODEL );
		if ( m_pFXTexture )
		{
			m_pDetailMaterial->SetTextureValue( m_pFXTexture );
		}
		m_pDetailBlendFactor->SetFloatValue( 0.4f );
		TextureTransform( 0, 5.0f );
		return;
	}

	C_ASWStatueProp *pStatue = dynamic_cast< C_ASWStatueProp * >( pEnt );
	if ( pStatue )
	{
		m_pFXTexture = materials->FindTexture( "effects/model_layer_ice_1", TEXTURE_GROUP_MODEL );
		if ( m_pFXTexture )
		{
			m_pDetailMaterial->SetTextureValue( m_pFXTexture );
		}
		m_pDetailBlendFactor->SetFloatValue( 0.4f );
		TextureTransform( 0, 5.0f );
		return;
	}

	C_ASW_Weapon *pWeapon = dynamic_cast< C_ASW_Weapon * >( pEnt );
	if ( pWeapon && pWeapon->GetOwner() && pWeapon->GetOwner()->IsInhabitableNPC() )
	{
		C_ASW_Inhabitable_NPC *pNPC = assert_cast< C_ASW_Inhabitable_NPC * >( pWeapon->GetOwner() );

		bShockBig = pNPC->m_bElectroStunned;
		if ( !bShockBig && pWeapon->Classify() == CLASS_ASW_ELECTRIFIED_ARMOR )
		{
			C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pNPC );
			bShockBig = pMarine && pMarine->IsElectrifiedArmorActive();
		}
		bOnFire = pNPC->m_bOnFire;
		flFrozen = pNPC->GetMoveType() == MOVETYPE_NONE ? 0.0f : pNPC->GetFrozenAmount();
		UpdateEffects( bShockBig, bOnFire, flFrozen );
		return;
	}

	m_pDetailBlendFactor->SetFloatValue( 0.0f );
}

void CASW_Model_FX_Proxy::UpdateEffects( bool bShockBig, bool bOnFire, float flFrozen )
{
	if ( bShockBig || bOnFire || flFrozen > 0 )
	{
		if ( bShockBig && bOnFire && flFrozen > 0 )
		{
			m_pFXTexture = materials->FindTexture( "effects/model_layer_ohgod_1", TEXTURE_GROUP_MODEL );
			if ( m_pFXTexture )
			{
				m_pDetailBlendFactor->SetFloatValue( 0.3f );
				m_pDetailMaterial->SetTextureValue( m_pFXTexture );
				TextureTransform( 80, 4.0f );
			}
		}
		else if ( bShockBig && bOnFire )
		{
			m_pFXTexture = materials->FindTexture( "effects/model_layer_shockfire_1", TEXTURE_GROUP_MODEL );
			if ( m_pFXTexture )
			{
				m_pDetailBlendFactor->SetFloatValue( 0.2f );
				m_pDetailMaterial->SetTextureValue( m_pFXTexture );
				TextureTransform( 80, 4.0f );
			}
		}
		else if ( bShockBig && flFrozen > 0 )
		{
			m_pFXTexture = materials->FindTexture( "effects/model_layer_shockice_1", TEXTURE_GROUP_MODEL );
			if ( m_pFXTexture )
			{
				m_pDetailBlendFactor->SetFloatValue( 0.4f );
				m_pDetailMaterial->SetTextureValue( m_pFXTexture );
				TextureTransform( 80, 4.0f );
			}
		}
		else if ( bOnFire && flFrozen > 0 )
		{
			m_pFXTexture = materials->FindTexture( "effects/model_layer_icefire_1", TEXTURE_GROUP_MODEL );
			if ( m_pFXTexture )
			{
				m_pDetailBlendFactor->SetFloatValue( 0.2f );
				m_pDetailMaterial->SetTextureValue( m_pFXTexture );
				TextureTransform( 24, 4.0f );
			}
		}
		else if ( bShockBig )
		{
			m_pFXTexture = materials->FindTexture( "effects/model_layer_shock_1", TEXTURE_GROUP_MODEL );
			if ( m_pFXTexture )
			{
				m_pDetailBlendFactor->SetFloatValue( 0.75f );
				m_pDetailMaterial->SetTextureValue( m_pFXTexture );
				TextureTransform( 80, 4.0f );
			}
		}
		else if ( flFrozen > 0 )
		{
			m_pFXTexture = materials->FindTexture( "effects/model_layer_ice_1", TEXTURE_GROUP_MODEL );
			if ( m_pFXTexture )
			{
				m_pDetailBlendFactor->SetFloatValue( MIN( 0.4f, flFrozen/4) );
				m_pDetailMaterial->SetTextureValue( m_pFXTexture );
				TextureTransform( 0, 5.0f );
			}
		}
		else if ( bOnFire )
		{
			m_pFXTexture = materials->FindTexture( "effects/TiledFire/fire_tiled", TEXTURE_GROUP_MODEL );
			if ( m_pFXTexture )
			{
				m_pDetailBlendFactor->SetFloatValue( 0.3f );
				m_pDetailMaterial->SetTextureValue( m_pFXTexture );
				TextureTransform( 24, 6.0f );
			}
		}
	}
	else
	{
		m_pDetailBlendFactor->SetFloatValue( 0.0f );
	}
}

void CASW_Model_FX_Proxy::TextureTransform( float flSpeed, float flScale )
{
	// scrolling of the detail material
	float flRate			= abs( flSpeed ) / 128.0;
	float flAngle			= (flSpeed >= 0) ? 180 : 0;

	float sOffset = gpGlobals->curtime * cos( flAngle * ( M_PI / 180.0f ) ) * flRate;
	float tOffset = gpGlobals->curtime * sin( flAngle * ( M_PI / 180.0f ) ) * flRate;

	// make sure that we are positive
	if( sOffset < 0.0f )
	{
		sOffset += 1.0f + -( int )sOffset;
	}
	if( tOffset < 0.0f )
	{
		tOffset += 1.0f + -( int )tOffset;
	}

	// make sure that we are in a [0,1] range
	sOffset = sOffset - ( int )sOffset;
	tOffset = tOffset - ( int )tOffset;

	if (m_pTextureScrollVar->GetType() == MATERIAL_VAR_TYPE_MATRIX)
	{
		VMatrix mat;
		MatrixBuildTranslation( mat, sOffset, tOffset, 0.0f );
		m_pTextureScrollVar->SetMatrixValue( mat );
	}
	else
	{
		m_pTextureScrollVar->SetVecValue( sOffset, tOffset, 0.0f );
	}

	m_pDetailScale->SetFloatValue( flScale );
}

IMaterial *CASW_Model_FX_Proxy::GetMaterial()
{
	if ( !m_pDetailMaterial )
		return NULL;

	return m_pDetailMaterial->GetOwningMaterial();
}

EXPOSE_MATERIAL_PROXY( CASW_Model_FX_Proxy, AlienSurfaceFX );

static const CRD_ItemInstance &GetItemInstanceFromRenderable( IClientRenderable *pRenderable )
{
	static const CRD_ItemInstance s_EmptyInstance;

	C_BaseEntity *pEnt = pRenderable->GetIClientUnknown()->GetBaseEntity();

	if ( C_RD_Weapon_Accessory *pAccessory = dynamic_cast< C_RD_Weapon_Accessory * >( pEnt ) )
	{
		pEnt = pAccessory->GetOwnerEntity();
	}

	if ( C_ASW_Weapon *pWeapon = dynamic_cast< C_ASW_Weapon * >( pEnt ) )
	{
		return pWeapon->m_InventoryItemData;
	}

	if ( C_ASW_Sentry_Top *pSentry = dynamic_cast< C_ASW_Sentry_Top * >( pEnt ) )
	{
		pEnt = pSentry->GetSentryBase();
	}

	if ( C_ASW_Sentry_Base *pSentry = dynamic_cast< C_ASW_Sentry_Base * >( pEnt ) )
	{
		return pSentry->m_InventoryItemData;
	}

	if ( IRD_Has_Projectile_Data *pProjectile = dynamic_cast< IRD_Has_Projectile_Data * >( pEnt ) )
	{
		return pProjectile->GetProjectileData()->m_InventoryItemData;
	}

	return s_EmptyInstance;
}

// Returns a number from -1 to RD_ITEM_MAX_ACCESSORIES-1 based on entity data.
class CRD_StatTrakAccessory_Proxy : public CResultProxy
{
public:
	void OnBind( void *pRenderable ) override;
};

void CRD_StatTrakAccessory_Proxy::OnBind( void *pRenderable )
{
	C_BaseEntity *pEnt = BindArgToEntity( pRenderable );
	if ( !pEnt )
	{
		SetFloatResult( -1 );
		return;
	}

	if ( C_RD_Weapon_Accessory *pAccessory = dynamic_cast< C_RD_Weapon_Accessory * >( pEnt ) )
	{
		SetFloatResult( pAccessory->m_iAccessoryIndex );
		return;
	}

	SetFloatResult( -1 );
}

EXPOSE_MATERIAL_PROXY( CRD_StatTrakAccessory_Proxy, StatTrakAccessory );

// works similarly to how StatTrakDigit works in Valve games, but has additional inputs and can (potentially) handle negatives
class CRD_StatTrakDigit_Proxy : public CResultProxy
{
public:
	bool Init( IMaterial *pMaterial, KeyValues *pKeyValues ) override;
	void OnBind( void *pRenderable ) override;
	int64_t GetStatValue( const CRD_ItemInstance &instance );

	CFloatInput m_DisplayDigit;
	CFloatInput m_TrimZeros;
	CFloatInput m_AccessoryIndex;
	CFloatInput m_PropertyIndex;
	CFloatInput m_DefaultValue;
};

bool CRD_StatTrakDigit_Proxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if ( !CResultProxy::Init( pMaterial, pKeyValues ) )
		return false;

	if ( !m_DisplayDigit.Init( pMaterial, pKeyValues, "displayDigit" ) )
		return false;

	if ( !m_TrimZeros.Init( pMaterial, pKeyValues, "trimZeros" ) )
		return false;

	if ( !m_AccessoryIndex.Init( pMaterial, pKeyValues, "accessoryIndex" ) )
		return false;

	if ( !m_PropertyIndex.Init( pMaterial, pKeyValues, "propertyIndex" ) )
		return false;

	if ( !m_DefaultValue.Init( pMaterial, pKeyValues, "defaultValue" ) )
		return false;

	return true;
}

void CRD_StatTrakDigit_Proxy::OnBind( void *pRenderable )
{
	int iDigit = int( m_DisplayDigit.GetFloat() );
	bool bTrim = m_TrimZeros.GetFloat() > 0;

	const CRD_ItemInstance &instance = GetItemInstanceFromRenderable( static_cast< IClientRenderable * >( pRenderable ) );
	int64_t iStat = GetStatValue( instance );

	// special case for the one time it won't be a leading zero
	if ( iStat == 0 && iDigit == 0 )
	{
		SetFloatResult( 0 );
		return;
	}

	bool bWasNegative = false;
	for ( int i = int( m_DisplayDigit.GetFloat() ); i > 0; i-- )
	{
		bWasNegative = iStat < 0;
		iStat /= 10;
	}

	if ( bWasNegative )
	{
		if ( iStat == 0 )
		{
			SetFloatResult( 11 );
			return;
		}

		iStat = -iStat;
	}

	if ( iStat == 0 && bTrim )
	{
		SetFloatResult( 10 );
		return;
	}

	SetFloatResult( iStat % 10 );
}

int64_t CRD_StatTrakDigit_Proxy::GetStatValue( const CRD_ItemInstance &instance )
{
	int iAccessory = int( m_AccessoryIndex.GetFloat() );
	int iProperty = int( m_PropertyIndex.GetFloat() );
	if ( !instance.IsSet() || iAccessory < -1 || iProperty < 0 || iAccessory >= RD_ITEM_MAX_ACCESSORIES )
	{
		return int64_t( m_DefaultValue.GetFloat() );
	}

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( instance.m_iItemDefID );
	if ( !pDef || iProperty >= ( iAccessory == -1 ? pDef->CompressedDynamicProps.Count() : RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY ) )
	{
		return int64_t( m_DefaultValue.GetFloat() );
	}

	if ( iAccessory == -1 )
	{
		if ( iProperty >= pDef->CompressedDynamicProps.Count() )
		{
			return int64_t( m_DefaultValue.GetFloat() );
		}

		return instance.m_nCounter[iProperty];
	}

	if ( instance.m_iAccessory[iAccessory] == 0 )
	{
		return int64_t( m_DefaultValue.GetFloat() );
	}

	int iTotalProperty = iProperty + pDef->CompressedDynamicProps.Count();
	for ( int i = 0; i < iAccessory; i++ )
	{
		const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( instance.m_iAccessory[i] );
		if ( !pAccessoryDef )
		{
			return int64_t( m_DefaultValue.GetFloat() );
		}

		iTotalProperty += pAccessoryDef->CompressedDynamicProps.Count();
	}

	const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( instance.m_iAccessory[iAccessory] );
	if ( !pAccessoryDef || iProperty >= pAccessoryDef->CompressedDynamicProps.Count() )
	{
		return int64_t( m_DefaultValue.GetFloat() );
	}

	return instance.m_nCounter[iTotalProperty];
}

EXPOSE_MATERIAL_PROXY( CRD_StatTrakDigit_Proxy, StatTrakDigit );

// differs from how StatTrakIcon works in Valve games; this sets $basetexture rather than returning a matrix
class CRD_StatTrakIcon_Proxy : public IMaterialProxy
{
public:
	bool Init( IMaterial *pMaterial, KeyValues *pKeyValues ) override;
	void Release() override;
	void OnBind( void *pRenderable ) override;
	IMaterial *GetMaterial() override;

	IMaterialVar *m_pBaseTextureVar{};
	CFloatInput m_AccessoryIndex;
};

bool CRD_StatTrakIcon_Proxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool bFound;
	m_pBaseTextureVar = pMaterial->FindVar( "$basetexture", &bFound );
	if ( !bFound )
		return false;

	if ( !m_AccessoryIndex.Init( pMaterial, pKeyValues, "accessoryIndex" ) )
		return false;

	return true;
}

void CRD_StatTrakIcon_Proxy::Release()
{
	delete this;
}

void CRD_StatTrakIcon_Proxy::OnBind( void *pRenderable )
{
	int iAccessory = int( m_AccessoryIndex.GetFloat() );
	const CRD_ItemInstance &instance = GetItemInstanceFromRenderable( static_cast< IClientRenderable * >( pRenderable ) );
	if ( !instance.IsSet() || iAccessory < -1 || iAccessory >= RD_ITEM_MAX_ACCESSORIES || ( iAccessory >= 0 && instance.m_iAccessory[iAccessory] == 0 ) )
	{
		m_pBaseTextureVar->SetTextureValue( NULL );
		return;
	}

	const ReactiveDropInventory::ItemDef_t *pDef = iAccessory == -1 ? ReactiveDropInventory::GetItemDef( instance.m_iItemDefID ) : ReactiveDropInventory::GetItemDef( instance.m_iAccessory[iAccessory] );

	m_pBaseTextureVar->SetTextureValue( pDef ? pDef->AccessoryIcon : NULL );
}

IMaterial *CRD_StatTrakIcon_Proxy::GetMaterial()
{
	if ( m_pBaseTextureVar )
		return m_pBaseTextureVar->GetOwningMaterial();

	return NULL;
}

EXPOSE_MATERIAL_PROXY( CRD_StatTrakIcon_Proxy, StatTrakIcon );
