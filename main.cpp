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
// TODO: interpolacao no main entity, ao mover add frames "para frente"
// TODO: refazer codigo, SOLID
// TODO: permitir duas direcoes ao mesmo tempo

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
	Position interpolateWith(const EntityPosition& other, const float t) {
		return {
			.x = interpolate(position.x, other.position.x, t),
			.y = interpolate(position.y, other.position.y, t),
		};
	}
private:
	int interpolate(const int a, const int b, const float t) {
		return a + (b - a) * t;
	}
};

class Entity {
public:
	bool isControllable = false;
	float rotation = 0;
	Position overridePosition{};
	std::vector<EntityPosition> positions;
	Color color = {0.2, 0.8, 0};
	int max_frames = 0;
	float current_entity_frame = 0;
	bool going_backwards = false;

	void updateCurrentFrame() {
		if ((going_backwards && (current_entity_frame > 0)) || current_entity_frame >= max_frames) {
			going_backwards = true;
			current_entity_frame -= 0.1;
		} else {
			going_backwards = false;
			current_entity_frame += 0.1;
		}
	}


	Position getEntityPosition() {
		if (isControllable) {
			return {
				.x = overridePosition.x,
				.y = overridePosition.y,
			};
		}

		const size_t floored_frame = normalizeIdx(std::floor(current_entity_frame), positions.size());
		const size_t ceiled_frame = normalizeIdx(std::ceil(current_entity_frame), positions.size());
		if (current_entity_frame - floored_frame > 0.0) {
			return positions[floored_frame].interpolateWith(
				positions[ceiled_frame],
				current_entity_frame - floored_frame
			);
		}

		const size_t idx = normalizeIdx(current_entity_frame, positions.size());
		return positions[idx].position;
	}
private:
	size_t normalizeIdx(const int idx, const int count) {
		return std::min<size_t>(static_cast<size_t>(idx), count - 1);
	}
};


std::vector<std::unique_ptr<Entity>> entities;
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

	Position posToTranslatef = entity.getEntityPosition();
	glTranslatef(posToTranslatef.x, posToTranslatef.y, 0);
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
	for (size_t i = 0; i < entities.size(); i++) {
		entities[i]->updateCurrentFrame();
	}

	glutPostRedisplay();
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
				.positions = positions,
				.max_frames = static_cast<int>(frames_count),
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
	glutCreateWindow("CG - T1 - Germano Corrêa");

	std::string filename = argv[1];
	initializeEntities(filename);
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
