#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H
#include <QGroupBox>
#include "DisplayHandler.h"

namespace Ui { class DisplayWidget; }

class DisplayWidget : public QGroupBox {
    Q_OBJECT

public:
    explicit DisplayWidget(DisplayHandler &display, int index, QWidget *parent = nullptr);
    ~DisplayWidget() override;

    void syncFromDisplay();

signals:
    void brightnessChanged(int displayIndex, int percent);

private slots:
    void onSliderMoved(int value);
    void onPresetClicked(int percent);

private:
    void updateSlider(int percent);
    void updateBrightnessLabel(int percent);

    int m_index;

    Ui::DisplayWidget *ui;

    DisplayHandler &m_display;
};

#endif