#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <filesystem>
#include <ranges>
#include <fstream>
#include <list>
#include <ctime>

#include "Cougar/FixedSizeAllocator.h"

#include "base/WidgetsBase.h"
#include "helpers/Helpers.h"
#include "base/SimpleTimer.h"
#include "src/Widgets/ImageButton.h"


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

class TwoStatesButton;



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
extern Relay r_ChPZ;

extern Relay r_1P;
extern Relay r_2P;
extern Relay r_4P;

extern Relay r_2MK;
extern Relay r_2PK;
extern Relay r_4MK;
extern Relay r_4PK;


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

enum RouteName_e
{
	At_1_line,
	At_2_line,
	At_4_line,
};


enum StationSegments_e
{
	s_None = 0,
	s_Chip = 1,
	s_Chdp = 2,
	s_2_4_SP = 3,
	s_1p = 4,
	s_2p = 5,
	s_4p = 6,
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
	bool			SendSignal_ToDestinationContacts(bool signal);

};



class PathSegmentContact : public Contact
{
	PathSegment* self_segment = nullptr;

public:

	PathSegmentContact();
	PathSegmentContact(PathSegment* segment);
	void ConnectToSegment(PathSegment* segment);
	bool SendSignalToSegment(bool signal);

};


class CoilContact : public Contact
{
	RelayCoil* selfCoil = nullptr;

public:

	CoilContact(RelayCoil* coil);
	bool SendSignalToCoil(bool signal);

};


class RelayContact : public Contact
{
	RelayContactName_e m_name = N11;
	RelayContactsGroup* selfContactGroup = nullptr;

public:

	RelayContact(RelayContactName_e name, RelayContactsGroup* selfGroup);
	RelayContactName_e getContactName();
	bool SendSignalToGroup(bool signal);

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
	bool					isOutputSegment = false;

public:

	PathSegment(const std::filesystem::path& m_path);

	PathSegmentContact* GetContact_1();
	PathSegmentContact* GetContact_2();
	const std::string	getName();

	void ResetSegment();
	bool isActive();
	bool SendSignalThroughItself(PathSegmentContact* sender, bool signal);
	void Draw();
	void MarkSegmentAsOutput();
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
	RelayContactsGroup* groupToCheck = nullptr;


public:

	RelayCoil();

	CoilContact* GetContact_1();
	CoilContact* GetContact_2();

	bool SendSignalThroughItself(CoilContact* sender, bool signal);
	void ResetCoil();
	bool isActive();
	void SetState(bool state);
	void DrawCoil();
	void setLeftContactPos(sf::Vector2f point);
	bool PrevFrameState();
	void setGroupToCheck(RelayContactsGroup* group);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay Contact Group


class RelayContactsGroup : public WidgetsBase
{
	Relay*			self_relay = nullptr;
	RelayContact	m_Contacts[3];
	bool			IsUsedOnThisFrame = false;

public:

	RelayContactsGroup(sf::Vector2f n11_pos, Relay* relay, bool invert_x = false, bool invert_y = false);

	void			ManageSpriteState();
	Relay*			GetSelfRelay();
	bool			SendSignalThroughItself(RelayContact* sender, bool signal);
	void			Draw();
	RelayContact*	getContact(RelayContactName_e name);
	bool			IsUsed();
	void			Reset();
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay


// it's mostly an abstract thing
class Relay
{
	const char*		m_name;
	RelayCoil		m_Coil;
	RelayState_e	m_CurrentState = n11_n13;

public:

	Relay(const char* name);

	RelayCoil*		GetCoil();
	RelayState_e	GetRelayState();
	void			UpdateState();
	const char*		GetName();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Station
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class TrainRoute
{

	RouteName_e					m_ThisRouteType = At_1_line;
	std::vector<sf::Vector2f>	m_BasePoints; // interpolation points
	
	auto GetLerpPointsBasedOnTrainPos(sf::Vector2f train_pos) -> std::pair<sf::Vector2f, sf::Vector2f>;

public:

	TrainRoute(RouteName_e Route);
	TrainRoute(std::initializer_list<sf::Vector2f> il);
	TrainRoute(RouteName_e Route, std::initializer_list<sf::Vector2f> il);
	
	void			SetupLerpPoints(std::initializer_list<sf::Vector2f> il);
	sf::Vector2f	GetTrainPos(sf::Vector2f train_head, sf::Vector2f offset);
	float			GetTrainRot(sf::Vector2f train_head, sf::Vector2f train_tail);

};


class Train : public WidgetsBase
{
	sf::Vector2f m_HeadPos{};
	sf::Vector2f m_TailPos{};

public:

	Train();
	
	void			Draw();
	void			SetPosition(const sf::Vector2f& new_pos);
	sf::Vector2f	GetHeadPos();
	sf::Vector2f	GetTailPos();
	void			FollowTheMouse(TrainRoute* route);
	void			ResetPosition();

};


class Station : public WidgetsBase
{
	TrainRoute  m_Routes[3];
	Train	    m_Train;
	RouteName_e m_CurrentRoute = At_1_line;
	RouteName_e m_RequestedRoute = At_1_line;
	
	//		segment            ->			left / right coord
	std::map<StationSegments_e, std::pair<sf::Vector2f, sf::Vector2f>> m_StationSegments;

	TwoStatesButton m_2StrButton;
	TwoStatesButton m_4StrButton;
	
	TwoStatesButton m_1_RouteButton;
	TwoStatesButton m_2_RouteButton;
	TwoStatesButton m_4_RouteButton;
	ImageButton		m_RouteUnlockButton;

	std::array<TwoStatesButton*, 3> RouteButtons;

	bool m_RequestTheForRouteUnlock = false;
	bool m_IsRouteLocked = false;

	auto GetCurrentTrainLocation() -> std::pair<StationSegments_e, StationSegments_e>;  // returns current segments for Head and tail positions
	bool VerifySafetyConditions_ForRequestedRoute(RouteName_e RequestedRoute);
	bool VerifyIfRouteCanBeUnlocked();
	void UnlockTheCurrentRoute();

public:

	Station();
	void Update_1Stage();
	void Update_2Stage();
	void Draw();
	Train& GetTrain();

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
	PathSegment*				s1_entry_segment = nullptr;
	Station						m_Station;

public:

	SchemeSegments();
	~SchemeSegments();

	void ResetPathSegments();
	void ResetConactGroups();
	void DrawSegments();
	void SendSignalFromEntry();
	Station& GetStation();
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
