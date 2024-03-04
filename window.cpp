#include "window.hpp"
#include "utils/debug.hpp"

// internal methods

namespace{
	int keysym(int code){
		int key = code;
		if(key >= 128) key -= 0x40000039;
		if(key < 0) return WINDOW_KEYCODES;
		return key + 128;
	}
}

// setup methods

Window::Window(const char *name, Uint32 flags, int w, int h, float frameRate, float inputRate){
	
	// SDL
	window = NULL;
	width = w;
	height = h;
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		debug("SDL init failed", SDL_GetError());
		return;
	}
	if((window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags)) == NULL){
		debug("SDL window creation failed", SDL_GetError());
		return;
	}
	
	// SDL OpenGL
	context = NULL;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	if((context = SDL_GL_CreateContext(window)) == NULL){
		debug("GL context creation failed", SDL_GetError());
		return;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	// OpenGL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(.2f, .2f, .2f, 1.f);
	
	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK){
		debug("GLEW init failed", glewGetErrorString(err));
		return;
	}
	
	// time
	timeStart = 0;
	timePrev[0] = timePrev[1] = 0;
	timePeriod[0] = CLOCKS_PER_SEC * (1.f / frameRate);
	timePeriod[1] = CLOCKS_PER_SEC * (1.f / inputRate);
	
	// input
	for(int i = 0; i < WINDOW_KEYCODES; i++) keyMap[i] = 0;
	for(int i = 0; i < WINDOW_MOUSECODES; i++) mouseMap[i] = 0;
	mousePosition[0] = mousePosition[1] = 0;
	mouseMotion[0] = mouseMotion[1] = 0;
	
	// focus
	isWindowFocused = false;
	isCursorPresent = false;
	isCursorNewlyFocused = false;
	SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
}

Window::~Window(){
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Window::timer(){
	timeStart = clock();
	for(int i = 0; i < WINDOW_RATES; i++) timePrev[i] = timeStart - timePeriod[i];
}

// update methods

bool Window::cap(WindowRate type){
	if(clock() - timePrev[type] > timePeriod[type]){
		timePrev[type] += timePeriod[type];
		return true;
	}
	return false;
}

WindowState Window::get(){
	
	// events
	SDL_Event event;
	int key;
	while(SDL_PollEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				return WindowExit;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button < WINDOW_MOUSECODES) mouseMap[event.button.button] = (mouseMap[event.button.button] == 0 ? 1 : -1);
				else debug("Input error: MOUSEDOWN code out of bounds", event.button.button);
				break;
			case SDL_MOUSEBUTTONUP:
				if(event.button.button < WINDOW_MOUSECODES) mouseMap[event.button.button] = 0;
				else debug("Input error: MOUSEUP code out of bounds", event.button.button);
				break;
			case SDL_KEYDOWN:
				key = keysym(event.key.keysym.sym);
				if(key < WINDOW_KEYCODES) keyMap[key] = (keyMap[key] == 0 ? 1 : -1);
				else debug("Input error: KEYDOWN code out of bounds", key);
				break;
			case SDL_KEYUP:
				key = keysym(event.key.keysym.sym);
				if(key < WINDOW_KEYCODES) keyMap[key] = 0;
				else debug("Input error: KEYUP code out of bounds", key);
				break;
			case SDL_MOUSEMOTION:
				if(isCursorNewlyFocused){
					mouseMotion[0] = 0;
					mouseMotion[1] = 0;
					isCursorNewlyFocused = false;
				}
				else{
					mouseMotion[0] += 2.f * event.motion.xrel / width;
					mouseMotion[1] -= 2.f * event.motion.yrel / height;
				}
				break;
			case SDL_WINDOWEVENT:
				switch(event.window.event){
					case SDL_WINDOWEVENT_ENTER:
						isCursorPresent = true;
						break;
					case SDL_WINDOWEVENT_LEAVE:
						isCursorPresent = false;
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						width = event.window.data1;
						height = event.window.data2;
						glViewport(0, 0, width, height);
						return WindowResizing;
						break;
					case SDL_WINDOWEVENT_RESIZED:
						width = event.window.data1;
						height = event.window.data2;
						glViewport(0, 0, width, height);
						return WindowResized;
						break;
				}
				break;
			default:
				break;
		}
	}
	
	return WindowDefault;
}

void Window::clear() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swap() const {
	SDL_GL_SwapWindow(window);
}

void Window::focus(){
	if(isCursorPresent && !isWindowFocused){
		isWindowFocused = true;
		isCursorNewlyFocused = true;
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
}

void Window::unfocus(){
	SDL_SetRelativeMouseMode(SDL_FALSE);
	isWindowFocused = false;
}

// properties

float Window::getAspectRatio() const {
	return (float)width / height;
}

void Window::getScreenSpace(float &w, float &h, float &cx, float &cy) const {
	w = (float)width / 2.f;
	h = (float)height / 2.f;
	cx = (float)width / 2.f;
	cy = (float)height / 2.f;
}

// input

int& Window::getMapHandle(WindowKey code){
	return keyMap[keysym(code)];
}

int& Window::getMapHandle(WindowButton code){
	return mouseMap[code];
}

float (&Window::getMouseMotionHandle())[2]{
	return mouseMotion;
}

float (&Window::getMousePositionHandle())[2]{
	return mousePosition;
}

// bindings

template <typename T>
void InputBind::bindAll(std::vector<std::pair<int, T>> const &bindings, Window &w){
	for(int i = 0; i < bindings.size(); i++) bind(std::get<0>(bindings[i]), std::get<1>(bindings[i]), w);
}

InputBind::InputBind(float (&m)[2], float(&p)[2]) : motion(m), position(p), isActive(false) {}

template <typename T>
void InputBind::bind(int id, T code, Window &w){
	bindings.insert({id, &w.getMapHandle(code)});
}

void InputBind::bindAll(std::vector<std::pair<int, WindowKey>> const &bindings, Window &w){
	bindAll<WindowKey>(bindings, w);
}

void InputBind::bindAll(std::vector<std::pair<int, WindowButton>> const &bindings, Window &w){
	bindAll<WindowButton>(bindings, w);
}

void InputBind::setActive(bool a){
	isActive = a;
}

int InputBind::getInactivePress(int id){
	if(*bindings[id] == 1){
		*bindings[id] = -1;
		return 1;
	}
	return 0;
}

int InputBind::getPress(int id){
	if(*bindings[id] == 1){
		*bindings[id] = -1;
		return 1 * isActive;
	}
	return 0;
}

int InputBind::getHold(int id){
	return abs(*bindings[id]) * isActive;
}

void InputBind::getMouseMotion(float (&m)[2]){
	m[0] = motion[0] * isActive;
	m[1] = motion[1] * isActive;
	motion[0] = motion[1] = 0;
}