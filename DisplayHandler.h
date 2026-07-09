#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

// libusb
#if __has_include(<libusb.h>)
#  include <libusb.h>
#elif __has_include(<libusb-1.0/libusb.h>)
#  include <libusb-1.0/libusb.h>
#else
#  error "libusb-1.0 not found"
#endif

// Supported displays (Vendor 0x043e = LG Electronics)
inline constexpr uint16_t LG_VENDOR_ID = 0x043e;
inline const std::unordered_map<uint16_t, const char *> SUPPORTED_DEVICES = {
    {0x9a40, "LG UltraFine 5K (27MD5KA)"},
    {0x9a70, "LG UltraFine 5K (27MD5KL)"},
    {0x9a63, "LG UltraFine 4K (24MD4KL)"}
};

// USB-HID report constants
inline constexpr int HID_GET_REPORT = 0x01;
inline constexpr int HID_SET_REPORT = 0x09;
inline constexpr int HID_REPORT_TYPE_FEATURE = 0x03;

class DisplayHandler {
public:
    explicit DisplayHandler(libusb_device *device);
    ~DisplayHandler();

    bool isValid() const { return m_handle != nullptr && m_claimErr == LIBUSB_SUCCESS; }

    bool setBrightness(uint16_t raw);
    bool setBrightnessPercent(int pct);

    std::optional<uint16_t> getBrightness();
    std::optional<int> getBrightnessPercent();

    void close();   // Must be called before destruction to release the kernel driver

    // Non-copyable (holds unique USB handle ownership)
    DisplayHandler(const DisplayHandler &) = delete;
    DisplayHandler &operator=(const DisplayHandler &) = delete;

    // Movable so it can live in a container
    DisplayHandler(DisplayHandler &&other) noexcept;
    DisplayHandler &operator=(DisplayHandler &&other) noexcept;

    std::string displayName() const { return m_name; }
    uint16_t productId() const { return m_productId; }

    static constexpr uint16_t BRIGHT_RANGE = 0xd2f0;

private:
    void swap(DisplayHandler &o) noexcept;

    static uint16_t percentToRaw(int percent);
    static int rawToPercent(uint16_t raw);

    static constexpr unsigned kUSBTimeoutMs = 500;

    int m_claimErr = LIBUSB_ERROR_OTHER;
    int m_iface = 1;
    uint16_t m_productId = 0;

    libusb_device *m_dev = nullptr;
    libusb_device_handle *m_handle = nullptr;

    std::string m_name;
};

#endif