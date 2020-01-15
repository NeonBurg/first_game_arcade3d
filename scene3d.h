#ifndef SCENE3D_H
#define SCENE3D_H

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>

#include <QGLWidget>
#include <QtCore>
#include <QtGui>

class scene3D : public QGLWidget
{
    Q_OBJECT
public:
    explicit scene3D(QWidget *parent = 0);

private:
    GLfloat	x_Rotate;			// Поворот X
    GLfloat	y_Rotate;			// Y
    QPoint m_ptPosition;

    GLfloat gWidth, gLength, x, y, z;
    GLfloat xTranslate, yTranslate, zTranslate, angle;
    GLfloat line_move, block_move, block_move2; //Движение

    int remove; //На сколько сдвинулось поле
    int move_speed;
    int curr_block_pos;
    GLfloat z_trans, z_marker; //Позиция блока в глубину

    void createBlock (float Xo, float width, float height, float depth);
    void createMarker (float Xo, float width, float depth, float white);
    int randomizePos(void);

    GLuint texHandle[15]; //Место для текстуры
    void LoadTextures();
    void setNumber(float Xo, float width, float height, int number);
    void showScore(int score);
    void gameOver();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    virtual void mousePressEvent(QMouseEvent* pe);
    virtual void mouseMoveEvent(QMouseEvent* pe);
    void keyPressEvent(QKeyEvent *);

public slots:
    void animate();
    void blink();
    void gameStart();

};

#endif // SCENE3D_H
