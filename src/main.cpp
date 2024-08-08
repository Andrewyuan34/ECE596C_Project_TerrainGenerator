#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include <fstream>
#include "shader.hpp"
#include "PerlinNoise.cpp"
#include "camera.hpp"
#include "command_line_parser.hpp"
#include "lighting.hpp"
#include "TerrainGenerate.hpp"

const int WIDTH = 1024; 

GLuint TerrainShaderProgram;
GLuint CubeShaderProgram;
GLuint texture1, texture2;

static auto terrain = std::make_unique<Terrain>(); 
static auto lighting = std::make_unique<Lighting>(); 
Camera camera({0, 0, WIDTH});
float angle = 0.0f;

std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

void updateFPS();

void init(double frequency, int octave, double amplitude, double persistence, double lacunarity, int width) {
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << '\n';
        return;
    }
    
    // Load the shader program
    TerrainShaderProgram = createShaderProgramFromFile("shader/sand_vertexShader.glsl", "shader/sand_fragmentShader.glsl");
    CubeShaderProgram = createShaderProgramFromFile("shader/cube_vertex_shader.glsl", "shader/cube_fragment_shader.glsl");
    if (TerrainShaderProgram == 0 || CubeShaderProgram == 0) {
        std::cerr << "Failed to create shader program" << '\n';
        return;
    }

    // Load the textures
    texture1 = loadTexture("texture/grass.bmp");
    texture2 = loadTexture("texture/sand.bmp");

    // Generate vertices, indices and normals for the terrain
    terrain->generateBaseTerrain(frequency, octave, amplitude, persistence, lacunarity);
    terrain->generateWater();
    terrain->generateTerrainNormals();
    terrain->initTerrain(TerrainShaderProgram);

    // Initialize the lighting cube
    lighting->initCube(width / 1024);

    // Set values below at the beginning instead of in the display function
    {
        //This part is for the terrain shader program

        // Pass the 'const' ambientlight, water level and height difference limits parameters to the shader
        glUseProgram(TerrainShaderProgram);
        GLint ambientLightLoc = glGetUniformLocation(TerrainShaderProgram, "ambientLight");
        glUniform3f(ambientLightLoc, 0.3f, 0.3f, 0.3f); // Set the ambient light color
        GLint waterLevelLoc = glGetUniformLocation(TerrainShaderProgram, "waterLevel");
        glUniform1f(waterLevelLoc, terrain->getWaterLevel()); // Set the water level
        GLint HeightDif_lowLoc = glGetUniformLocation(TerrainShaderProgram, "HeightDif_low");
        glUniform1f(HeightDif_lowLoc, terrain->getHeightDif_low()); 
        GLint HeightDif_highLoc = glGetUniformLocation(TerrainShaderProgram, "HeightDif_high");
        glUniform1f(HeightDif_highLoc, terrain->getHeightDif_high());
        GLint waterdepthMaxLoc = glGetUniformLocation(TerrainShaderProgram, "waterDepthMax");
        glUniform1f(waterdepthMaxLoc, terrain->getWaterdepthMax()); 

        // Pass the textures to the shader program
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(glGetUniformLocation(TerrainShaderProgram, "texture1"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(glGetUniformLocation(TerrainShaderProgram, "texture2"), 1);
    }

    {
        //This part is for the cube shader program

        // Pass the light color to the cube shader program
        glUseProgram(CubeShaderProgram);
        GLint lightColorLoc = glGetUniformLocation(CubeShaderProgram, "lightColor");
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // Change the light color to white
    }

    // Set the OpenGL state
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize the lastTime variable
    lastTime = std::chrono::high_resolution_clock::now();
}

void display() {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Set the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 10.0f, 10000.0f);
    GLdouble projectionMatrixD[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrixD);
    GLfloat projectionMatrix[16];
    convertMatrix(projectionMatrixD, projectionMatrix);

    // Set the view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.getCameraPos().x, camera.getCameraPos().y, camera.getCameraPos().z,  
                camera.getCameraPos().x + camera.getCameraFront().x, camera.getCameraPos().y + camera.getCameraFront().y, camera.getCameraPos().z + camera.getCameraFront().z,  // 目标位置 (x, y, z)
              0.0f, 1.0f, 0.0f); 

    GLdouble viewMatrixD[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewMatrixD);
    GLfloat viewMatrix[16];
    convertMatrix(viewMatrixD, viewMatrix);

    // Switch to the terrain shader program
    useShaderProgram(TerrainShaderProgram);
    GLint projLoc = glGetUniformLocation(TerrainShaderProgram, "projectionMatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix);
    GLint viewLoc = glGetUniformLocation(TerrainShaderProgram, "viewMatrix");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);

    // Set the diffuse light related properties
    GLint lightPosLoc = glGetUniformLocation(TerrainShaderProgram, "lightPos");
    glUniform3f(lightPosLoc, lighting->getmodelMatrix(12), lighting->getmodelMatrix(13), lighting->getmodelMatrix(14));


    GLint useWaterTextureLoc = glGetUniformLocation(TerrainShaderProgram, "useWaterTexture");
    glUniform1i(useWaterTextureLoc, GL_FALSE); // Forbid using water texture

    // Change the polygon mode based on the showWireframe flag
    if (camera.getShowWireframe()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Draw the terrain
    GL_CHECK(glBindVertexArray(terrain->getVAO()));
    GL_CHECK(glDrawElements(GL_TRIANGLES, (terrain->getHeight() / terrain->getStep() - 1) * (terrain->getWidth() / terrain->getStep() - 1) * 6, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    // Draw the water
    glUniform1i(useWaterTextureLoc, GL_TRUE); // Enable drawing water

    GL_CHECK(glBindVertexArray(terrain->getVAO()));
    GL_CHECK(glDrawElements(GL_TRIANGLES, (terrain->getHeight() / terrain->getStep() - 1) * (terrain->getWidth() / terrain->getStep() - 1) * 6, GL_UNSIGNED_INT, (GLvoid*)((terrain->getHeight() / terrain->getStep() - 1) * (terrain->getWidth() / terrain->getStep() - 1) * 6 * sizeof(GLuint))));
    GL_CHECK(glBindVertexArray(0));

    // Before drawing the light source, disable face culling and depth testing to ensure that the light source is always visible
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // Use the cube shader program to draw the light source
    glUseProgram(CubeShaderProgram);
    GLint modelLoc = glGetUniformLocation(CubeShaderProgram, "model");
    GLint viewLocCube = glGetUniformLocation(CubeShaderProgram, "view");
    GLint projLocCube = glGetUniformLocation(CubeShaderProgram, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, lighting->getmodelMatrix());
    glUniformMatrix4fv(viewLocCube, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projLocCube, 1, GL_FALSE, projectionMatrix);

    // Draw the light cube
    GL_CHECK(glBindVertexArray(lighting->getVAO()));
    GL_CHECK(glDrawElements(GL_TRIANGLES, lighting->getIndices().size(), GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    // Re-enable face culling and depth testing to make sure that next frame is rendered correctly
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // update FPS
    updateFPS();
    
    glutSwapBuffers();
}


void cleanup() {
    // Delete the shader programs
    deleteShaderProgram(TerrainShaderProgram);
    deleteShaderProgram(CubeShaderProgram);
  
    // Delete the textures
    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '2': // Move the light source clockwise
            angle += 0.1f; 
            if (angle >= 2 * M_PI) angle -= 2 * M_PI; // Make sure the angle is in the range [0, 2π]
            lighting->updateLightPosition(angle); // update the light source position
            break;
        case '3': // Move the light source counterclockwise
            angle -= 0.1f; 
            if (angle < 0) angle += 2 * M_PI; // Make sure the angle is in the range [0, 2π]
            lighting->updateLightPosition(angle); // Update the light source position
            break;
        default:
            camera.keyboard(key, x, y);
            break;
    }
    glutPostRedisplay(); 
}

void mouse(int button, int state, int x, int y) {
    camera.mouse(button, state, x, y);
}

void mouseMotion(int x, int y) {
    camera.mouseMotion(x, y);
}

int main(int argc, char** argv) {
    // Use the command line parser to parse the command line arguments
    CommandLineParser parser;
    try {
        parser.parse(argc, argv);
    } catch (const std::runtime_error& e) {
        return 1;
    }

    // Set the parameters for the Perlin noise generator
    double frequency = parser.getFrequency();
    int octave = parser.getOctave();
    double amplitude = parser.getAmplitude();
    double persistence = parser.getPersistence();
    double lacunarity = parser.getLacunarity();
    int seed = parser.getSeed();
    int width = WIDTH * parser.getWidth();
    int step = width / (32 * std::pow(2, parser.getStep()));
    std::cout << "Current Terrain Parameter: Frequency: " << frequency << " Octave: " << octave 
                                        << " Amplitude: " << amplitude << " Persistence: " << persistence 
                                        << " Lacunarity: " << lacunarity << " Seed: " << seed << " Width: " << width
                                        << " Step: " << step << '\n';

    terrain->init(width, step, seed);
    lighting->init(width * 0.1f, width / 30);

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Terrain Generator");

    glutDisplayFunc(display); // Register the display callback function
    glutIdleFunc(glutPostRedisplay); // Register the idle callback function
    glutKeyboardFunc(keyboard); // Register the keyboard callback function
    glutMouseFunc(mouse); // Register the mouse callback function
    glutMotionFunc(mouseMotion); // Register the mouse motion callback function

    atexit(cleanup); // Register the cleanup function
 
    init(frequency, octave, amplitude, persistence, lacunarity, width); // Initialize the program

    glutMainLoop(); // Enter the GLUT main event loop
    return 0;
}

void updateFPS() {
    static int frameCount = 0;
    static double fps = 0.0;

    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = currentTime - lastTime;

    if (elapsedTime.count() >= 1.0) {
        fps = frameCount / elapsedTime.count();
        frameCount = 0;
        lastTime = currentTime;

        std::string title = "Terrain Generator - FPS: " + std::to_string(fps);
        glutSetWindowTitle(title.c_str());
    }
}
