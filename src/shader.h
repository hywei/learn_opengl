#ifndef _SHADER_H_
#define _SHADER_H_

#include <string>

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);

	// use/active this shader
	void use();

	// utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec4f(const std::string& name, float x, float y, float z, float w) const;
	void setMat4fv(const std::string& name, const float* values) const;

public:
	uint32_t ID;
};

#endif
