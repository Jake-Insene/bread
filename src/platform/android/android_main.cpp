#include "debug/debug.h"
#include "scene/scene_manager.h"
#include "platform/android/android_engine.h"
#include "platform/android/android_mapped_keycodes.h"


struct SaveState
{
    bool saved = false;
};
SaveState save_state = {};

bool running = true;


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
                e.type = INPUT_EVENT_TOUCH;
                e.position = Vector2
                        (
                                AMotionEvent_getX(event, p),
                                -AMotionEvent_getY(event, p)
                        );
                e.pressed = (action == AMOTION_EVENT_ACTION_DOWN
                             || action == AMOTION_EVENT_ACTION_MOVE
                             || action == AMOTION_EVENT_ACTION_POINTER_DOWN);
                e.pointer = (i32) p;

                Engine::handle_input(e);

                DebugInfo("action pointer: {i}, action: {i}, pointer: {u}", action_pointer, action, p);
            }
        }
    }
        return 1;
    case AINPUT_EVENT_TYPE_KEY:
    {
        i32 action = AKeyEvent_getAction(event);
        i32 keycode = AKeyEvent_getKeyCode(event);
        Input::data.keys[(i32)MappedKeycodes[keycode]] = action == AKEY_EVENT_ACTION_DOWN;
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
        //save_state.last_scene = SceneManager::current_scene->klass;
        break;
    case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        if (AndroidEngine::data.app->window != nullptr)
        {
            Engine::recreate_window();
    	}
        break;
    case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        Engine::destroy();
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

void android_main(android_app* app)
{
    AndroidEngine::data.app = app;
    AndroidEngine::data.asset_manager = app->activity->assetManager;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    AConfiguration_setOrientation(app->config, ACONFIGURATION_ORIENTATION_LAND);

    StringView internal_data_path = StringView(
            app->activity->internalDataPath,
            __string_len(app->activity->internalDataPath)
    );
    DebugInfo("Internal data path: {v}", internal_data_path);

    StringView external_data_path = StringView(
            app->activity->externalDataPath,
            __string_len(app->activity->externalDataPath)
    );
    DebugInfo("External data path: {v}", external_data_path);

    StringView obb_path = StringView(
            app->activity->obbPath,
            __string_len(app->activity->obbPath)
    );
    DebugInfo("Obb path: {v}", obb_path);

    AndroidEngine::initialize();

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
        
        if(running)
        {
            AndroidEngine::step();
        }
    }
    
    AndroidEngine::shutdown();
}

