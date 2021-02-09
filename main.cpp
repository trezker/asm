#include <iostream>
#include <string>
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

class Allegro_ustr {
private:
	ALLEGRO_USTR* str;
public:
	Allegro_ustr() {
		str = al_ustr_new("");
	}

	Allegro_ustr(const char* s) {
		str = al_ustr_new(s);
	}

	Allegro_ustr(const Allegro_ustr& o) {
		str = al_ustr_dup(o.str);
	}

	Allegro_ustr(const ALLEGRO_USTR* o) {
		str = al_ustr_dup(o);
	}
	
	Allegro_ustr& operator=(const Allegro_ustr& o) {
		al_ustr_assign(str, o.str);
		return *this;
	}

	~Allegro_ustr() {
		al_ustr_free(str);
	}

	const ALLEGRO_USTR* ustr() {
		return str;
	}

	int Insert_chr(int index, int32_t c) {
		int pos = al_ustr_offset(str, index);
		return al_ustr_insert_chr(str, pos, c);
	}
	
	bool Remove_chr(int index) {
		int pos = al_ustr_offset(str, index);
		return al_ustr_remove_chr(str, pos);
	}

	Allegro_ustr Substr(int start, int end) {
		int startpos = al_ustr_offset(str, start);
		int endpos = al_ustr_offset(str, end);
		ALLEGRO_USTR *sub = al_ustr_dup_substr(str, startpos, endpos);
		Allegro_ustr substr(sub);
		al_ustr_free(sub);
		return substr;
	}

	int Substr_width(ALLEGRO_FONT* font, int index) {
		int pos = al_ustr_offset(str, index);
		ALLEGRO_USTR *sub = al_ustr_dup_substr(str, 0, pos);
		int width = al_get_ustr_width(font, sub);
		al_ustr_free(sub);
		return width;
	}

	int Length() {
		return al_ustr_length(str);
	}

	void Append(const Allegro_ustr& o) {
		al_ustr_append(str, o.str);
	}
};

typedef std::vector<Allegro_ustr> Lines;

class App {
private:
	ALLEGRO_COLOR black;
	ALLEGRO_COLOR text_color;
	ALLEGRO_FONT *font;

	Lines lines;
	int edit_index;
	int edit_line;
	float caret_time;
public:
	void Input_char(const ALLEGRO_EVENT& event) {
		switch(event.keyboard.keycode) {
			case ALLEGRO_KEY_ESCAPE:
				break;
			case ALLEGRO_KEY_UP:
				if(edit_line > 0) {
					--edit_line;
					if(edit_index > lines[edit_line].Length()) {
						edit_index = lines[edit_line].Length();
					}
				}
				break;
			case ALLEGRO_KEY_DOWN:
				if(edit_line < lines.size()-1) {
					++edit_line;
					if(edit_index > lines[edit_line].Length()) {
						edit_index = lines[edit_line].Length();
					}
				}
				break;
			case ALLEGRO_KEY_LEFT:
				if(edit_index > 0) {
					--edit_index;
				}
				break;
			case ALLEGRO_KEY_RIGHT:
				if(edit_index < lines[edit_line].Length()) {
					++edit_index;
				}
				break;
			case ALLEGRO_KEY_HOME:
				edit_index = 0;
				break;
			case ALLEGRO_KEY_END:
				edit_index = lines[edit_line].Length();
				break;
			case ALLEGRO_KEY_ENTER: {
				Allegro_ustr substr = lines[edit_line].Substr(edit_index, lines[edit_line].Length());
				lines.insert(lines.begin() + edit_line + 1, substr);
				substr = lines[edit_line].Substr(0, edit_index);
				lines[edit_line] = substr;
				++edit_line;
				edit_index = 0;
				break;
			}
			case ALLEGRO_KEY_BACKSPACE:
				if(edit_index != 0) {
					if(lines[edit_line].Remove_chr(edit_index-1)) {
						edit_index--;
					}
				}
				break;
			case ALLEGRO_KEY_DELETE:
				if(edit_index != 0) {
					if(edit_index < lines[edit_line].Length()) {
						lines[edit_line].Remove_chr(edit_index);
					} else {
						if(edit_line < lines.size() - 1) {
							lines[edit_line].Append(lines[edit_line+1]);
							lines.erase(lines.begin()+edit_line+1);
						}
					}
				}
				break;
			default:
				if(event.keyboard.unichar > 0) {
					if(lines[edit_line].Insert_chr(edit_index, event.keyboard.unichar)>0) {
						edit_index++;
					}
				}
				break;
		}
	}

	void Draw() {
		al_clear_to_color(black);
		al_draw_circle(100, 100, 10, text_color, 1);
		al_draw_text(font, text_color, 0, 0, 0, "Hullo! This text is 100 characters long. I'm writing this to check how wide that makes it.1234567890");
		int l = 0;
		for(Lines::iterator i = lines.begin(); i != lines.end(); i++) {
			al_draw_ustr(font, text_color, 0, 18+l*18, 0, (*i).ustr());
			++l;
		}

		int caretx = lines[edit_line].Substr_width(font, edit_index);
		if(caret_time < 0.75) {
			al_draw_line(caretx, 18 + edit_line*18, caretx, 18 + edit_line*18 +12, text_color, 2);
		}
		al_flip_display();
	}

	void Run() {
		lines.push_back("");
		edit_index = 0;
		edit_line = 0;

		al_init();
		al_init_primitives_addon();
		al_init_font_addon();
		al_init_ttf_addon();

		al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_REQUIRE);
		ALLEGRO_DISPLAY *display = al_create_display(1440, 900);
		if (!display) {
			std::cout<<"al_create_display failed";
			return;
		}

		if (!al_install_keyboard()) {
			std::cout<<"al_install_keyboard failed";
			return;
		}

		black = al_map_rgb(0, 0, 0);
		text_color = al_map_rgb(255, 255, 255);

		font = al_load_ttf_font("data/DejaVuSans.ttf", 12, 0);
		if(!font) {
			std::cout<<"Loading font failed";
			return;
		}

		ALLEGRO_TIMER *timer = al_create_timer(ALLEGRO_BPS_TO_SECS(1000));
		al_start_timer(timer);

		ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
		if (!event_queue) {
			std::cout<<"al_create_event_queue failed";
			return;
		}

		al_register_event_source(event_queue, al_get_keyboard_event_source());
		al_register_event_source(event_queue, al_get_display_event_source(display));
		al_register_event_source(event_queue, al_get_timer_event_source(timer));
		bool running = true;
		bool redraw = true;
		int ticks = 0;
		int frames = 0;
		caret_time = 0;
		while(running) {
			ALLEGRO_EVENT event;
			while (al_wait_for_event_timed(event_queue, &event, 0.000001))
			{
				switch (event.type) {
					case ALLEGRO_EVENT_DISPLAY_CLOSE:
						running = false;
						break;
					case ALLEGRO_EVENT_TIMER:
						if(ticks == 1000) {
							//std::cout<<frames<<std::endl;
							ticks = 0;
							frames = 0;
						}
						redraw = true;
						++ticks;
						caret_time += 0.001;
						if(caret_time >= 1.25) {
							caret_time = 0;
						}
						break;
					case ALLEGRO_EVENT_KEY_CHAR:
						Input_char(event);
						std::cout<<event.keyboard.unichar<<std::endl;
						break;
					default:
						break;
				}
			}
			if(redraw) {
				Draw();
				redraw = false;
				++frames;
			}
		}


		al_destroy_event_queue(event_queue);
		al_destroy_timer(timer);
	}
};

int main(int argc, char **argv) {
	App app = App();
	app.Run();
	return 0;
}