#pragma once
//чувствительность мыши при повороте сцены
#define MOUSE_SENSITIVITY 0.2f;

//чувствительность колеса мыши
#define WHEEL_SENSITIVITY 0.5f;


//описание объекта-помощника для поворота сцены с помощью курсора мыши 
class CameraMotionHelper
{
private:
	//переменная хранит состояние левой кнопки мыши
	bool leftButtonDown;
	//переменная для хранения текущей и предыдущей позиции курсора (x и y)
	int currentPosition[2];
	int lastPosition[2];
	//переменная для хранения углов поворота сцены вычисленные из позиции курсора
	float rotationY;
	float rotationXZ;
	//расстояние от наблюдателя до центра сцены
	float distance;
public:
	//методы для доступа к значениям переменных
	float getrotationY() { return rotationY; }
	float getRotationXZ() { return rotationXZ; }
	float getDistance() { return distance; }
public:
	//метод, который говорит, что мы нажали левую кнопку мыши и начинаем поворот сцены
	void startMove(int x, int y)
	{
		leftButtonDown = true;
		lastPosition[0] = x;
		lastPosition[1] = y;
		currentPosition[0] = x;
		currentPosition[1] = y;
	}
	//метод, вызываемый при движении курсора мыши с нажатой левой кнопкой
	void move(int x, int y)
	{
		if (leftButtonDown)
		{
			lastPosition[0] = currentPosition[0];
			lastPosition[1] = currentPosition[1];

			currentPosition[0] = x;
			currentPosition[1] = y;

			rotationY += (currentPosition[0]) - lastPosition[0] * MOUSE_SENSITIVITY;
			rotationXZ += (currentPosition[1]) - lastPosition[1] * MOUSE_SENSITIVITY;
		}
	}
	//метод говорит о том, что мы отпустили левую кнопку мыши и вращение сцены завершено
	void endMove()
	{
		leftButtonDown = false;
	}
	//методы для увеличения и уменьшения дистанции от наблюдателя до наблюдаемой точки 
	//методы вызываются при вращении колеса мыши
	void increaseDistance() { distance += WHEEL_SENSITIVITY; }
	void decreaseDistance() { distance -= WHEEL_SENSITIVITY; }

public:
	//конструктор класса
	CameraMotionHelper()
	{
		leftButtonDown = false;

		lastPosition[0] = 0; lastPosition[1] = 0;
		currentPosition[0] = 0; currentPosition[1] = 0;

		rotationY = 0.0f;
		rotationXZ = 0.0f;

		distance = 13.0f;
	}
};


