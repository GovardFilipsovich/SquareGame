#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

// defining constants
#define Y_MAX 600
#define X_MAX 800
#define MAX_ACCEL 3
#define MAX_SPEED 20
#define ACC_CONST 0.2

//defining events
#define MY_CROSS_EVENT ALLEGRO_GET_EVENT_TYPE('m', 's', 'e', 't')


struct Square{
  float x;
  float y;
  float size;
  float speed_y;
  float speed_x;
  float a_y;
  float a_x;
  float accel;
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
  struct USER_EVENT* user_event;
};

struct USER_EVENT{
  ALLEGRO_EVENT_SOURCE* user_src;
  ALLEGRO_EVENT* user_event;
};


int init(ALLEGRO_TIMER** timer, ALLEGRO_EVENT_QUEUE** queue, ALLEGRO_DISPLAY** disp, ALLEGRO_EVENT_SOURCE* user_src){
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

  al_init_user_event_source(user_src);

}

void all_destroy(ALLEGRO_TIMER* timer, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT_SOURCE* user_src){
  al_destroy_display(disp);
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
  al_destroy_user_event_source(user_src);
}

void event_register(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_TIMER* timer, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT_SOURCE* user_src){
  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(disp));
  al_register_event_source(queue, al_get_timer_event_source(timer));
  al_register_event_source(queue, user_src);
}

void check_cross_lines(struct Scene* sc){
  struct USER_EVENT* u_e = sc->user_event;
  u_e->user_event->type = MY_CROSS_EVENT;
  for (int i = 0; i < sc->size; i++){
    
    al_emit_user_event(u_e->user_src, u_e->user_event, NULL);
  }
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


  sq->speed_x += sq->a_x;
  sq->x += sq->speed_x;
  if (sq->accel == 1)
    sq->a_x += ACC_CONST;
  else if (sq->accel == -1)
    sq->a_x -= ACC_CONST;
  else if (sq->accel == 0)
    sq->a_x = 0;

  // Limit max acceleration
  sq->accel = (sq->accel >= MAX_ACCEL) ? MAX_ACCEL : (sq->accel <= -MAX_ACCEL) ? -MAX_ACCEL : sq->accel;

  // Limit max speed
  sq->speed_x = (sq->speed_x >= MAX_SPEED) ? MAX_SPEED : (sq->speed_x <= -MAX_SPEED) ? -MAX_SPEED : sq->speed_x;
  
  if(sq->speed_x > 0)
    sq->speed_x -= 1;
  else if (sq->speed_x < 0)
    sq->speed_x += 1;

  sq->speed_x = (abs(sq->speed_x) < 1) ? 0 : sq->speed_x;

  check_cross_lines(sc);
  
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

void loop(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_TIMER* timer, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT_SOURCE* user_src){
  
  struct Square sq = {0,Y_MAX,50,0,0,0};
  struct Scene scene = {.g = 5, .beams = (struct Beam**)malloc(1 * sizeof(struct Beam*)), .sq = &sq, .size=0};
  add_line(&((struct Beam){0, Y_MAX/2, X_MAX/2, Y_MAX/2}), &scene);
  add_line(&((struct Beam){X_MAX/2, Y_MAX/2, X_MAX/2, Y_MAX}), &scene);
  bool done = false;
  bool redraw = true;
  
  ALLEGRO_EVENT event;
  ALLEGRO_EVENT user_e;

  struct USER_EVENT user_event = {user_src, &user_e};
  scene.user_event = &user_event;

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
	  else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
	    sq.accel = 1;
	  else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
	    sq.accel = -1;
	  break;
	case ALLEGRO_EVENT_KEY_UP:
	  if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
	    sq.accel = 0;
	  else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
	    sq.accel = 0;
	  
	  break;
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
	  done = true;
	  break;
	case MY_CROSS_EVENT:
	  printf("Hello from event\n");
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
  ALLEGRO_EVENT_SOURCE user_src;
  
  
  
  init(&timer, &queue, &disp, &user_src);
    
  event_register(queue, timer, disp, &user_src);

  loop(queue, timer, disp, &user_src);
    
  all_destroy(timer, queue, disp, &user_src);

    
  return 0;
}
