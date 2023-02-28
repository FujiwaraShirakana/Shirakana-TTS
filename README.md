觉得现在存档还是为时过早，至少等语音合成的几个项目停更再说（）
Discussions我开了，如果有什么问题可以在里面提，等其他大佬的回答就可以了，issue请发布确定为BUG的反馈或对于期望加入的功能的反馈。

---

# 一些已经做好的前置模型：
停止更新（由于下载和上传速度）: [Vocoder & HiddenUnitBert](https://github.com/NaruseMioShirakana/RequireMent-Model-For-MoeSS) 

最新仓库地址 : [HuggingFace](https://huggingface.co/NaruseMioShirakana/MoeSS-SUBModel) 

自己导出前置：
- HuBert：input_names应该为["source"]，output_names应该为["embed"]，dynamic_axes应当为{"source":[0,2],}
- Diffusion模型使用的hifigan：input_names应该为["c","f0"]，output_names应该为["audio"]，dynamic_axes应当为{"c":[0,1],"f0":[0,1],}
- Tacotron2使用的hifigan：input_names应该为["x"]，output_names应该为["audio"]，dynamic_axes应当为{"x":[0,1],}
---
# 发布/获取模型：
[Maple的主题站](https://winmoes.com/voicepak)

---
# 用户协议：
- 引用该项目请注明该项目仓库。该项目暂时无法编译（由于使用到的界面库未开源）

- 使用本项目进行二创时请标注本项目仓库地址或作者bilibili空间地址：https://space.bilibili.com/108592413

## 使用该项目代表你同意如下几点：
- 1、你愿意自行承担由于使用该项目而造成的一切后果。
- 2、你承诺不会出售该程序以及其附属模型，若由于出售而造成的一切后果由你自己承担。
- 3、你不会使用之从事违法活动，若从事违法活动，造成的一切后果由你自己承担。
- 4、禁止用于任何商业游戏、低创游戏以及Galgame制作，不反对无偿的精品游戏制作以及Mod制作。
- 5、禁止使用该项目及该项目衍生物以及发布模型等制作各种电子垃圾（比方说AIGalgame，AI游戏制作等）
---

## Q&A：
### Q：该项目以后会收费吗？
    A：该项目永久开源免费，如果在其他地方存在本软件的收费版本，请立即举报且不要购买，本软件永久免费。如果想用疯狂星期四塞满白叶，可以前往爱发癫 https://afdian.net/a/NaruseMioShirakana 
### Q：是否提供有偿模型代训练？
    A：原则上不提供，训练TTS模型比较简单，没必要花冤枉钱，按照网上教程一步一步走就可以了。提供免费的Onnx转换。
### Q：电子垃圾评判标准是什么？
    A：1、原创度。自己的东西在整个项目中的比例（对于AI来说，使用完全由你独立训练模型的创作属于你自己；使用他人模型的创作属于别人）。涵盖的方面包括但不限于程序、美工、音频、策划等等。举个例子，套用Unity等引擎模板换皮属于电子垃圾。

    2、开发者态度。作者开发的态度是不是捞一波流量和钱走人或单纯虚荣。比方说打了无数的tag，像什么“国产”“首个”“最强”“自制”这种引流宣传，结果是非常烂或是平庸的东西，且作者明显没有好好制作该项目的想法，属于电子垃圾。
    
    3、反对一切使用未授权的数据集训练出来的AI模型商用的行为。 
### Q：技术支持？
    A：如果能够确定你做的不是电子垃圾，我会提供一些力所能及的技术支持。 
### 作者的吐槽
    以上均为君子协议，要真要做我也拦不住，但还是希望大家自觉，有这个想法的也希望乘早改悔罢
---

# Moe Speech Synthesis
一个基于各种开源TTS、VC以及SVS项目的完全C++Speech Synthesis UI软件

支持的项目的仓库：
- [DeepLearningExamples](https://github.com/NVIDIA/DeepLearningExamples)
- [VITS](https://github.com/jaywalnut310/vits)
- [SoVits](https://github.com/innnky/so-vits-svc/tree/32k)
- [DiffSvc](https://github.com/prophesier/diff-SVC)
- [DiffSinger](https://github.com/openvpi/DiffSinger)

使用的图像素材来源于：
- [SummerPockets](http://key.visualarts.gr.jp/summer/)

目前仅支持Windows，未来可能移植Android，并为Linux用户提供软件，暂无开发Mac与Ios的计划。

---
## 使用方法：
    1、在release中下载软件压缩包，解压之

    2、在上文 [Vocoder & HiddenUnitBert] 仓库中下载相应的前置模型或附加模块，并放置到相应文件夹，前置模型与项目的对应关系会在下文提到

    3、将模型放置在Mods文件夹中，在左上方模型选择模块中选择模型，标准模型结构请查阅下文“支持的项目”

    4、在下方输入框中输入要转换的文字，点击“启用插件”可以执行文本Cleaner，换行为批量转换的分句符号（SoVits/DiffSvc需要输入音频路径，DiffSinger需要输入ds或json项目文件的路径）

    5、点击开始合成，即可开始合成语音，等待进度完成后，可以在右上方播放器预览，也可以在右上方直接保存

    6、可以使用命令行启动：（仅1.X版本）
    Shell：& '.\xxx.exe' "ModDir" "InputText." "outputDir" "Symbol"
    CMD："xxx.exe" "ModDir" "InputText." "outputDir" "Symbol"
    其中ModDir为"模型路径\\模型名" 如预置模型的"Mods\\Shiroha\\Shiroha"
    InputText为需要转换的文字（仅支持空格逗号句号以及字母）
    outputDir为输出文件名（不是路径，是文件名，不需要加后缀）
    Symbol见下文
    输出文件默认在tmpDir中
---
## 模型制作：
- 本软件标准化了模型读取模块，模型保存在Mods文件夹下的子文件夹中********.json文件用于声明模型路径以及其显示名称，需要将模型转换为Onnx，转换的仓库在我GitHub主页Pin了出来。

### 通用参数(不管是啥模型都必须填的，不填就不识别)：
- Folder：保存模型的文件夹名
- Name：模型在UI中的显示名称
- Type：模型类别
- Rate：采样率（必须和你训练时候的一模一样，不明白原因建议去学计算机音频相关的知识）
### Tacotron2：
```jsonc
{
    "Folder" : "Atri",
    "Name" : "亚托莉-Tacotron2",
    "Type" : "Tacotron2",
    "Rate" : 22050,
    "Symbol" : "_-!'(),.:;? ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
    "Cleaner" : "JapaneseCleaner",
    "Hifigan": "hifigan"
}
//Symbol：模型的Symbol，不知道Symbol是啥的建议多看几个视频了解了解TTS的基础知识，这一项在Tacotron2中必须填。
//Cleaner：插件名，可以不填，填了就必须要在Cleaner文件夹防止相应的CleanerDll，如果Dll不存在或者是Dll内部有问题，则会在加载模型时报插件错误
//Hifigan：Hifigan模型名，必须填且必须将在前置模型中下载到的hifigan放置到hifigan文件夹
```
### Vits：
```jsonc
{
    "Folder" : "SummerPockets",
    "Name" : "SummerPocketsReflectionBlue",
    "Type" : "Vits",
    "Rate" : 22050,
    "Symbol" : "_,.!?-~…AEINOQUabdefghijkmnoprstuvwyzʃʧʦ↓↑ ",
    "Cleaner" : "JapaneseCleaner",
    "Characters" : ["鳴瀬しろは","空門蒼","鷹原うみ","紬ヴェンダース","神山識","水織静久","野村美希","久島鴎","岬鏡子"]
}
//Symbol：模型的Symbol，不知道Symbol是啥的建议多看几个视频了解了解TTS的基础知识，这一项在Vits中必须填。
//Cleaner：插件名，可以不填，填了就必须要在Cleaner文件夹防止相应的CleanerDll，如果Dll不存在或者是Dll内部有问题，则会在加载模型时报插件错误
//Characters：如果是多角色模型必须填写为你的角色名称组成的列表，如果是单角色模型可以不填
```
### SoVits：
```jsonc
{
    "Folder" : "NyaruTaffySo",
    "Name" : "NyaruTaffy-SoVits",
    "Type" : "SoVits",
    "Rate" : 32000,
    "Hop" : 320,
    "Cleaner" : "",
    "Hubert": "hubert",
    "SoVits3": true,
    "SoVits4": false,
    "Characters" : ["Taffy","Nyaru"]
}
//Hop：模型的HopLength，不知道HopLength是啥的建议多看几个视频了解了解音频的基础知识，这一项在SoVits中必须填。（数值必须为你训练时的数值，可以在你训练模型时候的配置文件里看到）
//Cleaner：插件名，可以不填，填了就必须要在Cleaner文件夹防止相应的CleanerDll，如果Dll不存在或者是Dll内部有问题，则会在加载模型时报插件错误
//Hubert：Hubert模型名，必须填且必须将在前置模型中下载到的Hubert放置到Hubert文件夹
//SoVits3：是否为SoVits3.0的，如果不是SoVits3.0统一False
//SoVits4：是否为SoVits4.0的，如果不是SoVits4.0统一False
//Characters：如果是多角色模型必须填写为你的角色名称组成的列表，如果是单角色模型可以不填
```
### DiffSVC：
```jsonc
{
    "Folder" : "DiffShiroha",
    "Name" : "白羽",
    "Type" : "DiffSvc",
    "Rate" : 44100,
    "Hop" : 512,
    "MelBins" : 128,
    "Cleaner" : "",
    "Hifigan": "nsf_hifigan",
    "Hubert": "hubert",
    "Characters" : [],
    "Pndm" : 100,
    "V2" : true
}
//Hop：模型的HopLength，不知道HopLength是啥的建议多看几个视频了解了解音频的基础知识，这一项在SoVits中必须填。（数值必须为你训练时的数值，可以在你训练模型时候的配置文件里看到）
//MelBins：模型的MelBins，不知道MelBins是啥的建议多看几个视频了解了解梅尔基础知识，这一项在SoVits中必须填。（数值必须为你训练时的数值，可以在你训练模型时候的配置文件里看到）
//Cleaner：插件名，可以不填，填了就必须要在Cleaner文件夹防止相应的CleanerDll，如果Dll不存在或者是Dll内部有问题，则会在加载模型时报插件错误
//Hubert：Hubert模型名，必须填且必须将在前置模型中下载到的Hubert放置到Hubert文件夹
//Hifigan：Hifigan模型名，必须填且必须将在前置模型中下载到的nsf_hifigan放置到hifigan文件夹
//Characters：如果是多角色模型必须填写为你的角色名称组成的列表，如果是单角色模型可以不填
//Pndm：加速倍数，如果是V1模型则必填且必须为导出时设置的加速倍率
//V2：是否为V2模型，V2模型就是后来我分4个模块导出的那个
```
### DiffSinger：
```jsonc
{
    "Folder" : "utagoe",
    "Name" : "utagoe",
    "Type" : "DiffSinger",
    "Rate" : 44100,
    "Hop" : 512,
    "Cleaner" : "",
    "Hifigan": "singer_nsf_hifigan",
    "Characters" : [],
    "MelBins" : 128
}
//Hop：模型的HopLength，不知道HopLength是啥的建议多看几个视频了解了解音频的基础知识，这一项在SoVits中必须填。（数值必须为你训练时的数值，可以在你训练模型时候的配置文件里看到）
//Cleaner：插件名，可以不填，填了就必须要在Cleaner文件夹防止相应的CleanerDll，如果Dll不存在或者是Dll内部有问题，则会在加载模型时报插件错误
//Hifigan：Hifigan模型名，必须填且必须将在前置模型中下载到的singer_nsf_hifigan放置到hifigan文件夹
//Characters：如果是多角色模型必须填写为你的角色名称组成的列表，如果是单角色模型可以不填
//MelBins：模型的MelBins，不知道MelBins是啥的建议多看几个视频了解了解梅尔基础知识，这一项在SoVits中必须填。（数值必须为你训练时的数值，可以在你训练模型时候的配置文件里看到）
```

---
## 支持的model项目
```cxx 
// ${xxx}是什么意思大家应该都知道吧，总之以下是多个不同项目需要的模型文件（需要放置在对应的模型文件夹下）。
// Tacotron2：
    ${Folder}_decoder_iter.onnx
    ${Folder}_encoder.onnx
    ${Folder}_postnet.onnx
// Vits:    单角色VITS
    ${Folder}_dec.onnx
    ${Folder}_flow.onnx
    ${Folder}_enc_p.onnx
    ${Folder}_dp.onnx 
// Vits:   多角色VITS
    ${Folder}_dec.onnx
    ${Folder}_emb.onnx
    ${Folder}_flow.onnx
    ${Folder}_enc_p.onnx
    ${Folder}_dp.onnx
// SoVits:
    ${Folder}_SoVits.onnx
// DiffSvc:
    ${Folder}_diffSvc.onnx
// DiffSvc: V2
    ${Folder}_encoder.onnx
    ${Folder}_denoise.onnx
    ${Folder}_pred.onnx
    ${Folder}_after.onnx
// DiffSinger: OpenVpiVersion
    ${Folder}_diffSinger.onnx
// DiffSinger: 
    ${Folder}_encoder.onnx
    ${Folder}_denoise.onnx
    ${Folder}_pred.onnx
    ${Folder}_after.onnx
```
---
## Symbol的设置
    例如：_-!'(),.:;? ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
    打开你训练模型的项目，打开text\symbol.py，如图按照划线的List顺序将上面的4个字符串连接即可
![image](https://user-images.githubusercontent.com/40709280/183290732-dcb93323-1061-431b-aafa-c285a3ec5e82.png)

---
## Cleaner的设置
```cxx
/*
Cleaner请放置于根目录的Cleaners文件夹内，应该是一个按照要求定义的动态库（.dll），dll应当命名为Cleaner名，Cleaner名即为模型定义Json文件中Cleaner一栏填写的内容。
所有的插件dll需要定义以下函数，函数名必须为PluginMain，Dll名必须为插件名（或Cleaner名）：
*/
const wchar_t* PluginMain(const wchar_t*);
// 该接口只要求输入输出一致，并不要求功能一致，也就是说，你可以在改Dll中实现任何想要的功能，比方说ChatGpt，机器翻译等等。
// 以ChatGpt为例，PluginMain函数传入了一个输入字符串input，将该输入传入ChatGpt，再将ChatGpt的输出传入PluginMain，最后返回输出。
wchar_t* PluginMain(wchar_t* input){
    wchar_t* tmpOutput = ChatGpt(input);
    return Clean(tmpOutput);
}
// 注意：导出dll时请使用 extern "C" 关键字来防止C++语言的破坏性命名。
```

## 杂项
- [已经制作好的模型](https://github.com/FujiwaraShirakana/ShirakanaTTSMods)
- [演示视频](https://www.bilibili.com/video/BV1bD4y1V7zu)

## 依赖列表
- [FFmpeg](https://ffmpeg.org/)
- [World](https://github.com/JeremyCCHsu/Python-Wrapper-for-World-Vocoder)
- [rapidJson](https://github.com/Tencent/rapidjson)
