#include "croma.h"
#include "raylib-6.0_linux_amd64/include/raylib.h"
#include "raylib-6.0_linux_amd64/include/raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 600
#define HEIGHT 400

typedef struct {
        Vector2 world;
        Vector2 velocity;
        float radius;
        Color color;
        struct {
                bool hold;
                Vector2 prev;
                double prev_t;
                Vector2 acel;
        } misc;
} Obj;

typedef struct {
        Vector2 pos;
} Cam;

// world to view
Vector2
WTV(Cam *cam, Vector2 from)
{
        Vector2 offset = GetWindowPosition();
        Vector2 ret = (Vector2) {
                .x = from.x - offset.x - cam->pos.x,
                .y = from.y - offset.y - cam->pos.y,
        };
        return ret;
}

// view to world
Vector2
VTW(Cam *cam, Vector2 from)
{
        Vector2 offset = GetWindowPosition();
        Vector2 ret = (Vector2) {
                .x = from.x + offset.x + cam->pos.x,
                .y = from.y + offset.y + cam->pos.y,
        };
        return ret;
}


void
obj_draw(Cam *cam, Obj *o)
{
        DrawCircleV(WTV(cam, o->world), o->radius, o->color);
}

void
recalc_obj_positions(Cam *cam, Obj *o)
{
        static bool some_hold = false;
        // hold
        if (!some_hold && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                o->misc.hold = o->misc.hold || CheckCollisionPointCircle(GetMousePosition(), WTV(cam, o->world), o->radius);
                some_hold = o->misc.hold;
                o->misc.prev = GetMousePosition();
                o->misc.prev_t = GetTime();
                o->misc.acel = (Vector2) { 0, 0 };
        }

        // throw (release ball)
        if (o->misc.hold && IsMouseButtonUp(MOUSE_LEFT_BUTTON)) {
                o->misc.hold = false;
                some_hold = false;
                o->velocity.x = o->misc.acel.x;
                o->velocity.y = o->misc.acel.y;
        }

        // follow mouse and calc acel vector for throw
        if (o->misc.hold) {
                Vector2 mouse_pos = GetMousePosition();
                o->world = VTW(cam, mouse_pos);
                double dpt = 0.006 / (GetTime() - o->misc.prev_t);
                o->misc.acel.x += (GetMousePosition().x - o->misc.prev.x) * dpt;
                o->misc.acel.y += (GetMousePosition().y - o->misc.prev.y) * dpt;
                o->misc.acel.x += -o->misc.acel.x * dpt;
                o->misc.acel.y += -o->misc.acel.y * dpt;
                o->misc.prev = mouse_pos;
                o->misc.prev_t = GetTime();
        }

        // free bouncing
        else {
                if (o->velocity.y < 1) o->velocity.y += 0.3 * GetFrameTime() * 70;
                if (o->velocity.x != 0) o->velocity.x -= o->velocity.x * 0.002 * GetFrameTime() * 70;
                o->velocity.y += 1 * GetFrameTime() * 70;
                o->world.x += o->velocity.x * GetFrameTime() * 70;
                o->world.y += o->velocity.y * GetFrameTime() * 70;

                Vector2 off = GetWindowPosition();
                Vector2 wsz = (Vector2) { GetScreenWidth(), GetScreenHeight() };

                if (o->world.x < off.x + o->radius) {
                        o->velocity.x = fabs(o->velocity.x);
                        o->world.x = off.x + o->radius;
                }
                if (o->radius + o->world.x - off.x > wsz.x) {
                        o->velocity.x = -fabs(o->velocity.x);
                        o->world.x = wsz.x + off.x - o->radius;
                }
                if (o->world.y < o->radius + off.y) {
                        o->velocity.y = fabs(o->velocity.y);
                        o->world.y = o->radius + off.y;
                }
                if (o->radius + o->world.y - off.y > wsz.y) {
                        o->velocity.y = -fabs(o->velocity.y);
                        o->world.y = wsz.y + off.y - o->radius;
                }
        }
}

void
check_collisions(Cam *cam, Obj *o1, Obj *o2)
{
        if (o1->misc.hold || o2->misc.hold) return;
        if (CheckCollisionCircles(o1->world, o1->radius, o2->world, o2->radius)) {
                Vector2 tmp = o1->velocity;
                o1->velocity = o2->velocity;
                o2->velocity = tmp;
                memset(&o1->misc, 0, sizeof o1->misc);
                memset(&o2->misc, 0, sizeof o2->misc);
                while (CheckCollisionCircles(o1->world, o1->radius, o2->world, o2->radius)) {
                        o1->world.x += o1->velocity.x * 0.1;
                        o1->world.y += o1->velocity.y * 0.1;
                        o2->world.x += o2->velocity.x * 0.1;
                        o2->world.y += o2->velocity.y * 0.1;
                }
        }
}

int
main(int argc, char **argv)
{
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
        InitWindow(600, 400, "Balls, again");
        SetTargetFPS(60);

        DA(Obj)
        objs = { 0 };

        da_append(&objs, (Obj) {
                         .color = RED,
                         .radius = 40,
                         .world = (Vector2) { WIDTH / 2., HEIGHT / 2. },
                         .velocity = (Vector2) { 10, 10 },
                         });

        Cam c = { 0 };


        while (!WindowShouldClose()) {
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                        da_append(&objs, (Obj) {
                                         .color = BLUE,
                                         .radius = 40,
                                         .world = VTW(&c, GetMousePosition()),
                                         .velocity = (Vector2) { 0, 10 },
                                         });
                }

                for_da_each(o, &objs)
                {
                        recalc_obj_positions(&c, o);
                }

                for (Obj *o1 = objs.items; (int) ((o1) -objs.items) < objs.count; ++o1) {
                        for (Obj *o2 = o1 + 1; (int) ((o2) -objs.items) < objs.count; ++o2) {
                                check_collisions(&c, o1, o2);
                        }
                }

                BeginDrawing();
                ClearBackground(BLACK);

                for_da_each(o, &objs)
                {
                        obj_draw(&c, o);
                }
                EndDrawing();
        }


        return 0;
}
