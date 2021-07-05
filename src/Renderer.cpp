#include "Renderer.h"
#include "UIButton.h"

static GLuint matToTexture(const cv::Mat& mat) {
	// Generate a number for our textureID's unique handle
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_RECTANGLE, textureID);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set incoming texture format to:
	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	// Work out other mappings as required ( there's a list in comments in main() )
	GLenum inputColourFormat = GL_BGR;
	if (mat.channels() == 1)
	{
		inputColourFormat = GL_LUMINANCE;
	}

	// Create the texture
	glTexImage2D(GL_TEXTURE_RECTANGLE,     // Type of texture
		0,                 // Pyramid level (for mip-mapping) - 0 is the top level
		GL_RGB,            // Internal colour format to convert to
		mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
		mat.rows,          // Image height i.e. 480 for Kinect in standard mode
		0,                 // Border width in pixels (can either be 1 or 0)
		inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		GL_UNSIGNED_BYTE,  // Image data type
		mat.ptr());        // The actual image data itself

	return textureID;
}

void Renderer::render(const cv::Mat& image, glm::mat3 transformation_matrix, SquareData* data, int cubePos, UIButton** a_pARButton)
{
	glViewport(0, 0, width, height); // use a screen size of WIDTH x HEIGHT

	// Clear color and depth buffers
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the window

	//Rendering the view
	{
		glMatrixMode(GL_PROJECTION);     // Make a simple 2D projection on the entire window
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, width, height, 0.0, 0.0, 100.0);

		glMatrixMode(GL_MODELVIEW);    // Set the matrix mode to object modeling
		glPushMatrix();
		glLoadIdentity();

		glEnable(GL_TEXTURE_RECTANGLE);
		GLuint image_tex = matToTexture(image);
		glBindTexture(GL_TEXTURE_RECTANGLE, image_tex);

		/* Draw a quad */
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
		glTexCoord2f(0, image.rows); glVertex3f(0, height, 0);
		glTexCoord2f(image.cols, image.rows); glVertex3f(width, height, 0);
		glTexCoord2f(image.cols, 0); glVertex3f(width, 0, 0);
		glEnd();

		glDeleteTextures(1, &image_tex);
		glDisable(GL_TEXTURE_RECTANGLE);
		
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	//Rendering each number
	if (data != nullptr) {
		for (int i = 0; i < 81; i++)
		{

			glMatrixMode(GL_PROJECTION);     // Make a simple 2D projection on the entire window
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0, width, height, 0.0, 0.0, 100.0);

			glMatrixMode(GL_MODELVIEW);    // Set the matrix mode to object modeling
			glPushMatrix();
			glLoadIdentity();

			glEnable(GL_TEXTURE_RECTANGLE);
			glBindTexture(GL_TEXTURE_RECTANGLE, number_textures[data[i].number - 1]);

			auto result1 = glm::vec3(-0.5 + data[i].x, -0.5 + data[i].y, 1.0f) * transformation_matrix;
			auto result2 = glm::vec3(-0.5 + data[i].x + data[i].width, -0.5 + data[i].y, 1.0f) * transformation_matrix;
			auto result3 = glm::vec3(-0.5 + data[i].x + data[i].width, -0.5 + data[i].y + data[i].height, 1.0f) * transformation_matrix;
			auto result4 = glm::vec3(-0.5 + data[i].x, -0.5 + data[i].y + data[i].height, 1.0f) * transformation_matrix;

			result1 /= result1.z;
			result2 /= result2.z;
			result3 /= result3.z;
			result4 /= result4.z;

			if (i == cubePos)
			{
				// here is the code for desinging the navigator
				glBegin(GL_POLYGON);
				//glClearColor(0.5, 0.5, 0.5, 1.0);
				glColor3f(1.f, 0.f, 1.f);
				glVertex3f(result1.x, result1.y - 20.f, 0.0f);
				glVertex3f(result2.x, result2.y - 20.f, 0.0f);
				glVertex3f((result1.x + result2.x) / 2, result2.y, 0.0f);
				glColor3f(1.f, 1.f, 1.f);
				glEnd();
			}
			if (!data[i].is_visible) {
				continue;
			}

			if (data[i].is_provided) {
				result1 = glm::vec3(-0.5 + data[i].x, -0.5 + data[i].y, 1.0f) * transformation_matrix;
				result2 = glm::vec3(-0.5 + data[i].x + data[i].width / 2, -0.5 + data[i].y, 1.0f) * transformation_matrix;
				result3 = glm::vec3(-0.5 + data[i].x + data[i].width / 2, -0.5 + data[i].y + data[i].height / 2, 1.0f) * transformation_matrix;
				result4 = glm::vec3(-0.5 + data[i].x, -0.5 + data[i].y + data[i].height / 2, 1.0f) * transformation_matrix;

				result1 /= result1.z;
				result2 /= result2.z;
				result3 /= result3.z;
				result4 /= result4.z;
			}

			/* Draw a quad */
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex3f(result1.x, result1.y, 0.0f);
			glTexCoord2f(64, 0); glVertex3f(result2.x, result2.y, 0.0f);
			glTexCoord2f(64, 64); glVertex3f(result3.x, result3.y, 0.0f);
			glTexCoord2f(0, 64); glVertex3f(result4.x, result4.y, 0.0f);
			glEnd();

			glDisable(GL_TEXTURE_RECTANGLE);

			glPopMatrix();

			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
		}
	}

	//Render Buttons
	{
		glMatrixMode(GL_PROJECTION);     // Make a simple 2D projection on the entire window
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, width, height, 0.0, 0.0, 100.0);

		glMatrixMode(GL_MODELVIEW);    // Set the matrix mode to object modeling
		glPushMatrix();
		glLoadIdentity();
		for (int l_iBtnIndex = 0; l_iBtnIndex < 4; l_iBtnIndex++)
		{
			UIButton& l_btnCurrent = *a_pARButton[l_iBtnIndex];
			if (!l_btnCurrent.is_visible) {
				continue;
			}

			glEnable(GL_TEXTURE_RECTANGLE);
			glBindTexture(GL_TEXTURE_RECTANGLE, btn_textures[(l_btnCurrent.m_BtnType * BTN_CONDITION::BTN_HIDDEN) + l_btnCurrent.m_BtnCondition]);

			const float l_fWidthDiv2 = l_btnCurrent.width * 0.5f;
			const float l_fHeightDiv2 = l_btnCurrent.height * 0.5f;
			auto result1 = glm::vec3(l_btnCurrent.x - l_fWidthDiv2, l_btnCurrent.y - l_fHeightDiv2, 1.0f);// *transformation_matrix;
			auto result2 = glm::vec3(l_btnCurrent.x + l_fWidthDiv2, l_btnCurrent.y - l_fHeightDiv2, 1.0f);// *transformation_matrix;
			auto result3 = glm::vec3(l_btnCurrent.x + l_fWidthDiv2, l_btnCurrent.y + l_fHeightDiv2, 1.0f);// *transformation_matrix;
			auto result4 = glm::vec3(l_btnCurrent.x - l_fWidthDiv2, l_btnCurrent.y + l_fHeightDiv2, 1.0f);// *transformation_matrix;

			/* Draw a quad */
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex3f(result1.x, result1.y, 0.0f);
			glTexCoord2f(195, 0); glVertex3f(result2.x, result2.y, 0.0f);
			glTexCoord2f(195, 60); glVertex3f(result3.x, result3.y, 0.0f);
			glTexCoord2f(0, 60); glVertex3f(result4.x, result4.y, 0.0f);
			glEnd();

			glDisable(GL_TEXTURE_RECTANGLE);
		}
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
		
	// Swap front and back buffers
	glfwSwapBuffers(window);
	// Poll for and process events
	glfwPollEvents();
}

void Renderer::init(int width_, int height_, float fov)
{
	//Init glfw
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	//OpenGL version
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	window = glfwCreateWindow(width_, height_, "OpenGL Sudoku Renderer", NULL, NULL);
	width = width_;
	height = height_;

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	//  Initialise glew (must occur AFTER window creation or glew will error)
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		exit(-1);
	}

	// Create a perspective projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// The camera should be calibrated -> a calibration results in the projection matrix -> then load the matrix
	// -> into GL_PROJECTION
	// -> adjustment of FOV is needed for each camera
	float ratio = (GLfloat)width / (GLfloat)height;

	float near = 0.01f, far = 100.f;
	float top = tan((double)(fov * M_PI / 360.0f)) * near;
	float bottom = -top;
	float left = ratio * bottom;
	float right = ratio * top;
	glFrustum(left, right, bottom, top, near, far);

	//Generate textures for numbers
	for (int i = 0; i < 9; i++) {
        
        //number_textures[i] = matToTexture(cv::imread("/Users/yangzonglin/Ar_project/data/" + std::to_string(i + 1) + ".jpg")); //for Mac path
		number_textures[i] = matToTexture(cv::imread("../data/" + std::to_string(i + 1) + ".jpg"));
	}

	//Generate textures for the buttons
	for (int i = 0; i < 12; i++) {

		btn_textures[i] = matToTexture(cv::imread("../Art/" + std::to_string(i / 3) + "_" + std::to_string(i%3) + ".png"));
	}

	printf("%s\n", glGetString(GL_VERSION));
}
