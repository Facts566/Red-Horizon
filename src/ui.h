#pragma once
#include <raylib.h>

struct UIButton {
    Rectangle rect;
    char label[32];
    Color fill;
    Color border;
    Color text;
    float hoverT;
    float pressT;
    float selectedT;
    float enterT;
    float enterDelay;
};

void UIButtonReset(UIButton *b, Rectangle rect, const char *label, Color fill, Color border, Color text);
void UIButtonUpdate(UIButton *b, bool hover, bool selected, float dt);
bool UIButtonClicked(UIButton *b, bool hover, bool selected);
void UIButtonDraw(UIButton *b, float time);
