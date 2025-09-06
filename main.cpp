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
	int frame;
};

class Entity {
public:
	bool isControllable = false;
	float rotation = 0;
	Position overridePosition;
	std::vector<EntityPosition> positions;
};


struct Color {
	float r, g, b;
};

Entity entities[100];
int entities_len = 0;
int paths_current_frame = 0;
float left = 0, right = 0, top = 0, bottom = 0, panX = 0, panY = 0;
using Clock = std::chrono::steady_clock;
auto time_before = Clock::now();
double soma_dt = 0.0;

void drawEntity(Entity entity) {
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
	glVertex2f(100, 0);
	glVertex2f(100, 100);
	glVertex2f(0, 100);
	glEnd();
	glPopMatrix();
}

void mainDraw() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 0, 0, 0); // TODO
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	for (int i = 0; i < entities_len; i++) {
		drawEntity(entities[i]);
	}

	glutSwapBuffers();
}

void animate() {
	auto time_now = Clock::now();
	std::chrono::duration<double> dt = time_now - time_before; // segundos em double
	time_before = time_now;

	soma_dt += dt.count();

	if (soma_dt <= (1.0 / 60.0)) {
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
			pos.position.x = std::stof(i->str());
			++i;
			pos.position.y = std::stof(i->str());
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

	// start();

	try {
		glutMainLoop();
	} catch (const std::exception &e) {}
}