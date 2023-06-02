#ifndef TTSMAINWINDOW_H
#define TTSMAINWINDOW_H

#include <QDialog>

#include "ui_ttsmainwindow.h"
#include "../../Modules/Models/header/Vits.hpp"
#include "../../Modules/Models/header/Pits.hpp"
#include "../../Modules/Models/header/Tacotron.hpp"

namespace Ui {
class TTSMainWindow;
}

class TTSMainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TTSMainWindow(const std::function<void(QDialog*)>& _callback, QWidget *parent = nullptr);
    ~TTSMainWindow() override;
    void SetParamsEnabled(bool condition) const;
    void closeEvent(QCloseEvent* e) override;
    void SetInferenceEnabled(bool condition) const;
    void SetCharaMixEnabled(bool condition) const;
    void unloadModel();
    void initSetup();
    void reloadModels();
    void modelsClear();
    void SetModelSelectEnabled(bool condition) const;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    [[nodiscard]] int saveProject(std::string& error);
#ifdef WIN32
    static void openModelFolder();
#endif
    void loadModel(size_t idx);
    void setParamsControlValue(const MoeVSProject::TTSParams& _params);
    [[nodiscard]] MoeVSProject::TTSParams dumpPatamsFromControl() const;

public slots:
    void on_NoiseScaleSlider_valueChanged(int value) const;
    void on_NoiseScaleWSlider_valueChanged(int value) const;
    void on_LengthScaleSlider_valueChanged(int value) const;
    void on_GateThSlider_valueChanged(int value) const;
    void on_MaxDecodeStepSlider_valueChanged(int value) const;
    void on_NoiseScaleSpinBox_valueChanged(double value) const;
    void on_NoiseScaleWSpinBox_valueChanged(double value) const;
    void on_LengthScaleSpinBox_valueChanged(double value) const;
    void on_GateThSpinBox_valueChanged(double value) const;
    void on_MaxDecodeStepSpinBox_valueChanged(int value) const;
    void on_ModelListRefreshButton_clicked();
    void on_LoadModelButton_clicked();
    void on_InferenceButton_clicked();
    void on_AddTextButton_clicked();
    void on_RemoveTextButton_clicked();
    void on_SaveTextButton_clicked();
    void on_EditTextButton_clicked();
    void on_TextCleaner_clicked();
    void on_ClearTextListButton_clicked();
    void on_TTSInferenceCleanerButton_clicked();
    void on_TTSInferOnceButton_clicked();
    void on_BatchedTTSButton_clicked();
    void on_TTSInferCur_clicked();
    void on_ClearListButton_clicked();
    void on_DrawMelSpecButton_clicked();
    void on_SendToPlayerButton_clicked();
    void on_DeleteSelectedAudioButton_clicked() const;

    void on_CharacterMixProportion_valueChanged(double value);
    void on_CharacterComboBox_currentIndexChanged(int value) const;
    void on_TextListWidget_itemSelectionChanged() const;

#ifdef WIN32
    static void on_ModelListDirButton_clicked();
#endif
private:
    Ui::TTSMainWindow *ui;
    const std::function<void(QDialog*)>& exit_callback;
    InferClass::TTS* _model = nullptr;
    MoeVSProject::TTSProject _proj;
    std::vector<rapidjson::Document> _models;
    std::vector<float> charaMixData;
    std::vector<std::wstring> waveFolder;
    int64_t n_speakers = 0;
    size_t cur_model_index = 0;
    int64_t samplingRate = 22050;
    InferClass::BaseModelType::Device __MOESS_DEVICE = InferClass::BaseModelType::Device::CPU;
public:
    InferClass::BaseModelType::callback BarCallback = [&](size_t _cur, size_t _max) -> void
    {
        if (!_cur)
            ui->InferProgressBar->setMaximum((int)_max);
        ui->InferProgressBar->setValue((int)_cur);
    };

    InferClass::BaseModelType::callback_params TTSParamCallback = [&]()
    {
        InferClass::InferConfigs _conf;
        _conf.noise_scale = (float)ui->NoiseScaleSpinBox->value();
        _conf.noise_scale_w = (float)ui->NoiseScaleWSpinBox->value();
        _conf.length_scale = (float)ui->LengthScaleSpinBox->value();
        _conf.seed = 52608;
        _conf.chara = ui->CharacterComboBox->currentIndex();
        if (_conf.chara < 0)
            _conf.chara = 0;
        _conf.emo = ui->EmotionEdit->text().toStdWString();
        _conf.gateThreshold = (float)ui->GateThSpinBox->value();
        _conf.maxDecoderSteps = ui->MaxDecodeStepSpinBox->value();
        _conf.cp = false;
        return _conf;
    };
};

#endif // TTSMAINWINDOW_H
