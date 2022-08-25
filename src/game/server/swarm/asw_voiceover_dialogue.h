#ifndef ASW_VOICEOVER_DIALOGUE_H
#define ASW_VOICEOVER_DIALOGUE_H

#ifdef _WIN32
#pragma once
#endif

class CASW_Voiceover_Dialogue : public CLogicalEntity
{
public:
	DECLARE_CLASS( CASW_Voiceover_Dialogue, CLogicalEntity );
	DECLARE_DATADESC();

	CASW_Voiceover_Dialogue();
	virtual ~CASW_Voiceover_Dialogue();

	virtual void Precache() override;
	virtual void Spawn() override;
	void InputStart( inputdata_t &inputdata );

	string_t m_iszSoundName;
	HSOUNDSCRIPTHANDLE m_hSoundScriptHandle;
	float m_flDuration;
	string_t m_iszActorName;
	bool m_bBusyActor;

	COutputEvent m_OnStart;
	COutputEvent m_OnCompletion;
};

#endif // ASW_VOICEOVER_DIALOGUE_H
