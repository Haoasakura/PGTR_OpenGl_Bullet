#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "Mesh.h"
#include "Shader.h"

using namespace std;

class Model
{
public:

	Model(const char *path, bool gamma = false);
	~Model();

	void Draw(Shader *shader);

	/*float getMinX();
	float getMaxX();
	float getMinY();
	float getMaxY();
	float getMinZ();
	float getMaxZ();
	float getWidth();
	float getHeight();
	float getDepth();*/

	vector<Texture> textures_loaded;
	vector<Mesh> meshes;

private:

	string directory;
	bool gammaCorrection;

	//float minX, maxX, minY, maxY,minZ,maxZ,width,height,depth;

	void loadModel(const string &path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);

	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
	unsigned int TextureFromFile(const char *path, const string &directory, bool gamma=false);

	//void calculateDimension();

};

