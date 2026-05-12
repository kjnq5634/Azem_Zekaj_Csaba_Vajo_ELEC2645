#ifndef HITBOX_H
#define HITBOX_H
typedef struct{
    int x;
    int y;
    int w;
    int h;
}HITBOX;
//struct for hitboxes
int CheckCollision(HITBOX a, HITBOX b);
// Collision checking function
#endif 