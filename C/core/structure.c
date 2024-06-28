#include <stddef.h>
#include <limits.h>

#include "C/api.h"
#include "C/macro.h"
#include "C/utils/memory-recycler.h"
#include "C/utils/vector.h"

#include "structure.private.h"

typedef struct surface_struct {
  math_vec edge;
  unit_vec normal;
} surface;

struct attachment_struct {
  structure* parent;
  surface* s;
  entity* e;
};

struct structure_struct {
  point center;
  PDRect hitbox;
  surface* surfaces;
  uint8_t edge_count;
  closure* apply;
  vector* attachments;
};

static memory_recycler* attachment_recycler = NULL;

static void* attachment_allocator(void) {
  attachment* a = malloc(sizeof(attachment));
  if (!a) {
    get_api()->system->error(
      "Could not allocate memory for surface attachment"
    );
  }
  return a;
}

static void attachment_destructor(void* attachment_to_cast) {
  attachment* a = (attachment*)attachment_to_cast;
  free(a);
}

static void initialize_if_needed(void) {
  if (!attachment_recycler) {
    attachment_recycler = memory_recycler_create(
      attachment_allocator, 
      attachment_destructor
    );
  }
}

structure* structure_create(
  point center,
  const point* perimeter, 
  uint8_t point_count,
  closure* apply
) { 
  structure* s = malloc(sizeof(structure));
  if (!s) {
    get_api()->system->error("Could not allocate memory for structure");
  }
  s->surfaces = malloc(point_count * sizeof(surface));
  if (!s->surfaces) {
    get_api()->system->error(
      "Could not allocate memory for structure surfaces"
    );
  }
  s->attachments = vector_create(1);

  memcpy(&(s->center), &center, sizeof(point));
  s->edge_count = point_count;
  s->apply = apply;

  LCDRect hitbox = { 
    .left = INT_MAX, 
    .top = INT_MAX,
    .right = INT_MIN, 
    .bottom = INT_MIN
  };
  for (int i=0; i < point_count; i++) {
    hitbox.left = min(hitbox.left, perimeter[i].x - HITBOX_OVERSIZE_PX);
    hitbox.right = max(hitbox.right, perimeter[i].x + HITBOX_OVERSIZE_PX);
    hitbox.top = min(hitbox.top, perimeter[i].y - HITBOX_OVERSIZE_PX);
    hitbox.bottom = max(hitbox.bottom, perimeter[i].y + HITBOX_OVERSIZE_PX);

    math_vec_init(
      &(s->surfaces[i].edge), 
      perimeter[i], 
      perimeter[(i+1)%point_count]
    );
    math_vec_unit_normal(s->surfaces[i].edge, &(s->surfaces[i].normal), center);
  }
  s->hitbox = (PDRect){ 
    .x = hitbox.left, 
    .y = hitbox.top, 
    .width = hitbox.right - hitbox.left,
    .height = hitbox.bottom - hitbox.top
  };
  return s;
}

bool structure_attach(structure* s, entity* e) {
  return false;
}

void structure_detatch(structure*s, entity* e) {

}

// Positive distances go from start to end, negative from end to start
void structure_move_along_surface(structure* s, entity* e, int distance) {

}

void structure_destroy(structure* s) {
  if (vector_length(s->attachments)) {
    get_api()->system->error("Cannot destroy structure with attached entities");
  }
  vector_destroy(s->attachments);
  free(s->surfaces);
  free(s);
}

void structure_log(structure* s) {
  PlaydateAPI* api = get_api();

  api->system->logToConsole("Logging structure %p", s);
  api->system->logToConsole("- center: { %d, %d }", s->center.x, s->center.y);
  api->system->logToConsole(
    "- hitbox: { %f, %f, w: %f, h: %f }", 
    s->hitbox.x, 
    s->hitbox.y,
    s->hitbox.width,
    s->hitbox.height
  );
  api->system->logToConsole("- %d surfaces ->", s->edge_count);
  for (int i = 0; i < s->edge_count; i++) {
    api->system->logToConsole("  - surface %d: {", i);
    api->system->logToConsole("      edge: { ");
    api->system->logToConsole("        start: { %d, %d }, end: { %d, %d }, magnitude: %f", s->surfaces[i].edge.start.x, s->surfaces[i].edge.start.y, s->surfaces[i].edge.end.x, s->surfaces[i].edge.end.y, s->surfaces[i].edge.magnitude);
    api->system->logToConsole("      }");
    api->system->logToConsole("      normal: { %f, %f }", s->surfaces[i].normal.x, s->surfaces[i].normal.y);
    api->system->logToConsole("    }");
  }
  api->system->logToConsole("- attachments: %d", vector_length(s->attachments));
}
