/*
 * GlobalLobbyProcessor.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "GlobalLobbyProcessor.h"

#include "CVCMIServer.h"
#include "../lib/CConfigHandler.h"

GlobalLobbyProcessor::GlobalLobbyProcessor(CVCMIServer & owner)
	: owner(owner)
	, networkClient(owner.networkHandler->createClientTCP(*this))
{
	std::string hostname = settings["lobby"]["hostname"].String();
	int16_t port = settings["lobby"]["port"].Integer();
	networkClient->start(hostname, port);
	logGlobal->info("Connecting to lobby server");
}

void GlobalLobbyProcessor::onDisconnected(const std::shared_ptr<INetworkConnection> & connection)
{
	throw std::runtime_error("Lost connection to a lobby server!");
}

void GlobalLobbyProcessor::onPacketReceived(const std::shared_ptr<INetworkConnection> & connection, const std::vector<uint8_t> & message)
{
	JsonNode json(message.data(), message.size());

	if(json["type"].String() == "loginFailed")
		return receiveLoginFailed(json);

	if(json["type"].String() == "loginSuccess")
		return receiveLoginSuccess(json);

	throw std::runtime_error("Received unexpected message from lobby server: " + json["type"].String());
}

void GlobalLobbyProcessor::receiveLoginFailed(const JsonNode & json)
{
	throw std::runtime_error("Failed to login into a lobby server!");
}

void GlobalLobbyProcessor::receiveLoginSuccess(const JsonNode & json)
{
	// no-op, wait just for any new commands from lobby
	logGlobal->info("Succesfully connected to lobby server");
}

void GlobalLobbyProcessor::onConnectionFailed(const std::string & errorMessage)
{
	throw std::runtime_error("Failed to connect to a lobby server!");
}

void GlobalLobbyProcessor::onConnectionEstablished(const std::shared_ptr<INetworkConnection> &)
{
	logGlobal->info("Connection to lobby server established");

	JsonNode toSend;
	toSend["type"].String() = "serverLogin";
	toSend["accountID"] = settings["lobby"]["accountID"];
	toSend["accountCookie"] = settings["lobby"]["accountCookie"];
	sendMessage(toSend);
}

void GlobalLobbyProcessor::sendMessage(const JsonNode & data)
{
	std::string payloadString = data.toJson(true);

	// FIXME: find better approach
	uint8_t * payloadBegin = reinterpret_cast<uint8_t *>(payloadString.data());
	uint8_t * payloadEnd = payloadBegin + payloadString.size();

	std::vector<uint8_t> payloadBuffer(payloadBegin, payloadEnd);

	networkClient->sendPacket(payloadBuffer);
}
