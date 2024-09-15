#include "lib/window.hpp" // scene window
#include "lib/shader.hpp" // scene display
#include "lib/camera.hpp" // scene navigation
#include "source/polyhedra.hpp" // polyhedron generation
#include "lib/model.hpp" // polyhedron model representation
#include "utils/argument.hpp" // argument fetching
#include "utils/debug.hpp" // debugging
#include "utils/filemanager.hpp" // file fetching
#include "utils/maths.hpp" // model rotation

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

#define MODEL_ROTATE_SENS .05f
#define MODEL_MAX_VERTICES 10000
#define MODEL_MAX_FACES 10000
#define MODEL_MAX_LINES 10000
#define MODEL_MAX_WHEEL_FACES 20000

// indexing
enum ProgramInput{
	InputForward, InputBackward, InputLeft, InputRight, InputUp, InputDown,  // movement
	InputSelect, InputRelease, InputProject, // camera
	InputFocus, InputTurn, // mouse
	InputGraphic,  // shader
	InputTabout, // window
	InputSpin, // model
	InputDual, InputAmbo, InputAkis, InputGyro, InputCanon, // operators
	InputRevert // polyhedron
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
	std::vector<Polyhedron> polydata = PolyhedronFactory::make(operators);
	{
	for(Polyhedron poly : polydata) polyhedra.push_back(Mesh(poly.vertices, poly.edges, poly.faces));
	if(polyhedra.empty()){
		debug("Error: no polyhedra generated from stream");
		return -1;
	}
	}
	Mesh &polyhedron = polyhedra.back();
	debug("shape", polyhedron);
	
	// window
	Window window("Polyhedra", WindowResize | WindowGraphic, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_PERSEC, INPUT_PERSEC);
	InputBind input(window.getMouseMotionHandle(), window.getMousePositionHandle());
	input.bindAll(std::vector<std::pair<int, WindowKey>>{
		{InputForward, KeyW}, {InputBackward, KeyS}, {InputLeft, KeyA}, {InputRight, KeyD}, {InputUp, KeySpace}, {InputDown, KeyLeftControl}, 
		{InputSelect, KeyE}, {InputRelease, KeyTab}, {InputProject, KeyP}, {InputGraphic, KeyG}, {InputTabout, KeyLeftAlt}, 
		{InputSpin, KeyR}, {InputDual, KeyV}, {InputAmbo, KeyB}, {InputAkis, KeyN}, {InputGyro, KeyM}, {InputCanon, KeyC}, {InputRevert, KeyX}}, window);
	input.bindAll(std::vector<std::pair<int, WindowButton>>{
		{InputFocus, MouseLeftClick}, {InputTurn, MouseRightClick}}, window);
	
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
	Buffer vertexBuffer(BufferStatic, polyhedron.getSerialVertices().data(), sizeof(float) * MODEL_MAX_VERTICES * 3);
	vertexBuffer.update(polyhedron.getFanCentreVertices().data(), sizeof(float) * polyhedron.getFanCentreVertices().size(), sizeof(float) * polyhedron.getSerialVertices().size());
	Buffer triangleBuffer(BufferStatic, polyhedron.getTriangularFaces().data(), sizeof(int) * MODEL_MAX_FACES);
	Buffer lineBuffer(BufferStatic, polyhedron.getSerialEdges().data(), sizeof(int) * MODEL_MAX_LINES);
	Buffer wheelBuffer(BufferStatic, polyhedron.getFanFaces().data(), sizeof(int) * MODEL_MAX_WHEEL_FACES);
	Index vertexIndex(vertexBuffer, 3, IndexFloat, IndexUnchanged, sizeof(float) * 3, 0);
	Index triangleIndex(triangleBuffer, IndexUint, sizeof(int), 0);
	Index lineIndex(lineBuffer, IndexUint, sizeof(int), 0);
	Index wheelIndex(wheelBuffer, IndexUint, sizeof(int), 0);
	Program basicProgram(std::vector<Shader*>{ &vertexShader, &fragmentShader });
	Program solidwireProgram(std::vector<Shader*>{ &vertexShader, &solidwireGeometryShader, &solidwireFragmentShader });
	DrawArray pointDraw(DrawPoint, std::vector<Index*>{ &vertexIndex }, polyhedron.getSerialVertices().size() / 3);
	DrawElements triangleDraw(DrawTriangle, std::vector<Index*>{ &vertexIndex }, triangleIndex, polyhedron.getTriangularFaces().size());
	DrawElements lineDraw(DrawLine, std::vector<Index*>{ &vertexIndex }, lineIndex, polyhedron.getSerialEdges().size());
	DrawElements solidwireDraw(DrawTriangle, std::vector<Index*>{ &vertexIndex }, wheelIndex, polyhedron.getFanFaces().size());
	
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
	std::array<float, 3> rotateNormal = {-1.f / sqrt(2), 1.f / sqrt(2), 0};
	float rotateMagnitude = 0;
	{
	std::array<float, 16> screenSpace{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1};
	std::array<float, 16> viewProjection = camera.getViewProjection();
	window.getScreenSpace(screenSpace[0], screenSpace[5], screenSpace[3], screenSpace[7]);
	basicProgram.setUniform("vp", DataMatrix4(viewProjection, DataUnchanged));
	solidwireProgram.setUniform("vp", DataMatrix4(viewProjection, DataUnchanged));
	solidwireProgram.setUniform("screen", DataMatrix4(screenSpace, DataUnchanged));
	std::array<float, 16> modelTransform = math::rotate(rotateMagnitude, rotateNormal);
	basicProgram.setUniform("m", DataMatrix4(modelTransform, DataUnchanged));
	solidwireProgram.setUniform("m", DataMatrix4(modelTransform, DataUnchanged));
	}
	
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
			camera.input(input.getPress(InputSelect), input.getHold(InputTurn), input.getPress(InputRelease), move, look);
			
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
			camera.input(input.getPress(InputSelect), input.getHold(InputTurn), input.getPress(InputRelease), move, look);
			
			// view
			if(input.getPress(InputGraphic)){
				if(++renderer == renderers.end()) renderer = renderers.begin();
			}
			if(input.getPress(InputProject)){
				projection.toggle();
				projection.set(camera, window.getAspectRatio());
			}
			
			// model
			if(input.getHold(InputSpin)){
				rotateMagnitude += MODEL_ROTATE_SENS;
				std::array<float, 16> modelTransform = math::rotate(rotateMagnitude, rotateNormal);
				basicProgram.setUniform("m", DataMatrix4(modelTransform, DataUnchanged));
				solidwireProgram.setUniform("m", DataMatrix4(modelTransform, DataUnchanged));
			}
			
			// polyhedron
			bool isMeshChanged = false;
			if(input.getPress(InputDual)){
				debug("Operator dual & reset testing");
				polydata.push_back(polydata.back()); // operate on polyhedron
				PolyhedronFactory::mutate(polydata.back(), 'd');
				polyhedra.push_back(Mesh(polydata.back().vertices, polydata.back().edges, polydata.back().faces));
				operators = "d" + operators;
				isMeshChanged = true;
			}
			if(input.getPress(InputAmbo)){
				debug("Operator ambo");
				polydata.push_back(polydata.back());
				PolyhedronFactory::mutate(polydata.back(), 'a');
				polyhedra.push_back(Mesh(polydata.back().vertices, polydata.back().edges, polydata.back().faces));
				operators = "a" + operators;
				isMeshChanged = true;
			}
			if(input.getPress(InputAkis)){
				debug("Operator akis");
				polydata.push_back(polydata.back());
				PolyhedronFactory::mutate(polydata.back(), 'k');
				polyhedra.push_back(Mesh(polydata.back().vertices, polydata.back().edges, polydata.back().faces));
				operators = "k" + operators;
				isMeshChanged = true;
			}
			if(input.getPress(InputGyro)){
				debug("Operator gyro");
				polydata.push_back(polydata.back());
				PolyhedronFactory::mutate(polydata.back(), 'g');
				polyhedra.push_back(Mesh(polydata.back().vertices, polydata.back().edges, polydata.back().faces));
				operators = "g" + operators;
				isMeshChanged = true;
			}
			if(input.getPress(InputCanon)){
				debug("Operator canon");
				polydata.push_back(polydata.back());
				PolyhedronFactory::mutate(polydata.back(), 'c');
				polyhedra.push_back(Mesh(polydata.back().vertices, polydata.back().edges, polydata.back().faces));
				operators = "c" + operators;
				isMeshChanged = true;
			}
			if(input.getPress(InputRevert)){
				if(polyhedra.size() > 1){
					polyhedra.pop_back();
					polydata.pop_back();
					operators.erase(operators.begin());
					isMeshChanged = true;
				}
			}
			if(isMeshChanged){ // update buffers
				Mesh &mesh = polyhedra.back();
				vertexBuffer.update(mesh.getSerialVertices().data(), sizeof(float) * mesh.getSerialVertices().size(), 0);
				vertexBuffer.update(mesh.getFanCentreVertices().data(), sizeof(float) * mesh.getFanCentreVertices().size(), 
					sizeof(float) * mesh.getSerialVertices().size());
				triangleBuffer.update(mesh.getTriangularFaces().data(), sizeof(int) * mesh.getTriangularFaces().size(), 0);
				lineBuffer.update(mesh.getSerialEdges().data(), sizeof(int) * mesh.getSerialEdges().size(), 0);
				wheelBuffer.update(mesh.getFanFaces().data(), sizeof(int) * mesh.getFanFaces().size(), 0);
				pointDraw.recount(mesh.getSerialVertices().size() / 3);
				triangleDraw.recount(mesh.getTriangularFaces().size());
				lineDraw.recount(mesh.getSerialEdges().size());
				solidwireDraw.recount(mesh.getFanFaces().size());
				debug("new operator stream", operators);
				debug("new mesh count", mesh.getFanFaces().size());
			}
		}
	}
	
	return 0;
}