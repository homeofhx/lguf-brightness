#include "DisplayWidget.h"
#include "ui_DisplayWidget.h"
#include <QPushButton>
#include <array>
#include <utility>

namespace { constexpr int kMinBrightnessPercent = 1; }

DisplayWidget::DisplayWidget(DisplayHandler &display, int index, QWidget *parent): QGroupBox(parent)
    , m_index(index)
    , ui(new Ui::DisplayWidget)
    , m_display(display) {
    ui->setupUi(this);
    setTitle(QString::fromStdString(display.displayName()));

    const std::array<std::pair<QPushButton *, int>, 5> presets{{
        {ui->btnMin, kMinBrightnessPercent},
        {ui->btn25, 25},
        {ui->btn50, 50},
        {ui->btn75, 75},
        {ui->btn100, 100},
    }};

    for (const auto &preset : presets) {
        QPushButton *button = preset.first;
        int percent = preset.second;
        connect(button, &QPushButton::clicked, this, [this, percent] { onPresetClicked(percent); });
    }

    connect(ui->brightnessSlider, &QSlider::sliderMoved, this, &DisplayWidget::onSliderMoved);
    syncFromDisplay();
}

void DisplayWidget::updateSlider(int percent) {
    ui->brightnessSlider->blockSignals(true);
    ui->brightnessSlider->setValue(percent);
    ui->brightnessSlider->blockSignals(false);
}

void DisplayWidget::updateBrightnessLabel(int percent) {
    ui->valueLabel->setText(percent >= 0 ? QString("%1%").arg(percent) : QStringLiteral("--%"));
}

void DisplayWidget::syncFromDisplay() {
    auto percent = m_display.getBrightnessPercent();
    if (!percent) {
        updateBrightnessLabel(-1);
        return;
    }

    updateSlider(*percent);
    updateBrightnessLabel(*percent);
}

void DisplayWidget::onSliderMoved(int value) {
    if (!m_display.setBrightnessPercent(value)) {
        syncFromDisplay();
        return;
    }

    updateBrightnessLabel(value);
    emit brightnessChanged(m_index, value);
}

void DisplayWidget::onPresetClicked(int percent) {
    percent = qMax(kMinBrightnessPercent, percent);

    if (!m_display.setBrightnessPercent(percent)) {
        syncFromDisplay();
        return;
    }

    updateSlider(percent);
    updateBrightnessLabel(percent);
    emit brightnessChanged(m_index, percent);
}

DisplayWidget::~DisplayWidget() { delete ui; }