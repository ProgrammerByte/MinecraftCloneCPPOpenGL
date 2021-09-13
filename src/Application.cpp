

//Improvements to make - Clean up code (move stuff to separate files and classes), move world gen stuff to a compute shader as this would be much faster, make the world infinite (this would be quite difficult).


//Range of perlin noise function should be [-sqrt(N)/4, sqrt(N)/4] where N is the number of dimensions



/*
keyBinds:
1 = grass
2 = dirt
3 = stone
4 = cobblestone
5 = wood
6 = log
7 = brick
8 = gravel
9 = leaves
0 = sand
*/


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include "PerlinNoise2D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

glm::mat4 projectionMatrix;
int projectionMatrixLocation;
glm::mat4 viewMatrix;
int viewMatrixLocation;

glm::vec3 position;
int positionLocation;
glm::vec3 direction = glm::vec3(0, -1, 0); //will be used for glm::lookat and raycasting TODO - MAKE UNIT
glm::dvec3 invDirection = glm::vec3(1000000, -1, 1000000); //1 / direction

glm::vec3 forwardVec;
glm::vec3 rightVec;

//bool isDark = false;
//int isDarkLocation;
int interpolationLocation;

GLFWwindow* window;

double lastMouseXPos = 0;
double lastMouseYPos = 0;
double camXRot = 0; //rotation about y axis - looking left and right
double camYRot = 0; //rotation about x axis - looking up and down
float sensitivity = 0.001;
float speed = 15;
float fov = 110;
bool mouseMoved = false;

bool justLeftClicked = false;
bool justRightClicked = false;
bool justNPressed = false;

double lastTime; //used to determine fps
int frames;

float dt;
float prevTime; //used for movement

int currBlock = 14; //current block selected

float startTime;
double dayNightInterpolation = 0;
float invDayLength = (float)1 / 180; //1 / number of seconds for a full day cycle - reciprocal as used for division


int targetBlock[3] = {-1, -1, -1};
int targetSide = -1; //which side of the block is targeted

float skyDayColour[3] = {0.529, 0.808, 0.922};
float skyNightColour[3] = {0, 0, 0};

float cubeVertices[6][6][3] = {
    {{0, 0, 0}, //x and y components can also be used for texture coordinates
    {0, 1, 0},
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {1, 0, 0}}, //Front Face

    {{0, 0, 1},
    {0, 1, 1},
    {0, 0, 0},
    {0, 1, 1},
    {0, 1, 0},
    {0, 0, 0}}, //Left Face

    {{1, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1},
    {0, 1, 1},
    {0, 0, 1}}, //Back Face

    {{1, 0, 0},
    {1, 1, 0},
    {1, 0, 1},
    {1, 1, 0},
    {1, 1, 1},
    {1, 0, 1}}, //Right Face

    {{0, 1, 0},
    {0, 1, 1},
    {1, 1, 0},
    {0, 1, 1},
    {1, 1, 1},
    {1, 1, 0}}, //Top Face

    {{0, 0, 1},
    {0, 0, 0},
    {1, 0, 1},
    {0, 0, 0},
    {1, 0, 0},
    {1, 0, 1}} //Bottom Face
};

int cubeNormals[6][3] = {
    {0, 0, -1}, //Front Face
    {-1, 0, 0}, //Left Face
    {0, 0, 1}, //Back Face
    {1, 0, 0}, //Right Face
    {0, 1, 0}, //Top Face
    {0, -1, 0} //Bottom Face
};


const float textureWidth = (float)1 / 16; //16 x 16 textures - could be changed

//Where textures are on the textureAtlas (how many textures are to the left and below it)
const int blockCount = 14;
float textureCoords[blockCount][6][2] = { //blockID - 1, blockFace, textureCoords - Needs to be multiplied by textureWidth
    //texture 1 - grass block
    {{3, 15}, //Front Face
    {3, 15}, //Left Face
    {3, 15}, //Back Face
    {3, 15}, //Right Face
    {0, 15}, //Top Face
    {2, 15}}, //Bottom Face

    //texture 2 - dirt block
    {{2, 15}, //Front Face
    {2, 15}, //Left Face
    {2, 15}, //Back Face
    {2, 15}, //Right Face
    {2, 15}, //Top Face
    {2, 15}}, //Bottom Face

    //texture 3 - stone
    {{1, 15},
    {1, 15},
    {1, 15},
    {1, 15},
    {1, 15},
    {1, 15}},

    //texture 4 - cloud
    {{2, 11},
    {2, 11},
    {2, 11},
    {2, 11},
    {2, 11},
    {2, 11}},

    //texture 5 - sand
    {{0, 4},
    {0, 4},
    {0, 4},
    {0, 4},
    {0, 4},
    {0, 4}},

    //texture 6 - sandstone
    {{0, 3},
    {0, 3},
    {0, 3},
    {0, 3},
    {0, 3},
    {0, 3}},

    //texture 7 - snowy grass
    {{4, 11},
    {4, 11},
    {4, 11},
    {4, 11},
    {2, 11},
    {2, 15}},

    //texture 8 - gravel
    {{3, 14},
    {3, 14},
    {3, 14},
    {3, 14},
    {3, 14},
    {3, 14}},

    //texture 9 - cactus
    {{6, 11},
    {6, 11},
    {6, 11},
    {6, 11},
    {5, 11},
    {5, 11}},

    //texture 10 - log
    {{4, 14},
    {4, 14},
    {4, 14},
    {4, 14},
    {5, 14},
    {5, 14}},

    //texture 11 - leaves
    {{5, 12},
    {5, 12},
    {5, 12},
    {5, 12},
    {5, 12},
    {5, 12}},

    //texture 12 - cobblestone
    {{0, 14},
    {0, 14},
    {0, 14},
    {0, 14},
    {0, 14},
    {0, 14}},

    //texture 13 - wood planks
    {{7, 3},
    {7, 3},
    {7, 3},
    {7, 3},
    {7, 3},
    {7, 3}},

    //texture 14 - brick
    {{7, 15},
    {7, 15},
    {7, 15},
    {7, 15},
    {7, 15},
    {7, 15}},
};

float vertexTextureCoords[6][2] = { //Needs to be multiplied by textureWidth
    {1, 0}, //x and y components can also be used for texture coordinates
    {1, 1},
    {0, 0},
    {1, 1},
    {0, 1},
    {0, 0}
};


//THE FOLLOWING IS FOR THE GAME ITSELF
const int chunkSize = 48; //chunks are of size chunkSize * 128 * chunkSize; - 48
const int chunkCount = 5; //width and length of world
const int worldSize = chunkSize * chunkCount; //worldSize * worldSize blocks
const int worldHeight = 256; //32
int world[worldSize][worldHeight][worldSize]; //value corresponds with what block is there
vector<float> chunks[chunkCount][chunkCount]; //stores mesh data to pass onto shaders

int seed = time(0);
PerlinNoise2D worldGen = PerlinNoise2D(seed, 5, 0.015625);
const double hillAmplitude = 50;
const int hillDisplacement = 100; //height of hill = (hillAmplitude * perlin2D(x, z)) + hillDisplacement
const int hillMax = (int)(hillAmplitude * (double)sqrt(2) / 2) + hillDisplacement;
const int cloudHeightMin = hillMax + 30; //lowest height clouds can spawn
const int cloudHeightMax = cloudHeightMin + 20; //highest height clouds can spawn

PerlinNoise2D biomeGen = PerlinNoise2D((seed % 10000) * (seed % 10000), 2, 0.015625 * 0.4); //will be used to determine biomes and stone types underground

void generateChunk(int ptr1, int ptr2) { //generates the vertex data for the given chunk
    if (ptr1 >= 0 && ptr2 >= 0 && ptr1 < chunkCount && ptr2 < chunkCount) {
        chunks[ptr1][ptr2] = {};
        int currI;
        int currJ;
        int currK;
        int blockID;
        for (int i = ptr1 * chunkSize; i < (ptr1 + 1) * chunkSize; i++) { //iterate through each block and push back triangles which can be seen
            for (int j = 0; j < worldHeight; j++) {
                for (int k = ptr2 * chunkSize; k < (ptr2 + 1) * chunkSize; k++) {
                    if (world[i][j][k] != 0) {
                        blockID = world[i][j][k] - 1;
                        for (int a = 0; a < 6; a++) { //check all 6 sides of the block and only render what's needed
                            //perform check here
                            currI = i + cubeNormals[a][0];
                            currJ = j + cubeNormals[a][1];
                            currK = k + cubeNormals[a][2];
                            if (currI < 0 || currI == worldSize || currJ < 0 || currJ == worldHeight || currK < 0 || currK == worldSize || world[currI][currJ][currK] == 0) {
                                for (int b = 0; b < 6; b++) { //add triangles to vector
                                    chunks[ptr1][ptr2].push_back(cubeVertices[a][b][0] + i);
                                    chunks[ptr1][ptr2].push_back(cubeVertices[a][b][1] + j);
                                    chunks[ptr1][ptr2].push_back(cubeVertices[a][b][2] + k);
                                    //TODO - ADD TEXTURE DATA HERE!!!
                                    chunks[ptr1][ptr2].push_back(textureCoords[blockID][a][0] + vertexTextureCoords[b][0]);
                                    chunks[ptr1][ptr2].push_back(textureCoords[blockID][a][1] + vertexTextureCoords[b][1]);
                                }
                            }
                        }
                    }
                }
            }
        }
        chunks[ptr1][ptr2].push_back(0); //I have no idea why this is needed but ok
        //cout << chunks[ptr1][ptr2].size() << endl;
        //for (int i = 0; i < chunks[ptr1][ptr2].size(); i++) {
        //    cout << chunks[ptr1][ptr2][i] << endl;
        //}
    }
}

void rayCasting() {
    targetBlock[0] = -1; //no valid block in range
    targetSide = -1;

    bool xPositive = true;
    bool yPositive = true;
    bool zPositive = true;
    if (invDirection.x < 0) {
        xPositive = false;
    }
    if (invDirection.y < 0) {
        yPositive = false;
    }
    if (invDirection.z < 0) {
        zPositive = false;
    }
    glm::dvec3 currPos = position;
    double totalDist = 0;
    double minDist;
    double currDist;
    int shortest = -1; //0 = x, 1 = y, 2 = z
    //int currX = (int)position.x;
    //int currY = (int)position.y;
    //int currZ = (int)position.z;
    int currX;
    int currY;
    int currZ;
    while (totalDist < 5) {
        currX = (int)currPos.x;
        currY = (int)currPos.y;
        currZ = (int)currPos.z;
        if (currPos.x < 0 || currX >= worldSize || currPos.y < 0 || currY > worldHeight || currPos.z < 0 || currZ >= worldSize) {
            break;
        }
        if (world[currX][currY][currZ] != 0) { //Not air - this could be replaced with an attribute if I change blocks to objects
            //cout << "PISerasijguearbguiaerbnguiwaqebrgbiuewrabngbiueS" << endl;
            targetBlock[0] = currX;
            targetBlock[1] = currY;
            targetBlock[2] = currZ;
            switch (shortest) {
                case 0:
                    if (xPositive == true) { //left block face
                        targetSide = 1;
                    }
                    else { //right block face
                        targetSide = 3;
                    }
                    break;

                case 1:
                    if (yPositive == true) { //bottom block face
                        targetSide = 5;
                    }
                    else { //top block face
                        targetSide = 4;
                    }
                    break;

                case 2:
                    if (zPositive == true) { //front block face
                        targetSide = 0;
                    }
                    else { //back block face
                        targetSide = 2;
                    }
                    break;

                default:
                    break;
            }
            break;
        }

        //minDist = 2; //shortest distance to get to next block along direction
        minDist = (xPositive - (currPos.x - currX)) * invDirection.x;
        shortest = 0;
        currDist = (yPositive - (currPos.y - currY)) * invDirection.y;
        if (currDist < minDist) {
            minDist = currDist;
            shortest = 1;
        }
        currDist = (zPositive - (currPos.z - currZ)) * invDirection.z;
        if (currDist < minDist) {
            minDist = currDist;
            shortest = 2;
        }
        //cout << minDist << endl;
        minDist += 0.00001;
        totalDist += minDist;
        currPos += (float)minDist * direction;
    }
}

int createShaderProgram(unsigned int &shaderProgram, const char* vertexPath, const char* fragmentPath) { //returns -1 if an error has occured, 0 otherwise
    const char* vertexShaderSource;
    const char* fragmentShaderSource;
    string vertexCode;
    string fragmentCode;
    ifstream vertexFile;
    ifstream fragmentFile;

    //read shader code from memory
    try {
        string current;
        vertexFile.open(vertexPath);
        while (getline(vertexFile, current)) {
            vertexCode += current + "\n";
        }
        vertexShaderSource = vertexCode.c_str();
        vertexFile.close();

        fragmentFile.open(fragmentPath);
        while (getline(fragmentFile, current)) {
            fragmentCode += current + "\n";
        }
        fragmentShaderSource = fragmentCode.c_str();
        fragmentFile.close();
    }
    catch (ifstream::failure e) {
        cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
        return -1;
    }

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    //THE FOLLOWING DETERMINES IF THE VERTEX SHADER WAS COMPILED SUCCESSFULLY
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return -1;
    }


    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    //THE FOLLOWING DETERMINES IF THE FRAGMENT SHADER WAS COMPILED SUCCESSFULLY
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return -1;
    }

    //THE FOLLOWING CREATES AND COMPILES THE SHADER PROGRAM
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader); //no longer needed as they have been bound to the program, and the program has been bound to OpenGL
    glDeleteShader(fragmentShader);

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    projectionMatrix = glm::perspective(glm::radians(fov), (float)width / height, 0.1f, 1000.0f);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glViewport(0, 0, width, height);
}

void input() {
    if (glfwGetKey(window, GLFW_KEY_W)) {
        position += forwardVec * (speed * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        position -= forwardVec * (speed * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        position -= rightVec * (speed * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        position += rightVec * (speed * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        position.y += speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        position.y -= speed * dt;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        //break block
        if (justLeftClicked == false) {
            if (targetBlock[0] != -1) {
                world[targetBlock[0]][targetBlock[1]][targetBlock[2]] = 0;
                int ptr1 = targetBlock[0] / chunkSize;
                int ptr2 = targetBlock[2] / chunkSize;
                generateChunk(ptr1, ptr2); //repeat for surrounding chunks
                generateChunk(ptr1 + 1, ptr2); //repeat for surrounding chunks
                generateChunk(ptr1 - 1, ptr2); //repeat for surrounding chunks
                generateChunk(ptr1, ptr2 + 1); //repeat for surrounding chunks
                generateChunk(ptr1, ptr2 - 1); //repeat for surrounding chunks
            }
            justLeftClicked = true;
        }
    }
    else {
        justLeftClicked = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        //place block
        if (justRightClicked == false) {
            if (targetSide != -1) {
                int x = targetBlock[0] + cubeNormals[targetSide][0];
                int y = targetBlock[1] + cubeNormals[targetSide][1];
                int z = targetBlock[2] + cubeNormals[targetSide][2];
                if (x >= 0 && x < worldSize && y >= 0 && y < worldHeight && z >= 0 && z < worldSize) {
                    world[x][y][z] = currBlock;
                    generateChunk(x / chunkSize, z / chunkSize);
                }
            }
            justRightClicked = true;
        }
    }
    else {
        justRightClicked = false;
    }
    glUniform3fv(positionLocation, 1, glm::value_ptr(position));
    /*if (glfwGetKey(window, GLFW_KEY_N)) {
        if (justNPressed == false) {
            isDark = !isDark;
            if (isDark == false) {
                glClearColor(skyDayColour[0], skyDayColour[1], skyDayColour[2], 1);
            }
            else {
                glClearColor(skyNightColour[0], skyNightColour[1], skyNightColour[2], 1);
            }
            glUniform1i(isDarkLocation, isDark);
            justNPressed = true;
        }
    }
    else {
        justNPressed = false;
    }*/

    if (glfwGetKey(window, GLFW_KEY_0)) { ///block picking - refer to top of code for more info
        currBlock = 5;
    }
    if (glfwGetKey(window, GLFW_KEY_1)) {
        currBlock = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_2)) {
        currBlock = 2;
    }
    if (glfwGetKey(window, GLFW_KEY_3)) {
        currBlock = 3;
    }
    if (glfwGetKey(window, GLFW_KEY_4)) {
        currBlock = 12;
    }
    if (glfwGetKey(window, GLFW_KEY_5)) {
        currBlock = 13;
    }
    if (glfwGetKey(window, GLFW_KEY_6)) {
        currBlock = 10;
    }
    if (glfwGetKey(window, GLFW_KEY_7)) {
        currBlock = 14;
    }
    if (glfwGetKey(window, GLFW_KEY_8)) {
        currBlock = 8;
    }
    if (glfwGetKey(window, GLFW_KEY_9)) {
        currBlock = 11;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouseMove(GLFWwindow* window, double xPos, double yPos) {
    if (!mouseMoved) {
        mouseMoved = true;
    }
    else {
        camXRot += sensitivity * (xPos - lastMouseXPos);
        camYRot += sensitivity * (yPos - lastMouseYPos);
        if (camYRot > 1.6) {
            camYRot = 1.6;
        }
        if (camYRot < -1.6) {
            camYRot = -1.6;
        }
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::rotate(viewMatrix, (float)camYRot, glm::vec3(1, 0, 0)); //REPLACE WITH GLM::LOOKAT SO I CAN USE THE LOOKAT VECTOR FOR RAYCASTING TO BREAK AND PLACE BLOCKS
        viewMatrix = glm::rotate(viewMatrix, (float)camXRot, glm::vec3(0, 1, 0)); //rotate about x axis, and then y axis
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        rightVec = glm::vec3(cos(camXRot), 0, sin(camXRot));
        forwardVec = glm::vec3(rightVec.z, 0, -rightVec.x);

        direction = forwardVec * (float)cos(camYRot);
        direction.y = (float)sin(-camYRot);
        invDirection = glm::vec3(1, 1, 1) / direction;
    }
    lastMouseXPos = xPos;
    lastMouseYPos = yPos;
}

int main() {
    
    //cout << (int)(-0.5) << endl;

    //THE FOLLOWING CREATES AND VERIFIES A WINDOW AS WELL AS INITIALISE GLFW AND GLEW

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 800, "Minecraft Clone", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewInit();


    glClearColor(skyDayColour[0], skyDayColour[1], skyDayColour[2], 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDepthFunc(GL_ALWAYS);


    unsigned int shaderProgram;
    createShaderProgram(shaderProgram, "src/VertexShader.vert", "src/FragmentShader.frag");

    //float triangleVertices[] = {
    //    -0.9f, -0.75f, 0,
    //    0, 0.75f, 0,
    //    0.9f, -0.75f, 0
    //};

    /*float quadVertices[] = {
        -1.0f, -1.0f, 0, //bl
        -1.0f, 1.0f, 0, //tl
        1.0f, 1.0f, 0, //tr
        1.0f, -1.0f, 0 //br
    };
    int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };*/



    //THE FOLLOWING INITIALISES THE VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    //THE FOLLOWING INITIALISES THE VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    //THE FOLLOWING INITIALISES THE EBO
    //unsigned int EBO;
    //glGenBuffers(1, &EBO);

    //THE FOLLOWING LOADS THE VERTEX DATA INTO THE VAO, AND VBO, HENCE THE PROGRAM - INITIALISATION CODE - ONLY NEEDS TO BE RUN ONCE UNLESS DATA CHANGES

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    //glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, chunks[0][0].size() * sizeof(GL_FLOAT), chunks[0][0].data(), GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); //vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); //texture position
    glEnableVertexAttribArray(1);


    glUseProgram(shaderProgram);


    //SET UNIFORM VARIABLES
    //int redLocation = glGetUniformLocation(shaderProgram, "red");
    //glUniform2f(redLocation, -0.5, -0.5);
    //int greenLocation = glGetUniformLocation(shaderProgram, "green");
    //glUniform2f(greenLocation, 0, 0.3);
    //int blueLocation = glGetUniformLocation(shaderProgram, "blue");
    //glUniform2f(blueLocation, 0.5, -0.5);

    //Set up texture Atlas
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    //glTexParameter
    for (int i = 0; i < blockCount; i++) {
        for (int j = 0; j < 6; j++) {
            textureCoords[i][j][0] *= textureWidth;
            textureCoords[i][j][1] *= textureWidth;
        }
    }
    for (int i = 0; i < 6; i++) {
        vertexTextureCoords[i][0] *= textureWidth;
        vertexTextureCoords[i][1] *= textureWidth;
    }
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("texAtlas2.png", &width, &height, &nrChannels, 0);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); //THIS NEEDS TO BE CHECKED
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);




    //the following generates the world
    int currHeight;
    //bool topBlock;
    int distTop; //distance from top block
    double currBiome;
    double currBiome3D; //3D biome
    for (int i = 0; i < worldSize; i++) {
        for (int k = 0; k < worldSize; k++) {
            currHeight = (int)(hillAmplitude * worldGen.getHeight(i, k, 5, 2, 0.3)) + hillDisplacement;
            currBiome = biomeGen.getHeight(i, k, 5, 2, 0.3);
            //topBlock = true;
            distTop = 0;
            for (int j = worldHeight - 1; j >= 0; j--) {
                if (j >= cloudHeightMin) {
                    if (j <= cloudHeightMax) {
                        //if (worldGen.iterativePerlin3D(i, j, k, 5, 2, 0.3, 1) > 0.01) {
                        //    world[i][j][k] = 4;
                       // }
                    }
                }
                else if (j > currHeight || worldGen.iterativePerlin3D(i, j, k, 5, 2, 0.3) < -0.05) {
                    world[i][j][k] = 0;
                }
                else if (j <= currHeight) {
                    //if (j == 0) {
                    //    world[i][j][k] = 3;
                    //}
                    if (distTop == 0) {
                        if (currBiome < -0.2) { //desert
                            world[i][j][k] = 5;
                        }
                        else if (currBiome > 0.2) { //snow
                            world[i][j][k] = 7;
                        }
                        else { //plains
                            world[i][j][k] = 1;
                        }
                        distTop += 1;
                    }
                    else if (distTop > 4) {
                        currBiome3D = biomeGen.iterativePerlin3D(i, j, k, 5, 2, 0.3, 10); //max = sqrt(3) / 2
                        if (currBiome3D > 0.1) { //gravel
                            world[i][j][k] = 8;
                        }
                        else if (currBiome3D < -0.1) { //dirt
                            world[i][j][k] = 2;
                        }
                        else { //regular stone
                            world[i][j][k] = 3;
                        }
                    }
                    else {
                        if (currBiome < -0.2) { //desert
                            world[i][j][k] = 6;
                        }
                        else { //snow or plains
                            world[i][j][k] = 2;
                        }
                    }
                }
                if (distTop != 0) {
                    distTop += 1;
                }
            }
        }
    }

    //add extra stuff (i.e. trees, cacti, bigger trees, and ores)
    int treeTop;
    srand(seed);
    for (int i = 0; i < worldSize; i++) {
        for (int k = 0; k < worldSize; k++) {
            for (int j = worldHeight - 1; j >= 0; j--) {
                if (world[i][j][k] == 5) { //have a chance to spawn a cactus
                    if (((double)rand() / RAND_MAX) < 0.01) { //chance to spawn cactus
                        for (int x = j + 1; x <= j + 1 + (rand() % 3); x++) { //Assumed to be below height limit - potentially add exception handling, 1-3 blocks high
                            world[i][x][k] = 9; //cactus
                        }
                    }
                }
                else if ((world[i][j][k] == 1 && ((double)rand() / RAND_MAX) < 0.01) || (world[i][j][k] == 7 && ((double)rand() / RAND_MAX) < 0.002)) {
                    treeTop = 3 + (rand() % 3); //3-5 blocks high
                    if (world[i][j][k] == 7) { //9-15 blocks high
                        treeTop *= 3;
                    }
                    treeTop += j;
                    for (int x = j + 1; x <= treeTop; x++) { //3-5 blocks high
                        world[i][x][k] = 10; //oak log
                    }
                    for (int b = treeTop + 2; b >= treeTop + 1; b--) {
                        for (int a = i - 1; a <= i + 1; a++) {
                            for (int c = k - 1; c <= k + 1; c++) {
                                if (a >= 0 && a < worldSize && c >= 0 && c < worldSize && world[a][b][c] == 0 && ((double)rand() / RAND_MAX) < 0.9) { //chance to spawn a leaf
                                    world[a][b][c] = 11;
                                }
                            }
                        }
                    }
                    for (int b = treeTop; b >= treeTop - 1; b--) {
                        for (int a = i - 2; a <= i + 2; a++) {
                            for (int c = k - 2; c <= k + 2; c++) {
                                if (a >= 0 && a < worldSize && c >= 0 && c < worldSize && world[a][b][c] == 0 && ((double)rand() / RAND_MAX) < 0.9) { //chance to spawn a leaf
                                    world[a][b][c] = 11;
                                }
                            }
                        }
                    }
                    if (world[i][j][k] == 7) {
                        for (int b = treeTop - 2; b >= treeTop - 5; b--) {
                            for (int a = i - 3; a <= i + 3; a++) {
                                for (int c = k - 3; c <= k + 3; c++) {
                                    if (a >= 0 && a < worldSize && c >= 0 && c < worldSize && world[a][b][c] == 0 && ((double)rand() / RAND_MAX) < 0.9) { //chance to spawn a leaf
                                        world[a][b][c] = 11;
                                    }
                                }
                            }
                        }
                    }
                }
                /*else if (world[i][j][k] == 7) {
                    if (((double)rand() / RAND_MAX) < 0.01) { //chance to spawn tree
                        for (int x = j + 1; x <= j + 6 + (rand() % 3); x++) { //6-8 blocks high
                            world[i][x][k] = 10; //ADD THIS!!! (spruce log)
                        }
                    }
                }*/
            }
        }
    }

    //intialise chunks
    for (int i = 0; i < chunkCount; i++) {
        for (int j = 0; j < chunkCount; j++) {
            generateChunk(i, j);
        }
    }

    //cout << chunks[0][0] << endl;

    startTime = glfwGetTime();



    
    //projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    forwardVec = glm::vec3(0, 0, -1);
    rightVec = glm::vec3(1, 0, 0);

    projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection");
    projectionMatrix = glm::perspective(glm::radians(fov), 1.0f, 0.1f, 1000.0f); //fov, aspect ratio, zNear, zFar
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    viewMatrixLocation = glGetUniformLocation(shaderProgram, "view");
    viewMatrix = glm::mat4(1.0f);
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    positionLocation = glGetUniformLocation(shaderProgram, "camPos");
    position = glm::vec3(worldSize / 2, worldHeight / 4, worldSize / 2);
    //position = glm::vec3(worldSize / 2, worldHeight / 4, worldSize / 2);
    glUniform3fv(positionLocation, 1, glm::value_ptr(position));

    //isDarkLocation = glGetUniformLocation(shaderProgram, "isDark");
    //glUniform1i(isDarkLocation, isDark);

    interpolationLocation = glGetUniformLocation(shaderProgram, "interpolation");
    glUniform1f(interpolationLocation, dayNightInterpolation);

    //RENDER LOOP
   // float x = 0;
   // float current;
    //cout << chunks[0][0].size();
    lastTime = glfwGetTime();
    prevTime = glfwGetTime();
    frames = 0;
    while (!glfwWindowShouldClose(window)) {
        dt = glfwGetTime() - prevTime;
        prevTime = glfwGetTime();

        rayCasting();
        input();

        dayNightInterpolation = 0.5 * (sin((2 * 3.1415926) * invDayLength * (glfwGetTime() - startTime)) + 1);
        glClearColor(dayNightInterpolation * skyDayColour[0], dayNightInterpolation * skyDayColour[1], dayNightInterpolation * skyDayColour[2], 0);
        glUniform1f(interpolationLocation, dayNightInterpolation);

        glClear(GL_COLOR_BUFFER_BIT); //CLEAR - READY TO START RENDERING
        glClear(GL_DEPTH_BUFFER_BIT);

        //current = sin(x);
        //glUniform2f(redLocation, -0.5 + current, -0.5 + current);
        //glUniform2f(greenLocation, 0, 0.3 - 1.4 * current);
        //glUniform2f(blueLocation, 0.5 - current, -0.5 + current);

        //glUniform2f(redLocation, 0, current);
        //glUniform2f(greenLocation, 0, current);
        //glUniform2f(blueLocation, 0, current);

        //glUniform2f()

        //RENDERING
        for (int i = 0; i < chunkCount; i++) {
            for (int j = 0; j < chunkCount; j++) {
                //generateChunk(i, j);
                //glBufferData(GL_ARRAY_BUFFER, sizeof(chunks[i][j]), &chunks[i][j], GL_STATIC_DRAW);
                //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //3 values per point
                //glDrawArrays(GL_TRIANGLES, 0, sizeof(chunks[i][j]) / 3);
                glBufferData(GL_ARRAY_BUFFER, chunks[i][j].size() * sizeof(GL_FLOAT), chunks[i][j].data(), GL_STATIC_DRAW);
                glDrawArrays(GL_TRIANGLES, 0, chunks[i][j].size() / 5);
                //cout << chunks[i][j].size() << endl;
            }
        }
        
        //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        //glDrawElements(GL_TRIANGLES, chunks[0][0].size() / 3, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //glDrawArrays(GL_TRIANGLES, 0, chunks[0][0].size() / 3);
        if (glfwGetTime() - lastTime >= 1) {
            lastTime = glfwGetTime();
            cout << frames << " FPS" << endl;
            frames = 0;
        }
        frames += 1;

        //cout << targetBlock[0] << endl;
        //END OF RENDER LOOP
        glfwSwapBuffers(window);
        glfwPollEvents();
        //x += 0.01;
    }

    //EXIT
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}