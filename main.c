#include "raylib-6.0_linux_amd64/include/raylib.h"
#include "raylib-6.0_linux_amd64/include/raymath.h"
#include <math.h>
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

#define dt GetFrameTime() * 70

        while (!WindowShouldClose()) {
                o.velocity.y += 1 * dt;
                o.world.x += o.velocity.x * dt;
                o.world.y += o.velocity.y * dt;

                Vector2 off = GetWindowPosition();
                Vector2 wsz = (Vector2) { GetScreenWidth(), GetScreenHeight() };

                if (o.world.x < off.x + o.radius) o.velocity.x = fabs(o.velocity.x);
                if (o.radius + o.world.x - off.x > wsz.x) o.velocity.x = -fabs(o.velocity.x);
                if (o.world.y < o.radius + off.y) o.velocity.y = fabs(o.velocity.y) - 1 * dt;
                if (o.radius + o.world.y - off.y > wsz.y) o.velocity.y = -fabs(o.velocity.y) - 1 * dt;


                BeginDrawing();
                ClearBackground(BLACK);

                obj_draw(&c, &o);
                EndDrawing();
        }


        return 0;
}
