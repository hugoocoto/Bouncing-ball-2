#include "raylib-6.0_linux_amd64/include/raylib.h"
#include "raylib-6.0_linux_amd64/include/raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 600
#define HEIGHT 400

typedef struct {
        Vector2 world;
        Vector2 velocity;
        float radius;
        Color color;
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

int
main(int argc, char **argv)
{
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
        InitWindow(600, 400, "Balls, again");
        SetTargetFPS(60);

        Obj o = {
                .color = RED,
                .radius = 40,
                .world = (Vector2) { WIDTH / 2., HEIGHT / 2. },
                .velocity = (Vector2) { 10, 10 },
        };
        Cam c = { 0 };

        bool hold = false;
        Vector2 prev;
        double prev_t;
        Vector2 acel;


        while (!WindowShouldClose()) {
                // hold
                if (!hold && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                        hold = hold || CheckCollisionPointCircle(GetMousePosition(), WTV(&c, o.world), o.radius);
                        prev = GetMousePosition();
                        prev_t = GetTime();
                        acel = (Vector2) { 0, 0 };
                }

                // throw (release ball)
                if (hold && IsMouseButtonUp(MOUSE_LEFT_BUTTON)) {
                        hold = false;
                        o.velocity.x = acel.x;
                        o.velocity.y = acel.y;
                }

                // follow mouse and calc acel vector for throw
                if (hold) {
                        Vector2 mouse_pos = GetMousePosition();
                        o.world = VTW(&c, mouse_pos);
                        double dpt = 0.006 / (GetTime() - prev_t);
                        acel.x += (GetMousePosition().x - prev.x) * dpt;
                        acel.y += (GetMousePosition().y - prev.y) * dpt;
                        acel.x += -acel.x * dpt;
                        acel.y += -acel.y * dpt;
                        prev = mouse_pos;
                        prev_t = GetTime();
                }

                // free bouncing
                else {
                        if (o.velocity.y < 1) o.velocity.y += 0.3 * GetFrameTime() * 70;
                        if (o.velocity.x != 0) o.velocity.x -= o.velocity.x * 0.002 * GetFrameTime() * 70;
                        o.velocity.y += 1 * GetFrameTime() * 70;
                        o.world.x += o.velocity.x * GetFrameTime() * 70;
                        o.world.y += o.velocity.y * GetFrameTime() * 70;

                        Vector2 off = GetWindowPosition();
                        Vector2 wsz = (Vector2) { GetScreenWidth(), GetScreenHeight() };

                        if (o.world.x < off.x + o.radius) {
                                o.velocity.x = fabs(o.velocity.x);
                                o.world.x = off.x + o.radius;
                        }
                        if (o.radius + o.world.x - off.x > wsz.x) {
                                o.velocity.x = -fabs(o.velocity.x);
                                o.world.x = wsz.x + off.x - o.radius;
                        }
                        if (o.world.y < o.radius + off.y) {
                                o.velocity.y = fabs(o.velocity.y);
                                o.world.y = o.radius + off.y;
                        }
                        if (o.radius + o.world.y - off.y > wsz.y) {
                                o.velocity.y = -fabs(o.velocity.y);
                                o.world.y = wsz.y + off.y - o.radius;
                        }
                }


                BeginDrawing();
                ClearBackground(BLACK);

                obj_draw(&c, &o);
                EndDrawing();
        }


        return 0;
}
