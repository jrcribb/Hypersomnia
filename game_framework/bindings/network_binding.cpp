#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "network/network_interface.h"
#include "misc/map_wrapper.h"
#include "misc/vector_wrapper.h"

#include "network/net_channel.h"

struct connection_attempt_result {};
struct send_priority {};
struct send_reliability {};
struct network_message {};

struct receive_result {};

void Write(RakNet::BitStream& bs, const std::string& str) {
	bs.Write(str.c_str());
}

void WriteGuid(RakNet::BitStream& bs, RakNet::RakNetGUID& guid) {
	bs.Write(guid.g);
}

RakNet::RakNetGUID ReadGuid(RakNet::BitStream& bs) {
	RakNet::RakNetGUID guid;

	decltype(guid.g) my_id;
	bs.Read(my_id);

	return RakNet::RakNetGUID(my_id);
}

template <class T>
void WritePOD(RakNet::BitStream& bs, T data) {
	bs.Write<T>(data);
}

void WriteBitstream(RakNet::BitStream& in, RakNet::BitStream& out) {
	in.WriteBits(out.GetData(), out.GetNumberOfBitsUsed(), false);
}

template <class T>
T ReadPOD(RakNet::BitStream& bs) {
	T data;
	bs.Read<T>(data);
	return data;
}

namespace bindings {
	luabind::scope _network_binding() {
		using namespace network;

		return
			(

			luabind::class_<network_message>("network_message")
			.enum_("network_message")[
				luabind::value("ID_CONNECTION_REQUEST_ACCEPTED", ID_CONNECTION_REQUEST_ACCEPTED),
				luabind::value("ID_CONNECTION_ATTEMPT_FAILED", ID_CONNECTION_ATTEMPT_FAILED),
				luabind::value("ID_ALREADY_CONNECTED", ID_ALREADY_CONNECTED),
				luabind::value("ID_NEW_INCOMING_CONNECTION", ID_NEW_INCOMING_CONNECTION),
				luabind::value("ID_NO_FREE_INCOMING_CONNECTIONS", ID_NO_FREE_INCOMING_CONNECTIONS),
				luabind::value("ID_DISCONNECTION_NOTIFICATION", ID_DISCONNECTION_NOTIFICATION),
				luabind::value("ID_CONNECTION_LOST", ID_CONNECTION_LOST),
				luabind::value("ID_CONNECTION_BANNED", ID_CONNECTION_BANNED),
				luabind::value("ID_INVALID_PASSWORD", ID_INVALID_PASSWORD),
				luabind::value("ID_INCOMPATIBLE_PROTOCOL_VERSION", ID_INCOMPATIBLE_PROTOCOL_VERSION),
				luabind::value("ID_IP_RECENTLY_CONNECTED", ID_IP_RECENTLY_CONNECTED),
				luabind::value("ID_TIMESTAMP", ID_TIMESTAMP),
				luabind::value("ID_UNCONNECTED_PONG", ID_UNCONNECTED_PONG),
				luabind::value("ID_ADVERTISE_SYSTEM", ID_ADVERTISE_SYSTEM),
				luabind::value("ID_DOWNLOAD_PROGRESS", ID_DOWNLOAD_PROGRESS),
				luabind::value("ID_USER_PACKET_ENUM", ID_USER_PACKET_ENUM)
			],

			luabind::class_<send_priority>("send_priority")
			.enum_("send_priority")[
				luabind::value("IMMEDIATE_PRIORITY", PacketPriority::IMMEDIATE_PRIORITY),
					luabind::value("HIGH_PRIORITY", PacketPriority::HIGH_PRIORITY),
				luabind::value("MEDIUM_PRIORITY", PacketPriority::MEDIUM_PRIORITY),
				luabind::value("LOW_PRIORITY", PacketPriority::LOW_PRIORITY)
			],




			luabind::class_<send_reliability>("send_reliability")
			.enum_("send_reliability")[
				luabind::value("UNRELIABLE", PacketReliability::UNRELIABLE),
				luabind::value("UNRELIABLE_SEQUENCED", PacketReliability::UNRELIABLE_SEQUENCED),
				luabind::value("RELIABLE", PacketReliability::RELIABLE),
				luabind::value("RELIABLE_ORDERED", PacketReliability::RELIABLE_ORDERED),
				luabind::value("RELIABLE_SEQUENCED", PacketReliability::RELIABLE_SEQUENCED),
				luabind::value("UNRELIABLE_WITH_ACK_RECEIPT", PacketReliability::UNRELIABLE_WITH_ACK_RECEIPT),
				luabind::value("RELIABLE_WITH_ACK_RECEIPT", PacketReliability::RELIABLE_WITH_ACK_RECEIPT),
				luabind::value("RELIABLE_ORDERED_WITH_ACK_RECEIPT", PacketReliability::RELIABLE_ORDERED_WITH_ACK_RECEIPT)
			],

			luabind::class_<connection_attempt_result>("connection_attempt_result")
			.enum_("connection_attempt_result")[
				luabind::value("CONNECTION_ATTEMPT_STARTED", RakNet::CONNECTION_ATTEMPT_STARTED),
					luabind::value("INVALID_PARAMETER", RakNet::INVALID_PARAMETER),
					luabind::value("CANNOT_RESOLVE_DOMAIN_NAME", RakNet::CANNOT_RESOLVE_DOMAIN_NAME),
					luabind::value("ALREADY_CONNECTED_TO_ENDPOINT", RakNet::ALREADY_CONNECTED_TO_ENDPOINT),
					luabind::value("CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS", RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS),
					luabind::value("SECURITY_INITIALIZATION_FAILED", RakNet::SECURITY_INITIALIZATION_FAILED)
			],

			luabind::class_<RakNet::RakString>("RakString")
			.def(luabind::constructor<>())
			.def("C_String", &RakNet::RakString::C_String),

			luabind::class_<RakNet::RakNetGUID>("RakNetGUID")
			.def(luabind::constructor<>()),

			luabind::class_<RakNet::BitStream>("BitStream")
			.def(luabind::constructor<>())
			.def("IgnoreBytes", &RakNet::BitStream::IgnoreBytes)
			.def("ReadRakString", &RakNet::BitStream::Read<RakNet::RakString>)
			.def("size", &RakNet::BitStream::GetNumberOfBitsUsed)
			.def("Reset", &RakNet::BitStream::Reset)
			,


			/* little helpers */
			luabind::def("WriteCString", Write),
			luabind::def("ReadRakNetGUID", ReadGuid),
			luabind::def("WriteRakNetGUID", WriteGuid),

			luabind::def("WriteBitstrean", WriteBitstream),
			luabind::def("WriteBit", WritePOD<bool>),
			luabind::def("WriteInt", WritePOD<int>),
			luabind::def("WriteByte", WritePOD<unsigned char>),
			luabind::def("WriteUint", WritePOD<unsigned>),
			luabind::def("WriteUshort", WritePOD<unsigned short>),
			luabind::def("WriteFloat", WritePOD<float>),
			luabind::def("WriteDouble", WritePOD<double>),
			luabind::def("Writeb2Vec2", WritePOD<b2Vec2>),
			luabind::def("WriteVec2", WritePOD<vec2<>>),

			luabind::def("ReadBit", ReadPOD<bool>),
			luabind::def("ReadInt", ReadPOD<int>),
			luabind::def("ReadByte", ReadPOD<unsigned char>),
			luabind::def("ReadUint", ReadPOD<unsigned>),
			luabind::def("ReadUshort", ReadPOD<unsigned short>),
			luabind::def("ReadFloat", ReadPOD<float>),
			luabind::def("ReadDouble", ReadPOD<double>),
			luabind::def("Readb2Vec2", ReadPOD<b2Vec2>),
			luabind::def("ReadVec2", ReadPOD<vec2<>>),

			luabind::class_<receive_result>("receive_result")
			.enum_("receive_result")[
				luabind::value("RELIABLE_RECEIVED", reliable_receiver::RELIABLE_RECEIVED),
				luabind::value("ONLY_UNRELIABLE_RECEIVED", reliable_receiver::ONLY_UNRELIABLE_RECEIVED),
				luabind::value("NOTHING_RECEIVED", reliable_receiver::NOTHING_RECEIVED)
			],

			luabind::class_<reliable_sender::message>("net_channel_message")
			.def_readwrite("flag_for_deletion", &reliable_sender::message::flag_for_deletion)
			.def_readwrite("script", &reliable_sender::message::script)
			.def_readwrite("output_bitstream", &reliable_sender::message::output_bitstream),
			
			misc::vector_wrapper<reliable_sender::message>::bind_vector("net_channel_message_vector"),

			luabind::class_<reliable_sender>("reliable_sender")
			.def(luabind::constructor<>())
			.def("post_message", &reliable_sender::post_message)
			.def_readwrite("reliable_buf", &reliable_sender::reliable_buf)
			.def_readwrite("sequence", &reliable_sender::sequence)
			.def_readwrite("ack_sequence", &reliable_sender::ack_sequence)
			,

			luabind::class_<reliable_receiver>("net_channel_sender")
			.def(luabind::constructor<>())
			.def_readwrite("last_sequence", &reliable_receiver::last_sequence)
			.def("read_sequence", &reliable_receiver::read_sequence)
			.def("write_ack", &reliable_receiver::write_ack),

			map_wrapper<RakNet::RakNetGUID, luabind::object>::bind("guid_to_object_map"),

			luabind::class_<network_interface::packet>("network_packet")
			.def(luabind::constructor<>())
			.def("byte", &network_interface::packet::byte)
			.def("length", &network_interface::packet::length)
			.def("guid", &network_interface::packet::guid)
			.def("get_bitstream", &network_interface::packet::get_bitstream)
			,

			luabind::class_<network_interface>("network_interface")
			.def(luabind::constructor<>())
			.def("listen", &network_interface::listen)
			.def("connect", &network_interface::connect)
			.def("receive", &network_interface::receive)
			.def("enable_lag", &network_interface::enable_lag)
			.def("close_connection", &network_interface::close_connection)
			.def("shutdown", &network_interface::shutdown)
			.def("send", &network_interface::send)
	

			);
	}
}