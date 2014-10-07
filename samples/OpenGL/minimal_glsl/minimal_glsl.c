#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/glfw.h>

static void pushModelview()
{
    GLenum prev_matrix_mode;
    glGetIntegerv(GL_MATRIX_MODE, &prev_matrix_mode);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(prev_matrix_mode);
}

static void popModelview()
{
    GLenum prev_matrix_mode;
    glGetIntegerv(GL_MATRIX_MODE, &prev_matrix_mode);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(prev_matrix_mode);
}

static const GLchar *vertex_shader_source =
"#version 120\n"
"void main()"
"{"
"   gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;"
"   gl_TexCoord[0] = gl_MultiTexCoord0;"
"   gl_FrontColor = gl_Color;"
"   gl_BackColor = gl_Color;"
"}\0";
GLuint shaderVertex = 0;

static const GLchar *fragment_shader_source = 
"#version 120\n"
"uniform sampler2D texCMYK;"
"uniform sampler2D texRGB;"
"void main()"
"{"
"   gl_FragColor = -texture2D(texCMYK, gl_TexCoord[0].st) + texture2D(texRGB, gl_TexCoord[0].st); "
"}\0";
GLuint shaderFragment = 0;

GLuint shaderProgram = 0;

#define TEX_CMYK_WIDTH 2
#define TEX_CMYK_HEIGHT 2
GLubyte textureDataCMYK[TEX_CMYK_WIDTH * TEX_CMYK_HEIGHT][3] = {
    {0x00, 0xff, 0xff}, {0xff, 0x00, 0xff},
    {0xff, 0xff, 0x00}, {0x00, 0x00, 0x00}
};
GLuint texCMYK = 0;

#define TEX_RGB_WIDTH 2
#define TEX_RGB_HEIGHT 2
GLubyte textureDataRGB[TEX_RGB_WIDTH * TEX_RGB_HEIGHT][3] = {
    {0x00, 0x00, 0xff}, {0xff, 0xff, 0xff},
    {0xff, 0x00, 0x00}, {0x00, 0xff, 0x00}
};
GLuint texRGB = 0;

/*  6----7
   /|   /|
  3----2 |
  | 5--|-4
  |/   |/
  0----1

*/

GLfloat cube_vertices[][8] =  {
	/*  X     Y     Z   Nx   Ny   Nz    S    T */
	{-1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, // 0
	{ 1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 1.0, 0.0}, // 1
	{ 1.0,  1.0,  1.0, 0.0, 0.0, 1.0, 1.0, 1.0}, // 2
	{-1.0,  1.0,  1.0, 0.0, 0.0, 1.0, 0.0, 1.0}, // 3

	{ 1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0}, // 4
	{-1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 1.0, 0.0}, // 5
	{-1.0,  1.0, -1.0, 0.0, 0.0, -1.0, 1.0, 1.0}, // 6
	{ 1.0,  1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 1.0}, // 7

	{-1.0, -1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0}, // 5
	{-1.0, -1.0,  1.0, -1.0, 0.0, 0.0, 1.0, 0.0}, // 0
	{-1.0,  1.0,  1.0, -1.0, 0.0, 0.0, 1.0, 1.0}, // 3
	{-1.0,  1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 1.0}, // 6
	
	{ 1.0, -1.0,  1.0,  1.0, 0.0, 0.0, 0.0, 0.0}, // 1
	{ 1.0, -1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 0.0}, // 4
	{ 1.0,  1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 1.0}, // 7
	{ 1.0,  1.0,  1.0,  1.0, 0.0, 0.0, 0.0, 1.0}, // 2
	
	{-1.0, -1.0, -1.0,  0.0, -1.0, 0.0, 0.0, 0.0}, // 5
	{ 1.0, -1.0, -1.0,  0.0, -1.0, 0.0, 1.0, 0.0}, // 4
	{ 1.0, -1.0,  1.0,  0.0, -1.0, 0.0, 1.0, 1.0}, // 1
	{-1.0, -1.0,  1.0,  0.0, -1.0, 0.0, 0.0, 1.0}, // 0

	{-1.0, 1.0,  1.0,  0.0,  1.0, 0.0, 0.0, 0.0}, // 3
	{ 1.0, 1.0,  1.0,  0.0,  1.0, 0.0, 1.0, 0.0}, // 2
	{ 1.0, 1.0, -1.0,  0.0,  1.0, 0.0, 1.0, 1.0}, // 7
	{-1.0, 1.0, -1.0,  0.0,  1.0, 0.0, 0.0, 1.0}, // 6
};

static void draw_cube(void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 8, &cube_vertices[0][0]);
    glNormalPointer(GL_FLOAT, sizeof(GLfloat) * 8, &cube_vertices[0][3]);
    glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 8, &cube_vertices[0][6]);

    glDrawArrays(GL_QUADS, 0, 24);
}

static void bind_sampler_to_unit_with_texture(GLchar const * const sampler_name, GLuint texture_unit, GLuint texture)
{
        glActiveTexture(GL_TEXTURE0 + texture_unit); 
        glBindTexture(GL_TEXTURE_2D, texture);
        GLuint loc_sampler = glGetUniformLocation(shaderProgram, sampler_name);
        glUniform1i(loc_sampler, texture_unit);
}

static void display(double T)
{
    int window_width, window_height;

    glfwGetWindowSize(&window_width, &window_height);
    if( !window_width || !window_height )
        return;

    const float window_aspect = (float)window_width / (float)window_height;

    glDisable(GL_SCISSOR_TEST);

    glClearColor(0.5, 0.5, 0.7, 1.0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-window_aspect, window_aspect, -1, 1, 2, 10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -5);

    pushModelview();
    glRotatef(T * 0.1 * 180, 0., 1., 0.);
    glRotatef(T * 0.1 *  60, 1., 0., 0.);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderProgram);
    bind_sampler_to_unit_with_texture("texCMYK", 0, texCMYK);
    bind_sampler_to_unit_with_texture("texRGB", 1, texRGB);

    draw_cube();
    popModelview();

    glfwSwapBuffers();
}

static int open_window(void)
{
#if 0
    glfwWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif

    if( glfwOpenWindow(0, 0,     /* default size */
                       8,  8, 8, /* 8 bits per channel */
                       8, 24, 8, /* 8 alpha, 24 depth, 8 stencil */
                       GLFW_WINDOW) != GL_TRUE ) {
        fputs("Could not open window.\n", stderr);
        return 0;
    }

    if( glewInit() != GLEW_OK ) {
        fputs("Could not initialize extensions.\n", stderr);
        return 0;
    }
    return 1;
}

static int check_extensions(void)
{
    if( !GLEW_ARB_vertex_shader ||
        !GLEW_ARB_fragment_shader ) {
        fputs("Required OpenGL functionality not supported by system.\n", stderr);
        return 0;
    }

    return 1;
}

static int check_shader_compilation(GLuint shader)
{
    GLint n;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &n);
        if( n == GL_FALSE ) {
        GLchar *info_log;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &n);
        info_log = malloc(n);
        glGetShaderInfoLog(shader, n, &n, info_log);
        fprintf(stderr, "Shader compilation failed: %*s\n", n, info_log);
        free(info_log);
        return 0;
    }
    return 1;
}

static int init_resources(void)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glGenTextures(1, &texCMYK);
    glBindTexture(GL_TEXTURE_2D, texCMYK);
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB8, TEX_CMYK_WIDTH, TEX_CMYK_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureDataCMYK);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &texRGB);
    glBindTexture(GL_TEXTURE_2D, texRGB);
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB8, TEX_RGB_WIDTH, TEX_RGB_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureDataRGB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    shaderVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderVertex, 1, (const GLchar**)&vertex_shader_source, NULL);
    glCompileShader(shaderVertex);
    if( !check_shader_compilation(shaderVertex) )
        return 0;

    shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderFragment, 1, (const GLchar**)&fragment_shader_source, NULL);
    glCompileShader(shaderFragment);
    if( !check_shader_compilation(shaderFragment) )
        return 0;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, shaderVertex);
    glAttachShader(shaderProgram, shaderFragment);
    glLinkProgram(shaderProgram);

    return 1;
}

static void main_loop(void)
{
    glfwSetTime(0);
    while( glfwGetWindowParam(GLFW_OPENED) == GL_TRUE ) {
        display(glfwGetTime());
    }
}

int main(int argc, char *argv[])
{
    if( glfwInit() != GL_TRUE ) {
        fputs("Could not initialize framework.\n", stderr);
        return -1;
    }

    if( !open_window() )
        return -1;

    if( !check_extensions() )
        return -1;

    if( !init_resources() )
        return -1;

    main_loop();

    glfwTerminate();
    return 0;
}

