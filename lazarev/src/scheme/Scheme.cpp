
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

void ResetCoils()
{
	r_ChGS.GetCoil()->ResetCoil();
	r_ChBS.GetCoil()->ResetCoil();
	r_ChIP.GetCoil()->ResetCoil();
	r_Ch1M.GetCoil()->ResetCoil();
	r_Ch2M.GetCoil()->ResetCoil();
	r_ChDP.GetCoil()->ResetCoil();
	r_24SP.GetCoil()->ResetCoil();
	r_1P.GetCoil()->ResetCoil();
	r_2P.GetCoil()->ResetCoil();
	r_4P.GetCoil()->ResetCoil();
	r_4Str.GetCoil()->ResetCoil();
	r_2Str.GetCoil()->ResetCoil();
	r_ChPZ.GetCoil()->ResetCoil();
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

void			Contact::PushContactAsDestination(Contact* contact) { if (this != contact) { destinations.push_back(contact); } }
ContactType_e	Contact::GetContactType() { return m_ContactType; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									segments contacts



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


bool				PathSegment::isActive() { return isActiveOnThisFrame; }
const std::string	PathSegment::getName() { return m_name; }
PathSegmentContact* PathSegment::GetContact_1() { return &m_Contacts[0]; }
PathSegmentContact* PathSegment::GetContact_2() { return &m_Contacts[1]; }



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
	isActiveOnThisFrame = false;
	m_sprite.setColor(sf::Color(0, 0, 0, 255));
}


bool RelayCoil::isActive() { return isActiveOnThisFrame; }
void RelayCoil::SetState(bool state) { isActiveOnThisFrame = state; }
CoilContact* RelayCoil::GetContact_1() { return &m_Contacts[0]; }
CoilContact* RelayCoil::GetContact_2() { return &m_Contacts[1]; }

void RelayCoil::DrawCoil() { RenderRequests::getWindow()->draw(m_sprite); }

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

RelayContact* RelayContactsGroup::getContact(RelayContactName_e name)
{
	return &m_Contacts[name];
}


Relay* RelayContactsGroup::GetSelfRelay() { return self_relay; }




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

RelayCoil* Relay::GetCoil() { return &m_Coil; }



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
	m_PathSegments.reserve(20);

	for (auto& file_entry : it)
	{
		if (!file_entry.is_regular_file())
			continue;

		m_PathSegments.push_back(new PathSegment(file_entry.path()));
		m_PathSegmentsMap[m_PathSegments.back()->getName()] = m_PathSegments.back();

		if (m_PathSegments.back()->getName() == "s2_1_1")
			s2_entry_segment = m_PathSegments.back();
	}

#define make_group(name, ...) {name, new RelayContactsGroup(__VA_ARGS__)} 
#define AttachContactAsDestination(contact1, contact2) contact1->PushContactAsDestination(contact2)

	m_ContactGroupsMap =
	{
		make_group(s2_c_CHGS1, {419,1590}, &r_ChGS),
		make_group(s2_c_CHBS1, {610,1648}, &r_ChBS),
		make_group(s2_c_CH2M,  {950,1715}, &r_Ch2M),
		make_group(s2_c_CHPZ,  {950,1883}, &r_ChPZ),
	};

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
	for (auto ptr : m_PathSegments)
		ptr->ResetSegment();
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

void SchemeSegments::SendSignalFromEntry() { s2_entry_segment->SendSignalThroughItself(s2_entry_segment->GetContact_1(), true); }



Scheme::Scheme() {}

void Scheme::DrawScheme()
{
	ResetCoils();
	m_SchemeSegments.ResetSegments();

	static ImageButton chbs_btn("F:\\source\\sfml_test_app\\sfml_test_app\\resources\\buttons.png");
	static ImageButton m_btn("F:\\source\\sfml_test_app\\sfml_test_app\\resources\\buttons.png");

	static bool init = false;
	static bool coil_flag = false;
	static bool coil_flag2 = true;

	if (!init)
	{
		m_btn.SetPosition({ 280, 1200 });
		m_btn.SetInactiveImageRectSprite({ 6,3,54,50 });
		m_btn.SetActiveImageRectSprite({ 262,3,54,50 });

		chbs_btn.SetPosition({ 280, 1280 });
		chbs_btn.SetInactiveImageRectSprite({ 6,3,54,50 });
		chbs_btn.SetActiveImageRectSprite({ 262,3,54,50 });
	}
	
	if (m_btn) {
		coil_flag = !coil_flag;
	}

	if (chbs_btn)
	{
		coil_flag2 = !coil_flag2;
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
