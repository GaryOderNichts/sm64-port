#ifdef TARGET_WII_U

#include "gfx_egl.h"
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __WIIU__
#include <coreinit/debug.h>
#include <sys/iosupport.h>
#include <whb/proc.h>
#endif

static EGLDisplay egl_display;
static EGLSurface egl_surface;
static EGLContext egl_context;

#ifdef __WIIU__
static ssize_t wiiu_log_write(struct _reent *r, void *fd, const char *ptr, size_t len)
{
    OSReport("%*.*s", len, len, ptr);
    return len;
}

static const devoptab_t dotab_stdout = {
    .name    = "stdout_whb",
    .write_r = wiiu_log_write,
};
#endif

static void gfx_egl_init(const char *game_name, bool start_in_fullscreen) {
#ifdef __WIIU__
    WHBProcInit();

    devoptab_list[STD_OUT] = &dotab_stdout;
    devoptab_list[STD_ERR] = &dotab_stdout;
#endif

    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Can't get egl display\n");
        exit(1);
    }

    if (eglInitialize(egl_display, NULL, NULL) != EGL_TRUE) {
        fprintf(stderr, "Can't initialise egl\n");
        exit(1);
    }

    static const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_NONE
    };

    static const EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 0,
        EGL_NONE
    };

    EGLConfig egl_conf;
    EGLint egl_conf_cnt;
    if (eglChooseConfig(egl_display, config_attribs, &egl_conf, 1, &egl_conf_cnt) != EGL_TRUE) {
        fprintf(stderr, "Failed to choose egl config\n");
        exit(1);
    }

    if (egl_conf_cnt == 0) {
        fprintf(stderr, "No matching egl configs\n");
        exit(1);  
    }

    egl_surface = eglCreateWindowSurface(egl_display, egl_conf, (EGLNativeWindowType)NULL, NULL);
    if (egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Can't create window surface\n");
        exit(1);
    }

    // TODO angle doesn't like desktop yet i guess
    // at least the commit I'm using
    if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
        fprintf(stderr, "Can't bind API\n");
        exit(1);  
    }

    egl_context = eglCreateContext(egl_display, egl_conf, EGL_NO_CONTEXT, context_attribs);
    if (egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Can't create egl context\n");
        exit(1);
    }

    if (eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context) != EGL_TRUE) {
        fprintf(stderr, "Made current failed\n");
    }

    eglSwapInterval(egl_display, 2);
}

static void gfx_egl_set_fullscreen_changed_callback(void (*on_fullscreen_changed)(bool is_now_fullscreen)) {

}

static void gfx_egl_set_fullscreen(bool enable) {

}

static void gfx_egl_set_keyboard_callbacks(bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode), void (*on_all_keys_up)(void)) {

}

static void gfx_egl_main_loop(void (*run_one_game_iter)(void)) {
#ifdef __WIIU__
    while (WHBProcIsRunning()) {
        run_one_game_iter();
    }

    WHBProcShutdown();
    exit(0);
#else
    while (1) {
        run_one_game_iter();
    }
#endif
}

static void gfx_egl_get_dimensions(uint32_t *width, uint32_t *height) {
    *width = 1920;
    *height = 1080;
}

static void gfx_egl_handle_events(void) {

}

static bool gfx_egl_start_frame(void) {
    return true;
}

static void gfx_egl_swap_buffers_begin(void) {

}

static void gfx_egl_swap_buffers_end(void) {
    eglSwapBuffers(egl_display, egl_surface);
}

static double gfx_egl_get_time(void) {
    return 0.0;
}

struct GfxWindowManagerAPI gfx_egl = {
    gfx_egl_init,
    gfx_egl_set_keyboard_callbacks,
    gfx_egl_set_fullscreen_changed_callback,
    gfx_egl_set_fullscreen,
    gfx_egl_main_loop,
    gfx_egl_get_dimensions,
    gfx_egl_handle_events,
    gfx_egl_start_frame,
    gfx_egl_swap_buffers_begin,
    gfx_egl_swap_buffers_end,
    gfx_egl_get_time
};

#endif
