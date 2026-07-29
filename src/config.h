#pragma once

constexpr float TILE_SIZE        = 5.0f;
constexpr float WALL_HEIGHT      = 20.0f;
constexpr float CAMERA_FOVY      = 60.0f;

constexpr float MOUSE_SENSITIVITY = 0.003f;
constexpr float ARROW_YAW_SPEED   = 2.0f;

constexpr float PLAYER_SPEED          = 20.0f;
constexpr float PLAYER_RADIUS         = 1.0f;
constexpr float PLAYER_SPRINT_MULT    = 1.5f;

constexpr int   HEALTH_MAX            = 100;
constexpr float TOUCH_DAMAGE          = 10.0f;
constexpr float FAST_TOUCH_DAMAGE     = 5.0f;
constexpr float TOUCH_INTERVAL        = 0.3f;
constexpr float HIT_FLASH_DURATION    = 0.2f;
constexpr float HIT_SHAKE_DURATION    = 0.15f;
constexpr float ZOMBIE_HIT_TIME       = 0.15f;

constexpr float ZOMBIE_SPAWN_Y        = 5.4f;
constexpr float ZOMBIE_MODEL_SIZE     = 5.4f;
constexpr float ZOMBIE_BILLBOARD_SIZE = 10.8f;
constexpr float ZOMBIE_HEALTH         = 100.0f;
constexpr float ZOMBIE_SPEED          = 12.0f;
constexpr float FAST_ZOMBIE_HEALTH    = 100.0f;
constexpr float FAST_ZOMBIE_SPEED     = 24.0f;
constexpr float ZOMBIE_RADIUS         = 1.5f;
constexpr float ZOMBIE_PATH_RECALC    = 0.3f;
constexpr float ZOMBIE_ANIM_INTERVAL  = 0.3f;
constexpr float ZOMBIE_HIT_RADIUS     = 5.5f;
constexpr float ZOMBIE_MIL_SHOOT_CD   = 2.0f;

constexpr float DOOR_TRIGGER_RADIUS   = 8.0f;
constexpr float DOOR_HEIGHT_MULT      = 3.0f;
constexpr float DOOR_CAP_Y_MULT       = 3.5f;
constexpr float DOOR_DECAL_Y_OFFSET   = 0.3f;
constexpr float DOOR_OPEN_ANGLE       = 90.0f;
constexpr float DOOR_SPEED            = 3.0f;

constexpr float LIGHT_RANGE           = 80.0f;
constexpr float LIGHT_AMBIENT         = 0.15f;
constexpr float LAMP_Y_OFFSET         = 5.0f;
constexpr float LAMP_RANGE            = 30.0f;

constexpr float WEAPON_DECAL_SIZE     = 0.6f;
constexpr int   MAX_BULLET_HOLES      = 50;

constexpr float BOB_SPEED_WALK        = 8.0f;
constexpr float BOB_SPEED_SPRINT      = 12.0f;
constexpr float BOB_AMPLITUDE_X       = 6.0f;
constexpr float BOB_AMPLITUDE_Y       = 8.0f;
constexpr float BOB_DECAY             = 0.85f;
constexpr float BOB_THRESHOLD         = 0.01f;

constexpr float GUN_BASE_SCALE        = 5.0f;
constexpr float GUN_KICK_MULT         = 12.0f;
constexpr float GUN_SCALE_BOOST       = 0.2f;
constexpr float GUN_Y_OFFSET          = 50.0f;

constexpr float BONUS_BOB_SPEED       = 3.0f;
constexpr float BONUS_PICKUP_RADIUS   = 3.5f;
constexpr float BONUS_HEALTH_AMOUNT   = 20.0f;
constexpr float BONUS_Y_HEIGHT        = 5.0f;
constexpr float BONUS_BOB_AMPLITUDE   = 1.0f;
constexpr float BONUS_SIZE            = 4.0f;

constexpr int   RAYCAST_MAX_STEPS     = 200;
constexpr int   ZOMBIE_MAX_PATH      = 256;
constexpr int   MAX_ZOMBIE_SPAWNS     = 64;
constexpr int   MAX_DOOR_SPAWNS       = 8;
constexpr int   MAX_BONUSES           = 64;
constexpr int   MAX_DOORS             = 8;
constexpr int   SCENE_MAX_OBJECTS     = 64;
constexpr int   SCENE_MAX_ZOMBIES     = 64;
constexpr int   WEAPON_COUNT          = 3;

constexpr int   BOX_PARTICLE_COUNT    = 10;
constexpr float BOX_PARTICLE_SPEED    = 15.0f;
constexpr float BOX_PARTICLE_LIFETIME = 0.8f;
constexpr float BOX_PARTICLE_SIZE     = 1.2f;
constexpr float BOX_PARTICLE_GRAVITY  = 25.0f;
constexpr int   MAX_KEYS              = 8;
constexpr int   MAX_PARTICLES         = 64;
