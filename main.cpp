#include <chrono>
#include <cstdio>
#include <fstream>
#include <regex>
#include <vector>
#include <iostream>
#include <filesystem>
#include <GL/gl.h>     // Funções da OpenGL
#include <GL/glu.h>    // Funções da GLU
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h
// interpolação linear

/**
 * TODO
 * - ler arquivo(s) de posições
 * - posicionar objetos de acordo com o arquivo e animar em função do tempo
 * - colisões
 * - controle de uma entidade com o teclado
 */

class Position {
public:
	float x, y;
};

class EntityPosition {
public:
	Position position;
	long unsigned int frame;
};

class Entity {
public:
	bool isControllable = false;
	float rotation = 0;
	Position overridePosition{};
	std::vector<EntityPosition> positions;
};


struct Color {
	float r, g, b;
};

Entity entities[100];
int entities_len = 0;
int paths_current_frame = 0;
float left = 0, right = 2, top = 2, bottom = 0, panX = 0, panY = 0;
using Clock = std::chrono::steady_clock;
auto time_before = Clock::now();
double soma_dt = 0.0;

float entities_size = 0.1;

void drawAxis() {
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1, 1, 1);
	glLineWidth(1);

	glBegin(GL_LINES);
	glVertex2f(left, 0);
	glVertex2f(right, 0);
	glVertex2f(0, top);
	glVertex2f(0, bottom);
	glEnd();

	glPopMatrix();
}

void drawEntity(Entity entity) {
	glColor3f(1, 0, 0); // TODO: melhorar
	glPushMatrix();

	float x, y;
	// if (entity.overridePosition != NULL) {
	// 	x = entity.overridePosition.x;
	// 	y = entity.overridePosition.y;
	// } else {
		x = entity.positions[paths_current_frame].position.x;
		y = entity.positions[paths_current_frame].position.y;
	// }
	glTranslatef(x, y, 0);
	glRotatef(entity.rotation, 0, 0, 1);

	glBegin(GL_QUADS); // TODO: trocar por outra coisa
	glVertex2f(0, 0);
	glVertex2f(entities_size, 0);
	glVertex2f(entities_size, entities_size);
	glVertex2f(0, entities_size);
	glEnd();
	glPopMatrix();
}

void mainDraw() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, right, bottom, top); // TODO
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	for (int i = 0; i < entities_len; i++) {
		drawEntity(entities[i]);
	}

	drawAxis();
	glutSwapBuffers();
}

void animate() {

	/**
	 * FIXME
	/usr/include/c++/15.2.1/bits/stl_vector.h:1263: constexpr std::vector<_Tp, _Alloc>::reference std::vector<_Tp, _Alloc>::operator[](size_type)
	[with _Tp = EntityPosition; _Alloc = std::allocator<EntityPosition>; reference = EntityPosition&; size_type = long unsigned int]: Assertion '__n < this->size()' failed.
	 */

	auto time_now = Clock::now();
	std::chrono::duration<double> dt = time_now - time_before; // segundos em double
	time_before = time_now;

	soma_dt += dt.count();

	if (soma_dt <= (1.0 / 30.0)) {
		return;
	}
	soma_dt = 0.0;
	paths_current_frame++;
	glutPostRedisplay();
}

void initializeEntities() {
	std::string line;
	std::ifstream MyReadFile("../assets/Paths_D.txt"); // FIXME
	if (!MyReadFile.is_open()) {
		std::cerr << "Erro: não foi possível abrir 'assets/Paths_D.txt'. CWD: " << std::filesystem::current_path() << std::endl;
		return;
	}

	if (!getline(MyReadFile, line)) {
		std::cerr << "Aviso: arquivo 'assets/Paths_D.txt' está vazio ou sem a primeira linha esperada. CWD: " << std::filesystem::current_path() << std::endl;
		return;
	}

	int linhas_processadas = 0;
	while (getline(MyReadFile, line)) {
		std::vector<EntityPosition> positions;
		std::regex number_regex("\\d+\\.?\\d*");
		auto numbers_begin = std::sregex_iterator(line.begin(), line.end(), number_regex);
		auto numbers_end = std::sregex_iterator();

		++numbers_begin;

		for (auto i = numbers_begin; i != numbers_end;) {
			EntityPosition pos{};
			pos.position.x = std::stof(i->str()) / 1000;
			++i;
			pos.position.y = std::stof(i->str()) / 1000;
			++i;
			pos.frame = std::stoi(i->str());
			++i;
			positions.push_back(pos);
		}

		Entity entity;
		entity.positions = positions;
		entities[entities_len++] = entity;
		linhas_processadas++;
	}

	if (entities_len == 0) {
		std::cerr << "Aviso: arquivo lido, porém nenhuma entidade foi carregada (linhas processadas: " << linhas_processadas << ")." << std::endl;
	}

	MyReadFile.close();



	// pixel por metro é a primeira info no arquivo de paths
	// dps, cada linha é uma entidade e o 1o nro é a qtd de frames em que ela aparece
	// dps cada valor é uma tupla de x,y,f sendo f o frame
	// o arquivo representa um frame
	// TODO: carrega arquivo
	// inicializa objetos para cada entidade
	// ordena frames
}

void start() {
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, 800, 800);
	glLoadIdentity();
	gluOrtho2D(left, right, bottom, top); // TODO
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char **argv) {
	initializeEntities();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutCreateWindow("CG - T1 - Germano Corrêa");

	glutDisplayFunc(mainDraw);
	glutIdleFunc(animate); // garante a animação contínua
	// glutKeyboardFunc(teclado);
	// glutSpecialFunc(teclasEspeciais);

	start();

	try {
		glutMainLoop();
	} catch (const std::exception &e) {}
}