#pragma once
#if 0

enum RelayContactType_e
{
	Sender,
	Reciever
};

enum ContactName_e
{
	n11, n12, n13, n_Coil
};

enum CoilContactName_e
{
	left, right
};

enum RelayState_e
{
	n11_n12,
	n11_n13,
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Forward decl
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Contact;
class Relay;
class PathSegment;
class RelayContactsGroup;
class RelayCoil;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Externs
//	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern Relay r_ChGS;
extern Relay r_ChBS;
extern Relay r_ChIP;
extern Relay r_Ch1M;
extern Relay r_Ch2M;
extern Relay r_ChDP;
extern Relay r_24SP;
extern Relay r_1P;
extern Relay r_2P;
extern Relay r_4P;
extern Relay r_4Str;
extern Relay r_2Str;
extern Relay r_ChPZ;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class Contact
{
protected:
	ContactName_e		m_name = n11;
	RelayContactsGroup* self_ContactGroup = nullptr;
	PathSegment* connected_segment = nullptr;

public:

	Contact() = default;
	Contact(ContactName_e name, RelayContactsGroup* group);
	Contact(ContactName_e name, PathSegment* segment, RelayContactsGroup* group);

	void			SetName(ContactName_e name);
	ContactName_e	GetName();
	void			ConnectToSegment(PathSegment* segment);
	void			ConnectToContactGroup(RelayContactsGroup* group);
	void			SendSignalToConnectedSegment(bool signal);

};


class CoilContact : public Contact
{
	RelayCoil* self_Coil = nullptr;

public:

	CoilContact() = default;
	CoilContact(RelayCoil* Coil);
	void		SendThroughCoil(bool signal);
	void		ConnectToCoil(RelayCoil* Coil);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class RelayContactsGroup
{
protected:

	Relay* self_relay = nullptr;
	Contact m_Contacts[3];

public:

	RelayContactsGroup(Relay* relay);

	Contact& GetContact(ContactName_e name);
	void		AttachToRelay(Relay* relay);
	void		SendTheSignalOnward(bool signal);
	Relay* GetRelay();
};


class RelayContactGroup_Drawable : public RelayContactsGroup, public WidgetsBase
{
	void ManageSpriteState();

public:

	RelayContactGroup_Drawable(sf::Vector2f n11_pos, Relay* relay, bool invert_x = false, bool invert_y = false);
	void SetGroupPosition(sf::Vector2f pos);
	void DrawThisGroup();

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class RelayCoil
{
	bool isPoweredState = false;
	bool IsUsedOnThisFrame = false;

	struct
	{
		CoilContact left;
		CoilContact right;

	} m_Contacts;

public:

	RelayCoil();
	void			SendSignalThrough(bool signal);
	void			SetNextSegment(PathSegment* segment);
	Contact& GetLeftContact();
	Contact& GetRightContact();
	bool			IsPowered();
	void			Reset();
	void			SetState(bool state);
};

class RelayCoil_Drawable
{

public:

};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class Relay
{
	std::string		m_name;
	RelayCoil		m_Coil;
	RelayState_e	m_current_state = n11_n13;

public:

	Relay(std::string name);
	void			UpdateRelay();
	void			ResetCoil();
	RelayCoil& GetCoil();
	RelayState_e	GetRelayState();
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class PathSegment : public WidgetsBase
{
	using DestinationsVecT = std::vector<Contact*>;

	static sf::Vector2f		SegmentsGlobImageOffset;
	std::string				m_name;
	DestinationsVecT		m_destinations;

public:

	PathSegment(const std::filesystem::path& m_path);
	void				SendSignalToDestinations(bool signal);
	const std::string& getName() const;
	void				PushContactAsDestination(Contact* contact);
};



enum ContactsGroupName_e
{
	c_CHGS_1,
	c_CHBS_1,
	c_CHIP_1,
	c_CHIP_2,
	c_CH1M_1,
	c_CH2M_1,
	c_CHPZ_1,
	c_CHDP_1,
	c_2_4_SP,
	c_2PK_1,
	c_2PK_2,
	c_2MK,
	c_4PK,
	c_4MK,
	c_1P,
	c_2P,
	c_4P,
};


class SchemeSegments
{
protected:

	std::vector<PathSegment*> m_Segments;
	std::map<std::string, PathSegment*> m_SegmentsMap;
	std::vector<std::string> m_SegmentsNames;

	PathSegment* s1_entry = nullptr;
	PathSegment* s1_output = nullptr;

	PathSegment* s2_entry = nullptr;
	PathSegment* s2_output = nullptr;

	std::map<ContactsGroupName_e, RelayContactGroup_Drawable*> ContactsGroupsMap;

public:

	SchemeSegments();
	~SchemeSegments();
	void DrawSegments();
	void SendSignalFromEntry(bool signal);
};



class SchemeOverlay : public WidgetsBase
{
public:

	SchemeOverlay();
	void DrawOverlay();

};


class Scheme
{
	SchemeSegments m_SchemeSegments;
	SchemeOverlay  m_Overlay;

public:

	Scheme();
	void DrawScheme();

};

#endif


