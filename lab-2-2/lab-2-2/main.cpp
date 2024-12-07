#include <stdlib.h>
#include <stdio.h>


#include "freeglut.h"
#include "freeglut_ext.h"


#include "CameraMotionHelper.h"
#include "ModelDescription.h"


#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/type_ptr.hpp"


using namespace glm;



constexpr auto ESCAPE = '\033';

//начальное разрешенрие экрана
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

//объект-помощник для поворота сценыс помощью курсора мыши 
CameraMotionHelper cameraMotionHelper;

//значение угла для анимации вращения модели
GLfloat animationAngle = 0.0f;

void printExtList(const char* extensions)
{
	char name[1024];
	int i = 0, j = 0;

	printf("Supported extensions:\n");

	for (i = 0, j = 0; extensions[i] != '\0'; i++)
	{
		if (!iswspace(extensions[i]))
			name[j++] = extensions[i];
		else
		{
			name[j] = '\0';
			printf("\t%s\n", name);
			j = 0;
		}
	}
	if (j > 0)
	{
		name[j] = '\0';
		printf("\t%s\n", name);
	}
}

//метод настройки источников освещения
void setutLight()
{
	//включение освещения
	glEnable(GL_LIGHTING);
	//включение точечного источника света 0 (нулевой лампы
	//всего может быть 8 источников света
	glEnable(GL_LIGHT0);

	//установка фоновой освещенности (белый цвет)
	GLfloat ambientLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	//настройка точечного источника света
	//в модели освещения фонга свет представляется в виде трех составляющих
	//зеркальная (отвечает за блики)
	GLfloat specularLight[] = { 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

	//фоновая (равномерный серый цвет
	GLfloat ambientLight0[] = { 0.3f, 0.3f, 0.3f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight0);

	//рассеяная (белый цвет)
	GLfloat diffuseLight[] = { 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
}

void init()
{
	//установка общего фона (черный цвет
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//активирование функции глубины
	glEnable(GL_DEPTH_TEST);
	//автоматическое приведение нормалей к единичной длине
	//затратная процедура, нужна при изменении размеров модели и для правильного расчета освещения
	glEnable(GL_NORMALIZE);
	//отключаем задание материалов объектов одним параметром glColor3f
	//для задания материала объектов мы будем использовать команду glMaterialfv
	glDisable(GL_COLOR_MATERIAL);
	//настройка источников освещения
	setutLight();
}
//метод прорисовки координатных осей
//параметр задает размер осей
void axis(GLfloat size)
{
	//включаем задание материалов объектов одним параметром glColor3f для непосредственного задания цвета линий
	glEnable(GL_COLOR_MATERIAL);
	//сбрасываем настройки материала, чтобы материал не оказывал воздействия на цвет линий
	GLfloat emissionColor[] = { 0.0, 0.0, 0.0, 1.0f };
	glMaterialfv(GL_FRONT, GL_EMISSION, emissionColor);
	//устанавливаем толщину линий
	glLineWidth(2.0f);
	//прорисовка оси X с помощью отрезка
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-size, 0.0f, 0.0f);
	glEnd();
	//прорисовка названия начала оси X - "-X"
	glRasterPos3f(-size, 0.0f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'+');
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'X');

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, size, 0.0f);
	glVertex3f(0.0f, -size, 0.0f);
	glEnd;

	glRasterPos3f(0.0f, -size, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'-');
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'Y');

	glRasterPos3f(0.0f, size, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'+');
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'Y');

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, size);
	glVertex3f(0.0f, 0.0f, -size);
	glEnd();

	glRasterPos3f(0.0f, 0.0f, -size);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'-');
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'Z');

	glRasterPos3f(0.0f, 0.0f, size);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'+');
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)'Z');

	//отключаем использование непосредственного задания цвета
	glDisable(GL_COLOR_MATERIAL);
}
//метод прорисовки модели "лампочки"
void renderLamp()
{
	//когда речь идет о свойствах материала в приложении к освещению, то имеется ввиду его способность воспринимать каждую из трех компонент цвета составляющей освещенности
	//собственное свечение объекта "лампочка" светится белым цветом
	GLfloat emissionColor[] = { 1.0f,1.0f,1.0f,1.0f };

	//остальные составляющие материала оставляем "черными", така как они в нашем случае не влияют на конечный (белый) цвет модели
	GLfloat ambientColor[] = { 0.0f,0.0f,0.0f,1.0f };
	GLfloat diffuseColor[] = { 0.0f,0.0f,0.0f,1.0f };
	GLfloat specularColor[] = { 0.0f,0.0f,0.0f,1.0f };

	float shinines[] = { 20 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularColor);
	glMaterialfv(GL_FRONT, GL_EMISSION, emissionColor);
	glMaterialfv(GL_FRONT, GL_SHININESS, shinines);

	//в библиотеке GLUT существует ряд методов для прорисовки простых примитивов в данном случае сферы
	glutSolidSphere(0.2f, 32, 32);
}
//метод отрисовки модели
void renderModel()
{
	//устанавливаем материал модели "бронза"ю собсственное сечение материала наш объект не имеет собственного сечения
	GLfloat emissionColor[] = { 0.0f,0.0f,0.0f,1.0f };
	glMaterialfv(GL_FRONT, GL_EMISSION, emissionColor);

	//фоновое освещение материала (цвет тени)
	GLfloat ambientColor[] = { 0.2125f,0.1275f,0.054f,1.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientColor);

	//рассеяный свет от материала
	GLfloat diffuseColor[] = { 0.714f,0.4284f,0.18144f,1.0f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

	//бликовая (зеркальная) составляющая
	GLfloat specularColor[] = { 0.393548f,0.271906f,0.166721f,1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularColor);

	//целое или вещественное значение в диапазоне от 0 до 128, которое определяет степень зеркального тражения материала
	float gl_shininess[] = { 128 * 0.2f };
	glMaterialfv(GL_FRONT, GL_SHININESS, gl_shininess);

	//непосредственно прорисовка объекта
	//зададим массив нормалей для каждой вершины
	glEnableClientState(GL_VERTEX_ARRAY);
	glNormalPointer(GL_FLOAT, 0, normalArr);

	//зададим массив вершин модели
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertexArr);

	//glDrawArray метод для прорисовки полигонов по заданному алгоритму
	//(GL_TRIANGLES) согласно массиву вершин и нормалей заданных ранее
	glDrawArrays(GL_TRIANGLES, 0, 36);

	//методы glDisableClientState вызываются в конце прорисовки модели для каждого из использованных массивов данных 
	//мы испоьзовали массив вершин и массив нормалей
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

}
//функция управляет всем выводом на экран
void display()
{
	//команда glClear очищает буферы, а параметр buf определяет буферы, которые нужно очистить. типичная программа вызывает команду glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) для очистки буферов цвета и глубины
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//выбираем видовую матрицу
	glMatrixMode(GL_MODELVIEW);

	//сбрасываем матрицу
	glLoadIdentity();

	//устанавливаем наблюдателя относительно нулевой координаты в пространстве. положение наблюдателя получаем от объекта-помощника
	gluLookAt(cameraMotionHelper.getDistance(),
		cameraMotionHelper.getDistance(),
		cameraMotionHelper.getDistance(),
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
	//поворачиваем всю наблюдаемую систему по вертикали (управление сценой курсором мыши)
	//угол поворота получаем от объекта-помощника
	glRotatef(cameraMotionHelper.getRotationXZ(), 1.0f, 0.0f, -1.0f);

	//поворачиваем всю наблюдаемую систему по горизонтали (управление сценой курсором мыши) угол поворота получаем от объекта-помощника
	glRotatef(cameraMotionHelper.getrotationY(), 0.0f, 1.0f, 0.0f);
	//управление положением источника света
	//сохраняем текущую видовую матрицу в стек
	glPushMatrix();
	//установка поиции источника освещения
	GLfloat lightPos[] = { 5.0f,5.0f,0.0f,1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	//установка позиции модели "лампочки"
	glTranslatef(lightPos[0], lightPos[1], lightPos[2]);

	//прорисовка "лампочки"
	renderLamp();
	//извлекаем видовую матрицу из стека
	glPopMatrix();
	//прорисовка глобальной системы координат
	axis(15.0f);

	//смещение модели относительно глобальной оси
	glTranslatef(2.0f, 2.0f, -2.0f);
	//поворот модели на задний угол относительно осей X и Z
	glRotatef(-animationAngle, 1.0f, 0.0f, 1.0f);
	//прорисовка собственной системы координат модели
	axis(5.0f);
	//изменение размера модели. преобразование выполняем после прорисовки собственной системы координат
	glScalef(0.5f, 1.5f, 1.5f);
	//прорисовка модели 
	renderModel();
	//команда смены кадров (для двойной буферизации)
	glutSwapBuffers();

}
//функция вызывается при изменении размеров окна

void reshape(GLint w, GLint h)
{
	//устанавливаем размеры области отображения
	glViewport(0, 0, w, h);
	//выбираем для настройки матрицу проекций
	glMatrixMode(GL_PROJECTION);
	//сбросим выбранную матрицу (матрице проекций). заменяет текущую матрицу на единичную
	glLoadIdentity();
	//настройка перспективной проекции
	GLfloat zNear = 1;
	GLfloat zFar = 200;
	//параметры zFar и zNear задают расстояние от наблюдателя до плоскостей отсечения по глубине и должны быть положительными
	gluPerspective(45.0, (float)w / (float)h, zNear, zFar);
}
//функция обрабатывает сообщения от клавиатуры
void keyboardFunc(unsigned char key, int x, int y)
{
	if (key == ESCAPE) exit(0);
}
//метод получения событий таймера. используем для реализации анимаций
void updateAnimation(int value)
{
	//для анимации вращения модели изменяем угол вращения по таймеру
	animationAngle += 1.0f;
	if (animationAngle > 360)
		animationAngle -= 360;
	//посылаем сообщение для перерисовки сцены
	glutPostRedisplay();
	//запускаем таймер заново
	glutTimerFunc(25, updateAnimation, 0);
}

//метод полученрия сообщений от мыши (нажатие клавиш). будем использовать его для управления отображения сцены
void mouseFunc(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)//поступило событие от левой кнопки мыши
	{
		if (state == GLUT_DOWN)//нажатие кнопки мыши
		{
			//запоминаем начальные координаты в которой нажали левую кнопку мыши. сообщаем объекту-помощнику, что начат поворот сцены
			cameraMotionHelper.startMove(x, y);
		}
		else
		{
			//кнопка мыши была отжата, поэтому заканчиваем поворот сцены, сообщаем объекту-помощнику, что повором сцены окончен
			cameraMotionHelper.endMove();
		}
	}
}
//метод получения событий перемещения курсора мыши при любой нажатой кнопке мыши
void mouseMotionFunc(int x, int y)
{
	//сообщаем объекту-помощнику новые координаты курсора
	cameraMotionHelper.move(x, y);
}
//метод получения событий от колеса мыши. используется для приближения и отдаления камеры наблюдателя
void mouseWheelFunc(int button, int dir, int x, int y)
{
	//в зависимости от направления прокрутки колеса, сообщаем объекту-помощнику либо увеличить дистанцию до наблюдаемой точки - либо уменьшить
	if (dir < 0)
		cameraMotionHelper.increaseDistance();
	else
		cameraMotionHelper.decreaseDistance();
}
//метод для расчета нормалей для каждой вершины каждого треугольника модели
void calcModelNormals(const GLfloat* v, GLfloat* n, GLuint vertexCount)
{
	for (GLuint i = 0; i < vertexCount - 2; i += 3)
	{
		vec3 a = vec3(v[i * 3 + 0], v[i * 3 + 1], v[i * 3 + 2]);
		vec3 b = vec3(v[i * 3 + 3], v[i * 3 + 4], v[i * 3 + 5]);
		vec3 c = vec3(v[i * 3 + 6], v[i * 3 + 7], v[i * 3 + 8]);

		vec3 norm = glm::normalize(glm::cross(b - a, c - a));

		//в нашем случае в каждом треугольнике нормали сориентированы одинаково
		memcpy_s(&n[i * 3 + 0], sizeof(GLfloat) * 3, glm::value_ptr(norm), sizeof(GLfloat) * 3);
		memcpy_s(&n[i * 3 + 3], sizeof(GLfloat) * 3, glm::value_ptr(norm), sizeof(GLfloat) * 3);
		memcpy_s(&n[i * 3 + 6], sizeof(GLfloat) * 3, glm::value_ptr(norm), sizeof(GLfloat) * 3);
	}
}
int main(int argc, char** argv)
{
	//функция glutInit производит начальную инициализацию самой библиотеки glut
	glutInit(&argc, argv);
	//команда glutInitDisplayMode инициализирует буфер кадра и настраивает полноцветный (непалитрованный) режим RGB
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	//glutInitWindowSize используется для задания начальных размеров окна
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//задать положение создаваемого окна относительно верхнего левого угла экрана
	glutInitWindowPosition(100, 100);
	//эта команда создает окно с заголовком, который указываем в качестве параметра и возвращает HANDLER окна в виде числа int. этот HANDLER обычно используется для последующих операций над этим оканом, таких, как изменение параметров окна и закрытие окна. 
	glutCreateWindow("Light, Transformations, Material");

	const char* vendor = (const char*)glGetString(GL_VENDOR);
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	const char* version = (const char*)glGetString(GL_VERSION);
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);

	printf("Vendor: %s\n Renderer: %s\n Version: %s\n ", vendor, renderer, version);

	//рассчитываем нормали
	calcModelNormals(vertexArr, normalArr, 36);
	init();
	//регистрируем функцию прорисовки сцены
	glutDisplayFunc(display);
	//регистрируем функцию изменения размера окна
	glutReshapeFunc(reshape);
	//регистрируем функцию получения событий от клавиатуры
	glutKeyboardFunc(keyboardFunc);
	//регистрируем функцию получения событий от колеса мыши
	glutMouseWheelFunc(mouseFunc);
	//регистрируем функцию получения событий от таймера для реализации анимации
	glutTimerFunc(25, updateAnimation, 0);
	//регистрируем функцию получения событий от кнопок мыши
	glutMouseFunc(mouseFunc);
	//регистрируем фенкцию получения событий от движения курсора
	glutMotionFunc(mouseMotionFunc);
	//контроль всех событий и вызовов нужных функций происходит внутри цикла обработки сообщений в функции glutMainLoop
	glutMainLoop();

	return 0;
}