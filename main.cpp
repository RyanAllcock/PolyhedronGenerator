#include "window.hpp" // scene window
#include "shader.hpp" // scene display
#include "camera.hpp" // scene navigation
#include "polyhedra.hpp" // polyhedron generation
#include "model.hpp" // polyhedron model representation
#include "utils/argument.hpp" // argument fetching
#include "utils/debug.hpp" // debugging
#include "utils/filemanager.hpp" // file fetching

#include <vector> // mesh data
#include <array> // data passing
#include <list> // list renderers

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_PERSEC 144.f
#define INPUT_PERSEC 60.f

#define CAMERA_ROTATE_SENS .8f
#define CAMERA_MOVE_SENS .002f
#define CAMERA_FIELD_OF_VISION 70.f
#define PROJECTION_NEAR .1f
#define PROJECTION_FAR 100.f

// indexing
enum ProgramInput{
	InputForward, InputBackward, InputLeft, InputRight, InputUp, InputDown,  // movement
	InputSelect, InputRelease, InputProject, // camera
	InputFocus, InputSpin, // mouse
	InputGraphic,  // shader
	InputTabout // window
};
enum ArgumentType{
	ArgumentOperators, // operator stream
	ArgumentRenderer, // model renderer id
	ArgumentProjection // camera projection id
};
enum RendererType{
	RendererPoint, 
	RendererTriangle, 
	RendererLine, 
	RendererSolidwire
};

int main(int argc, char *argv[]){ 
	
	// arguments
	std::string operators;
	RendererType rendererId;
	ProjectionType projectionId;
	{
	std::vector<std::string> const properties = ArgumentReader::get(argc - 1, &argv[1], {"--operators", "--shader", "--projection"}, 1);
	debug("properties", properties);
	operators = properties[ArgumentOperators];
	rendererId = ArgumentReader::match<RendererType>(
		{{"point", RendererPoint}, 
		{"tri", RendererTriangle}, 
		{"line", RendererLine}, 
		{"solid", RendererSolidwire}}, properties[ArgumentRenderer], RendererTriangle);
	projectionId = ArgumentReader::match<ProjectionType>(
		{{"ortho", CameraOrthographic}, 
		{"persp", CameraPerspective}}, properties[ArgumentProjection], CameraOrthographic);
	}
	
	// check for operator stream
	if(operators == ""){
		debug("Error: no operator argument found");
		return -1;
	}
	
	// shape
	std::vector<Mesh> polyhedra;
	{
	std::vector<Polyhedron> const polydata = PolyhedronFactory::make(operators);
	for(Polyhedron poly : polydata) polyhedra.push_back(Mesh(poly.vertices, poly.edges, poly.faces));
	if(polyhedra.empty()){
		debug("Error: no polyhedra generated from stream");
		return -1;
	}
	}
	Mesh &polyhedron = polyhedra.front();
	debug("shape", polyhedron);
	
	// window
	Window window("Polyhedra", WindowResize | WindowGraphic, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_PERSEC, INPUT_PERSEC);
	InputBind input(window.getMouseMotionHandle(), window.getMousePositionHandle());
	input.bindAll(std::vector<std::pair<int, WindowKey>>{
		{InputForward, KeyW}, {InputBackward, KeyS}, {InputLeft, KeyA}, {InputRight, KeyD}, {InputUp, KeySpace}, {InputDown, KeyLeftControl}, 
		{InputSelect, KeyE}, {InputRelease, KeyTab}, {InputProject, KeyP}, {InputGraphic, KeyG}, {InputTabout, KeyLeftAlt}}, window);
	input.bindAll(std::vector<std::pair<int, WindowButton>>{
		{InputFocus, MouseLeftClick}, {InputSpin, MouseRightClick}}, window);
	
	// camera
	Camera camera(std::array<float, 3>{0.f, 0.f, 2.f}, CAMERA_ROTATE_SENS, CAMERA_MOVE_SENS);
	CameraProjection projection(projectionId, CAMERA_FIELD_OF_VISION, PROJECTION_NEAR, PROJECTION_FAR);
	projection.set(camera, window.getAspectRatio());
	
	// shader sources
	Shader vertexShader, fragmentShader, solidwireGeometryShader, solidwireFragmentShader;
	{
	std::string const basicVSrc = FileManager::get("shaders/basicVertex.glsl");
	std::string const basicFSrc = FileManager::get("shaders/basicFragment.glsl");
	std::string const solidwireGSrc = FileManager::get("shaders/solidwireGeometry.glsl");
	std::string const solidwireFSrc = FileManager::get("shaders/solidwireFragment.glsl");
	if(	basicVSrc == "" || basicFSrc == "" || 
		solidwireGSrc == "" || solidwireFSrc == ""){
		debug("Error: shader source files not found");
		return -1;
	}
	vertexShader = Shader(ShaderVertex, std::vector<const char*>{basicVSrc.c_str()});
	fragmentShader = Shader(ShaderFragment, std::vector<const char*>{basicFSrc.c_str()});
	solidwireGeometryShader = Shader(ShaderGeometry, std::vector<const char*>{solidwireGSrc.c_str()});
	solidwireFragmentShader = Shader(ShaderFragment, std::vector<const char*>{solidwireFSrc.c_str()});
	}
	
	// renderer components
	Buffer vertexBuffer(BufferStatic, polyhedron.getSerialVertices().data(), sizeof(float) * polyhedron.getSerialVertices().size());
	Buffer triangleBuffer(BufferStatic, polyhedron.getTriangularFaces().data(), sizeof(int) * polyhedron.getTriangularFaces().size());
	Buffer lineBuffer(BufferStatic, polyhedron.getSerialEdges().data(), sizeof(int) * polyhedron.getSerialEdges().size());
	Buffer wheelBuffer(BufferStatic, polyhedron.getSerialVertices().data(), sizeof(float) * (polyhedron.getSerialVertices().size() + polyhedron.getFanCentreVertices().size()));
	wheelBuffer.update(polyhedron.getFanCentreVertices().data(), sizeof(float) * polyhedron.getFanCentreVertices().size(), sizeof(float) * polyhedron.getSerialVertices().size());
	Buffer wheelElementBuffer(BufferStatic, polyhedron.getFanFaces().data(), sizeof(int) * polyhedron.getFanFaces().size());
	Index vertexIndex(vertexBuffer, 3, IndexFloat, IndexUnchanged, sizeof(float) * 3, 0);
	Index triangleIndex(triangleBuffer, IndexUint, sizeof(int), 0);
	Index lineIndex(lineBuffer, IndexUint, sizeof(int), 0);
	Index wheelIndex(wheelBuffer, 3, IndexFloat, IndexUnchanged, sizeof(float) * 3, 0);
	Index wheelElementIndex(wheelElementBuffer, IndexUint, sizeof(int), 0);
	Program basicProgram(std::vector<Shader*>{ &vertexShader, &fragmentShader });
	Program solidwireProgram(std::vector<Shader*>{ &vertexShader, &solidwireGeometryShader, &solidwireFragmentShader });
	DrawArray pointDraw(DrawPoint, std::vector<Index*>{ &vertexIndex }, polyhedron.getSerialVertices().size() / 3);
	DrawElements triangleDraw(DrawTriangle, std::vector<Index*>{ &vertexIndex }, triangleIndex, polyhedron.getTriangularFaces().size());
	DrawElements lineDraw(DrawLine, std::vector<Index*>{ &vertexIndex }, lineIndex, polyhedron.getSerialEdges().size());
	DrawElements solidwireDraw(DrawTriangle, std::vector<Index*>{ &wheelIndex }, wheelElementIndex, polyhedron.getFanFaces().size());
	
	// renderers
	std::list<Renderer> const renderers{
		Renderer(basicProgram, pointDraw), 
		Renderer(basicProgram, triangleDraw), 
		Renderer(basicProgram, lineDraw), 
		Renderer(solidwireProgram, solidwireDraw)
	};
	std::list<Renderer>::const_iterator renderer = renderers.begin();
	std::advance(renderer, rendererId);
	
	// uniforms
	std::array<float, 16> screenSpace;
	screenSpace[0] = screenSpace[5] = screenSpace[10] = screenSpace[15] = 1;
	window.getScreenSpace(screenSpace[0], screenSpace[5], screenSpace[3], screenSpace[7]);
	basicProgram.setUniform("vp", DataMatrix4(camera.getViewProjection(), DataUnchanged));
	solidwireProgram.setUniform("vp", DataMatrix4(camera.getViewProjection(), DataUnchanged));
	solidwireProgram.setUniform("screen", DataMatrix4(screenSpace, DataUnchanged));
	
	// loop
	window.timer();
	bool isRunning = true;
	while(isRunning){
		
		// input
		switch(window.get()){
			case WindowExit:
				isRunning = false;
				break;
			case WindowResized:
				projection.set(camera, window.getAspectRatio());
				std::array<float, 16> screenSpace;
				screenSpace[0] = screenSpace[5] = screenSpace[10] = screenSpace[15] = 1;
				window.getScreenSpace(screenSpace[0], screenSpace[5], screenSpace[3], screenSpace[7]);
				solidwireProgram.setUniform("screen", DataMatrix4(screenSpace, DataUnchanged));
				break;
		}
		
		// focusing
		if(input.getInactivePress(InputFocus)){
			window.focus();
			input.setActive(true);
		}
		if(input.getInactivePress(InputTabout)){
			window.unfocus();
			input.setActive(false);
		}
		
		// screen
		if(window.cap(WindowDisplay)){
			
			// camera
			float move[3] = { 0.f, 0.f, 0.f };
			float look[2];
			input.getMouseMotion(look);
			camera.input(input.getPress(InputSelect), input.getHold(InputSpin), input.getPress(InputRelease), move, look);
			
			// view
			basicProgram.setUniform("vp", DataMatrix4(camera.getViewProjection(), DataUnchanged));
			solidwireProgram.setUniform("vp", DataMatrix4(camera.getViewProjection(), DataUnchanged));
			
			// display
			window.clear();
			renderer->display();
			window.swap();
		}
		
		// simulation
		if(window.cap(WindowInput)){
			
			// movement
			float look[2] = { 0, 0 };
			float move[3] = {
				(float)(input.getHold(InputRight) - input.getHold(InputLeft)), 
				(float)(input.getHold(InputUp) - input.getHold(InputDown)), 
				(float)(input.getHold(InputForward) - input.getHold(InputBackward))};
			camera.input(input.getPress(InputSelect), input.getHold(InputSpin), input.getPress(InputRelease), move, look);
			
			// view
			if(input.getPress(InputGraphic) == 1){
				if(++renderer == renderers.end()) renderer = renderers.begin();
			}
			if(input.getPress(InputProject)){
				projection.toggle();
				projection.set(camera, window.getAspectRatio());
			}
		}
	}
	
	return 0;
}