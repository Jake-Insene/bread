#include "platform/platform_header.h"

#include "debug/debug.h"
#include "debug/fail.h"
#include "engine/engine.h"
#include "log/log.h"
#include "platform/android/android_display.h"
#include "platform/android/android_engine.h"
#include "platform/android/android_mapped_keycodes.h"


struct SaveState
{
    bool saved = false;
};

SaveState save_state = {};

bool running = true;
bool initialized = false;

static int32_t engine_handle_input(android_app*, AInputEvent* event)
{
    i32 type = AInputEvent_getType(event);
    i32 source = AInputEvent_getSource(event);
    switch(type)
    {
    case AINPUT_EVENT_TYPE_MOTION:
    {
        if (source == AINPUT_SOURCE_TOUCHSCREEN)
        {
            i32 action_pointer = AMotionEvent_getAction(event);
            i32 action = action_pointer & AMOTION_EVENT_ACTION_MASK;
            usize pointer_count = AMotionEvent_getPointerCount(event);
            for (usize p = 0; p < pointer_count; p++)
            {
                InputEventTouch e = {};

                // Y positive is up
                // TODO: Fix coordinates.
                e.type = EventType::Touch;
                e.position = Vector2
                        (
                                AMotionEvent_getX(event, p),
                                AMotionEvent_getY(event, p)
                        );
                e.pressed = (action == AMOTION_EVENT_ACTION_DOWN
                             || action == AMOTION_EVENT_ACTION_MOVE
                             || action == AMOTION_EVENT_ACTION_POINTER_DOWN);
                e.pointer = i32(p);

                Engine::local_data.engine_runtime->handle_event(e);

                Log::debug("action pointer: {}, action: {}, pointer: {}", action_pointer, action, p);
            }
        }
    }
        return 1;
    case AINPUT_EVENT_TYPE_KEY:
    {
        i32 action = AKeyEvent_getAction(event);
        i32 keycode = AKeyEvent_getKeyCode(event);

		InputEventKey event = {};
        event.type = EventType::KeyPress;
		event.pressed = action == AKEY_EVENT_ACTION_DOWN;
		event.key = MappedKeycodes[keycode];

        if(action == AKEY_EVENT_ACTION_DOWN)
        {
            Input::data.keys[i32(MappedKeycodes[keycode])] = KeyState::Pressed;
        }
        else
        {
            Input::data.keys[i32(MappedKeycodes[keycode])] = KeyState::Released;
        }
    }
    default:
        break;
    }
    return 0;
}

static void engine_handle_cmd(android_app*, int32_t cmd)
{
    switch (cmd) {
    case APP_CMD_SAVE_STATE:
        Log::info("Saving state...");
        break;
    case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        if (AndroidEngine::data.app->window != nullptr)
        {
            Engine::local_data.engine_runtime->initialize();
            Engine::local_data.engine_runtime->request_recreate_window();
            initialized = true;
    	}
        break;
    case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        running = false;
        break;
    case APP_CMD_GAINED_FOCUS:
        running = true;
        break;
    case APP_CMD_LOST_FOCUS:
        running = false;
        break;
    default:
        break;
    }
}

AndroidEngine engine = {};

void android_loop(android_app* app)
{
    Engine::local_data.engine_runtime = &engine;

    AndroidEngine::data.app = app;
    AndroidEngine::data.asset_manager = app->activity->assetManager;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    AConfiguration_setOrientation(app->config, ACONFIGURATION_ORIENTATION_LAND);

    StringView internal_data_path = StringView(
            app->activity->internalDataPath,
            __string_len(app->activity->internalDataPath)
    );
    Log::debug("Internal data path: {}", internal_data_path);

    StringView external_data_path = StringView(
            app->activity->externalDataPath,
            __string_len(app->activity->externalDataPath)
    );
    Log::debug("External data path: {}", external_data_path);

    StringView obb_path = StringView(
            app->activity->obbPath,
            __string_len(app->activity->obbPath)
    );
    Log::debug("Obb path: {}", obb_path);

    while (!app->destroyRequested) {
        android_poll_source *source = nullptr;
        auto result = ALooper_pollOnce(0, nullptr, nullptr, reinterpret_cast<void **>(&source));
        if (result == ALOOPER_POLL_ERROR)
        {
            Fatal("ALooper_pollOnce returned an error");
        }

        if (source != nullptr)
        {
            source->process(app, source);
        }

        if (AndroidEngine::data.app->destroyRequested)
        {
            break;
        }
        
        if(running && initialized)
        {
            engine.step();
        }
    }

    engine.shutdown();
}

void android_main(android_app* app)
{
    android_loop(app);
}

