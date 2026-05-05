#ifndef AHA_HABUTTON_H
#define AHA_HABUTTON_H

#include "HABaseDeviceType.h"

#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
#include <functional>
#endif

#ifndef EX_ARDUINOHA_BUTTON

#define HABUTTON_CALLBACK(name) void (*name)(HAButton* sender)

/**
 * HAButton represents a button that's displayed in the Home Assistant panel and
 * triggers some logic on your Arduino/ESP device once clicked.
 *
 * @note
 * You can find more information about this entity in the Home Assistant documentation:
 * https://www.home-assistant.io/integrations/button.mqtt/
 */
class HAButton : public HABaseDeviceType
{
public:
    /**
     * @param uniqueId The unique ID of the button. It needs to be unique in a scope of your device.
     */
    HAButton(const char* uniqueId);

    /**
     * Sets class of the device.
     * You can find list of available values here: https://www.home-assistant.io/integrations/button/#device-class
     *
     * @param deviceClass The class name.
     */
    inline void setDeviceClass(const char* deviceClass)
        { _class = deviceClass; }

    /**
     * Sets icon of the button.
     * Any icon from MaterialDesignIcons.com (for example: `mdi:home`).
     *
     * @param icon The icon name.
     */
    inline void setIcon(const char* icon)
        { _icon = icon; }

    /**
     * Sets retain flag for the button's command.
     * If set to `true` the command produced by Home Assistant will be retained.
     *
     * @param retain
     */
    inline void setRetain(const bool retain)
        { _retain = retain; }

    void setPayloadPress(const char* payload);
    void setCommandTemplate(const char* commandTemplate);

    /**
     * Registers callback that will be called each time the press command from HA is received.
     * Please note that it's not possible to register multiple callbacks for the same button.
     *
     * @param callback
     */
    inline void onCommand(HABUTTON_CALLBACK(callback))
    {
        _commandCallback = callback;
#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
        _commandStdCallback = nullptr;
#endif
    }

#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    /**
     * Registers callback using std::function.
     * It allows passing capturing lambdas and std::bind expressions.
     *
     * @param callback
     */
    inline void onCommand(const std::function<void(HAButton*)>& callback)
    {
        _commandCallback = nullptr;
        _commandStdCallback = callback;
    }
#endif

protected:
    virtual void buildSerializer() override;
    virtual HASerializer* buildDeviceDiscoverySerializer() override;
    virtual bool supportsDeviceDiscovery() const override
        { return true; }
    virtual void onMqttConnected() override;
    virtual void onMqttMessage(
        const char* topic,
        const uint8_t* payload,
        const uint16_t length
    ) override;

private:
    /// The device class. It can be nullptr.
    const char* _class;

    /// The icon of the button. It can be nullptr.
    const char* _icon;

    /// The retain flag for the HA commands.
    bool _retain;

    const char* _payloadPress;
    const char* _commandTemplate;

    /// The command callback that will be called once clicking the button in HA panel.
    HABUTTON_CALLBACK(_commandCallback);

#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    /// The std::function command callback.
    std::function<void(HAButton*)> _commandStdCallback;
#endif
};

#endif
#endif
