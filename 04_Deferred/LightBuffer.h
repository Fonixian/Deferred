#ifndef LIGHT_BUFFER__H
#define LIGHT_BUFFER__H
#include "LightTypes.h"
#include <vector>
#include <GL/glew.h>

class LightBuffer {
	std::vector<LightInfo> infos;
	GLuint buffer;
	int lastSize;
public:
	LightBuffer();
	~LightBuffer();

	void bind(GLuint base);
	void unbind();

	void addLight(const LightType& type, const LightInfo& info);
	void updateLight(const LightType& type, int index);
	void deleteLight(const LightType& type, int index);

	std::vector<LightInfo>& getInfos();
	int getSize();

};

#endif // !LIGHT_BUFFER__H
