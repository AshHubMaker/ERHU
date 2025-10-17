// Erhu Tuner v1.1 — single-file (no notes.h needed)
// OK toggles play/stop; Hold BACK quits; LEFT=Inner(D4), RIGHT=Outer(A4); UP/DOWN volume
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <gui/gui.h>

#define D4 293.66f
#define A4 440.00f
#define VOLUME_DEFAULT 0.5f
#define VOLUME_STEP 0.05f

typedef enum { SelInner = 0, SelOuter = 1 } ErhuSelection;

typedef struct {
    FuriMutex* mutex;
    Gui* gui;
    ViewPort* vp;
    float volume;
    ErhuSelection selection;
    bool playing;
    bool running;
} ErhuState;

static float sel_to_freq(ErhuSelection s) { return s == SelInner ? D4 : A4; }

static void erhu_play(ErhuState* st){
    if(!st->playing){
        if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(1000)){
            furi_hal_speaker_start(sel_to_freq(st->selection), st->volume);
            st->playing = true;
        }
    }
}
static void erhu_stop(ErhuState* st){
    if(st->playing && furi_hal_speaker_is_mine()) furi_hal_speaker_stop();
    st->playing = false;
}
static void erhu_toggle(ErhuState* st){ st->playing ? erhu_stop(st) : erhu_play(st); }

static void render_cb(Canvas* canvas, void* ctx){
    ErhuState* st = ctx;
    furi_mutex_acquire(st->mutex, FuriWaitForever);

    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_draw_rframe(canvas, 20, 10, 88, 24, 4);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 22, AlignCenter, AlignBottom, "ERHU");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, "Tuning Fork");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 10, 52, AlignLeft, AlignCenter, "< Inner (D4)");
    canvas_draw_str_aligned(canvas, 118, 52, AlignRight, AlignCenter, "Outer (A4) >");

    if(st->selection == SelInner) canvas_draw_rframe(canvas, 4, 44, 64, 16, 3);
    else                          canvas_draw_rframe(canvas, 60, 44, 64, 16, 3);

    // Status line (int pct to avoid float->double warning)
    char buf[48];
    int pct = (int)(st->volume * 100.0f + 0.5f);
    snprintf(buf, sizeof(buf), "Vol %d%%  %s  OK=Toggle", pct, st->playing ? "Play" : "Ready");
    buf[20] = '\0';
    canvas_draw_str_aligned(canvas, 64, 6, AlignCenter, AlignTop, buf);

    furi_mutex_release(st->mutex);
}

static void input_cb(InputEvent* e, void* ctx){
    ErhuState* st = ctx;
    if(e->type == InputTypeShort || e->type == InputTypeRepeat){
        if(e->key == InputKeyLeft) st->selection = SelInner;
        else if(e->key == InputKeyRight) st->selection = SelOuter;
        else if(e->key == InputKeyOk) erhu_toggle(st);
        else if(e->key == InputKeyUp){
            if(st->volume <= 1.0f - VOLUME_STEP) st->volume += VOLUME_STEP;
            if(st->playing) erhu_play(st);
        } else if(e->key == InputKeyDown){
            if(st->volume >= VOLUME_STEP) st->volume -= VOLUME_STEP;
            if(st->playing) erhu_play(st);
        }
        // BACK short: do nothing
    } else if(e->type == InputTypeLong && e->key == InputKeyBack){
        st->running = false; // quit
    }
}

int32_t erhu_tuner_app(){
    ErhuState* st = malloc(sizeof(ErhuState));
    st->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    st->gui = furi_record_open(RECORD_GUI);
    st->vp = view_port_alloc();
    st->volume = VOLUME_DEFAULT;
    st->selection = SelOuter;
    st->playing = false;
    st->running = true;

    view_port_draw_callback_set(st->vp, render_cb, st);
    view_port_input_callback_set(st->vp, input_cb, st);
    gui_add_view_port(st->gui, st->vp, GuiLayerFullscreen);

    while(st->running) furi_delay_ms(50);

    erhu_stop(st);
    gui_remove_view_port(st->gui, st->vp);
    view_port_free(st->vp);
    furi_record_close(RECORD_GUI);
    furi_mutex_free(st->mutex);
    free(st);
    return 0;
}
