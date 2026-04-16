#ifndef AHA_HASCENE_H
#define AHA_HASCENE_H

#include "HABaseDeviceType.h"

#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
#include <functional>
#endif

#ifndef EX_ARDUINOHA_SCENE

#define HASCENE_CALLBACK(name) void (*name)(HAScene* sender)

/**
 * HAScene adds a new scene to the Home Assistant that triggers your callback once activated.
 *
 * @note
 * You can find more information about this entity in the Home Assistant documentation:
 * https://www.home-assistant.io/integrations/scene.mqtt/
 */
class HAScene : public HABaseDeviceType
{
public:
    /**
     * @param uniqueId The unique ID of the scene. It needs to be unique in a scope of your device.
     */
    HAScene(const char* uniqueId);

    /**
     * Sets icon of the scene.
     * Any icon from MaterialDesignIcons.com (for example: `mdi:home`).
     *
     * @param icon The icon name.
     */
    inline void setIcon(const char* icon)
        { _icon = icon; }

    /**
     * Sets retain flag for the scene's command.
     * If set to `true` the command produced by Home Assistant will be retained.
     *
     * @param retain
     */
    inline void setRetain(const bool retain)
        { _retain = retain; }

    /**
     * Registers callback that will be called when the scene is activated in the HA panel.
     * Please note that it's not possible to register multiple callbacks for the same scene.
     *
     * @param callback
     */
    inline void onCommand(HASCENE_CALLBACK(callback))
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
    inline void onCommand(const std::function<void(HAScene*)>& callback)
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
    /// The icon of the scene. It can be nullptr.
    const char* _icon;

    /// The retain flag for the HA commands.
    bool _retain;

    /// The command callback that will be called when scene is activated from the HA panel.
    HASCENE_CALLBACK(_commandCallback);

#if defined(ARDUINOHA_ENABLE_STDFUNCTION)
    /// The std::function callback that will be called when scene is activated from the HA panel.
    std::function<void(HAScene*)> _commandStdCallback;
#endif
};

#endif
#endif
