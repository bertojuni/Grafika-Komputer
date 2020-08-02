// PRAKTIKUM KE-9 GRAFIKA KOMPUTER
// REPRESENTASI PERMUKAAN KURVA
// ADHI PRAHARA
// TEKNIK INFORMATIKA
// UNIVERSITAS AHMAD DAHLAN
// 2017

// deklarasi header 
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// header untuk GLUT
#include <glut.h>

using namespace std;

#define PHI 3.141592654		
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 480

// container untuk membuat tipe data 3D (X, Y, Z)
struct Vec3
{
	float X; float Y; float Z;
	Vec3(float x, float y, float z) { X = x; Y = y; Z = z; }
	//
	Vec3() { }
	~Vec3() { }
};

// inisialisasi variabel untuk kecepatan pergerakan kamera
// (akan digunakan di praktikum ke-5 tentang Proyeksi)
float fov = 45;						// sudut proyeksi
float moveSpeed = 0.5f;				// kecepatan pergerakan kamera
float rotateSpeed = 0.05f;			// kecepatan rotasi kamera

// inisialisasi variabel untuk proyeksi
// (akan digunakan di praktikum ke-5 tentang Proyeksi)			
// posisi kamera
Vec3 camPosition = Vec3(0.0f, 0.0f, 5.0f);
Vec3 camLookAt = Vec3(0.0f, 0.0f, -1.0f);
Vec3 camUp = Vec3(0.0f, 1.0f, 0.0f);	
// sudut transformasi kamera
float camAngle = 0.0f;

// inisialisasi variabel untuk proyeksi dengan mouse
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;

// inisialisasi variabel untuk transformasi 
// seperti translasi, rotasi atau scaling
// (akan digunakan di praktikum ke-4 tentang transformasi 2D/3D)
float objectAngle = 0.0f;				// sudut tranformasi obyek
Vec3 objectRotation = Vec3(0.0f, 1.0f, 0.0f); // posisi rotasi
Vec3 objectPosition = Vec3(0.0f, 0.0f, 0.0f);	// posisi obyek

// parameter kurva NURB
Vec3 cPoint[5][5];
bool showPoints = true, modePoly = true;
GLUnurbsObj *theNurb;

// parameter pencahayaan
GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_shininess[] = { 100.0 };

// fungsi untuk melakukan normalisasi koordinat posisi
Vec3 normalize(Vec3 value)
{
	Vec3 result;
	float lengths = sqrt((value.X * value.X) + (value.Y * value.Y)
		+ (value.Z * value.Z));
	result.X = value.X / lengths;
	result.Y = value.Y / lengths;
	result.Z = value.Z / lengths;

	return result;
}

// fungsi untuk melakukan operasi perkalian cross
Vec3 cross(Vec3 value1, Vec3 value2)
{
	Vec3 result;
	result.X = value1.Y * value2.Z - value2.Y * value1.Z;
	result.Y = value1.Z * value2.X - value2.Z * value1.X;
	result.Z = value1.X * value2.Y - value2.X * value1.Y;

	return result;
}

// fungsi ini digunakan untuk menggambar obyek 
void drawObject()
{
	glPushMatrix();

	glRotatef(objectAngle, objectRotation.X, objectRotation.Y, objectRotation.Z);

	glScalef(0.2, 0.2, 0.2);

	glColor3f(0.0f, 1.0f, 0.0f);

	if (modePoly)
		// pindah mode ke tampilan wireframe
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	else
		// pindah mode ke tampilan solid
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	// tentukan titik kontrolnya
	cPoint[0][0] = Vec3(-6,-6, 0); 
	cPoint[0][1] = Vec3(-6,-3, 0); 
	cPoint[0][2] = Vec3(-6, 0, 0); 
	cPoint[0][3] = Vec3(-6, 3, 0); 
	cPoint[0][4] = Vec3(-6, 6, 0); 
	cPoint[1][0] = Vec3(-6,-6, 0); 
	cPoint[1][1] = Vec3(-3,-3, 5); 
	cPoint[1][2] = Vec3(-3, 0, 5); 
	cPoint[1][3] = Vec3(-3, 3, 5);
	cPoint[1][4] = Vec3(-3, 6, 0);
	cPoint[2][0] = Vec3( 0,-6, 0); 
	cPoint[2][1] = Vec3( 0,-3, 0); 
	cPoint[2][2] = Vec3( 0, 0, 0); 
	cPoint[2][3] = Vec3( 0, 3, 0);
	cPoint[2][4] = Vec3( 0, 6, 0);
	cPoint[3][0] = Vec3( 3,-6, 0); 
	cPoint[3][1] = Vec3( 3,-3, 0); 
	cPoint[3][2] = Vec3( 3, 0, 0); 
	cPoint[3][3] = Vec3( 3, 3, 0);
	cPoint[3][4] = Vec3( 3, 6, 0);
	cPoint[4][0] = Vec3( 6,-6, 0); 
	cPoint[4][1] = Vec3( 6,-3, 0); 
	cPoint[4][2] = Vec3( 6, 0, 0); 
	cPoint[4][3] = Vec3( 6, 3, 0);
	cPoint[4][4] = Vec3( 6, 6, 0);

	// definisikan knotnya 
	// jumlah knot = derajat kurva + jumlah titik kontrol + 1
	GLfloat knots[9] = { 0.0, 0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0 };

	// menggambar permukaan diawali dengan gluBeginSurface()
	// diakhiri dengan gluEndSurface()
	// gambar kurva NURB
	gluBeginSurface(theNurb);				
	gluNurbsSurface(theNurb,	// obyek NURB
		9,						// jumlah knot di parameter u
		knots,					// array dari knot
		9,						// jumlah knot di parameter v
		knots,					// array dari knot
		5*3,					// jarak antar titik kontrol di parameter u
		3,						// jarak antar titik kontrol di parameter v
		&cPoint[0][0].X,		// titik kontrol kurva NURB
		4,						// ordo dari kurva di parameter u
		4,						// ordo dari kurva di parameter v
		GL_MAP2_VERTEX_3);		// tipe data permukaan
	gluEndSurface(theNurb);

	if (showPoints) 
	{
		glPointSize(5.0);
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 0.0);
		glBegin(GL_POINTS);
		for (int i = 0; i < 5; i++) 
		{
			for (int j = 0; j < 5; j++) 
				glVertex3f(cPoint[i][j].X, cPoint[i][j].Y, cPoint[i][j].Z);
		}
		glEnd();
		glEnable(GL_LIGHTING);
	}

	glPopMatrix();
}

// taruh semua fungsi obyek yang akan digambar di fungsi display()
void display()
{
	// bersihkan dan reset layar dan buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// posisikan pandangan kamera
	// dalam hal ini sumbu Y kamera ada di atas 
	// dan posisi kamera di camPosition
	gluLookAt(camPosition.X, camPosition.Y, camPosition.Z,
		camPosition.X + camLookAt.X, 
		camPosition.Y + camLookAt.Y, 
		camPosition.Z + camLookAt.Z,
		camUp.X, camUp.Y, camUp.Z);

	// panggil fungsi untuk menggambar obyek
	drawObject();
	
	// tampilkan obyek ke layar
	// gunakan glFlush() bila memakai single buffer
	// gunakan glutSwapBuffers() bila memakai double buffer
	glutSwapBuffers();
}

// inisialisasikan variabel, pencahayaan, tekstur, 
// pengaturan pandangan kamera dan sebagainya di fungsi init()
void init(void)
{
	// inisialisasi warna latar belakang layar 
	// dalam hal ini warna putih warna putih (1.0, 1.0, 1.0, 0.0)
	glClearColor(1.0, 1.0, 1.0, 0.0);
	// mengaktifkan depth buffer
	glEnable(GL_DEPTH_TEST);				
	glMatrixMode(GL_PROJECTION);		
	glLoadIdentity();
	// set proyeksi ke proyeksi perspektif
	gluPerspective(fov, 1.0, 1.0, 100.0);	
	glMatrixMode(GL_MODELVIEW);				
	glLoadIdentity();						
	// inisialisasi kamera pandang
	// kamera berada di posisi (0.0f, 0.0f, 0.0f)
	gluLookAt(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	//int u, v;
	//for (u = 0; u < 4; u++) 
	//{
	//	for (v = 0; v < 4; v++) 
	//	{
	//		controlPoint[u][v].X = 2.0*((GLfloat)u - 1.5);
	//		controlPoint[u][v].Y  = 2.0*((GLfloat)v - 1.5);

	//		if ((u == 1 || u == 2) && (v == 1 || v == 2))
	//			controlPoint[u][v].Z = 3.0;
	//		else
	//			controlPoint[u][v].Z = 0.0;
	//	}
	//}

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	theNurb = gluNewNurbsRenderer();
	gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 25.0);
	gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);
}

// fungsi ini digunakan bila layar akan diresize (default)
// (akan dijelaskan pada praktikum ke-5)
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

// fungsi untuk mengatur masukan dari keyboard 
void keyboard(unsigned char key, int x, int y)
{
	Vec3 result = normalize(cross(camLookAt, camUp));

	switch (key)
	{
	case 'w':	// bila tombol 'w' pada keyboard ditekan
				// geser pandangan kamera ke depan
			camPosition.X += moveSpeed * camLookAt.X;
			camPosition.Y += moveSpeed * camLookAt.Y;
			camPosition.Z += moveSpeed * camLookAt.Z;
			glutPostRedisplay();
			break;
	case 's': 	// bila tombol 's' pada keyboard ditekan
				// geser pandangan kamera ke belakang
			camPosition.X -= moveSpeed * camLookAt.X;
			camPosition.Y -= moveSpeed * camLookAt.Y;
			camPosition.Z -= moveSpeed * camLookAt.Z;
			glutPostRedisplay();
		break;
	case 'a':	// bila tombol 'a' pada keyboard ditekan
				// geser pandangan kamera ke ke kiri
			camPosition.X -= moveSpeed * result.X;
			camPosition.Y -= moveSpeed * result.Y;
			camPosition.Z -= moveSpeed * result.Z;
			glutPostRedisplay();
		break;
	case 'd':	// bila tombol 'd' pada keyboard ditekan
				// geser pandangan kamera ke kanan
			camPosition.X += moveSpeed * result.X;
			camPosition.Y += moveSpeed * result.Y;
			camPosition.Z += moveSpeed * result.Z;
			glutPostRedisplay();
		break;
	case 'f':	// bila tombol 'f' pada keyboard ditekan
				// tampilkan / sembunyikan titik
			showPoints = !showPoints;
			glutPostRedisplay();
		break;
	case 'g':	// bila tombol 'g' pada keyboard ditekan
				// sembunyikan titik
			modePoly = !modePoly;
			glutPostRedisplay();
		break;
	case 27:	// bila tombol 'esc' pada keyboard ditekan
				// keluar program
			exit(0);
		break;
	}
}

// fungsi untuk mengatur masukan dari keyboard 
// untuk arah panah kiri, kanan, atas, bawah, PgDn dan PgUp
void keyboardGLUT(int key, int x, int y)
{
	float fraction = 0.1f;

	switch (key)
	{
		// masukkan perintah disini bila tombol panah kiri ditekan
	case GLUT_KEY_LEFT:
		// perintah rotasi obyek ke kiri 
		objectAngle -= 1.0f;
		objectRotation.X = 0.0f;
		objectRotation.Y = 1.0f;
		objectRotation.Z = 0.0f;
		glutPostRedisplay();	// update obyek
		break;
		// masukkan perintah disini bila tombol panah kanan ditekan
	case GLUT_KEY_RIGHT:
		// perintah rotasi obyek ke kanan
		objectAngle += 1.0f;
		objectRotation.X = 0.0f;
		objectRotation.Y = 1.0f;
		objectRotation.Z = 0.0f;
		glutPostRedisplay();	// update obyek
		break;
		// masukkan perintah disini bila tombol panah atas ditekan
	case GLUT_KEY_UP:
		// perintah rotasi obyek ke atas
		objectAngle += 1.0f;
		objectRotation.X = 1.0f;
		objectRotation.Y = 0.0f;
		objectRotation.Z = 0.0f;
		glutPostRedisplay();	// update obyek
		break;
		// masukkan perintah disini bila tombol panah bawah ditekan
	case GLUT_KEY_DOWN:
		// perintah rotasi obyek ke bawah
		objectAngle -= 1.0f;
		objectRotation.X = 1.0f;
		objectRotation.Y = 0.0f;
		objectRotation.Z = 0.0f;
		glutPostRedisplay();	// update obyek
		break;
	}
}

// fungsi untuk mengatur masukan tombol dari mouse
void mousebutton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_UP)
			firstMouse = false;
		else
			firstMouse = true;
	}
}

// fungsi untuk mengatur masukan pergerakan dari mouse
void mousemove(int x, int y)
{
	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;
	lastX = x;
	lastY = y;

	GLfloat sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	Vec3 front;
	front.X = cos(yaw * PHI / 180) * cos(pitch * PHI / 180);
	front.Y = sin(pitch * PHI / 180);
	front.Z = sin(yaw * PHI / 180) * cos(pitch * PHI / 180);
	camLookAt = normalize(front);

	glutPostRedisplay();
}

// timer untuk animasi (gunakan bila perlu)
void timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(55, timer, 0);
}

// program utama
int main(int argc, char** argv)
{
	// inisialisasi jendela OpenGL
	// GLUT_SINGLE berarti memakai single buffer
	// GLUT_DOUBLE berarti memakai double buffer
	// GLUT_RGB berarti mode tampilan yang dipakai RGB
	// GLUT_RGBA berarti mode tampilan yang dipakai RGBA
	// GLUT_DEPTH berarti memakai depth buffer
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	// set ukuran jendela tampilan
	// besarnya jendela dalam piksel dalam hal ini 480x480
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);		
	// posisi jendela dilayar komputer dalam piksel
	glutInitWindowPosition(100, 100);	

	// judul jendela (wajib diubah dengan informasi 
	// NAMA / NIM - JUDUL PRAKTIKUM masing-masing)
	glutCreateWindow("NAMA / NIM - KODE DASAR PRAKTIKUM GRAFIKA KOMPUTER");
	
	// panggil fungsi init untuk melakukan inisialisasi awal
	init();

	// event handler untuk display, reshape dan keyboard
	glutDisplayFunc(display);   // display
	glutReshapeFunc(reshape);   // reshape
	glutKeyboardFunc(keyboard);  // keyboard
	glutSpecialFunc(keyboardGLUT);
	glutMouseFunc(mousebutton);	// mouse button
	glutMotionFunc(mousemove);	// mouse movement
	//glutTimerFunc(0, timer, 0); // aktifkan timer bila perlu

	// looping
	glutMainLoop();

	return 0;
}