#include "NativeApi.h"
#include <deque>
#include "../Modules/Modules.hpp"
#include "../Modules/Models/EnvManager.hpp"

struct LibTTSParams
{
	float NoiseScale = 0.3f;                           //������������          0-10
	int64_t Seed = 52468;                              //����
	int64_t SpeakerId = 0;                             //��ɫID
	uint64_t SrcSamplingRate = 48000;                  //Դ������
	int64_t SpkCount = 2;                              //ģ�ͽ�ɫ��

	std::vector<float> SpeakerMix;                     //��ɫ��ϱ���
	float LengthScale = 1.0f;                          //ʱ����������
	float DurationPredictorNoiseScale = 0.8f;          //���ʱ��Ԥ����������������
	float FactorDpSdp = 0.f;                           //���ʱ��Ԥ������ʱ��Ԥ������ϱ���
	float RestTime = 0.5f;                             //ͣ��ʱ�䣬Ϊ������ֱ�ӶϿ���Ƶ����������Ƶ
	float GateThreshold = 0.66666f;                    //Tacotron2������EOS��ֵ
	int64_t MaxDecodeStep = 2000;                      //Tacotron2�����벽��
	std::vector<std::wstring> EmotionPrompt;           //��б��
	std::wstring PlaceHolderSymbol = L"|";             //���طָ���
	std::string LanguageSymbol = "JP";                 //����
	std::wstring AdditionalInfo = L"";                 //G2P������Ϣ
	std::wstring SpeakerName = L"0";                   //��ɫ��
};

struct LibTTSToken
{
	std::wstring Text;                                 //�����ı�
	std::vector<std::wstring> Phonemes;                //��������
	std::vector<int64_t> Tones;                        //��������
	std::vector<int64_t> Durations;                    //ʱ������
	std::vector<std::string> Language;                 //��������
};

struct LibTTSSeq
{
	std::wstring TextSeq;
	std::vector<LibTTSToken> SlicedTokens;
	std::vector<float> SpeakerMix;                     //��ɫ��ϱ���
	std::vector<std::wstring> EmotionPrompt;           //��б��
	std::wstring PlaceHolderSymbol = L"|";             //���طָ���
	float NoiseScale = 0.3f;                           //������������             0-10
	float LengthScale = 1.0f;                          //ʱ����������
	float DurationPredictorNoiseScale = 0.3f;          //���ʱ��Ԥ����������������
	float FactorDpSdp = 0.3f;                          //���ʱ��Ԥ������ʱ��Ԥ������ϱ���
	float GateThreshold = 0.66666f;                    //Tacotron2������EOS��ֵ
	int64_t MaxDecodeStep = 2000;                      //Tacotron2�����벽��
	int64_t Seed = 52468;                              //����
	float RestTime = 0.5f;                             //ͣ��ʱ�䣬Ϊ������ֱ�ӶϿ���Ƶ����������Ƶ
	std::string LanguageSymbol = "ZH";                 //���Ա��
	std::wstring SpeakerName = L"0";                   //��ɫ������ID
	std::wstring AdditionalInfo = L"";                       //G2P������Ϣ
};

const wchar_t* LibTTSNullString = L"";
#define LibTTSNullStrCheck(Str) ((Str)?(Str):(LibTTSNullString))
std::deque<std::wstring> ErrorQueue;
size_t MaxErrorCount = 20;
std::mutex ErrorMx;

void RaiseError(const std::wstring& _Msg)
{
	logger.log(_Msg);
	ErrorMx.lock();
	ErrorQueue.emplace_front(_Msg);
	if (ErrorQueue.size() > MaxErrorCount)
		ErrorQueue.pop_back();
}

void LibTTSInit()
{
	MoeVSModuleManager::MoeVoiceStudioCoreInitSetup();
}

INT32 LibTTSSetGlobalEnv(UINT32 ThreadCount, UINT32 DeviceID, UINT32 Provider)
{
	try
	{
		moevsenv::GetGlobalMoeVSEnv().Load(ThreadCount, DeviceID, Provider);
	}
	catch (std::exception& e)
	{
		RaiseError(to_wide_string(e.what()));
		return 1;
	}
	return 0;
}

BSTR LibTTSGetError(size_t Index)
{
	const auto& Ref = ErrorQueue.at(Index);
	auto Ret = SysAllocString(Ref.c_str());
	ErrorQueue.erase(ErrorQueue.begin() + ptrdiff_t(Index));
	ErrorMx.unlock();
	return Ret;
}

void* LibTTSLoadVocoder(LPWSTR VocoderPath)
{
	if (!VocoderPath)
	{
		RaiseError(L"VocoderPath Could Not Be Null");
		return nullptr;
	}

	const auto& Env = moevsenv::GetGlobalMoeVSEnv();
	try
	{
		return new Ort::Session(*Env.GetEnv(), VocoderPath, *Env.GetSessionOptions());
	}
	catch (std::exception& e)
	{
		RaiseError(to_wide_string(e.what()));
		return nullptr;
	}
}

INT32 LibTTSUnloadVocoder(void* _Model)
{
	try
	{
		delete (Ort::Session*)_Model;
	}
	catch (std::exception& e)
	{
		RaiseError(to_wide_string(e.what()));
		return 1;
	}

	return 0;
}

void LibTTSEnableFileLogger(bool _Cond)
{
	MoeSSLogger::GetLogger().enable(_Cond);
}

void LibTTSWriteAudioFile(void* _PCMData, LPWSTR _OutputPath, INT32 _SamplingRate)
{
	InferTools::Wav::WritePCMData(_SamplingRate, 1, *(std::vector<int16_t>*)(_PCMData), _OutputPath);
}