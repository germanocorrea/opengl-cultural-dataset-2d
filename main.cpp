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
#include "src/CollisionChecker.h"
#include "src/Entity.h"


std::vector<Entity *> entities;
Entity *mainEntity;
float left = 0, right = 0, top = 0, bottom = 0, panX = 0, panY = 0;
using Clock = std::chrono::steady_clock;
auto time_before = Clock::now();
double soma_dt = 0.0;

float entities_size = 20.0;
static size_t global_max_frames = 0;

void drawEntity(Entity& entity) {
	glColor3f(entity.color.r, entity.color.g, entity.color.b);
	glPushMatrix();

	if (entity.positions.empty() && !entity.isControllable) {
		glPopMatrix();
		return;
	}

	EntityPosition* posToTranslatef = entity.calculateEntityPosition();
	glTranslatef(posToTranslatef->position.x, posToTranslatef->position.y, 0);

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
	std::vector<EntityPosition*> positions;
	for (size_t i = 0; i < entities.size(); i++) {
		entities[i]->updateCurrentFrame();
		positions.push_back(entities[i]->calculateEntityPosition());
	}
	auto collision = new CollisionChecker(positions, entities, entities_size);
	std::vector<EntityPairs *> colliding_entities = collision->getCollidingEntities();
	for (const auto pairs : colliding_entities) {
		Color newColor = {
			.r = static_cast<float>(rand() % 255 / 255.0),
			.g = static_cast<float>(rand() % 255 / 255.0),
			.b = static_cast<float>(rand() % 255 / 255.0),
		};
		pairs->first->color = newColor;
		pairs->second->color = newColor;
	}

	glutPostRedisplay();
	delete collision;
}

void initializeEntities(const std::string& filename) {
	std::string line;
	std::ifstream MyReadFile(filename);
	if (!MyReadFile.is_open()) {
		std::cerr << "Erro: não foi possível abrir '" << filename << "'. CWD: " << std::filesystem::current_path() <<
				std::endl;
		return;
	}

	if (!getline(MyReadFile, line)) {
		std::cerr << "Aviso: arquivo '" << filename << "' está vazio ou sem a primeira linha esperada. CWD: " <<
				std::filesystem::current_path() << std::endl;
		return;
	}

	int biggest_x = 0;
	int biggest_y = 0;
	int lowest_x = 0;
	int lowest_y = 0;

	int entity_id = 0;
	while (getline(MyReadFile, line)) {
		std::vector<EntityPosition*> positions;
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
			int position_x, position_y, frame;
			position_x = std::stoi(i->str());
			++i;
			position_y = std::stoi(i->str());
			++i;
			frame = std::stoi(i->str());
			++i;

			auto pos = new EntityPosition (entity_id, {.x = position_x, .y = position_y});
			positions.push_back(pos);

			if (position_x > biggest_x) {
				biggest_x = position_x;
			}
			if (position_y > biggest_y) {
				biggest_y = position_y;
			}

			if (position_x < lowest_x) {
				lowest_x = position_x;
			}
			if (position_y < lowest_y) {
				lowest_y = position_y;
			}
		}

		auto entity = new Entity(entity_id++, positions, static_cast<int>(frames_count));
		entities.push_back(entity);
	}
	mainEntity = new Entity(entity_id, true, {
		.x = (biggest_x - lowest_x) / 2,
		.y = (biggest_y - lowest_y) / 2,
	});
	entities.push_back(mainEntity);


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
	gluOrtho2D(left, right, bottom, top);
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
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutCreateWindow("CG - T1 - Germano Bruscato Corrêa");

	std::string filename = argv[1];
	initializeEntities(filename);
	glutDisplayFunc(mainDraw);
	glutIdleFunc(animate);
	glutKeyboardFunc(teclado);
	glutSpecialFunc(teclasEspeciais);

	start();

	try {
		glutMainLoop();
	} catch (const std::exception &e) {
	}
	for (auto entity : entities) {
		delete entity;
	}
	return 0;
}
