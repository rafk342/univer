#include "unused.h"

#if 0

sf::Vector2f PathSegment::SegmentsGlobImageOffset(35, 380);
const char* ContactsSpritePath = "F:\\source\\lazarev\\lazarev\\assets\\relay_parts.png";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Globals
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ResetCoils()
{
	r_ChGS.GetCoil().Reset();
	r_ChBS.GetCoil().Reset();
	r_ChIP.GetCoil().Reset();
	r_Ch1M.GetCoil().Reset();
	r_Ch2M.GetCoil().Reset();
	r_ChDP.GetCoil().Reset();
	r_24SP.GetCoil().Reset();
	r_1P.GetCoil().Reset();
	r_2P.GetCoil().Reset();
	r_4P.GetCoil().Reset();
	r_4Str.GetCoil().Reset();
	r_2Str.GetCoil().Reset();
}

//void UpdateRelays()
//{
//	r_ChGS.UpdateRelay();
//	r_ChBS.UpdateRelay();
//	r_ChIP.UpdateRelay();
//	r_Ch1M.UpdateRelay();
//	r_Ch2M.UpdateRelay();
//	r_ChDP.UpdateRelay();
//	r_24SP.UpdateRelay();
//	r_1P.UpdateRelay();
//	r_2P.UpdateRelay();
//	r_3P.UpdateRelay();
//	r_4Str.UpdateRelay();
//	r_2Str.UpdateRelay();
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Contacts
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



Contact::Contact(ContactName_e name, RelayContactsGroup* group) :
	m_name(name),
	self_ContactGroup(group)
{

}


Contact::Contact(ContactName_e name, PathSegment* segment, RelayContactsGroup* group) :
	m_name(name),
	connected_segment(segment),
	self_ContactGroup(group)
{

}


void Contact::SetName(ContactName_e name)
{
	m_name = name;
}

ContactName_e Contact::GetName()
{
	return m_name;
}


void Contact::ConnectToSegment(PathSegment* segment)
{
	connected_segment = segment;
}


void Contact::ConnectToContactGroup(RelayContactsGroup* group)
{
	self_ContactGroup = group;
}


void Contact::SendSignalToConnectedSegment(bool signal)
{
	if (connected_segment)
		connected_segment->SendSignalToDestinations(signal);
}


CoilContact::CoilContact(RelayCoil* Coil)
{
	m_name = n_Coil;
	this->self_Coil = Coil;
}


void CoilContact::SendThroughCoil(bool signal)
{
	if (self_Coil)
		self_Coil->SendSignalThrough(signal);
}


void CoilContact::ConnectToCoil(RelayCoil* Coil)
{
	self_Coil = Coil;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												RelayContactsGroup
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



RelayContactsGroup::RelayContactsGroup(Relay* relay) : self_relay(relay)
{
	m_Contacts[n11].SetName(n11);
	m_Contacts[n12].SetName(n12);
	m_Contacts[n13].SetName(n13);

	m_Contacts[n11].ConnectToContactGroup(this);
	m_Contacts[n12].ConnectToContactGroup(this);
	m_Contacts[n13].ConnectToContactGroup(this);
}


Contact& RelayContactsGroup::GetContact(ContactName_e name)
{
	return m_Contacts[name];
}


void RelayContactsGroup::AttachToRelay(Relay* relay)
{
	self_relay = relay;
}

void RelayContactsGroup::SendTheSignalOnward(bool signal)
{
	auto state = self_relay->GetRelayState();

	if (state == n11_n12)
	{
		m_Contacts[n12].SendSignalToConnectedSegment(signal);
		m_Contacts[n11].SendSignalToConnectedSegment(signal);
	}
	if (state == n11_n13)
	{
		m_Contacts[n13].SendSignalToConnectedSegment(signal);
		m_Contacts[n11].SendSignalToConnectedSegment(signal);
	}
}

Relay* RelayContactsGroup::GetRelay()
{
	return self_relay;
}


void RelayContactGroup_Drawable::ManageSpriteState()
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


RelayContactGroup_Drawable::RelayContactGroup_Drawable(sf::Vector2f n11_pos, Relay* relay, bool invert_x, bool invert_y) :
	RelayContactsGroup(relay),
	WidgetsBase("F:\\source\\lazarev\\lazarev\\assets\\relay_parts.png")
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


void RelayContactGroup_Drawable::SetGroupPosition(sf::Vector2f pos)
{
	this->SetPosition(pos);
}


void RelayContactGroup_Drawable::DrawThisGroup()
{
	GetRelay()->UpdateRelay();
	ManageSpriteState();

	RenderRequests::InvokeWidgetUpdate([this]
		{
			RenderRequests::getWindow()->draw(m_sprite);
		});
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												RelayCoil
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



RelayCoil::RelayCoil()
{
	m_Contacts.left.SetName(n_Coil);
	m_Contacts.right.SetName(n_Coil);

	m_Contacts.left.ConnectToCoil(this);
	m_Contacts.right.ConnectToCoil(this);
}


bool RelayCoil::IsPowered()
{
	return isPoweredState;
}


void RelayCoil::Reset()
{
	IsUsedOnThisFrame = false;
	isPoweredState = false;
}


void RelayCoil::SetState(bool state)
{
	isPoweredState = state;
}


void RelayCoil::SendSignalThrough(bool signal)
{
	IsUsedOnThisFrame = true;
	isPoweredState = signal;
	m_Contacts.right.SendSignalToConnectedSegment(signal);

}


void RelayCoil::SetNextSegment(PathSegment* segment)
{
	m_Contacts.right.ConnectToSegment(segment);
}


Contact& RelayCoil::GetLeftContact()
{
	return m_Contacts.left;
}


Contact& RelayCoil::GetRightContact()
{
	return m_Contacts.right;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Relay
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



Relay::Relay(std::string name) : m_name(name)
{

}


void Relay::UpdateRelay()
{
	if (m_Coil.IsPowered())
	{
		m_current_state = n11_n12;
	}
	else
	{
		m_current_state = n11_n13;
	}
}


void Relay::ResetCoil()
{
	m_Coil.Reset();
}


RelayCoil& Relay::GetCoil()
{
	return m_Coil;
}


RelayState_e Relay::GetRelayState()
{
	return m_current_state;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



PathSegment::PathSegment(const std::filesystem::path& m_path) :
	WidgetsBase(m_path.string()),
	m_name(m_path.filename().replace_extension("").string())
{
	m_texture.setSmooth(true);
	m_sprite.setPosition(SegmentsGlobImageOffset);
}


const std::string& PathSegment::getName() const
{
	return m_name;
}


void PathSegment::PushContactAsDestination(Contact* contact)
{
	m_destinations.push_back(contact);
}


void PathSegment::SendSignalToDestinations(bool signal)
{
	for (auto contact : m_destinations)
	{
		if (contact->GetName() == n_Coil) {
			static_cast<CoilContact*>(contact)->SendThroughCoil(signal);
		}
		else {
			contact->SendSignalToConnectedSegment(signal);
		}
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



SchemeSegments::SchemeSegments()
{
	std::filesystem::directory_iterator it("F:\\source\\lazarev\\lazarev\\assets\\SchemeParts");
	m_Segments.reserve(20);

	for (auto& file_entry : it)
	{
		if (!file_entry.is_regular_file())
			continue;

		m_Segments.push_back(new PathSegment(file_entry.path()));
		const std::string& fName = m_Segments.back()->getName();

		m_SegmentsMap[fName] = m_Segments.back();
		m_SegmentsNames.push_back(fName);

		if (fName == "s_1_entry")
			s1_entry = m_Segments.back();

		if (fName == "s_1_output")
			s1_output = m_Segments.back();

		if (fName == "s_2_entry")
			s2_entry = m_Segments.back();

		if (fName == "s_2_output")
			s2_output = m_Segments.back();
	}

	SM_ASSERT((s1_entry && s1_output && s2_entry && s2_output), "SchemeSegments::SchemeSegments() -> entries or outputs were not found");

#define make_group(name, ...) {name, new RelayContactGroup_Drawable(__VA_ARGS__)}

	ContactsGroupsMap =
	{
		make_group(c_CHGS_1,	{255,805},	&r_ChGS),
		make_group(c_CHBS_1,	{447,805},	&r_ChBS),
		make_group(c_CHIP_1,	{262,968},	&r_ChIP),
		make_group(c_CHIP_2,	{996,1195}, &r_ChIP),
		make_group(c_CH1M_1,	{1210,1300},&r_Ch1M, true),
		make_group(c_CH2M_1,	{1084,546},	&r_Ch2M),
		make_group(c_CHPZ_1,	{1210,1420},&r_ChPZ, true),
		make_group(c_CHDP_1,	{1260,1060},&r_ChDP, true),
		make_group(c_2_4_SP,	{1070,1070},&r_24SP, true),
		make_group(c_2PK_1,		{787,935},	&r_2Str),
		make_group(c_2PK_2,		{787,793},	&r_2Str),
		make_group(c_2MK,		{785,670},	&r_2Str),
		make_group(c_4PK,		{998,797},	&r_4Str),
		make_group(c_4MK,		{997,934},	&r_4Str),
		make_group(c_1P,		{1189,665}, &r_1P),
		make_group(c_2P,		{1189,785}, &r_2P),
		make_group(c_4P,		{1190,930}, &r_4P),
	};

#define push_destination_of_segment(segment_name, group_name, contact_name) \
	m_SegmentsMap.at(segment_name)->PushContactAsDestination(&ContactsGroupsMap.at(group_name)->GetContact(contact_name));

#define attach_segment_to_contact(group_name, contact_name, segment_name) \
	ContactsGroupsMap.at(group_name)->GetContact(contact_name).ConnectToSegment(m_SegmentsMap.at(segment_name));

	push_destination_of_segment("s_1_entry", c_CHGS_1, n11);
	attach_segment_to_contact(c_CHGS_1, n13, "s_1_2");

	push_destination_of_segment("s_1_2", c_CHBS_1, n11);
	attach_segment_to_contact(c_CHBS_1, n13, "s_1_3");

	push_destination_of_segment("s_1_entry", c_CHIP_1, n11);
	attach_segment_to_contact(c_CHIP_1, n12, "s_1_3");

	push_destination_of_segment("s_1_3", c_CHIP_2, n11);
	push_destination_of_segment("s_1_3", c_CH1M_1, n12);
	push_destination_of_segment("s_1_3", c_CH2M_1, n11);

	attach_segment_to_contact(c_CH2M_1, n12, "s_1_15");
	attach_segment_to_contact(c_CHDP_1, n11, "s_1_5");
	attach_segment_to_contact(c_CHDP_1, n12, "s_1_12");

}



SchemeSegments::~SchemeSegments()
{
	for (auto ptr : m_Segments)
		delete ptr;

	for (auto [name, ptr] : ContactsGroupsMap)
		delete ptr;
}


void SchemeSegments::DrawSegments()
{
	RenderRequests::InvokeWidgetUpdate([this]
		{
			if (m_Segments.empty())
				return;

			for (auto SchemePart : m_Segments)
			{
				RenderRequests::getWindow()->draw(SchemePart->GetSprite());
			}

			for (auto [name, group] : ContactsGroupsMap)
			{
				RenderRequests::getWindow()->draw(group->GetSprite());
			}
		});
}


void SchemeSegments::SendSignalFromEntry(bool signal)
{
	s1_entry->SendSignalToDestinations(signal);
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



Scheme::Scheme()
{

}

void Scheme::DrawScheme()
{
	ResetCoils();
	m_SchemeSegments.DrawSegments();
	m_Overlay.DrawOverlay();
}

#endif
