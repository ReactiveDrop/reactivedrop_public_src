#ifndef CASW_OBJECTIVEINFO_H
#define CASW_OBJECTIVEINFO_H
#pragma once

// This class holds information about a particular objective
class C_ASW_Objective : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_ASW_Objective, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_ASW_Objective();
	virtual ~C_ASW_Objective();

	virtual const wchar_t *GetObjectiveTitle( void );
	const char *GetObjectiveImage( void ) { return m_ObjectiveImage; }
	const char *GetDescription( int i );
	const char *GetInfoIcon( int i );
	const char *GetObjectiveIconName( void );

	// allows this objective to paint its status on the HUD
	virtual void PaintObjective( float &current_y );

	virtual bool IsObjectiveComplete() { return m_bComplete; }
	virtual bool IsObjectiveFailed() { return m_bFailed; }
	virtual bool IsObjectiveOptional() { return m_bOptional; }
	virtual bool IsObjectiveDummy() { return m_bDummy; }
	virtual bool IsObjectiveHidden() { return !m_bVisible; }
	virtual bool NeedsTitleUpdate() { return false; }	// most objectives don't have constantly changing titles, they can just be set once
	virtual float GetObjectiveProgress() { return IsObjectiveComplete() ? 1.0f : 0.0f; }

	char		m_ObjectiveTitle[256];
	char		m_ObjectiveDescription1[256];
	char		m_ObjectiveDescription2[256];
	char		m_ObjectiveDescription3[256];
	char		m_ObjectiveDescription4[256];
	char		m_ObjectiveImage[256];
	char		m_ObjectiveInfoIcon1[256];
	char		m_ObjectiveInfoIcon2[256];
	char		m_ObjectiveInfoIcon3[256];
	char		m_ObjectiveInfoIcon4[256];
	char		m_ObjectiveInfoIcon5[256];
	char		m_ObjectiveIcon[256];
	char		m_LegacyMapMarkings[256];
	int			m_Priority;
	bool		m_bComplete, m_bFailed, m_bOptional, m_bDummy, m_bVisible;

	int m_ObjectiveIconTextureID;
	int GetObjectiveIconTextureID();

private:
	C_ASW_Objective( const C_ASW_Objective & ) = delete;
};

#endif /* CASW_OBJECTIVEINFO_H */
