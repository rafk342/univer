#pragma once
#include <vector>
#include <array>
#include <filesystem>

#include "Widgets/ImageButton.h"
#include "base/WidgetsBase.h"
#include "base/RenderRequests.h"
#include "helpers/Helpers.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Forward Decl
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Contact;
class PathSegmentContact;
class PathSegment;
class SchemeSegments;
class SchemeOverlay;
class RelayCoil;
class Relay;
class RelayContactsGroup;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Externs
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Enums
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


enum ContactType_e
{
	T_NONE,
	T_COIL,
	T_RELAY,
};

enum RelayContactName_e
{
	N11,
	N12,
	N13
};

enum RelayState_e
{
	n11_n12,
	n11_n13,
};

enum ContactsGroupName_e
{
	s1_c_CHGS_1,
	s1_c_CHBS_1,
	s1_c_CHIP_1,
	s1_c_CHIP_2,
	s1_c_CH1M_1,
	s1_c_CH2M_1,
	s1_c_CHPZ_1,
	s1_c_CHDP_1,
	s1_c_2_4_SP,
	s1_c_2PK_1,
	s1_c_2PK_2,
	s1_c_2MK,
	s1_c_4PK,
	s1_c_4MK,
	s1_c_1P,
	s1_c_2P,
	s1_c_4P,
	
	s2_c_CHGS1,
	s2_c_CHBS1,
	s2_c_CH2M,
	s2_c_CHPZ,
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Classes
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Contacts
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Contact -> ( RelayContact / CoilContact / PathSegment )


class Contact
{
protected:

	ContactType_e			m_ContactType;
	std::vector<Contact*>	destinations;

	Contact(ContactType_e Type);

public:

	ContactType_e	GetContactType();
	void			PushContactAsDestination(Contact* contact);
	void			SendSignal_ToDestinationContacts(bool signal);

};



class PathSegmentContact : public Contact
{
	PathSegment* self_segment = nullptr;

public:

	PathSegmentContact();
	PathSegmentContact(PathSegment* segment);
	void ConnectToSegment(PathSegment* segment);
	void SendSignalToSegment(bool signal);

};


class CoilContact : public Contact
{
	RelayCoil* selfCoil = nullptr;

public:

	CoilContact(RelayCoil* coil);
	void SendSignalToCoil(bool signal);

};


class RelayContact : public Contact
{
	RelayContactName_e m_name = N11;
	RelayContactsGroup* selfContactGroup = nullptr;

public:

	RelayContact(RelayContactName_e name, RelayContactsGroup* selfGroup);
	RelayContactName_e getContactName();
	void SendSignalToGroup(bool signal);

};





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Scheme Segments
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Path

class PathSegment : public WidgetsBase
{
	static sf::Vector2f		SegmentsGlobImageOffset;
	PathSegmentContact		m_Contacts[2];
	std::string				m_name;
	bool					isActiveOnThisFrame = false;

public:

	PathSegment(const std::filesystem::path& m_path);
	const std::string getName();
	PathSegmentContact* GetContact_1();
	PathSegmentContact* GetContact_2();
	void ResetSegment();
	bool isActive();
	void SendSignalThroughItself(PathSegmentContact* sender, bool signal);

};


//
//	Coil -> relay <-> (vec contact groups)
//


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Coil


class RelayCoil : public WidgetsBase
{
	CoilContact			m_Contacts[2];
	bool				isActiveOnThisFrame = false;
	bool				wasActiveOnPrevFrame = false;
	RelayContactsGroup*	groupToCheck = nullptr;


public:

	RelayCoil();
	CoilContact* GetContact_1();
	CoilContact* GetContact_2();
	void SendSignalThroughItself(CoilContact* sender, bool signal);
	void ResetCoil();
	bool isActive();
	void SetState(bool state);
	void DrawCoil();
	void setLeftContactPos(sf::Vector2f point);

	void setGroupToCheck(RelayContactsGroup* group);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay Contact Group


class RelayContactsGroup : public WidgetsBase
{
	Relay* self_relay = nullptr;
	RelayContact m_Contacts[3];
	bool IsUsedOnThisFrame = false;

public:

	RelayContactsGroup(sf::Vector2f n11_pos, Relay* relay, bool invert_x = false, bool invert_y = false);
	void ManageSpriteState();
	Relay* GetSelfRelay();
	void SendSignalThroughItself(RelayContact* sender, bool signal);
	void Draw();
	RelayContact* getContact(RelayContactName_e name);
	bool IsUsed();
	void Reset();
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay


// mostly abstract thing
class Relay
{
	const char* name;
	RelayCoil m_Coil;
	RelayState_e m_CurrentState = n11_n13;

public:

	void UpdateState();
	Relay(const char* name);
	RelayCoil* GetCoil();
	RelayState_e GetRelayState();

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Scheme
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class SchemeSegments
{
protected:

	using PathsSegmentsMapType = std::map<std::string, PathSegment*>;
	using ContactGroupsMapType = std::map<ContactsGroupName_e, RelayContactsGroup*>;

	std::vector<PathSegment*>	m_PathSegments;
	PathsSegmentsMapType		m_PathSegmentsMap;
	ContactGroupsMapType		m_ContactGroupsMap;
	PathSegment*				s2_entry_segment = nullptr;
	
public:

	SchemeSegments();
	~SchemeSegments();
	void ResetSegments();
	void DrawSegments();
	void SendSignalFromEntry();

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
