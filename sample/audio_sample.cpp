#include "webrtc/modules/audio_device/include/audio_device.h"
#include "webrtc/pc/localaudiosource.h"
#include "webrtc/pc/audiotrack.h"

#include "webrtc/api/audio_codecs/audio_encoder_factory.h"
#include "webrtc/api/audio_codecs/builtin_audio_encoder_factory.h"
#include "webrtc/api/audio_codecs/builtin_audio_decoder_factory.h"

#include "webrtc/modules/audio_coding/codecs/builtin_audio_encoder_factory_internal.h"
#include "webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory_internal.h"

#include "webrtc/voice_engine/voice_engine_impl.h"

using namespace std;
using namespace webrtc;

int main()
{
    // string audio_track_id = "audio_label";

    // rtc::scoped_refptr<AudioEncoderFactory> audio_encoder_factory = CreateBuiltinAudioEncoderFactory();

    // rtc::scoped_refptr<webrtc::AudioDecoderFactory> audio_decoder_factory = CreateBuiltinAudioDecoderFactory();

    // rtc::scoped_refptr<AudioSourceInterface> audio_source = LocalAudioSource::Create((const cricket::AudioOptions*)NULL);

    // rtc::scoped_refptr<AudioTrackInterface> audio_track(AudioTrack::Create(audio_track_id, audio_source));

    // VoiceEngine* voice_engine = VoiceEngine::Create();

    VoiceEngine* voe = VoiceEngine::Create();//new VoiceEngineImpl();

    VoEBase* base = VoEBase::GetInterface(voe);

    VoEFile* file  = VoEFile::GetInterface(voe);

    base->Init();

    // int channel = base->CreateChannel();

    // base->StartPlayout(channel);

    // file->StartRecordingMicrophone("data_file_16kHz.pcm");
    // file->Start

    // base->StartReceive(channel);
    // voice_engine->StartSend(channel);

    file->StartRecordingMicrophone("data_file_16kHz.pcm");

    sleep(5);

    file->StopRecordingMicrophone();

    // base->StopSend(channel);
    // base->StopReceive(channel);
    // base->StopPlayout(channel);

    base->Terminate();

    base->Release();

    file->Release();

    VoiceEngine::Delete(voe);

    return 0;
}