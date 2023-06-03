#include "../header/Vits.hpp"

#include <random>

INFERCLASSHEADER
Vits::~Vits()
{
	logger.log(L"[Info] unloading Vits Models");
	delete sessionDec;
	delete sessionDp;
	delete sessionEnc_p;
	delete sessionFlow;
	delete sessionEmb;
	sessionDec = nullptr;
	sessionDp = nullptr;
	sessionEnc_p = nullptr;
	sessionFlow = nullptr;
	sessionEmb = nullptr;
	logger.log(L"[Info] Vits Models unloaded");
}

Vits::Vits(const rapidjson::Document& _config, const callback& _cb, const callback_params& _mr,const DurationCallback& _dcbb, Device _dev)
{
	_modelType = modelType::Vits;

	ChangeDevice(_dev);

	//Check Folder
	if (_config["Folder"].IsNull())
		throw std::exception("[Error] Missing field \"folder\" (Model Folder)");
	if (!_config["Folder"].IsString())
		throw std::exception("[Error] Field \"folder\" (Model Folder) Must Be String");
	const auto _folder = to_wide_string(_config["Folder"].GetString());
	if (_folder.empty())
		throw std::exception("[Error] Field \"folder\" (Model Folder) Can Not Be Empty");
	const std::wstring _path = GetCurrentFolder() + L"\\Models\\" + _folder + L"\\" + _folder;

	//Check SamplingRate
	if (_config["Rate"].IsNull())
		throw std::exception("[Error] Missing field \"Rate\" (SamplingRate)");
	if (_config["Rate"].IsInt() || _config["Rate"].IsInt64())
		_samplingRate = _config["Rate"].GetInt();
	else
		throw std::exception("[Error] Field \"Rate\" (SamplingRate) Must Be Int/Int64");

	logger.log(L"[Info] Current Sampling Rate is" + std::to_wstring(_samplingRate));

	//Check Symbol
	if (!_config.HasMember("Symbol") || _config["Symbol"].IsNull())
		throw std::exception("[Error] Missing field \"Symbol\" (PhSymbol)");
	if (_config.HasMember("AddBlank") && !_config["AddBlank"].IsNull())
		add_blank = _config["AddBlank"].GetBool();
	else
		logger.log(L"[Warn] Field \"AddBlank\" Is Missing, Use Default Value");

	//Load Symbol
	int64_t iter = 0;
	if (_config["Symbol"].IsArray())
	{
		logger.log(L"[Info] Use Phs");
		if (_config["Symbol"].Empty())
			throw std::exception("[Error] Field \"Symbol\" (PhSymbol) Can Not Be Empty");
		for (const auto& it : _config["Symbol"].GetArray())
			_Phs.insert({ to_wide_string(it.GetString()), iter++ });
		use_ph = true;
	}
	else
	{
		if (!_config["Symbol"].IsString())
			throw std::exception("[Error] Field \"Symbol\" (PhSymbol) Must Be Array<String> or String");
		logger.log(L"[Info] Use Symbols");
		const std::wstring SymbolsStr = to_wide_string(_config["Symbol"].GetString());
		if (SymbolsStr.empty())
			throw std::exception("[Error] Field \"Symbol\" (PhSymbol) Can Not Be Empty");
		for (auto it : SymbolsStr)
			_Symbols.insert(std::pair<wchar_t, int64_t>(it, iter++));
		use_ph = false;
	}

	if (_config.HasMember("Cleaner") && _config["Cleaner"].IsString())
	{
		const auto Cleaner = to_wide_string(_config["Cleaner"].GetString());
		if (!Cleaner.empty())
			switch (_plugin.Load(Cleaner))
			{
			case (-1):
			{
				logger.log(L"[Error] Plugin File Does Not Exist");
				_plugin.unLoad();
				break;
			}
			case (1):
			{
				logger.log(L"[Error] Plugin Has Some Error");
				_plugin.unLoad();
				break;
			}
			default:
			{
				logger.log(L"[Info] Plugin Loaded");
				break;
			}
			}
		else
			logger.log(L"[Info] Disable Plugin");
	}
	else
		logger.log(L"[Info] Disable Plugin");

	if(_config.HasMember("EmotionalPath") && _config["EmotionalPath"].IsString())
	{
		const auto emoStringload = to_wide_string(_config["EmotionalPath"].GetString());
		if(!emoStringload.empty())
		{
			logger.log(L"[Info] Loading EmotionVector");
			const auto emopath = GetCurrentFolder() + L"\\emotion\\" + emoStringload + L".npy";
			const auto emopathdef = GetCurrentFolder() + L"\\emotion\\" + emoStringload + L".json";
			emoLoader.open(emopath);
			emoStringa.clear();
			EmoJson.Clear();
			std::ifstream EmoFiles(emopathdef.c_str());
			if (EmoFiles.is_open())
			{
				std::string JsonLine;
				while (std::getline(EmoFiles, JsonLine))
					emoStringa += JsonLine;
				EmoFiles.close();
				EmoJson.Parse(emoStringa.c_str());
			}
			logger.log(L"[Info] EmotionVector Loaded");
			emo = true;
		}
	}

	if (!_config.HasMember("CharaMix") || !_config["CharaMix"].IsBool())
		logger.log(L"[Warn] Missing Field \"CharaMix\", Use Default Value (False)");
	else
		CharaMix = _config["CharaMix"].GetBool();
	if (_config.HasMember("Characters") && _config["Characters"].IsArray())
		n_speaker = _config["Characters"].Size();

	_callback = _cb;
	_get_init_params = _mr;
	_dcb = _dcbb;

	//LoadModels
	try
	{
		logger.log(L"[Info] loading Vits Models");
		sessionDec = new Ort::Session(*env, (_path + L"_dec.onnx").c_str(), *session_options);
		sessionDp = new Ort::Session(*env, (_path + L"_dp.onnx").c_str(), *session_options);
		sessionEnc_p = new Ort::Session(*env, (_path + L"_enc_p.onnx").c_str(), *session_options);
		sessionFlow = new Ort::Session(*env, (_path + L"_flow.onnx").c_str(), *session_options);
		if (_waccess((_path + L"_emb.onnx").c_str(), 0) != -1)
			sessionEmb = new Ort::Session(*env, (_path + L"_emb.onnx").c_str(), *session_options);
		else
			sessionEmb = nullptr;
		logger.log(L"[Info] Vits Models loaded");
	}
	catch (Ort::Exception& _exception)
	{
		delete sessionDec;
		delete sessionDp;
		delete sessionEnc_p;
		delete sessionFlow;
		delete sessionEmb;
		throw std::exception(_exception.what());
	}
}

std::vector<int16_t> Vits::Inference(std::wstring& _inputLens) const
{
	if (_inputLens.length() == 0)
	{
		logger.log(L"[Warn] Empty Input Box");
		int ret = InsertMessageToEmptyEditBox(_inputLens);
		if (ret == -1)
			throw std::exception("TTS Does Not Support Automatic Completion");
		if (ret == -2)
			throw std::exception("Please Select Files");
	}
	_inputLens += L'\n';
	std::vector<std::wstring> _Lens = CutLens(_inputLens);
	auto _configs = GetParam(_Lens);
	size_t proc = 0;
	_callback(proc, _Lens.size());
	std::vector<int16_t> _wavData;
	_wavData.reserve(441000);
	logger.log(L"[Info] Inferring");
	for (const auto& _input : _Lens)
	{
		logger.log(L"[Inferring] Inferring \"" + _input + L'\"');
		if (_input.empty())
		{
			logger.log(L"[Inferring] Skip Empty Len");
			continue;
		}
		auto noise_scale = _configs[proc].noise_scale;
		auto noise_scale_w = _configs[proc].noise_scale_w;
		auto length_scale = _configs[proc].length_scale;
		if (noise_scale < 0.0f || noise_scale > 50.0f)
			noise_scale = 0.667f;
		if (noise_scale_w < 0.0f || noise_scale_w > 50.0f)
			noise_scale_w = 0.800f;
		if (length_scale < 0.01f || length_scale > 50.0f)
			length_scale = 1.000f;
		const auto seed = _configs[proc].seed;
		const auto chara = _configs[proc].chara;
		//
		std::mt19937 gen(static_cast<unsigned int>(seed));
		std::normal_distribution<float> normal(0, 1);

		std::vector<int64> text;
		text.reserve(_input.length() * 4 + 4);
		if (!use_ph)
		{
			for (auto it : _input)
			{
				if (add_blank)
					text.push_back(0);
				text.push_back(_Symbols.at(it));
			}
			if (add_blank)
				text.push_back(0);
		}
		else
		{
			std::vector<std::wstring> textVec;
			textVec.reserve((_input.length() + 2) * 2);
			std::wstring _inputStrW = _input + L'|';
			while (!_inputStrW.empty())
			{
				const auto this_ph = _inputStrW.substr(0, _inputStrW.find(L'_'));
				if (add_blank)
					text.push_back(0);
				text.push_back(_Phs.at(this_ph));
				const auto idx = _inputStrW.find(L'|');
				if (idx != std::wstring::npos)
					_inputStrW = _inputStrW.substr(idx + 1);
			}
			if (add_blank)
				text.push_back(0);
		}
		int64 textLength[] = { (int64)text.size() };
		std::vector<MTensor> outputTensors;
		std::vector<MTensor> inputTensors;
		const int64 inputShape1[2] = { 1,textLength[0] };
		constexpr int64 inputShape2[1] = { 1 };
		inputTensors.push_back(MTensor::CreateTensor<int64>(
			*memory_info, text.data(), textLength[0], inputShape1, 2));
		inputTensors.push_back(MTensor::CreateTensor<int64>(
			*memory_info, textLength, 1, inputShape2, 1));
		logger.log(L"[Inferring] Inferring \"" + _input + L"\" Encoder");
		if (emo)
		{
			constexpr int64 inputShape3[1] = { 1024 };
			auto emoVec = GetEmotionVector(_configs[proc].emo);
			inputTensors.push_back(MTensor::CreateTensor<float>(
				*memory_info, emoVec.data(), 1024, inputShape3, 1));
			try
			{
				outputTensors = sessionEnc_p->Run(Ort::RunOptions{ nullptr },
					EnceInput.data(),
					inputTensors.data(),
					inputTensors.size(),
					EncOutput.data(),
					EncOutput.size());
			}
			catch (Ort::Exception& e)
			{
				throw std::exception((std::string("Locate: enc_p\n") + e.what()).c_str());
			}
		}
		else
		{
			try
			{
				outputTensors = sessionEnc_p->Run(Ort::RunOptions{ nullptr },
					EncInput.data(),
					inputTensors.data(),
					inputTensors.size(),
					EncOutput.data(),
					EncOutput.size());
			}
			catch (Ort::Exception& e)
			{
				throw std::exception((std::string("Locate: enc_p\n") + e.what()).c_str());
			}
		}
		std::vector<float>
			m_p(outputTensors[1].GetTensorData<float>(), outputTensors[1].GetTensorData<float>() + outputTensors[1].GetTensorTypeAndShapeInfo().GetElementCount()),
			logs_p(outputTensors[2].GetTensorData<float>(), outputTensors[2].GetTensorData<float>() + outputTensors[2].GetTensorTypeAndShapeInfo().GetElementCount()),
			x_mask(outputTensors[3].GetTensorData<float>(), outputTensors[3].GetTensorData<float>() + outputTensors[3].GetTensorTypeAndShapeInfo().GetElementCount());

		const auto xshape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
		std::vector<const char*> DpInput = DpInputTmp, FlowInput = FlowInputTmp, DecInput = DecInputTmp;
		std::vector<MTensor> inputSid;
		std::vector<MTensor> outputG;
		if (sessionEmb)
		{
			logger.log(L"[Inferring] Inferring \"" + _input + L"\" Character Embidding");
			try {
				if (CharaMix && !_configs[proc].chara_mix.empty())
				{
					const auto& Charas = _configs[proc].chara_mix;
					int64_t csid = 0;
					for (const auto& CharaP : Charas)
					{
						if (!csid)
						{
							int64 Character[1] = { csid };
							inputSid.push_back(MTensor::CreateTensor<int64>(
								*memory_info, Character, 1, inputShape2, 1));

							outputG = sessionEmb->Run(Ort::RunOptions{ nullptr },
								EMBInput.data(),
								inputSid.data(),
								inputSid.size(),
								EMBOutput.data(),
								EMBOutput.size());

							auto gemb = outputG[0].GetTensorMutableData<float>();
							for (int gembi = 0; gembi < 256; ++gembi)
								gemb[gembi] *= float(CharaP);
							++csid;
						}
						else
						{
							inputSid.clear();
							int64 Character[1] = { csid };
							inputSid.push_back(MTensor::CreateTensor<int64>(
								*memory_info, Character, 1, inputShape2, 1));

							const auto TempG = sessionEmb->Run(Ort::RunOptions{ nullptr },
								EMBInput.data(),
								inputSid.data(),
								inputSid.size(),
								EMBOutput.data(),
								EMBOutput.size());

							auto gemb = outputG[0].GetTensorMutableData<float>();
							for (int gembi = 0; gembi < 256; ++gembi)
								gemb[gembi] += TempG[0].GetTensorData<float>()[gembi] * float(CharaP);
							++csid;
						}
					}
				}
				else
				{
					int64 Character[1] = { chara };
					inputSid.push_back(MTensor::CreateTensor<int64>(
						*memory_info, Character, 1, inputShape2, 1));
					outputG = sessionEmb->Run(Ort::RunOptions{ nullptr },
						EMBInput.data(),
						inputSid.data(),
						inputSid.size(),
						EMBOutput.data(),
						EMBOutput.size());
				}
			}
			catch (Ort::Exception& e)
			{
				throw std::exception((std::string("Locate: emb\n") + e.what()).c_str());
			}
		}
		else
		{
			DpInput.pop_back();
			FlowInput.pop_back();
			DecInput.pop_back();
		}
		const int64 zinputShape[3] = { xshape[0],2,xshape[2] };
		const int64 zinputCount = xshape[0] * xshape[2] * 2;
		std::vector<float> zinput(zinputCount, 0.0);
		for (auto& it : zinput)
			it = normal(gen) * noise_scale_w;

		inputTensors.clear();
		inputTensors.push_back(std::move(outputTensors[0]));
		inputTensors.push_back(std::move(outputTensors[3]));
		inputTensors.push_back(MTensor::CreateTensor<float>(
			*memory_info, zinput.data(), zinputCount, zinputShape, 3));
		std::vector<int64> GShape;
		if (sessionEmb) {
			GShape = outputG[0].GetTensorTypeAndShapeInfo().GetShape();
			GShape.push_back(1);
			inputTensors.push_back(MTensor::CreateTensor<float>(
				*memory_info, outputG[0].GetTensorMutableData<float>(), outputG[0].GetTensorTypeAndShapeInfo().GetElementCount(), GShape.data(), 3));
		}
		try
		{
			logger.log(L"[Inferring] Inferring \"" + _input + L"\" DurationPredictor");
			outputTensors = sessionDp->Run(Ort::RunOptions{ nullptr },
				DpInput.data(),
				inputTensors.data(),
				inputTensors.size(),
				DpOutput.data(),
				DpOutput.size());
		}
		catch (Ort::Exception& e)
		{
			throw std::exception((std::string("Locate: dp\n") + e.what()).c_str());
		}
		const auto w_ceil = outputTensors[0].GetTensorMutableData<float>();
		const size_t w_ceilSize = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
		for (size_t i = 0; i < w_ceilSize; ++i)
			w_ceil[i] = ceil(exp(w_ceil[i]) * x_mask[i] * length_scale);
		const auto maskSize = x_mask.size();
		if (_configs[proc].cp)
		{
			/*
			std::vector<UISliderLayer::itemParam> dstValue;
			dstValue.reserve((_inputStr.length()));
			for (size_t itr = 0; itr < _inputStr.length(); ++itr)
				dstValue.emplace_back(UISliderLayer::itemParam(_inputStr[itr] + std::to_wstring((int)w_ceil[itr * 2 + 1]), 1.0f));
			auto callback = [&](const std::vector<float>& input)
			{
				if (add_blank)
					for (size_t itr = 0; itr < input.size(); ++itr)
						w_ceil[itr * 2 + 1] = ceil(w_ceil[itr * 2 + 1] * input[itr]);
				else
					for (size_t itr = 0; itr < input.size(); ++itr)
						w_ceil[itr] = ceil(w_ceil[itr] * input[itr]);
				_mainWindow->tts_layer->opened = false;
			};
			_mainWindow->tts_layer->ShowLayer(L"��������Duration�ı���", std::move(dstValue), callback);
			while (_mainWindow->tts_layer->opened)
				Sleep(200);
			 */
		}
		float y_length_f = 0.0;
		int64 y_length;
		for (size_t i = 0; i < w_ceilSize; ++i)
			y_length_f += w_ceil[i];
		if (y_length_f < 1.0f)
			y_length = 1;
		else
			y_length = (int64)y_length_f;
		auto attn = generatePath(w_ceil, y_length, maskSize);
		std::vector<std::vector<float>> logVec(192, std::vector<float>(y_length, 0.0f));
		std::vector<std::vector<float>> mpVec(192, std::vector<float>(y_length, 0.0f));
		std::vector<float> nlogs_pData(192 * y_length);
		for (size_t i = 0; i < static_cast<size_t>(y_length); ++i)
		{
			for (size_t j = 0; j < 192; ++j)
			{
				for (size_t k = 0; k < maskSize; k++)
				{
					if (attn[i][k])
					{
						mpVec[j][i] += m_p[j * maskSize + k];
						logVec[j][i] += logs_p[j * maskSize + k];
					}
				}
				nlogs_pData[j * y_length + i] = mpVec[j][i] + normal(gen) * exp(logVec[j][i]) * noise_scale;
			}
		}
		inputTensors.clear();
		std::vector<float> y_mask(y_length);
		for (size_t i = 0; i < static_cast<size_t>(y_length); ++i)
			y_mask[i] = 1.0f;
		const int64 zshape[3] = { 1,192,y_length };
		const int64 yshape[3] = { 1,1,y_length };
		try
		{
			inputTensors.push_back(MTensor::CreateTensor<float>(
				*memory_info, nlogs_pData.data(), 192 * y_length, zshape, 3));
			inputTensors.push_back(MTensor::CreateTensor<float>(
				*memory_info, y_mask.data(), y_length, yshape, 3));
			logger.log(L"[Inferring] Inferring \"" + _input + L"\" Flow");
			if (sessionEmb)
			{
				inputTensors.push_back(MTensor::CreateTensor<float>(
					*memory_info, outputG[0].GetTensorMutableData<float>(), outputG[0].GetTensorTypeAndShapeInfo().GetElementCount(), GShape.data(), 3));
			}
			outputTensors = sessionFlow->Run(Ort::RunOptions{ nullptr },
				FlowInput.data(),
				inputTensors.data(),
				inputTensors.size(),
				FlowOutput.data(),
				FlowOutput.size());
			inputTensors[0] = std::move(outputTensors[0]);
			if (sessionEmb)
				inputTensors[1] = std::move(inputTensors[2]);
			inputTensors.pop_back();
			logger.log(L"[Inferring] Inferring \"" + _input + L"\" Decoder");
			outputTensors = sessionDec->Run(Ort::RunOptions{ nullptr },
				DecInput.data(),
				inputTensors.data(),
				inputTensors.size(),
				DecOutput.data(),
				DecOutput.size());
		}
		catch (Ort::Exception& e)
		{
			throw std::exception((std::string("Locate: dec & flow\n") + e.what()).c_str());
		}
		const auto shapeOut = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
		const auto outData = outputTensors[0].GetTensorData<float>();
		for (int bbb = 0; bbb < shapeOut[2]; bbb++)
			_wavData.emplace_back(static_cast<int16_t>(outData[bbb] * 32768.0f));
		_callback(++proc, _Lens.size());
		logger.log(L"[Inferring] \"" + _input + L"\" Finished");
	}
	logger.log(L"[Info] Finished");
	return _wavData;
}

std::vector<int16_t> Vits::Inference(const MoeVSProject::TTSParams& _input) const
{
	std::vector<int16_t> _wavData;
	_wavData.reserve(441000);
	logger.log(L"[Inferring] Inferring \"" + _input.phs + L'\"');
	if (_input.phs.empty())
	{
		logger.log(L"[Inferring] Skip Empty Len");
		return {};
	}
	auto noise_scale = (float)_input.noise;
	auto noise_scale_w = (float)_input.noise_w;
	auto length_scale = float(_input.length);
	const auto seed = _input.seed;
	const int64_t chara = _input.chara;
	bool use_spk_mix = CharaMix;
	if (_input.chara_mix.empty())
		use_spk_mix = false;
	auto chara_mix_dat = _input.chara_mix;
	auto input_text = _input.phs;
	auto dur_vec = _input.durations;
	auto inp_emo = _input.emotion;
	if (chara_mix_dat.size() > size_t(n_speaker))
		chara_mix_dat.resize(n_speaker);
	LinearCombination(chara_mix_dat);

	std::mt19937 gen(static_cast<unsigned int>(seed));
	std::normal_distribution<float> normal(0, 1);

	std::vector<int64> text;
	text.reserve(input_text.length() * 4 + 4);
	if (!use_ph)
	{
		for (auto it : input_text)
		{
			if (add_blank)
				text.push_back(0);
			text.push_back(_Symbols.at(it));
		}
		if (add_blank)
			text.push_back(0);
	}
	else
	{
		std::vector<std::wstring> textVec;
		textVec.reserve((input_text.length() + 2) * 2);
		std::wstring _inputStrW = input_text + L'|';
		while (!_inputStrW.empty())
		{
			const auto this_ph = _inputStrW.substr(0, _inputStrW.find(L'_'));
			if (add_blank)
				text.push_back(0);
			text.push_back(_Phs.at(this_ph));
			const auto idx = _inputStrW.find(L'|');
			if (idx != std::wstring::npos)
				_inputStrW = _inputStrW.substr(idx + 1);
		}
		if (add_blank)
			text.push_back(0);
	}
	int64 textLength[] = { (int64)text.size() };
	std::vector<MTensor> outputTensors;
	std::vector<MTensor> inputTensors;
	const int64 inputShape1[2] = { 1,textLength[0] };
	constexpr int64 inputShape2[1] = { 1 };
	inputTensors.push_back(MTensor::CreateTensor<int64>(
		*memory_info, text.data(), textLength[0], inputShape1, 2));
	inputTensors.push_back(MTensor::CreateTensor<int64>(
		*memory_info, textLength, 1, inputShape2, 1));
	logger.log(L"[Inferring] Inferring \"" + input_text + L"\" Encoder");
	if (emo)
	{
		constexpr int64 inputShape3[1] = { 1024 };
		auto emoVec = GetEmotionVector(inp_emo);
		inputTensors.push_back(MTensor::CreateTensor<float>(
			*memory_info, emoVec.data(), 1024, inputShape3, 1));
		try
		{
			outputTensors = sessionEnc_p->Run(Ort::RunOptions{ nullptr },
				EnceInput.data(),
				inputTensors.data(),
				inputTensors.size(),
				EncOutput.data(),
				EncOutput.size());
		}
		catch (Ort::Exception& e)
		{
			throw std::exception((std::string("Locate: enc_p\n") + e.what()).c_str());
		}
	}
	else
	{
		try
		{
			outputTensors = sessionEnc_p->Run(Ort::RunOptions{ nullptr },
				EncInput.data(),
				inputTensors.data(),
				inputTensors.size(),
				EncOutput.data(),
				EncOutput.size());
		}
		catch (Ort::Exception& e)
		{
			throw std::exception((std::string("Locate: enc_p\n") + e.what()).c_str());
		}
	}
	std::vector<float>
		m_p(outputTensors[1].GetTensorData<float>(), outputTensors[1].GetTensorData<float>() + outputTensors[1].GetTensorTypeAndShapeInfo().GetElementCount()),
		logs_p(outputTensors[2].GetTensorData<float>(), outputTensors[2].GetTensorData<float>() + outputTensors[2].GetTensorTypeAndShapeInfo().GetElementCount()),
		x_mask(outputTensors[3].GetTensorData<float>(), outputTensors[3].GetTensorData<float>() + outputTensors[3].GetTensorTypeAndShapeInfo().GetElementCount());

	const auto xshape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
	std::vector<const char*> DpInput = DpInputTmp, FlowInput = FlowInputTmp, DecInput = DecInputTmp;
	std::vector<MTensor> inputSid;
	std::vector<float> GEmbidding;
	std::vector<int64_t> GOutShape;
	if (sessionEmb)
	{
		logger.log(L"[Inferring] Inferring \"" + input_text + L"\" Character Embidding");
		std::vector<MTensor> outputG;
		if (use_spk_mix)
		{
			int64_t csid = 0;
			for (const auto& CharaP : chara_mix_dat)
			{
				outputG.clear();
				inputSid.clear();
				if (csid >= n_speaker)
					break;
				if (CharaP < 0.0001f)
				{
					++csid;
					continue;
				}
				int64 Character[1] = { csid };
				inputSid.push_back(MTensor::CreateTensor<int64>(
					*memory_info, Character, 1, inputShape2, 1));
				try
				{
					outputG = sessionEmb->Run(Ort::RunOptions{ nullptr },
					   EMBInput.data(),
					   inputSid.data(),
					   inputSid.size(),
					   EMBOutput.data(),
					   EMBOutput.size());
				}
				catch (Ort::Exception& e)
				{
					throw std::exception((std::string("Locate: emb\n") + e.what()).c_str());
				}
				const auto GOutCount = outputG[0].GetTensorTypeAndShapeInfo().GetElementCount();
				if (GOutShape.empty())
				{
					GEmbidding = std::vector(outputG[0].GetTensorData<float>(), outputG[0].GetTensorData<float>() + GOutCount);
					GOutShape = outputG[0].GetTensorTypeAndShapeInfo().GetShape();
					GOutShape.emplace_back(1);
					for (auto idx : GEmbidding)
						idx *= float(CharaP);
				}
				else
					for (size_t i = 0; i < GOutCount; ++i)
						GEmbidding[i] += outputG[0].GetTensorData<float>()[i] * float(CharaP);
				++csid;
			}
		}
		else
		{
			int64 Character[1] = { chara };
			inputSid.push_back(MTensor::CreateTensor<int64>(
				*memory_info, Character, 1, inputShape2, 1));
			try
			{
				outputG = sessionEmb->Run(Ort::RunOptions{ nullptr },
				   EMBInput.data(),
				   inputSid.data(),
				   inputSid.size(),
				   EMBOutput.data(),
				   EMBOutput.size());
			}
			catch (Ort::Exception& e)
			{
				throw std::exception((std::string("Locate: emb\n") + e.what()).c_str());
			}
			const auto GOutCount = outputG[0].GetTensorTypeAndShapeInfo().GetElementCount();
			GEmbidding = std::vector(outputG[0].GetTensorData<float>(), outputG[0].GetTensorData<float>() + GOutCount);
			GOutShape = outputG[0].GetTensorTypeAndShapeInfo().GetShape();
		}
	}
	else
	{
		DpInput.pop_back();
		FlowInput.pop_back();
		DecInput.pop_back();
	}
	std::vector w_ceil(textLength[0], 2.f);

	bool enable_dp = true;
	if (dur_vec.size() == w_ceil.size() || dur_vec.size() == w_ceil.size() / 2)
		enable_dp = false;
	if(!enable_dp)
		for (auto& i : dur_vec)
			if (i < 1)
				i = 2;

	if(enable_dp)
	{
		const int64 zinputShape[3] = { xshape[0],2,xshape[2] };
		const int64 zinputCount = xshape[0] * xshape[2] * 2;
		std::vector<float> zinput(zinputCount, 0.0);
		for (auto& it : zinput)
			it = normal(gen) * noise_scale_w;
		inputTensors.clear();
		inputTensors.push_back(std::move(outputTensors[0]));
		inputTensors.push_back(std::move(outputTensors[3]));
		inputTensors.push_back(MTensor::CreateTensor<float>(
			*memory_info, zinput.data(), zinputCount, zinputShape, 3));
		if (sessionEmb) {
			inputTensors.push_back(MTensor::CreateTensor<float>(
				*memory_info, GEmbidding.data(), GEmbidding.size(), GOutShape.data(), 3));
		}
		try
		{
			logger.log(L"[Inferring] Inferring \"" + input_text + L"\" DurationPredictor");
			outputTensors = sessionDp->Run(Ort::RunOptions{ nullptr },
				DpInput.data(),
				inputTensors.data(),
				inputTensors.size(),
				DpOutput.data(),
				DpOutput.size());
		}
		catch (Ort::Exception& e)
		{
			throw std::exception((std::string("Locate: dp\n") + e.what()).c_str());
		}
		const auto w_data = outputTensors[0].GetTensorMutableData<float>();
		const auto w_data_length = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
		if (w_data_length != w_ceil.size())
			w_ceil.resize(w_data_length, 2.f);
		for (size_t i = 0; i < w_ceil.size(); ++i)
			w_ceil[i] = ceil(exp(w_data[i]) * x_mask[i] * length_scale);
	}
	else
	{
		if (dur_vec.size() == text.size())
			for (size_t i = 0; i < w_ceil.size(); ++i)
				w_ceil[i] = float(dur_vec[i]);
		else if (add_blank && dur_vec.size() == text.size() / 2ull)
			for (size_t i = 0; i < dur_vec.size(); ++i)
				w_ceil[1 + i * 2] = float(dur_vec[i]);
	}
	const auto maskSize = x_mask.size();
	float y_length_f = 0.0;
	int64 y_length;
	for (size_t i = 0; i < w_ceil.size(); ++i)
		y_length_f += w_ceil[i];
	if (y_length_f < 1.0f)
		y_length = 1;
	else
		y_length = (int64)y_length_f;

	auto attn = generatePath(w_ceil.data(), y_length, maskSize);
	std::vector logVec(192, std::vector(y_length, 0.0f));
	std::vector mpVec(192, std::vector(y_length, 0.0f));
	std::vector<float> nlogs_pData(192 * y_length);
	for (size_t i = 0; i < static_cast<size_t>(y_length); ++i)
	{
		for (size_t j = 0; j < 192; ++j)
		{
			for (size_t k = 0; k < maskSize; k++)
			{
				if (attn[i][k])
				{
					mpVec[j][i] += m_p[j * maskSize + k];
					logVec[j][i] += logs_p[j * maskSize + k];
				}
			}
			nlogs_pData[j * y_length + i] = mpVec[j][i] + normal(gen) * exp(logVec[j][i]) * noise_scale;
		}
	}
	inputTensors.clear();
	std::vector y_mask(y_length, 1.0f);
	const int64 zshape[3] = { 1,192,y_length };
	const int64 yshape[3] = { 1,1,y_length };
	try
	{
		inputTensors.push_back(MTensor::CreateTensor<float>(
			*memory_info, nlogs_pData.data(), 192 * y_length, zshape, 3));
		inputTensors.push_back(MTensor::CreateTensor<float>(
			*memory_info, y_mask.data(), y_length, yshape, 3));
		logger.log(L"[Inferring] Inferring \"" + input_text + L"\" Flow");
		if (sessionEmb)
			inputTensors.push_back(MTensor::CreateTensor<float>(
				*memory_info, GEmbidding.data(), GEmbidding.size(), GOutShape.data(), 3));
		outputTensors = sessionFlow->Run(Ort::RunOptions{ nullptr },
			FlowInput.data(),
			inputTensors.data(),
			inputTensors.size(),
			FlowOutput.data(),
			FlowOutput.size());
		inputTensors[0] = std::move(outputTensors[0]);
		if (sessionEmb)
			inputTensors[1] = std::move(inputTensors[2]);
		inputTensors.pop_back();
		logger.log(L"[Inferring] Inferring \"" + input_text + L"\" Decoder");
		outputTensors = sessionDec->Run(Ort::RunOptions{ nullptr },
			DecInput.data(),
			inputTensors.data(),
			inputTensors.size(),
			DecOutput.data(),
			DecOutput.size());
	}
	catch (Ort::Exception& e)
	{
		throw std::exception((std::string("Locate: dec & flow\n") + e.what()).c_str());
	}
	const auto shapeOut = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
	const auto outData = outputTensors[0].GetTensorData<float>();
	for (int bbb = 0; bbb < shapeOut[2]; bbb++)
		_wavData.emplace_back(static_cast<int16_t>(outData[bbb] * 32768.0f));
	logger.log(L"[Inferring] \"" + input_text + L"\" Finished");
	return _wavData;
}
INFERCLASSEND