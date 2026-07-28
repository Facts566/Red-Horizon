#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "config.h"
#include "level.h"
#include "map.h"

#define MAX_EDITOR_OBJECTS 256
#define MAX_ENEMIES 256
#define EDITOR_CAM_SPEED 30.0f
#define EDITOR_SENSITIVITY 0.003f
#define GRID_Y 0.05f

enum EditorCategory {
    CAT_DECOR = 0,
    CAT_DOORS,
    CAT_ENEMIES,
    CAT_BONUSES,
    CAT_PLAYER,
    CAT_COUNT
};

struct EditorObject {
    char type[32];
    float col, row, y, rotation, scale;
    bool collision;
    bool isDoor;
    bool isLocked, isExit;
};

struct EnemyPlacement {
    int col, row;
    char type;
};

struct PaletteItem {
    const char *name;
    const char *modelPath;
    const char *texPath;
    float defaultY;
    float defaultScale;
    bool collision;
    Color color;
    bool isDoor;
    bool isLocked;
    bool isExit;
    char enemyChar;
};

struct EditorModels {
    Model sofa, lamp, blood, trash, box;
    Texture2D sofaTex, lampTex, bloodTex, trashTex, boxTex;
};

struct EditorState {
    EditorObject objects[MAX_EDITOR_OBJECTS];
    int objectCount;
    EnemyPlacement enemies[MAX_ENEMIES];
    int enemyCount;
    int category;
    int selection;
    float yaw, pitch;
    float previewRotation;
    float previewYOffset;
    float previewScale;
    bool showGrid;
    Camera3D camera;
    Level level;
    EditorModels models;
    char statusMsg[128];
    float statusTimer;
    int selectedObjIdx;
    int selectedEnemyIdx;
};

static const char *categoryNames[] = {
    "DECOR", "DOORS", "ENEMIES", "BONUSES", "PLAYER"
};

static PaletteItem allItems[] = {
    {"sofa",     "models/sofa.obj",  "tex/decor/sofa.png",  3.5f,  4.0f,  true,  {139,90,43,255},   false, false, false, 0},
    {"lamp",     "models/lamp.obj",  "tex/decor/lamp.png",  19.0f, 0.5f,  false, {255,255,0,255},   false, false, false, 0},
    {"blood",    "models/blood.obj", "tex/decor/blood.png", 0.1f,  6.0f,  false, {200,0,0,255},     false, false, false, 0},
    {"trash",    "models/trash.obj", "tex/decor/trash.png", 3.0f,  3.0f,  true,  {128,128,128,255}, false, false, false, 0},
    {"box",      NULL,               "tex/decor/box.png",   2.5f,  5.0f,  true,  {200,150,50,255},  false, false, false, 0},
    {"door",     NULL, NULL, 0.0f, 1.0f, false, {0,100,255,255},  true, false, false, 0},
    {"door",     NULL, NULL, 0.0f, 1.0f, false, {255,50,50,255},  true, true,  false, 0},
    {"door",     NULL, NULL, 0.0f, 1.0f, false, {50,255,50,255},  true, false, true,  0},
    {"zombie",   NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {255,50,50,255},  false, false, false, 'Z'},
    {"military", NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {80,80,80,255},   false, false, false, 'M'},
    {"fast",     NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {255,150,0,255},  false, false, false, 'F'},
    {"health",   NULL, NULL, BONUS_Y_HEIGHT, 1.0f, false, {255,255,0,255},  false, false, false, 'H'},
    {"key",      NULL, NULL, BONUS_Y_HEIGHT, 1.0f, false, {255,215,0,255},  false, false, false, 'K'},
    {"player",   NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {0,255,0,255},    false, false, false, 'P'},
};

static int categoryStart[] = {0, 5, 8, 11, 13};
static int categoryCount[] = {5, 3, 3, 2, 1};

static Vector3 TilePos(float col, float row, float y = 0.0f) {
    return {col * TILE_SIZE + TILE_SIZE / 2.0f, y, row * TILE_SIZE + TILE_SIZE / 2.0f};
}

static bool GetFloorHit(Camera3D cam, Vector3 *result) {
    Vector2 sc = {(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2};
    Ray ray = GetScreenToWorldRay(sc, cam);
    if (fabs(ray.direction.y) > 0.0001f) {
        float t = -ray.position.y / ray.direction.y;
        if (t > 0) {
            *result = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
            return true;
        }
    }
    *result = {0, -1000, 0};
    return false;
}

static Vector3 SnapToTile(Vector3 hit) {
    int col = (int)roundf(hit.x / TILE_SIZE);
    int row = (int)roundf(hit.z / TILE_SIZE);
    return TilePos((float)col, (float)row, hit.y);
}

static Model *GetModel(EditorModels &m, const char *type) {
    if (strcmp(type, "sofa") == 0) return &m.sofa;
    if (strcmp(type, "lamp") == 0) return &m.lamp;
    if (strcmp(type, "blood") == 0) return &m.blood;
    if (strcmp(type, "trash") == 0) return &m.trash;
    if (strcmp(type, "box") == 0) return &m.box;
    return NULL;
}

static void LoadEditorModels(EditorModels &m) {
    m.sofa = LoadModel("models/sofa.obj");
    m.sofaTex = LoadTexture("tex/decor/sofa.png");
    SetTextureFilter(m.sofaTex, TEXTURE_FILTER_POINT);
    m.sofa.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.sofaTex;

    m.lamp = LoadModel("models/lamp.obj");
    m.lampTex = LoadTexture("tex/decor/lamp.png");
    SetTextureFilter(m.lampTex, TEXTURE_FILTER_POINT);
    m.lamp.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.lampTex;

    m.blood = LoadModel("models/blood.obj");
    m.bloodTex = LoadTexture("tex/decor/blood.png");
    SetTextureFilter(m.bloodTex, TEXTURE_FILTER_POINT);
    m.blood.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.bloodTex;

    m.trash = LoadModel("models/trash.obj");
    m.trashTex = LoadTexture("tex/decor/trash.png");
    SetTextureFilter(m.trashTex, TEXTURE_FILTER_POINT);
    m.trash.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.trashTex;

    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    m.box = LoadModelFromMesh(cube);
    m.boxTex = LoadTexture("tex/decor/box.png");
    SetTextureFilter(m.boxTex, TEXTURE_FILTER_POINT);
    m.box.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.boxTex;
}

static void UnloadEditorModels(EditorModels &m) {
    UnloadModel(m.sofa); UnloadTexture(m.sofaTex);
    UnloadModel(m.lamp); UnloadTexture(m.lampTex);
    UnloadModel(m.blood); UnloadTexture(m.bloodTex);
    UnloadModel(m.trash); UnloadTexture(m.trashTex);
    UnloadModel(m.box); UnloadTexture(m.boxTex);
}

static void LoadDecorFile(EditorState *s, int lvl) {
    char path[256];
    sprintf(path, "map/level_%d/decor.txt", lvl);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f) && s->objectCount < MAX_EDITOR_OBJECTS) {
        if (line[0] == '#' || line[0] == '\n') continue;
        EditorObject obj = {};
        if (sscanf(line, "obj %31s %f %f %f %f %f %d",
                   obj.type, &obj.col, &obj.row, &obj.y,
                   &obj.rotation, &obj.scale, (int*)&obj.collision) == 7) {
            s->objects[s->objectCount++] = obj;
        } else if (strncmp(line, "door", 4) == 0) {
            if (sscanf(line, "door %f %f %f %d %d",
                      &obj.col, &obj.row, &obj.rotation,
                      (int*)&obj.isLocked, (int*)&obj.isExit) == 5) {
                strcpy(obj.type, "door");
                obj.isDoor = true;
                s->objects[s->objectCount++] = obj;
            }
        }
    }
    fclose(f);
}

static void SaveDecorFile(EditorState *s, int lvl) {
    char path[256];
    sprintf(path, "map/level_%d/decor.txt", lvl);
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < s->objectCount; i++) {
        EditorObject &o = s->objects[i];
        if (o.isDoor) {
            fprintf(f, "door %.1f %.1f %.0f %d %d\n",
                    o.col, o.row, o.rotation, o.isLocked ? 1 : 0, o.isExit ? 1 : 0);
        } else {
            fprintf(f, "obj %s %.1f %.1f %.1f %.0f %.1f %d\n",
                    o.type, o.col, o.row, o.y, o.rotation, o.scale, o.collision ? 1 : 0);
        }
    }
    fclose(f);
}

static void LoadEnemyFile(EditorState *s, int lvl) {
    char path[256];
    sprintf(path, "map/level_%d/enemy.txt", lvl);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[512];
    int row = 0;
    while (fgets(line, sizeof(line), f)) {
        int col = 0;
        bool inToken = false;
        for (int i = 0; line[i] && line[i] != '\n'; i++) {
            if (line[i] == ' ') {
                inToken = false;
            } else {
                if (!inToken) {
                    char c = line[i];
                    if ((c == 'Z' || c == 'M' || c == 'F' ||
                         c == 'H' || c == 'K' || c == 'P') &&
                        s->enemyCount < MAX_ENEMIES) {
                        s->enemies[s->enemyCount].col = col;
                        s->enemies[s->enemyCount].row = row;
                        s->enemies[s->enemyCount].type = c;
                        s->enemyCount++;
                    }
                    col++;
                    inToken = true;
                }
            }
        }
        row++;
    }
    fclose(f);
}

static void SaveEnemyFile(EditorState *s, int lvl) {
    char path[256];
    sprintf(path, "map/level_%d/enemy.txt", lvl);
    int w = s->level.width;
    int h = s->level.height;
    if (w <= 0 || h <= 0) return;
    char **grid = (char **)malloc(h * sizeof(char *));
    for (int r = 0; r < h; r++) {
        grid[r] = (char *)calloc(w + 1, sizeof(char));
        for (int c = 0; c < w; c++) grid[r][c] = '.';
    }
    for (int i = 0; i < s->enemyCount; i++) {
        EnemyPlacement &e = s->enemies[i];
        if (e.col >= 0 && e.col < w && e.row >= 0 && e.row < h)
            grid[e.row][e.col] = e.type;
    }
    FILE *f = fopen(path, "w");
    if (f) {
        for (int r = 0; r < h; r++) {
            for (int c = 0; c < w; c++) {
                fprintf(f, "%c", grid[r][c]);
                if (c < w - 1) fprintf(f, " ");
            }
            fprintf(f, "\n");
        }
        fclose(f);
    }
    for (int r = 0; r < h; r++) free(grid[r]);
    free(grid);
}

static void UpdateCamera(EditorState *s, float dt) {
    float speed = EDITOR_CAM_SPEED;
    if (IsKeyDown(KEY_LEFT_CONTROL)) speed *= 3.0f;
    Vector3 fwd;
    fwd.x = cosf(s->pitch) * sinf(s->yaw);
    fwd.y = sinf(s->pitch);
    fwd.z = cosf(s->pitch) * cosf(s->yaw);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(fwd, {0, 1, 0}));
    Vector3 flat = Vector3Normalize({fwd.x, 0, fwd.z});
    if (IsKeyDown(KEY_W)) s->camera.position = Vector3Add(s->camera.position, Vector3Scale(flat, speed * dt));
    if (IsKeyDown(KEY_S)) s->camera.position = Vector3Subtract(s->camera.position, Vector3Scale(flat, speed * dt));
    if (IsKeyDown(KEY_A)) s->camera.position = Vector3Subtract(s->camera.position, Vector3Scale(right, speed * dt));
    if (IsKeyDown(KEY_D)) s->camera.position = Vector3Add(s->camera.position, Vector3Scale(right, speed * dt));
    if (IsKeyDown(KEY_SPACE)) s->camera.position.y += speed * dt;
    if (IsKeyDown(KEY_LEFT_SHIFT)) s->camera.position.y -= speed * dt;
    Vector2 md = GetMouseDelta();
    s->yaw -= md.x * EDITOR_SENSITIVITY;
    s->pitch -= md.y * EDITOR_SENSITIVITY;
    if (s->pitch > 89.0f * DEG2RAD) s->pitch = 89.0f * DEG2RAD;
    if (s->pitch < -89.0f * DEG2RAD) s->pitch = -89.0f * DEG2RAD;
    s->camera.target = Vector3Add(s->camera.position, fwd);
}

static PaletteItem *CurrentItem(EditorState *s) {
    return &allItems[categoryStart[s->category] + s->selection];
}

static void PlaceObject(EditorState *s) {
    Vector3 hit;
    if (!GetFloorHit(s->camera, &hit)) return;
    PaletteItem *item = CurrentItem(s);
    if (item->enemyChar != 0) {
        if (s->enemyCount >= MAX_ENEMIES) return;
        Vector3 pos = SnapToTile(hit);
        EnemyPlacement &e = s->enemies[s->enemyCount++];
        e.col = (int)((pos.x - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        e.row = (int)((pos.z - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        e.type = item->enemyChar;
        strcpy(s->statusMsg, TextFormat("Placed %s", item->name));
    } else {
        if (s->objectCount >= MAX_EDITOR_OBJECTS) return;
        bool snap = item->isDoor;
        Vector3 pos = snap ? SnapToTile(hit) : hit;
        EditorObject &o = s->objects[s->objectCount++];
        strcpy(o.type, item->name);
        o.col = (pos.x - TILE_SIZE / 2.0f) / TILE_SIZE;
        o.row = (pos.z - TILE_SIZE / 2.0f) / TILE_SIZE;
        o.y = item->defaultY + s->previewYOffset;
        o.rotation = s->previewRotation;
        o.scale = item->defaultScale * s->previewScale;
        o.collision = item->collision;
        o.isDoor = item->isDoor;
        o.isLocked = item->isLocked;
        o.isExit = item->isExit;
        strcpy(s->statusMsg, TextFormat("Placed %s", item->name));
    }
    s->statusTimer = 1.5f;
}

static void FindClosest(EditorState *s, Vector3 hit, float maxDist2, int &outType, int &outIdx) {
    outType = -1;
    outIdx = -1;
    float best = maxDist2;
    for (int i = 0; i < s->objectCount; i++) {
        Vector3 pos = TilePos(s->objects[i].col, s->objects[i].row, s->objects[i].y);
        float d = Vector3DistanceSqr(pos, hit);
        if (d < best) { best = d; outType = 0; outIdx = i; }
    }
    for (int i = 0; i < s->enemyCount; i++) {
        EnemyPlacement &e = s->enemies[i];
        Vector3 pos = {(float)e.col * TILE_SIZE + TILE_SIZE / 2.0f, ZOMBIE_SPAWN_Y, (float)e.row * TILE_SIZE + TILE_SIZE / 2.0f};
        float d = Vector3DistanceSqr(pos, hit);
        if (d < best) { best = d; outType = 1; outIdx = i; }
    }
}

static bool IsSelectedObj(EditorState *s, int idx) {
    return s->selectedObjIdx == idx;
}

static bool IsSelectedEnemy(EditorState *s, int idx) {
    return s->selectedEnemyIdx == idx;
}

static void SelectAtCrosshair(EditorState *s) {
    Vector3 hit;
    if (!GetFloorHit(s->camera, &hit)) return;
    int type, idx;
    FindClosest(s, hit, 8.0f * 8.0f, type, idx);
    s->selectedObjIdx = -1;
    s->selectedEnemyIdx = -1;
    if (type == 0 && idx >= 0) {
        s->selectedObjIdx = idx;
        EditorObject &o = s->objects[idx];
        strcpy(s->statusMsg, TextFormat("Selected: %s (%.1f, %.1f)", o.type, o.col, o.row));
        s->statusTimer = 2.0f;
    } else if (type == 1 && idx >= 0) {
        s->selectedEnemyIdx = idx;
        EnemyPlacement &e = s->enemies[idx];
        strcpy(s->statusMsg, TextFormat("Selected: %c (%d, %d)", e.type, e.col, e.row));
        s->statusTimer = 2.0f;
    } else {
        strcpy(s->statusMsg, "Nothing selected");
        s->statusTimer = 1.0f;
    }
}

static void Deselect(EditorState *s) {
    s->selectedObjIdx = -1;
    s->selectedEnemyIdx = -1;
}

static void DeleteSelected(EditorState *s) {
    if (s->selectedObjIdx >= 0) {
        s->objects[s->selectedObjIdx] = s->objects[s->objectCount - 1];
        s->objectCount--;
        strcpy(s->statusMsg, "Deleted object");
        s->statusTimer = 1.0f;
        s->selectedObjIdx = -1;
    } else if (s->selectedEnemyIdx >= 0) {
        s->enemies[s->selectedEnemyIdx] = s->enemies[s->enemyCount - 1];
        s->enemyCount--;
        strcpy(s->statusMsg, "Deleted enemy");
        s->statusTimer = 1.0f;
        s->selectedEnemyIdx = -1;
    }
}

static void MoveSelectedToCrosshair(EditorState *s) {
    Vector3 hit;
    if (!GetFloorHit(s->camera, &hit)) return;
    if (s->selectedObjIdx >= 0) {
        EditorObject &o = s->objects[s->selectedObjIdx];
        bool snap = o.isDoor;
        Vector3 pos = snap ? SnapToTile(hit) : hit;
        o.col = (pos.x - TILE_SIZE / 2.0f) / TILE_SIZE;
        o.row = (pos.z - TILE_SIZE / 2.0f) / TILE_SIZE;
    } else if (s->selectedEnemyIdx >= 0) {
        EnemyPlacement &e = s->enemies[s->selectedEnemyIdx];
        Vector3 pos = SnapToTile(hit);
        e.col = (int)((pos.x - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        e.row = (int)((pos.z - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
    }
}

static void HandleInput(EditorState *s, int lvl) {
    bool hasSelection = (s->selectedObjIdx >= 0 || s->selectedEnemyIdx >= 0);

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (hasSelection) {
            Deselect(s);
            strcpy(s->statusMsg, "Deselected");
            s->statusTimer = 1.0f;
        }
        return;
    }

    if (hasSelection) {
        MoveSelectedToCrosshair(s);

        float wheel = GetMouseWheelMove();
        if (s->selectedObjIdx >= 0) {
            EditorObject &o = s->objects[s->selectedObjIdx];
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                o.y += wheel * 2.0f;
            } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
                o.scale += wheel * 0.1f;
                if (o.scale < 0.1f) o.scale = 0.1f;
            } else {
                o.rotation += wheel * 15.0f;
            }
        }
        if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
            DeleteSelected(s);
        }
        if (IsKeyPressed(KEY_ENTER)) {
            Deselect(s);
            strcpy(s->statusMsg, "Confirmed");
            s->statusTimer = 1.0f;
        }
        return;
    }

    if (IsKeyPressed(KEY_TAB)) {
        s->category = (s->category + 1) % CAT_COUNT;
        s->selection = 0;
        s->previewRotation = 0;
        s->previewYOffset = 0;
        s->previewScale = 1.0f;
    }
    for (int i = 0; i < 9; i++) {
        if (IsKeyPressed(KEY_ONE + i) && i < categoryCount[s->category]) {
            s->selection = i;
            s->previewRotation = 0;
            s->previewYOffset = 0;
            s->previewScale = 1.0f;
        }
    }
    if (IsKeyPressed(KEY_G)) s->showGrid = !s->showGrid;
    float wheel = GetMouseWheelMove();
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        s->previewYOffset += wheel * 2.0f;
    } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
        s->previewScale += wheel * 0.1f;
        if (s->previewScale < 0.1f) s->previewScale = 0.1f;
    } else {
        s->previewRotation += wheel * 15.0f;
    }
    if (IsKeyPressed(KEY_E)) PlaceObject(s);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) SelectAtCrosshair(s);
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        int type, idx;
        Vector3 hit;
        if (GetFloorHit(s->camera, &hit)) {
            FindClosest(s, hit, 8.0f * 8.0f, type, idx);
            if (type == 0 && idx >= 0) {
                s->objects[idx] = s->objects[s->objectCount - 1];
                s->objectCount--;
                strcpy(s->statusMsg, "Deleted object");
                s->statusTimer = 1.0f;
            } else if (type == 1 && idx >= 0) {
                s->enemies[idx] = s->enemies[s->enemyCount - 1];
                s->enemyCount--;
                strcpy(s->statusMsg, "Deleted enemy");
                s->statusTimer = 1.0f;
            }
        }
    }
    if (IsKeyPressed(KEY_ENTER)) {
        SaveDecorFile(s, lvl);
        SaveEnemyFile(s, lvl);
        strcpy(s->statusMsg, "SAVED!");
        s->statusTimer = 2.0f;
    }
}

static void DrawGridOverlay(Level level) {
    float ts = level.tileSize;
    Color c = ColorAlpha(GREEN, 0.3f);
    for (int row = 0; row <= level.height; row++)
        DrawLine3D({0, GRID_Y, row * ts}, {level.width * ts, GRID_Y, row * ts}, c);
    for (int col = 0; col <= level.width; col++)
        DrawLine3D({col * ts, GRID_Y, 0}, {col * ts, GRID_Y, level.height * ts}, c);
}

static void DrawEditorObjects(EditorState *s) {
    for (int i = 0; i < s->objectCount; i++) {
        EditorObject &o = s->objects[i];
        Vector3 pos = TilePos(o.col, o.row, o.y);
        Color tint = IsSelectedObj(s, i) ? YELLOW : WHITE;
        if (o.isDoor) {
            Color dc = o.isExit ? GREEN : (o.isLocked ? RED : BLUE);
            if (IsSelectedObj(s, i)) dc = YELLOW;
            DrawCube({pos.x, pos.y + 7.5f, pos.z}, 5.0f, 15.0f, 1.0f, dc);
        } else {
            Model *mdl = GetModel(s->models, o.type);
            if (mdl) DrawModelEx(*mdl, pos, {0, 1, 0}, o.rotation,
                                 {o.scale, o.scale, o.scale}, tint);
        }
        if (IsSelectedObj(s, i)) {
            DrawCubeWires({pos.x, pos.y + 7.5f, pos.z}, 6.0f, 16.0f, 1.0f, YELLOW);
        }
    }
    for (int i = 0; i < s->enemyCount; i++) {
        EnemyPlacement &e = s->enemies[i];
        Vector3 pos = {(float)e.col * TILE_SIZE + TILE_SIZE / 2.0f, ZOMBIE_SPAWN_Y,
                       (float)e.row * TILE_SIZE + TILE_SIZE / 2.0f};
        Color ec;
        switch (e.type) {
            case 'Z': ec = RED; break;
            case 'M': ec = DARKGRAY; break;
            case 'F': ec = ORANGE; break;
            case 'H': ec = YELLOW; break;
            case 'K': ec = GOLD; break;
            case 'P': ec = GREEN; break;
            default: ec = WHITE; break;
        }
        DrawSphere(pos, 2.0f, ec);
        if (IsSelectedEnemy(s, i)) {
            DrawSphereWires(pos, 3.0f, 8, 8, YELLOW);
        }
    }
}

static void DrawGhost(EditorState *s) {
    Vector3 hit;
    if (!GetFloorHit(s->camera, &hit)) return;
    PaletteItem *item = CurrentItem(s);
    bool snap = (item->enemyChar != 0) || item->isDoor;
    Vector3 pos = snap ? SnapToTile(hit) : hit;
    pos.y = item->defaultY + s->previewYOffset;
    Color tint = ColorAlpha(item->color, 0.7f);
    if (item->enemyChar != 0) {
        DrawSphere(pos, 2.0f, tint);
    } else if (item->isDoor) {
        DrawCube({pos.x, pos.y + 7.5f, pos.z}, 5.0f, 15.0f, 1.0f, tint);
    } else {
        Model *mdl = GetModel(s->models, item->name);
        if (mdl) {
            float sc = item->defaultScale * s->previewScale;
            DrawModelEx(*mdl, pos, {0, 1, 0}, s->previewRotation, {sc, sc, sc}, tint);
        }
    }
}

static void DrawHUD(EditorState *s) {
    int x = 10, y = 10, lh = 20;
    bool hasSel = (s->selectedObjIdx >= 0 || s->selectedEnemyIdx >= 0);

    DrawText("RED HORIZON - LEVEL EDITOR", x, y, 20, WHITE); y += lh + 10;

    if (hasSel) {
        DrawText("[ MOVING MODE ]", x, y, 18, YELLOW); y += lh;
        DrawText("Object follows crosshair", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Wheel - rotate | Shift+Wheel - Y | Ctrl+Wheel - scale", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Enter - confirm | Esc - cancel | Del - delete", x, y, 14, LIGHTGRAY); y += lh + 10;
    } else {
        DrawText(TextFormat("Category: %s", categoryNames[s->category]), x, y, 16, YELLOW); y += lh;
        PaletteItem *item = CurrentItem(s);
        DrawText(TextFormat("Selected: %s", item->name), x, y, 16, GREEN); y += lh;
        DrawText(TextFormat("Objects: %d | Enemies: %d", s->objectCount, s->enemyCount), x, y, 16, LIGHTGRAY); y += lh + 10;
        DrawText("CONTROLS:", x, y, 16, WHITE); y += lh;
        DrawText("WASD/Space/Shift - Move camera", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Mouse - Look around", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Tab - Cycle category", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("1-9 - Select item in palette", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("E - Place new object", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Left Click - Select existing object", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Right Click - Delete under crosshair", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Wheel - rotate | Shift+Wheel - Y | Ctrl+Wheel - scale", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("G - Toggle grid | Enter - Save", x, y, 14, LIGHTGRAY);
    }

    if (s->statusTimer > 0) {
        int tw = MeasureText(s->statusMsg, 30);
        DrawText(s->statusMsg, GetScreenWidth() / 2 - tw / 2, 60, 30, GREEN);
    }
}

int main(int argc, char *argv[]) {
    int levelIndex = 1;
    if (argc > 1) levelIndex = atoi(argv[1]);
    if (levelIndex < 1 || levelIndex > 9) levelIndex = 1;

    InitWindow(1280, 720, "Red Horizon - Level Editor");
    SetTargetFPS(60);
    DisableCursor();

    Texture2D floorTex = LoadTexture("tex/map/floor.png");
    SetTextureFilter(floorTex, TEXTURE_FILTER_POINT);
    Texture2D planksTex = LoadTexture("tex/map/planks.png");
    SetTextureFilter(planksTex, TEXTURE_FILTER_POINT);
    Texture2D wallTex = LoadTexture("tex/map/wall.png");
    SetTextureFilter(wallTex, TEXTURE_FILTER_POINT);
    Texture2D greenTex = LoadTexture("tex/map/green.png");
    SetTextureFilter(greenTex, TEXTURE_FILTER_POINT);
    Texture2D whiteTex = LoadTexture("tex/map/white.png");
    SetTextureFilter(whiteTex, TEXTURE_FILTER_POINT);

    char mapPath[256];
    sprintf(mapPath, "map/level_%d/map.txt", levelIndex);
    Shader dummy = {0};
    Level level = LoadLevel(mapPath, TILE_SIZE, WALL_HEIGHT, floorTex, planksTex, wallTex, greenTex, whiteTex, dummy);

    EditorState state = {};
    state.level = level;
    state.showGrid = true;
    state.previewScale = 1.0f;
    LoadEditorModels(state.models);
    LoadDecorFile(&state, levelIndex);
    LoadEnemyFile(&state, levelIndex);

    state.camera.position = {level.width * TILE_SIZE / 2.0f, 50.0f, level.height * TILE_SIZE / 2.0f};
    state.camera.up = {0, 1, 0};
    state.camera.fovy = 60.0f;
    state.camera.projection = CAMERA_PERSPECTIVE;
    state.yaw = 0;
    state.pitch = -0.5f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        UpdateCamera(&state, dt);
        HandleInput(&state, levelIndex);
        if (state.statusTimer > 0) state.statusTimer -= dt;

        BeginDrawing();
        ClearBackground({20, 20, 30, 255});

        BeginMode3D(state.camera);
        DrawLevel(level);
        DrawEditorObjects(&state);
        if (state.showGrid) DrawGridOverlay(level);
        if (state.selectedObjIdx < 0 && state.selectedEnemyIdx < 0)
            DrawGhost(&state);
        EndMode3D();

        int cx = GetScreenWidth() / 2, cy = GetScreenHeight() / 2;
        DrawLine(cx - 10, cy, cx + 10, cy, WHITE);
        DrawLine(cx, cy - 10, cx, cy + 10, WHITE);

        DrawHUD(&state);
        EndDrawing();
    }

    SaveDecorFile(&state, levelIndex);
    SaveEnemyFile(&state, levelIndex);

    UnloadEditorModels(state.models);
    UnloadLevel(level);
    UnloadTexture(floorTex); UnloadTexture(planksTex);
    UnloadTexture(wallTex); UnloadTexture(greenTex); UnloadTexture(whiteTex);
    CloseWindow();
    return 0;
}
