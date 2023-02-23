/*
* file: MainWindow.h
* info: Ӧ�ó��������ڶ��� ʹ��MiaoUI
*
* Author: Maplespe(mapleshr@icloud.com)
* 
* date: 2022-9-19 Create
*/
#pragma once
#include "..\framework.h"
#include <onnxruntime_cxx_api.h>
#include <regex>
#include <shellapi.h>
#include <Render\Sound\Mui_DirectSound.h>
#include "../Infer/inferTools.hpp"
#include "../PluginApi/pluginApi.hpp"
#include "UISliderLayer.h"
#include "../Lib/rapidjson/document.h"
#include "../Lib/rapidjson/writer.h"
#include "../Player/AudioPlayer.h"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")

#define CUDAMOESS

namespace ttsUI
{
	constexpr size_t MaxPath = 32000;

	class MioTTSControl;

	using namespace Mui;
	//�����ڳߴ�
	extern const UISize m_wndSize;
	//�����ڱ���
	extern std::wstring m_wndTitle;

	//��ʼ��and����ʼ��
	extern bool MiaoUI_Initialize(std::wstring& error);
	extern void MiaoUI_Uninitialize();

	//Ӧ�ó��������� WIndowsƽ̨
	class MainWindow : public Window::UIWindowsWnd
	{
	public:
		MainWindow(MRender* _render_) : UIWindowsWnd(_render_) {}
		~MainWindow() override;

		//�����¼��ص�
		bool EventProc(UINotifyEvent, UIControl*, _m_param) override;
		//�����¼�Դ WIndows�����¼��ص�
		_m_result EventSource(_m_param, _m_param) override;

		void setTTSCon(MioTTSControl*&& _ttsCon)
		{
			ttsCon = _ttsCon;
		}

		UISliderLayer* tts_layer = nullptr;
		VcConfigLayer* vc_layer = nullptr;
		UIAudioPlayer* m_player = nullptr;
	private:
		bool AfterCreated();
		bool CreateControls(UIControl* root);
		MioTTSControl* ttsCon = nullptr;
		MHelper::MuiXML* m_uiXML = nullptr;
		MFontLoader* m_fontLoader = nullptr;
		friend void MainWindow_CreateLoop();
	};

	struct modelInfo
	{
		std::wstring Folder;
		std::wstring Name;
		std::wstring Type;
		std::wstring Symbol;
		std::wstring Cleaner;
		int Samrate = 0;
		int hop = 320;
		std::wstring Hifigan;
		std::vector<std::wstring> Speakers;
		std::wstring Hubert;
		int pndm = 100;
		int melBins = 128;
		bool SV3 = false;
		bool SV4 = false;
		bool emo = false;
		std::wstring emoPath;
		bool V2 = false;
		std::wstring dicFolder;
		bool MidiVer = false;
	};

	enum class modelType
	{
		Taco,
		Vits,
		SoVits,
		diffSvc,
		diffSinger
	};

	struct CutInfo
	{
		long keys = 0;
		long threshold = 30;
		long minLen = 5;
		long frame_len = 4 * 1024;
		long frame_shift = 512;
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
			if(emofile)
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
			if(emofile)
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

	struct DiffusionInfo
	{
		int64_t pndm = 100;
		int64_t step = 1000;
	};

	struct DiffSingerInput
	{
		std::vector<std::vector<int64_t>> inputLens;
		std::vector<std::vector<int64_t>> durations;
		std::vector<std::vector<bool>> is_slur;
		std::vector<std::vector<int64_t>> pitch_durations;
		std::vector<std::vector<int64_t>> pitchs;
		std::vector<std::vector<float>> f0;
		std::vector<double> offset;
	};

	//�ؼ�����
	class MioTTSControl
	{
	public:
		MioTTSControl() = delete;

		MioTTSControl(ttsUI::MainWindow*&& _main)
		{
			_mainWindow = _main;
			_uiControl = _main->GetRootControl();
			hWnd = (HWND)(_main->GetWindowHandle());
		}
		
		friend class MainWindow;

		//����ģ��
		void loadmodel();

		//ע���������¼�
		void RegisterLeftClickEvent(std::wstring&& _control, std::function<bool()>&& _fun) {
			_levents.insert(std::pair<std::wstring, std::function<bool()>>(_control, _fun));
		}

		//ע����ֵ�¼�
		void RegisterValEvent(std::wstring&& _control, std::function<bool(uint64_t)>&& _fun) {
			_sevents.insert(std::pair<std::wstring, std::function<bool(uint64_t)>>(_control, _fun));
		}

		//ע��ؼ�
		void RegisterControl();

		//ע���¼�
		void RegisterEvent();

		//��ʼ���ؼ�
		void setConsDefalut() const;

		//���ģ��
		void ClearModel();

		//��ʼ��
		void InitControl();

		//��ʼ��
		void InferQuit() const;

		//Tacotron2
		void InferTaco2(const std::wstring& _input);

		//InferVits
		//void InferVits(const std::vector<std::wstring>& _input);

		//InferVits
		void InferVits(const std::vector<std::wstring>& _input, const std::vector<std::vector<std::wstring>>& Params);

		//���
		std::wstring getCleanerStr(std::wstring& _input) const;

		//����ģ������
		void ClearTModel();

		//���ز���ƥ�����
		void initRegex();

		void setSoundModule(bool Switch) const
		{
			voice_play->SetEnabled(Switch);
			voice_save->SetEnabled(Switch);
			voice_prog->SetEnabled(Switch);
		}

		void inferSovits(std::wstring& path, const CutInfo& cutinfo, int charEmb = 0);

		void inferSvc(std::wstring& path, const CutInfo& cutinfo, int charEmb);

		void inferDiff(std::wstring& path, const CutInfo& cutinfo, int charEmb, const DiffusionInfo& finfo);

		void inferSovitsT(const std::vector<std::wstring>&, const std::vector<std::vector<std::wstring>>&, const CutInfo&, int);

		void inferSvcT(const std::vector<std::wstring>&, const std::vector<std::vector<std::wstring>>&, const CutInfo&, int, const DiffusionInfo& finfo);

		void inferDiffSinger(const DiffSingerInput&, const CutInfo&, int, const DiffusionInfo& finfo);

		void inferDiffSinger2(const DiffSingerInput& input, const CutInfo& cutinfo, int chara, const DiffusionInfo& finfo);

		//��ȡDiffSinger����
		std::vector<DiffSingerInput> preprocessDiffSinger(const std::vector<std::wstring>& Jsonpath);

		//��ȡEmotionVector
		std::vector<float> GetEmotionVector(std::wstring src);

		//��ȡ����
		std::vector<std::vector<std::wstring>> GetParam(std::vector<std::wstring>& input) const;

		void OutPutBatchAudio(const std::wstring& _inputPath);

		void GetUIResourceFromDataChunk();

		void GetUIResourceFromDataChunk(int);

		void GetAudio();

		UIProgressBar* process = nullptr;      //ת������
		UIBlurLayer* titlebar = nullptr;       //������
		UIButton* title_close = nullptr;       //�رհ�ť
		UIButton* title_minisize = nullptr;    //��С����ť
		UILabel* title_label = nullptr;        //����
		IconButton* tts_begin = nullptr;       //��ʼת����ť
		UIEditBox* wave_editbox = nullptr;     //�����ַ���
		UIComBox* modlist_main = nullptr;      //ģ���б�
		UIComBox* modlist_child = nullptr;     //˵����
		IconButton* modlist_refresh = nullptr; //ˢ���б�
		IconButton* modlist_folder = nullptr;  //��ģ���ļ���
		UILabel* voice_timecur = nullptr;      //��ǰ����ʱ��
		UILabel* voice_timeall = nullptr;      //��Ƶʱ��
		UIButton* voice_play = nullptr;        //���Ű�ť
		IconButton* voice_save = nullptr;      //���水ť
		UISlider* voice_volume = nullptr;      //��������
		UILabel* volume_text = nullptr;        //����
		UIImgBox* voice_imgbox = nullptr;      //ģ������
		UILabel* voice_imgname = nullptr;      //ģ����
		UISlider* voice_prog = nullptr;        //���Ž�����
		IconButton* clean_begin = nullptr;     //���
		UIEditBox* noise_scale_box = nullptr;  //������ģ
		UIEditBox* length_scale_box = nullptr; //���ȹ�ģ
		UIEditBox* noise_scale_w_box = nullptr;//DurationPredictor������ģ
		UIComBox* modlist_child2 = nullptr;    //����ģ��
		UIEditBox* tran_seeds = nullptr;       //����
		UICheckBox* customPathStat = nullptr;  //�Զ���Path��Duration��

		int sampingRate = 0;
	protected:
		//Regex
		std::wstring EndString = L">>";
		std::wregex EmoReg;
		std::wregex EmoParam;
		std::wregex NoiseScaleParam;
		std::wregex NoiseScaleWParam;
		std::wregex LengthScaleParam;
		std::wregex SeedParam;
		std::wregex CharaParam;

		//Event
		std::map <std::wstring, std::function<bool()>> _levents;
		std::map <std::wstring, std::function<bool(uint64_t)>> _sevents;

		//ModelConfig
		std::vector<modelInfo> _models;
		std::wstring _curCleaner;
		std::vector<std::wstring> TModelFolder;
		int curHop = 0;
		bool Sv3 = false;
		bool Sv4 = false;
		bool emo = false;
		std::map<wchar_t, int64_t> _symbol;
		MoeSSPluginAPI pluginApi;
		int curPndm = 100;
		bool enableTModel = false;
		int curMelBins = 128;
		int tModelSr = 0;
		EmoLoader emoLoader;
		std::string emoStringa;
		rapidjson::Document EmoJson;
		bool V2 = false;
		std::map<std::wstring, std::vector<std::wstring>> PhonesPair;
		std::map<std::wstring, int64_t> Phones;
		modelType _modelType = modelType::Taco;
		uint32_t n_speakers = 0;
		bool MidiVer = false;

		//OutWavTmp
		std::vector<short> _wavData;
		UIResource tmpWav;

		//MainWindow
		MainWindow* _mainWindow = nullptr;
		UIControl* _uiControl = nullptr;
		HWND hWnd = nullptr;

		//OrtEnv
		Ort::Env* env = nullptr;
		Ort::SessionOptions* session_options = nullptr;
#ifdef CUDAMOESS
		Ort::SessionOptions* session_options_nsf = nullptr;
#endif
		Ort::MemoryInfo* memory_info = nullptr;
		
		//Tacotron2
		Ort::Session* sessionEncoder = nullptr;
		Ort::Session* sessionDecoderIter = nullptr;
		Ort::Session* sessionPostNet = nullptr;
		Ort::Session* sessionGan = nullptr;

		//Vits
		Ort::Session* sessionDec = nullptr;
		Ort::Session* sessionDp = nullptr;
		Ort::Session* sessionEnc_p = nullptr;
		Ort::Session* sessionFlow = nullptr;
		Ort::Session* sessionEmb = nullptr;

		//HiddenUnitBERT
		Ort::Session* hubert = nullptr;

		//SoftVits
		Ort::Session* soVits = nullptr;

		//Diffusion
		Ort::Session* diffSvc = nullptr;
		Ort::Session* nsfHifigan = nullptr;
		Ort::Session* pred = nullptr;
		Ort::Session* denoise = nullptr;
		Ort::Session* after = nullptr;
		Ort::Session* encoder = nullptr;

		//Singer
		Ort::Session* diffSinger = nullptr;
	};

	class MDropTarget : public IDropTarget {
	public:
		MDropTarget(std::function<void(const std::wstring&)>&& callback) : callback_(callback)
		{
			ref_count_ = 1;
		}

		virtual ~MDropTarget() = default;

		HRESULT __stdcall DragEnter(IDataObject* data_obj, DWORD key_state, POINTL pt, DWORD* effect) override
		{
			*effect = DROPEFFECT_COPY;
			return S_OK;
		}

		HRESULT __stdcall DragOver(DWORD key_state, POINTL pt, DWORD* effect) override
		{
			*effect = DROPEFFECT_COPY;
			return S_OK;
		}

		HRESULT __stdcall DragLeave() override { return S_OK; }

		HRESULT __stdcall Drop(IDataObject* data_obj, DWORD key_state, POINTL pt, DWORD* effect) override
		{
			FORMATETC format = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM medium;

			if (data_obj->GetData(&format, &medium) != S_OK) {
				return E_INVALIDARG;
			}

			std::wstring DragInfos;
			const auto drop = static_cast<HDROP>(GlobalLock(medium.hGlobal));
			if (!drop)
			{
				return E_UNEXPECTED;
			}
			const UINT file_count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);

			for (UINT i = 0; i < file_count; i++) {
				WCHAR DragInfo[MAX_PATH];
				DragQueryFile(drop, i, DragInfo, MAX_PATH);
				DragInfos += DragInfo;
				DragInfos += L'\n';
			}

			GlobalUnlock(medium.hGlobal);
			ReleaseStgMedium(&medium);

			if (callback_) {
				callback_(DragInfos);
			}

			return S_OK;
		}

		// IUnknown Methods
		HRESULT __stdcall QueryInterface(const IID& iid, void** object) override
		{
			if (iid == IID_IUnknown || iid == IID_IDropTarget) {
				*object = this;
				AddRef();
				return S_OK;
			}
			return E_NOINTERFACE;
		}

		ULONG __stdcall AddRef() override { return InterlockedIncrement(&ref_count_); }

		ULONG __stdcall Release() override
		{
			const ULONG count = InterlockedDecrement(&ref_count_);
			if (count == 0) {
				delete this;
			}
			return count;
		}

	private:
		std::function<void(const std::wstring&)> callback_;
		LONG ref_count_;
	};

	class ThreadingOptions
	{
	public:
		OrtThreadingOptions* threadingOptions = nullptr;

		ThreadingOptions()
		{
			const OrtApi& ortApi = Ort::GetApi();

			ortApi.CreateThreadingOptions(&threadingOptions);

			ortApi.SetGlobalIntraOpNumThreads(threadingOptions, 0);
			ortApi.SetGlobalInterOpNumThreads(threadingOptions, 0);
		}
		~ThreadingOptions()
		{
			const OrtApi& ortApi = Ort::GetApi();

			ortApi.ReleaseThreadingOptions(threadingOptions);
		}
	};

	//����������
	extern void MainWindow_CreateLoop();
}


