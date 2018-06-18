#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//--------------------------
#include <freecamera.hpp>
#include <simpleshader.hpp> // I've lost these libraries and can't find them anywhere
#include <blenderimport.hpp>
//--------------------------

#include <string>
#include <vector>

#define SIZE 256

const int mapSize = 1024;

int screenWidth = 1920;
int screenHeight = 1080;

#define lerp(t, a, b) ( a + t * (b - a) )


int main() {

    srand((unsigned)time(NULL));
    //srand(0);

    GLint ***field;
    field = new GLint**[SIZE];
    for (int i = 0; i<SIZE; i++) {
	field[i] = new GLint*[SIZE];
	for (int j = 0; j<SIZE; j++) {
	    field[i][j] = new GLint[SIZE];
	    for (int k = 0; k<SIZE; k++) {
		field[i][j][k] = rand() & 0xf;
	    }
	}
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window* window = SDL_CreateWindow(
	    "Perlin", 
	    100, 
	    100, 
	    screenWidth, 
	    screenHeight, 
	    SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
	    );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    SDL_GL_SetSwapInterval(SDL_FALSE);

    //SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);

    bool cont = true;
    SDL_Event event;

    GLuint vao, vbo, ebo;
    GLuint pr;
    GLuint ssbo;

    blenderImport model;

    model.openFile(std::string("model.b3d"));

    unsigned size = model.getSize();

    float* verts = new float[sizeof(float)*size];
    GLuint* elems = new GLuint[size];

    std::vector<float> *rawverts = model.getVerts();

    for (unsigned i = 0; i<size; i++) {
        verts[i] = rawverts->at(i);
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ssbo);
    glBindBuffer(ssbo, GL_SHADER_STORAGE_BUFFER);

    /*glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE*SIZE*SIZE*sizeof(GLint), NULL, GL_STATIC_DRAW);

    for (int i=0; i<SIZE; i++) {
	for (int j=0; j<SIZE; j++) {
	    glBufferSubData(GL_SHADER_STORAGE_BUFFER, i*SIZE*SIZE + j*SIZE, SIZE*sizeof(GLint), field[i][j]);
	}
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);*/

    //SDL_GL_DeleteContext(context);
    //return 0;


    glGenVertexArrays(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*sizeof(GLuint), elems, GL_STATIC_DRAW);
    simpleShader shader("");
    shader.createProgram();
    shader.addVertexShader("shader.vert");
    shader.addFragmentShader("shader.frag");
    pr = shader.getProgram();
    glUseProgram(pr);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size*sizeof(float), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLuint viewL = glGetUniformLocation(pr, "view");
    GLuint rotL = glGetUniformLocation(pr, "rot");
    GLuint timeL = glGetUniformLocation(pr, "time");

    GLuint tex;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    unsigned char *perlinTexture = new unsigned char[1024*1024];
    for (int i=0; i<1024*1024; i++) {
	    perlinTexture[i] = (rand() % 0x0f)*16;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 1024, 0, GL_RED, GL_UNSIGNED_BYTE, perlinTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    delete[] perlinTexture;

    freeCamera *camera = new freeCamera(glm::vec3(4, 0, 0), glm::vec3(-1, 0, 0));
    camera->setFOV(35);
    camera->setResolution(screenWidth, screenHeight);

    glm::mat4 rotation = glm::mat4(1);

    glUniform1i(glGetUniformLocation(pr, "seed"), rand() & 0xffff);

    bool cameraFixed = true; 

    unsigned frames = 0;
    unsigned lastTime = SDL_GetTicks();

    while (cont) {
	while (SDL_PollEvent(&event)) {
	    switch (event.type) {
		case SDL_QUIT: 
		    {
			cont = false; break;
		    }
		case SDL_KEYDOWN: 
		{
			switch (event.key.keysym.scancode) {
			    case SDL_SCANCODE_W: camera->moveF(true); 
						 break;
			    case SDL_SCANCODE_S: camera->moveB(true);
						 break;
			    case SDL_SCANCODE_A: camera->moveL(true);
						 break;
			    case SDL_SCANCODE_D: camera->moveR(true);
						 break;
                	    case SDL_SCANCODE_C: cameraFixed = !cameraFixed;
						 SDL_SetRelativeMouseMode(cameraFixed?SDL_FALSE:SDL_TRUE);
						 break;
			    case SDL_SCANCODE_SPACE: camera->moveU(true); 
						     break;
			    case SDL_SCANCODE_ESCAPE: cont = false;
						      break;
			    default: break;
			}
			break;
		}
		case SDL_KEYUP: 
		{
		    switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_SPACE: {camera->moveU(false); break;}
			case SDL_SCANCODE_W: camera->moveF(false);
					     break;
			case SDL_SCANCODE_S: camera->moveB(false);
					     break;
			case SDL_SCANCODE_A: camera->moveL(false);
					     break;
			case SDL_SCANCODE_D: camera->moveR(false);
					     break;
			default: break;
		    }
		    break;
		}
		case SDL_MOUSEMOTION: if (!cameraFixed) {
			              camera->rotX(event.motion.xrel/4.f);
				      camera->rotY(event.motion.yrel/4.f);}
				      break;
		case SDL_MOUSEWHEEL: camera->changeFOV(event.wheel.y*-2);
				     break;
	    }

	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->move(.01f);
	glm::mat4 view = camera->getView();
	rotation = glm::rotate(rotation, .001f, glm::vec3(0,0,1));
	glUniformMatrix4fv(viewL, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(rotL, 1, GL_FALSE, &rotation[0][0]);
	glUniform1i(timeL, SDL_GetTicks());
	//glDrawElements(GL_TRIANGLES, (strip-1)*size*6, GL_UNSIGNED_INT, NULL);
	glDrawArrays(GL_TRIANGLES, 0, size);
	glFinish();
	SDL_GL_SwapWindow(window);
	frames++;
	if (SDL_GetTicks() - lastTime >= 5000) {
	    std::cout << frames/5. << " FPS" << std::endl;
	    frames = 0;
	    lastTime = SDL_GetTicks();
	}
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
