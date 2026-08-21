/*
 * types_name.c: enum names
 */

#include "types.h"

char const*
animation_name(enum animation anim) {
    switch (anim) {
        case ANIMATION_ARM_SWING: return "arm_swing";
        default: return "unknown";
    }
}