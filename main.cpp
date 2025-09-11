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
	int x, y;
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
float left = 0, right = 0, top = 0, bottom = 0, panX = 0, panY = 0;
using Clock = std::chrono::steady_clock;
auto time_before = Clock::now();
double soma_dt = 0.0;

float entities_size = 20.0;
// Mantém o maior número de frames entre as entidades carregadas
static size_t global_max_frames = 0;

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

	// Evita acesso fora do vetor
	if (entity.positions.empty()) {
		glPopMatrix();
		return;
	}

	const size_t idx = std::min<size_t>(static_cast<size_t>(paths_current_frame), entity.positions.size() - 1);
	float x = entity.positions[idx].position.x;
	float y = entity.positions[idx].position.y;
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

	// drawAxis();
	glutSwapBuffers();
}

void animate() {
	auto time_now = Clock::now();
	std::chrono::duration<double> dt = time_now - time_before; // segundos em double
	time_before = time_now;

	soma_dt += dt.count();

	if (soma_dt <= (1.0 / 5.0)) {
		return;
	}
	soma_dt = 0.0;

	if (global_max_frames == 0) {
		return;
	}

	paths_current_frame = (paths_current_frame + 1) % static_cast<int>(global_max_frames);

	glutPostRedisplay();
}

void initializeEntities() {
	std::string line;
	std::ifstream MyReadFile("../assets/Paths_D.txt"); // FIXME
	if (!MyReadFile.is_open()) {
		std::cerr << "Erro: não foi possível abrir 'assets/Paths_D.txt'. CWD: " << std::filesystem::current_path() <<
				std::endl;
		return;
	}

	if (!getline(MyReadFile, line)) {
		std::cerr << "Aviso: arquivo 'assets/Paths_D.txt' está vazio ou sem a primeira linha esperada. CWD: " <<
				std::filesystem::current_path() << std::endl;
		return;
	}

	int biggest_x = 0;
	int biggest_y = 0;
	int lowest_x = 0;
	int lowest_y = 0;

	int linhas_processadas = 0;
	while (getline(MyReadFile, line)) {
		std::vector<EntityPosition> positions;
		std::regex number_regex("\\d+\\.?\\d*");
		auto numbers_begin = std::sregex_iterator(line.begin(), line.end(), number_regex);
		auto numbers_end = std::sregex_iterator();

		size_t frames_count = 0;
		if (numbers_begin != numbers_end) {
			frames_count = std::stoul(numbers_begin->str());
			if (frames_count > global_max_frames) {
				global_max_frames = frames_count;
			}
			++numbers_begin;
		}

		for (auto i = numbers_begin; i != numbers_end;) {
			EntityPosition pos{};
			pos.position.x = std::stoi(i->str());
			++i;
			pos.position.y = std::stoi(i->str());
			++i;
			pos.frame = std::stoi(i->str());
			++i;
			positions.push_back(pos);

			if (pos.position.x > biggest_x) {
				biggest_x = pos.position.x;
			}
			if (pos.position.y > biggest_y) {
				biggest_y = pos.position.y;
			}

			if (pos.position.x < lowest_x) {
				lowest_x = pos.position.x;
			}
			if (pos.position.y < lowest_y) {
				lowest_y = pos.position.y;
			}
		}

		Entity entity;
		entity.positions = positions;
		entities[entities_len++] = entity;
		linhas_processadas++;
	}

	if (entities_len == 0) {
		std::cerr << "Aviso: arquivo lido, porém nenhuma entidade foi carregada (linhas processadas: " <<
				linhas_processadas << ")." << std::endl;
	}

	MyReadFile.close();

	right = biggest_x;
	top = biggest_y;
	left = lowest_x;
	bottom = lowest_y;
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
	} catch (const std::exception &e) {
	}
}
