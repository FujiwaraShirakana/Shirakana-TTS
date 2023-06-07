#pragma once
#include <QPainter>
#include "ttseditorblock.h"
class MTTSEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MTTSEditor(QWidget* parent = nullptr);
    ~MTTSEditor() override = default;
    void paintEvent(QPaintEvent* event) override;
private:
    static constexpr double BeginPointPos[9] = { 0.0,0.125,0.25,0.375,0.5,0.625,0.75,0.875,1.0 };
    int pixPerFrame = 5;
    double scaleMultiple = 1.0;
    int BlockHeight = 200;
    int pageFrame = 140;
    std::vector<TTSEditorBlock*> _blocks;
    std::vector<int64_t> _durations;
    std::vector<int64_t> _tones;
};