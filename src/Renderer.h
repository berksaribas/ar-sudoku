#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

struct SquareData {
	int x, y;
	int width, height;
	int number;
	bool is_provided;
	bool is_visible;
	SquareData(int x_, int y_, int width_, int height_, int number_, bool draw_, bool is_visible_) {
		x = x_;
		y = y_;
		width = width_;
		height = height_;
		number = number_;
		is_provided = draw_;
		is_visible = is_visible_;
	}
	SquareData() {}
};

class Renderer {
public:
	void init(int width, int height, float fov);
    void render(const cv::Mat& image, glm::mat3 transformation_matrix, SquareData* data,int cubePos); 

	inline GLFWwindow* getWindow() { return window; };

private: 
	GLFWwindow* window;
	int width;
	int height;
	GLuint number_textures[9];
};
