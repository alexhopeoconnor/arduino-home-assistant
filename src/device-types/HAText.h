#ifndef AHA_HATEXT_H
#define AHA_HATEXT_H

#include "HABaseDeviceType.h"
#include "../utils/HANumeric.h"

#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
#include <functional>
#endif

#ifndef EX_ARDUINOHA_TEXT

#define HATEXT_CALLBACK(name) void (*name)(const char* value, HAText* sender)

/**
 * HAText adds a text input in the Home Assistant panel.
 *
 * @note
 * You can find more information about this entity in the Home Assistant documentation:
 * https://www.home-assistant.io/integrations/text.mqtt/
 */
class HAText : public HABaseDeviceType
{
public:
    /// Represents mode of the text input.
    enum Mode {
        ModeText = 0,
        ModePassword
    };

    /**
     * @param uniqueId The unique ID of the text entity. It needs to be unique in a scope of your device.
     */
    HAText(const char* uniqueId);

    /**
     * Changes state of the text and publishes MQTT message.
     * Please note that if a new value is the same as previous one,
     * the MQTT message won't be published.
     *
     * @param state New state of the text.
     * @param force Forces to update state without comparing it to previous known state.
     * @returns Returns `true` if MQTT message has been published successfully.
     */
    bool setState(const char* state, const bool force = false);

    /**
     * Sets current state of the text without publishing it to Home Assistant.
     * This method may be useful if you want to change state before connection
     * with MQTT broker is acquired.
     *
     * @param state New state of the text.
     */
    inline void setCurrentState(const char* state)
        { _currentState = state; }

    /**
     * Returns last known state of the text.
     */
    inline const char* getCurrentState() const
        { return _currentState; }

    /**
     * Sets icon of the text.
     * Any icon from MaterialDesignIcons.com (for example: `mdi:home`).
     *
     * @param icon The icon name.
     */
    inline void setIcon(const char* icon)
        { _icon = icon; }

    /**
     * Sets retain flag for the text command.
     * If set to `true` the command produced by Home Assistant will be retained.
     *
     * @param retain
     */
    inline void setRetain(const bool retain)
        { _retain = retain; }

    /**
     * Sets optimistic flag for the text state.
     * In this mode the text state doesn't need to be reported back to the HA panel when a command is received.
     * By default the optimistic mode is disabled.
     *
     * @param optimistic The optimistic mode (`true` - enabled, `false` - disabled).
     */
    inline void setOptimistic(const bool optimistic)
        { _optimistic = optimistic; }

    /**
     * Sets mode of the text input.
     * By default it's `HAText::ModeText`.
     *
     * @param mode Mode to set.
     */
    inline void setMode(const Mode mode)
        { _mode = mode; }

    /**
     * Sets minimum accepted length of the command payload.
     *
     * @param min Minimum number of characters.
     */
    inline void setMin(const uint16_t min)
        { _minValue = HANumeric(min, PrecisionP0); }

    /**
     * Sets maximum accepted length of the command payload.
     *
     * @param max Maximum number of characters.
     */
    inline void setMax(const uint16_t max)
        { _maxValue = HANumeric(max, PrecisionP0); }

    /**
     * Sets regex pattern for command payload validation in Home Assistant.
     *
     * @param pattern Regex pattern.
     */
    inline void setPattern(const char* pattern)
        { _pattern = pattern; }

    /**
     * Registers callback that will be called each time text command from HA is received.
     * Please note that it's not possible to register multiple callbacks for the same text entity.
     *
     * @param callback
     * @note In non-optimistic mode, the state should be reported back to HA using HAText::setState.
     */
    inline void onCommand(HATEXT_CALLBACK(callback))
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
    inline void onCommand(const std::function<void(const char*, HAText*)>& callback)
    {
        _commandCallback = nullptr;
        _commandStdCallback = callback;
    }
#endif

protected:
    virtual void buildSerializer() override;
    virtual void onMqttConnected() override;
    virtual void onMqttMessage(
        const char* topic,
        const uint8_t* payload,
        const uint16_t length
    ) override;

private:
    /**
     * Publishes the MQTT message with the given state.
     *
     * @param state The state to publish.
     * @returns Returns `true` if the MQTT message has been published successfully.
     */
    bool publishState(const char* state);

    /**
     * Returns progmem string representing mode of the text.
     */
    const __FlashStringHelper* getModeProperty() const;

    /// The icon of the text. It can be nullptr.
    const char* _icon;

    /// The retain flag for the HA commands.
    bool _retain;

    /// The optimistic mode of the text (`true` - enabled, `false` - disabled).
    bool _optimistic;

    /// Controls how the text should be displayed in the UI.
    Mode _mode;

    /// The minimal number of characters accepted by Home Assistant.
    HANumeric _minValue;

    /// The maximum number of characters accepted by Home Assistant.
    HANumeric _maxValue;

    /// Regular expression pattern accepted by Home Assistant. It can be nullptr.
    const char* _pattern;

    /// The current state of the text. It can be nullptr if state wasn't set.
    const char* _currentState;

    /// The callback that will be called when command is received from the HA.
    HATEXT_CALLBACK(_commandCallback);

#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    /// The std::function callback that will be called when command is received from the HA.
    std::function<void(const char*, HAText*)> _commandStdCallback;
#endif
};

#endif
#endif
