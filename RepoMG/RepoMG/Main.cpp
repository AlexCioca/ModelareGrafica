#include <filesystem>
#include <vector>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fs = std::filesystem;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
glm::vec3 moveTrain(float &X, float &Y, float &Z);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Brasov to Bucharest", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// build and compile shaders
	// -------------------------
	//Shader shader("default.vs", "default.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");
	Shader trainShader("model.vs", "model.fs");
	Shader terrainShader("model.vs", "model.fs");
	Shader stationShader("model.vs", "model.fs");
	Shader ndStationShader("model.vs", "model.fs");
	Shader bvSignShader("model.vs", "model.fs");
	Shader bucSignShader("model.vs", "model.fs");

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// load textures
	// -------------
	fs::path localPath = fs::current_path();
	std::string textureFolder = localPath.string() + "/Resources/Textures";

	Model tom(localPath.string() + "/Resources/train/asd/0Q1GQ99342K5Y2OJFEP68SLNO.obj");
	Model driverWagon(localPath.string() + "/Resources/train/tren/emd-gp40-2/train.obj");
	Model terrain(localPath.string() + "/Resources/terrain/terrain.obj");
	Model station(localPath.string() + "/Resources/station/milwaukeeroaddepot.obj");
	Model secondStation(localPath.string() + "/Resources/station/milwaukeeroaddepot.obj");
	Model bvSign(localPath.string() + "/Resources/station/bvSign/ExitSign_HiPoly.obj");
	Model bucSign(localPath.string() + "/Resources/station/bucSign/ExitSign_HiPoly.obj");

	std::vector<std::string> faces
	{
		/*textureFolder + "/right.jpg",
		textureFolder + "/left.jpg",
		textureFolder + "/top.jpg",
		textureFolder + "/bottom.jpg",
		textureFolder + "/front.jpg",
		textureFolder + "/back.jpg"*/

		textureFolder + "/_right.jpg",
		textureFolder + "/_left.jpg",
		textureFolder + "/_top.jpg",
		textureFolder + "/_bottom.jpg",
		textureFolder + "/_front.jpg",
		textureFolder + "/_back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// shader configuration
	// --------------------
	//shader.use();
	//shader.setInt("texture1", 0);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	float startX = -265.0f;
	float startY = -17.0f;
	float startZ = 190.0f;

	float moveX = -10.0f;
	float moveY = 0.0f;
	float moveZ = 0.0f;
	float degrees = 0.0f;

	unsigned int key = 0;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		

		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw scene as normal
		stationShader.use();
		trainShader.use();
		terrainShader.use();

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 3000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		trainShader.setMat4("projection", projection);
		trainShader.setMat4("view", view);
		terrainShader.setMat4("projection", projection);
		terrainShader.setMat4("view", view);
		stationShader.setMat4("projection", projection);
		stationShader.setMat4("view", view);

		// render the loaded model
		glm::mat4 train = glm::mat4(1.0f);
		glm::mat4 _terrain = glm::mat4(1.0f);
		glm::mat4 _station = glm::mat4(1.0f);
		glm::mat4 _ndStation = glm::mat4(1.0f);
		glm::mat4 _bvSign = glm::mat4(1.0f);
		glm::mat4 _bucSign = glm::mat4(1.0f);

		train = glm::translate(train, moveTrain(startX, startY, startZ)); // translate it down so it's at the center of the scene
		train = glm::scale(train, glm::vec3(4.3f, 4.3f, 4.3f));	// if it's a bit too big for our scene, scale it down
		train = glm::rotate(train, glm::radians(90.0f), glm::vec3(0, 1, 0));
		trainShader.setMat4("model", train);
		driverWagon.Draw(trainShader);
		//tom.Draw(trainShader);
		
		_terrain = glm::translate(_terrain, glm::vec3(650.0f, -38.0f, -750.0f));
		_terrain = glm::scale(_terrain, glm::vec3(2500.0f, 2500.0f, 2500.0f));
		terrainShader.setMat4("model", _terrain);
		terrain.Draw(terrainShader);

		_station = glm::translate(_station, glm::vec3(-320.0f, -17.0f, 180.0f));
		_station = glm::scale(_station, glm::vec3(0.03f, 0.03f, 0.03f));
		_station = glm::rotate(_station, glm::radians(90.0f), glm::vec3(0, 1, 0));
		stationShader.setMat4("model", _station);
		station.Draw(stationShader);

		_ndStation = glm::translate(_ndStation, glm::vec3(-90.0f, 22.0f, -1860.0f));
		_ndStation = glm::scale(_ndStation, glm::vec3(0.03f, 0.03f, 0.03f));
		_ndStation = glm::rotate(_ndStation, glm::radians(10.0f), glm::vec3(0, 1, 0));
		ndStationShader.setMat4("model", _ndStation);
		secondStation.Draw(ndStationShader);

		_bucSign = glm::translate(_bucSign, glm::vec3(-291.0f, 55.0f, 180.0f));
		_bucSign = glm::scale(_bucSign, glm::vec3(7.0f, 7.0f, 7.0f));
		_bucSign = glm::rotate(_bucSign, glm::radians(90.0f), glm::vec3(0, 1, 0));
		bvSignShader.setMat4("model", _bucSign);
		bvSign.Draw(bvSignShader);

		_bvSign = glm::translate(_bvSign, glm::vec3(-85.0f, 93.5f, -1831.0f));
		//_bvSign = glm::translate(_bvSign, glm::vec3(-300.0f, 40.0f, 180.0f));
		_bvSign = glm::scale(_bvSign, glm::vec3(7.0f, 7.0f, 7.0f));
		_bvSign = glm::rotate(_bvSign, glm::radians(10.0f), glm::vec3(0, 1, 0));
		bucSignShader.setMat4("model", _bvSign);
		bucSign.Draw(bucSignShader);

		//camera.printPosition();

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // driver camera
			key = 1;
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // passanger camera
			key = 2;
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) // 3rd person camera
			key = 3;
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) // free camera
			key = 4;

		switch (key)
		{
		case 1:
			camera.setViewMatrix(glm::vec3(/*moveX +*/ 21.6f, 14.2f, 4.5f));
			break;
		case 2:
			//
			break;
		case 3:
			//
			break;
		case 4:
			//
			break;
		default:
			break;
		}

		// draw skybox as last
	    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();

		//moveX += 0.1f;
		//moveY += 0.005f;
		//degrees += 0.5f;
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	//glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	//glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &skyboxVBO);

	glfwTerminate();
	return 0;
}

glm::vec3 moveTrain(float &X, float &Y, float &Z)
{
	if (X == -265 && Z > 114)
	{
		Z -= 0.1;
	}
	else if (X < -248 && Z > -40)
	{
		X += 0.02;
		Z -= 0.1;
		Y += 0.004;
	}
	else if (X < -214 && Z > -28)
	{
		X += 0.05;
		Z -= 0.082;
		Y += 0.002;
	}
	else if (X < -170 && Z > -82)
	{
		X += 0.06;
		Z -= 0.09;
		Y += 0.002;
	}
	else if (X < -111 && Z > -139)
	{
		X += 0.07;
		Z -= 0.07;
		Y += 0.002;
	}
	else if (X < -54 && Z > -174)
	{
		X += 0.07;
		Z -= 0.055;
		Y += 0.002;
	}
	else if (X < 159 && Z > -248)
	{
		X += 0.09;
		Z -= 0.028;
		Y += 0.003;
	}
	else if (X < 270 && Z > -353)
	{
		X += 0.07;
		Z -= 0.07;
		Y += 0.003;
	}
	else if (X < 303 && Z > -419)
	{
		X += 0.04;
		Z -= 0.07;
		Y += 0.004;
	}

	return glm::vec3(X, Y, Z);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		camera.printPosition();
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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}