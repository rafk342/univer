﻿
#include "Widgets/ImageButton.h"
#include "Scheme.h"

#define m_debug 0

sf::Vector2f PathSegment::SegmentsGlobImageOffset(35, 380);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Variables
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* buttons_image_path				= "assets\\buttons.png";
const char* relay_parts_path				= "assets\\relay_parts.png";
const char* train_pic_path					= "assets\\train4.png";
const char* very_important_data_file_path	= "assets\\SomeVeryImportantData.txt";
const char* scheme_segments_path			= "assets\\SchemeSegments2";
const char* overlay_path					= "assets\\scheme_overlay.png";
const char* station_pic_path				= "assets\\station2.png";


Relay r_ChGS("r_ChGS");
Relay r_ChBS("r_ChBS");
Relay r_ChIP("r_ChIP");
Relay r_Ch1M("r_Ch1M");
Relay r_Ch2M("r_Ch2M");
Relay r_ChDP("r_ChDP");
Relay r_24SP("r_24SP");
Relay r_ChPZ("r_ChPZ");

Relay r_1P("r_1P");
Relay r_2P("r_2P");
Relay r_4P("r_4P");

Relay r_2MK("r_2MK");
Relay r_2PK("r_2PK");

Relay r_4PK("r_4PK");
Relay r_4MK("r_4MK");

std::array m_RelaysArray = 
{ 
	&r_ChGS,
	&r_ChBS,
	&r_ChIP,
	&r_Ch1M,
	&r_Ch2M,
	&r_ChDP,
	&r_24SP,
	&r_ChPZ,
	&r_1P,
	&r_2P,
	&r_4P,
	&r_2MK,
	&r_2PK,
	&r_4PK,
	&r_4MK,
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Functions
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ResetCoils()
{
	for (auto relay : m_RelaysArray)
	{
		relay->GetCoil()->ResetCoil();
	}
}

Relay* FindRelayByName(const std::string& name)
{
	auto it = std::ranges::find_if(m_RelaysArray, [=](Relay* relay)
		{
			return (name == relay->GetName());
		});

	return it == m_RelaysArray.end() ? nullptr : (*it);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Classes
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Contacts
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Contacts base

Contact::Contact(ContactType_e Type) 
	: m_ContactType(Type) 
{ }


bool Contact::SendSignal_ToDestinationContacts(bool signal)
{
	if (destinations.empty())
		return false;
	
	if (signal == false)
		return false;
	
	std::list<bool, hmcgr::StackFirstFitAllocator<bool, 200>> results;

	for (auto dest : destinations)
	{
		bool result = false;

		switch (dest->GetContactType())
		{
		case T_NONE:
			
			result = static_cast<PathSegmentContact*>(dest)->SendSignalToSegment(signal);
			if (!result && signal)
				static_cast<PathSegmentContact*>(dest)->SendSignalToSegment(false);

			break;
		case T_COIL:
			
			result = static_cast<CoilContact*>(dest)->SendSignalToCoil(signal);
			if (!result && signal)
				static_cast<CoilContact*>(dest)->SendSignalToCoil(false);

			break;
		case T_RELAY:
			
			result = static_cast<RelayContact*>(dest)->SendSignalToGroup(signal);
			if (!result && signal)
				static_cast<RelayContact*>(dest)->SendSignalToGroup(false);

			break;
		default:
			break;
		}

		results.push_back(result);
	}
	
	return std::ranges::any_of(results, [](bool val) { return val == true; });
}


void Contact::PushContactAsDestination(Contact* contact)
{
	if (this != contact)
		destinations.push_back(contact);
}

ContactType_e Contact::GetContactType()	{ return m_ContactType; }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									segment contacts



PathSegmentContact::PathSegmentContact() 
	: Contact(T_NONE) 
{ }

PathSegmentContact::PathSegmentContact(PathSegment* segment) 
	: Contact(T_NONE) 
{ }

void PathSegmentContact::ConnectToSegment(PathSegment* segment) { self_segment = segment; }


bool PathSegmentContact::SendSignalToSegment(bool signal)
{
	if (self_segment) 
		return self_segment->SendSignalThroughItself(this, signal);

	return false;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									relays contacts


RelayContact::RelayContact(RelayContactName_e name, RelayContactsGroup* selfGroup)
	: Contact(T_RELAY)
	, m_name(name) 
	, selfContactGroup(selfGroup)
{ }


RelayContactName_e RelayContact::getContactName() { return m_name; }

bool RelayContact::SendSignalToGroup(bool signal)
{
	if (!selfContactGroup)
		return false;

	return selfContactGroup->SendSignalThroughItself(this, signal);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									coils contact


CoilContact::CoilContact(RelayCoil* coil) 
	: Contact(T_COIL)
	, selfCoil(coil) 
{ }

bool CoilContact::SendSignalToCoil(bool signal)
{
	if (selfCoil)
		return selfCoil->SendSignalThroughItself(this, signal);
	
	return false;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Path Segments									
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



PathSegment::PathSegment(const std::filesystem::path& m_path) 
	: WidgetsBase(m_path.string())
	, m_name(m_path.filename().replace_extension("").string())
{
	m_Contacts[0].ConnectToSegment(this);
	m_Contacts[1].ConnectToSegment(this);

	helpers::InvertTexture(m_texture);
	m_sprite.setTexture(m_texture);

	m_texture.setSmooth(true);
	m_sprite.setPosition(SegmentsGlobImageOffset);
}


void PathSegment::ResetSegment()
{
	isActiveOnThisFrame = false;
	m_sprite.setColor(sf::Color(0,0,0,255));
}


bool PathSegment::SendSignalThroughItself(PathSegmentContact* sender, bool signal)
{
	if (isOutputSegment && signal)
	{
		isActiveOnThisFrame = signal;
		m_sprite.setColor(sf::Color(200, 0, 0, 255));
		return true;
	}

	if (isActiveOnThisFrame && signal)
		return true;

	isActiveOnThisFrame = signal;

	if (isActiveOnThisFrame)
		m_sprite.setColor(sf::Color(200, 0, 0, 255));
	else
		m_sprite.setColor(sf::Color(0, 0, 0, 255));


	if (sender == &m_Contacts[0])
	{
		return m_Contacts[1].SendSignal_ToDestinationContacts(signal);
	}
	else if (sender == &m_Contacts[1])
	{
		return m_Contacts[0].SendSignal_ToDestinationContacts(signal);
	}
}

void PathSegment::Draw()
{
	if(!isActiveOnThisFrame)
		m_sprite.setColor(sf::Color(0, 0, 0, 255));

	RenderRequests::getWindow()->draw(m_sprite);
}

void PathSegment::MarkSegmentAsOutput() { isOutputSegment = true; }


bool					PathSegment::isActive()				{ return isActiveOnThisFrame; }
const std::string		PathSegment::getName()				{ return m_name; }
PathSegmentContact*		PathSegment::GetContact_1()			{ return &m_Contacts[0]; }
PathSegmentContact*		PathSegment::GetContact_2()			{ return &m_Contacts[1]; }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay Coil									
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



RelayCoil::RelayCoil() 
	: WidgetsBase(relay_parts_path)
	, m_Contacts{this, this}
{
	helpers::InvertTexture(m_texture);
	m_sprite.setTexture(m_texture);

	m_texture.setSmooth(true);
	m_sprite.setTextureRect({0,0,73,73});
}


bool RelayCoil::SendSignalThroughItself(CoilContact* sender, bool signal)
{
	isActiveOnThisFrame = signal;

	if (isActiveOnThisFrame)
		m_sprite.setColor(sf::Color(255, 0, 0, 255));
	else
		m_sprite.setColor(sf::Color(0, 0, 0, 255));

	if (sender == &m_Contacts[0])
	{
		return m_Contacts[1].SendSignal_ToDestinationContacts(signal);
	}
	else if (sender == &m_Contacts[1])
	{
		return m_Contacts[0].SendSignal_ToDestinationContacts(signal);
	}
}


void RelayCoil::ResetCoil()
{
	wasActiveOnPrevFrame = isActiveOnThisFrame;

	if (groupToCheck)
	{
		if (groupToCheck->IsUsed()) 
			return;
	}

	isActiveOnThisFrame = false;
	m_sprite.setColor(sf::Color(0, 0, 0, 255));
}


void RelayCoil::setLeftContactPos(sf::Vector2f point)
{
	point.y -= 73/2;
	SetPosition(point);
}


void RelayCoil::DrawCoil()
{
	if (!isActiveOnThisFrame)
		m_sprite.setColor(sf::Color(0, 0, 0, 255));

	RenderRequests::getWindow()->draw(m_sprite);
}


bool			RelayCoil::isActive()				{ return isActiveOnThisFrame; }
void			RelayCoil::SetState(bool state)		{ isActiveOnThisFrame = state; }
CoilContact*	RelayCoil::GetContact_1()			{ return &m_Contacts[0]; }
CoilContact*	RelayCoil::GetContact_2()			{ return &m_Contacts[1]; }
void			RelayCoil::setGroupToCheck(RelayContactsGroup* group) { groupToCheck = group; }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay Contact Group									
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



RelayContactsGroup::RelayContactsGroup(sf::Vector2f n11_pos, Relay* relay, bool invert_x, bool invert_y)
	: WidgetsBase(relay_parts_path)
	, self_relay(relay)
	, m_Contacts
	{
		{N11, this}, 
		{N12, this}, 
		{N13, this},
	}
{
	m_texture.setSmooth(true);
	m_sprite.setPosition(n11_pos);
	if (invert_x)
	{
		m_sprite.scale({ -1, 1 });
	}
	if (invert_y)
	{
		m_sprite.scale({ 1, -1 });
	}

	ManageSpriteState();
}


void RelayContactsGroup::ManageSpriteState()
{
	switch (self_relay->GetRelayState())
	{
	case n11_n12:
		m_sprite.setTextureRect({ 0,80, 82, 6 });
		break;

	case n11_n13:
		m_sprite.setTextureRect({ 0,107, 90, 32 });
		break;

	default:
		break;
	}
}


bool RelayContactsGroup::SendSignalThroughItself(RelayContact* sender, bool signal)
{
	auto relay_state = self_relay->GetRelayState();
	auto sender_name = sender->getContactName();

	IsUsedOnThisFrame = signal;
	
	if ((relay_state == n11_n12 && sender_name == N13) || (relay_state == n11_n13 && sender_name == N12))
		return false;

	if (relay_state == n11_n12 && sender_name == N11)
		return m_Contacts[N12].SendSignal_ToDestinationContacts(signal);
	
	if (relay_state == n11_n12 && sender_name == N12)
		return m_Contacts[N11].SendSignal_ToDestinationContacts(signal);

	if (relay_state == n11_n13 && sender_name == N11)
		return m_Contacts[N13].SendSignal_ToDestinationContacts(signal);

	if (relay_state == n11_n13 && sender_name == N13)
		return m_Contacts[N11].SendSignal_ToDestinationContacts(signal);
	
	return false;
}


void RelayContactsGroup::Draw()
{
	ManageSpriteState();
	RenderRequests::getWindow()->draw(m_sprite);
}


RelayContact*	RelayContactsGroup::getContact(RelayContactName_e name)		{ return &m_Contacts[name]; }
Relay*			RelayContactsGroup::GetSelfRelay()							{ return self_relay; }
void			RelayContactsGroup::Reset()									{ IsUsedOnThisFrame = false; }
bool			RelayContactsGroup::IsUsed()								{ return IsUsedOnThisFrame; }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Relay::Relay(const char* name) 
	: m_name(name) 
{ }


void Relay::UpdateState()
{
	if (m_Coil.isActive())
	{
		m_CurrentState = n11_n12;
	}
	else
	{
		m_CurrentState = n11_n13;
	}
}


RelayState_e Relay::GetRelayState()
{
	UpdateState();
	return m_CurrentState;
}


RelayCoil*		Relay::GetCoil()		{ return &m_Coil; }
const char*		Relay::GetName()		{ return m_name; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Scheme 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




SchemeOverlay::SchemeOverlay() : WidgetsBase(overlay_path)
{
	m_texture.setSmooth(true);
	m_sprite.setPosition({ 0, 0 });
}


void SchemeOverlay::DrawOverlay()
{
	RenderRequests::InvokeWidgetUpdate([this]
		{
			RenderRequests::getWindow()->draw(m_sprite);
		});
}


SchemeSegments::SchemeSegments()
{
	std::filesystem::directory_iterator it(scheme_segments_path);
	m_PathSegments.reserve(50);

	for (auto& file_entry : it)
	{
		if (!file_entry.is_regular_file())
			continue;

		m_PathSegments.push_back(new PathSegment(file_entry.path()));
		m_PathSegmentsMap[m_PathSegments.back()->getName()] = m_PathSegments.back();

		if (m_PathSegments.back()->getName() == "s2_1_1")
			s2_entry_segment = m_PathSegments.back();

		if (m_PathSegments.back()->getName() == "s1_1_1_entry")
			s1_entry_segment = m_PathSegments.back();

		if (m_PathSegments.back()->getName() == "s1_15_6_output")
			m_PathSegments.back()->MarkSegmentAsOutput();
		
		if (m_PathSegments.back()->getName() == "s2_5_1")
			m_PathSegments.back()->MarkSegmentAsOutput();
	}

#define make_group(name, ...) {name, new RelayContactsGroup(__VA_ARGS__)} 

	m_ContactGroupsMap =
	{
		//Scheme 1
		make_group(s1_c_CHGS_1,	{255,805},	&r_ChGS),
		make_group(s1_c_CHBS_1,	{447,805},	&r_ChBS),
		make_group(s1_c_CHIP_1,	{262,968},	&r_ChIP),
		make_group(s1_c_CHIP_2,	{996,1195}, &r_ChIP),
		make_group(s1_c_CH1M_1,	{1210,1300},&r_Ch1M, true),
		make_group(s1_c_CH2M_1,	{1084,546},	&r_Ch2M),
		make_group(s1_c_CHPZ_1,	{1210,1420},&r_ChPZ, true),
		make_group(s1_c_CHDP_1,	{1260,1060},&r_ChDP, true),
		make_group(s1_c_2_4_SP,	{1070,1070},&r_24SP, true),

		make_group(s1_c_2PK_1,	{787,935},	&r_2PK),
		make_group(s1_c_2PK_2,	{787,793},	&r_2PK),
		make_group(s1_c_2MK,	{785,670},	&r_2MK),
		make_group(s1_c_4PK,	{998,797},	&r_4PK),
		make_group(s1_c_4MK,	{997,934},	&r_4MK),

		make_group(s1_c_1P,		{1189,665}, &r_1P),
		make_group(s1_c_2P,		{1189,785}, &r_2P),
		make_group(s1_c_4P,		{1190,930}, &r_4P),

		//Scheme 2
		make_group(s2_c_CHGS1, {419,1590}, &r_ChGS),
		make_group(s2_c_CHBS1, {610,1648}, &r_ChBS),
		make_group(s2_c_CH2M,  {950,1715}, &r_Ch2M),
		make_group(s2_c_CHPZ,  {950,1883}, &r_ChPZ),
	};
	
	std::map<std::string, ContactsGroupName_e> __StrContactGroupsMap__
	{
		{"s1_c_CHGS_1",s1_c_CHGS_1	},
		{"s1_c_CHBS_1",s1_c_CHBS_1	},
		{"s1_c_CHIP_1",s1_c_CHIP_1	},
		{"s1_c_CHIP_2",s1_c_CHIP_2	},
		{"s1_c_CH1M_1",s1_c_CH1M_1	},
		{"s1_c_CH2M_1",s1_c_CH2M_1	},
		{"s1_c_CHPZ_1",s1_c_CHPZ_1	},
		{"s1_c_CHDP_1",s1_c_CHDP_1	},
		{"s1_c_2_4_SP",s1_c_2_4_SP	},
		{"s1_c_2PK_1",s1_c_2PK_1	},
		{"s1_c_2PK_2",s1_c_2PK_2	},
		{"s1_c_2MK",s1_c_2MK		},
		{"s1_c_4PK",s1_c_4PK		},
		{"s1_c_4MK",s1_c_4MK		},
		{"s1_c_1P",s1_c_1P			},
		{"s1_c_2P",s1_c_2P			},
		{"s1_c_4P",s1_c_4P			},
		{"s2_c_CHGS1",s2_c_CHGS1	},
		{"s2_c_CHBS1",s2_c_CHBS1	},
		{"s2_c_CH2M",s2_c_CH2M		},
		{"s2_c_CHPZ",s2_c_CHPZ		},
	};

	std::map<std::string, RelayContactName_e> __StrRelayContactNamesMap__
	{
		{"N11",N11},
		{"N12",N12},
		{"N13",N13},
	};
	
	
	//#define AttachContactAsDestination(from, to) from->PushContactAsDestination(to)
	//
	//  Purpose is to connect some Contact from the one path to the Contact from another path
	// 
	//  #G - Group
	//  #P - Path
	//  #C - Coil
	// 
	//			#P										#G/#C											#P
	//  ( PATH NAME (Contact 1/2) ) @ ( Through something (can be group or coil or nothing) ) @ ( PATH NAME (Contact 1/2) )
	//
	//	Examples: 
	//
	//	From s2_2_1 Through s2_c_CHBS1 To s2_3_1:
	// 
	//	#P * s2_2_1 * C2   @   #G * s2_c_CHBS1 * N11   &   #G * s2_c_CHBS1 * N13   @    #P * s2_3_1 * C1
	// 
	// 	AttachContactAsDestination(m_PathSegmentsMap.at("s2_2_1")->GetContact_2(),      m_ContactGroupsMap.at(s2_c_CHBS1)->getContact(N11));
	//  AttachContactAsDestination(m_ContactGroupsMap.at(s2_c_CHBS1)->getContact(N13),  m_PathSegmentsMap.at("s2_3_1")->GetContact_1());
	//	
	//	From s2_3_1 To s2_3_2:
	//	
	//	#P * s2_3_1 * C2 @  @ #P * s2_3_2 * C1
	//					  ^
	//				  nothing here
	// 
	//	AttachContactAsDestination(m_PathSegmentsMap.at("s2_3_1")->GetContact_2(),		m_PathSegmentsMap.at("s2_3_2")->GetContact_1());
	//	
	//	

	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									file parser 
//
	std::ifstream input(very_important_data_file_path);
	SM_ASSERT(input.is_open(), "SchemeSegments::SchemeSegments() -> Unable to open some important data ");
	
	std::string line;
	line.reserve(100);

	while (std::getline(input, line))
	{
		if (line.find("//") != -1)
			continue;

		line = helpers::strip_string(line);

		if (line.empty())
			continue;

		std::array<std::string, 3> elements_array;
		for (auto [index, elem] : std::views::enumerate(helpers::split_string(line, "@") | std::views::take(3)))
		{
			elements_array[index] = helpers::strip_string(elem);
		}

		auto& [Path1_Contact2, WayThrough, Path2_Contact1] = elements_array;

		SM_ASSERT(Path1_Contact2.size() != 0, "SchemeSegments::SchemeSegments() -> Path1_Contact2 string len == 0");
		SM_ASSERT(Path2_Contact1.size() != 0, "SchemeSegments::SchemeSegments() -> Path2_Contact1 string len == 0");

		auto Path1_Contact_elems = helpers::split_string(Path1_Contact2, "* ");
		auto Path2_Contact_elems = helpers::split_string(Path2_Contact1, "* ");
		SM_ASSERT(Path1_Contact_elems.size() == 3 && Path2_Contact_elems.size() == 3, std::format("SchemeSegments::SchemeSegments()) {} ", Path1_Contact2));
		
		std::string& path1_name			= Path1_Contact_elems[1];
		std::string& path1_contact_name = Path1_Contact_elems[2];
			
		std::string& path2_name			= Path2_Contact_elems[1];
		std::string& path2_contact_name = Path2_Contact_elems[2];


		if (!WayThrough.empty())
		{
			for (auto [index, contact_str] : std::views::enumerate(helpers::split_string(WayThrough, "&")))
			{
				auto contact_str_elems = helpers::split_string(helpers::strip_string(contact_str), "*");
				SM_ASSERT(contact_str_elems.size() == 3, "SchemeSegments::SchemeSegments() contact_str_elems size != 3");

				char		 group_or_coil_type		= contact_str_elems[0][1];
				std::string& group_or_relay_name	= contact_str_elems[1];
				std::string& contact_type			= contact_str_elems[2];

				if (index == 0)
				{
					PathSegmentContact* contact_path1 = nullptr;

					if (path1_contact_name == "C2")
						contact_path1 = m_PathSegmentsMap.at(path1_name)->GetContact_2();

					if (path1_contact_name == "C1")
						contact_path1 = m_PathSegmentsMap.at(path1_name)->GetContact_1();

					if (group_or_coil_type == 'G' && contact_path1)
					{
						contact_path1->PushContactAsDestination(m_ContactGroupsMap.at(__StrContactGroupsMap__.at(group_or_relay_name))
							->getContact(__StrRelayContactNamesMap__.at(contact_type)));
					}
					if (group_or_coil_type == 'C' && contact_path1)
					{
						Relay* relay = FindRelayByName(group_or_relay_name);
						if (relay)
						{
							if (contact_type == "C1")
								contact_path1->PushContactAsDestination(relay->GetCoil()->GetContact_1());
							if (contact_type == "C2")
								contact_path1->PushContactAsDestination(relay->GetCoil()->GetContact_2());
						}
					}
				}

				if (index == 1)
				{
					PathSegmentContact* dest_contact = nullptr;

					if (path2_contact_name == "C1")
						dest_contact = m_PathSegmentsMap.at(path2_name)->GetContact_1();
					if (path2_contact_name == "C2")
						dest_contact = m_PathSegmentsMap.at(path2_name)->GetContact_2();
					
					if (dest_contact)
					{
						if (group_or_coil_type == 'G')
						{

							m_ContactGroupsMap.at(__StrContactGroupsMap__.at(group_or_relay_name))
								->getContact(__StrRelayContactNamesMap__.at(contact_type))
									->PushContactAsDestination(dest_contact);
						}

						if (group_or_coil_type == 'C')
						{
							Relay* relay = FindRelayByName(group_or_relay_name);
							if (relay)
							{
								if (contact_type == "C1")
									relay->GetCoil()->GetContact_1()->PushContactAsDestination(dest_contact);
								
								if (contact_type == "C2")
									relay->GetCoil()->GetContact_2()->PushContactAsDestination(dest_contact);
							}
						}
					}
				}
			}
		}
		else
		{
			PathSegmentContact* contact1 = nullptr;

			if (path1_contact_name == "C1")
				contact1 = m_PathSegmentsMap.at(path1_name)->GetContact_1();
			if (path1_contact_name == "C2")
				contact1 = m_PathSegmentsMap.at(path1_name)->GetContact_2();

			PathSegmentContact* contact2 = nullptr;

			if (path2_contact_name == "C1")
				contact2 = m_PathSegmentsMap.at(path2_name)->GetContact_1();
			if (path2_contact_name == "C2")
				contact2 = m_PathSegmentsMap.at(path2_name)->GetContact_2();

			if (contact1 && contact2)
				contact1->PushContactAsDestination(contact2);
		}

	}

	r_ChPZ.GetCoil()->setLeftContactPos({ 1310, 1692 });
	r_ChPZ.GetCoil()->setGroupToCheck(m_ContactGroupsMap.at(s2_c_CHPZ));
	r_Ch1M.GetCoil()->setLeftContactPos({1588, 1277});
	r_Ch1M.GetCoil()->setGroupToCheck(m_ContactGroupsMap.at(s1_c_CH1M_1));
	r_Ch2M.GetCoil()->setLeftContactPos({1589, 521});
	r_Ch2M.GetCoil()->setGroupToCheck(m_ContactGroupsMap.at(s1_c_CH2M_1));
} 



SchemeSegments::~SchemeSegments()
{
	for (auto ptr : m_PathSegments)
		delete ptr;
	
	for (auto [v1, ptr] : m_ContactGroupsMap)
		delete ptr;
}


void SchemeSegments::ResetPathSegments()
{
	for (auto path : m_PathSegments)
		path->ResetSegment();
}


void SchemeSegments::ResetConactGroups()
{
	for (auto [name, group] : m_ContactGroupsMap)
		group->Reset();
}


void SchemeSegments::DrawSegments()
{
	RenderRequests::InvokeWidgetUpdate([this]
		{
			if (m_PathSegments.empty())
				return;

			for (auto path : m_PathSegments)
			{
				path->Draw();
			}

			for (auto [name, group] : m_ContactGroupsMap)
			{
				group->GetSelfRelay()->UpdateState();
				group->Draw();
			}
			r_Ch1M.GetCoil()->DrawCoil();
			r_ChPZ.GetCoil()->DrawCoil();
			r_Ch2M.GetCoil()->DrawCoil();

			m_Station.Draw();
		});
}


void SchemeSegments::SendSignalFromEntry() 
{
	s1_entry_segment->SendSignalThroughItself(s1_entry_segment->GetContact_1(), true);
	s2_entry_segment->SendSignalThroughItself(s2_entry_segment->GetContact_1(), true); 
}

Station& SchemeSegments::GetStation() { return m_Station; }





Scheme::Scheme()
{ }


void Text_SetColPos(sf::Text& text, sf::Vector2f vec)
{
	text.setFillColor(sf::Color::Black);
	text.setPosition(vec);
}



void Scheme::DrawScheme()
{
	static bool init = false;
	static bool coil_flag = false;
	static bool coil_flag2 = true;
	static bool coil_flag3 = false;
	static bool coil_flag4 = false;
	static bool coil_flag5 = false;
	static bool coil_flag6 = false;

	static ImageButton chbs_btn(buttons_image_path);
	static ImageButton m_btn(buttons_image_path);
	static ImageButton m_btn2(buttons_image_path);
	static ImageButton m_btn3(buttons_image_path);
	static ImageButton m_btn4(buttons_image_path);
	static ImageButton m_btn5(buttons_image_path);
	static TwoStatesButton m_2StrBtn(buttons_image_path);
	static TwoStatesButton m_4StrBtn(buttons_image_path);
	
	static sf::Text m_text1(L"ЧБС", m_SFMLRenderer.get_font(), 35);
	static sf::Text m_text2(L"ЧИП", m_SFMLRenderer.get_font(), 35);
	static sf::Text m_text3(L"ЧДП", m_SFMLRenderer.get_font(), 35);
	static sf::Text m_text4(L"2-4 СП", m_SFMLRenderer.get_font(), 35);
	static sf::Text m_text5(L"�1�", m_SFMLRenderer.get_font(), 35);
	static sf::Text m_text6(L"Стрелка 2", m_SFMLRenderer.get_font(), 35);
	static sf::Text m_text7(L"Стрелка 4", m_SFMLRenderer.get_font(), 35);
	
	Text_SetColPos(m_text1, { 210, 1280 });
	Text_SetColPos(m_text2, { 210, 1360 });
	Text_SetColPos(m_text3, { 210, 1440 });
	Text_SetColPos(m_text4, { 210, 1520 });
	Text_SetColPos(m_text5, { 210, 1600 });
	Text_SetColPos(m_text6, { 210, 1680 });
	Text_SetColPos(m_text7, { 210, 1760 });
	
	RenderRequests::InvokeWidgetUpdate([]
		{
			//RenderRequests::getWindow()->draw(m_text);
			RenderRequests::getWindow()->draw(m_text1);
			RenderRequests::getWindow()->draw(m_text2);
			RenderRequests::getWindow()->draw(m_text3);
			RenderRequests::getWindow()->draw(m_text4);
			//RenderRequests::getWindow()->draw(m_text5);
			RenderRequests::getWindow()->draw(m_text6);
			RenderRequests::getWindow()->draw(m_text7);
		});


	if (!init)
	{
		m_btn.SetPosition({ 150, 1200 });
		m_btn.SetInactiveImageRectSprite({ 6,3,54,50 });
		m_btn.SetActiveImageRectSprite({ 262,3,54,50 });

		chbs_btn.SetPosition({ 150, 1280 });
		chbs_btn.SetInactiveImageRectSprite({ 6,3,54,50 });
		chbs_btn.SetActiveImageRectSprite({ 262,3,54,50 });

		m_btn2.SetPosition({ 150, 1360 });
		m_btn2.SetInactiveImageRectSprite({ 6,3,54,50 });
		m_btn2.SetActiveImageRectSprite({ 262,3,54,50 });

		m_btn3.SetPosition({ 150, 1440 });
		m_btn3.SetInactiveImageRectSprite({ 6,3,54,50 });
		m_btn3.SetActiveImageRectSprite({ 262,3,54,50 });

		m_btn4.SetPosition({ 150, 1520 });
		m_btn4.SetInactiveImageRectSprite({ 6,3,54,50 });
		m_btn4.SetActiveImageRectSprite({ 262,3,54,50 });

		m_btn5.SetPosition({ 150, 1600 });
		m_btn5.SetInactiveImageRectSprite({ 6,3,54,50 });
		m_btn5.SetActiveImageRectSprite({ 262,3,54,50 });

		m_4StrBtn.SetPosition({ 150, 1760 });
		m_4StrBtn.SetFalseStateSpriteRect({ 6,67, 54,50 });
		m_4StrBtn.SetTrueStateSpriteRect({ 6,131, 54,50 });

		m_2StrBtn.SetPosition({ 150, 1680 });
		m_2StrBtn.SetFalseStateSpriteRect({ 6,67, 54,50 });
		m_2StrBtn.SetTrueStateSpriteRect({ 6,131, 54,50 });

		init = true;
	}

	/*if (m_btn) 
		coil_flag = !coil_flag;*/
	
	if (chbs_btn)
		coil_flag2 = !coil_flag2;

	if (m_btn2)
		coil_flag3 = !coil_flag3;

	if (m_btn3)
		coil_flag4 = !coil_flag4;

	if (m_btn4)
		coil_flag5 = !coil_flag5;
	
	//if (m_btn5)
	//	coil_flag6 = !coil_flag6;
	
	if (m_2StrBtn) {}
	if (m_4StrBtn) {}

	ResetCoils();
	m_SchemeSegments.ResetPathSegments();
	m_SchemeSegments.ResetConactGroups();

	m_SchemeSegments.GetStation().Update();

	r_2PK.GetCoil()->SetState(!m_2StrBtn.getState());
	r_2MK.GetCoil()->SetState(m_2StrBtn.getState());
	r_4PK.GetCoil()->SetState(!m_4StrBtn.getState());
	r_4MK.GetCoil()->SetState(m_4StrBtn.getState());

	//r_Ch2M.GetCoil()->SetState(coil_flag);
	r_ChBS.GetCoil()->SetState(coil_flag2);
	r_ChIP.GetCoil()->SetState(coil_flag3);
	r_ChDP.GetCoil()->SetState(coil_flag4);
	r_24SP.GetCoil()->SetState(coil_flag5);

	m_SchemeSegments.SendSignalFromEntry();

	m_SchemeSegments.DrawSegments();
	m_Overlay.DrawOverlay();
	
#if m_debug
	std::cout << "\n\n\n\n";
#endif
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Train 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void Train::UpdateHeadAndTailPos()
{
	auto pos = m_sprite.getPosition();
	auto texture_sz = m_texture.getSize();
	m_TailPos = { pos.x, pos.y - texture_sz.y / 2 };
	m_HeadPos = { pos.x + texture_sz.x, pos.y - texture_sz.y / 2 };
}

Train::Train()
	: WidgetsBase(train_pic_path)
{
	m_texture.setSmooth(true);
	SetPosition({ 500, 143 });
	m_sprite.setOrigin(m_texture.getSize().x, m_texture.getSize().y );
}


void Train::Draw()
{
	RenderRequests::getWindow()->draw(m_sprite);
}


void Train::SetPosition(const sf::Vector2f& new_pos)
{
	m_TailPos = { new_pos.x - m_sprite.getGlobalBounds().getSize().x, new_pos.y };
	m_HeadPos = { new_pos.x , new_pos.y  };
	
	m_sprite.setPosition(new_pos);
}


sf::Vector2f Train::GetHeadPos() { return m_HeadPos; }
sf::Vector2f Train::GetTailPos() { return m_TailPos; }


void Train::FollowTheMouse(TrainRoute* route)
{
	if (!m_SFMLRenderer.get_sfWindow()->hasFocus())
		return;

	bool is_mouse_pressed_on_this_frame = sf::Mouse::isButtonPressed(sf::Mouse::Left);
	static bool cought = false;
	static sf::Vector2f mouse_offset;

	if (is_hovered() && is_mouse_pressed_on_this_frame && !cought)
	{
		cought = true;
		mouse_offset = m_sprite.getPosition() - m_SFMLRenderer.get_world_mouse_position();
	}
	else if (cought && is_mouse_pressed_on_this_frame)
	{
		sf::Vector2f targetHeadPos = route->GetTrainPos(m_sprite.getPosition(), mouse_offset);
		targetHeadPos.x += mouse_offset.x;
		SetPosition(targetHeadPos);
		m_sprite.setRotation(route->GetTrainRot(m_HeadPos, m_TailPos));

		float rotationRad = m_sprite.getRotation() * (PI / 180.0);
		float offsetX = -((float)m_texture.getSize().x);
		float offsetY = 0;
		float rotatedOffsetX = offsetX * cos(rotationRad) - offsetY * sin(rotationRad);
		float rotatedOffsetY = offsetX * sin(rotationRad) + offsetY * cos(rotationRad);

		m_TailPos = { m_sprite.getPosition().x - +rotatedOffsetX, m_sprite.getPosition().y + rotatedOffsetY};

	}
	else if (!is_mouse_pressed_on_this_frame)
	{
		cought = false;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Route 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1

TrainRoute::TrainRoute(RouteName_e Route) 
	: m_CurrentRoute(Route)
{ }

TrainRoute::TrainRoute(std::initializer_list<sf::Vector2f> il) 
	: m_BasePoints(il) 
{ }

TrainRoute::TrainRoute(RouteName_e Route, std::initializer_list<sf::Vector2f> il) 
	: m_CurrentRoute(Route)
	, m_BasePoints(il)
{ }

void TrainRoute::SetupLerpPoints(std::initializer_list<sf::Vector2f> il) 
{
	m_BasePoints.clear(); 
	m_BasePoints = il;
}

#endif

std::pair<sf::Vector2f, sf::Vector2f> TrainRoute::GetRailwaySegment(sf::Vector2f pos) 
{
	std::pair<sf::Vector2f, sf::Vector2f> points;
	bool found = false;

	for (size_t i = 0; i < m_BasePoints.size() - 1; ++i)
	{
		const sf::Vector2f& pointA = m_BasePoints[i];
		const sf::Vector2f& pointB = m_BasePoints[i + 1];

		if (pointB.x >= pos.x && pointA.x <= pos.x)
		{
			found = true;
			points = { pointA, pointB };
			break;
		}
	}

	if (!found)
		return{};
	
	return points;
}


sf::Vector2f TrainRoute::GetTrainPos(sf::Vector2f train_head, sf::Vector2f mouse_offset)
{
	auto mouse_pos = m_SFMLRenderer.get_world_mouse_position();

	if (mouse_pos.x + mouse_offset.x < 257)
		mouse_pos.x = 257 - mouse_offset.x;

	if (mouse_pos.x + mouse_offset.x > 1978)
		mouse_pos.x = 1978 - mouse_offset.x;
	
	auto current_segment = GetRailwaySegment(train_head);

	float mid_y_point = std::lerp(current_segment.first.y, current_segment.second.y, helpers::NormalizeValue(current_segment.first.x, current_segment.second.x, train_head.x));

	SM_ASSERT(!std::isnan(mid_y_point), "Lerp returned nan");

	return {mouse_pos.x, mid_y_point};
}

float TrainRoute::GetTrainRot(sf::Vector2f train_head, sf::Vector2f train_tail)
{
	auto current_segment = GetRailwaySegment(train_tail);
	float mid_y_point = std::lerp(current_segment.first.y, current_segment.second.y, helpers::NormalizeValue(current_segment.first.x, current_segment.second.x, train_tail.x));
	SM_ASSERT(!std::isnan(mid_y_point), "Lerp returned nan");

	return helpers::angleBetweenPoints({ train_tail.x, mid_y_point }, train_head);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Station 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Station::Update()
{
	m_Train.FollowTheMouse(&m_Routes[2]);

}


Station::Station()
	: WidgetsBase(station_pic_path) 
	, m_Routes {
		{ At_1_line,
			{
				{78, 262},
				{1140, 262},
				{1270, 181},
				{1989, 181},
			}
		},
		{ At_2_line,
			{
				{78, 262},
				{1989, 262},
			}
		},
		{ At_4_line,
			{
				{78, 262},
				{1245, 262},
				{1371, 343},
				{1989, 343},
			}
		},
	}
{
	m_texture.setSmooth(true);

}

void Station::Draw()
{
	RenderRequests::getWindow()->draw(m_sprite);

	if (train_should_be_drawn)
		m_Train.Draw();
}

