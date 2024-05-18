
#include "Widgets/ImageButton.h"
#include "Scheme.h"

#define m_debug 0


sf::Vector2f PathSegment::SegmentsGlobImageOffset(35, 380);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Globals
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Relay r_ChGS("ChGS");
Relay r_ChBS("ChBS");
Relay r_ChIP("Chip");
Relay r_Ch1M("Ch1M");
Relay r_Ch2M("Ch2M");
Relay r_ChDP("ChDP");
Relay r_24SP("24SP");
Relay r_1P("1P");
Relay r_2P("2P");
Relay r_4P("3P");
Relay r_4Str("4Str");
Relay r_2Str("2Str");
Relay r_ChPZ("ChPZ");

std::array m_RelaysArray = { &r_ChGS, &r_ChBS, &r_ChIP, &r_Ch1M, &r_Ch2M, &r_ChDP, &r_24SP, &r_1P, &r_2P, &r_4P, &r_4Str, &r_2Str, &r_ChPZ };

void ResetCoils()
{
	for (auto relay : m_RelaysArray)
	{
		relay->GetCoil()->ResetCoil();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Contacts
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Contacts base

Contact::Contact(ContactType_e Type) : m_ContactType(Type) {}

void Contact::SendSignal_ToDestinationContacts(bool signal)
{
	for (auto dest : destinations)
	{
		auto type = dest->GetContactType();
#if m_debug
		std::cout << "sending to : " << type << " ptr : " << (void*)dest << std::endl;
#endif
		if (type == T_NONE)
		{
			static_cast<PathSegmentContact*>(dest)->SendSignalToSegment(signal);
		}
		else if (type == T_RELAY)
		{
			static_cast<RelayContact*>(dest)->SendSignalToGroup(signal);
		}
		else if (type == T_COIL)
		{
			static_cast<CoilContact*>(dest)->SendSignalToCoil(signal);
		}
	}
}

void			Contact::PushContactAsDestination(Contact* contact)		{ if (this != contact) destinations.push_back(contact); }
ContactType_e	Contact::GetContactType()								{ return m_ContactType; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									segment contacts



PathSegmentContact::PathSegmentContact() : Contact(T_NONE) {}
PathSegmentContact::PathSegmentContact(PathSegment* segment) : Contact(T_NONE) {}
void PathSegmentContact::ConnectToSegment(PathSegment* segment) { self_segment = segment; }

void PathSegmentContact::SendSignalToSegment(bool signal)
{
#if m_debug
	std::cout << "In____ PathSegmentContact::SendSignalToSegment " << std::endl;
#endif
	if (self_segment) 
	{
#if m_debug
		std::cout << " PathSegmentContact::SendSignalToSegment " << std::endl;
#endif
		self_segment->SendSignalThroughItself(this, signal);
	}
		
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									relays contacts


RelayContact::RelayContact(RelayContactName_e name, RelayContactsGroup* selfGroup)
	: Contact(T_RELAY)
	, m_name(name) 
	, selfContactGroup(selfGroup) { }


RelayContactName_e RelayContact::getContactName() { return m_name; }

void RelayContact::SendSignalToGroup(bool signal)
{
	if (!selfContactGroup)
		return;

	selfContactGroup->SendSignalThroughItself(this, signal);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									coils contact


CoilContact::CoilContact(RelayCoil* coil) : Contact(T_COIL), selfCoil(coil) {}

void CoilContact::SendSignalToCoil(bool signal)
{
	if (selfCoil)
		selfCoil->SendSignalThroughItself(this, signal);
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


void PathSegment::SendSignalThroughItself(PathSegmentContact* sender, bool signal)
{
	isActiveOnThisFrame = true;
	m_sprite.setColor(sf::Color(230, 0, 0, 255));

	if (sender == &m_Contacts[0])
	{
#if m_debug
		std::cout << m_name << " Path segment [1] -> sending to dest" << std::endl;
#endif
		m_Contacts[1].SendSignal_ToDestinationContacts(signal);
	}
	else if (sender == &m_Contacts[1])
	{
#if m_debug
		std::cout << m_name << " Path segment [0] -> sending to dest" << std::endl;
#endif
		m_Contacts[0].SendSignal_ToDestinationContacts(signal);
	}
}


bool					PathSegment::isActive()				{ return isActiveOnThisFrame; }
const std::string		PathSegment::getName()				{ return m_name; }
PathSegmentContact*		PathSegment::GetContact_1()			{ return &m_Contacts[0]; }
PathSegmentContact*		PathSegment::GetContact_2()			{ return &m_Contacts[1]; }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay Coil									
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



RelayCoil::RelayCoil() 
	: WidgetsBase("F:\\source\\lazarev\\lazarev\\assets\\relay_parts.png")
	, m_Contacts{this, this}
{
	helpers::InvertTexture(m_texture);
	m_sprite.setTexture(m_texture);

	m_texture.setSmooth(true);
	m_sprite.setTextureRect({0,0,73,73});
}


void RelayCoil::SendSignalThroughItself(CoilContact* sender, bool signal)
{
	isActiveOnThisFrame = true;
	m_sprite.setColor(sf::Color(255, 0, 0, 255));

	if (sender == &m_Contacts[0])
	{
		m_Contacts[1].SendSignal_ToDestinationContacts(signal);
	}
	else if (sender == &m_Contacts[1])
	{
		m_Contacts[0].SendSignal_ToDestinationContacts(signal);
	}

}


void RelayCoil::ResetCoil()
{
	wasActiveOnPrevFrame = isActiveOnThisFrame;

	if (groupToCheck)	// I couldn't come up with a better solution
	{
		if (groupToCheck->IsUsed())
			return;
	}

	isActiveOnThisFrame = false;
	m_sprite.setColor(sf::Color(0, 0, 0, 255));
}


bool			RelayCoil::isActive()									{ return isActiveOnThisFrame; }
void			RelayCoil::SetState(bool state)							{ isActiveOnThisFrame = state; }
CoilContact*	RelayCoil::GetContact_1()								{ return &m_Contacts[0]; }
CoilContact*	RelayCoil::GetContact_2()								{ return &m_Contacts[1]; }
void			RelayCoil::setGroupToCheck(RelayContactsGroup* group)	{ groupToCheck = group; }
void			RelayCoil::DrawCoil()									{ RenderRequests::getWindow()->draw(m_sprite); }

void RelayCoil::setLeftContactPos(sf::Vector2f point)
{
	point.y -= 73/2;
	SetPosition(point);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Relay Contact Group									
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



RelayContactsGroup::RelayContactsGroup(sf::Vector2f n11_pos, Relay* relay, bool invert_x, bool invert_y)
	: WidgetsBase("F:\\source\\lazarev\\lazarev\\assets\\relay_parts.png")
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
	auto state = self_relay->GetRelayState();

	if (state == n11_n12)
	{
		m_sprite.setTextureRect({ 0,80, 82, 6 });
	}
	if (state == n11_n13)
	{
		m_sprite.setTextureRect({ 0,107, 90, 32 });
	}
}


void RelayContactsGroup::SendSignalThroughItself(RelayContact* sender, bool signal)
{
	auto relay_state = self_relay->GetRelayState();
	auto sender_name = sender->getContactName();
	
	if ((relay_state == n11_n12 && sender_name == N13) || (relay_state == n11_n13 && sender_name == N12))
		return;

	IsUsedOnThisFrame = true;
#if m_debug
	std::cout << "relay contact group | state : " << relay_state << " | sender : " << sender_name << std::endl;
#endif
	if (relay_state == n11_n12 && sender_name == N11)
		m_Contacts[N12].SendSignal_ToDestinationContacts(signal);
	
	if (relay_state == n11_n12 && sender_name == N12)
		m_Contacts[N11].SendSignal_ToDestinationContacts(signal);

	if (relay_state == n11_n13 && sender_name == N11)
		m_Contacts[N13].SendSignal_ToDestinationContacts(signal);

	if (relay_state == n11_n13 && sender_name == N13)
		m_Contacts[N11].SendSignal_ToDestinationContacts(signal);
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


Relay::Relay(const char* name) : name(name) {}


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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Scheme 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



SchemeOverlay::SchemeOverlay() : WidgetsBase("F:\\source\\lazarev\\lazarev\\assets\\scheme_overlay.png")
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
	std::filesystem::directory_iterator it("F:\\source\\lazarev\\lazarev\\assets\\SchemeSegments2");
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
	}

#define make_group(name, ...) {name, new RelayContactsGroup(__VA_ARGS__)} 
#define AttachContactAsDestination(from, to) from->PushContactAsDestination(to)

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
		make_group(s1_c_2PK_1,	{787,935},	&r_2Str),
		make_group(s1_c_2PK_2,	{787,793},	&r_2Str),
		make_group(s1_c_2MK,	{785,670},	&r_2Str),
		make_group(s1_c_4PK,	{998,797},	&r_4Str),
		make_group(s1_c_4MK,	{997,934},	&r_4Str),
		make_group(s1_c_1P,		{1189,665}, &r_1P),
		make_group(s1_c_2P,		{1189,785}, &r_2P),
		make_group(s1_c_4P,		{1190,930}, &r_4P),

		//Scheme 2
		make_group(s2_c_CHGS1, {419,1590}, &r_ChGS),
		make_group(s2_c_CHBS1, {610,1648}, &r_ChBS),
		make_group(s2_c_CH2M,  {950,1715}, &r_Ch2M),
		make_group(s2_c_CHPZ,  {950,1883}, &r_ChPZ),
	};


#define PathContact1(name) m_PathSegmentsMap.at(name)->GetContact_1()
#define PathContact2(name) m_PathSegmentsMap.at(name)->GetContact_2()

#define GroupContact(name, contact_name) m_ContactGroupsMap.at(name)->getContact(contact_name)

	//Scheme 2
	
		//
		//  Purpose is to connect some Contact2 from the one path to Contact1 from another path
		// 
		//  #G - Group
		//  #P - Path
		//  #C - Coil
		// 
		//			#P										#G/#C											#P
		//  ( PATH NAME (Contact 1/2) ) @@@ ( Through something (can be group or coil or nothing) ) @@@ ( PATH NAME (Contact 1/2) )
		//
		//	Examples: 
		//
		//	From s2_2_1 Through s2_c_CHBS1 To s2_3_1:
		// 
		//	#P * s2_2_1 * C2   @@@   #G * s2_c_CHBS1 * N11  #G * s2_c_CHBS1 * N13   @@@    #P * s2_3_1 * C1
		// 
		// 	AttachContactAsDestination(m_PathSegmentsMap.at("s2_2_1")->GetContact_2(),      m_ContactGroupsMap.at(s2_c_CHBS1)->getContact(N11));
		//  AttachContactAsDestination(m_ContactGroupsMap.at(s2_c_CHBS1)->getContact(N13),  m_PathSegmentsMap.at("s2_3_1")->GetContact_1());
		//	
		//	From s2_3_1 To s2_3_2:
		//	
		//	#P * s2_3_1 * C2 @@@ @@@ #P * s2_3_2 * C1
		//	
		//	AttachContactAsDestination(m_PathSegmentsMap.at("s2_3_1")->GetContact_2(),		m_PathSegmentsMap.at("s2_3_2")->GetContact_1());
		//	
		//	

	AttachContactAsDestination(s2_entry_segment->GetContact_2(),					m_ContactGroupsMap.at(s2_c_CHGS1)->getContact(N11));
	AttachContactAsDestination(m_ContactGroupsMap.at(s2_c_CHGS1)->getContact(N13),	m_PathSegmentsMap.at("s2_2_1")->GetContact_1());
	
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_2_1")->GetContact_2(),		m_ContactGroupsMap.at(s2_c_CHBS1)->getContact(N11));
	AttachContactAsDestination(m_ContactGroupsMap.at(s2_c_CHBS1)->getContact(N13),	m_PathSegmentsMap.at("s2_3_1")->GetContact_1());
	
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_3_1")->GetContact_2(),		m_PathSegmentsMap.at("s2_3_2")->GetContact_1());
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_3_1")->GetContact_2(),		m_PathSegmentsMap.at("s2_3_3")->GetContact_1());
	
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_3_2")->GetContact_2(),		m_ContactGroupsMap.at(s2_c_CH2M)->getContact(N11));
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_3_3")->GetContact_2(),		m_ContactGroupsMap.at(s2_c_CHPZ)->getContact(N11));
	AttachContactAsDestination(m_ContactGroupsMap.at(s2_c_CH2M)->getContact(N12),	m_PathSegmentsMap.at("s2_4_3")->GetContact_1());
	AttachContactAsDestination(m_ContactGroupsMap.at(s2_c_CHPZ)->getContact(N12),	m_PathSegmentsMap.at("s2_4_1")->GetContact_1());
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_4_3")->GetContact_2(),		m_PathSegmentsMap.at("s2_4_2")->GetContact_1());
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_4_1")->GetContact_2(),		m_PathSegmentsMap.at("s2_4_2")->GetContact_1());
	AttachContactAsDestination(m_PathSegmentsMap.at("s2_4_2")->GetContact_2(),		r_ChPZ.GetCoil()->GetContact_1());
	AttachContactAsDestination(r_ChPZ.GetCoil()->GetContact_2(),					m_PathSegmentsMap.at("s2_5_1")->GetContact_1());

	r_ChPZ.GetCoil()->setLeftContactPos({1310, 1692});
	r_ChPZ.GetCoil()->setGroupToCheck(m_ContactGroupsMap.at(s2_c_CHPZ));

	//Scheme 1

	AttachContactAsDestination(s1_entry_segment->GetContact_2(),	PathContact1("s1_1_3"));
	AttachContactAsDestination(s1_entry_segment->GetContact_2(),	PathContact1("s1_1_2"));
	
	AttachContactAsDestination(PathContact2("s1_1_2"),				GroupContact(s1_c_CHGS_1, N11));
	AttachContactAsDestination(GroupContact(s1_c_CHGS_1, N13),		PathContact1("s1_2"));
	AttachContactAsDestination(PathContact2("s1_2"),				GroupContact(s1_c_CHBS_1, N11));
	AttachContactAsDestination(GroupContact(s1_c_CHBS_1, N13),		PathContact1("s1_3_1"));

	AttachContactAsDestination(PathContact2("s1_1_3"),				GroupContact(s1_c_CHIP_1, N11));
	AttachContactAsDestination(GroupContact(s1_c_CHIP_1, N12),		PathContact1("s1_3_2"));
		
	AttachContactAsDestination(PathContact2("s1_3_1"),				PathContact1("s1_3_11"));
	AttachContactAsDestination(PathContact2("s1_3_1"),				PathContact1("s1_3_9"));


} 



SchemeSegments::~SchemeSegments()
{
	for (auto ptr : m_PathSegments)
		delete ptr;
	
	for (auto [v1, ptr] : m_ContactGroupsMap)
		delete ptr;
}

void SchemeSegments::ResetSegments()
{
	for (auto path : m_PathSegments)
		path->ResetSegment();
	
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
				RenderRequests::getWindow()->draw(path->GetSprite());
			}

			for (auto [name, group] : m_ContactGroupsMap)
			{
				group->GetSelfRelay()->UpdateState();
				group->Draw();
			}

			r_ChPZ.GetCoil()->DrawCoil();
		});
}

void SchemeSegments::SendSignalFromEntry() 
{
	s2_entry_segment->SendSignalThroughItself(s2_entry_segment->GetContact_1(), true); 
	s1_entry_segment->SendSignalThroughItself(s1_entry_segment->GetContact_1(), true);
}



Scheme::Scheme() 
{
	
}

ImageButton chbs_btn("F:\\source\\sfml_test_app\\sfml_test_app\\resources\\buttons.png");
ImageButton m_btn("F:\\source\\sfml_test_app\\sfml_test_app\\resources\\buttons.png");

void Scheme::DrawScheme()
{
	static bool init = false;
	static bool coil_flag = false;
	static bool coil_flag2 = true;

	if (m_btn) {
		coil_flag = !coil_flag;
	}

	if (chbs_btn)
		coil_flag2 = !coil_flag2;


	ResetCoils();
	m_SchemeSegments.ResetSegments();


	if (!init)
	{
		m_btn.SetPosition({ 280, 1200 });
		m_btn.SetInactiveImageRectSprite({ 6,3,54,50 });
		m_btn.SetActiveImageRectSprite({ 262,3,54,50 });

		chbs_btn.SetPosition({ 280, 1280 });
		chbs_btn.SetInactiveImageRectSprite({ 6,3,54,50 });
		chbs_btn.SetActiveImageRectSprite({ 262,3,54,50 });
	}
	

	r_Ch2M.GetCoil()->SetState(coil_flag);
	r_ChBS.GetCoil()->SetState(coil_flag2);

	m_SchemeSegments.SendSignalFromEntry();


	m_SchemeSegments.DrawSegments();
	m_Overlay.DrawOverlay();

#if m_debug
	std::cout << "\n\n\n\n";
#endif

}
