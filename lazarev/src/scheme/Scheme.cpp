﻿
#include "Scheme.h"
#include "base/RenderRequests.h"

namespace _win {
#include "windows.h"
}

#define __debug 0


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Globals
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace
{
	const char* buttons_image_path				= "assets\\buttons.png";
	const char* relay_parts_path				= "assets\\relay_parts.png";
	const char* train_pic_path					= "assets\\train4.png";
	const char* very_important_data_file_path	= "assets\\SomeVeryImportantData.txt";
	const char* scheme_segments_path			= "assets\\SchemeSegments2";
	const char* overlay_path					= "assets\\scheme_overlay.png";
	const char* station_pic_path				= "assets\\station2.png";
	const char* trackside_lights_path =			"assets\\tracklight.png";

	int DefaultTrain_X_AxisLimit_LeftSide = 257;
	int DefaultTrain_X_AxisLimit_RightSide = 1978;

	int TrainLimit_on_X_axis_LeftSide = 257;
	int TrainLimit_on_X_axis_RightSide = 1978;
}


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

static std::array s_RelaysArray = 
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
	for (auto relay : s_RelaysArray)
	{
		relay->GetCoil()->ResetCoil();
	}
}

Relay* FindRelayByName(const std::string& name)
{
	auto it = std::ranges::find_if(s_RelaysArray, [=](Relay* relay)
		{
			return (name == relay->GetName());
		});

	return it == s_RelaysArray.end() ? nullptr : (*it);
}

void Text_SetColAndPos(sf::Text& text, sf::Vector2f pos)
{
	text.setFillColor(sf::Color::Black);
	text.setPosition(pos);
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

	//if (signal == false)
	//	return false;

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
	
	return std::ranges::any_of(results, [](bool v) { return v == true; });
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


PathSegment::PathSegment(const std::filesystem::path& _path)
	: WidgetsBase(_path.string())
	, m_name(_path.filename().replace_extension("").string())
{
	m_Contacts[0].ConnectToSegment(this);
	m_Contacts[1].ConnectToSegment(this);

	helpers::InvertTexture(m_texture);
	m_sprite.setTexture(m_texture);

	m_texture.setSmooth(true);
	m_sprite.setPosition(SegmentsGlobImageOffset);
	ResetSegment();
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
		return true;
	}

	if (isActiveOnThisFrame && signal)
		return true;

	isActiveOnThisFrame = signal;

	int OtherContactIdx = (sender == &m_Contacts[0]) ? 1 : (sender == &m_Contacts[1]) ? 0 : -1;
	if (OtherContactIdx != -1) 
	{
		return m_Contacts[OtherContactIdx].SendSignal_ToDestinationContacts(signal);
	}
	return false;
}


void PathSegment::ManageSpriteColor()
{
	if (isActiveOnThisFrame) {
		m_sprite.setColor(sf::Color(200, 0, 0, 255));
	} else {
		m_sprite.setColor(sf::Color(0, 0, 0, 255));
	}
}


void PathSegment::Draw()
{
	ManageSpriteColor();
	RenderRequests::getWindow()->draw(m_sprite);
}

void					PathSegment::MarkSegmentAsOutput()	{ isOutputSegment = true; }
bool					PathSegment::isActive()				{ return isActiveOnThisFrame; }
const std::string&		PathSegment::getName()				{ return m_name; }
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
	wasActiveOnPrevFrame = isActiveOnThisFrame;
	isActiveOnThisFrame = signal;

	int OtherContactIdx = (sender == &m_Contacts[0]) ? 1 : (sender == &m_Contacts[1]) ? 0 : -1;
	if (OtherContactIdx != -1)
	{
		return m_Contacts[OtherContactIdx].SendSignal_ToDestinationContacts(signal);
	}
	return false;
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
	point.y -= m_sprite.getTextureRect().height / 2;
	SetPosition(point);
}


void RelayCoil::DrawCoil()
{
	if (isActiveOnThisFrame) {
		m_sprite.setColor(sf::Color(255, 0, 0, 255));
	} else {
		m_sprite.setColor(sf::Color(0, 0, 0, 255));
	}
	RenderRequests::getWindow()->draw(m_sprite);
}


bool			RelayCoil::isActive()				{ return isActiveOnThisFrame; }
void			RelayCoil::SetState(bool state)		{ isActiveOnThisFrame = state; }
CoilContact*	RelayCoil::GetContact_1()			{ return &m_Contacts[0]; }
CoilContact*	RelayCoil::GetContact_2()			{ return &m_Contacts[1]; }
void			RelayCoil::setGroupToCheck(RelayContactsGroup* group) { groupToCheck = group; }
bool			RelayCoil::PrevFrameState()			{ return wasActiveOnPrevFrame; }



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

	sf::Vector2f scale;
	scale.x = invert_x ? -1 : 1;
	scale.y = invert_y ? -1 : 1;
	m_sprite.scale(scale);

	ManageSpriteState();
}


void RelayContactsGroup::ManageSpriteState()
{
	if (!self_relay) {
		m_sprite.setTextureRect({ 0,107, 90, 32 });
		return;
	}

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
	if (!self_relay)
		return false;

	auto relay_state = self_relay->GetRelayState();
	auto sender_name = sender->getContactName();

	IsUsedOnThisFrame = signal;
	
	if ((relay_state == n11_n12 && sender_name == N13) || (relay_state == n11_n13 && sender_name == N12))
	{
		IsUsedOnThisFrame = false;
		return false;
	}

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
//									Train 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



Train::Train()
	: WidgetsBase(train_pic_path)
{
	m_texture.setSmooth(true);
	SetPosition({ 277, 262 });
	m_sprite.setOrigin(m_texture.getSize().x, m_texture.getSize().y/2 );
}


void Train::Draw()
{
	RenderRequests::getWindow()->draw(m_sprite);
}


void Train::SetPosition(sf::Vector2f new_pos)
{
	m_HeadPos = { new_pos.x , new_pos.y  };
	m_sprite.setPosition(new_pos);
}


bool Train::FollowTheMouse(TrainRoute* route)
{
	if (!g_SFMLRenderer.get_sfWindow()->hasFocus())
		return false;

	bool is_mouse_pressed_on_this_frame = sf::Mouse::isButtonPressed(sf::Mouse::Left);

	if (is_hovered() && is_mouse_pressed_on_this_frame && !m_cought)
	{
		m_cought = true;
		m_mouse_offset = m_sprite.getPosition() - g_SFMLRenderer.GetWorldMousePos();
	}
	else if (m_cought && is_mouse_pressed_on_this_frame)
	{
		sf::Vector2f targetHeadPos = route->GetTrainPos(m_sprite.getPosition(), m_mouse_offset);
		SetPosition(targetHeadPos);
		m_sprite.setRotation(route->GetTrainRot(m_HeadPos, (m_sprite.getPosition() - m_sprite.getGlobalBounds().getSize()) ));

		float rotationRad = m_sprite.getRotation() * (PI / 180.0);
		float offsetX = -((float)m_texture.getSize().x);
		float offsetY = 0;
		float rotatedOffsetX = offsetX * cos(rotationRad) - offsetY * sin(rotationRad);
		float rotatedOffsetY = offsetX * sin(rotationRad) + offsetY * cos(rotationRad);

		m_TailPos = { m_sprite.getPosition().x + rotatedOffsetX, m_sprite.getPosition().y + rotatedOffsetY };
	}
	else if (!is_mouse_pressed_on_this_frame)
	{
		m_cought = false;
	}
	return m_cought;
}

void Train::ResetPosition()
{
	SetPosition({ 277, 262 });
	TrainLimit_on_X_axis_LeftSide = DefaultTrain_X_AxisLimit_LeftSide;

	m_HeadPos = { 277.0f, 262.0f };
	m_TailPos = { 277.0f - m_texture.getSize().x, 262.0f };
}




sf::Vector2f	Train::GetHeadPos()			{ return m_HeadPos; }
sf::Vector2f	Train::GetTailPos()			{ return m_TailPos; }




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Route 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if 1

TrainRoute::TrainRoute(RouteName_e Route) 
	: m_ThisRouteType(Route)
{ }

TrainRoute::TrainRoute(std::initializer_list<sf::Vector2f> il) 
	: m_BasePoints(il) 
{ }

TrainRoute::TrainRoute(RouteName_e Route, std::initializer_list<sf::Vector2f> il) 
	: m_ThisRouteType(Route)
	, m_BasePoints(il)
{ }

void TrainRoute::SetupLerpPoints(std::initializer_list<sf::Vector2f> il) 
{
	m_BasePoints.clear(); 
	m_BasePoints = il;
}

#endif


std::pair<sf::Vector2f, sf::Vector2f> TrainRoute::GetLerpPointsBasedOnTrainPos(sf::Vector2f pos) 
{
	std::pair<sf::Vector2f, sf::Vector2f> points = {};

	for (size_t i = 0; i < m_BasePoints.size() - 1; ++i)
	{
		sf::Vector2f pointA = m_BasePoints[i];
		sf::Vector2f pointB = m_BasePoints[i + 1];

		if (pointB.x >= pos.x && pointA.x <= pos.x)
		{
			points = { pointA, pointB };
			break;
		}
	}
	return points;
}


sf::Vector2f TrainRoute::GetTrainPos(sf::Vector2f train_head, sf::Vector2f mouse_offset)
{
	sf::Vector2f mouse_pos = g_SFMLRenderer.GetWorldMousePos();
	std::pair<sf::Vector2f, sf::Vector2f> CurrSegmentLerpP = GetLerpPointsBasedOnTrainPos(train_head);


	if (mouse_pos.x + mouse_offset.x < TrainLimit_on_X_axis_LeftSide)
		mouse_pos.x = TrainLimit_on_X_axis_LeftSide - mouse_offset.x;

	if (mouse_pos.x + mouse_offset.x > TrainLimit_on_X_axis_RightSide)
		mouse_pos.x = TrainLimit_on_X_axis_RightSide - mouse_offset.x;

	///TrainLimit_on_X_axis_LeftSide = mouse_pos.x + mouse_offset.x;

	float mid_y_point = std::lerp(CurrSegmentLerpP.first.y, CurrSegmentLerpP.second.y, math::NormalizeValue(CurrSegmentLerpP.first.x, CurrSegmentLerpP.second.x, train_head.x));
	SM_ASSERT(!std::isnan(mid_y_point), "TrainRoute::GetTrainPos -> Lerp returned nan!");

	mouse_pos.x += mouse_offset.x;

	return { mouse_pos.x, mid_y_point};
}


float TrainRoute::GetTrainRot(sf::Vector2f train_head, sf::Vector2f train_tail)
{
	auto current_segment = GetLerpPointsBasedOnTrainPos(train_tail);
	float mid_y_point = std::lerp(current_segment.first.y, current_segment.second.y, math::NormalizeValue(current_segment.first.x, current_segment.second.x, train_tail.x));
	SM_ASSERT(!std::isnan(mid_y_point), "TrainRoute::GetTrainRot -> Lerp returned nan!");

	return math::angleBetweenPoints({ train_tail.x, mid_y_point }, train_head);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									TrackLights 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TracksideLights::Light::Light(sf::Vector2f base_pos, sf::Vector2f pos)
	: WidgetsBase(trackside_lights_path)
{
	m_texture.setSmooth(true);
	m_sprite.setTexture(m_texture);
	m_sprite.setTextureRect({ 82, 569, 44, 44 });
	m_sprite.setPosition(base_pos + pos);
}

void TracksideLights::Light::Draw()
{
	RenderRequests::getWindow()->draw(m_sprite);
}

TracksideLights::TracksideLights(sf::Vector2f pos)
	: WidgetsBase(trackside_lights_path)
	, m_Lights
	{ 
		{ pos, { 41.0f, 38.0f  } },
		{ pos, { 41.0f, 101.0f } },
		{ pos, { 41.0f, 207.0f } },
		{ pos, { 41.0f, 270.0f } },
		{ pos, { 41.0f, 397.0f } },
	}
{
	m_texture.setSmooth(true);
	m_sprite.setTexture(m_texture);
	m_sprite.setTextureRect({ 0, 0, 126, 549 });
	SetPosition(pos);
	for (auto& light : m_Lights)
	{
		light.GetSprite().setColor(sm_Black);
	}
}

void TracksideLights::SwitchState(LightsState_e state)
{
	//if (m_isInTransition)
	//	return;

	if (m_CurrentState == state)
		return;

	m_TransitionRequest = true;
	m_NextState = state;
}


sf::Color TracksideLights::GetFlashingScalar()
{
	m_FlashingTimer.Update();
	double elapsedTime = m_FlashingTimer.GetElapsedSeconds();

	if (elapsedTime > FLASH_DURATION)
	{
		m_isReverseFlashing = !m_isReverseFlashing;
		m_FlashingTimer.Reset();
		m_FlashingTimer.Update();
		elapsedTime = m_FlashingTimer.GetElapsedSeconds();
	}

	double normalizedTime = math::mapRange(elapsedTime, 0, FLASH_DURATION, 0, 1);
	double easedTime = math::easeInOutSine(normalizedTime);

	sf::Color resultColor;
	if (m_isReverseFlashing)
	{
		resultColor.r = static_cast<uint8_t>(std::lerp(sm_White.r, sm_Black.r, 1 - easedTime));
		resultColor.g = static_cast<uint8_t>(std::lerp(sm_White.g, sm_Black.g, 1 - easedTime));
		resultColor.b = static_cast<uint8_t>(std::lerp(sm_White.b, sm_Black.b, 1 - easedTime));
	}
	else
	{
		resultColor.r = static_cast<uint8_t>(std::lerp(sm_White.r, sm_Black.r, easedTime));
		resultColor.g = static_cast<uint8_t>(std::lerp(sm_White.g, sm_Black.g, easedTime));
		resultColor.b = static_cast<uint8_t>(std::lerp(sm_White.b, sm_Black.b, easedTime));
	}
	resultColor.a = 255;
	
	if (resultColor.r < 5)
		m_isLockedForTransition = false;
	else
		m_isLockedForTransition = true;

	return resultColor;
}

void TracksideLights::ApplyScalarToColor(sf::Color& col, sf::Color scalar)
{
	col.r = static_cast<uint8_t>(static_cast<float>(col.r) * static_cast<float>(scalar.r) / 255.0f);
	col.g = static_cast<uint8_t>(static_cast<float>(col.g) * static_cast<float>(scalar.g) / 255.0f);
	col.b = static_cast<uint8_t>(static_cast<float>(col.b) * static_cast<float>(scalar.b) / 255.0f);
	col.a = static_cast<uint8_t>(static_cast<float>(col.a) * static_cast<float>(scalar.a) / 255.0f);
}

void TracksideLights::TransitionTest()
{
	if (!m_TransitionRequest)
		return;

	if (m_isFlashing)
	{
		if (!m_isLockedForTransition)
		{
			m_FlashingTimer.Stop();
			TriggerTransition();
			return;
		}
		else
		{
			return; // try again on the next frame
		}
	}
	else
	{
		TriggerTransition();
	}
}

void TracksideLights::TriggerTransition()
{
	m_isInTransition = true;
	m_TransitionRequest = false;
	m_TransitionTimer.Start();
}

void TracksideLights::DoTransition()
{
	m_TransitionTimer.Update();
	double t = m_TransitionTimer.GetElapsedSeconds();
	constexpr double half_duration = TRANSITION_DURATION * 0.5;
	
	bool is_halfway = t > half_duration;
	if (!is_halfway)
	{
		double normalizedTime = math::mapRange(t, 0, half_duration, 0, 1);
	
		for (auto& light : m_Lights) 
		{
			sf::Color currColor = light.GetSprite().getColor();
			sf::Color resultColor;

			resultColor.r = static_cast<uint8_t>(std::lerp(currColor.r, 0,  normalizedTime));	 //1 -
			resultColor.g = static_cast<uint8_t>(std::lerp(currColor.g, 0, normalizedTime));	 //1 -
			resultColor.b = static_cast<uint8_t>(std::lerp(currColor.b, 0, normalizedTime));	// 1 -
			resultColor.a = 255;
	
			light.GetSprite().setColor(resultColor);
		}
	}
	else
	{
		double normalizedTime = math::mapRange(t, half_duration, TRANSITION_DURATION, 0, 1);

		switch (m_NextState)
		{
		case TracksideLights::ONE_GREEN:
		{
			sf::Color resultColor;

			resultColor.r = static_cast<uint8_t>(std::lerp(sm_Black.r, sm_Green.r, normalizedTime));
			resultColor.g = static_cast<uint8_t>(std::lerp(sm_Black.g, sm_Green.g, normalizedTime));
			resultColor.b = static_cast<uint8_t>(std::lerp(sm_Black.b, sm_Green.b, normalizedTime));
			resultColor.a = 255;

			m_Lights[1].GetSprite().setColor(resultColor);
			break;
		}
		case TracksideLights::ONE_YELLOW:
		case TracksideLights::ONE_YELLOW_FLASHING:
		{
			sf::Color resultColor;

			resultColor.r = static_cast<uint8_t>(std::lerp(sm_Black.r, sm_Yellow.r, normalizedTime));
			resultColor.g = static_cast<uint8_t>(std::lerp(sm_Black.g, sm_Yellow.g, normalizedTime));
			resultColor.b = static_cast<uint8_t>(std::lerp(sm_Black.b, sm_Yellow.b, normalizedTime));
			resultColor.a = 255;

			m_Lights[0].GetSprite().setColor(resultColor);
			break;
		}
		case TracksideLights::TWO_YELLOW:
		case TracksideLights::TWO_YELLOW_UPPER_FLASHING:
		{
			sf::Color resultColor;

			resultColor.r = static_cast<uint8_t>(std::lerp(sm_Black.r, sm_Yellow.r, normalizedTime));
			resultColor.g = static_cast<uint8_t>(std::lerp(sm_Black.g, sm_Yellow.g, normalizedTime));
			resultColor.b = static_cast<uint8_t>(std::lerp(sm_Black.b, sm_Yellow.b, normalizedTime));
			resultColor.a = 255;

			m_Lights[0].GetSprite().setColor(resultColor);
			m_Lights[3].GetSprite().setColor(resultColor);
			break;
		}
		case TracksideLights::ONE_RED:
		{
			sf::Color resultColor;

			resultColor.r = static_cast<uint8_t>(std::lerp(sm_Black.r, sm_Red.r, normalizedTime));
			resultColor.g = static_cast<uint8_t>(std::lerp(sm_Black.g, sm_Red.g, normalizedTime));
			resultColor.b = static_cast<uint8_t>(std::lerp(sm_Black.b, sm_Red.b, normalizedTime));
			resultColor.a = 255;

			m_Lights[2].GetSprite().setColor(resultColor);
			break;
		}
		default:
			break;
		}
	}
}




void TracksideLights::Draw()
{
	if (!m_isInTransition)
	{
		switch (m_CurrentState)
		{
		case TracksideLights::ONE_GREEN:
		{
			m_Lights[1].GetSprite().setColor(sm_Green);
			break;
		}
		case TracksideLights::ONE_YELLOW_FLASHING:
		{
			sf::Color scalar = GetFlashingScalar();
			sf::Color flashing_yellow = sm_Yellow;
			ApplyScalarToColor(flashing_yellow, scalar);
			m_Lights[0].GetSprite().setColor(flashing_yellow);

			break;
		}
		case TracksideLights::ONE_YELLOW:
		{
			m_Lights[0].GetSprite().setColor(sm_Yellow);
			break;
		}
		case TracksideLights::TWO_YELLOW_UPPER_FLASHING:
		{
			sf::Color scalar = GetFlashingScalar();
			sf::Color flashing_yellow = sm_Yellow;
			ApplyScalarToColor(flashing_yellow, scalar);
			m_Lights[0].GetSprite().setColor(flashing_yellow);
			m_Lights[3].GetSprite().setColor(sm_Yellow);
			break;
		}
		case TracksideLights::TWO_YELLOW:
		{
			m_Lights[0].GetSprite().setColor(sm_Yellow);
			m_Lights[3].GetSprite().setColor(sm_Yellow);

			break;
		}
		case TracksideLights::ONE_RED:
		{
			m_Lights[2].GetSprite().setColor(sm_Red);
			break;
		}
		default:
			break;
		}
	}
	else
	{
		DoTransition();
		if (m_TransitionTimer.GetElapsedSeconds() > TRANSITION_DURATION)
		{
			m_isInTransition = false;
			m_CurrentState = m_NextState;
			m_TransitionTimer.Stop();
			m_TransitionTimer.Reset();
			m_TransitionTimer.Update();
		}
		m_FlashingTimer.Reset();
		if (m_CurrentState == ONE_YELLOW_FLASHING || m_CurrentState == TWO_YELLOW_UPPER_FLASHING)
		{
			m_FlashingTimer.Start();
			m_isFlashing = true;
		}
		else
		{
			m_FlashingTimer.Stop();
			m_isFlashing = false;
		}
	}
	TransitionTest();

	RenderRequests::getWindow()->draw(m_sprite);
	for (auto& light : m_Lights)
	{
		light.Draw();
	}
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Station 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



Station::Station()
	: WidgetsBase(station_pic_path)
	, m_Routes{
		{ At_1_line,
			{
				{58, 262},
				{1140, 262},
				{1270, 181},
				{1989, 181},
			}
		},
		{ At_2_line,
			{
				{58, 262},
				{1989, 262},
			}
		},
		{ At_4_line,
			{
				{58, 262},
				{1245, 262},
				{1371, 343},
				{1989, 343},
			}
		},
	}
	, m_Lights({ 2030, 1100 })
	, m_StationSegments{
		{ s_None, {{58, 262},{315, 262}}},
		{ s_Chip, {{315, 262},{655, 262}}},
		{ s_Chdp, {{655, 262},{963, 262}}},
		{ s_2_4_SP, {{963, 170},{1449, 355}}},
		{ s_1p, {{1449, 181},{2020, 181}}},
		{ s_2p, {{1449, 262},{2020, 262}}},
		{ s_4p, {{1449, 343},{2020, 343}}},
	}
	, m_2StrButton(buttons_image_path)
	, m_4StrButton(buttons_image_path)
	, m_1_RouteButton(buttons_image_path)
	, m_2_RouteButton(buttons_image_path)
	, m_4_RouteButton(buttons_image_path)
	, m_RouteUnlockButton(buttons_image_path)
	, m_ResetTrainPosButton(buttons_image_path)
	, m_RouteButtons
	{ 
		&m_1_RouteButton, 
		&m_2_RouteButton, 
		&m_4_RouteButton 
	}
{
	m_texture.setSmooth(true);
	
	m_2StrButton.SetPosition({ 2030, 520 });
	m_2StrButton.SetFalseStateSpriteRect({ 6,67, 54,50 });
	m_2StrButton.SetTrueStateSpriteRect({ 6,131, 54,50 });

	m_4StrButton.SetPosition({ 2030, 590 });
	m_4StrButton.SetFalseStateSpriteRect({ 6,67, 54,50 });
	m_4StrButton.SetTrueStateSpriteRect({ 6,131, 54,50 });
	
	m_1_RouteButton.SetPosition({ 2030, 730 });
	m_1_RouteButton.SetFalseStateSpriteRect({ 6,3,  54,50 });
	m_1_RouteButton.SetTrueStateSpriteRect({ 198,3,  54,50 });
	m_1_RouteButton.SetId(At_1_line);

	m_2_RouteButton.SetPosition({ 2030, 800 });
	m_2_RouteButton.SetFalseStateSpriteRect({ 6,3, 54,50 });
	m_2_RouteButton.SetTrueStateSpriteRect({ 198,3, 54,50 });
	m_2_RouteButton.SetId(At_2_line);

	m_4_RouteButton.SetPosition({ 2030, 870 });
	m_4_RouteButton.SetFalseStateSpriteRect({ 6,3, 54,50 });
	m_4_RouteButton.SetTrueStateSpriteRect({ 198,3, 54,50 });
	m_4_RouteButton.SetId(At_4_line);

	m_RouteUnlockButton.SetPosition({ 2030, 940 });
	m_RouteUnlockButton.SetInactiveImageRectSprite({ 6,3,54,50 });
	m_RouteUnlockButton.SetActiveImageRectSprite({ 262,3,54,50 });

	m_ResetTrainPosButton.SetPosition({2030, 1010 });
	m_ResetTrainPosButton.SetInactiveImageRectSprite({ 6,3,54,50 });
	m_ResetTrainPosButton.SetActiveImageRectSprite({ 262,3,54,50 });

	m_Lights.SwitchState(TracksideLights::ONE_YELLOW_FLASHING);

}




std::pair<StationSegments_e, StationSegments_e> Station::GetCurrentTrainLocation()
{
	std::pair<StationSegments_e, StationSegments_e> result {};

	auto headPos = m_Train.GetHeadPos();
	auto tailPos = m_Train.GetTailPos();

	for (auto [station_segment, coords_pair] : m_StationSegments)
	{
		auto [left, right] = coords_pair;

		if ((headPos.x >= left.x && headPos.x <= right.x) && (headPos.y >= left.y && headPos.y <= right.y))
			result.first = station_segment;
		
		if ((tailPos.x >= left.x && tailPos.x <= right.x) && (tailPos.y >= left.y && tailPos.y <= right.y))
			result.second = station_segment;
	}

	return result;
}



bool Station::VerifySafetyConditions_ForRequestedRoute(RouteName_e RequestedRoute)
{
	switch (RequestedRoute)
	{
	case At_1_line:

		if( r_ChDP.GetCoil()->isActive() &&
			r_24SP.GetCoil()->isActive() &&
			r_2MK.GetCoil()->isActive() &&
			r_1P.GetCoil()->isActive() )
		{
			return true;
		}
		
		break;

	case At_2_line:

		if( r_ChDP.GetCoil()->isActive() &&
			r_24SP.GetCoil()->isActive() &&
			r_2PK.GetCoil()->isActive() &&
			r_4PK.GetCoil()->isActive() &&
			r_2P.GetCoil()->isActive() )
		{
			return true;
		}

		break;

	case At_4_line:

		if( r_ChDP.GetCoil()->isActive() &&
			r_24SP.GetCoil()->isActive() &&
			r_2PK.GetCoil()->isActive() &&
			r_4MK.GetCoil()->isActive() &&
			r_4P.GetCoil()->isActive() )
		{
			return true;
		}

		break;

	default:
		break;
	}

	return false;
}


bool Station::VerifyIfRouteCanBeUnlocked()
{
	auto [head_segment, tail_segment] = GetCurrentTrainLocation();


	return false;
}



void Station::EarlyUpdate()
{
	for (auto* button : m_RouteButtons)
	{
		if (*button)
		{
			m_RequestedRoute = static_cast<RouteName_e>(button->GetId());

			if (VerifySafetyConditions_ForRequestedRoute(m_RequestedRoute) && !m_IsRouteLocked)
			{
				m_IsRouteLocked = true;
				m_CurrentRoute = m_RequestedRoute;
				std::ranges::for_each(m_RouteButtons, [](TwoStatesButton* btn) { btn->lock(); });
			}
			else if (!m_IsRouteLocked)
			{
				button->SetState(false);
			}
		}
	}

	auto ActiveRouteButton_It = std::ranges::find_if(m_RouteButtons, [](TwoStatesButton* ptr) -> bool { return ptr->getState(); });
	bool isAnyActive = ActiveRouteButton_It != m_RouteButtons.end();

	if (m_RouteUnlockButton)
	{
		if (isAnyActive)
		{
			UnlockTheCurrentRoute();
		}
	}

	if (m_ResetTrainPosButton)
	{
		m_Train.ResetPosition();
	}

}



void Station::LateUpdate()
{

#if 0
	
	auto [head_pos, tail_pos] = TrainLocation_OnThisFrame;

	if ( (tail_pos == s_1p ) || ( tail_pos == s_2p ) || (tail_pos == s_4p) )
	{
		TrainLimit_on_X_axis_LeftSide = 1449 + m_Train.GetSprite().getTexture()->getSize().x;
	}
	else
	{
		TrainLimit_on_X_axis_LeftSide = DefaultTrain_X_AxisLimit_LeftSide;
	}

#endif

	if (m_IsRouteLocked)
	{
		switch (m_CurrentRoute)
		{
		case At_1_line:
		case At_4_line:
			r_ChBS.GetCoil()->SetState(true);
			break;
		case At_2_line:
			r_ChGS.GetCoil()->SetState(true);
			break;
		default:
			break;
		}
	}


	if (m_IsRouteLocked)
	{
		m_2StrButton.lock();
		m_4StrButton.lock();
	}
	else
	{
		m_2StrButton.unlock();
		m_4StrButton.unlock();
	}

	if (m_2StrButton) {}
	if (m_4StrButton) {}

	r_2PK.GetCoil()->SetState(!m_2StrButton.getState());
	r_2MK.GetCoil()->SetState(m_2StrButton.getState());
	r_4PK.GetCoil()->SetState(!m_4StrButton.getState());
	r_4MK.GetCoil()->SetState(m_4StrButton.getState());

	auto TrainLocation_OnThisFrame = GetCurrentTrainLocation();
	auto [head_location, tail_location] = TrainLocation_OnThisFrame;

	if (!m_IsRouteLocked)
	{
		if (r_2MK.GetCoil()->isActive())
			m_RequestedRoute = At_1_line;

		if (r_2PK.GetCoil()->isActive() && r_4PK.GetCoil()->isActive())
			m_RequestedRoute = At_2_line;

		if (r_2PK.GetCoil()->isActive() && r_4MK.GetCoil()->isActive())
			m_RequestedRoute = At_4_line;

		m_CurrentRoute = m_RequestedRoute;
	}

	if (m_Train.FollowTheMouse(&m_Routes[m_CurrentRoute]))
	{
		if (head_location == tail_location)											//when head and tail will be on the same station segment - we set left limit on this segment,
			TrainLimit_on_X_axis_LeftSide = m_StationSegments.at(head_location).first.x + m_Train.GetTexture().getSize().x;		// update happens when train is dragged
	}

	if (_win::GetAsyncKeyState(VK_INSERT))
	{
		m_Lights.SwitchState((TracksideLights::LightsState_e)(rand() % 6));
	}

#define SwitchRelay_If_TrainIsOnTheSegment(location, segment, relay) \
	if (location.first == segment || location.second == segment) \
		relay.GetCoil()->SetState(false); \
	else \
		relay.GetCoil()->SetState(true); \

	SwitchRelay_If_TrainIsOnTheSegment(TrainLocation_OnThisFrame, s_Chip, r_ChIP);
	SwitchRelay_If_TrainIsOnTheSegment(TrainLocation_OnThisFrame, s_Chdp, r_ChDP);
	SwitchRelay_If_TrainIsOnTheSegment(TrainLocation_OnThisFrame, s_2_4_SP, r_24SP);
	SwitchRelay_If_TrainIsOnTheSegment(TrainLocation_OnThisFrame, s_1p, r_1P);
	SwitchRelay_If_TrainIsOnTheSegment(TrainLocation_OnThisFrame, s_2p, r_2P);
	SwitchRelay_If_TrainIsOnTheSegment(TrainLocation_OnThisFrame, s_4p, r_4P);

	if (m_IsRouteLocked)
	{
		if ((!r_1P.GetCoil()->isActive() || !r_2P.GetCoil()->isActive() || !r_4P.GetCoil()->isActive()) && r_24SP.GetCoil()->isActive())
		{
			UnlockTheCurrentRoute();
		}
	}
}



void Station::UnlockTheCurrentRoute()
{
	m_IsRouteLocked = false;
	std::ranges::for_each(m_RouteButtons, [](TwoStatesButton* button) { button->unlock(); button->SetState(false);  });

	r_ChBS.GetCoil()->SetState(false);
	r_ChGS.GetCoil()->SetState(false);
}

Train& Station::GetTrain() { return m_Train; }


void Station::Draw()
{
	RenderRequests::getWindow()->draw(m_sprite);
	m_Train.Draw();
	m_Lights.Draw();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Scheme Segments
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




SchemeOverlay::SchemeOverlay() : WidgetsBase(overlay_path)
{
	m_texture.setSmooth(true);
	m_sprite.setPosition({ 0, 0 });
}


void SchemeOverlay::DrawOverlay()
{
	static sf::Text st_text6(L"Стр. 2", g_SFMLRenderer.get_font(), 35);
	static sf::Text st_text7(L"Стр. 4", g_SFMLRenderer.get_font(), 35);
	static sf::Text st_text8(L"Отмена Маршр.", g_SFMLRenderer.get_font(), 35);
	static sf::Text st_text9(L"ResetTrainPos", g_SFMLRenderer.get_font(), 35);

	static sf::Text st_text_1route(L"Прием На 1п", g_SFMLRenderer.get_font(), 35);
	static sf::Text st_text_2route(L"Прием На 2п", g_SFMLRenderer.get_font(), 35);
	static sf::Text st_text_4route(L"Прием На 4п", g_SFMLRenderer.get_font(), 35);

	Text_SetColAndPos(st_text6, { 2100, 520 });
	Text_SetColAndPos(st_text7, { 2100, 590 });
	Text_SetColAndPos(st_text_1route, { 2100, 730 });
	Text_SetColAndPos(st_text_2route, { 2100, 800 });
	Text_SetColAndPos(st_text_4route, { 2100, 870 });
	Text_SetColAndPos(st_text8, { 2100, 940 });
	Text_SetColAndPos(st_text9, { 2100, 1010 });


	RenderRequests::DrawInvoke([this]
		{
			RenderRequests::getWindow()->draw(m_sprite);
			RenderRequests::getWindow()->draw(st_text6);
			RenderRequests::getWindow()->draw(st_text7);
			RenderRequests::getWindow()->draw(st_text8);
			RenderRequests::getWindow()->draw(st_text9);
			RenderRequests::getWindow()->draw(st_text_1route);
			RenderRequests::getWindow()->draw(st_text_2route);
			RenderRequests::getWindow()->draw(st_text_4route);
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
		make_group(s2_c_CHGS1, {419,1471}, &r_ChGS),
		make_group(s2_c_CHBS1, {610,1529}, &r_ChBS),
		make_group(s2_c_CH2M,  {950,1597}, &r_Ch2M),
		make_group(s2_c_CHPZ,  {950,1767}, &r_ChPZ),
		
		make_group(s1_c_VV,		{955,425},	nullptr),
		make_group(s1_c_CHRI,	{1190,425}, nullptr),
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
	// 
	//	// it's a bit easier than writing a bunch of code like this 
	//		//"m_PathSegmentsMap.at("s2_3_1")->GetContact_2()->PushContactAsDestination(m_PathSegmentsMap.at("s2_3_2")->GetContact_1()))" 
	//
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									file parser 
//

	std::ifstream input(very_important_data_file_path);
	SM_ASSERT(input.is_open(), "SchemeSegments::SchemeSegments() -> Unable to open important data file");
	
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
		
		auto view =
			helpers::split_string(line, "@")
			| std::views::take(3)
			| std::views::transform(helpers::strip_string);
		
		std::ranges::copy(view, elements_array.begin());
		auto& [Path1_Contact2, WayThrough, Path2_Contact1] = elements_array;

		SM_ASSERT(Path1_Contact2.size() != 0, "SchemeSegments::SchemeSegments() -> Path1_Contact2 string len == 0");
		SM_ASSERT(Path2_Contact1.size() != 0, "SchemeSegments::SchemeSegments() -> Path2_Contact1 string len == 0");

		auto Path1_Contact_elems = helpers::split_string(Path1_Contact2, "* ");
		auto Path2_Contact_elems = helpers::split_string(Path2_Contact1, "* ");
		SM_ASSERT(Path1_Contact_elems.size() == 3 && Path2_Contact_elems.size() == 3, std::format("SchemeSegments::SchemeSegments()) {}", Path1_Contact2));
		
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

	r_ChPZ.GetCoil()->setLeftContactPos({ 1310, 1575 });
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
	for (auto* path : m_PathSegments)
		path->ResetSegment();
}


void SchemeSegments::ResetConactGroups()
{
	for (auto [name, group] : m_ContactGroupsMap)
		group->Reset();
}


void SchemeSegments::DrawSegments()
{
	RenderRequests::DrawInvoke([this]
		{
			if (m_PathSegments.empty())
				return;

			for (auto path : m_PathSegments)
			{
				path->Draw();
			}

			for (auto [name, group] : m_ContactGroupsMap)
			{
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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									Scheme
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Scheme::Scheme()
{ }


__forceinline void Scheme::FirstInit()
{
	static bool init = false;
	if (!init) [[unlikely]]
	{
		r_Ch2M.GetCoil()->SetState(true);
		r_Ch1M.GetCoil()->SetState(true);
		init = true;
	}
}


void Scheme::DrawScheme()
{
	m_SchemeSegments.GetStation().EarlyUpdate();
	ResetCoils();
	m_SchemeSegments.ResetPathSegments();
	m_SchemeSegments.ResetConactGroups();

	FirstInit();

	m_SchemeSegments.GetStation().LateUpdate();
	
	m_SchemeSegments.SendSignalFromEntry();
	m_SchemeSegments.DrawSegments();
	m_Overlay.DrawOverlay();

#if __debug
	std::cout < "\n\n\n\n";
#endif
}
