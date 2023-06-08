#include "raylib.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const int screenWidth = 1024;
const int screenHeight = 576;
const int targetFPS = 40;
const int generationMax = 5;
const int minChild = 2;
const int maxChild = 8;
const float basicSpeed = 5.0f;
const float minSpeed = 0.5f;
unsigned childCounter = 1;

typedef union TrueColor {
    Color Color;
    unsigned long all : 32;
} TrueColor;

typedef struct Circle {
    unsigned short generation;
    Vector2 Speed;
    Vector2 Position;
    unsigned radius : 28;
    unsigned hasChild : 4;
    TrueColor Color;
    TrueColor NewColor;
    struct Circle *Parent;
    struct Circle **Child;
} Circle;

Color ColorLerp(Color c1, Color c2, float amount) {
    Color result = {0};
    result.r = (int)((c1.r * (1.0f - amount)) + (c2.r * amount));
    result.g = (int)((c1.g * (1.0f - amount)) + (c2.g * amount));
    result.b = (int)((c1.b * (1.0f - amount)) + (c2.b * amount));
    result.a = (int)((c1.a * (1.0f - amount)) + (c2.a * amount));
    return result;
}

static inline Color SetNewRandomColor() {
    return (Color){GetRandomValue(0, 255), GetRandomValue(0, 255),
                   GetRandomValue(0, 255), 255};
}

static inline float absf(float x) { return (x < 0) ? -x : x; }

void ConceiveNewChild(Circle *circle) {
    if (circle->generation < generationMax && GetFPS() >= targetFPS) {
        circle->hasChild = GetRandomValue(minChild, maxChild);
        childCounter += circle->hasChild;
        float basicAngle = atan2f(circle->Speed.y, circle->Speed.x);
        float basicdeflectionAngle = PI / 6.0f;
        float deflectionAngle = basicdeflectionAngle / circle->hasChild;

        circle->Child = (Circle **)malloc(circle->hasChild * sizeof(Circle *));
        for (unsigned short counter = 0; counter < circle->hasChild;
             counter++) {
            circle->Child[counter] = (Circle *)malloc(sizeof(Circle));
            circle->Child[counter]->generation = circle->generation + 1;
            circle->Child[counter]->Speed.x =
                basicSpeed * cosf(basicAngle - basicdeflectionAngle +
                                  deflectionAngle * counter);
            circle->Child[counter]->Speed.y =
                basicSpeed * sinf(basicAngle - basicdeflectionAngle +
                                  deflectionAngle * counter);
            circle->Child[counter]->Position.x = circle->Position.x;
            circle->Child[counter]->Position.y = circle->Position.y;
            circle->Child[counter]->radius =
                sqrt((circle->radius * circle->radius * PI) / circle->hasChild /
                     PI) +
                1;
            circle->Child[counter]->hasChild = 0;
            circle->Child[counter]->Color = circle->Color;
            circle->Child[counter]->NewColor.Color = SetNewRandomColor();
            circle->Child[counter]->Parent = circle;
            circle->Child[counter]->Child = NULL;
        }
    }
}

void RecalculatePosition(Circle *circle) {
    if (!circle->hasChild) {
        circle->Position.x += circle->Speed.x;
        circle->Position.y += circle->Speed.y;

        if ((circle->Position.x + circle->radius) >= screenWidth) {
            circle->Position.x = screenWidth - circle->radius - 1.0f;
            circle->Speed.x *= -1;
            ConceiveNewChild(circle);
        }
        if ((circle->Position.x - circle->radius) <= 0.0f) {
            circle->Position.x = 0.0f + circle->radius + 1.0f;
            circle->Speed.x *= -1;
            ConceiveNewChild(circle);
        }
        if ((circle->Position.y + circle->radius) >= screenHeight) {
            circle->Position.y = screenHeight - circle->radius - 1.0f;
            circle->Speed.y *= -1;
            ConceiveNewChild(circle);
        }
        if ((circle->Position.y - circle->radius) <= 0) {
            circle->Position.y = 0.0f + circle->radius + 1.0f;
            circle->Speed.y *= -1;
            ConceiveNewChild(circle);
        }
        if (circle->NewColor.all != circle->Color.all)
            circle->Color.Color =
                ColorLerp(circle->Color.Color, circle->NewColor.Color, 0.025f);

        DrawCircle(circle->Position.x, circle->Position.y, circle->radius,
                   circle->Color.Color);
    }
}

void SetNewChild(Circle *circle) {
    for (unsigned short counter = 0; counter < circle->hasChild; counter++)
        SetNewChild(circle->Child[counter]);

    RecalculatePosition(circle);
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Playground");

    SetTargetFPS(targetFPS);

    Circle father = {.generation = 0,
                     .Speed.x = basicSpeed * cosf(GetRandomValue(0.0f, 2 * PI)),
                     .Speed.y = basicSpeed * sinf(GetRandomValue(0.0f, 2 * PI)),
                     .Position.x = screenWidth / 2 - 50,
                     .Position.y = screenHeight / 2 - 50,
                     .radius = 64,
                     .hasChild = 0,
                     .Color = YELLOW,
                     .NewColor = YELLOW,
                     .Parent = NULL,
                     .Child = NULL};

    char string[80] = {0};
    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        SetNewChild(&father);
        DrawFPS(10, 10);
        snprintf(string, 80, "Total tree levels: %d", childCounter);
        DrawText(string, 10, 30, 20, LIME);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}