#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>


#include <iostream>
#include <algorithm>    // std::min

#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp> 
//#include <FreeImage.h>

using namespace nlohmann; // for parsing JSON files


#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadImageSTBI(const std::string path, const bool flip_vertically, const bool Write); // extern 
void renderScene(const Shader &shader);
void renderCube();
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;
const char APP_NAME[] = "LFR";

// camera
Camera camera(glm::vec3(0.0f, 0.0f, -30.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90);
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO;



// current view
unsigned int currView = 0;

//Global Variables to differentiate intitiation and rendering in Renderer
GLFWwindow* window;
Shader debugDepthQuad;
Shader projectShader;
Shader demShader;
unsigned int planeVBO;
Model dem; 
unsigned int demTexture;
unsigned int depthMapFBO;
unsigned int depthMap;
glm::vec4 *fboData;

float demAlpha = 0.25;
const unsigned int SHADOW_WIDTH = 512;
const unsigned int SHADOW_HEIGHT = 512;
unsigned int fboCount = 0; 
glm::vec4 fboMin(0.0f); 
glm::vec4 fboMax(1.0f);

/////////////////////////////////////////Light Field Class/////////////////////////////////////////
class Lightfield
{
public:
    std::vector<std::string> pynames;
	std::vector<std::vector<double>> vectorposes;
    std::vector<unsigned int> ogl_textures;
    unsigned char *image_to_bind;
    float camerafocallength;
    glm::mat4 imgprojectionmat;
	Lightfield(const int preallocate = 0)
	{
		if (preallocate > 0)
		{
			pynames.reserve(preallocate);
			vectorposes.reserve(preallocate);
			ogl_textures.reserve(preallocate);
		}
	}
	void allocate(const int allocate = 0)
	{
		if (allocate > 0)
		{
			ogl_textures.reserve(allocate);
			pynames.reserve(allocate);
			vectorposes.reserve(allocate);
		}
		
	}
    //const glm::vec3 GetPosition(const unsigned int index) { return glm::vec3(glm::inverse(poses[index])[3]); }
	//const glm::vec3 GetUp(const unsigned int index) { return glm::vec3(glm::inverse(poses[index])[1]); }
	//const glm::vec3 GetForward(const unsigned int index) { return glm::vec3(glm::inverse(poses[index])[2]); }
	const unsigned int& GetOglTexture(const unsigned int index) { return ogl_textures[index]; }
	const std::string& GetPyNames(const unsigned int index) { return pynames[index];  }
	const unsigned int GetLFSize() { return vectorposes.size(); }
	//void GenerateLightFied(const std::string &jsonPoseFile, const std::string &imgFilePath = "" );
	const glm::mat4 GetvectorPose(const unsigned int index);
	void GenrateTextureID();
	void BindImageWithTextureID(unsigned int index,unsigned int width, unsigned int height, unsigned int nrComponents);
	void RendererInitializationPart1();
	void RendererInitializationPart2();
	void RenderOnce(float *RenderedImage);
	void LoadDemModel(string ObjFile, unsigned int width, unsigned int height, unsigned int nrComponents);
	void LFRTerminateRenderer();
	void SetProjectionMatrix();
	glm::mat4 GetProjectionMatrix();
};

const glm::mat4 Lightfield::GetvectorPose(const unsigned int index)
{
    int size = vectorposes[index].size(); 
    double *Poseaaray = new double[size + 4]; 
    std::copy(vectorposes[index].begin(),vectorposes[index].end(),Poseaaray);
    *(Poseaaray + size) = 0.0;*(Poseaaray + size + 1) = 0.0;*(Poseaaray + size + 2) = 0.0;*(Poseaaray + size + 3) = 1.0;
    auto glmMat = glm::mat4( 1.0 );
    glmMat = glm::transpose(glm::make_mat4(Poseaaray));
    
    /*
    if (index == 1)
    {
	std::vector<double> Pose0 = vectorposes[index];
	std::copy(vectorposes[index].begin(),vectorposes[index].end(),std::ostream_iterator<int>(std::cout,""));
	std::cout<< "Double Array Of Size " << size <<std::endl;
	for (int i = 0; i < (size +4); i++) 
	    { 
		std::cout <<  *(Poseaaray + i) << std::endl; 
	    } 
	std::cout << "matrix: " << glm::to_string(glmMat) << std::endl;
    }
    ////////////////////////////Debug/////////////
    */
    delete (Poseaaray);
    return glmMat; 
	
}

void Lightfield::GenrateTextureID()
{
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);
    ogl_textures.push_back(textureID);
}

void Lightfield::BindImageWithTextureID(unsigned int index, unsigned int width, unsigned int height, unsigned int nrComponents)
{
    //unsigned int width = 512;
    //unsigned int height = 512;
    //unsigned int nrComponents = 1;
    unsigned int textureID = ogl_textures[index];
    GLenum format;
	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;
    ///////////////DEBUG///////////////////////////////////////////////////////////
	//string NameIndex = std::to_string(index) + ".png";
	//std::cout << NameIndex << std::endl;
	//stbi_write_png(NameIndex.c_str(), 512, 512, 1, this->LoadImage, 512 * 1);
    ///////////////DEBUG///////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, this->image_to_bind);
	//glGenerateMipmap(GL_TEXTURE_2D); // <- not supported in OpenGL ES!

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR <- not supported in OpenGL ES!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

#define CHECK_GL_ERROR check_gl_error_and_print();

void check_gl_error_and_print()
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		printf("GL error: 0x%04x\n", error);
}

// loop through all pixels of the fbo and compute the minimum/maximum 
unsigned int GetMinMaxFromFBO(float *RenderData, const unsigned int fboSize, unsigned int &count, glm::vec4 &minRGBA, glm::vec4 &maxRGBA )
{
	minRGBA.r = minRGBA.g = minRGBA.b = minRGBA.a = numeric_limits<float>::max();
	maxRGBA.r = maxRGBA.g = maxRGBA.b = maxRGBA.a = numeric_limits<float>::min();
	count = 0;

	for (int i = 0; i < fboSize; i++)
	{
		auto px = glm::vec4(RenderData[i*4],RenderData[(i*4)+1],RenderData[(i*4)+2],RenderData[(i*4)+3]);
		//std::cout << "Px: " << RenderData[i*4] << " " << RenderData[(i*4)+1] << " " << RenderData[(i*4)+2] << " " << RenderData[(i*4)+3] <<std::endl;
		//std::cout << "Px: " << glm::to_string(px).c_str() << std::endl;
		if (px.a > 0)
		{
			px.r /= px.a; px.g /= px.a; px.b /= px.a;
			minRGBA = glm::min(minRGBA, px);
			maxRGBA = glm::max(maxRGBA, px);
			count++;
		}
	}

	return count;

}

// fps related variables:
float fpsDelta = 0.0f;
unsigned long fpsNumberOfFrames = 0;
// updates the FPS in the title, every second
void showFPS(GLFWwindow *pWindow)
{
	// Measure speed
	fpsDelta += deltaTime;
	fpsNumberOfFrames++;
	if(fpsDelta >= 1.0) { // If last cout was more than 1 sec ago
		//cout << 1000.0 / double(nbFrames) << endl;

		double fps = double(fpsNumberOfFrames) / fpsDelta;

		std::stringstream ss;
		ss << APP_NAME <<  " [" << fps << " FPS]";

		glfwSetWindowTitle(pWindow, ss.str().c_str());
		fpsDelta = 0;
		fpsNumberOfFrames = 0;
	}
}

void Lightfield::SetProjectionMatrix()
{

    float near_plane = 0.1f, far_plane = 100.0f;
    //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    imgprojectionmat = glm::perspective(glm::radians(camerafocallength), 1.0f, near_plane, far_plane);
    
	
}

glm::mat4 Lightfield::GetProjectionMatrix()
{
    //glm::mat4 imgProjection;
    //float near_plane = 0.1f, far_plane = 100.0f;
    //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    //imgProjection = glm::perspective(glm::radians(camerafocallength), 1.0f, near_plane, far_plane);
    return imgprojectionmat;
	
}
//////////////////////////Indrajit Edited////////////////////////////////////////
void Lightfield::LoadDemModel(string ObjFile, unsigned int width, unsigned int height, unsigned int nrComponents)
{
    // load the DEM 
    // -----------------------
    dem.Load(ObjFile);
    GLenum format;
	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, this->image_to_bind);
	//glGenerateMipmap(GL_TEXTURE_2D); // <- not supported in OpenGL ES!

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR <- not supported in OpenGL ES!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    demTexture = textureID ;// tell stb_image.h to flip loaded texture's on the y-axis
}

void Lightfield::RendererInitializationPart1()
{
    // glfw: initialize and configure
    // ------------------------------
    if(!glfwInit()){
        std::cout << "Failed to init GLFW" << std::endl;
        return;
    }
        
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	std::stringstream ss;
	ss << APP_NAME << " [loading...]";

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, ss.str().c_str(), NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    int texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    std::cout << texture_units << " texture units available!" << std::endl;
    // https://stackoverflow.com/questions/46426331/number-of-texture-units-gl-texturei-in-opengl-4-implementation-in-visual-studi

    // configure global opengl state
    // -----------------------------
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    // build and compile shaders
    // -------------------------
    //Shader shader("../3.1.3.shadow_mapping.vs", "../3.1.3.shadow_mapping.fs");
    //Shader simpleDepthShader("../3.1.3.shadow_mapping_depth.vs", "../3.1.3.shadow_mapping_depth.fs");
    debugDepthQuad.generate("../show_fbo.vs.glsl", "../show_fbo.fs.glsl");
    projectShader.generate("../project_image.vs.glsl", "../project_image.fs.glsl");
    demShader.generate("../project_image.vs.glsl", "../show_fbo.fs.glsl");
    //Shader demShader("../project_image.vs.glsl", "../project_image.fs.glsl");
    CHECK_GL_ERROR
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float planeVertices[] = {
        // positions            // normals         // texcoords
         50.0f,  50.0f, 0.0f,  0.0f, 1.0f, 0.0f,   1.0f,  0.0f,
        -50.0f,  50.0f, 0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -50.0f, -50.0f, 0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  1.0f,

         50.0f,  50.0f, 0.0f,  0.0f, 1.0f, 0.0f,   1.0f,  0.0f,
        -50.0f, -50.0f, 0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  1.0f,
         50.0f, -50.0f, 0.0f,  0.0f, 1.0f, 0.0f,   1.0f,  1.0f
    };
    // plane VAO
    
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

}

void Lightfield::RendererInitializationPart2()
{
    // configure depth map FBO
    // -----------------------
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECK_GL_ERROR
    //float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); // <- not available in OpenGL ES 2
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0);
    //glDrawBuffers(1, GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // framebuffer data
    //fboData = new glm::vec4[SHADOW_WIDTH*SHADOW_HEIGHT]; // RGBA float
    // shader configuration
    // --------------------
    projectShader.use();
    projectShader.setInt("imageTexture", 0);
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 0);

    // lighting info
    // -------------
    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
    return;
}

void Lightfield::RenderOnce(float *RenderedImage)
{
    //while (!glfwWindowShouldClose(window))
    //{
    // per-frame time logic
        // --------------------
        //float currentFrame = glfwGetTime();
        //deltaTime = currentFrame - lastFrame;
        //lastFrame = currentFrame;
	    //showFPS(window);

        // input
        // -----
        processInput(window);

        // change light position over time
        //lightPos.x = sin(glfwGetTime()) * 3.0f;
        //lightPos.z = cos(glfwGetTime()) * 2.0f;
        //lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    glBlendFunc(GL_ONE, GL_ONE);

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 imgProjection, imgView;
        glm::mat4 imgSpaceMatrix;
	    glm::mat4 projection, view;
        float near_plane = 0.1f, far_plane = 100.0f;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	    //imgProjection = glm::perspective(glm::radians(50.815436217896945f), 1.0f, near_plane, far_plane);
	    imgProjection = GetProjectionMatrix();	
	    // render scene from light's point of view
	    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // enable framebuffer
	    glEnable(GL_BLEND);
	    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	    glBlendFunc(GL_ONE, GL_ONE);
	    projectShader.use();
	    projection = glm::perspective(glm::radians(camera.Zoom), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 0.1f, 1000.0f);
	    view = camera.GetViewMatrix();
	    // set uniforms that do no change
	    projectShader.setMat4("projection", projection);
	    projectShader.setMat4("view", view);
	    //std::cout << "Iterting over Images" << this->GetLFSize() << std::endl;
	    //std::cout << "matrix: " << glm::to_string(this->GetvectorPose(1)) << std::endl;
	    for (int i = 0; i < this->GetLFSize(); i++)
	        {
		        // view i 
		        {
		            
		            imgView = this->GetvectorPose(i);
		            imgSpaceMatrix = imgProjection * imgView;
		        }
			    //std::cout << "Iterting over Images" << i << std::endl;
		        // set uniforms
		        projectShader.setMat4("lightSpaceMatrix", imgSpaceMatrix);
		        glActiveTexture(GL_TEXTURE0);
		        glBindTexture(GL_TEXTURE_2D, this->GetOglTexture(i));
		        //renderScene(projectShader);
		        glm::mat4 model = glm::mat4(1.0f);
		        projectShader.setMat4("model", model);
		        dem.Draw(projectShader);
	        }
	    CHECK_GL_ERROR
	    // read framebuffer to CPU
	    glReadBuffer(GL_COLOR_ATTACHMENT0);
	    glReadPixels(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGBA, GL_FLOAT, RenderedImage);
	    //glReadPixels(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGBA, GL_FLOAT, fboData);
	    // to access a single pixel use indexing like (j)width+i, where j is the row
	    // calculate minimum and maximum of FBO. Note this is very slow!
	    // if it takes too long, do not do this every frame! e.g. only once every second or so
	    //GetMinMaxFromFBO(fboData, SHADOW_WIDTH*SHADOW_HEIGHT, fboCount, fboMin, fboMax);
	    GetMinMaxFromFBO(RenderedImage, SHADOW_WIDTH*SHADOW_HEIGHT, fboCount, fboMin, fboMax);
	    glBindFramebuffer(GL_FRAMEBUFFER, 0); // disable framebuffer
	    // reset viewport
	    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/*
        // 2. render scene as normal using the generated depth/shadow map  
        // --------------------------------------------------------------
	    {
	        auto pos = lf->GetPosition(currView);
	        auto forward = lf->GetForward(currView);
	        auto lookAt = pos + forward;
	        auto up = lf->GetUp(currView);	up = glm::normalize(up); // normalization is important, otherwise lookAt fails!!!

	        imgView = glm::lookAt(pos, lookAt, up);
	        imgSpaceMatrix = imgProjection * imgView;
	    }
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    projectShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
	    projectShader.setMat4("projection", projection);
	    projectShader.setMat4("view", view);
        // set light uniforms
	    projectShader.setVec3("viewPos", camera.Position);
	    projectShader.setVec3("lightPos", lightPos);
	    projectShader.setMat4("lightSpaceMatrix", imgSpaceMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lf->GetOglTexture(currView));
        //renderScene(projectShader);
	    glm::mat4 model = glm::mat4(1.0f);
	    projectShader.setMat4("model", model);
	    dem.Draw(projectShader);
     */

        // display framebuffer
        // ---------------------------------------------
        debugDepthQuad.use();
	    if (fboCount > 0 && fboMax.a > 0)
	        {
		        debugDepthQuad.setFloat("fbo_min", fboMin.r);
		        debugDepthQuad.setFloat("fbo_max", fboMax.r);
	        }
	    else
	        {
		        debugDepthQuad.setFloat("fbo_min", 0.0f);
		        debugDepthQuad.setFloat("fbo_max", 1.0f);
	        }
	    std::cout << "fbo_min: " << glm::to_string(fboMin).c_str() << std::endl;
	    std::cout << "fbo_max: " << glm::to_string(fboMax).c_str() << std::endl;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderQuad();

	    // render DEM
	    // ---------------------------------------------
	    demShader.use();
	    demShader.setFloat("fbo_min", 0.0f);
	    demShader.setFloat("fbo_max", 1.0f);
	    glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, demTexture);
	    glm::mat4 model = glm::mat4(1.0f);
	    demShader.setMat4("model", model);
	    demShader.setMat4("projection", projection);
	    demShader.setMat4("view", view);
	    glBlendColor(0.0f, 0.0f, 0.0f, demAlpha);
	    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	    dem.Draw(demShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    //}
}

void Lightfield::LFRTerminateRenderer()
{
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return;
}
// renders the 3D scene
// --------------------
void renderScene(const Shader &shader)
{
    // plane
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
	/*
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.25));
    shader.setMat4("model", model);
    renderCube();
	*/
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		currView = glm::max( (int)currView - 1, 0);
	//if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		//currView = glm::min( currView + 1, lf->GetSize()-1 );
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

   // camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


/* Look into:
	command line arguments: https://github.com/CLIUtils/CLI11#install 
	OBJ loading: https://github.com/tinyobjloader/tinyobjloader or keep Assimp
	JSON: https://github.com/nlohmann/json#cmake
	Image loading: stb_image does not support TIFF so maybe use FreeImage or SDL

	Try to avoid BOOST (too large and heavy)


*/
