#include "ui.h"
#include <cstring>
#include <cstdio>
#include <cmath>

#define UI_HOVER_SPEED 12.0f
#define UI_PRESS_DECAY 8.0f
#define UI_ENTER_SPEED 2.5f
#define UI_GLOW_TIME 4.0f
#define UI_GLOW_STRENGTH 60

static float UIEaseOut(float t)
{
    return 1.0f - powf(1.0f - t, 3.0f);
}

static Color UIColorLerp(Color a, Color b, float t)
{
    Color c;
    c.r = (unsigned char)(a.r + (b.r - a.r) * t);
    c.g = (unsigned char)(a.g + (b.g - a.g) * t);
    c.b = (unsigned char)(a.b + (b.b - a.b) * t);
    c.a = (unsigned char)(a.a + (b.a - a.a) * t);
    return c;
}

void UIButtonReset(UIButton *b, Rectangle rect, const char *label, Color fill, Color border, Color text)
{
    b->rect = rect;
    snprintf(b->label, sizeof(b->label), "%s", label);
    b->fill = fill;
    b->border = border;
    b->text = text;
    b->hoverT = 0.0f;
    b->pressT = 0.0f;
    b->selectedT = 0.0f;
    b->enterT = 0.0f;
    b->enterDelay = 0.0f;
}

void UIButtonUpdate(UIButton *b, bool hover, bool selected, float dt)
{
    float target = hover ? 1.0f : 0.0f;
    b->hoverT += (target - b->hoverT) * (1.0f - expf(-UI_HOVER_SPEED * dt));
    float selTarget = selected ? 1.0f : 0.0f;
    b->selectedT += (selTarget - b->selectedT) * (1.0f - expf(-UI_HOVER_SPEED * dt));
    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) b->pressT = 1.0f;
    b->pressT -= UI_PRESS_DECAY * dt;
    if (b->pressT < 0.0f) b->pressT = 0.0f;
    b->enterT += UI_ENTER_SPEED * dt;
}

bool UIButtonClicked(UIButton *b, bool hover, bool selected)
{
    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        b->pressT = 1.0f;
        return true;
    }
    if (selected && IsKeyPressed(KEY_ENTER)) {
        b->pressT = 1.0f;
        return true;
    }
    return false;
}

static void DrawGlow(Rectangle rect, Color color, float intensity)
{
    for (int i = 4; i >= 1; i--) {
        float t = (float)i / 4.0f;
        float pad = 4.0f * t;
        Color c = color;
        c.a = (unsigned char)(intensity * 60.0f * (1.0f - t));
        DrawRectangleLinesEx((Rectangle){rect.x - pad, rect.y - pad, rect.width + pad * 2, rect.height + pad * 2}, t * 2.0f, c);
    }
}

void UIButtonDraw(UIButton *b, float time)
{
    float et = b->enterT - b->enterDelay;
    if (et <= 0.0f) return;
    float e = UIEaseOut(et > 1.0f ? 1.0f : et);

    float scale = 1.0f + 0.05f * b->hoverT - 0.06f * b->pressT;
    float w = b->rect.width * scale;
    float h = b->rect.height * scale;
    Rectangle r = {
        b->rect.x + (b->rect.width - w) / 2.0f,
        b->rect.y + (b->rect.height - h) / 2.0f,
        w, h
    };

    r.y += (1.0f - e) * 30.0f;

    Color fill = UIColorLerp(b->fill, (Color){255, 80, 40, 255}, b->hoverT * 0.55f);
    fill.a = (unsigned char)(fill.a * e);

    Color selColor = (Color){255, 60, 40, 255};
    Color border = UIColorLerp(b->border, RED, b->hoverT);
    border = UIColorLerp(border, selColor, b->selectedT * 0.6f);
    border.a = (unsigned char)(border.a * e);

    float selPulse = 0.4f + 0.3f * sinf((float)time * 6.0f);
    float glow = b->hoverT * 0.8f + (b->pressT > 0.0f ? 0.5f * b->pressT : 0.0f) + b->selectedT * selPulse;
    if (glow > 0.01f) DrawGlow(r, selColor, glow);

    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 2, border);

    int fontSize = (int)(b->rect.height * 0.5f);
    int lw = MeasureText(b->label, fontSize);
    int lx = (int)(r.x + r.width / 2.0f) - lw / 2;
    int ly = (int)(r.y + r.height / 2.0f) - fontSize / 2;

    Color tc = (b->hoverT > 0.01f) ? (Color){255, 220, 200, 255} : b->text;
    tc.a = (unsigned char)(tc.a * e);
    DrawText(b->label, lx, ly, fontSize, tc);
}