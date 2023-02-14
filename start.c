#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#define Y_MAX 600
#define X_MAX 800

struct Square{
  int x;
  int y;
  int size;
  int speed_y;
  int speed_x;
  int a_y;
};

struct Beam{
  int start_x;
  int start_y;
  int end_x;
  int end_y;
};

struct Scene{
  int g;
  struct Beam** beams;
  struct Square* sq;
  int size;
};

int init(ALLEGRO_TIMER** timer, ALLEGRO_EVENT_QUEUE** queue, ALLEGRO_DISPLAY** disp){
  if(!al_init())
    {
      printf("couldn't initialize allegro\n");
      return 1;
    }

  if(!al_install_keyboard())
    {
      printf("couldn't initialize keyboard\n");
      return 1;
    }

  *timer = al_create_timer(1.0 / 30.0);
  if(!timer)
    {
      printf("couldn't initialize timer\n");
      return 1;
    }
  *queue = al_create_event_queue();
  if(!queue)
    {
      printf("couldn't initialize queue\n");
      return 1;
    }

  *disp = al_create_display(X_MAX, Y_MAX);
  if(!disp)
    {
      printf("couldn't initialize display\n");
      return 1;
    }

}

void all_destroy(ALLEGRO_TIMER* timer, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* disp){
  al_destroy_display(disp);
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
}

void event_register(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_TIMER* timer, ALLEGRO_DISPLAY* disp){
  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(disp));
  al_register_event_source(queue, al_get_timer_event_source(timer));
}

void phisics(struct Scene* sc){
  struct Square* sq = sc->sq;
  int g = sc->g;
  
  if(sq->y + sq->speed_y >= Y_MAX - sq->size){
    sq->speed_y = 0;
    sq->y += (Y_MAX - sq->y - sq->size);
  }
  sq->y += sq->speed_y - sq->a_y;
  
  sq->speed_y += g - sq->a_y;

  if (sq->a_y != 0)
    sq->a_y -= 1;
  
  
}

void draw(struct Scene* scene){
  //this function draw scene
  struct Square* sq = scene->sq;
  
  al_clear_to_color(al_map_rgb(255, 255, 255));
  al_draw_filled_rectangle(sq->x, sq->y, sq->x+sq->size, sq->y+sq->size, al_map_rgb_f(0, 1, 0));
  for(int i = 0; i < scene->size; i++){
    struct Beam* b = scene->beams[i];
    al_draw_line(b->start_x, b->start_y, b->end_x, b->end_y, al_map_rgb_f(0,0,0), 1);
    }
  al_flip_display();
}

void add_line(struct Beam* beam, struct Scene* scene){
  scene->size += 1;
  scene->beams = realloc(scene->beams, scene->size * sizeof(struct Beam*));
  scene->beams[scene->size-1] = beam;
}

void loop(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_TIMER* timer, ALLEGRO_DISPLAY* disp){
  struct Square sq = {0,Y_MAX,50,0,0,0};
  struct Scene scene = {.g = 5, .beams = (struct Beam**)malloc(1 * sizeof(struct Beam*)), .sq = &sq, .size=0};
  add_line(&((struct Beam){0, Y_MAX/2, X_MAX/2, Y_MAX/2}), &scene);
  add_line(&((struct Beam){X_MAX/2, Y_MAX/2, X_MAX/2, Y_MAX}), &scene);
  bool done = false;
  bool redraw = true;
  ALLEGRO_EVENT event;

  al_start_timer(timer);
  while(1)
    {
      al_wait_for_event(queue, &event);
      switch(event.type)
        {
	case ALLEGRO_EVENT_TIMER:
	  phisics(&scene);
	  redraw = true;
	  break;
	case ALLEGRO_EVENT_KEY_DOWN:
	  if(event.keyboard.keycode == ALLEGRO_KEY_UP)
            sq.a_y = 11;
	  else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
	    done = true;
	  break;
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
	  done = true;
	  break;
        }

      if(done)
	break;

      if(redraw && al_is_event_queue_empty(queue))
        {
	  draw(&scene);
	  redraw = false;
        }
    }

  free(scene.beams);

}

int main()
{
  ALLEGRO_TIMER* timer;
  ALLEGRO_EVENT_QUEUE* queue;
  ALLEGRO_DISPLAY* disp;
  
  
  init(&timer, &queue, &disp);
    
  event_register(queue, timer, disp);

  loop(queue, timer, disp);
    
  all_destroy(timer, queue, disp);

    
  return 0;
}
