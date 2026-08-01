#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "config.h"
#include "level.h"
#include "map.h"
#include "light.h"

#define MAX_EDITOR_OBJECTS 256
#define MAX_ENEMIES 256
#define EDITOR_CAM_SPEED 30.0f
#define EDITOR_SENSITIVITY 0.003f
#define GRID_Y 0.05f

enum EditorCategory {
    CAT_DECOR = 0,
    CAT_DOORS,
    CAT_KEYDOOR,
    CAT_ENEMIES,
    CAT_BONUSES,
    CAT_PLAYER,
    CAT_TERRAIN,
    CAT_COUNT
};

struct EditorObject {
    char type[32];
    float col, row, y, rotation, scale;
    bool collision;
    bool isDoor;
    bool isLocked, isExit;
    char wallTexTypeLeft[16];
    char wallTexTypeRight[16];
    int keyId;
};

struct EnemyPlacement {
    int col, row;
    char type;
};

struct BonusPlacement {
    int col, row;
    char type;
    float param;
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
    Model doorBox;
    Model doorCap;
    Texture2D sofaTex, lampTex, bloodTex, trashTex, boxTex;
    Texture2D doorTex;
    Texture2D brickTex, greenWallTex, whiteWallTex;
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
    int doorWallTexLeft;
    int doorWallTexRight;
    Texture2D texZombie, texMilitary, texFast;
    Texture2D texHealth, texKey, texWeapon, texWeapon2;
    bool editingKeyId;
    char keyIdInput[8];
    BonusPlacement bonuses[MAX_BONUSES];
    int bonusCount;
    int selectedBonusIdx;
    bool editingBonusAmount;
    char bonusAmountInput[8];
};

static const char *categoryNames[] = {
    "DECOR", "DOORS", "KEYDOOR", "ENEMIES", "BONUSES", "PLAYER", "TERRAIN"
};

static PaletteItem allItems[] = {
    {"sofa",     "models/sofa.obj",  "tex/decor/sofa.png",  3.5f,  4.0f,  true,  {139,90,43,255},   false, false, false, 0},
    {"lamp",     "models/lamp.obj",  "tex/decor/lamp.png",  19.0f, 0.5f,  false, {255,255,0,255},   false, false, false, 0},
    {"blood",    "models/blood.obj", "tex/decor/blood.png", 0.1f,  6.0f,  false, {200,0,0,255},     false, false, false, 0},
    {"trash",    "models/trash.obj", "tex/decor/trash.png", 3.0f,  3.0f,  true,  {128,128,128,255}, false, false, false, 0},
    {"box",      NULL,               "tex/decor/box.png",   2.5f,  5.0f,  true,  {200,150,50,255},  false, false, false, 0},
    {"door",     NULL, NULL, 0.0f, 1.0f, false, {0,100,255,255},  true, false, false, 0},
    {"door",     NULL, NULL, 0.0f, 1.0f, false, {50,255,50,255},  true, false, true,  0},
    {"door",     NULL, NULL, 0.0f, 1.0f, false, {255,50,50,255},  true, true,  false, 0},
    {"zombie",   NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {255,50,50,255},  false, false, false, 'Z'},
    {"military", NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {80,80,80,255},   false, false, false, 'M'},
    {"fast",     NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {255,150,0,255},  false, false, false, 'F'},
    {"health",   NULL, NULL, BONUS_Y_HEIGHT, 1.0f, false, {255,255,0,255},  false, false, false, 'H'},
    {"key",      NULL, NULL, BONUS_Y_HEIGHT, 1.0f, false, {255,215,0,255},  false, false, false, 0},
    {"weapon",   NULL, NULL, BONUS_Y_HEIGHT, 1.0f, false, {0,150,255,255},  false, false, false, 'W'},
    {"double",   NULL, NULL, BONUS_Y_HEIGHT, 1.0f, false, {0,200,200,255},  false, false, false, 'D'},
    {"player",   NULL, NULL, ZOMBIE_SPAWN_Y, 1.0f, false, {0,255,0,255},    false, false, false, 'P'},
    {"brick",    NULL, NULL, 0.0f, 0.0f, false, {139,90,43,255},   false, false, false, 0},
    {"green",    NULL, NULL, 0.0f, 0.0f, false, {0,150,0,255},     false, false, false, 0},
    {"white",    NULL, NULL, 0.0f, 0.0f, false, {200,200,200,255}, false, false, false, 0},
    {"planks",   NULL, NULL, 0.0f, 0.0f, false, {160,120,60,255},  false, false, false, 0},
    {"floor",    NULL, NULL, 0.0f, 0.0f, false, {100,100,100,255}, false, false, false, 0},
    {"void",     NULL, NULL, 0.0f, 0.0f, false, {50,50,50,255},    false, false, false, 0},
};

static const char terrainChars[] = {'&', '@', '#', '0', '.', ' '};
static int categoryStart[] = {0, 5, 7, 8, 11, 15, 16};
static int categoryCount[] = {5, 2, 1, 3, 4, 1, 6};
static const char *doorWallTexNames[] = {"brick", "green", "white"};
static const int DOOR_WALL_TEX_COUNT = 3;

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

static Vector3 SnapDoor(Vector3 hit) {
    int col = (int)floorf(hit.x / TILE_SIZE);
    int row = (int)floorf(hit.z / TILE_SIZE);
    return TilePos((float)col + 0.5f, (float)row + 0.5f);
}

static Vector3 SnapFine(Vector3 hit, float step = 0.5f) {
    float col = roundf(hit.x / step) * step;
    float row = roundf(hit.z / step) * step;
    return {col, hit.y, row};
}

static Model *GetModel(EditorModels &m, const char *type) {
    if (strcmp(type, "sofa") == 0) return &m.sofa;
    if (strcmp(type, "lamp") == 0) return &m.lamp;
    if (strcmp(type, "blood") == 0) return &m.blood;
    if (strcmp(type, "trash") == 0) return &m.trash;
    if (strcmp(type, "box") == 0) return &m.box;
    return NULL;
}

static Texture2D LoadTexRepeat(const char *path) {
    Texture2D tex = LoadTexture(path);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
    rlTextureParameters(tex.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(tex.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);
    return tex;
}

static void LoadEditorModels(EditorModels &m, Shader shader) {
    m.sofa = LoadModel("models/sofa.obj");
    m.sofaTex = LoadTexture("tex/decor/sofa.png");
    SetTextureFilter(m.sofaTex, TEXTURE_FILTER_POINT);
    m.sofa.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.sofaTex;
    m.sofa.materials[0].shader = shader;

    m.lamp = LoadModel("models/lamp.obj");
    m.lampTex = LoadTexture("tex/decor/lamp.png");
    SetTextureFilter(m.lampTex, TEXTURE_FILTER_POINT);
    m.lamp.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.lampTex;
    m.lamp.materials[0].shader = shader;

    m.blood = LoadModel("models/blood.obj");
    m.bloodTex = LoadTexture("tex/decor/blood.png");
    SetTextureFilter(m.bloodTex, TEXTURE_FILTER_POINT);
    m.blood.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.bloodTex;
    m.blood.materials[0].shader = shader;

    m.trash = LoadModel("models/trash.obj");
    m.trashTex = LoadTexture("tex/decor/trash.png");
    SetTextureFilter(m.trashTex, TEXTURE_FILTER_POINT);
    m.trash.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.trashTex;
    m.trash.materials[0].shader = shader;

    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    m.box = LoadModelFromMesh(cube);
    m.boxTex = LoadTexture("tex/decor/box.png");
    SetTextureFilter(m.boxTex, TEXTURE_FILTER_POINT);
    m.box.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.boxTex;
    m.box.materials[0].shader = shader;

    Mesh doorMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    float *v = doorMesh.vertices;
    for (int i = 0; i < doorMesh.vertexCount * 3; i += 3)
        v[i] += 0.5f;
    UploadMesh(&doorMesh, false);
    m.doorBox = LoadModelFromMesh(doorMesh);
    m.doorTex = LoadTexture("tex/decor/door_1.png");
    SetTextureFilter(m.doorTex, TEXTURE_FILTER_POINT);
    m.doorBox.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.doorTex;
    m.doorBox.materials[0].shader = shader;

    Mesh capMesh = GenMeshCube(TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
    m.doorCap = LoadModelFromMesh(capMesh);
    m.doorCap.materials[0].shader = shader;

    m.brickTex = LoadTexRepeat("tex/map/bricks.png");
    m.greenWallTex = LoadTexRepeat("tex/map/green_wall.png");
    m.whiteWallTex = LoadTexRepeat("tex/map/white_wall.png");
    m.doorCap.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m.brickTex;
}

static void UnloadEditorModels(EditorModels &m) {
    UnloadModel(m.sofa); UnloadTexture(m.sofaTex);
    UnloadModel(m.lamp); UnloadTexture(m.lampTex);
    UnloadModel(m.blood); UnloadTexture(m.bloodTex);
    UnloadModel(m.trash); UnloadTexture(m.trashTex);
    UnloadModel(m.box); UnloadTexture(m.boxTex);
    UnloadModel(m.doorBox); UnloadTexture(m.doorTex);
    UnloadModel(m.doorCap);
    UnloadTexture(m.brickTex); UnloadTexture(m.greenWallTex); UnloadTexture(m.whiteWallTex);
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
        if (sscanf(line, "obj %31s %f %f %f %f %f %d %d",
                   obj.type, &obj.col, &obj.row, &obj.y,
                   &obj.rotation, &obj.scale, (int*)&obj.collision, &obj.keyId) >= 7) {
            if (strcmp(obj.type, "key") == 0) continue;
            if (obj.keyId < 0 || obj.keyId >= MAX_KEYS) obj.keyId = 0;
            s->objects[s->objectCount++] = obj;
        } else if (strncmp(line, "door", 4) == 0) {
            char wl[16] = "brick", wr[16] = "brick";
            obj.keyId = 0;
            if (sscanf(line, "door %f %f %f %d %d %15s %15s %d",
                      &obj.col, &obj.row, &obj.rotation,
                      (int*)&obj.isLocked, (int*)&obj.isExit, wl, wr, &obj.keyId) >= 5) {
                strcpy(obj.type, "door");
                obj.isDoor = true;
                strncpy(obj.wallTexTypeLeft, wl, sizeof(obj.wallTexTypeLeft) - 1);
                obj.wallTexTypeLeft[sizeof(obj.wallTexTypeLeft) - 1] = '\0';
                strncpy(obj.wallTexTypeRight, wr, sizeof(obj.wallTexTypeRight) - 1);
                obj.wallTexTypeRight[sizeof(obj.wallTexTypeRight) - 1] = '\0';
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
            if (o.isLocked) {
                fprintf(f, "door %.1f %.1f %.0f %d %d %s %s %d\n",
                        o.col, o.row, o.rotation, o.isLocked ? 1 : 0, o.isExit ? 1 : 0,
                        o.wallTexTypeLeft[0] ? o.wallTexTypeLeft : "brick",
                        o.wallTexTypeRight[0] ? o.wallTexTypeRight : "brick",
                        o.keyId);
            } else {
                fprintf(f, "door %.1f %.1f %.0f %d %d %s %s\n",
                        o.col, o.row, o.rotation, o.isLocked ? 1 : 0, o.isExit ? 1 : 0,
                        o.wallTexTypeLeft[0] ? o.wallTexTypeLeft : "brick",
                        o.wallTexTypeRight[0] ? o.wallTexTypeRight : "brick");
            }
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
                    if ((c == 'Z' || c == 'M' || c == 'F' || c == 'P') &&
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

static void LoadBonusFile(EditorState *s, int lvl) {
    char path[256];
    sprintf(path, "map/level_%d/bonus.txt", lvl);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f) && s->bonusCount < MAX_BONUSES) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char type[16];
        float col = 0, row = 0;
        int p1 = 0;
        if (sscanf(line, "%15s %f %f %d", type, &col, &row, &p1) < 3) continue;
        BonusPlacement &b = s->bonuses[s->bonusCount];
        b.col = (int)col;
        b.row = (int)row;
        if (strcmp(type, "health") == 0) {
            b.type = 'H';
            b.param = (p1 > 0) ? (float)p1 : 20.0f;
        } else if (strcmp(type, "weapon") == 0) {
            b.type = (p1 == 2) ? 'D' : 'W';
            b.param = (float)((p1 >= 1 && p1 <= 2) ? p1 : 1);
        } else if (strcmp(type, "key") == 0) {
            b.type = 'K';
            b.param = (float)p1;
        } else continue;
        s->bonusCount++;
    }
    fclose(f);
}

static void SaveBonusFile(EditorState *s, int lvl) {
    char path[256];
    sprintf(path, "map/level_%d/bonus.txt", lvl);
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < s->bonusCount; i++) {
        BonusPlacement &b = s->bonuses[i];
        if (b.type == 'H')
            fprintf(f, "health %d %d %d\n", b.col, b.row, (int)b.param);
        else if (b.type == 'K')
            fprintf(f, "key %d %d %d\n", b.col, b.row, (int)b.param);
        else if (b.type == 'W')
            fprintf(f, "weapon %d %d 1\n", b.col, b.row);
        else if (b.type == 'D')
            fprintf(f, "weapon %d %d 2\n", b.col, b.row);
    }
    fclose(f);
}

static void SaveMapFile(EditorState *s, int lvl) {
    char path[256];
    sprintf(path, "map/level_%d/map.txt", lvl);
    FILE *f = fopen(path, "w");
    if (!f) return;
    int w = s->level.width;
    int h = s->level.height;
    for (int r = 0; r < h; r++) {
        int last = -1;
        for (int c = 0; c < w; c++) {
            char ch = s->level.data[r * w + c];
            if (ch != ' ' && ch != '\0') last = c;
        }
        for (int c = 0; c <= last; c++) {
            char ch = s->level.data[r * w + c];
            if (ch == '\0') ch = ' ';
            fprintf(f, "%c", ch);
            if (c < last) fprintf(f, " ");
        }
        fprintf(f, "\n");
    }
    fclose(f);
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
    if (s->category == CAT_BONUSES) {
        if (s->bonusCount >= MAX_BONUSES) return;
        Vector3 pos = SnapToTile(hit);
        BonusPlacement &b = s->bonuses[s->bonusCount++];
        b.col = (int)((pos.x - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        b.row = (int)((pos.z - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        if (strcmp(item->name, "health") == 0) {
            b.type = 'H'; b.param = 20.0f;
        } else if (strcmp(item->name, "key") == 0) {
            b.type = 'K'; b.param = 1.0f;
        } else if (strcmp(item->name, "weapon") == 0) {
            b.type = 'W'; b.param = 1.0f;
        } else if (strcmp(item->name, "double") == 0) {
            b.type = 'D'; b.param = 2.0f;
        } else { s->bonusCount--; return; }
        strcpy(s->statusMsg, TextFormat("Placed %s", item->name));
    } else if (item->enemyChar != 0) {
        if (s->enemyCount >= MAX_ENEMIES) return;
        Vector3 pos = SnapToTile(hit);
        EnemyPlacement &e = s->enemies[s->enemyCount++];
        e.col = (int)((pos.x - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        e.row = (int)((pos.z - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        e.type = item->enemyChar;
        strcpy(s->statusMsg, TextFormat("Placed %s", item->name));
    } else {
        if (s->objectCount >= MAX_EDITOR_OBJECTS) return;
        Vector3 pos;
        if (item->isDoor)
            pos = SnapDoor(hit);
        else
            pos = SnapFine(hit, 0.5f);
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
        o.keyId = o.isLocked ? 1 : 0;
        if (o.isDoor) {
            strncpy(o.wallTexTypeLeft, doorWallTexNames[s->doorWallTexLeft], sizeof(o.wallTexTypeLeft) - 1);
            strncpy(o.wallTexTypeRight, doorWallTexNames[s->doorWallTexRight], sizeof(o.wallTexTypeRight) - 1);
        } else {
            o.wallTexTypeLeft[0] = '\0';
            o.wallTexTypeRight[0] = '\0';
        }
        strcpy(s->statusMsg, TextFormat("Placed %s", item->name));
    }
    s->statusTimer = 1.5f;
}

static void FindClosest(EditorState *s, Vector3 hit, float maxDist2, int &outType, int &outIdx) {
    outType = -1;
    outIdx = -1;
    float best = maxDist2;
    for (int i = 0; i < s->objectCount; i++) {
        Vector3 pos = TilePos(s->objects[i].col, s->objects[i].row, 0.0f);
        float dx = pos.x - hit.x;
        float dz = pos.z - hit.z;
        float d = dx * dx + dz * dz;
        if (d < best) { best = d; outType = 0; outIdx = i; }
    }
    for (int i = 0; i < s->enemyCount; i++) {
        EnemyPlacement &e = s->enemies[i];
        float dx = (float)e.col * TILE_SIZE + TILE_SIZE / 2.0f - hit.x;
        float dz = (float)e.row * TILE_SIZE + TILE_SIZE / 2.0f - hit.z;
        float d = dx * dx + dz * dz;
        if (d < best) { best = d; outType = 1; outIdx = i; }
    }
    for (int i = 0; i < s->bonusCount; i++) {
        BonusPlacement &b = s->bonuses[i];
        float dx = (float)b.col * TILE_SIZE + TILE_SIZE / 2.0f - hit.x;
        float dz = (float)b.row * TILE_SIZE + TILE_SIZE / 2.0f - hit.z;
        float d = dx * dx + dz * dz;
        if (d < best) { best = d; outType = 2; outIdx = i; }
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
    s->selectedBonusIdx = -1;
    s->editingKeyId = false;
    s->editingBonusAmount = false;
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
    } else if (type == 2 && idx >= 0) {
        s->selectedBonusIdx = idx;
        BonusPlacement &b = s->bonuses[idx];
        strcpy(s->statusMsg, TextFormat("Selected: %c bonus (%d, %d)", b.type, b.col, b.row));
        s->statusTimer = 2.0f;
    } else {
        strcpy(s->statusMsg, "Nothing selected");
        s->statusTimer = 1.0f;
    }
}

static void Deselect(EditorState *s) {
    s->selectedObjIdx = -1;
    s->selectedEnemyIdx = -1;
    s->selectedBonusIdx = -1;
    s->editingKeyId = false;
    s->editingBonusAmount = false;
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
    } else if (s->selectedBonusIdx >= 0) {
        s->bonuses[s->selectedBonusIdx] = s->bonuses[s->bonusCount - 1];
        s->bonusCount--;
        strcpy(s->statusMsg, "Deleted bonus");
        s->statusTimer = 1.0f;
        s->selectedBonusIdx = -1;
    }
}

static void MoveSelectedToCrosshair(EditorState *s) {
    Vector3 hit;
    if (!GetFloorHit(s->camera, &hit)) return;
    if (s->selectedObjIdx >= 0) {
        EditorObject &o = s->objects[s->selectedObjIdx];
        Vector3 pos;
        if (o.isDoor)
            pos = SnapDoor(hit);
        else
            pos = SnapFine(hit, 0.5f);
        o.col = (pos.x - TILE_SIZE / 2.0f) / TILE_SIZE;
        o.row = (pos.z - TILE_SIZE / 2.0f) / TILE_SIZE;
    } else if (s->selectedEnemyIdx >= 0) {
        EnemyPlacement &e = s->enemies[s->selectedEnemyIdx];
        Vector3 pos = SnapToTile(hit);
        e.col = (int)((pos.x - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        e.row = (int)((pos.z - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
    } else if (s->selectedBonusIdx >= 0) {
        BonusPlacement &b = s->bonuses[s->selectedBonusIdx];
        Vector3 pos = SnapToTile(hit);
        b.col = (int)((pos.x - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
        b.row = (int)((pos.z - TILE_SIZE / 2.0f) / TILE_SIZE + 0.5f);
    }
}

static void PaintTerrain(EditorState *s, char ch);
static void DrawTerrainHighlight(EditorState *s);

static void HandleInput(EditorState *s, int lvl) {
    if (IsKeyPressed(KEY_TAB)) {
        s->category = (s->category + 1) % CAT_COUNT;
        s->selection = 0;
        s->previewRotation = 0;
        s->previewYOffset = 0;
        s->previewScale = 1.0f;
    }

    if (s->editingKeyId || s->editingBonusAmount) {
        int ch;
        while ((ch = GetCharPressed()) > 0) {
            if (ch >= '0' && ch <= '9') {
                char *input = s->editingKeyId ? s->keyIdInput : s->bonusAmountInput;
                int len = (int)strlen(input);
                if (len < 3) {
                    input[len] = (char)ch;
                    input[len + 1] = '\0';
                }
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            char *input = s->editingKeyId ? s->keyIdInput : s->bonusAmountInput;
            int len = (int)strlen(input);
            if (len > 0) input[len - 1] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            if (s->editingKeyId) {
                int val = atoi(s->keyIdInput);
                if (val < 0) val = 0;
                if (val >= MAX_KEYS) val = MAX_KEYS - 1;
                if (s->selectedObjIdx >= 0)
                    s->objects[s->selectedObjIdx].keyId = val;
                else if (s->selectedBonusIdx >= 0 && s->bonuses[s->selectedBonusIdx].type == 'K')
                    s->bonuses[s->selectedBonusIdx].param = (float)val;
                s->editingKeyId = false;
                DisableCursor();
                strcpy(s->statusMsg, TextFormat("keyId = %d", val));
                s->statusTimer = 1.5f;
            } else {
                int val = atoi(s->bonusAmountInput);
                if (val < 1) val = 1;
                if (s->selectedBonusIdx >= 0) {
                    BonusPlacement &b = s->bonuses[s->selectedBonusIdx];
                    if (b.type == 'K') {
                        if (val >= MAX_KEYS) val = MAX_KEYS - 1;
                        b.param = (float)val;
                        strcpy(s->statusMsg, TextFormat("keyId = %d", val));
                    } else if (b.type == 'H') {
                        if (val > 999) val = 999;
                        b.param = (float)val;
                        strcpy(s->statusMsg, TextFormat("HP = %d", val));
                    }
                }
                s->editingBonusAmount = false;
                DisableCursor();
                s->statusTimer = 1.5f;
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            s->editingKeyId = false;
            s->editingBonusAmount = false;
            DisableCursor();
            strcpy(s->statusMsg, "Cancelled edit");
            s->statusTimer = 1.0f;
        }
        return;
    }

    bool hasSelection = (s->selectedObjIdx >= 0 || s->selectedEnemyIdx >= 0 || s->selectedBonusIdx >= 0);

    if (hasSelection) {
        MoveSelectedToCrosshair(s);

        float wheel = GetMouseWheelMove();
        if (s->selectedObjIdx >= 0) {
            EditorObject &o = s->objects[s->selectedObjIdx];
            if (o.isDoor && IsKeyDown(KEY_LEFT_ALT)) {
                bool right = IsKeyDown(KEY_LEFT_SHIFT);
                char *wt = right ? o.wallTexTypeRight : o.wallTexTypeLeft;
                wt[15] = '\0';
                int cur = 0;
                for (int i = 0; i < DOOR_WALL_TEX_COUNT; i++)
                    if (strcmp(wt, doorWallTexNames[i]) == 0) { cur = i; break; }
                if (wheel > 0) cur = (cur + 1) % DOOR_WALL_TEX_COUNT;
                else if (wheel < 0) cur = (cur - 1 + DOOR_WALL_TEX_COUNT) % DOOR_WALL_TEX_COUNT;
                strncpy(wt, doorWallTexNames[cur], 15);
                wt[15] = '\0';
                strcpy(s->statusMsg, TextFormat("Wall %s: %s", right ? "R" : "L", wt));
                s->statusTimer = 1.5f;
            } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
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
        if (s->selectedObjIdx >= 0 && s->objects[s->selectedObjIdx].isDoor) {
            EditorObject &o = s->objects[s->selectedObjIdx];
            for (int i = 0; i < DOOR_WALL_TEX_COUNT; i++) {
                if (IsKeyPressed(KEY_ONE + i)) {
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        strncpy(o.wallTexTypeRight, doorWallTexNames[i], 15);
                        o.wallTexTypeRight[15] = '\0';
                        strcpy(s->statusMsg, TextFormat("Right wall: %s", o.wallTexTypeRight));
                    } else {
                        strncpy(o.wallTexTypeLeft, doorWallTexNames[i], 15);
                        o.wallTexTypeLeft[15] = '\0';
                        strcpy(s->statusMsg, TextFormat("Left wall: %s", o.wallTexTypeLeft));
                    }
                    s->statusTimer = 1.5f;
                }
            }
        }
        if (s->selectedObjIdx >= 0 && IsKeyPressed(KEY_K)) {
            EditorObject &sel = s->objects[s->selectedObjIdx];
            if (sel.isLocked) {
                s->editingKeyId = true;
                EnableCursor();
                ShowCursor();
                snprintf(s->keyIdInput, sizeof(s->keyIdInput), "%d", sel.keyId);
                strcpy(s->statusMsg, "Enter keyId, Enter to confirm, Esc to cancel");
                s->statusTimer = 3.0f;
            }
        }
        if (s->selectedBonusIdx >= 0 && IsKeyPressed(KEY_K)) {
            BonusPlacement &b = s->bonuses[s->selectedBonusIdx];
            if (b.type == 'K') {
                s->editingBonusAmount = true;
                EnableCursor();
                ShowCursor();
                snprintf(s->bonusAmountInput, sizeof(s->bonusAmountInput), "%d", (int)b.param);
                strcpy(s->statusMsg, "Enter keyId, Enter to confirm, Esc to cancel");
                s->statusTimer = 3.0f;
            } else if (b.type == 'H') {
                s->editingBonusAmount = true;
                EnableCursor();
                ShowCursor();
                snprintf(s->bonusAmountInput, sizeof(s->bonusAmountInput), "%d", (int)b.param);
                strcpy(s->statusMsg, "Enter HP amount, Enter to confirm, Esc to cancel");
                s->statusTimer = 3.0f;
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            Deselect(s);
            strcpy(s->statusMsg, "Confirmed");
            s->statusTimer = 1.0f;
        }
        return;
    }

    if (!s->editingKeyId && !s->editingBonusAmount) {
        if ((s->category == CAT_DOORS || s->category == CAT_KEYDOOR)) {
            for (int i = 0; i < DOOR_WALL_TEX_COUNT; i++) {
                if (IsKeyPressed(KEY_ONE + i)) {
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        s->doorWallTexRight = i;
                        strcpy(s->statusMsg, TextFormat("Right wall: %s", doorWallTexNames[i]));
                    } else {
                        s->doorWallTexLeft = i;
                        strcpy(s->statusMsg, TextFormat("Left wall: %s", doorWallTexNames[i]));
                    }
                    s->statusTimer = 1.5f;
                }
            }
        } else {
            for (int i = 0; i < 9; i++) {
                if (IsKeyPressed(KEY_ONE + i) && i < categoryCount[s->category]) {
                    s->selection = i;
                    s->previewRotation = 0;
                    s->previewYOffset = 0;
                    s->previewScale = 1.0f;
                }
            }
        }
    }
    if (IsKeyPressed(KEY_G)) s->showGrid = !s->showGrid;

    if (s->category == CAT_TERRAIN) {
        PaletteItem *item = CurrentItem(s);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PaintTerrain(s, terrainChars[s->selection]);
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            PaintTerrain(s, ' ');
        }
        if (IsKeyPressed(KEY_ENTER)) {
            SaveDecorFile(s, lvl);
            SaveEnemyFile(s, lvl);
            SaveBonusFile(s, lvl);
            SaveMapFile(s, lvl);
            strcpy(s->statusMsg, "SAVED!");
            s->statusTimer = 2.0f;
        }
        return;
    }

    float wheel = GetMouseWheelMove();
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        s->previewYOffset += wheel * 2.0f;
    } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
        s->previewScale += wheel * 0.1f;
        if (s->previewScale < 0.1f) s->previewScale = 0.1f;
    } else {
        s->previewRotation += wheel * 15.0f;
    }
    if (IsKeyPressed(KEY_E)) SelectAtCrosshair(s);
    if (IsKeyPressed(KEY_K)) {
        Vector3 hit;
        if (GetFloorHit(s->camera, &hit)) {
            int type, idx;
            FindClosest(s, hit, 8.0f * 8.0f, type, idx);
            if (type == 0 && idx >= 0) {
                EditorObject &o = s->objects[idx];
                if (o.isLocked) {
                    s->selectedObjIdx = idx;
                    s->editingKeyId = true;
                    EnableCursor();
                    ShowCursor();
                    snprintf(s->keyIdInput, sizeof(s->keyIdInput), "%d", o.keyId);
                    strcpy(s->statusMsg, "Enter keyId, Enter to confirm, Esc to cancel");
                    s->statusTimer = 3.0f;
                } else {
                    strcpy(s->statusMsg, "Not a locked door");
                    s->statusTimer = 1.5f;
                }
            } else if (type == 2 && idx >= 0) {
                BonusPlacement &b = s->bonuses[idx];
                s->selectedBonusIdx = idx;
                if (b.type == 'K') {
                    s->editingBonusAmount = true;
                    EnableCursor();
                    ShowCursor();
                    snprintf(s->bonusAmountInput, sizeof(s->bonusAmountInput), "%d", (int)b.param);
                    strcpy(s->statusMsg, "Enter keyId, Enter to confirm, Esc to cancel");
                    s->statusTimer = 3.0f;
                } else if (b.type == 'H') {
                    s->editingBonusAmount = true;
                    EnableCursor();
                    ShowCursor();
                    snprintf(s->bonusAmountInput, sizeof(s->bonusAmountInput), "%d", (int)b.param);
                    strcpy(s->statusMsg, "Enter HP amount, Enter to confirm, Esc to cancel");
                    s->statusTimer = 3.0f;
                } else {
                    strcpy(s->statusMsg, "Only key/health bonuses are editable");
                    s->statusTimer = 1.5f;
                }
            } else {
                strcpy(s->statusMsg, "Nothing editable under crosshair");
                s->statusTimer = 1.5f;
            }
        }
    }
    if (!s->editingKeyId && !s->editingBonusAmount && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) PlaceObject(s);
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
            } else if (type == 2 && idx >= 0) {
                s->bonuses[idx] = s->bonuses[s->bonusCount - 1];
                s->bonusCount--;
                strcpy(s->statusMsg, "Deleted bonus");
                s->statusTimer = 1.0f;
            }
        }
    }
    if (IsKeyPressed(KEY_ENTER)) {
        SaveDecorFile(s, lvl);
        SaveEnemyFile(s, lvl);
        SaveBonusFile(s, lvl);
        strcpy(s->statusMsg, "SAVED!");
        s->statusTimer = 2.0f;
    }
}

static void DrawGridOverlay(Level level) {
    float ts = level.tileSize;
    float w = level.width * ts;
    float h = level.height * ts;
    float step = 0.5f;

    Color fine = ColorAlpha(WHITE, 0.08f);
    Color tile = ColorAlpha(GREEN, 0.3f);

    for (float x = 0; x <= w; x += step) {
        Color c = (fmodf(x, ts) < 0.01f) ? tile : fine;
        DrawLine3D({x, GRID_Y, 0}, {x, GRID_Y, h}, c);
    }
    for (float z = 0; z <= h; z += step) {
        Color c = (fmodf(z, ts) < 0.01f) ? tile : fine;
        DrawLine3D({0, GRID_Y, z}, {w, GRID_Y, z}, c);
    }
}

static bool GetTerrainTileAtCursor(EditorState *s, int &outCol, int &outRow) {
    Vector3 hit;
    if (!GetFloorHit(s->camera, &hit)) return false;
    int col = (int)floorf(hit.x / s->level.tileSize);
    int row = (int)floorf(hit.z / s->level.tileSize);
    if (col < 0 || col >= s->level.width || row < 0 || row >= s->level.height) return false;
    outCol = col;
    outRow = row;
    return true;
}

static void PaintTerrain(EditorState *s, char ch) {
    int col, row;
    if (!GetTerrainTileAtCursor(s, col, row)) return;
    s->level.data[row * s->level.width + col] = ch;
}

static void DrawTerrainHighlight(EditorState *s) {
    int col, row;
    if (!GetTerrainTileAtCursor(s, col, row)) return;
    float ts = s->level.tileSize;
    float cx = col * ts + ts / 2.0f;
    float cz = row * ts + ts / 2.0f;
    char cur = s->level.data[row * s->level.width + col];
    PaletteItem *item = CurrentItem(s);
    Color hc = YELLOW;
    if (terrainChars[s->selection] == cur) hc = GREEN;
    DrawCubeWires({cx, 0.3f, cz}, ts + 0.1f, 0.5f, ts + 0.1f, hc);
}

static Texture2D GetCapTexture(EditorModels &m, const char *wt) {
    if (strcmp(wt, "green") == 0) return m.greenWallTex;
    if (strcmp(wt, "white") == 0) return m.whiteWallTex;
    return m.brickTex;
}

static void DrawDoorCaps(EditorState *s, Vector3 doorPos, float rotation, const char *leftTex, const char *rightTex, Color tint) {
    float rad = rotation * 3.14159f / 180.0f;
    float sx = sinf(rad);
    float sz = cosf(rad);
    float hw = TILE_SIZE / 2.0f;

    Model capL = s->models.doorCap;
    capL.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = GetCapTexture(s->models, leftTex);
    Vector3 capLPos = {doorPos.x - sx * hw, DOOR_CAP_Y_MULT * TILE_SIZE, doorPos.z - sz * hw};
    DrawModelEx(capL, capLPos, {0,1,0}, rotation, {1,1,1}, tint);

    Model capR = s->models.doorCap;
    capR.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = GetCapTexture(s->models, rightTex);
    Vector3 capRPos = {doorPos.x + sx * hw, DOOR_CAP_Y_MULT * TILE_SIZE, doorPos.z + sz * hw};
    DrawModelEx(capR, capRPos, {0,1,0}, rotation, {1,1,1}, tint);
}

static void DrawEditorBillboard(Camera3D camera, Vector3 pos, Texture2D tex, float size, Color tint) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, pos));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, {0, 1.0f, 0.0f}));
    Vector3 up = {0, 1.0f, 0};
    Vector3 bl = Vector3Subtract(pos, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
    Vector3 br = Vector3Add(pos, Vector3Subtract(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
    Vector3 tr = Vector3Add(pos, Vector3Add(Vector3Scale(right, size * 0.5f), Vector3Scale(up, size * 0.5f)));
    Vector3 tl = Vector3Add(pos, Vector3Subtract(Vector3Scale(up, size * 0.5f), Vector3Scale(right, size * 0.5f)));
    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(bl.x, bl.y, bl.z);
        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(br.x, br.y, br.z);
        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(tr.x, tr.y, tr.z);
        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(tl.x, tl.y, tl.z);
    rlEnd();
    rlSetTexture(0);
}

static Texture2D GetEnemyTexture(EditorState *s, char type) {
    switch (type) {
        case 'Z': return s->texZombie;
        case 'M': return s->texMilitary;
        case 'F': return s->texFast;
        case 'H': return s->texHealth;
        case 'K': return s->texKey;
        case 'W': return s->texWeapon;
        case 'D': return s->texWeapon2;
        default: return s->texZombie;
    }
}

static void DrawEditorObjects(EditorState *s) {
    for (int i = 0; i < s->objectCount; i++) {
        EditorObject &o = s->objects[i];
        Vector3 pos = TilePos(o.col, o.row, o.y);
        Color tint = IsSelectedObj(s, i) ? YELLOW : WHITE;
        if (o.isDoor) {
            Color dc = o.isExit ? GREEN : (o.isLocked ? RED : BLUE);
            if (IsSelectedObj(s, i)) dc = YELLOW;
            DrawModelEx(s->models.doorBox, pos, {0,1,0}, o.rotation, {5.0f, 15.0f, 1.0f}, dc);
            DrawDoorCaps(s, pos, o.rotation, o.wallTexTypeLeft, o.wallTexTypeRight, WHITE);
        } else {
            if (strcmp(o.type, "key") == 0) {
                DrawEditorBillboard(s->camera, pos, s->texKey, ZOMBIE_BILLBOARD_SIZE, tint);
            } else {
                Model *mdl = GetModel(s->models, o.type);
                if (mdl) DrawModelEx(*mdl, pos, {0, 1, 0}, o.rotation,
                                     {o.scale, o.scale, o.scale}, tint);
            }
        }
        if (IsSelectedObj(s, i)) {
            DrawModelEx(s->models.doorBox, pos, {0,1,0}, o.rotation, {6.0f, 16.0f, 2.0f}, YELLOW);
        }
    }
    for (int i = 0; i < s->enemyCount; i++) {
        EnemyPlacement &e = s->enemies[i];
        Vector3 pos = {(float)e.col * TILE_SIZE + TILE_SIZE / 2.0f, ZOMBIE_SPAWN_Y,
                       (float)e.row * TILE_SIZE + TILE_SIZE / 2.0f};
        if (e.type == 'P') {
            DrawSphere(pos, 2.0f, GREEN);
        } else {
            Texture2D tex = GetEnemyTexture(s, e.type);
            DrawEditorBillboard(s->camera, pos, tex, ZOMBIE_BILLBOARD_SIZE, WHITE);
        }
        if (IsSelectedEnemy(s, i)) {
            DrawSphereWires(pos, 3.0f, 8, 8, YELLOW);
        }
    }
    for (int i = 0; i < s->bonusCount; i++) {
        BonusPlacement &b = s->bonuses[i];
        Vector3 pos = {(float)b.col * TILE_SIZE + TILE_SIZE / 2.0f, BONUS_Y_HEIGHT,
                       (float)b.row * TILE_SIZE + TILE_SIZE / 2.0f};
        Texture2D tex;
        switch (b.type) {
            case 'H': tex = s->texHealth; break;
            case 'K': tex = s->texKey; break;
            case 'W': tex = s->texWeapon; break;
            case 'D': tex = s->texWeapon2; break;
            default: tex = s->texHealth; break;
        }
        Color tint = (s->selectedBonusIdx == i) ? YELLOW : WHITE;
        DrawEditorBillboard(s->camera, pos, tex, ZOMBIE_BILLBOARD_SIZE, tint);
        if (s->selectedBonusIdx == i) {
            DrawSphereWires(pos, 3.0f, 8, 8, YELLOW);
        }
    }
}

static void DrawGhost(EditorState *s) {
    if (s->category == CAT_TERRAIN) return;
    Vector3 hit;
    if (!GetFloorHit(s->camera, &hit)) return;
    PaletteItem *item = CurrentItem(s);
    Vector3 pos;
    if (item->enemyChar != 0)
        pos = SnapToTile(hit);
    else if (item->isDoor)
        pos = SnapDoor(hit);
    else
        pos = SnapFine(hit, 0.5f);
    pos.y = item->defaultY + s->previewYOffset;
    Color tint = ColorAlpha(item->color, 0.7f);
    if (s->category == CAT_BONUSES) {
        Texture2D tex;
        if (strcmp(item->name, "health") == 0) tex = s->texHealth;
        else if (strcmp(item->name, "key") == 0) tex = s->texKey;
        else if (strcmp(item->name, "weapon") == 0) tex = s->texWeapon;
        else if (strcmp(item->name, "double") == 0) tex = s->texWeapon2;
        else tex = s->texHealth;
        DrawEditorBillboard(s->camera, pos, tex, ZOMBIE_BILLBOARD_SIZE, ColorAlpha(WHITE, 0.7f));
    } else if (item->enemyChar != 0) {
        if (item->enemyChar == 'P') {
            DrawSphere(pos, 2.0f, ColorAlpha(GREEN, 0.7f));
        } else {
            Texture2D tex = GetEnemyTexture(s, item->enemyChar);
            DrawEditorBillboard(s->camera, pos, tex, ZOMBIE_BILLBOARD_SIZE, ColorAlpha(WHITE, 0.7f));
        }
    } else if (strcmp(item->name, "key") == 0) {
        DrawEditorBillboard(s->camera, pos, s->texKey, ZOMBIE_BILLBOARD_SIZE, ColorAlpha(GOLD, 0.7f));
    } else if (item->isDoor) {
        DrawModelEx(s->models.doorBox, pos, {0,1,0}, s->previewRotation, {5.0f, 15.0f, 1.0f}, tint);
        const char *lName = doorWallTexNames[s->doorWallTexLeft];
        const char *rName = doorWallTexNames[s->doorWallTexRight];
        DrawDoorCaps(s, pos, s->previewRotation, lName, rName, ColorAlpha(WHITE, 0.7f));
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
    bool hasSel = (s->selectedObjIdx >= 0 || s->selectedEnemyIdx >= 0 || s->selectedBonusIdx >= 0);

    DrawText("RED HORIZON - LEVEL EDITOR", x, y, 20, WHITE); y += lh + 10;

    if (s->editingKeyId) {
        DrawText("[ EDIT KEY ID ]", x, y, 18, GOLD); y += lh;
        DrawText("Type digits, Enter to confirm, Esc to cancel", x, y, 14, LIGHTGRAY); y += lh + 10;
        DrawRectangle(x, y, 200, 36, ColorAlpha(BLACK, 0.8f));
        DrawRectangleLines(x, y, 200, 36, GOLD);
        DrawText(TextFormat("keyId: %s_", s->keyIdInput), x + 8, y + 8, 20, GOLD);
        y += 50;
    } else if (s->editingBonusAmount) {
        DrawText("[ EDIT BONUS ]", x, y, 18, GOLD); y += lh;
        DrawText("Type digits, Enter to confirm, Esc to cancel", x, y, 14, LIGHTGRAY); y += lh + 10;
        DrawRectangle(x, y, 200, 36, ColorAlpha(BLACK, 0.8f));
        DrawRectangleLines(x, y, 200, 36, GOLD);
        DrawText(TextFormat("value: %s_", s->bonusAmountInput), x + 8, y + 8, 20, GOLD);
        y += 50;
    } else if (hasSel) {
        DrawText("[ MOVING MODE ]", x, y, 18, YELLOW); y += lh;
        DrawText("Object follows crosshair", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Wheel - rotate | Shift+Wheel - Y | Ctrl+Wheel - scale", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Enter - confirm | Esc - cancel | Del - delete", x, y, 14, LIGHTGRAY); y += lh - 2;
        if (s->selectedObjIdx >= 0) {
            EditorObject &sel = s->objects[s->selectedObjIdx];
            if (sel.isLocked) {
                DrawText(TextFormat("keyId: %d  [K to edit]", sel.keyId), x, y, 16, GOLD); y += lh;
            }
        }
        if (s->selectedBonusIdx >= 0) {
            BonusPlacement &b = s->bonuses[s->selectedBonusIdx];
            const char *typeName = "?";
            if (b.type == 'H') typeName = "health";
            else if (b.type == 'K') typeName = "key";
            else if (b.type == 'W') typeName = "weapon";
            else if (b.type == 'D') typeName = "double";
            if (b.type == 'K')
                DrawText(TextFormat("Bonus: %s keyId=%d  [K to edit]", typeName, (int)b.param), x, y, 16, GOLD);
            else if (b.type == 'H')
                DrawText(TextFormat("Bonus: %s HP=%d  [K to edit]", typeName, (int)b.param), x, y, 16, GOLD);
            else
                DrawText(TextFormat("Bonus: %s", typeName), x, y, 16, GOLD);
            y += lh;
        }
    } else if (s->category == CAT_TERRAIN) {
        DrawText(TextFormat("Category: %s", categoryNames[s->category]), x, y, 16, YELLOW); y += lh;
        PaletteItem *item = CurrentItem(s);
        DrawText(TextFormat("Tool: %s (%c)", item->name, terrainChars[s->selection]), x, y, 16, GREEN); y += lh;
        DrawText(TextFormat("Tile: %dx%d", s->level.width, s->level.height), x, y, 16, LIGHTGRAY); y += lh + 10;
        DrawText("CONTROLS:", x, y, 16, WHITE); y += lh;
        DrawText("WASD/Space/Shift - Move camera", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Mouse - Look around", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Tab - Cycle category", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("1-6 - Select terrain type", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Left Click - Paint tile", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Right Click - Erase (void)", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("G - Toggle grid | Enter - Save all", x, y, 14, LIGHTGRAY);
    } else {
        DrawText(TextFormat("Category: %s", categoryNames[s->category]), x, y, 16, YELLOW); y += lh;
        PaletteItem *item = CurrentItem(s);
        DrawText(TextFormat("Selected: %s", item->name), x, y, 16, GREEN); y += lh;
        if ((s->category == CAT_DOORS || s->category == CAT_KEYDOOR)) {
            DrawText(TextFormat("Left wall:  %s (1/2/3)", doorWallTexNames[s->doorWallTexLeft]), x, y, 16, SKYBLUE); y += lh;
            DrawText(TextFormat("Right wall: %s (Shift+1/2/3)", doorWallTexNames[s->doorWallTexRight]), x, y, 16, SKYBLUE); y += lh;
        }
        DrawText(TextFormat("Objects: %d | Enemies: %d | Bonuses: %d", s->objectCount, s->enemyCount, s->bonusCount), x, y, 16, LIGHTGRAY); y += lh + 10;
        DrawText("CONTROLS:", x, y, 16, WHITE); y += lh;
        DrawText("WASD/Space/Shift - Move camera", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Mouse - Look around", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Tab - Cycle category", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("1-9 - Select item in palette", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("E - Select object", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Left Click - Place new object", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Right Click - Delete under crosshair", x, y, 14, LIGHTGRAY); y += lh - 2;
        DrawText("Wheel - rotate | Shift+Wheel - Y | Ctrl+Wheel - scale", x, y, 14, LIGHTGRAY); y += lh - 2;
        if (s->category == CAT_KEYDOOR || s->category == CAT_BONUSES) {
            DrawText("K - Edit keyId (for key/locked door)", x, y, 14, GOLD); y += lh - 2;
        }
        if ((s->category == CAT_DOORS || s->category == CAT_KEYDOOR)) {
            DrawText("Alt+Wheel - left wall | Alt+Shift+Wheel - right wall", x, y, 14, SKYBLUE); y += lh - 2;
        }
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
    rlDisableBackfaceCulling();

    Texture2D floorTex = LoadTexRepeat("tex/map/floor.png");
    Texture2D planksTex = LoadTexRepeat("tex/map/planks.png");
    Texture2D wallTex = LoadTexRepeat("tex/map/bricks.png");
    Texture2D greenTex = LoadTexRepeat("tex/map/green_wall.png");
    Texture2D whiteTex = LoadTexRepeat("tex/map/white_wall.png");

    char mapPath[256];
    sprintf(mapPath, "map/level_%d/map.txt", levelIndex);

    Shader shader = LoadLightShader();
    Level level = LoadLevel(mapPath, TILE_SIZE, WALL_HEIGHT, floorTex, planksTex, wallTex, greenTex, whiteTex, shader);
    float ambient = 1.0f;
    SetLightUniforms(shader, {0, 0, 0}, {1, 1, 1}, LIGHT_RANGE, ambient);

    EditorState state = {};
    state.level = level;
    state.showGrid = true;
    state.previewScale = 1.0f;
    state.selectedObjIdx = -1;
    state.selectedEnemyIdx = -1;
    state.doorWallTexLeft = 0;
    state.doorWallTexRight = 0;
    state.texZombie = LoadTexture("tex/zombi/zombi.png");
    state.texMilitary = LoadTexture("tex/zombie_military/zombie_military.png");
    state.texFast = LoadTexture("tex/zombi_fast/zombi_fast.png");
    state.texHealth = LoadTexture("tex/bonus/medic.png");
    state.texKey = LoadTexture("tex/bonus/key.png");
    state.texWeapon = LoadTexture("tex/weapons/gun.png");
    state.texWeapon2 = LoadTexture("tex/weapons/gun_1.png");
    SetTextureFilter(state.texZombie, TEXTURE_FILTER_POINT);
    SetTextureFilter(state.texMilitary, TEXTURE_FILTER_POINT);
    SetTextureFilter(state.texFast, TEXTURE_FILTER_POINT);
    SetTextureFilter(state.texHealth, TEXTURE_FILTER_POINT);
    SetTextureFilter(state.texKey, TEXTURE_FILTER_POINT);
    SetTextureFilter(state.texWeapon, TEXTURE_FILTER_POINT);
    SetTextureFilter(state.texWeapon2, TEXTURE_FILTER_POINT);
    LoadEditorModels(state.models, shader);
    LoadDecorFile(&state, levelIndex);
    LoadEnemyFile(&state, levelIndex);
    LoadBonusFile(&state, levelIndex);

    state.camera.position = {level.width * TILE_SIZE / 2.0f, 50.0f, level.height * TILE_SIZE / 2.0f};
    state.camera.up = {0, 1, 0};
    state.camera.fovy = 60.0f;
    state.camera.projection = CAMERA_PERSPECTIVE;
    state.yaw = 0;
    state.pitch = -0.5f;

    while (true) {
        if (WindowShouldClose()) {
            if (state.selectedObjIdx >= 0 || state.selectedEnemyIdx >= 0 || state.selectedBonusIdx >= 0) {
                Deselect(&state);
                strcpy(state.statusMsg, "Deselected (Esc to quit)");
                state.statusTimer = 1.0f;
            } else {
                break;
            }
        }
        float dt = GetFrameTime();
        UpdateCamera(&state, dt);
        HandleInput(&state, levelIndex);
        if (state.statusTimer > 0) state.statusTimer -= dt;

        BeginDrawing();
        ClearBackground({20, 20, 30, 255});

        SetLightUniforms(shader, state.camera.position, {1, 1, 1}, LIGHT_RANGE, ambient);

        BeginMode3D(state.camera);
        DrawLevel(level, false);
        DrawEditorObjects(&state);
        if (state.showGrid) DrawGridOverlay(level);
        if (state.category == CAT_TERRAIN) DrawTerrainHighlight(&state);
        if (state.selectedObjIdx < 0 && state.selectedEnemyIdx < 0 && state.selectedBonusIdx < 0)
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
    SaveBonusFile(&state, levelIndex);

    UnloadEditorModels(state.models);
    UnloadTexture(state.texZombie); UnloadTexture(state.texMilitary); UnloadTexture(state.texFast);
    UnloadTexture(state.texHealth); UnloadTexture(state.texKey); UnloadTexture(state.texWeapon); UnloadTexture(state.texWeapon2);
    UnloadLevel(level);
    UnloadTexture(floorTex); UnloadTexture(planksTex);
    UnloadTexture(wallTex); UnloadTexture(greenTex); UnloadTexture(whiteTex);
    CloseWindow();
    return 0;
}
