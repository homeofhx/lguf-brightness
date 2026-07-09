#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QShowEvent>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->rescanButton, &QPushButton::clicked, this, &MainWindow::onRescan);

    m_usbReady = initUSB();
    scanDisplays();

    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(5000);
    connect(m_syncTimer, &QTimer::timeout, this, &MainWindow::onSyncTimer);
    m_syncTimer->start();
}

bool MainWindow::initUSB() {
    if (m_usbCtx) { return true; }

    int r = libusb_init(&m_usbCtx);
    if (r < 0) {
        showStatus(QString("Failed to initialise libusb (error %1)").arg(r));
        return false;
    }

    return true;
}

void MainWindow::showStatus(const QString &msg) {
    ui->statusLabel->setText(msg);
    ui->statusLabel->setVisible(true);
    adjustSize();
}

void MainWindow::scanDisplays() {
    freeDisplays();

    if (!m_usbReady) { ui->statusLabel->setVisible(true); adjustSize(); return; }

    libusb_device **devList = nullptr;
    int cnt = libusb_get_device_list(m_usbCtx, &devList);
    if (cnt < 0) {
        showStatus("Failed to enumerate USB devices");
        return;
    }

    for (int i = 0; i < cnt; ++i) {
        struct libusb_device_descriptor desc{};

        if (libusb_get_device_descriptor(devList[i], &desc) < 0) continue;
        if (desc.idVendor != LG_VENDOR_ID) continue;
        if (SUPPORTED_DEVICES.find(desc.idProduct) == SUPPORTED_DEVICES.end()) continue;

        m_displays.emplace_back(devList[i]);
    }

    libusb_free_device_list(devList, 1);

    if (m_displays.empty()) {
        showStatus("No LG UltraFine display(s) detected!");
        return;
    }

    ui->statusLabel->setVisible(false);

    for (int i = 0; i < static_cast<int>(m_displays.size()); ++i) {
        if (!m_displays[i].isValid()) continue;
        DisplayWidget *dw = new DisplayWidget(m_displays[i], i, ui->scrollContents);
        m_displayWidgets.push_back(dw);
        ui->displaysLayout->insertWidget(ui->displaysLayout->count() - 1, dw);
    }

    if (m_displayWidgets.empty()) {
        showStatus("Found LG UltraFine display(s), but couldn't claim the USB interface");
        return;
    }

    adjustSize();
}

void MainWindow::freeDisplays() {
    for (DisplayWidget *dw : m_displayWidgets) {
        ui->displaysLayout->removeWidget(dw);
        delete dw;
    }

    m_displayWidgets.clear();
    m_displays.clear();
}

void MainWindow::onSyncTimer() {
    for (DisplayWidget *dw : m_displayWidgets) { dw->syncFromDisplay(); }
}

void MainWindow::onRescan() {
    m_usbReady = initUSB();
    scanDisplays();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    freeDisplays();
    m_syncTimer->stop();
    event->accept();
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    for (DisplayWidget *dw : m_displayWidgets) dw->syncFromDisplay();
}

MainWindow::~MainWindow() {
    freeDisplays();
    if (m_usbCtx) { libusb_exit(m_usbCtx); m_usbCtx = nullptr; }
    delete ui;
}