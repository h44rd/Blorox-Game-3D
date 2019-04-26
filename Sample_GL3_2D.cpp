#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <irrKlang.h>
using namespace std;
using namespace irrklang;
void rotInit(int);
void gameover();
void rotatecube();
int checkIfTheEffingBlockIsOnTheBoard();
void increaseLevel();
int checkPres(int,int);
void changeCam(int);
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;
    GLuint Normal;
    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
    GLuint ModelID;
} Matrices;
struct cameraVectors
{
    int type;
    glm::vec3 eye;
    glm::vec3 target;
    glm::vec3 up;
}cv;
double last_update_time, current_time;
int  SunRev=0;
int startRotUP=0;
int Level=10;
double blink;
int blinkPar=0;
int Moves=0;
ISoundEngine* engine;
//camera
double mouse_x,mouse_y;
double camx,camy,camz,r,theta,phi;
double cam_rev_speed;
bool firstMouse = true;
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 0.0f,  1.0f);
GLfloat yaw    = 0.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch  =  0.0f;
int fbwidth,fbheight;
GLuint WIDTH =800,HEIGHT=800;
GLfloat lastX  =   WIDTH / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
GLfloat fov =  45.0f;
double color_change_speed;
bool keys[1024];
float fallz=0;
float triangle_rot_dir = 1;
float cube_rot_dir = 1;
bool triangle_rot_status = true;
bool cube_rot_status = true;
float zoom=1;
int orthpers=0;
int fell=0;
float falltime=50,startFall=-1;
GLuint programID;
GLuint programIDcube;
GLuint lightPosID ;
GLuint lightColorID ;
GLuint objectColorID ;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    engine->drop();
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data,const GLfloat* normal_buffer_data ,GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors
    glGenBuffers (1, &(vao->Normal));
    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                           0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                           6 * sizeof(GLfloat),                  // stride
                          (void*)0            // array buffer offset
                          );
  glBindBuffer (GL_ARRAY_BUFFER, vao->Normal); // Bind the VBO normals
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), normal_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
                        2,                  // attribute 1. Color
                        3,                  // size (r,g,b)
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                         0,                  // stride
                        (void*)0           // array buffer offset
                        );

    return vao;
}

void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glEnableVertexAttribArray(2);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->Normal);
    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(orthpers==0)
    {
        if(yoffset==-1)
        {
            if(zoom < 2)
            {
                zoom += 0.01;
            }
        }
        else
        {
            if(zoom-0.01 > 0)
            {
                zoom -= (0.01);
            }
        }
    }

}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                cube_rot_status = !cube_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;

            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_UP:
            if(fell==0)
            {
                rotInit(1);
                startRotUP=1;
            }
                break;

            case GLFW_KEY_LEFT:
            if(fell==0)
            {
                rotInit(2);
                startRotUP=1;
            }
                break;
            case GLFW_KEY_DOWN:
            if(fell==0)
            {
                rotInit(3);
                startRotUP=1;
            }
                break;
            case GLFW_KEY_RIGHT:
            if(fell==0)
            {
                rotInit(4);
                startRotUP=1;
            }
                break;
            case GLFW_KEY_P:
                SunRev=(SunRev+1)%2;
                break;
            case GLFW_KEY_L:
                increaseLevel();
                break;
            case GLFW_KEY_1:
                changeCam(1);
                break;
            case GLFW_KEY_2:
                changeCam(2);
                break;
            case GLFW_KEY_K:
                orthpers=(orthpers+1)%2;
                break;
            case GLFW_KEY_3:
                changeCam(3);
                break;
            case GLFW_KEY_4:
                changeCam(4);
                break;
            case GLFW_KEY_5:
                changeCam(5);
                break;
            default:
                break;
        }
    }
}

void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                cube_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    fbwidth=width;
    fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);


	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    //Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
    Matrices.projection = glm::ortho(-4.0f*zoom, 4.0f*zoom, -4.0f*zoom, 4.0f*zoom, 0.1f, 500.0f);
}

VAO *triangle, *cube;
float camera_rotation_angle = 90;
float colorTheta = 0;
float colorTheta2 = 0;
float cube_rotation = 0;
float triangle_rotation = 0;


int X,Y,Z;
struct Player {
    float x,y;
    int orientation;
    int parity;
    float angle;
    float angularSpeed;
    int rotType;
    glm::vec3 rotAxis;
    glm::vec3 helprot;
    glm::vec3 cubescale;
}BlockInfo;
typedef struct Player Player;



class Cube
{
private:
    VAO* vao;
    float x,y,z;

public:
    VAO* createCube()
    {
      static const GLfloat vertex_buffer_data [] = {
          -0.5f,-0.5f,-0.5f,
          0.5f,-0.5f,-0.5f,
          0.5f,0.5f,-0.5f,
          0.5f,0.5f,-0.5f,
          -0.5f,0.5f,-0.5f,
          -0.5f,-0.5f,-0.5f,
          -0.5f,-0.5f,0.5f,
          0.5f,-0.5f,0.5f,
          0.5f,0.5f,0.5f,
          0.5f,0.5f,0.5f,
          -0.5f,0.5f,0.5f,
          -0.5f,-0.5f,0.5f,
          -0.5f,0.5f,0.5f,
          -0.5f,0.5f,-0.5f,
          -0.5f,-0.5f,-0.5f,
          -0.5f,-0.5f,-0.5f,
          -0.5f,-0.5f,0.5f,
          -0.5f,0.5f,0.5f,
          0.5f,0.5f,0.5f,
          0.5f,0.5f,-0.5f,
          0.5f,-0.5f,-0.5f,
          0.5f,-0.5f,-0.5f,
          0.5f,-0.5f,0.5f,
          0.5f,0.5f,0.5f,
          -0.5f,-0.5f,-0.5f,
          0.5f,-0.5f,-0.5f,
          0.5f,-0.5f,0.5f,
          0.5f,-0.5f,0.5f,
          -0.5f,-0.5f,0.5f,
          -0.5f,-0.5f,-0.5f,
          -0.5f,0.5f,-0.5f,
          0.5f,0.5f,-0.5f,
          0.5f,0.5f,0.5f,
          0.5f,0.5f,0.5f,
          -0.5f,0.5f,0.5f,
          -0.5f,0.5f,-0.5f,
        };
      static const GLfloat color_buffer_data [] = {
    	1.0f, 1.0f, 0.0f,
    	1.0f, 1.0f, 0.0f,
    	1.0f, 1.0f, 0.0f,
    	1.0f, 1.0f, 0.0f,
    	1.0f, 1.0f, 0.0f,
    	1.0f, 1.0f, 0.0f,
    	1.0f, 0.0f, 1.0f,
    	1.0f, 0.0f, 1.0f,
    	1.0f, 0.0f, 1.0f,
    	1.0f, 0.0f, 1.0f,
    	1.0f, 0.0f, 1.0f,
    	1.0f, 0.0f, 1.0f,
    	0.0f, 1.0f, 1.0f,
    	0.0f, 1.0f, 1.0f,
    	0.0f, 1.0f, 1.0f,
    	0.0f, 1.0f, 1.0f,
    	0.0f, 1.0f, 1.0f,
    	0.0f, 1.0f, 1.0f,
    	1.0f, 0.0f, 0.0f,
    	1.0f, 0.0f, 0.0f,
    	1.0f, 0.0f, 0.0f,
    	1.0f, 0.0f, 0.0f,
    	1.0f, 0.0f, 0.0f,
    	1.0f, 0.0f, 0.0f,
    	0.0f, 1.0f, 0.0f,
    	0.0f, 1.0f, 0.0f,
    	0.0f, 1.0f, 0.0f,
    	0.0f, 1.0f, 0.0f,
    	0.0f, 1.0f, 0.0f,
    	0.0f, 1.0f, 0.0f,
    	0.0f, 0.0f, 1.0f,
    	0.0f, 0.0f, 1.0f,
    	0.0f, 0.0f, 1.0f,
    	0.0f, 0.0f, 1.0f,
    	0.0f, 0.0f, 1.0f,
    	0.0f, 0.0f, 1.0f,
        };
        static const GLfloat normal_buffer_data [] = {
            0.0f,0.0f,-1.0f,
            0.0f,0.0f,-1.0f,
            0.0f,0.0f,-1.0f,
            0.0f,0.0f,-1.0f,
            0.0f,0.0f,-1.0f,
            0.0f,0.0f,-1.0f,
            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f,
            -1.0f,0.0f,0.0f,
            -1.0f,0.0f,0.0f,
            -1.0f,0.0f,0.0f,
            -1.0f,0.0f,0.0f,
            -1.0f,0.0f,0.0f,
            -1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,
            1.0f,0.0f,0.0f,
            0.0f,-1.0f,0.0f,
            0.0f,-1.0f,0.0f,
            0.0f,-1.0f,0.0f,
            0.0f,-1.0f,0.0f,
            0.0f,-1.0f,0.0f,
            0.0f,-1.0f,0.0f,
            0.0f,1.0f,0.0f,
            0.0f,1.0f,0.0f,
            0.0f,1.0f,0.0f,
            0.0f,1.0f,0.0f,
            0.0f,1.0f,0.0f,
            0.0f,1.0f,0.0f,
        };
        vao = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data,normal_buffer_data, GL_FILL);
        return vao;
    }
    void draw3DObject()
    {
        // Change the Fill Mode for this object
        glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

        // Bind the VAO to use
        glBindVertexArray (vao->VertexArrayID);

        // Enable Vertex Attribute 0 - 3d Vertices
        glEnableVertexAttribArray(0);
        // Bind the VBO to use
        glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

        // Enable Vertex Attribute 1 - Color
        glEnableVertexAttribArray(1);
        // Bind the VBO to use
        glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
        glEnableVertexAttribArray(2);
        // Bind the VBO to use
        glBindBuffer(GL_ARRAY_BUFFER, vao->Normal);
        // Draw the geometry !
        glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
    }
}Block;

class Board
{
private:

public:
    int width,length;
    int boardMat[100][100];
    int keyx,keyy;
    int brdx,brdy;
    void createBoard()
    {
        for(int i=0;i<width;i++)
        {
            for(int j=0;j<length;j++)
            {
                boardMat[i][j]=rand()%3;
                //cout<<board[i][j]<<" ";
            }
        }
        keyx=rand()%width;
        keyy=rand()%length;
        boardMat[keyx][keyy]=3;
        for(int i=0;i<length;i++)
        {
            if(i<length/2)
            {
                boardMat[i][0]=rand()%2+1;
            }
            else
            {
                boardMat[i][width-1]=rand()%2+1;
            }
        }
        boardMat[length-1][width-1]=4;
        brdx=rand()%width;
        brdy=rand()%length;
        for(int i=0;i<5;i++)
        {
            if(boardMat[brdx][brdy+i]!=boardMat[keyx][keyy] && boardMat[brdx][brdy+i]!=4 && brdx!=0 && brdy!=0)
            {
                boardMat[brdx][brdy+i]=0;
            }
        }
        boardMat[length-1][width-1]=4;
        boardMat[0][0]=1;
    }
    void changeWidthLength(int w,int l)
    {
        width=w;
        length=l;
    }
    void fillBridge()
    {
        for(int i=0;i<5;i++)
        {
            if(boardMat[brdx][brdy+i]!=boardMat[keyx][keyy] && boardMat[brdx][brdy+i]!=4 && brdx!=0 && brdy!=0)
            {
                boardMat[brdx][brdy+i]=1;
            }
        }
        boardMat[length-1][width-1]=4;
        boardMat[0][0]=1;
    }

}board;


void newGame()
{
    fallz=0;
    startFall=-1;
    engine->play2D("bell.wav");
    Moves=0;
    BlockInfo.x=0;
    BlockInfo.y=0;
    BlockInfo.angle=0;
    BlockInfo.orientation=0;
    BlockInfo.angularSpeed=180.0f;
    board.changeWidthLength(Level,Level);
    board.createBoard();
    BlockInfo.helprot=glm::vec3(0,4.0/board.length,8.0/board.width);
    BlockInfo.parity=-1;
    BlockInfo.rotAxis=glm::vec3(1,0,0);
    BlockInfo.cubescale=glm::vec3(8.0/board.width,8.0/board.length,2*8.0/(board.length));
    X=1;
    Y=1;
    Z=2;
    Block.createCube();
}

void increaseLevel()
{
    if(Level*2<=80)
    {
        Level*=2;
    }
    else
    {
        Level=10;
    }
    newGame();
}

void rotInit(int R)
{
    if(startRotUP==0 && fell==0)
    {
        BlockInfo.rotType=R;
        if(R==1)
        {
            startRotUP=1;
            BlockInfo.helprot=glm::vec3(0,Y*-4.0/board.length,Z*4.0/board.width);
            BlockInfo.parity=-1;
            BlockInfo.rotAxis=glm::vec3(1,0,0);
        }
        if(R==2)
        {
            startRotUP=1;
            BlockInfo.helprot=glm::vec3(X*4.0/board.length,0,Z*4.0/board.length);
            BlockInfo.parity=-1;
            BlockInfo.rotAxis=glm::vec3(0,1,0);
        }
        if(R==3)
        {
            startRotUP=1;
            BlockInfo.helprot=glm::vec3(0,Y*4.0/board.length,Z*4.0/board.width);
            BlockInfo.parity=1;
            BlockInfo.rotAxis=glm::vec3(1,0,0);
        }
        if(R==4)
        {
            startRotUP=1;
            BlockInfo.helprot=glm::vec3(X*-4.0/board.length,0,Z*4.0/board.length);
            BlockInfo.parity=1;
            BlockInfo.rotAxis=glm::vec3(0,1,0);
        }
        Moves++;
    }
}
void rotatecube()
{
        double deltaT=current_time - last_update_time;
        BlockInfo.angle+=(BlockInfo.parity*deltaT*BlockInfo.angularSpeed);
        if(BlockInfo.parity*BlockInfo.angle>=90)
        {
            BlockInfo.angle=0;
            startRotUP=0;
            if(X==1 && Y==1 && Z==2)
            {
                if(BlockInfo.rotType==1)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.y+=1.5f;
                    X=1;
                    Y=2;
                    Z=1;
                }
                if(BlockInfo.rotType==2)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.x-=1.5f;
                    X=2;
                    Y=1;
                    Z=1;
                }
                if(BlockInfo.rotType==3)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.y-=1.5f;
                    X=1;
                    Y=2;
                    Z=1;
                }
                if(BlockInfo.rotType==4)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.x+=1.5f;
                    X=2;
                    Y=1;
                    Z=1;
                }
            }
            else if(X==1 && Y==2 && Z==1)
            {
                if(BlockInfo.rotType==1)
                {
                    BlockInfo.orientation=0;
                    BlockInfo.y+=1.5f;
                    X=1;
                    Y=1;
                    Z=2;
                }
                if(BlockInfo.rotType==2)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.x-=1;
                    X=1;
                    Y=2;
                    Z=1;
                }
                if(BlockInfo.rotType==3)
                {
                    BlockInfo.orientation=0;
                    BlockInfo.y-=1.5f;
                    X=1;
                    Y=1;
                    Z=2;
                }
                if(BlockInfo.rotType==4)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.x+=1;
                    X=1;
                    Y=2;
                    Z=1;
                }
            }
            else if(X==2 && Y==1 && Z==1)
            {
                if(BlockInfo.rotType==1)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.y+=1;
                    X=2;
                    Y=1;
                    Z=1;
                }
                if(BlockInfo.rotType==2)
                {
                    BlockInfo.orientation=0;
                    BlockInfo.x-=1.5f;
                    X=1;
                    Y=1;
                    Z=2;
                }
                if(BlockInfo.rotType==3)
                {
                    BlockInfo.orientation=1;
                    BlockInfo.y-=1;
                    X=2;
                    Y=1;
                    Z=1;
                }
                if(BlockInfo.rotType==4)
                {
                    BlockInfo.orientation=0;
                    BlockInfo.x+=1.5f;
                    X=1;
                    Y=1;
                    Z=2;
                }
            }
        //    cout<<"X: "<<BlockInfo.x<<" Y: "<<BlockInfo.y<<endl;
            checkIfTheEffingBlockIsOnTheBoard();
        //    BlockInfo.orientation=Z-1;
        }
}

int checkIfTheEffingBlockIsOnTheBoard()
{
    int stat=0;
    if(BlockInfo.x==board.width-1 && BlockInfo.y==board.length-1)
    {
        cout<<"Congrats! You Pass the level"<<endl;
        increaseLevel();
        return stat;
    }
    if(BlockInfo.x<0 || BlockInfo.x > board.width-1 || BlockInfo.y < 0 || BlockInfo.y > board.length-1)
    {
        if(BlockInfo.x<-0.5 || BlockInfo.x > board.width+0.5-1 || BlockInfo.y < -0.5 || BlockInfo.y > board.length+0.5-1)
        {
            cout<<"Out of bounds!!!"<<endl;
            // engine->play2D("fall.mp3", true);
            //ewGame();
            //gameover();
            fell=1;
        }
    }
    else if(X==1 && Y==1 && Z==2)
    {
        if(board.boardMat[int(BlockInfo.x)][int(BlockInfo.y)]==0)
        {
            cout<<"You lose, Game Over X:"<<int(BlockInfo.x)<<" Y: "<<int(BlockInfo.y)<<endl;
            // engine->play2D("fall.mp3", true);
            //newGame();
                //gameover();
                fell=1;

        }
    }
    else if(X==1 && Y==2 && Z==1)
    {
        if(board.boardMat[int(BlockInfo.x)][int(BlockInfo.y-0.5)]==0 && board.boardMat[int(BlockInfo.x)][int(BlockInfo.y+0.5)]==0)
        {
            cout<<"You lose, Game Over X:"<<int(BlockInfo.x)<<" Y: "<<int(BlockInfo.y-0.5)<<endl;
            // engine->play2D("fall.mp3", true);
            // newGame();
            fell=1;

//                gameover();
        }
    }
    else if(X==2 && Y==1 && Z==1)
    {
        if(board.boardMat[int(BlockInfo.x-0.5)][int(BlockInfo.y)]==0 && board.boardMat[int(BlockInfo.x+0.5)][int(BlockInfo.y)]==0)
        {
            cout<<"You lose, Game Over X:"<<int(BlockInfo.x-0.5)<<" Y: "<<int(BlockInfo.y)<<endl;
            // engine->play2D("fall.mp3", true);
            // newGame();
            fell=1;

//                gameover();
        }
    }
    return stat;
}
void gameover()
{
    if(startFall==-1 && fell==1)
    {
        //cout<<"played"<<endl;
        engine->play2D("fall.wav");
        startFall=current_time;
    //    cout<<"First: "<<current_time<<" "<<startFall<<endl;
    }
    else if(current_time-startFall <= falltime)
    {
    //    cout<<current_time<<" "<<startFall<<endl;
        fallz-=0.007;
    }
    else
    {
    //    cout<<"Third: "<<current_time<<" "<<startFall<<endl;
        startFall=-1;
        fell=0;
        newGame();
    }
}
int checkPres(int x,int y)
{
    int stat=0;
    if(X==1 && Y==1 && Z==2)
    {
        if(int(BlockInfo.x)==x && int(BlockInfo.y)==y)
        {
            stat=1;
        }
    }
    else if(X==1 && Y==2 && Z==1)
    {
        if((int(BlockInfo.x)==x && int(BlockInfo.y-0.5)==y) || (int(BlockInfo.x)==x && int(BlockInfo.y+0.5)==y))
        {
            stat=1;
        }
    }
    else if(X==2 && Y==1 && Z==1)
    {
        if((int(BlockInfo.x-0.5)==x&&int(BlockInfo.y)==y) || (int(BlockInfo.x+0.5)==x && int(BlockInfo.y)==y))
        {
            stat=1;
        }
    }
    return stat;
}
void scaleMouse(double&,double&);

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(cv.type==5)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
        lastX = xpos;
        lastY = ypos;

        GLfloat sensitivity = 0.1;	// Change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw   += xoffset;
        pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.z = sin(glm::radians(pitch));
        front.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}
void camMov(GLFWwindow* window)
{
    double deltaT=current_time - last_update_time;
    GLfloat cameraSpeed = 5.0f * deltaT;
    if (glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void changeCam(int c)
{
    cv.type=c;
    if(c==1)
    {
        if(BlockInfo.rotType==1)
        {
            cv.eye=glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length)+0.5f,(1.25f)*(8.0/board.length)+0.5f);
            cv.target=glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y+2-board.length/2.0)+0.5f)*(8.0/board.length),(1.25f)*(8.0/board.length));
        }
        else if(BlockInfo.rotType==2)
        {
            cv.eye=glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width)-0.5f,((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length)-0.5f,(1.25f)*(8.0/board.length)+0.5f);
            cv.target=glm::vec3(((BlockInfo.x-1-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length)-1.0f,(1.25f)*(8.0/board.length));
        }
        else if(BlockInfo.rotType==3)
        {
            cv.eye=glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length)-0.5f,(1.25f)*(8.0/board.length)+0.5f);
            cv.target=glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y-2-board.length/2.0)+0.5f)*(8.0/board.length),(1.25f)*(8.0/board.length));
        }
        else
        {

            cv.eye=glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width)+0.5f,((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length),(1.25f)*(8.0/board.length)+0.5f);
            cv.target=glm::vec3(((BlockInfo.x+2-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length),(1.25f)*(8.0/board.length));
        }
        cv.up=glm::vec3(0,0,1);
    }
    else if(c==2)
    {
        cv.eye=glm::vec3(0,0,4);
        cv.target=glm::vec3(0,0,0);
        cv.up=glm::vec3(0,1,0);
    }
    else if(c==3)
    {
        orthpers=0;
        cv.eye=glm::vec3(-5,-5,3);
        cv.target=glm::vec3(0,0,0);
        cv.up=glm::vec3(0,0,1);
    }
    else if(c==4)
    {
        cv.eye=glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width)-1,((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length)-1,4);
        cv.target=glm::vec3(4,4,0);
        cv.up=glm::vec3(0,0,1);
    }
    else if(c==5)
    {
        /*r=5;
        scaleMouse(mouse_x,mouse_y);
        theta+=mouse_x;
        phi+=mouse_y;
        camx=r*sin(theta)*cos(phi);
        camy=r*sin(theta)*sin(phi);
        camz=r*cos(theta);*/
        //cv.eye=glm::vec3(camx,camy,camz);
        orthpers=1;
        cv.eye=cameraPos;
        cv.target=cameraFront+cameraPos;
        cv.up=cameraUp;

    }
    if(orthpers==0)
    {
        Matrices.projection = glm::ortho(-4.0f*zoom, 4.0f*zoom, -4.0f*zoom, 4.0f*zoom, 0.1f, 500.0f);
    }
    else
    {
        if(cv.type==2 || cv.type==1)
        {
            fov=90.0f;
        }
        else
        {
            fov=45.0f;
        }
        Matrices.projection = glm::perspective (fov, (GLfloat) WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f);
    }
}
void scaleMouse(double &x,double &y)
{
    x-=400;
    y+=400;
    x/=8000;
    y/=8000;
}
char title[100];
char time1[10];
char move1[10];
double addOne;
void draw (GLFWwindow* window)
{
    double deltaT=current_time - last_update_time;
  camMov(window);
  //glfwGetCursorPos(window, &mouse_x, &mouse_y);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  strcpy(title,"Time: ");
  int currt=int(current_time);
  stringstream ss (stringstream::in | stringstream::out);
  ss<<currt;
  strcat(title,ss.str().c_str());
  ss.str("");
  strcat(title," sec");
  strcat(title," Moves: ");
  ss<<Moves;
  strcat(title,ss.str().c_str());
  glfwSetWindowTitle(window,title);
  glUseProgram (programID);
  glUniform3f(lightPosID, 5*cos(camera_rotation_angle*M_PI/180.0f), 3, 5*sin(camera_rotation_angle*M_PI/180.0f));
  glUniform3f(lightColorID,  1.0f, 1.0f, 1.0f);
  glUniform3f(objectColorID, 1.0f, 0.5f, 0.31f);
  changeCam(cv.type);
  Matrices.view = glm::lookAt(cv.eye,cv.target,cv.up); // Fixed camera for 2D (ortho) in XY plane
 // cout<<fell<<endl;
  if(fell==1)
  {
     // cout<<"calling gameover"<<endl;
      gameover();
  }
  glm::mat4 VP = Matrices.projection * Matrices.view;
  if(startRotUP==1)
  {
      rotatecube();
  }
 // checkIfTheEffingBlockIsOnTheBoard();
  glm::mat4 MVP;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatecube;
  if(BlockInfo.orientation==0)
    translatecube = glm::translate (glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length),(1.25f)*(8.0/board.length)+fallz));
  else
    translatecube = glm::translate (glm::vec3(((BlockInfo.x-board.width/2.0)+0.5f)*(8.0/board.width),((BlockInfo.y-board.length/2.0)+0.5f)*(8.0/board.length),(0.75f)*(8.0/board.length)+fallz));
  glm::mat4 scalecube = glm::scale(glm::vec3(X*8.0/board.width,Y*8.0/board.length,Z*8.0/(board.length)));

  glm::mat4 rotHelp = glm::translate(BlockInfo.helprot);
  glm::mat4 rotatecube = glm::rotate((float)(BlockInfo.angle*M_PI/180.0f),BlockInfo.rotAxis); // rotate about vector (-1,1,1)
  rotatecube=inverse(rotHelp)*rotatecube*rotHelp;
  Matrices.model *= (translatecube * rotatecube * scalecube);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(Matrices.ModelID, 1, GL_FALSE, &Matrices.model[0][0]);
  Block.draw3DObject();

  for(int i=0;i<board.width;i++)
  {
      for(int j=0;j<board.length;j++)
      {
          if(board.boardMat[i][j]!=0)
          {
              if(board.boardMat[i][j]==1)
              {
                 glUniform3f(objectColorID,0, 0.5f,0);
              }
              if(board.boardMat[i][j]==2)
              {
                 glUniform3f(objectColorID,0, 0,0.5f);
                 if(checkPres(i,j)==1 && BlockInfo.orientation==0)
                 {
                     board.boardMat[i][j]=0;
                 }
              }
              if(board.boardMat[i][j]==3)
              {
                 glUniform3f(objectColorID,1.0f*sin(colorTheta*M_PI/180.f),0.25,0.5f*cos(colorTheta*M_PI/180.f));
                 if(checkPres(i,j)==1)
                 {
                     board.fillBridge();
                 }
              }
              if(board.boardMat[i][j]==4)
              {
                 glUniform3f(objectColorID,1.0f*blinkPar,1.0f*blinkPar,1.0f*blinkPar);
              }
              Matrices.model = glm::mat4(1.0f);
              translatecube = glm::translate (glm::vec3(((i-board.width/2.0)+0.5f)*(8.0/board.width),((j-board.length/2.0)+0.5f)*8.0/board.length,0));        // glTranslatef
              scalecube = glm::scale(glm::vec3(8.0/board.width,8.0/board.length,8.0/(board.length*2.0)));

              //rotatecube = glm::rotate((float)(cube_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
              Matrices.model *= (translatecube * scalecube);
              MVP = VP * Matrices.model;
              glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
              glUniformMatrix4fv(Matrices.ModelID, 1, GL_FALSE, &Matrices.model[0][0]);
              Block.draw3DObject();
          }
      }
  }
  // draw3DObject draws the VAO given to it using current MVP matrix
  // Increment angles
  colorTheta2 = colorTheta2 + deltaT*color_change_speed;
  colorTheta=(int(colorTheta2))%360;
  float increments = 1;

  if(SunRev==1)
    if(addOne >= 1) {
        camera_rotation_angle=(int(camera_rotation_angle)+1)%360;
        addOne = 0; // Simulating camera rotation
    }
    else
        addOne += deltaT*cam_rev_speed;
  //cube_rotation = cube_rotation + increments*cube_rot_dir*cube_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
            int hello;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{

	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
    Matrices.ModelID = glGetUniformLocation(programID,"model");
    lightPosID = glGetUniformLocation(programID,"lightPos");
    lightColorID = glGetUniformLocation(programID,"lightColor");
    objectColorID = glGetUniformLocation(programID,"objectColor");
    BlockInfo.x=0;
    BlockInfo.y=0;
    BlockInfo.angle=0;
    BlockInfo.orientation=0;
    BlockInfo.angularSpeed=180.0f;
    board.changeWidthLength(Level,Level);
    board.createBoard();
    BlockInfo.helprot=glm::vec3(0,4.0/board.length,8.0/board.width);
    BlockInfo.parity=-1;
    BlockInfo.rotAxis=glm::vec3(1,0,0);
    BlockInfo.rotType=1;
    BlockInfo.cubescale=glm::vec3(8.0/board.width,8.0/board.length,2*8.0/(board.length));
    X=1;
    Y=1;
    Z=2;
    Block.createCube();
    changeCam(3);
    engine = createIrrKlangDevice();
	reshapeWindow (window, width, height);

    cam_rev_speed = 10;
    color_change_speed = 50;
    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = WIDTH;
	int height = HEIGHT;

    GLFWwindow* window = initGLFW(width, height);
    srand (time(NULL));
    initGL (window, width, height);
    if (!engine)
    {
        printf("Could not startup engine\n");
        return 0; // error starting up the engine
    }

    last_update_time = glfwGetTime();
    blink=last_update_time;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        // OpenGL Draw commands
        current_time = glfwGetTime();
        if(current_time-blink>=0.5)
        {
            blinkPar=(blinkPar+1)%2;
            blink=current_time;
        }
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
        last_update_time=current_time;
        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
