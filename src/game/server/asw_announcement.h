#ifndef asw_announcement_h__
#define asw_announcement_h__

class CASW_Announcement : public CLogicalEntity 
{
public:
	DECLARE_CLASS( CASW_Announcement, CLogicalEntity );
	CASW_Announcement();
	~CASW_Announcement();

	virtual void Spawn();
	virtual void Think();
};

#endif // asw_announcement_h__