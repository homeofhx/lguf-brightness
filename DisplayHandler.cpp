#include "DisplayHandler.h"
#include <algorithm>
#include <cmath>
#include <utility>

DisplayHandler::DisplayHandler(libusb_device *device) {
    m_dev = libusb_ref_device(device);

    struct libusb_device_descriptor descriptor{};
    if (libusb_get_device_descriptor(m_dev, &descriptor) < 0) return;

    m_productId = descriptor.idProduct;

    auto it = SUPPORTED_DEVICES.find(m_productId);
    m_name = (it != SUPPORTED_DEVICES.end()) ? it->second : "LG UltraFine Display";

    if (libusb_open(m_dev, &m_handle) < 0) {
        m_handle = nullptr;
        return;
    }

    libusb_set_auto_detach_kernel_driver(m_handle, 1);
    m_claimErr = libusb_claim_interface(m_handle, m_iface);
}

DisplayHandler::DisplayHandler(DisplayHandler &&other) noexcept {
    swap(other);
}

DisplayHandler &DisplayHandler::operator=(DisplayHandler &&other) noexcept {
    if (this != &other) {
        close();
        swap(other);
    }

    return *this;
}

void DisplayHandler::swap(DisplayHandler &o) noexcept {
    std::swap(m_dev, o.m_dev);
    std::swap(m_handle, o.m_handle);
    std::swap(m_claimErr, o.m_claimErr);
    std::swap(m_iface, o.m_iface);
    std::swap(m_productId, o.m_productId);
    std::swap(m_name, o.m_name);
}

void DisplayHandler::close() {
    if (m_handle) {
        if (m_claimErr == LIBUSB_SUCCESS) {
            libusb_release_interface(m_handle, m_iface);
            libusb_attach_kernel_driver(m_handle, m_iface);
        }

        libusb_close(m_handle);
        m_handle = nullptr;
    }

    if (m_dev) {
        libusb_unref_device(m_dev);
        m_dev = nullptr;
    }

    m_claimErr = LIBUSB_ERROR_OTHER;
}

uint16_t DisplayHandler::percentToRaw(int percent) {
    percent = std::clamp(percent, 0, 100);
    return static_cast<uint16_t>(percent * BRIGHT_RANGE / 100);
}

int DisplayHandler::rawToPercent(uint16_t raw) {
    int percent = std::lround((static_cast<float>(raw) / BRIGHT_RANGE) * 100.0f);
    return std::clamp(percent, 0, 100);
}

std::optional<uint16_t> DisplayHandler::getBrightness() {
    if (!isValid()) return std::nullopt;

    uint8_t data[8] = {0};
    int res = libusb_control_transfer(m_handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                      HID_GET_REPORT,
                                      (HID_REPORT_TYPE_FEATURE << 8) | 0x00,
                                      static_cast<uint16_t>(m_iface),
                                      data, sizeof(data),
                                      kUSBTimeoutMs);

    if (res < 0) return std::nullopt;
    return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
}

bool DisplayHandler::setBrightness(uint16_t raw) {
    if (!isValid()) return false;

    uint8_t data[6] = {
        static_cast<uint8_t>(raw & 0x00ff),
        static_cast<uint8_t>((raw >> 8) & 0x00ff),
        0x00, 0x00, 0x00, 0x00
    };

    int res = libusb_control_transfer(m_handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                      HID_SET_REPORT,
                                      (HID_REPORT_TYPE_FEATURE << 8) | 0x00,
                                      static_cast<uint16_t>(m_iface),
                                      data, sizeof(data),
                                      kUSBTimeoutMs);

    return res >= 0;
}

std::optional<int> DisplayHandler::getBrightnessPercent() {
    auto raw = getBrightness();
    if (!raw) return std::nullopt;
    return rawToPercent(*raw);
}

bool DisplayHandler::setBrightnessPercent(int pct) {
    return setBrightness(percentToRaw(pct));
}

DisplayHandler::~DisplayHandler() {
    close();
}