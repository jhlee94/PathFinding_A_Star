#pragma warning (disable:4996)

#include <GL\glew.h>

#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>

#include <glm\glm.hpp>
#include <glm/gtx/transform.hpp>

#include <math.h>
#include <string>
#include <fstream>
#include <iostream>

#define PHI 1.61803398875

float current_time, previous_time = 0, fps;
int frame_count=0;

sf::Clock fps_clock, move_clock; 
sf::Font *main_font;

GLuint vao,vbo,cbo;
GLuint programID;

int xpos, ypos;
// position
glm::vec3 position = glm::vec3(0, 0, 20);
// horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

void CalculateFPS();
void PrintString(float x, float y, sf::Text& text, sf::Font *main_font, const char* string, ...);

void InitGL();
void Clean();

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;
	settings.majorVersion = 4;
	settings.minorVersion = 0;

	sf::RenderWindow app_window(sf::VideoMode(1024, 768), "A* Algorithm", sf::Style::Default, settings);
	app_window.setVerticalSyncEnabled(1);
	
	app_window.setActive();
	main_font = new sf::Font();
	main_font->loadFromFile("arial.ttf");
	sf::Text fpsText;
	
	// Init OpenGL
	InitGL();

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection;

	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,-10.0f,10.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View;
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	
	glm::mat4 mvp = Projection * View * Model;
	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	float FoV = initialFoV;
	sf::Event event;
	while (app_window.isOpen()){
		CalculateFPS();

		float deltaTime = move_clock.restart().asSeconds();

		glm::vec3 direction(
			cos(verticalAngle) * sin(horizontalAngle),
			sin(verticalAngle),
			cos(verticalAngle) * cos(horizontalAngle)
			);
		glm::vec3 right = glm::vec3(
			sin(horizontalAngle - 3.14f / 2.0f),
			0,
			cos(horizontalAngle - 3.14f / 2.0f)
			);

		// Up vector : perpendicular to both direction and right
		glm::vec3 up = glm::cross(right, direction);

		while (app_window.pollEvent(event)){
		

			switch (event.type)
			{
			case sf::Event::Closed:
				app_window.close();
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
				}
				if (event.key.code == sf::Keyboard::Down) {
					position -= direction * deltaTime * speed;
				}
				if (event.key.code == sf::Keyboard::Right) {
					position += right * deltaTime * speed;
				}
				if (event.key.code == sf::Keyboard::Left) {
					position -= right * deltaTime * speed;
				}
				break;
			default:
				break;
			}
			
		}
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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 1);

		// 1rst attribute buffer : vertices
		glUseProgram(programID);
		// Send our transformation to the currently bound shader, in the "MVP" uniform
		// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		//// 2nd attribute buffer : colors
		//glEnableVertexAttribArray(1);
		//glBindBuffer(GL_ARRAY_BUFFER, cbo);
		//glVertexAttribPointer(
		//	1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		//	3,                                // size
		//	GL_FLOAT,                         // type
		//	GL_FALSE,                         // normalized?
		//	0,                                // stride
		//	(void*)0                          // array buffer offset
		//	);

		// Draw the triangle !
		glDrawArrays(GL_POINTS, 0, 60); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1);


		glUseProgram(0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		app_window.pushGLStates();
		//app_window.resetGLStates();
		PrintString(5, 16, fpsText, main_font,"FPS: %5.2f", fps);
		app_window.draw(fpsText);
		app_window.popGLStates();
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vao);
		app_window.display();
	}
	
	Clean();
	return 0;
}

void InitGL()
{
	glewExperimental = true;
	glewInit();
	programID = LoadShaders("shader.vert", "shader.frag");


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/*static const GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};*/

	static const GLfloat vertices[] = {
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
	-2*PHI, -1, -(2 + PHI),
	};

	//static const GLfloat vertices[] = {
	//	-1.0f, -1.0f, -1.0f, // triangle 1 : begin
	//	-1.0f, -1.0f, 1.0f,
	//	-1.0f, 1.0f, 1.0f, // triangle 1 : end
	//	1.0f, 1.0f, -1.0f, // triangle 2 : begin
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, 1.0f, -1.0f, // triangle 2 : end
	//	1.0f, -1.0f, 1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	1.0f, 1.0f, -1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, 1.0f, 1.0f,
	//	-1.0f, 1.0f, -1.0f,
	//	1.0f, -1.0f, 1.0f,
	//	-1.0f, -1.0f, 1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, 1.0f, 1.0f,
	//	-1.0f, -1.0f, 1.0f,
	//	1.0f, -1.0f, 1.0f,
	//	1.0f, 1.0f, 1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	1.0f, 1.0f, -1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	1.0f, 1.0f, 1.0f,
	//	1.0f, -1.0f, 1.0f,
	//	1.0f, 1.0f, 1.0f,
	//	1.0f, 1.0f, -1.0f,
	//	-1.0f, 1.0f, -1.0f,
	//	1.0f, 1.0f, 1.0f,
	//	-1.0f, 1.0f, -1.0f,
	//	-1.0f, 1.0f, 1.0f,
	//	1.0f, 1.0f, 1.0f,
	//	-1.0f, 1.0f, 1.0f,
	//	1.0f, -1.0f, 1.0f
	//};

	// One color for each vertex. They were generated randomly.
	static const GLfloat colors[] = {
		0.583f, 0.771f, 0.014f,
		0.609f, 0.115f, 0.436f,
		0.327f, 0.483f, 0.844f,
		0.822f, 0.569f, 0.201f,
		0.435f, 0.602f, 0.223f,
		0.310f, 0.747f, 0.185f,
		0.597f, 0.770f, 0.761f,
		0.559f, 0.436f, 0.730f,
		0.359f, 0.583f, 0.152f,
		0.483f, 0.596f, 0.789f,
		0.559f, 0.861f, 0.639f,
		0.195f, 0.548f, 0.859f,
		0.014f, 0.184f, 0.576f,
		0.771f, 0.328f, 0.970f,
		0.406f, 0.615f, 0.116f,
		0.676f, 0.977f, 0.133f,
		0.971f, 0.572f, 0.833f,
		0.140f, 0.616f, 0.489f,
		0.997f, 0.513f, 0.064f,
		0.945f, 0.719f, 0.592f,
		0.543f, 0.021f, 0.978f,
		0.279f, 0.317f, 0.505f,
		0.167f, 0.620f, 0.077f,
		0.347f, 0.857f, 0.137f,
		0.055f, 0.953f, 0.042f,
		0.714f, 0.505f, 0.345f,
		0.783f, 0.290f, 0.734f,
		0.722f, 0.645f, 0.174f,
		0.302f, 0.455f, 0.848f,
		0.225f, 0.587f, 0.040f,
		0.517f, 0.713f, 0.338f,
		0.053f, 0.959f, 0.120f,
		0.393f, 0.621f, 0.362f,
		0.673f, 0.211f, 0.457f,
		0.820f, 0.883f, 0.371f,
		0.982f, 0.099f, 0.879f
	};

	GLubyte pentagon[12][5] = {
		{ 13, 23, 35, 47, 12 },
		{ 18, 59, 11, 27, 16 },
		{ 19, 30, 7, 26, 55 },
		{ 21, 24, 40, 4, 6 },
		{ 43, 3, 32, 25, 42 },
		{ 44, 9, 33, 52, 51 },
		{ 46, 54, 0, 50, 8 },
		{ 48, 17, 37, 22, 56 },
		{ 49, 31, 15, 20, 29 },
		{ 5, 39, 38, 45, 14 },
		{ 53, 1, 58, 10, 34 },
		{ 57, 28, 36, 2, 41 }
	};

	GLubyte hexagon[20][6] = {
		{ 10, 14, 45, 40, 4, 58 },
		{ 11, 26, 7, 0, 50, 27 },
		{ 18, 59, 39, 38, 43, 42 },
		{ 2, 36, 6, 21, 49, 29 },
		{ 20, 37, 22, 41, 2, 29 },
		{ 25, 51, 52, 15, 31, 32 },
		{ 26, 11, 59, 39, 5, 55 },
		{ 28, 1, 53, 12, 47, 57 },
		{ 28, 1, 58, 4, 6, 36 },
		{ 31, 32, 3, 24, 21, 49 },
		{ 34, 19, 30, 13, 12, 53 },
		{ 37, 17, 33, 52, 15, 20 },
		{ 41, 22, 56, 35, 47, 57 },
		{ 44, 9, 8, 50, 27, 16 },
		{ 45, 38, 43, 3, 24, 40 },
		{ 48, 46, 54, 23, 35, 56 },
		{ 5, 55, 19, 34, 10, 14 },
		{ 51, 25, 42, 18, 16, 44 },
		{ 7, 0, 54, 23, 13, 30 },
		{ 9, 8, 46, 48, 17, 33 }
	};

	// Generate 1 buffer, put the resulting identifier in vbo
	glGenBuffers(1, &vbo);
	// The following commands will talk about our 'vbo' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &cbo);
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);
}

void Clean()
{
	delete main_font;

	GLenum errCheckVal = glGetError();

	glDisable(GL_DEPTH_TEST);

	glDisableVertexAttribArray(0);
	//glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(0, &vbo);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);

	glDeleteProgram(programID);

	errCheckVal = glGetError();
	if (errCheckVal != GL_NO_ERROR) {
		std::cout << "Error: Couldn't destroy the VBO:" << gluErrorString(errCheckVal) << std::endl;
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
	text.setColor(sf::Color::White);
}

void CalculateFPS()
{
	//  Increase frame count
	frame_count++;

	//  Get the number of milliseconds since glutInit called
	//  (or first call to glutGet(GLUT ELAPSED TIME)).
	current_time = fps_clock.getElapsedTime().asMilliseconds();
	//  Calculate time passed
	int timeInterval = current_time - previous_time;

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