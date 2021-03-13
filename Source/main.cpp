#include "../Externals/Include/Include.h"

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define INITIAL 4
#define NORMAL 5
#define PIXEL 6
#define ABSTRACT 7
#define WATERCOLOR 8
#define BLOOM 9
#define SINEWAVE 10
#define MAGNIFIER 11
#define BAR 12


#define deg2rad(x) ((x)*((3.1415926f)/(180.0f)))

using namespace glm;
using namespace std;

float timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

//default window size
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

// current window size
int screenWidth = WINDOW_WIDTH, screenHeight = WINDOW_HEIGHT;

int prevX = 0, prevY = 0;
// track ball
int trackLeftRight = 90, trackUpDown = 90, click = 0, magnify = 0;
vec3 rotate_axis_local = vec3(1.0f, 0.0f, 0.0f);
mat4 T, M;
mat4 R = mat4(1.0);
vec3 position = vec3(0.0f, 0.0f, 0.0f);

typedef struct
{
	GLuint diffuseTexture;
} PhongMaterial;

vector<PhongMaterial> allMaterial;
vector<PhongMaterial> allMaterialBar;

typedef struct
{
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object
	GLuint vbo_normal;
	GLuint vbo_texCoord;
	GLuint ibo;

	int materialId;
	int vertexCount;
	GLuint m_texture;
} Shape;

struct model
{
	vector<Shape> shapes;

	vec3 position = vec3(0, 0, 0);
	vec3 scale = vec3(1, 1, 1);
	vec3 rotation = vec3(0, 0, 0);
};
model scene_model;
model bar_model;

struct camera
{
	vec3 position;
	vec3 center;
	vec3 up_vector;
};
camera main_camera;

struct project_setting
{
	GLfloat nearClip, farClip;
	GLfloat fovy;
	GLfloat aspect; 
};
project_setting proj;

GLuint program;
GLuint iLocP;
GLuint iLocMV;
GLuint Width;
GLuint Bar;
mat4 project_matrix(1.0f);
mat4 view_matrix(1.0f);

// which state
GLuint state;

// window FBO
GLuint vao2;
GLuint fbo;
GLuint depthrbo;
GLuint program1;
GLuint window_vertex_buffer;
GLuint fboDataTexture;
GLuint pixelization;
GLuint window_bar;
GLuint window_width;

// blur FBO
GLuint fbo_blur;
GLuint depthrbo_blur;
GLuint program_blur;
GLuint fboDataTexture_blur;
GLuint Blur;
GLuint Bar_blur;
GLuint Width_blur;

// blur FBO
GLuint fbo_blur2;
GLuint depthrbo_blur2;
GLuint program_blur2;
GLuint fboDataTexture_blur2;
GLuint Blur2;

// Quantization FBO
GLuint fbo_Quantization;
GLuint depthrbo_Quantization;
GLuint program_Quantization;
GLuint fboDataTexture_Quantization;
GLuint Quantization;
GLuint Bar_q;
GLuint Width_q;

// DOG FBO
GLuint fbo_DOG;
GLuint depthrbo_DOG;
GLuint program_DOG;
GLuint fboDataTexture_DOG;
GLuint Dog;
GLuint WIDTH_DOG;
GLuint HEIGHT_DOG;
GLuint Bar_DOG;
GLuint Width_DOG;

// WaterColor FBO
GLuint fbo_Water;
GLuint depthrbo_Water;
GLuint program_Water;
GLuint fboDataTexture_Water;

// noise texture
GLuint noise_tex;

// Bloom FBO
GLuint fbo_Bloom;
GLuint depthrbo_Bloom;
GLuint fboDataTexture_Bloom;

// sub window
GLuint fbo_Sub;
GLuint depthrbo_Sub;
GLuint fboDataTexture_Sub;

// sineWave
GLuint sineWave_bar;
GLuint sineWave_width;
GLuint sineWave;
GLfloat timer;

// tecture binding point
GLuint texture_location_blur1;
GLuint texture_location_blur2;
GLuint texture_location_blur3;
GLuint texture_location_quantization1;
GLuint texture_location_quantization2;
GLuint texture_location_quantization3;
GLuint texture_location_DOG1;
GLuint texture_location_DOG2;
GLuint texture_location_DOG3;

// magnifier
GLfloat mouse_x;
GLfloat mouse_y;
GLuint Magnify;
GLuint Magnify_width;
GLuint Magnify_height;
GLfloat LenSize;
GLfloat uniBar_click;
float lenSize = 0.2f;
int magnifier_click = 0;
int flag = 0;



int stateFlag = 0;
int bar_click = 0;
int X = 300; //for comparison bar
int bar = 1;

static const GLfloat window_vertex[] =
{
	//vec2 position vec2 texture_coord
	1.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};

static const GLenum scene_buffers[] =
{
	GL_COLOR_ATTACHMENT0,
};

char** loadShaderSource(const char* file)
{
    FILE* fp = fopen(file, "rb");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *src = new char[sz + 1];
    fread(src, sizeof(char), sz, fp);
    src[sz] = '\0';
    char **srcp = new char*[1];
    srcp[0] = src;
    return srcp;
}

void freeShaderSource(char** srcp)
{
    delete[] srcp[0];
    delete[] srcp;
}

// define a simple data structure for storing texture image raw data
typedef struct _TextureData
{
    _TextureData(void) :
        width(0),
        height(0),
        data(0)
    {
    }

    int width;
    int height;
    unsigned char* data;
} TextureData;

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
TextureData loadImage(const char* const Filepath)
{
	TextureData texture;
	int n;
	int len = strlen("..\\Assets\\sponza\\") + strlen(Filepath) + 1;
	char *filepath = (char*)malloc(sizeof(char) * len);
	memset(filepath, '\0', len);
	strcat(filepath, "..\\Assets\\sponza\\");
	strcat(filepath, Filepath);
	stbi_set_flip_vertically_on_load(true);
	stbi_uc *data = stbi_load(filepath, &texture.width, &texture.height, &n, 4);
	if (data != NULL)
	{
		texture.data = new unsigned char[texture.width * texture.height * 4 * sizeof(unsigned char)];
		memcpy(texture.data, data, texture.width * texture.height * 4 * sizeof(unsigned char));
		stbi_image_free(data);
	}
	return texture;
}

void createScene() {
	aiString file;
	file = "../Assets/sponza/sponza.obj";
	const aiScene *scene = aiImportFile(file.C_Str() ,aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scene)
		cout << "error" << endl;
	cout << scene->mNumMaterials << endl;
	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial *material = scene->mMaterials[i];
		PhongMaterial Material;
		aiString texturePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			cout << "load" << endl;
			// load width, height and data from texturePath.C_Str();
			TextureData tdata = loadImage(texturePath.C_Str());

			glGenTextures(1, &Material.diffuseTexture);
			glBindTexture(GL_TEXTURE_2D, Material.diffuseTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{	
			cout << "fail" << endl;
			// load some default image as default_diffuse_tex
			// Material.diffuseTexture = 0;
		}
		// save material
		allMaterial.push_back(Material);
	}


	cout << scene->mNumMeshes << endl;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[i];
		Shape shape;
		glGenVertexArrays(1, &shape.vao);
		glBindVertexArray(shape.vao);

		// create 3 vbos to hold data
		vector<GLfloat> vertices;
		vector<GLfloat> normals;
		vector<GLfloat> textureCoords;

		cout << mesh->mNumVertices << endl;

		for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
		{
			// mesh->mVertices[v][0~2] => position
			vertices.push_back(mesh->mVertices[v][0]);
			vertices.push_back(mesh->mVertices[v][1]);
			vertices.push_back(mesh->mVertices[v][2]);

			// mesh->mNormals[v][0~2] => normal
			normals.push_back(mesh->mNormals[v][0]);
			normals.push_back(mesh->mNormals[v][1]);
			normals.push_back(mesh->mNormals[v][2]);

			// mesh->mTextureCoords[0][v][0~1] => texcoord

			textureCoords.push_back(mesh->mTextureCoords[0][v][0]);
			textureCoords.push_back(mesh->mTextureCoords[0][v][1]);
		}
		// create 1 ibo to hold data
		vector<GLint> indices;
		cout << mesh->mNumFaces << endl;
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		{
			// mesh->mFaces[f].mIndices[0~2] => index
			indices.push_back(mesh->mFaces[f].mIndices[0]);
			indices.push_back(mesh->mFaces[f].mIndices[1]);
			indices.push_back(mesh->mFaces[f].mIndices[2]);
		}

		// glVertexAttribPointer / glEnableVertexArray calls¡K
		glGenBuffers(1, &shape.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), &vertices.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &shape.vbo_texCoord);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_texCoord);
		glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(GL_FLOAT), &textureCoords.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &shape.vbo_normal);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_normal);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), &normals.at(0), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &shape.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GL_INT), &indices.at(0), GL_STATIC_DRAW);
		//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		//glEnableVertexAttribArray(3);

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);

		shape.materialId = mesh->mMaterialIndex;
		shape.vertexCount = mesh->mNumFaces * 3;

		// save shape¡K
		scene_model.shapes.push_back(shape);
	}

	aiReleaseImport(scene);

}

void initParameter()
{
	proj.nearClip = 0.1;
	proj.farClip = 20000.0;
	proj.fovy = 100;
	proj.aspect = (float)(WINDOW_WIDTH) / (float)WINDOW_HEIGHT; // adjust width for side by side view

	main_camera.position = vec3(0.0f, 0.0f, 0.0f);
	main_camera.center = vec3(sin(deg2rad(trackLeftRight)), 0.0f, cos(deg2rad(trackUpDown)));
	main_camera.up_vector = vec3(0.0f, 1.0f, 0.0f);
}

void createShader()
{
	// Create Shader Program
	program = glCreateProgram();
	program1 = glCreateProgram();
	program_blur = glCreateProgram();
	program_Quantization = glCreateProgram();
	program_DOG = glCreateProgram();

	/* Create customize shader by tell openGL specify shader type */ 
	// intial scene
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);


	// window
	GLuint vertexShader1 = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);

	// abstraction
	GLuint fragmentShader_blur = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint fragmentShader_Q = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint fragmentShader_DOG = glCreateShader(GL_FRAGMENT_SHADER);

	/* Load shader file */
	// initial scene shader
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");

	// abstraction shader
	char** fragmentShaderSource_Blur = loadShaderSource("fragmentBlur.fs.glsl");
	char** fragmentShaderSource_Q = loadShaderSource("fragmentQ.fs.glsl");
	char** fragmentShaderSource_DOG = loadShaderSource("fragmentDOG.fs.glsl");

	// window shader
	char** vertexShaderSource1 = loadShaderSource("vertex1.vs.glsl");
	char** fragmentShaderSource1 = loadShaderSource("fragment1.fs.glsl");

	/* Assign content of these shader files to those shaders we created before */
	// intial scene
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	// abstraction
	glShaderSource(fragmentShader_blur, 1, fragmentShaderSource_Blur, NULL);
	glShaderSource(fragmentShader_Q, 1, fragmentShaderSource_Q, NULL);
	glShaderSource(fragmentShader_DOG, 1, fragmentShaderSource_DOG, NULL);

	// window
	glShaderSource(vertexShader1, 1, vertexShaderSource1, NULL);
	glShaderSource(fragmentShader1, 1, fragmentShaderSource1, NULL);


	/* Free the shader file string(won't be used any more) */
	// intial scene
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	// abstraction
	freeShaderSource(fragmentShaderSource_Blur);
	freeShaderSource(fragmentShaderSource_Q);
	freeShaderSource(fragmentShaderSource_DOG);

	// window
	freeShaderSource(vertexShaderSource1);
	freeShaderSource(fragmentShaderSource1);

	/* Compile these shaders */
	// intial scene
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	// abstraction
	glCompileShader(fragmentShader_blur);
	glCompileShader(fragmentShader_Q);
	glCompileShader(fragmentShader_DOG);

	// window
	glCompileShader(vertexShader1);
	glCompileShader(fragmentShader1);

	/* Assign the program we created before with these shaders */
	// intial scene
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// abstraction
	glAttachShader(program_blur, vertexShader1);
	glAttachShader(program_blur, fragmentShader_blur);
	glLinkProgram(program_blur);

	glAttachShader(program_Quantization, vertexShader1);
	glAttachShader(program_Quantization, fragmentShader_Q);
	glLinkProgram(program_Quantization);

	glAttachShader(program_DOG, vertexShader1);
	glAttachShader(program_DOG, fragmentShader_DOG);
	glLinkProgram(program_DOG);

	// window
	glAttachShader(program1, vertexShader1);
	glAttachShader(program1, fragmentShader1);
	glLinkProgram(program1);

	glUseProgram(program);
}

void setUniform() {
	iLocP = glGetUniformLocation(program, "um4p");
	iLocMV = glGetUniformLocation(program, "um4mv");

	// intial scene
	state = glGetUniformLocation(program, "State");
	Bar = glGetUniformLocation(program, "bar");
	Width = glGetUniformLocation(program, "width");

	// window
	pixelization = glGetUniformLocation(program1, "pixel");
	window_bar = glGetUniformLocation(program1, "bar");
	window_width = glGetUniformLocation(program1, "width");
	mouse_x = glGetUniformLocation(program1, "mx");
	mouse_y = glGetUniformLocation(program1, "my");
	Magnify = glGetUniformLocation(program1, "magnify");
	Magnify_width = glGetUniformLocation(program1, "magnify_width");
	Magnify_height = glGetUniformLocation(program1, "magnify_height");
	LenSize = glGetUniformLocation(program1, "lenSize");
	uniBar_click = glGetUniformLocation(program1, "bar_click");

	// blur
	Blur = glGetUniformLocation(program_blur, "blur");
	Bar_blur = glGetUniformLocation(program_blur, "bar");
	Width_blur = glGetUniformLocation(program_blur, "width");

	texture_location_blur1 = glGetUniformLocation(program_blur, "tex");
	glUniform1i(texture_location_blur1, 0);
	texture_location_blur2 = glGetUniformLocation(program_blur, "tex_b");
	glUniform1i(texture_location_blur2, 1);

	// quantization
	Quantization = glGetUniformLocation(program_Quantization, "quantization");
	Bar_q = glGetUniformLocation(program_Quantization, "bar");
	Width_q = glGetUniformLocation(program_Quantization, "width");

	texture_location_quantization1 = glGetUniformLocation(program_Quantization, "tex");
	glUniform1i(texture_location_quantization1, 0);
	texture_location_quantization2 = glGetUniformLocation(program_Quantization, "tex_N");
	glUniform1i(texture_location_quantization2, 1);
	texture_location_quantization3 = glGetUniformLocation(program_Quantization, "tex2");
	glUniform1i(texture_location_quantization3, 2);

	// dog
	Dog = glGetUniformLocation(program_DOG, "DOG");
	WIDTH_DOG = glGetUniformLocation(program_DOG, "width_Dog");
	HEIGHT_DOG = glGetUniformLocation(program_DOG, "height_Dog");
	Bar_DOG = glGetUniformLocation(program_DOG, "bar");
	Width_DOG = glGetUniformLocation(program_DOG, "width");

	texture_location_DOG1 = glGetUniformLocation(program_blur, "tex");
	glUniform1i(texture_location_DOG1, 0);
	texture_location_DOG2 = glGetUniformLocation(program_blur, "tex_q");
	glUniform1i(texture_location_DOG2, 1);

	// sineWave
	sineWave = glGetUniformLocation(program1, "sinewave");
	sineWave_bar = glGetUniformLocation(program1, "bar");
	sineWave_width = glGetUniformLocation(program1, "width");
	timer = glGetUniformLocation(program1, "Timer");


}

void createFBO() {
	/* window fbo */
	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);

	glGenBuffers(1, &window_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, window_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertex), window_vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	//Create FBO
	glGenFramebuffers(1, &fbo);

	/* blur fbo */
	glGenFramebuffers(1, &fbo_blur);

	/* blur2 fbo */
	glGenFramebuffers(1, &fbo_blur2);

	/* quantization fbo */
	glGenFramebuffers(1, &fbo_Quantization);

	/* dog fbo */
	glGenFramebuffers(1, &fbo_DOG);

	/* waterColor fbo */
	glGenFramebuffers(1, &fbo_Water);

	/* Bloom fbo */
	glGenFramebuffers(1, &fbo_Bloom);

	/* subWindow fbo */
	glGenFramebuffers(1, &fbo_Sub);
}

void My_Init()
{
    glClearColor(0.0f, 0.6f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	createShader();
	initParameter();
	setUniform();
	glUniform1i(Width, screenWidth/2);

	createScene();
	createFBO();

	TextureData tdata = loadImage("textures\\noise.jpg");

	glGenTextures(1, &noise_tex);
	glBindTexture(GL_TEXTURE_2D, noise_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void change_camera() {
	main_camera.center.x = sin(deg2rad(trackLeftRight)) * sin(deg2rad(trackUpDown));
	main_camera.center.y = cos(deg2rad(trackUpDown));
	main_camera.center.z = cos(deg2rad(trackLeftRight)) * sin(deg2rad(trackUpDown));
}

void My_Display()
{
	//Bind FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	//which render buffer attachment is written
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	mat4 model_matrix = translate(mat4(1.0), position);

	view_matrix = lookAt(main_camera.position, main_camera.center, main_camera.up_vector);

	glUniformMatrix4fv(iLocMV, 1, GL_FALSE, value_ptr(view_matrix * model_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	glActiveTexture(GL_TEXTURE0);
	if (bar == 1)
		glUniform1i(Bar, 1);
	else
		glUniform1i(Bar, 0);
	glUniform1i(Width, X);

	if (stateFlag == 0) {
		glUniform1i(state, 0);
	}
	else if (stateFlag == 1) {
		glUniform1i(state, 1);
	}
	else {
		glUniform1i(state, 0);
	}

	for (int i = 0; i < scene_model.shapes.size(); ++i)
	{
		glBindVertexArray(scene_model.shapes[i].vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene_model.shapes[i].ibo);

		int materialID = scene_model.shapes[i].materialId;
		glBindTexture(GL_TEXTURE_2D, allMaterial[materialID].diffuseTexture);
		glDrawElements(GL_TRIANGLES, scene_model.shapes[i].vertexCount, GL_UNSIGNED_INT, 0);
	}

	/* abstraction */
	// Blur
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_blur);
	glBindVertexArray(vao2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program_blur);
	//which render buffer attachment is written
	//glViewport(0, 0, screenWidth, screenHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	// blur mode
	glUniform1i(Width_blur, X);
	if (stateFlag == 3 || stateFlag == 4 || stateFlag == 5) {
		cout << "blur" << endl;
		glUniform1i(Blur, 1);
	}
	else {
		glUniform1i(Blur, 0);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


	// Blur2
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_blur2);
	glBindVertexArray(vao2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program_blur);
	//which render buffer attachment is written
	//glViewport(0, 0, screenWidth, screenHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_blur);
	// blur mode
	glUniform1i(Width_blur, X);
	if (stateFlag == 5) {
		cout << "blur2" << endl;
		glUniform1i(Blur, 1);
	}
	else {
		glUniform1i(Blur, 0);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Bloom
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Bloom);
	glBindVertexArray(vao2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program_blur);
	//which render buffer attachment is written
	//glViewport(0, 0, screenWidth, screenHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_blur2);
	// blur mode
	glUniform1i(Width_blur, X);
	if (bar == 1)
		glUniform1i(Bar_blur, 1);
	else
		glUniform1i(Bar_blur, 0);
	if (stateFlag == 5) {
		cout << "bloom" << endl;
		glUniform1i(Blur, 2);
	}
	else {
		glUniform1i(Blur, 0);
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// WaterColor
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Water);
	glBindVertexArray(vao2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program_Quantization);
	//which render buffer attachment is written
	//glViewport(0, 0, screenWidth, screenHeight);
	glUniform1i(texture_location_quantization1, 0);
	glUniform1i(texture_location_quantization2, 1);
	glUniform1i(texture_location_quantization3, 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_blur);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, noise_tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);


	// WaterColor mode
	if (stateFlag == 4) {
		cout << "watercolor" << endl;
		glUniform1i(Quantization, 2);
	}
	else {
		glUniform1i(Quantization, 0);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


	// Quantization
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Quantization);
	glBindVertexArray(vao2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program_Quantization);
	//which render buffer attachment is written
	//glViewport(0, 0, screenWidth, screenHeight);

	glUniform1i(texture_location_quantization1, 0);
	glUniform1i(texture_location_quantization2, 1);
	glUniform1i(texture_location_quantization3, 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Water);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, noise_tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);

	// quantization mode
	glUniform1i(Width_q, X);
	if (bar == 1)
		glUniform1i(Bar_q, 1);
	else
		glUniform1i(Bar_q, 0);
	if (stateFlag == 3 || stateFlag == 4) {
		cout << "quantizationWater" << endl;
		glUniform1i(Quantization, 1);
	}
	else {
		glUniform1i(Quantization, 0);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


	// DOG
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_DOG);
	glBindVertexArray(vao2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program_DOG);
	//which render buffer attachment is written
	//glViewport(0, 0, screenWidth, screenHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Quantization);
	// DOG mode
	if (stateFlag == 3) {
		cout << "DOG" << endl;
		if (bar == 1) {
			glUniform1i(Bar_DOG, 1);
		}
		else {
			glUniform1i(Bar_DOG, 0);
		}
		glUniform1i(Width_DOG, X);
		glUniform1i(Dog, 1);
		glUniform1i(WIDTH_DOG, screenWidth);
		glUniform1i(HEIGHT_DOG, screenHeight);
	}
	else {
		glUniform1i(Dog, 0);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// subWindow
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Sub);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glViewport(0, 0, screenWidth, screenHeight);

	glBindVertexArray(vao2);
	glUseProgram(program1);

	glActiveTexture(GL_TEXTURE0);

	//clear
	glUniform1i(pixelization, 0);
	glUniform1i(sineWave, 0);

	if(stateFlag == 2) {
		if (bar == 1)
			glUniform1i(window_bar, 1);
		else
			glUniform1i(window_bar, 0);
		glUniform1i(window_width, X);
		glUniform1i(pixelization, 1);
		glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	}
	else if (stateFlag == 6) {
		if (bar == 1)
			glUniform1i(sineWave_bar, 1);
		else
			glUniform1i(sineWave_bar, 0);
		glUniform1i(sineWave_width, X);
		glUniform1i(sineWave, 1);
		glUniform1f(timer, timer_cnt);
		glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Now Return to the default framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glViewport(0, 0, screenWidth, screenHeight);

	glBindVertexArray(vao2);
	glUseProgram(program1);

	glActiveTexture(GL_TEXTURE0);

	//clear
	glUniform1i(pixelization, 0);
	glUniform1i(sineWave, 0);
	glUniform1i(window_width, X);
	if (stateFlag == 0) { // initial scene
		glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	}
	else if (stateFlag == 1) {
		glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	}
	else if (stateFlag == 2) { // pixelization
		glBindTexture(GL_TEXTURE_2D, fboDataTexture_Sub);
	}
	else if (stateFlag == 3) // abstraction
		glBindTexture(GL_TEXTURE_2D, fboDataTexture_DOG);
	else if (stateFlag == 4) // watercolor
		glBindTexture(GL_TEXTURE_2D, fboDataTexture_Quantization);
	else if (stateFlag == 5) // bloom
		glBindTexture(GL_TEXTURE_2D, fboDataTexture_Bloom);
	else if (stateFlag == 6) { // sinewave
		glBindTexture(GL_TEXTURE_2D, fboDataTexture_Sub);
	}
	else {
		glUniform1i(pixelization, 0);
		glUniform1i(sineWave, 0);
	}

	glUniform1f(mouse_x, prevX);
	glUniform1f(mouse_y, prevY);
	if (magnify == 1) {
		glUniform1i(uniBar_click, bar_click);
		glUniform1i(Magnify_width, screenWidth);
		glUniform1i(Magnify_height, screenHeight);
		glUniform1i(Magnify, 1);
	} else{
		glUniform1i(Magnify, 0);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	proj.aspect = (float)(width) / (float)height;
	screenWidth = width;
	screenHeight = height;

	glUniform1i(Width, screenWidth / 2);
	glViewport(0, 0, screenWidth, screenHeight);
	project_matrix = perspective(deg2rad(proj.fovy), proj.aspect, proj.nearClip, proj.farClip);


	/* subWindow */
	//renew Depth RBO
	glDeleteRenderbuffers(1, &depthrbo_Sub);
	glDeleteTextures(1, &fboDataTexture_Sub);
	glGenRenderbuffers(1, &depthrbo_Sub);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_Sub);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	//renew fboDataTexture
	glGenTextures(1, &fboDataTexture_Sub);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Sub);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Sub);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_Sub);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_Sub, 0);

	/* Quantization */
	//renew Depth RBO
	glDeleteRenderbuffers(1, &depthrbo_Quantization);
	glDeleteTextures(1, &fboDataTexture_Quantization);
	glGenRenderbuffers(1, &depthrbo_Quantization);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_Quantization);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	//renew fboDataTexture
	glGenTextures(1, &fboDataTexture_Quantization);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Quantization);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Quantization);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_Quantization);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_Quantization, 0);

	/* DOG */
	//renew Depth RBO
	glDeleteRenderbuffers(1, &depthrbo_DOG);
	glDeleteTextures(1, &fboDataTexture_DOG);
	glGenRenderbuffers(1, &depthrbo_DOG);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_DOG);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	//renew fboDataTexture
	glGenTextures(1, &fboDataTexture_DOG);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_DOG);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_DOG);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_DOG);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_DOG, 0);

	/* WaterColor */
	//renew Depth RBO
	glDeleteRenderbuffers(1, &depthrbo_Water);
	glDeleteTextures(1, &fboDataTexture_Water);
	glGenRenderbuffers(1, &depthrbo_Water);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_Water);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	//renew fboDataTexture
	glGenTextures(1, &fboDataTexture_Water);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Water);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Water);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_Water);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_Water, 0);


	/* bloom */
	//renew Depth RBO
	glDeleteRenderbuffers(1, &depthrbo_Bloom);
	glDeleteTextures(1, &fboDataTexture_Bloom);
	glGenRenderbuffers(1, &depthrbo_Bloom);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_Bloom);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	//renew fboDataTexture
	glGenTextures(1, &fboDataTexture_Bloom);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Bloom);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Bloom);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_Bloom);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_Bloom, 0);


	/* blur2 */
	//renew Depth RBO
	glDeleteRenderbuffers(1, &depthrbo_blur2);
	glDeleteTextures(1, &fboDataTexture_blur2);
	glGenRenderbuffers(1, &depthrbo_blur2);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_blur2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	//renew fboDataTexture
	glGenTextures(1, &fboDataTexture_blur2);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_blur2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_blur2);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_blur2);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_blur2, 0);



	/* blur */
	//renew Depth RBO
	glDeleteRenderbuffers(1, &depthrbo_blur);
	glDeleteTextures(1, &fboDataTexture_blur);
	glGenRenderbuffers(1, &depthrbo_blur);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_blur);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);

	//renew fboDataTexture
	glGenTextures(1, &fboDataTexture_blur);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_blur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_blur);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_blur);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_blur, 0);

	/* window */
	// renew DEPTH_ATTACHMENT and COLOR_ATTACHMENT
	glDeleteRenderbuffers(1, &depthrbo);
	glDeleteTextures(1, &fboDataTexture);
	glGenRenderbuffers(1, &depthrbo);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);

	glGenTextures(1, &fboDataTexture);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture, 0);
}

void My_Timer(int val)
{
	timer_cnt += 0.05f;
	glutPostRedisplay();
	glutTimerFunc(timer_speed, My_Timer, val);
}

void My_Mouse(int button, int state, int x, int y)
{
	if(state == GLUT_DOWN)
	{
		printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
		bar_click = 1;
		click = 1;
		magnifier_click = 1;
		prevX = x;
		prevY = y;
	}
	else if (state == GLUT_UP)
	{
		printf("Mouse %d is released at (%d, %d)\n", button, x, y);
		bar_click = 0;
		magnifier_click = 0;
		click = 0;
		flag = 0;
	}
}

void MouseMove(int x, int y)
{
	cout << "Move" << endl;
	if (x > X - 5 && x < X + 5 && bar_click == 1 && magnifier_click != 2) {
		cout << x << endl;
		flag = 1;
		bar_click = 2;
		X = x;
	}
	else if (bar_click == 2) {
		X = x;
	}
	else if (click == 1 && bar == 1 && magnify != 1 && bar_click != 2) {
		cout << main_camera.center.x << endl;
		cout << main_camera.center.y << endl;
		cout << main_camera.center.z << endl;
		trackLeftRight += (x - prevX) / 2;
		float tempY = y - prevY;

		if (tempY > 0) {
			if (trackUpDown - tempY > 0) {
				trackUpDown -= tempY / 2;
			}
		}
		else {
			if (trackUpDown - tempY < 180) {
				trackUpDown -= tempY / 2;
			}
		}

		change_camera();
		prevX = x;
		prevY = y;
		view_matrix = lookAt(main_camera.position, main_camera.center, main_camera.up_vector);
	}
	else if (click == 1 && magnify == 1) {
		vec2 d1 = vec2(x, y).xy / vec2(screenWidth, screenHeight).xy;
		vec2 d2 = vec2(prevX, prevY).xy / vec2(screenWidth, screenHeight).xy;

		vec2 d = d1 - d2;
		float dist = sqrt(dot(d, d)); 
		cout <<  "dist: "<< dist - lenSize << endl;
		glUniform1f(LenSize, lenSize);
		
		if (bar_click != 2 && flag == 0) {
			magnifier_click = 2;
			prevX = x;
			prevY = y;
		}
	}
	glutPostRedisplay();
}

void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);

	if (key == 'z') {
		if (trackUpDown - 15 > 0) {
			trackUpDown -= 15;
		}
		change_camera();
	}
	else if (key == 'x') {
		if (trackUpDown + 15 < 180) {
			trackUpDown += 15;
		}
		change_camera();
	}
	else if (key == 'w') {
		vec3 move = vec3(sin(deg2rad(trackLeftRight)) * 3, cos(deg2rad(trackUpDown)) * 3, cos(deg2rad(trackLeftRight)) * 3);
		position -= move;
	}
	else if (key == 's') {
		vec3 move = vec3(sin(deg2rad(trackLeftRight)) * 3, cos(deg2rad(trackUpDown)) * 3, cos(deg2rad(trackLeftRight)) * 3);
		position += move;
	}
	else if (key == 'a') {
		vec3 move = vec3(sin(deg2rad(trackLeftRight - 90)) * 3, 0, cos(deg2rad(trackLeftRight - 90)) * 3);
		position += move;
	}
	else if (key == 'd') {
		vec3 move = vec3(sin(deg2rad(trackLeftRight - 90)) * 3, 0, cos(deg2rad(trackLeftRight - 90)) * 3);
		position -= move;
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	vec3 move;
	switch(key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		//move = vec3(sin(deg2rad(trackLeftRight)) * 3, cos(deg2rad(trackUpDown)) * 3, cos(deg2rad(trackLeftRight)) * 3);
		//position -= move;
		break;
	case GLUT_KEY_DOWN:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		//move = vec3(sin(deg2rad(trackLeftRight)) * 3, cos(deg2rad(trackUpDown)) * 3, cos(deg2rad(trackLeftRight)) * 3);
		//position += move;
		break;
	case GLUT_KEY_LEFT:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		//move = vec3(sin(deg2rad(trackLeftRight - 90)) * 3, 0, cos(deg2rad(trackLeftRight - 90)) * 3);
		//position += move;
		break;
	case GLUT_KEY_RIGHT:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		//move = vec3(sin(deg2rad(trackLeftRight - 90)) * 3, 0, cos(deg2rad(trackLeftRight - 90)) * 3);
		//position -= move;
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch(id)
	{
	case INITIAL:
		stateFlag = 0;
		break;
	case NORMAL:
		cout << "Normal" << endl;
		stateFlag = 1;
		break;
	case PIXEL:
		cout << "Pixelization" << endl;
		stateFlag = 2;
		break;
	case ABSTRACT:
		cout << "Abstraction" << endl;
		stateFlag = 3;
		break;
	case WATERCOLOR:
		cout << "WaterColor" << endl;
		stateFlag = 4;
		break;
	case BLOOM:
		cout << "Bloom" << endl;
		stateFlag = 5;
		break;
	case SINEWAVE:
		cout << "SineWave" << endl;
		stateFlag = 6;
		break;
	case MAGNIFIER:
		cout << "Magnifier" << endl;
		//if (bar != 1) {
		if (magnify == 0)
			magnify = 1;
		else
			magnify = 0;
		//}
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
    // Change working directory to source code path
    chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("AS2_FrameBuffer"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
    glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_state = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("State", menu_state);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_state);
	glutAddMenuEntry("Initial", INITIAL);
	glutAddMenuEntry("Normal", NORMAL);
	glutAddMenuEntry("Pixelization", PIXEL);
	glutAddMenuEntry("Abstraction", ABSTRACT);
	glutAddMenuEntry("WaterColor", WATERCOLOR);
	glutAddMenuEntry("Bloom", BLOOM);
	glutAddMenuEntry("SineWave", SINEWAVE);
	glutAddMenuEntry("Magnifier", MAGNIFIER);
	// glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutMotionFunc(MouseMove);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0); 

	// Enter main event loop.
	glutMainLoop();

	return 0;
}
