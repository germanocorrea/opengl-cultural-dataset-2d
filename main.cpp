#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <regex>
#include <vector>
#include <memory>
#include <iostream>
#include <filesystem>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// TODO: implementar colisoes
// TODO: aumentar velocidade da entidade controlada por mouse
// TODO: permitir duas direcoes ao mesmo tempo
// TODO: entender ou mudar interpolacao linear usada
// TODO: refazer codigo, SOLID

struct Color {
	float r, g, b;
};

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
	Color color = {0.2, 0.8, 0};
};



std::vector<std::unique_ptr<Entity>> entities;
Entity *mainEntity;
int entities_len = 0;
float paths_current_frame = 0;
float left = 0, right = 0, top = 0, bottom = 0, panX = 0, panY = 0;
using Clock = std::chrono::steady_clock;
auto time_before = Clock::now();
double soma_dt = 0.0;

float entities_size = 20.0;
static size_t global_max_frames = 0;

size_t normalizeIdx(const int idx, const int count) {
	return std::min<size_t>(static_cast<size_t>(idx), count - 1);
}

void drawEntity(Entity& entity) {
	glColor3f(entity.color.r, entity.color.g, entity.color.b);
	glPushMatrix();

	if (entity.positions.empty() && !entity.isControllable) {
		glPopMatrix();
		return;
	}

	float x, y;
	if (entity.isControllable) {
		x = entity.overridePosition.x;
		y = entity.overridePosition.y;
	} else {
		const size_t floored_frame = normalizeIdx(std::floor(paths_current_frame), entity.positions.size());
		const size_t ceiled_frame = normalizeIdx(std::ceil(paths_current_frame), entity.positions.size());
		if (paths_current_frame - floored_frame > 0.0) {
			x = entity.positions[floored_frame].position.x + (entity.positions[ceiled_frame].position.x - entity.positions[floored_frame].position.x) * (paths_current_frame - floored_frame);
			y = entity.positions[floored_frame].position.y + (entity.positions[ceiled_frame].position.y - entity.positions[floored_frame].position.y) * (paths_current_frame - floored_frame);
		} else {
			const size_t idx = normalizeIdx(paths_current_frame, entity.positions.size());
			x = entity.positions[idx].position.x;
			y = entity.positions[idx].position.y;
		}

	}
	glTranslatef(x, y, 0);
	glRotatef(entity.rotation, 0, 0, 1);

	glBegin(GL_QUADS);
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
	gluOrtho2D(left, right, bottom, top);
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	for (size_t i = 0; i < entities.size(); i++) {
		drawEntity(*entities[i]);
	}

	// drawAxis();
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

	if (global_max_frames == 0) {
		return;
	}

	paths_current_frame += 0.1;

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

		entities.push_back(
			std::make_unique<Entity>(Entity{
				.positions = positions
			})
		);
	}
	auto mainEntityPtr = std::make_unique<Entity>(Entity{
		.isControllable = true,
		.rotation = 0,
		.overridePosition = {
			.x = (biggest_x - lowest_x) / 2,
			.y = (biggest_y - lowest_y) / 2,
		},
	});
	mainEntity = mainEntityPtr.get();
	entities.push_back(std::move(mainEntityPtr));


	if (entities.empty()) {
		std::cerr << "Aviso: arquivo lido, porém nenhuma entidade foi carregada." << std::endl;
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

void teclado(unsigned char key, int x, int y) {
	if (key == 27) {
		exit(0);
	}
}

void teclasEspeciais(int key, int x, int y) {
	int movement = 10;
	switch (key) {
		case GLUT_KEY_LEFT:
			mainEntity->overridePosition.x -= movement;
			break;
		case GLUT_KEY_RIGHT:
			mainEntity->overridePosition.x += movement;
			break;
		case GLUT_KEY_UP:
			mainEntity->overridePosition.y += movement;
			break;
		case GLUT_KEY_DOWN:
			mainEntity->overridePosition.y -= movement;
			break;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	initializeEntities();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutCreateWindow("CG - T1 - Germano Corrêa");

	glutDisplayFunc(mainDraw);
	glutIdleFunc(animate); // garante a animação contínua
	glutKeyboardFunc(teclado);
	glutSpecialFunc(teclasEspeciais);

	start();

	try {
		glutMainLoop();
	} catch (const std::exception &e) {
	}
}
