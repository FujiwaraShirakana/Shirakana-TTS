﻿#pragma once
#include <functional>
#include <onnxruntime_cxx_api.h>
#include <regex>
#include <fstream>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include <Mui.h>
#include "../../StringPreprocess.hpp"
#include "../../PluginApi/pluginApi.hpp"
#include "../../Logger/MoeSSLogger.hpp"
#include "../../DataStruct/KDTree.hpp"
#include "../../InferTools/Project.hpp"
#ifdef max
#undef max
#include "../../Lib/rapidjson/rapidjson.h"
#include "../../Lib/rapidjson/document.h"
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#define INFERCLASSHEADER namespace InferClass{
#define INFERCLASSEND }
#include "../../Helper/Helper.h"

INFERCLASSHEADER
enum class FileType
{
	Audio,
	Image,
	Video,
	ImageVideo,
	DS
};

enum class modelType
{
	Taco,
	Vits,
	SoVits,
	diffSvc,
	diffSinger,
	Pits,
	RVC,
	DDSP
};

struct InferConfigs
{
	long keys = 0;
	long threshold = 30;
	long minLen = 5;
	long frame_len = 4 * 1024;
	long frame_shift = 512;
	int64_t pndm = 100;
	int64_t step = 1000;
	float gateThreshold = 0.666f;
	int64_t maxDecoderSteps = 3000;
	float noise_scale = 0.800f;
	float noise_scale_w = 1.f;
	float length_scale = 1.f;
	int64_t chara = 0;
	int64_t seed = 52468608;
	std::wstring emo;
	bool cp = false;
	size_t filter_window_len = 3;
	bool index = false;
	float index_rate = 1.f;
	float kmeans_rate = 0.f;
	double rt_batch_length = 200.0;
	uint64_t rt_batch_count = 10;
	double rt_cross_fade_length = 0.20;
	long rt_th = 1600;
	std::vector<float> chara_mix;
	bool use_iti = false;
	InferConfigs() = default;
};

class BaseModelType
{
public:
	enum class Device
	{
		CPU,
		CUDA
	};
	using callback = std::function<void(size_t, size_t)>;
	using callback_params = std::function<InferConfigs()>;
	using int64 = int64_t;
	using MTensor = Ort::Value;

	int InsertMessageToEmptyEditBox(std::wstring& _inputLens) const;
	static std::vector<std::wstring> CutLens(const std::wstring& input);
	void initRegex();
	long GetSamplingRate() const
	{
		return _samplingRate;
	}
	void ChangeDevice(Device _dev);
	std::wstring getEndString() const
	{
		return EndString;
	}
	const MoeSSPluginAPI* getPlugin() const
	{
		return &_plugin;
	}
	BaseModelType();
	virtual ~BaseModelType();
	virtual std::vector<int16_t> Inference(std::wstring& _inputLens) const;
protected:
	Ort::Env* env = nullptr;
	Ort::SessionOptions* session_options = nullptr;
	Ort::MemoryInfo* memory_info = nullptr;

	modelType _modelType = modelType::Taco;
	Device device_ = Device::CPU;

	long _samplingRate = 22050;

	MoeSSPluginAPI _plugin;

	callback _callback;
	callback_params _get_init_params;
	
	std::wstring EndString = L">>";
	std::wregex EmoReg, EmoParam, NoiseScaleParam, NoiseScaleWParam, LengthScaleParam, SeedParam, CharaParam, DecodeStepParam, GateParam;

	static constexpr long MaxPath = 8000l;
	std::wstring _outputPath = GetCurrentFolder() + L"\\outputs";
};

class EmoLoader
{
public:
	static constexpr long startPos = 128;
	EmoLoader() = default;
	EmoLoader(const std::wstring& path)
	{
		if (emofile)
			fclose(emofile);
		emofile = nullptr;
		_wfopen_s(&emofile, path.c_str(), L"r");
		if (!emofile)
			throw std::exception("emoFile not exists");
	}
	~EmoLoader()
	{
		if (emofile)
			fclose(emofile);
		emofile = nullptr;
	}
	void close()
	{
		if (emofile)
			fclose(emofile);
		emofile = nullptr;
	}
	void open(const std::wstring& path)
	{
		if (emofile)
			fclose(emofile);
		emofile = nullptr;
		_wfopen_s(&emofile, path.c_str(), L"rb");
		if (!emofile)
			throw std::exception("emoFile not exists");
	}
	std::vector<float> operator[](long index) const
	{
		if (emofile)
		{
			fseek(emofile, index * 4096 + startPos, SEEK_SET);
			char buffer[4096];
			const auto buf = reinterpret_cast<float*>(buffer);
			const auto bufread = fread_s(buffer, 4096, 1, 4096, emofile);
			if (bufread == 4096)
				return { buf ,buf + 1024 };
			throw std::exception("emo index out of range");
		}
		throw std::exception("emo file not opened");
	}
private:
	FILE* emofile = nullptr;
};

class TTS : public BaseModelType
{
public:
	using DurationCallback = std::function<void(std::vector<float>&)>;

	TTS() = default;

	std::vector<InferConfigs> GetParam(std::vector<std::wstring>& input) const;

	static std::vector<std::vector<bool>> generatePath(float* duration, size_t durationSize, size_t maskSize);

	std::vector<float> GetEmotionVector(std::wstring src) const;
protected:
	~TTS() override = default;
	DurationCallback _dcb;

	int64_t n_speaker = 1;

	bool CharaMix = false;
	bool add_blank = true;
	bool use_ph = false;
	bool emo = false;

	std::map<std::wstring, int64_t> _Phs;
	std::map<wchar_t, int64_t> _Symbols;
	
	EmoLoader emoLoader;
	std::string emoStringa;
	rapidjson::Document EmoJson;
};

class Kmeans
{
public:
	Kmeans() = default;
	~Kmeans() = default;
	Kmeans(const std::wstring& _path, size_t hidden_size, size_t KmeansLen)
	{
		FILE* file = nullptr;
		_wfopen_s(&file, _path.c_str(), L"rb");
		if (!file)
			throw std::exception("KMeansFileNotExist");
		constexpr long idx = 128;
		fseek(file, idx, SEEK_SET);
		std::vector<float> tmpData(hidden_size);
		const size_t ec = size_t(hidden_size) * sizeof(float);
		std::vector<std::vector<float>> _tmp;
		_tmp.reserve(KmeansLen);
		while (fread(tmpData.data(), 1, ec, file) == ec)
		{
			_tmp.emplace_back(tmpData);
			if (_tmp.size() == KmeansLen)
			{
				_tree.emplace_back(_tmp);
				_tmp.clear();
			}
		}
	}
	std::vector<float> find(const std::vector<float>& point, long chara)
	{
		auto res = _tree[chara].nearest_point(point);
		return res;
	}
private:
	std::vector<KDTree> _tree;
};

class SVC : public BaseModelType
{
public:
	SVC() = default;

	std::vector<MoeVSProject::Params> GetSvcParam(std::wstring& RawPath) const;

	std::vector<int64_t> GetNSFF0(const std::vector<float>&) const;

	static std::vector<float> GetInterpedF0(const std::vector<float>&);

	static std::vector<float> GetUV(const std::vector<float>&);

	static std::vector<int64_t> GetAligments(size_t, size_t);

	template<typename T>
	static std::vector<T> InterpResample(const std::vector<int16_t>& PCMData, long src, long dst)
	{
		if (src != dst)
		{
			const double intstep = double(src) / double(dst);
			const auto xi = arange(0, double(PCMData.size()), intstep);
			auto x0 = arange(0, double(PCMData.size()));
			while (x0.size() < PCMData.size())
				x0.emplace_back(x0[x0.size() - 1] + 1.0);
			while (x0.size() > PCMData.size())
				x0.pop_back();
			std::vector<double> y0(PCMData.size());
			for (size_t i = 0; i < PCMData.size(); ++i)
				y0[i] = double(PCMData[i]) / 32768.0;
			std::vector<double> yi(xi.size());
			interp1(x0.data(), y0.data(), long(x0.size()), xi.data(), long(xi.size()), yi.data());
			std::vector<T> out(xi.size());
			for (size_t i = 0; i < yi.size(); ++i)
				out[i] = T(yi[i]);
			return out;
		}
		std::vector<T> out(PCMData.size());
		for (size_t i = 0; i < PCMData.size(); ++i)
			out[i] = T(PCMData[i]) / (T)(32768.0);
		return out;
	}

	template<typename T>
	static std::vector<T> InterpFunc(const std::vector<T>& Data, long src, long dst)
	{
		if (src != dst)
		{
			const double intstep = double(src) / double(dst);
			auto xi = arange(0, double(Data.size()), intstep);
			while (xi.size() < dst)
				xi.emplace_back(xi[xi.size() - 1] + 1.0);
			while (xi.size() > dst)
				xi.pop_back();
			auto x0 = arange(0, double(Data.size()));
			while (x0.size() < Data.size())
				x0.emplace_back(x0[x0.size() - 1] + 1.0);
			while (x0.size() > Data.size())
				x0.pop_back();
			std::vector<double> y0(Data.size());
			for (size_t i = 0; i < Data.size(); ++i)
				y0[i] = double(Data[i]);
			std::vector<double> yi(xi.size());
			interp1(x0.data(), y0.data(), long(x0.size()), xi.data(), long(xi.size()), yi.data());
			std::vector<T> out(xi.size());
			for (size_t i = 0; i < yi.size(); ++i)
				out[i] = T(yi[i]);
			return out;
		}
		return Data;
	}

	std::vector<float> ExtractVolume(const std::vector<double>& OrgAudio) const;

	std::vector<float> GetInterpedF0log(const std::vector<float>&) const;

	InferConfigs GetInferConfigs() const
	{
		return _get_init_params();
	}

	int GetHopSize() const
	{
		return hop;
	}

	int64_t GetHiddenSize() const
	{
		return Hidden_Size;
	}

	long GetPndm() const
	{
		return pndm;
	}

	long GetMelBins() const
	{
		return melBins;
	}

	int64_t GetNSpeaker() const
	{
		return n_speaker;
	}
protected:
	~SVC() override = default;
	Ort::Session* hubert = nullptr;

	int hop = 320;
	int64_t Hidden_Size = 256;
	long pndm = 100;
	long melBins = 256;
	int64_t n_speaker = 1;
	bool CharaMix = false;
	bool VolumeB = false;
	bool V2 = false;
	bool ddsp = false;
	bool SV3 = false;
	bool SV4 = false;
	bool SVV2 = false;

	Kmeans* kmeans_ = nullptr;
	int64_t KMeans_Size = 10000;
	bool KMenas_Stat = false;
	bool Index_Stat = false;

	Ort::AllocatorWithDefaultOptions allocator;

	const int f0_bin = 256;
	const float f0_max = 1100.0;
	const float f0_min = 50.0;
	const float f0_mel_min = 1127.f * log(1.f + f0_min / 700.f);
	const float f0_mel_max = 1127.f * log(1.f + f0_max / 700.f);

	const std::vector<const char*> hubertOutput = { "embed" };
	const std::vector<const char*> hubertInput = { "source" };
};
INFERCLASSEND