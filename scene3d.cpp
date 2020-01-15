#include "scene3d.h"
#include <QLabel>

/* ---------- Некоторые переменные ---------- */
QTimer *aTimer = new QTimer; //Таймер для анимации движения
QTimer *blink_Timer = new QTimer; //Таймер для мерцания маркера
QTimer *start_Timer = new QTimer; //Таймер для старта
int fraction(40); //Номер фракции (всего 40)
int between(3); //Расстояние между блоками
int marker_pos(2); //Нач. положение маркера по горизонтали
int m_start_pos(2); //Нач. положение маркера по вертикали
int beg_move_speed(30); //Нач. скорость движения
int cScore(0); //Счет игры
int start(0); //Начало игры
int start_count(0);
int blink_freq(0);
int loose(0);
int change_speed(20); //Частота изменения скорости, относительно счета игры
QLinkedList <int> bPosition; //Лист из блоков (всего 3 возможных позиции)
/* ------------------------------------------ */

scene3D::scene3D(QWidget *parent) :
    QGLWidget(parent)
{
    gLength = 0.5; //Размер клетки
    gWidth = 3*gLength; //Каждая третяя часть ширины будет квадратом

    remove = 0;
    xTranslate = 0;
    yTranslate = 0;
    connect(aTimer, SIGNAL(timeout()), SLOT(animate())); //Свяжем таймер с функц. анимации
    connect(blink_Timer, SIGNAL(timeout()), SLOT(blink())); //Свяжем таймер с функц. мерцания
    connect(start_Timer, SIGNAL(timeout()), SLOT(gameStart())); //Свяжем таймер с функц. старта
    //aTimer->start(30);
}

void scene3D::initializeGL()
{
    LoadTextures(); //Вызов загрузки текстур
    qglClearColor(Qt::black);
    glEnable(GL_TEXTURE_2D);		// Разрешение наложение текстуры
}

void scene3D::resizeGL(int w, int h)
{
    glLoadIdentity();
    glViewport(0,0, (GLint)w, (GLint)h);
}

/* --------====== РЕНДЕР ИГРЫ ======-------- */
void scene3D::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glClearDepth(1.0f);             // установка буфера глубины
    glEnable(GL_DEPTH_TEST);        // разрешить тест глубины
    glDepthFunc(GL_LESS);         // тип теста глубины

    glMatrixMode(GL_PROJECTION);
    gluPerspective (70, 1, 3, 100);

    glPushMatrix();
    glTranslatef(0.0f, -2.1, -3.0);
    glRotatef(27.8, 1.0f, 0.0f, 0.0f);      //Вращение по X
    glRotatef(0.0f, 0.0f, 1.0f, 0.0f);  //по Y

    /* Отрисовываем прямоугольники, разделенные на 3 части,
       всего 40 прямоугольников, назовем их фракциями*/
    for (int i=1; i<= 40; i++)
    {
        /* Рисуем фракцию */
        glBegin(GL_LINE_LOOP);
                    z = gLength*i; //На каждом шаге отдаляем Z на величину i
                    z -= line_move; //Изменяем значение Z для анимации движения
                    glColor3f(1.0, 1.0, 1.0);
                    glVertex3f(-gWidth, 0.0, 0.0);
                    glVertex3f(gWidth, 0.0, 0.0);
                    glVertex3f(gWidth, 0.0, -z);
                    glVertex3f(-gWidth, 0.0, -z);
        glEnd();

        /* Делим фрацию на 3 части */
        glBegin(GL_LINES);
                glVertex3f(-gLength, 0.0, 0.0);    glVertex3f(-gLength, 0.0, -z);
                glVertex3f(gLength, 0.0, 0.0);    glVertex3f(gLength, 0.0, -z);
        glEnd();
    }

    /* Если вошли в функцию первый раз (начало игры)
       проинициализируем позиции блоков*/
    if(bPosition.isEmpty())
    {
        for (int i=0; i<fraction; i++)
        {
            bPosition.append(randomizePos()); //Добавляем в лист блок со случайной позицией
        }
    }

    /* Отрисовка блоков (препятствий) */
    int k(1); //Порядковый номер блока

    /* ----------- Отображаем список блоков -------------- */
    foreach (int i, bPosition)
    {
        z_trans = gLength*k; //Рисуем каждый блок с новой позиции, смещенной на шаг k
        z_trans -= block_move; //Анимация движения блока

        if (!(k%between))
        {
            if(i == 1) createBlock(-gWidth, gLength*2, gLength, z_trans);
            if(i == 2) createBlock(-gLength, gLength*2, gLength, z_trans);
            if(i == 3) createBlock(gLength, gLength*2, gLength, z_trans);
        }

        if (k == between) curr_block_pos = i; //Текущая позиция блока для проверки на
                                        //совпадение с позицией маркера

        k++;
    }

    /* ----------- Маркер следования игрока ----------- */
    z_marker = gLength*(m_start_pos-1);
    if (!(blink_freq%2))
    {
        if (marker_pos == 1) createMarker(-gWidth, gLength, z_marker, 1);
        if (marker_pos == 2) createMarker(-gLength, gLength, z_marker, 1);
        if (marker_pos == 3) createMarker(gLength, gLength, z_marker, 1);
    }
    /* ------------------------------------------------ */

    /* Элементы выбора ENTER и ESC */
    if(start == 0)
    {
        glEnable(GL_BLEND); //Активируем режим наложения текстур с альфа каналом
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* --- ENTER --- */
        glPushMatrix();

        glLoadIdentity();
        glTranslatef(-0.85, 0.1, 0);

        qglColor(Qt::white);
        glBindTexture(GL_TEXTURE_2D, texHandle[11]);

        int entSize = 3;
        float entWidth = 0.13*entSize;
        float entHeight = 0.14*entSize;

        glBegin(GL_POLYGON);
                glVertex3f(0, 0, 0);                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(0, entHeight, 0);        glTexCoord2f(1.0f, 1.0f);
                glVertex3f(entWidth, entHeight, 0); glTexCoord2f(1.0f, 0.0f);
                glVertex3f(entWidth, 0, 0);         glTexCoord2f(0.0f, 0.0f);
        glEnd();

        /* --- ESC --- */
        glTranslatef(1.32, 0, 0);

        qglColor(Qt::white);
        glBindTexture(GL_TEXTURE_2D, texHandle[12]);

        int escSize = 3;
        float escWidth = 0.111*escSize;
        float escHeight = 0.087*escSize;

        glBegin(GL_POLYGON);
                glVertex3f(0, 0, 0);                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(0, escHeight, 0);        glTexCoord2f(1.0f, 1.0f);
                glVertex3f(escWidth, escHeight, 0); glTexCoord2f(1.0f, 0.0f);
                glVertex3f(escWidth, 0, 0);         glTexCoord2f(0.0f, 0.0f);
        glEnd();

        glPopMatrix();
        glDisable(GL_BLEND);
    }
    /* --------------------------- */

    /* --- Отсчет старта --- */

    if (start == 1)
    {
        cScore = 0; //Оюнуляем игровой счет
        loose = 0;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPushMatrix();

        glLoadIdentity();
        glTranslatef(-0.14, 0.58, 0);

        float scHeight = 0.25;
        float numWidth = (scHeight/3)*2; //Ширина цифр (2/3 от высоты)

        if(start_count == 0) setNumber(0.3, numWidth, scHeight, 3);
        if(start_count == 1) setNumber(0.3, numWidth, scHeight, 2);
        if(start_count == 2) setNumber(0.3, numWidth, scHeight, 1);
        if(start_count == 3 || start_count == 4)
        {
            qglColor(Qt::green);
            glBindTexture(GL_TEXTURE_2D, texHandle[13]);

            float goSize = 3;
            float goWidth = 0.096*goSize;
            float goHeight = 0.077*goSize;

            glBegin(GL_POLYGON);
                    glVertex3f(0, 0, 0);                glTexCoord2f(0.0f, 1.0f);
                    glVertex3f(0, goHeight, 0);        glTexCoord2f(1.0f, 1.0f);
                    glVertex3f(goWidth, goHeight, 0); glTexCoord2f(1.0f, 0.0f);
                    glVertex3f(goWidth, 0, 0);         glTexCoord2f(0.0f, 0.0f);
            glEnd();

            if(start_count == 4)
            {
                start_Timer->stop();
                start_count = 0;
                start = 2;
                move_speed = beg_move_speed; //Устанавливаем текущ. скор. движ. равной начальной
                aTimer->start(move_speed); //Нач. движение
            }
        }

        glPopMatrix();
        glDisable(GL_BLEND);
    }


    /* --------------------- */

    int maxSize(40); //Макс. размер списка

    /* -------- Проверка совпадения положения маркера и блока --------*/

    int inter = between - m_start_pos; //Расстояние от маркера до первого блока
    int diff (remove - inter); //Разница пройденного пути и расстояния
                               //от маркера до блока
    if ( !(diff%between) )
    {
        /* Если положение текущего блока равно положению маркера */
        if(curr_block_pos == marker_pos)
        {
            aTimer->stop(); //Остановка движения
            if(start == 2)
            {
                blink_Timer->start(150);
                start = 0;
                loose = 1;
            }

        }
    }

    /* Предотвращение переполнения списка */
    if(bPosition.size() >= (40 + between)) //Если кол-во блоков(элементов) в списке привысило
                                           //начальное кол-во блоков + расстояние между ними
    {
        int del = bPosition.size() - maxSize; //Вычисляем кол-во элементов,
                                              //которые необходимо удалить из списка
        for(int i = 0; i<del; i++)
        {
            bPosition.removeFirst(); //Удаляем, начиная с первого элемента
        }
        block_move = block_move - (del*gLength); //Сдвигаем оставшиеся блоки на путь,
                                                 //пройденный удаленными блоками
    }

    showScore(cScore); //Выводим счет на экран

    /* Остановка мерцания маркера после проигрыша */
    if(blink_freq == 6)
    {
        blink_Timer->stop();
        blink_freq = 0;
    }

    if (loose == 1 && !(blink_freq%2)) gameOver();

    glPopMatrix();
}

/* Функция определяет случайную позицию блока */
int scene3D::randomizePos(void)
{
    int p = qrand() % 3 +1; //Рандом [1;3]

    return p;
}

/* ------======= НАДПИСЬ "Game Over" =======------ */
void scene3D::gameOver()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();

    glLoadIdentity();
    glTranslatef(-0.97, 0.7, 0);

    qglColor(Qt::red);
    glBindTexture(GL_TEXTURE_2D, texHandle[14]);

    int gmoSize = 3;
    float gmoWidth = 0.186*gmoSize;
    float gmoHeight = 0.029*gmoSize;

    glBegin(GL_POLYGON);
            glVertex3f(0, 0, 0);                glTexCoord2f(0.0f, 1.0f);
            glVertex3f(0, gmoHeight, 0);        glTexCoord2f(1.0f, 1.0f);
            glVertex3f(gmoWidth, gmoHeight, 0); glTexCoord2f(1.0f, 0.0f);
            glVertex3f(gmoWidth, 0, 0);         glTexCoord2f(0.0f, 0.0f);
    glEnd();

    glPopMatrix();
    glDisable(GL_BLEND);
}

/* ------======= СОЗДАЕМ МАРКЕР =======------ */
void scene3D::createMarker (float Xo, float width, float depth, float white)
{
    glPushMatrix();

    glTranslatef(Xo, 0.0, -depth);

    glBegin(GL_QUADS);
    if (white == 1) qglColor(Qt::white);
    else qglColor(Qt::black);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, 0.0, -width);
            glVertex3f(width*2, 0.0, -width);
            glVertex3f(width*2, 0.0, 0.0);
    glEnd();

    glPopMatrix();
}

/* ------======= СОЗДАЕМ БЛОК =======------ */
void scene3D::createBlock (float Xo, float width, float height, float depth)
{
    glPushMatrix();

    glTranslatef(Xo, 0.0, -depth);

    glBegin(GL_QUADS);
            /*Правая сторона*/
            qglColor(Qt::yellow);
            glVertex3f(width, height, 0.0);
            glVertex3f(width, height, -height);
            glVertex3f(width, 0.0, -height);
            glVertex3f(width, 0.0, 0.0);

            /*Левая сторона*/
            qglColor(Qt::blue);
            glVertex3f(0.0, 0.0, -height);
            glVertex3f(0.0, height, -height);
            glVertex3f(0.0, height, 0.0);
            glVertex3f(0.0, 0.0, 0.0);

            /*Передняя сторона*/
            qglColor(Qt::green);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, height, 0.0);
            glVertex3f(width, height, 0.0);
            glVertex3f(width, 0.0, 0.0);

            /*Верхняя сторона*/
            qglColor(Qt::red);
            glVertex3f(0.0, height, 0.0);
            glVertex3f(0.0, height, -height);
            glVertex3f(width, height, -height);
            glVertex3f(width, height, 0.0);

    glEnd();
    glPopMatrix();
}

/* ------======= ОТСЧЕТ СТАРТА =======------ */
void scene3D::gameStart()
{
    start_count++;
    qDebug() << "start_count = " << start_count;
    updateGL();
}

/* ------======= МЕРЦАНИЕ МАРКЕРА =======------ */
void scene3D::blink()
{
    qDebug() << "blink_freq = " << blink_freq;
    blink_freq += 1;
    updateGL();
}

/* ------======= АНИМАЦИЯ =======------ */
void scene3D::animate()
{
    if(line_move >= gLength)
    {
        line_move = 0.0;
        remove += 1; //Прибавляем +1 к перемещению
        cScore += 1; //+1 к счету

        /* Увеличеваем скорость движения при увеличении счета */
        if (!(cScore%change_speed) && cScore != 0 && move_speed > 0)
        {
            move_speed -= 5;
            aTimer->start(move_speed);
            qDebug() << "move_speed = " << move_speed;
        }

        bPosition.append(randomizePos()); //Добавляем новый блок в конец списка
    }
    else
    {
        line_move += 0.1;
        block_move += 0.1;
    }

    updateGL();
}

/* ------======= СЧЕТ ИГРЫ =======------ */
void scene3D::showScore(int score)
{
    glPushMatrix();

    glEnable(GL_BLEND); //Активируем режим наложения текстур с альфа каналом
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLoadIdentity();
    glTranslatef(-0.98, 0.85, 0.0); //Позицианируем счет в левом верхнем углу

    float scHeight = 0.13;
    float scWidth = scHeight*4; //Ширина надписи "Score"
    float numWidth = (scHeight/3)*2; //Ширина цифр (2/3 от высоты)

    qglColor(Qt::white);

    /* Отображаем надпись "Score" */
    glBindTexture(GL_TEXTURE_2D, texHandle[10]);
    glBegin(GL_POLYGON);
            glVertex3f(0.0, 0.0, 0.0);glTexCoord2f(0.0f, 1.0f);
            glVertex3f(0.0, scHeight, 0.0);glTexCoord2f(1.0f, 1.0f);
            glVertex3f(scWidth, scHeight, 0.0);glTexCoord2f(1.0f, 0.0f);
            glVertex3f(scWidth, 0.0, 0.0);glTexCoord2f(0.0f, 0.0f);
    glEnd();

    /*Отображаем цифры*/
    QString str; //Строка в которую конвертируется счет
    QString symbol; //Символ из строки со счетом
    int num_temp; //Число

    str.setNum(score); //Конвертируем счет в строку

    glTranslatef(scWidth+0.03, 0, 0); //Смест. вправо на ширину надписи "Score:"

    /* Выводим цифры счета на экран */
    for (int i=0; i<str.size(); i++)
    {
        symbol = str.at(i); //Получим символ из строки с счетом
        num_temp = symbol.toInt(); //Конверт. символ в Integer
        setNumber(i, numWidth, scHeight, num_temp); //Вызов. функц. рисования числа
    }

    glDisable(GL_BLEND);

    glPopMatrix();
}

/* ------======= РИСУЕМ ЧИСЛО =======------ */
void scene3D::setNumber(float Xo, float width, float height, int number)
{
    float x0 = width*Xo;

    glPushMatrix();

    glTranslatef(x0, 0.0, 0.0);

    if (number == 0) glBindTexture(GL_TEXTURE_2D, texHandle[0]); //Накладываем текстуру
    if (number == 1) glBindTexture(GL_TEXTURE_2D, texHandle[1]);
    if (number == 2) glBindTexture(GL_TEXTURE_2D, texHandle[2]);
    if (number == 3) glBindTexture(GL_TEXTURE_2D, texHandle[3]);
    if (number == 4) glBindTexture(GL_TEXTURE_2D, texHandle[4]);
    if (number == 5) glBindTexture(GL_TEXTURE_2D, texHandle[5]);
    if (number == 6) glBindTexture(GL_TEXTURE_2D, texHandle[6]);
    if (number == 7) glBindTexture(GL_TEXTURE_2D, texHandle[7]);
    if (number == 8) glBindTexture(GL_TEXTURE_2D, texHandle[8]);
    if (number == 9) glBindTexture(GL_TEXTURE_2D, texHandle[9]);

    glBegin(GL_POLYGON);
            glVertex3f(0.0, 0.0, 0.0);glTexCoord2f(0.0f, 1.0f);
            glVertex3f(0.0, height, 0.0);glTexCoord2f(1.0f, 1.0f);
            glVertex3f(width, height, 0.0);glTexCoord2f(1.0f, 0.0f);
            glVertex3f(width, 0.0, 0.0);glTexCoord2f(0.0f, 0.0f);
    glEnd();

    glPopMatrix();
}

/* ------======= Создадим ТЕКСТУРЫ из файлов =======------ */
void scene3D::LoadTextures()
{
    // Загружаем картинку
    glEnable(GL_TEXTURE_2D);
    texHandle[0] = bindTexture(QImage(":/Textures/Textures/Zero.png"));
    texHandle[1] = bindTexture(QImage(":/Textures/Textures/One.png"));
    texHandle[2] = bindTexture(QImage(":/Textures/Textures/Two.png"));
    texHandle[3] = bindTexture(QImage(":/Textures/Textures/Three.png"));
    texHandle[4] = bindTexture(QImage(":/Textures/Textures/Four.png"));
    texHandle[5] = bindTexture(QImage(":/Textures/Textures/Five.png"));
    texHandle[6] = bindTexture(QImage(":/Textures/Textures/Six.png"));
    texHandle[7] = bindTexture(QImage(":/Textures/Textures/Seven.png"));
    texHandle[8] = bindTexture(QImage(":/Textures/Textures/Eight.png"));
    texHandle[9] = bindTexture(QImage(":/Textures/Textures/Nine.png"));
    texHandle[10] = bindTexture(QImage(":/Textures/Textures/Score.png"));
    texHandle[11] = bindTexture(QImage(":/Textures/Textures/Play.png"));
    texHandle[12] = bindTexture(QImage(":/Textures/Textures/Exit.png"));
    texHandle[13] = bindTexture(QImage(":/Textures/Textures/GO.png"));
    texHandle[14] = bindTexture(QImage(":/Textures/Textures/Game_Over.png"));

    // Устанавливаем самый близкий способ фильтрации для текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Устанавливаем самый близкий способ фильтрации для текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

/* ------======= НАЖАТИЕ КЛАВИШ =======------ */
void scene3D::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
            /* ----------------- */
            case Qt::Key_Left: {
                if(marker_pos != 1) marker_pos -= 1; //Если позиция маркера не равна
                                                     //крайнему значению, сдвигаем маркер влево <-

                updateGL();
            break;
            /* ----------------- */
            case Qt::Key_Right:
                if(marker_pos != 3) marker_pos += 1; //Сдвигаем маркер вправо ->

                updateGL();
            break;
            /* ----------------- */
                case Qt::Key_W:
                    yTranslate += 0.01;
                    qDebug() << "yTranslate = " << yTranslate;
                    updateGL();
                break;
            /* ----------------- */
                case Qt::Key_S:
                    yTranslate -= 0.01;
                    qDebug() << "yTranslate = " << yTranslate;
                    updateGL();
                break;
                    /* ----------------- */
                        case Qt::Key_A:
                            xTranslate += 0.01;
                            qDebug() << "xTranslate = " << xTranslate;
                            updateGL();
                        break;
                    /* ----------------- */
                        case Qt::Key_D:
                            xTranslate -= 0.01;
                            qDebug() << "xTranslate = " << xTranslate;
                            updateGL();
                        break;
                    /* ------ИГРАТЬ------- */
                        case Qt::Key_Return:
                            qDebug() << "start = " << start;
                            if (start == 0)
                            {
                                start = 1;
                                updateGL();
                                start_Timer->start(1000);
                                qDebug() << "start = " << start;
                            }
                        break;
                            /* ------ВЫХОД------ */
                                case Qt::Key_Escape:
                                    close();
                                break;

          }
     default:
     break;
     }
}

/*virtual*/ void scene3D::mousePressEvent(QMouseEvent* pe)
{
    m_ptPosition = pe->pos();
}

/*virtual*/ void scene3D::mouseMoveEvent(QMouseEvent* pe)
{
    x_Rotate += 180* (GLfloat)(pe->y() - m_ptPosition.y()) / height();
    y_Rotate += 180* (GLfloat)(pe->x() - m_ptPosition.x()) / width();
    updateGL();

    m_ptPosition = pe->pos();
}
