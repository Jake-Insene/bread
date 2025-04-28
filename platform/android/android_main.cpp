#include "debug/debug.h"
#include "objects/scene_manager.h"
#include "platform/android/android_engine.h"

#define BUILD_APP 1

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
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        if(source == AINPUT_SOURCE_TOUCHSCREEN)
        {
            i32 action_pointer = AMotionEvent_getAction(event);
            i32 action = action_pointer & AMOTION_EVENT_ACTION_MASK;
            size_t pointer_count = (size_t)AMotionEvent_getPointerCount(event);
            for(size_t p = 0; p < pointer_count; p++)
            {
                InputEventTouch e = {};
                
                // Y positive is up
                e.type = INPUT_EVENT_TOUCH;
                e.position = Vector2
                (
                    AMotionEvent_getX(event, p),
                    -AMotionEvent_getY(event, p)
                );
                e.pressed =
                (action == AMOTION_EVENT_ACTION_DOWN
                 || action == AMOTION_EVENT_ACTION_MOVE
                 || action == AMOTION_EVENT_ACTION_POINTER_DOWN);
                 e.pointer = (i32)p;
                
                Engine::handle_input(e);
                
                //Log::info("action pointer: %08X, action: %d, pointer: %d", action_pointer, action, p);
            }
        }
        
        return 1;
    }
    return 0;
}

static void engine_handle_cmd(android_app* app, int32_t cmd)
{
    switch (cmd) {
    case APP_CMD_SAVE_STATE:
        Log::info("Saving state...");
        //save_state.last_scene = SceneManager::current_scene->klass;

        app->savedState = malloc(sizeof(SaveState));
        *((SaveState*)app->savedState) = save_state;
        app->savedStateSize = sizeof(SaveState);
        break;
    case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        if (AndroidEngine::app->window != nullptr)
        {
#if BUILD_APP
        Engine::recreate_window();
#endif
    	}
        break;
    case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
#if BUILD_APP
            Engine::destroy();
#endif 
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
    AndroidEngine::app = app;
    AndroidEngine::asset_manager = app->activity->assetManager;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    
    Debug::info("Internal data path: %s", app->activity->internalDataPath);
    Debug::info("External data path: %s", app->activity->externalDataPath);
    Debug::info("Obb path: %s", app->activity->obbPath);
    
#if BUILD_APP
    AndroidEngine::initialize();
#endif

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

        if (AndroidEngine::app->destroyRequested)
        {
            break;
        }
        
        if(running)
        {
#if BUILD_APP
            AndroidEngine::step();
#endif
        }
    }
    
#if BUILD_APP
    AndroidEngine::shutdown();
#endif
}

