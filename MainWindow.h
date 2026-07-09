#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTimer>
#include <deque>
#include <vector>
#include "DisplayHandler.h"
#include "DisplayWidget.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onSyncTimer();
    void onRescan();

private:
    bool initUSB();
    void scanDisplays();
    void freeDisplays();
    void showStatus(const QString &msg);

    bool m_usbReady = false;

    Ui::MainWindow *ui = nullptr;
    libusb_context *m_usbCtx = nullptr;
    QTimer *m_syncTimer = nullptr;   // Periodic hardware sync

    std::deque<DisplayHandler> m_displays;
    std::vector<DisplayWidget *> m_displayWidgets;
};

#endif