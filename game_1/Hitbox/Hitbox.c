#include "Hitbox.h"
// this is and AABB based collision checking system so if location of a = location of b at a given time an action is commited
// (in this case damage to player)
int CheckCollision(HITBOX a, HITBOX b){
  return
  (a.x < b.x + b.w &&
    a.x + a.w > b.x &&
    a.y < b.y + b.w &&
    a.y + a.w > b.y );
}