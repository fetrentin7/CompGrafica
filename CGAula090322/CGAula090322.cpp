#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <glew.h>
#include <vector>
#include <GL/glut.h>
#include <string>
#include <chrono>
#include <fstream>
using namespace std;

GLuint v, f;
GLuint p;

GLuint VAO, VBO;

float escala = 0.001f;
float posicaoX,posicaoY = 0.0f;
float rotacaoX, rotacaoZ = 0.0f;
float rotacaoY = 3.2f;

struct Vertice {
	double x, y, z;
};

struct VerticeTextura {
	double x, y;
};

struct Face {
	std::vector<int> verticesIndice;
	std::vector<int> verticesIndiceTextura;
	std::vector<int> verticesIndiceNormal;
};

struct Poligono {
	std::vector<Vertice> vertices;
	std::vector<Vertice> verticesNormal;
	std::vector<VerticeTextura> verticesTextura;
	std::vector<Face> faces;
};

static unsigned int texturas[2];

Poligono objeto;

float intensidadeLuzAmbiente = 0.1;
float intensidadeLuzDifusa = 0.9;
float corLuz[3] = { 1, 1, 1 };
float posLuz[3] = { 0, 1, 0.25 };


struct BitMapFile
{
	int sizeX;
	int sizeY;
	unsigned char* data;
};


Vertice CriaVertice(double pos_x, double pos_y, double pos_z) {
	Vertice novo_v;
	novo_v.x = pos_x;
	novo_v.y = pos_y;
	novo_v.z = pos_z;

	return novo_v;
}

VerticeTextura criaVerticeTextura(double posX, double posY) {
	VerticeTextura vertice;
	vertice.x = posX;
	vertice.y = posY;

	return vertice;
}

std::vector<std::string> trataString(std::string pivo) {

	std::vector<std::string> pivoSplit;
	int inicio = 0;
	int final = pivo.find("/");

	while (final != -1) {
		pivoSplit.push_back(pivo.substr(inicio, final - inicio));
		inicio = final + 1;
		final = pivo.find("/", inicio);
	}

	pivoSplit.push_back(pivo.substr(inicio, final - inicio));

	return pivoSplit;
}

Face criaFace(std::string x, std::string y, std::string z) {

	Face face;
	std::vector<std::string> pivo;

	//vertices
	pivo = trataString(x);
	face.verticesIndice.push_back(abs((pivo[0] == "") ? 0 : std::stoi(pivo[0]) - 1));
	face.verticesIndiceTextura.push_back(abs((pivo[1] == "") ? 0 : std::stoi(pivo[1]) - 1));
	face.verticesIndiceNormal.push_back(abs((pivo[2] == "") ? 0 : std::stoi(pivo[2]) - 1));

	//textura
	pivo = trataString(y);
	face.verticesIndice.push_back(abs((pivo[0] == "") ? 0 : std::stoi(pivo[0]) - 1));
	face.verticesIndiceTextura.push_back(abs((pivo[1] == "") ? 0 : std::stoi(pivo[1]) - 1));
	face.verticesIndiceNormal.push_back(abs((pivo[2] == "") ? 0 : std::stoi(pivo[2]) - 1));

	//normal
	pivo = trataString(z);
	face.verticesIndice.push_back(abs((pivo[0] == "") ? 0 : std::stoi(pivo[0]) - 1));
	face.verticesIndiceTextura.push_back(abs((pivo[1] == "") ? 0 : std::stoi(pivo[1]) - 1));
	face.verticesIndiceNormal.push_back(abs((pivo[2] == "") ? 0 : std::stoi(pivo[2]) - 1));

	return face;
}

Poligono loadObj(std::string fname) {

	Poligono poligono;

	double x, y, z;

	std::ifstream arquivo(fname);

	if (!arquivo.is_open()) {
		std::cout << "Arquivo nao encontrado\n";
		exit(1);
	}
	else {
		std::string tipo;
		while (arquivo >> tipo)
		{
			if (tipo == "v")
			{
				double x, y, z;
				arquivo >> x >> y >> z;
				poligono.vertices.push_back(CriaVertice(x, y, z));
			}

			if (tipo == "f")
			{
				std::string x, y, z;
				arquivo >> x >> y >> z;
				poligono.faces.push_back(criaFace(x, y, z));
			}

			if (tipo == "vn")
			{
				double x, y, z;
				arquivo >> x >> y >> z;
				poligono.verticesNormal.push_back(CriaVertice(x, y, z));
			}

			if (tipo == "vt")
			{
				double x, y;
				arquivo >> x >> y;
				poligono.verticesTextura.push_back(criaVerticeTextura(x, y));
			}

		}
	}

	return poligono;
}


// Funciona somente com bmp de 24 bits
BitMapFile* getBMPData(string filename)
{
	BitMapFile* bmp = new BitMapFile;
	unsigned int size, offset, headerSize;

	// Ler o arquivo de entrada
	ifstream infile(filename.c_str(), ios::binary);

	// Pegar o ponto inicial de leitura
	infile.seekg(10);
	infile.read((char*)&offset, 4);

	// Pegar o tamanho do cabeçalho do bmp.
	infile.read((char*)&headerSize, 4);

	// Pegar a altura e largura da imagem no cabeçalho do bmp.
	infile.seekg(18);
	infile.read((char*)&bmp->sizeX, 4);
	infile.read((char*)&bmp->sizeY, 4);

	// Alocar o buffer para a imagem.
	size = bmp->sizeX * bmp->sizeY * 24;
	bmp->data = new unsigned char[size];

	// Ler a informação da imagem.
	infile.seekg(offset);
	infile.read((char*)bmp->data, size);

	// Reverte a cor de bgr para rgb
	int temp;
	for (int i = 0; i < size; i += 3)
	{
		temp = bmp->data[i];
		bmp->data[i] = bmp->data[i + 2];
		bmp->data[i + 2] = temp;
	}

	return bmp;
}

void loadExternalTextures(std::string nome_arquivo, int id_textura)
{
	BitMapFile* image[1];

	image[0] = getBMPData(nome_arquivo);

	glBindTexture(GL_TEXTURE_2D, texturas[id_textura]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);
}

char* readStringFromFile(char* fn) {

	FILE* fp;
	char* content = NULL;
	int count = 0;

	if (fn != NULL) {
		fopen_s(&fp, fn, "rt");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char*)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}


void setShaders() {

	char* vs = NULL, * fs = NULL, * fs2 = NULL;


	glewInit();

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	char vertex_shader[] = "simples.vert";
	char fragment_shader[] = "simples.frag";
	vs = readStringFromFile(vertex_shader);
	fs = readStringFromFile(fragment_shader);

	const char* vv = vs;
	const char* ff = fs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &ff, NULL);

	free(vs); free(fs);

	glCompileShader(v);
	glCompileShader(f);

	int result;
	glGetShaderiv(v, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int tam;
		glGetShaderiv(v, GL_INFO_LOG_LENGTH, &tam);
		char* mensagem = new char[tam];
		glGetShaderInfoLog(v, tam, &tam, mensagem);
		std::cout << mensagem << std::endl;
	}

	glGetShaderiv(f, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int tam;
		glGetShaderiv(f, GL_INFO_LOG_LENGTH, &tam);
		char* mensagem = new char[tam];
		glGetShaderInfoLog(f, tam, &tam, mensagem);
		std::cout << mensagem << std::endl;
	}

	p = glCreateProgram();
	glAttachShader(p, v);
	glAttachShader(p, f);

	glLinkProgram(p);

	vector<int> verticesIndicePos;

	for (int i = 0; i < objeto.faces.size(); i++) {
		for (int j = 0; j < objeto.faces[i].verticesIndice.size(); j++) {
			verticesIndicePos.push_back(objeto.faces[i].verticesIndice[j]);
		}
	}

	vector<int> verticesIndiceTextura;

	for (int i = 0; i < objeto.faces.size(); i++) {
		for (int j = 0; j < objeto.faces[i].verticesIndiceTextura.size(); j++) {
			verticesIndiceTextura.push_back(objeto.faces[i].verticesIndiceTextura[j]);
		}
	}

	vector<int> verticesIndiceNormal;

	for (int i = 0; i < objeto.faces.size(); i++) {
		for (int j = 0; j < objeto.faces[i].verticesIndiceNormal.size(); j++) {
			verticesIndiceNormal.push_back(objeto.faces[i].verticesIndiceNormal[j]);
		}
	}

	vector<float> pre_vertices;

	for (int i = 0; i < verticesIndicePos.size(); i++) {
		int indiceVerticePos = verticesIndicePos[i];
		int indiceVerticeTextura = verticesIndiceTextura[i];
		int inticeVerticeNormal = verticesIndiceNormal[i];

		pre_vertices.push_back(objeto.vertices[indiceVerticePos].x);
		pre_vertices.push_back(objeto.vertices[indiceVerticePos].y);
		pre_vertices.push_back(objeto.vertices[indiceVerticePos].z);

		pre_vertices.push_back(objeto.verticesTextura[indiceVerticeTextura].x);
		pre_vertices.push_back(objeto.verticesTextura[indiceVerticeTextura].y);

		pre_vertices.push_back(objeto.verticesNormal[inticeVerticeNormal].x);
		pre_vertices.push_back(objeto.verticesNormal[inticeVerticeNormal].y);
		pre_vertices.push_back(objeto.verticesNormal[inticeVerticeNormal].z);
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, pre_vertices.size() * sizeof(float), &pre_vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glUseProgram(p);

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	glTranslatef(0, 0, -10);

	glColor3f(1, 0, 0);
	glPushMatrix();
	glScaled(2, 2, 2);

	int rotX = glGetUniformLocation(p, "anguloRotacaoX");
	glUniform1f(rotX, rotacaoX);

	int rotY = glGetUniformLocation(p, "anguloRotacaoY");
	glUniform1f(rotY, rotacaoY);

	int rotZ = glGetUniformLocation(p, "anguloRotacaoZ");
	glUniform1f(rotZ, rotacaoZ);

	int posix = glGetUniformLocation(p, "posicaoX");
	glUniform1f(posix, posicaoX);

	int posiy = glGetUniformLocation(p, "posicaoY");
	glUniform1f(posiy, posicaoY);

	int scale = glGetUniformLocation(p, "escala");
	glUniform1f(scale, escala);

	int id_int_luz_amb = glGetUniformLocation(p, "luz_int_amb");
	glUniform1f(id_int_luz_amb, intensidadeLuzAmbiente);

	int id_int_luz_dif = glGetUniformLocation(p, "luz_int_dif");
	glUniform1f(id_int_luz_dif, intensidadeLuzDifusa);

	int id_cor_luz = glGetUniformLocation(p, "cor_luz");
	glUniform3f(id_cor_luz, corLuz[0], corLuz[1], corLuz[2]);

	int id_pos_luz = glGetUniformLocation(p, "pos_luz");
	glUniform3f(id_pos_luz, posLuz[0], posLuz[1], posLuz[2]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturas[0]);
	int sp_texture = glGetUniformLocation(p, "textura");
	glUniform1i(sp_texture, 0);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, texturas[1]);
	int sp_texture_normal = glGetUniformLocation(p, "textura_normal");
	glUniform1i(sp_texture_normal, 1);


	glBindTexture(GL_TEXTURE_2D, texturas[1]);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * objeto.faces.size());

	glPopMatrix();

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}


void key(unsigned char k, int x, int y)
{
	//BRILHO DA LUZ 1 E 2
	if (k == '1')
		intensidadeLuzAmbiente -= 0.05;
	else if (k == '2')
		intensidadeLuzAmbiente += 0.05;

	//MOVIMENTO DA LUZ IJKLUO
	if (k == 'l')
		posLuz[0] -= 0.05;
	else if (k == 'j')
		posLuz[0] += 0.05;

	if (k == 'i')
		posLuz[1] += 0.05;
	else if (k == 'k')
		posLuz[1] -= 0.05;

	if (k == 'u')
		posLuz[2] += 0.05;
	else if (k == 'o')
		posLuz[2] -= 0.05;

	//ESCALA X + Z
	if (k == 'x')
		escala += 0.001f;
	else if (k == 'z' && escala > 0.001f)
		escala -= 0.001f;

	//ROTACAO WASDQE
	if (k == 'w')
		rotacaoX += 0.1f;
	else if(k == 's')
		rotacaoX -= 0.1f;

	if (k == 'a')
		rotacaoY -= 0.1f;
	else if (k == 'd')
		rotacaoY += 0.1f;

	if (k == 'q')
		rotacaoZ -= 0.1f;
	else if (k == 'e')
		rotacaoZ += 0.1f;
}

void keyboard_special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		posicaoY += 0.1f;
		break;
	case GLUT_KEY_DOWN:
		posicaoY -= 0.1f;
		break;
	case GLUT_KEY_LEFT:
		posicaoX -= 0.1f;
		break;
	case GLUT_KEY_RIGHT:
		posicaoX += 0.1f;
		break;
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (float)640 / (float)480, 0.1f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void setupRC() {

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat diffuseLight[] = { 0.7, 0.7, 0.7, 1.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glEnable(GL_LIGHT0);

	GLfloat ambientLight[] = { 0.05, 0.05, 0.05, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

	//GLfloat specularLight[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat spectre[] = { 1.0, 1.0, 1.0, 1.0 };

	/*glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);*/
	glMaterialfv(GL_FRONT, GL_SPECULAR, spectre);
	glMateriali(GL_FRONT, GL_SHININESS, 128);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_LIGHTING);
}

void init()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	setupRC();

	glGenTextures(2, texturas);
	loadExternalTextures("Textures/brickwall.bmp", 0);
	loadExternalTextures("Textures/brickwall_normal.bmp", 1);
	glEnable(GL_TEXTURE_2D);


	//comente essa linha para desabilitar os shaders
	setShaders();
}



int main(int argc, char* argv[])
{

	objeto = loadObj("data/mba1.obj");

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Shader");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutSpecialFunc(keyboard_special);

	glutMainLoop();

	return 0;
}