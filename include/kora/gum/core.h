#ifndef _KORA_GUM_CORE_H
#define _KORA_GUM_CORE_H 1

typedef struct GUM_cell GUM_cell;
typedef struct GUM_skin GUM_skin;
typedef struct GUM_display GUM_display;
typedef struct GUM_surface GUM_surface;
typedef struct GUM_layout GUM_layout;
typedef struct GUM_skins GUM_skins;
typedef struct GUM_event_manager GUM_event_manager;
typedef struct GUM_event GUM_event;

/* Driver */
void *gum_create_surface(int width, int height);
void* gum_surface_info(GUM_surface *img);
void gum_destroy_surface(void *win);

void gum_draw_cell(void *win, GUM_cell *cell, int x, int y);
void gum_draw_scrolls(void *win, GUM_cell *cell, int x, int y);

int gum_event_poll(void *win, GUM_event *event, int timeout);
int gum_check_events(void *win, int block);
void gum_invalid_surface(void *win, int x, int y, int w, int h);

void *gum_context(void *win);
void gum_complete(void *win, void *ctx);

void gum_text_size(const char *text, int *w, int *h);



#define ALIGN_UP(v,a) (((v)+((a)-1)) & ~((a)-1))

/* Commmon */
const char *gum_error();
void gum_log(const char *log);


#endif  /* _KORA_GUM_CORE_H */
