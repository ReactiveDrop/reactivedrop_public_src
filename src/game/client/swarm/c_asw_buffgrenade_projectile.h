#ifndef _INCLUDED_C_ASW_BUFFGREN_PROJECTILE_H
#define _INCLUDED_C_ASW_BUFFGREN_PROJECTILE_H
#pragma once

#include "c_asw_aoegrenade_projectile.h"
#include "iasw_client_usable_entity.h"

class C_ASW_BuffGrenade_Projectile : public C_ASW_AOEGrenade_Projectile, public IASW_Client_Usable_Entity
{
public:
	DECLARE_CLASS( C_ASW_BuffGrenade_Projectile, C_ASW_AOEGrenade_Projectile );
	DECLARE_CLIENTCLASS();

	virtual Color GetGrenadeColor( void ) override;
	virtual const char* GetLoopSoundName( void ) override { return "ASW_BuffGrenade.BuffLoop"; }
	virtual const char* GetStartSoundName( void ) override { return "ASW_BuffGrenade.StartBuff"; }
	virtual const char* GetActivateSoundName( void ) override { return "ASW_BuffGrenade.GrenadeActivate"; }
	virtual const char* GetPingEffectName( void ) override { return "buffgrenade_pulse"; }
	virtual const char* GetArcEffectName( int index ) override { return index == 1 ? "buffgrenade_noconnect" : "buffgrenade_attach_arc"; }
	virtual const char* GetArcAttachmentName( void ) override { return "beam_attach"; }
	virtual int GetArcEffectIndex( C_BaseEntity *pEnt ) override;
	virtual bool ShouldAttachEffectToWeapon( void ) override { return true; }
	virtual bool ShouldSpawnSphere( void ) override { return true; }
	virtual float GetSphereScale( void ) override { return 0.98f; }
	virtual int GetSphereSkin( void ) override { return 1; }

	virtual C_BaseEntity *GetEntity() override { return this; }
	virtual bool IsUsable( C_BaseEntity *pUser ) override;
	virtual bool GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser ) override;
	virtual void CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon ) override {}
	virtual bool ShouldPaintBoxAround() override { return true; }
	virtual bool NeedsLOSCheck() override { return false; }

	EHANDLE m_hSphereModel;
	//float m_flPrevRotAngle;
	float m_flTimeCreated;
};


#endif // _INCLUDED_C_ASW_BUFFGREN_PROJECTILE_H