#pragma warning (disable:4996)

#include <GL\glew.h>

#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>

#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

#include <math.h>
#include <string>
#include <fstream>
#include <iostream>

#include "PathFinding.h"

#define PHI 1.61803398875f

sf::RenderWindow *app_window;

float current_time, previous_time = 0, fps;
int frame_count=0;

sf::Clock fps_clock, move_clock; 
sf::Font *main_font;
sf::Text fpsText;

GLuint MatrixID;
glm::mat4 Projection;
// Camera matrix
glm::mat4 View;
glm::mat4 Model;
glm::mat4 mvp;

GLuint vao,vbo,cbo;
GLuint edge_vbo, edge_cbo;
GLuint programID;

int xpos, ypos;
// position
glm::vec3 position = glm::vec3(0, 0, 20);
glm::vec3 direction;
glm::vec3 right;
glm::vec3 up;

// horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float verticalAngle = 0.0f;
// Initial Field of View
float FoV = 45.0f;

float speed = 5.0f; // 5 units / second
float mouseSpeed = 0.005f;

PathFinding path;

std::string s, g, r;
sf::Text start;
sf::Text goal;
sf::Text current;

bool startBool = false, goalBool = false;

// FUNCTIONS
void CalculateFPS();
void PrintString(float x, float y, sf::Text& text, sf::Font *main_font, const char* string, ...);
void UpdatePath();
void TextCommandLoop();

void InitGL();
void InitSFML();
void InitGUI();
void Clean();
void HandleInput();
void DrawGL();
void DrawSFML();
void UpdateMVP();

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line)
{

	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	if (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n",
			file, line, gluErrorString(glErr));
		retCode = 1;
	}
	return retCode;
}

int main()
{
	path.Initialise();
	// Init SFML
	InitSFML();

	//InitGUI();

	// Init OpenGL
	InitGL();
	///path.edge_list.at(0).cost = 25;
	path.edge_list.at(0).isPassable = false;
	path.edge_list.at(9).isPassable = false;
	//path.edge_list.at(10).isPassable = false;
	//path.edge_list.at(11).isPassable = false;
	//path.edge_list.at(12).isPassable = false;
	//path.edge_list.at(13).isPassable = false;
	//path.edge_list.at(14).isPassable = false;
	//path.edge_list.at(15).isPassable = false;
	//path.edge_list.at(16).isPassable = false;
	//path.edge_list.at(18).isPassable = false;
	//path.edge_list.at(19).isPassable = false;
	//path.edge_list.at(20).isPassable = false;
	//path.edge_list.at(28).isPassable = false;
	//path.edge_list.at(60).isPassable = false;
	////path.nodes.at(10).isWalkable = false;
	//path.nodes.at(40).isWalkable = false;
	//path.nodes.at(16).isWalkable = false;
	//path.nodes.at(12).isWalkable = false;
	//path.nodes.at(36).isWalkable = false;
	//path.nodes.at(33).isWalkable = false;
	while (app_window->isOpen()){
		CalculateFPS();
		HandleInput();
		UpdateMVP();
		DrawGL();
		DrawSFML();
		app_window->display();
		//TextCommandLoop();
	}
	Clean();
	system("PAUSE");
	return 0;
}

void InitGL()
{
	glewExperimental = true;
	glewInit();
	programID = LoadShaders("shader.vert", "shader.frag");


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLfloat vertices[] = {
	0, 1, 3*PHI,
	0, 1, -3*PHI,
	0, -1, 3*PHI,
	0, -1, -3*PHI,
	1, 3*PHI, 0,
	1, -3*PHI, 0,
	-1, 3*PHI, 0,
	-1, -3*PHI, 0,
	3*PHI, 0, 1,
	3*PHI, 0, -1,
	-3*PHI, 0, 1,
	-3*PHI, 0, -1,
	2, (1 + 2*PHI), PHI,
	2, (1 + 2*PHI), -PHI,
	2, -(1 + 2*PHI), PHI,
	2, -(1 + 2*PHI), -PHI,
	-2, (1 + 2*PHI), PHI,
	-2, (1 + 2*PHI), -PHI,
	-2, -(1 + 2*PHI), PHI,
	-2, -(1 + 2*PHI), -PHI,
	(1 + 2*PHI), PHI, 2,
	(1 + 2*PHI), PHI, -2,
	(1 + 2*PHI), -PHI, 2,
	(1 + 2*PHI), -PHI, -2,
	-(1 + 2*PHI), PHI, 2,
	-(1 + 2*PHI), PHI, -2,
	-(1 + 2*PHI), -PHI, 2,
	-(1 + 2*PHI), -PHI, -2,
	PHI, 2, (1 + 2*PHI),
	PHI, 2, -(1 + 2*PHI),
	PHI, -2, (1 + 2*PHI),
	PHI, -2, -(1 + 2*PHI),
	-PHI, 2, (1 + 2*PHI),
	-PHI, 2, -(1 + 2*PHI),
	-PHI, -2, (1 + 2*PHI),
	-PHI, -2, -(1 + 2*PHI),
	1, (2 + PHI), 2*PHI,
	1, (2 + PHI), -2*PHI,
	1, -(2 + PHI), 2*PHI,
	1, -(2 + PHI), -2*PHI,
	-1, (2 + PHI), 2*PHI,
	-1, (2 + PHI), -2*PHI,
	-1, -(2 + PHI), 2*PHI,
	-1, -(2 + PHI), -2*PHI,
	(2 + PHI), 2*PHI, 1,
	(2 + PHI), 2*PHI, -1,
	(2 + PHI), -2*PHI, 1,
	(2 + PHI), -2*PHI, -1,
	-(2 + PHI), 2*PHI, 1,
	-(2 + PHI), 2*PHI, -1,
	-(2 + PHI), -2*PHI, 1,
	-(2 + PHI), -2*PHI, -1,
	2*PHI, 1, (2 + PHI),
	2*PHI, 1, -(2 + PHI),
	2*PHI, -1, (2 + PHI),
	2*PHI, -1, -(2 + PHI),
	-2*PHI, 1, (2 + PHI),
	-2*PHI, 1, -(2 + PHI),
	-2*PHI, -1, (2 + PHI),
	-2*PHI, -1, -(2 + PHI)
	};

	// One color for each vertex. They were generated randomly.
	GLfloat colors[180];
	for (int i = 0; i < 180; i++)
	{
		colors[i] = 0.0f;
	}

	GLfloat edges[6 * 90];
	GLfloat edge_colors[6 * 90];
	int count = 0;
	for (int i = 0; i < 6 * 90; i += 6)
	{
		glm::vec3 a(path.nodes.at(path.edge_list.at(count).nodes[0]).m_position);
		glm::vec3 b(path.nodes.at(path.edge_list.at(count).nodes[1]).m_position);
		edges[i] = a.x;
		edges[i + 1] = a.y;
		edges[i + 2] = a.z;
		edges[i + 3] = b.x;
		edges[i + 4] = b.y;
		edges[i + 5] = b.z;

		edge_colors[i] = 0.f;
		edge_colors[i+1] = 0.f;
		edge_colors[i+2] = 0.f;
		edge_colors[i+3] = 0.f;
		edge_colors[i+4] = 0.f;
		edge_colors[i+5] = 0.f;
		count++;
	}


	// Generate 1 buffer, put the resulting identifier in vbo
	glGenBuffers(1, &vbo);
	// The following commands will talk about our 'vbo' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &cbo);
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	// Generate 1 buffer, put the resulting identifier in vbo
	glGenBuffers(1, &edge_vbo);
	// The following commands will talk about our 'vbo' buffer
	glBindBuffer(GL_ARRAY_BUFFER, edge_vbo);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(edges), edges, GL_STATIC_DRAW);

	glGenBuffers(1, &edge_cbo);
	glBindBuffer(GL_ARRAY_BUFFER, edge_cbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(edge_colors), edge_colors, GL_STATIC_DRAW);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Model = glm::mat4(1.0f);

	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	MatrixID = glGetUniformLocation(programID, "MVP");
	//glClearColor(1, 1, 1, 1);
}

void InitSFML()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	settings.majorVersion = 4;
	settings.minorVersion = 0;

	app_window = new sf::RenderWindow(sf::VideoMode(1024, 768), "A* Algorithm", sf::Style::Default, settings);
	app_window->setVerticalSyncEnabled(1);

	main_font = new sf::Font();
	main_font->loadFromFile("arial.ttf");
}

void Clean()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(0, &vbo);
	glDeleteBuffers(0, &edge_vbo);
	glDeleteBuffers(0, &cbo);
	glDeleteBuffers(0, &edge_cbo);
	

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindVertexArray(0);
	glDeleteVertexArrays(0, &vao);

	glDeleteProgram(programID);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	printOpenGLError();

	delete main_font;
	delete app_window;
}

void TextCommandLoop()
{
	int start = 0, goal = 0;
	std::cout << "WELCOME TO A* PATHFINDING" << std::endl;
	std::cout << "Please Enter the start node and the goal node: " << std::endl;
	std::cin >> start >> goal;
	std::cout << start << " " << goal << std::endl;
	path.Reset();
	path.FindPath(start, goal);
}

void HandleInput()
{
	sf::Event event;
	float deltaTime = move_clock.restart().asSeconds();

	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);

	// Up vector : perpendicular to both direction and right
	up = glm::cross(right, direction);

	while (app_window->pollEvent(event)){
		//m_desktop.HandleEvent(event);
		switch (event.type)
		{
		case sf::Event::Closed:
			app_window->close();
			break;
		case sf::Event::Resized:
			glViewport(0, 0, event.size.width, event.size.height);
			break;
		case sf::Event::MouseWheelMoved:
			FoV += 5 * event.mouseWheel.delta*deltaTime;
			break;
		case sf::Event::MouseMoved:
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				xpos = event.mouseMove.x;
				ypos = event.mouseMove.y;

				// Compute new orientation
				horizontalAngle += mouseSpeed * deltaTime * float(1024 / 2 - xpos);
				verticalAngle += mouseSpeed * deltaTime * float(768 / 2 - ypos);
			}
			//std::cout << "mouse moved" << std::endl;
			break;
		case sf::Event::KeyPressed:
			if (event.key.code == sf::Keyboard::Up) {
				position += direction * deltaTime * speed;
				break;
			}
			if (event.key.code == sf::Keyboard::Down) {
				position -= direction * deltaTime * speed;
				break;
			}
			if (event.key.code == sf::Keyboard::Right) {
				position += right * deltaTime * speed;
				break;
			}
			if (event.key.code == sf::Keyboard::Left) {
				position -= right * deltaTime * speed;
				break;
			}
			if (event.key.code == sf::Keyboard::W) {
				Model *= glm::rotate(-deltaTime*speed, glm::vec3(1, 0, 0));
				break;
			}
			if (event.key.code == sf::Keyboard::S) {
				Model *= glm::rotate(deltaTime*speed, glm::vec3(1, 0, 0));
				break;
			}
			if (event.key.code == sf::Keyboard::D) {
				Model *= glm::rotate(deltaTime*speed, glm::vec3(0, 1, 0));
				break;
			}
			if (event.key.code == sf::Keyboard::A) {
				Model *= glm::rotate(-deltaTime*speed, glm::vec3(0, 1, 0));
				break;
			}
			if (event.key.code == sf::Keyboard::R) {
				path.Reset();
				break;
			}
			break;
		case sf::Event::TextEntered:
			// Handle ASCII numbers only
			if (event.text.unicode < 57)
			{
				if (event.text.unicode == 13) {
					if (!startBool) {
						startBool = true;
					}
					else if (startBool && !goalBool){
						startBool = false;
						goalBool = false;
						int startVal = atoi(s.c_str()); //value = 45 
						int goalVal = atoi(g.c_str());
						path.Reset();
						path.FindPath(startVal, goalVal);
						r.clear();
						r = s + "-" + g;
						s.clear();
						g.clear();
					}
				}
				else {
					if (!startBool && !goalBool) {
						s.push_back((char)event.text.unicode);
						std::cout << s << std::endl;
					}
					else if(startBool && !goalBool){
						g.push_back((char)event.text.unicode);
						std::cout << g << std::endl;
					}
				}
			}	
			break;
		default:
			break;
		}

	}
}

void PrintString(float x, float y, sf::Text& text, sf::Font *main_font, const char* string, ...){
	char buffer[128];
	va_list arg;
	_crt_va_start(arg, string);
	vsprintf(buffer, string, arg);
	_crt_va_end(arg);

	if (!text.getFont())
		text.setFont(*main_font);
	text.setCharacterSize(15);
	text.setPosition(x, y);
	text.setString(buffer);
	text.setColor(sf::Color::Green);
}

void CalculateFPS()
{
	//  Increase frame count
	frame_count++;

	//  Get the number of milliseconds since glutInit called
	//  (or first call to glutGet(GLUT ELAPSED TIME)).
	current_time = static_cast<float>(fps_clock.getElapsedTime().asMilliseconds());
	//  Calculate time passed
	int timeInterval = (int)(current_time - previous_time);

	if (timeInterval > 1000)
	{
		//  calculate the number of frames per second
		fps = frame_count / (timeInterval / 1000.0f);

		//  Set time
		previous_time = current_time;
		//  Reset frame count
		frame_count = 0;
	}
}

void UpdateMVP()
{
	// Projection matrix : 45&deg; Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	View = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
		);
	// Our ModelViewProjection : multiplication of our 3 matrices
	mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
}

void DrawGL()
{
	if (path.updated) {
		UpdatePath();
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);

	// 1rst attribute buffer : vertices
	glUseProgram(programID);
	glPointSize(10);
	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_TRUE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	//// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// Draw the triangle !
	glDrawArrays(GL_POINTS, 0, 60); // Starting from vertex 0; 3 vertices total -> 1 triangle

	glEnableVertexAttribArray(0); 
	glLineWidth(5);
	glBindBuffer(GL_ARRAY_BUFFER, edge_vbo);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_TRUE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, edge_cbo);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	glDrawArrays(GL_LINES, 0, 180);
}

void DrawSFML()
{
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	app_window->pushGLStates();
	//app_window->resetGLStates();
	PrintString(5, 16, fpsText, main_font, "FPS: %5.2f", fps);
	app_window->draw(fpsText);
	PrintString(5, 35, start, main_font, "Start: %s", s.c_str());
	app_window->draw(start);
	PrintString(5, 55, goal, main_font, "Goal: %s", g.c_str());
	app_window->draw(goal);
	PrintString(5, 75, current, main_font, "Current Path: %s", r.c_str());
	app_window->draw(current);
	//m_desktop.Update(1.0f);
	//// Draw the GUI
	//m_sfgui.Display(*app_window);

	app_window->popGLStates();
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vao);
}

void UpdatePath()
{
	glDeleteBuffers(0, &cbo);
	GLfloat colors[180];
	for (int i = 0; i < 180; i+=3)
	{
		colors[i] = 0.0f;
		colors[i + 1] = 0.0f;
		colors[i + 2] = 0.0f;
	}
	for (Node node : path.nodes) {
		if (!node.isWalkable) {
			int idx = node.m_ID;
			colors[idx * 3] = 1.0f;
			colors[idx * 3 + 1] = 1.0f;
			colors[idx * 3 + 2] = 1.0f;
		}
	}
	for (Node node : path.final_paths) {
		int idx = node.m_ID;
		colors[idx * 3] = 0.0f;
		colors[idx * 3 + 1] = 1.0f;
		colors[idx * 3 + 2] = 0.0f;
	}

	glGenBuffers(1, &cbo);
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);


	glDeleteBuffers(0, &edge_cbo);
	GLfloat edge_colors[6 * 90];
	for (int i = 0; i < 6 * 90; i += 6)
	{
		// a
		edge_colors[i] = 0.f;
		edge_colors[i + 1] = 0.f;
		edge_colors[i + 2] = 0.f;

		//b
		edge_colors[i + 3] = 0.f;
		edge_colors[i + 4] = 0.f;
		edge_colors[i + 5] = 0.f;
	}
	for (PathFinding::Edge edge : path.edge_list){
		if (!edge.isPassable) {
			int idx = edge.id;
			// a
			edge_colors[idx * 6] = 1.f;
			edge_colors[idx * 6 + 1] = 1.f;
			edge_colors[idx * 6 + 2] = 1.f;

			//b
			edge_colors[idx * 6 + 3] = 1.f;
			edge_colors[idx * 6 + 4] = 1.f;
			edge_colors[idx * 6 + 5] = 1.f;
		}
	}
	for (PathFinding::Edge edge : path.final_edges){
		int idx = edge.id;
		// a
		edge_colors[idx*6] = 0.f;
		edge_colors[idx*6 + 1] = 1.f;
		edge_colors[idx*6 + 2] = 0.f;

		//b
		edge_colors[idx*6 + 3] = 0.f;
		edge_colors[idx*6 + 4] = 1.f;
		edge_colors[idx*6 + 5] = 0.f;
	}
	glGenBuffers(1, &edge_cbo);
	glBindBuffer(GL_ARRAY_BUFFER, edge_cbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(edge_colors), edge_colors, GL_STATIC_DRAW);

	path.updated = false;
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()){
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()){
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0){
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}