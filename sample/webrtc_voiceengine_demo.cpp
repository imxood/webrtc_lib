/*
*  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <vector>

// #include "webrtc/engine_configurations.h"
#include "webrtc/modules/audio_processing/include/audio_processing.h"
// #include "webrtc/test/channel_transport/udp_transport.h"
// #include "webrtc/voice_engine/include/voe_audio_processing.h"
#include "webrtc/voice_engine/include/voe_base.h"
#include "webrtc/voice_engine/include/voe_codec.h"
// #include "webrtc/voice_engine/include/voe_dtmf.h"
#include "webrtc/voice_engine/include/voe_errors.h"
// #include "webrtc/voice_engine/include/voe_external_media.h"
#include "webrtc/voice_engine/include/voe_file.h"
// #include "webrtc/voice_engine/include/voe_hardware.h"
// #include "webrtc/voice_engine/include/voe_neteq_stats.h"
#include "webrtc/voice_engine/include/voe_network.h"
#include "webrtc/voice_engine/include/voe_rtp_rtcp.h"
// #include "webrtc/voice_engine/include/voe_video_sync.h"
// #include "webrtc/voice_engine/include/voe_volume_control.h"

#include "webrtc/p2p/base/udptransport.h"

using namespace webrtc;

// Helper class for VoiceEngine tests.
class VoiceChannelTransport : public test::UdpTransportData {
public:
	VoiceChannelTransport(VoENetwork* voe_network, int channel);

	virtual ~VoiceChannelTransport();

	// Start implementation of UdpTransportData.
	void IncomingRTPPacket(const int8_t* incoming_rtp_packet,
		const size_t packet_length,
		const char* /*from_ip*/,
		const uint16_t /*from_port*/) override;

	void IncomingRTCPPacket(const int8_t* incoming_rtcp_packet,
		const size_t packet_length,
		const char* /*from_ip*/,
		const uint16_t /*from_port*/) override;
	// End implementation of UdpTransportData.

	// Specifies the ports to receive RTP packets on.
	int SetLocalReceiver(uint16_t rtp_port);

	// Specifies the destination port and IP address for a specified channel.
	int SetSendDestination(const char* ip_address, uint16_t rtp_port);

private:
	int channel_;
	VoENetwork* voe_network_;
	test::UdpTransport* socket_transport_;
};


VoiceChannelTransport::VoiceChannelTransport(VoENetwork* voe_network,
	int channel)
	: channel_(channel),
	voe_network_(voe_network) {
	uint8_t socket_threads = 1;
	socket_transport_ = test::UdpTransport::Create(channel, socket_threads);
	int registered = voe_network_->RegisterExternalTransport(channel,
		*socket_transport_);
#if !defined(WEBRTC_ANDROID) && !defined(WEBRTC_IOS)
	if (registered != 0)
		return;
#else
	assert(registered == 0);
#endif
}

VoiceChannelTransport::~VoiceChannelTransport() {
	voe_network_->DeRegisterExternalTransport(channel_);
	test::UdpTransport::Destroy(socket_transport_);
}

void VoiceChannelTransport::IncomingRTPPacket(
	const int8_t* incoming_rtp_packet,
	const size_t packet_length,
	const char* /*from_ip*/,
	const uint16_t /*from_port*/) {
	voe_network_->ReceivedRTPPacket(
		channel_, incoming_rtp_packet, packet_length, PacketTime());
}

void VoiceChannelTransport::IncomingRTCPPacket(
	const int8_t* incoming_rtcp_packet,
	const size_t packet_length,
	const char* /*from_ip*/,
	const uint16_t /*from_port*/) {
	voe_network_->ReceivedRTCPPacket(channel_, incoming_rtcp_packet,
		packet_length);
}

int VoiceChannelTransport::SetLocalReceiver(uint16_t rtp_port) {
	static const int kNumReceiveSocketBuffers = 500;
	int return_value = socket_transport_->InitializeReceiveSockets(this,
		rtp_port);
	if (return_value == 0) {
		return socket_transport_->StartReceiving(kNumReceiveSocketBuffers);
	}
	return return_value;
}

int VoiceChannelTransport::SetSendDestination(const char* ip_address,
	uint16_t rtp_port) {
	return socket_transport_->InitializeSendSockets(ip_address, rtp_port);
}

class MyObserver : public VoiceEngineObserver {
public:
	virtual void CallbackOnError(int channel, int err_code);
};

void MyObserver::CallbackOnError(int channel, int err_code) {
	// Add printf for other error codes here
	if (err_code == VE_TYPING_NOISE_WARNING) {
		printf("  TYPING NOISE DETECTED \n");
	}
	else if (err_code == VE_TYPING_NOISE_OFF_WARNING) {
		printf("  TYPING NOISE OFF DETECTED \n");
	}
	else if (err_code == VE_RECEIVE_PACKET_TIMEOUT) {
		printf("  RECEIVE PACKET TIMEOUT \n");
	}
	else if (err_code == VE_PACKET_RECEIPT_RESTARTED) {
		printf("  PACKET RECEIPT RESTARTED \n");
	}
	else if (err_code == VE_RUNTIME_PLAY_WARNING) {
		printf("  RUNTIME PLAY WARNING \n");
	}
	else if (err_code == VE_RUNTIME_REC_WARNING) {
		printf("  RUNTIME RECORD WARNING \n");
	}
	else if (err_code == VE_SATURATION_WARNING) {
		printf("  SATURATION WARNING \n");
	}
	else if (err_code == VE_RUNTIME_PLAY_ERROR) {
		printf("  RUNTIME PLAY ERROR \n");
	}
	else if (err_code == VE_RUNTIME_REC_ERROR) {
		printf("  RUNTIME RECORD ERROR \n");
	}
	else if (err_code == VE_REC_DEVICE_REMOVED) {
		printf("  RECORD DEVICE REMOVED \n");
	}
}

int VoiceEngineSample()
{
	int error = 0;

	//
	// Create VoiceEngine related instance
	//
	VoiceEngine* ptrVoE = VoiceEngine::Create();

	VoEBase* ptrVoEBase = VoEBase::GetInterface(ptrVoE);

	VoECodec* ptrVoECodec = VoECodec::GetInterface(ptrVoE);

	VoENetwork* ptrVoENetwork = VoENetwork::GetInterface(ptrVoE);

	VoEFile* ptrVoEFile = VoEFile::GetInterface(ptrVoE);

	// VoEAudioProcessing* ptrVoEAp = VoEAudioProcessing::GetInterface(ptrVoE);

	// VoEVolumeControl* ptrVoEVolume = VoEVolumeControl::GetInterface(ptrVoE);

	// VoEHardware* ptrVoEHardware = VoEHardware::GetInterface(ptrVoE);

	MyObserver my_observer;

	//
	//Set Trace File and Record File
	//
	const std::string trace_filename = "webrtc_trace.txt";
	VoiceEngine::SetTraceFilter(kTraceAll);
	error = VoiceEngine::SetTraceFile(trace_filename.c_str());
	if (error != 0)
	{
		printf("ERROR in VoiceEngine::SetTraceFile\n");
		return error;
	}
	error = VoiceEngine::SetTraceCallback(NULL);
	if (error != 0)
	{
		printf("ERROR in VoiceEngine::SetTraceCallback\n");
		return error;
	}
	const std::string play_filename = "recorded_playout.wav";
	const std::string mic_filename = "recorded_mic.wav";

	//
	//Init
	//
	error = ptrVoEBase->Init();
	if (error != 0)
	{
		printf("ERROR in VoEBase::Init\n");
		return error;
	}
	error = ptrVoEBase->RegisterVoiceEngineObserver(my_observer);
	if (error != 0)
	{
		printf("ERROR in VoEBase:;RegisterVoiceEngineObserver\n");
		return error;
	}
	printf("Version\n");
	char version[1024];
	error = ptrVoEBase->GetVersion(version);
	if (error != 0)
	{
		printf("ERROR in VoEBase::GetVersion\n");
		return error;
	}
	printf("%s\n", version);

	//
	//Network Settings
	//
	int audiochannel;
	audiochannel = ptrVoEBase->CreateChannel();
	if (audiochannel < 0)
	{
		printf("ERROR in VoEBase::CreateChannel\n");
		return audiochannel;
	}
	VoiceChannelTransport* voice_channel_transport = new VoiceChannelTransport(ptrVoENetwork, audiochannel);
	char ip[64] = "127.0.0.1";
	int rPort = 800;//remote port
	int lPort = 800;//local port
	error = voice_channel_transport->SetSendDestination(ip, rPort);
	if (error != 0)
	{
		printf("ERROR in set send ip and port\n");
		return error;
	}
	error = voice_channel_transport->SetLocalReceiver(lPort);
	if (error != 0)
	{
		printf("ERROR in set receiver and port\n");
		return error;
	}

	//
	//Setup Codecs
	//
	CodecInst codec_params;
	CodecInst cinst;
	for (int i = 0; i < ptrVoECodec->NumOfCodecs(); ++i) {
		int error = ptrVoECodec->GetCodec(i, codec_params);
		if (error != 0)
		{
			printf("ERROR in VoECodec::GetCodec\n");
			return error;
		}
		printf("%2d. %3d  %s/%d/%d \n", i, codec_params.pltype, codec_params.plname,
			codec_params.plfreq, codec_params.channels);
	}
	printf("Select send codec: ");
	int codec_selection;
	scanf("%i", &codec_selection);
	ptrVoECodec->GetCodec(codec_selection, cinst);
	error = ptrVoECodec->SetSendCodec(audiochannel, cinst);
	if (error != 0)
	{
		printf("ERROR in VoECodec::SetSendCodec\n");
		return error;
	}

	//
	//Setup Devices
	//
	int rd(-1), pd(-1);
	error = ptrVoEHardware->GetNumOfRecordingDevices(rd);
	if (error != 0)
	{
		printf("ERROR in VoEHardware::GetNumOfRecordingDevices\n");
		return error;
	}
	error = ptrVoEHardware->GetNumOfPlayoutDevices(pd);
	if (error != 0)
	{
		printf("ERROR in VoEHardware::GetNumOfPlayoutDevices\n");
		return error;
	}

	char dn[128] = { 0 };
	char guid[128] = { 0 };
	printf("\nPlayout devices (%d): \n", pd);
	for (int j = 0; j < pd; ++j) {
		error = ptrVoEHardware->GetPlayoutDeviceName(j, dn, guid);
		if (error != 0)
		{
			printf("ERROR in VoEHardware::GetPlayoutDeviceName\n");
			return error;
		}
		printf("  %d: %s \n", j, dn);
	}

	printf("Recording devices (%d): \n", rd);
	for (int j = 0; j < rd; ++j) {
		error = ptrVoEHardware->GetRecordingDeviceName(j, dn, guid);
		if (error != 0)
		{
			printf("ERROR in VoEHardware::GetRecordingDeviceName\n");
			return error;
		}
		printf("  %d: %s \n", j, dn);
	}

	printf("Select playout device: ");
	scanf("%d", &pd);
	error = ptrVoEHardware->SetPlayoutDevice(pd);
	if (error != 0)
	{
		printf("ERROR in VoEHardware::SetPlayoutDevice\n");
		return error;
	}
	printf("Select recording device: ");
	scanf("%d", &rd);
	getchar();
	error = ptrVoEHardware->SetRecordingDevice(rd);
	if (error != 0)
	{
		printf("ERROR in VoEHardware::SetRecordingDevice\n");
		return error;
	}

	//
	//Audio Processing
	//
	error = ptrVoECodec->SetVADStatus(audiochannel, 1);
	if (error != 0)
	{
		printf("ERROR in VoECodec::SetVADStatus\n");
		return error;
	}
	error = ptrVoEAp->SetAgcStatus(1);
	if (error != 0)
	{
		printf("ERROR in VoEAudioProcess::SetAgcStatus\n");
		return error;
	}
	error = ptrVoEAp->SetEcStatus(1);
	if (error != 0)
	{
		printf("ERROR in VoEAudioProcess::SetEcStatus\n");
		return error;
	}
	error = ptrVoEAp->SetNsStatus(1);
	if (error != 0)
	{
		printf("ERROR in VoEAudioProcess::SetNsStatus\n");
		return error;
	}
	error = ptrVoEAp->SetRxAgcStatus(audiochannel, 1);
	if (error != 0)
	{
		printf("ERROR in VoEAudioProcess::SetRxAgcStatus\n");
		return error;
	}
	error = ptrVoEAp->SetRxNsStatus(audiochannel, 1);
	if (error != 0)
	{
		printf("ERROR in VoEAudioProcess::SetRxNsStatus\n");
		return error;
	}

	//Start Receive
	error = ptrVoEBase->StartReceive(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEBase::StartReceive\n");
		return error;
	}
	//Start Playout
	error = ptrVoEBase->StartPlayout(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEBase::StartPlayout\n");
		return error;
	}
	//Start Send
	error = ptrVoEBase->StartSend(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEBase::StartSend\n");
		return error;
	}
	//Start Record
	error = ptrVoEFile->StartRecordingMicrophone(mic_filename.c_str());
	if (error != 0)
	{
		printf("ERROR in VoEFile::StartRecordingMicrophone\n");
		return error;
	}
	error = ptrVoEFile->StartRecordingPlayout(audiochannel, play_filename.c_str());
	if (error != 0)
	{
		printf("ERROR in VoEFile::StartRecordingPlayout\n");
		return error;
	}

	unsigned int vol = 999;
	error = ptrVoEVolume->GetMicVolume(vol);
	if (error != 0)
	{
		printf("ERROR in VoEVolume::GetMicVolume\n");
		return error;
	}
	if ((vol > 255) || (vol < 1)) {
		printf("\n****ERROR in GetMicVolume");
	}
	printf("mic volume: %d \n", vol);


	printf("\n call started\n\n");
	printf("Press enter to stop...");
	while ((getchar()) != '\n')
		;

	//Stop Record
	error = ptrVoEFile->StopRecordingMicrophone();
	if (error != 0)
	{
		printf("ERROR in VoEFile::StopRecordingMicrophone\n");
		return error;
	}
	error = ptrVoEFile->StopRecordingPlayout(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEFile::StopRecordingPlayout\n");
		return error;
	}
	//Stop Receive
	error = ptrVoEBase->StopReceive(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEBase::StopReceive\n");
		return error;
	}
	//Stop Send
	error = ptrVoEBase->StopSend(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEBase::StopSend\n");
		return error;
	}
	//Stop Playout
	error = ptrVoEBase->StopPlayout(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEBase::StopPlayout\n");
		return error;
	}
	//Delete Channel
	error = ptrVoEBase->DeleteChannel(audiochannel);
	if (error != 0)
	{
		printf("ERROR in VoEBase::DeleteChannel\n");
		return error;
	}
	

	delete voice_channel_transport;

	ptrVoEBase->DeRegisterVoiceEngineObserver();
	error = ptrVoEBase->Terminate();
	if (error != 0)
	{
		printf("ERROR in VoEBase::Terminate\n");
		return error;
	}

	int remainingInterfaces = 0;
	remainingInterfaces += ptrVoEBase->Release();
	remainingInterfaces = ptrVoECodec->Release();
	remainingInterfaces += ptrVoEVolume->Release();
	remainingInterfaces += ptrVoEFile->Release();
	remainingInterfaces += ptrVoEAp->Release();
	remainingInterfaces += ptrVoEHardware->Release();
	remainingInterfaces += ptrVoENetwork->Release();


	/*if (remainingInterfaces > 0)
	{
	printf("ERROR: Could not release all interfaces\n");
	return -1;
	}*/

	bool deleted = VoiceEngine::Delete(ptrVoE);
	if (deleted == false)
	{
		printf("ERROR in VoiceEngine::Delete\n");
		return -1;
	}

	return 0;

}

int main(int argc, char** argv) {
	VoiceEngineSample();
}
