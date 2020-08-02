// PRAKTIKUM KE-7 GRAFIKA KOMPUTER
// KURVA SPLINE
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

// menggambar setiap titik kontrol kurva
void markPoint(Vec3 points, Vec3 colors, float width)
{
	// tandai setiap titik dengan warna
	glPushMatrix();
	glColor3f(colors.X, colors.Y, colors.Z);

	glBegin(GL_QUADS);
	glVertex3f(points.X - width, points.Y - width, points.Z);
	glVertex3f(points.X + width, points.Y - width, points.Z);
	glVertex3f(points.X + width, points.Y + width, points.Z);
	glVertex3f(points.X - width, points.Y + width, points.Z);
	glEnd();

	glPopMatrix();
}

// fungsi untuk menghitung invers matriks ordo 4x4
bool inverse(float inMat[16], float outMat[16])
{
	float inv[16], det;
	int i;

	inv[0] = 
		inMat[5] * inMat[10] * inMat[15] -
		inMat[5] * inMat[11] * inMat[14] -
		inMat[9] * inMat[6] * inMat[15] +
		inMat[9] * inMat[7] * inMat[14] +
		inMat[13] * inMat[6] * inMat[11] -
		inMat[13] * inMat[7] * inMat[10];

	inv[4] = 
	   -inMat[4] * inMat[10] * inMat[15] +
		inMat[4] * inMat[11] * inMat[14] +
		inMat[8] * inMat[6] * inMat[15] -
		inMat[8] * inMat[7] * inMat[14] -
		inMat[12] * inMat[6] * inMat[11] +
		inMat[12] * inMat[7] * inMat[10];

	inv[8] = 
		inMat[4] * inMat[9] * inMat[15] -
		inMat[4] * inMat[11] * inMat[13] -
		inMat[8] * inMat[5] * inMat[15] +
		inMat[8] * inMat[7] * inMat[13] +
		inMat[12] * inMat[5] * inMat[11] -
		inMat[12] * inMat[7] * inMat[9];

	inv[12] = 
	   -inMat[4] * inMat[9] * inMat[14] +
		inMat[4] * inMat[10] * inMat[13] +
		inMat[8] * inMat[5] * inMat[14] -
		inMat[8] * inMat[6] * inMat[13] -
		inMat[12] * inMat[5] * inMat[10] +
		inMat[12] * inMat[6] * inMat[9];

	inv[1] = 
	   -inMat[1] * inMat[10] * inMat[15] +
		inMat[1] * inMat[11] * inMat[14] +
		inMat[9] * inMat[2] * inMat[15] -
		inMat[9] * inMat[3] * inMat[14] -
		inMat[13] * inMat[2] * inMat[11] +
		inMat[13] * inMat[3] * inMat[10];

	inv[5] = 
		inMat[0] * inMat[10] * inMat[15] -
		inMat[0] * inMat[11] * inMat[14] -
		inMat[8] * inMat[2] * inMat[15] +
		inMat[8] * inMat[3] * inMat[14] +
		inMat[12] * inMat[2] * inMat[11] -
		inMat[12] * inMat[3] * inMat[10];

	inv[9] = 
	   -inMat[0] * inMat[9] * inMat[15] +
		inMat[0] * inMat[11] * inMat[13] +
		inMat[8] * inMat[1] * inMat[15] -
		inMat[8] * inMat[3] * inMat[13] -
		inMat[12] * inMat[1] * inMat[11] +
		inMat[12] * inMat[3] * inMat[9];

	inv[13] = 
		inMat[0] * inMat[9] * inMat[14] -
		inMat[0] * inMat[10] * inMat[13] -
		inMat[8] * inMat[1] * inMat[14] +
		inMat[8] * inMat[2] * inMat[13] +
		inMat[12] * inMat[1] * inMat[10] -
		inMat[12] * inMat[2] * inMat[9];

	inv[2] = 
		inMat[1] * inMat[6] * inMat[15] -
		inMat[1] * inMat[7] * inMat[14] -
		inMat[5] * inMat[2] * inMat[15] +
		inMat[5] * inMat[3] * inMat[14] +
		inMat[13] * inMat[2] * inMat[7] -
		inMat[13] * inMat[3] * inMat[6];

	inv[6] = 
	   -inMat[0] * inMat[6] * inMat[15] +
		inMat[0] * inMat[7] * inMat[14] +
		inMat[4] * inMat[2] * inMat[15] -
		inMat[4] * inMat[3] * inMat[14] -
		inMat[12] * inMat[2] * inMat[7] +
		inMat[12] * inMat[3] * inMat[6];

	inv[10] = 
		inMat[0] * inMat[5] * inMat[15] -
		inMat[0] * inMat[7] * inMat[13] -
		inMat[4] * inMat[1] * inMat[15] +
		inMat[4] * inMat[3] * inMat[13] +
		inMat[12] * inMat[1] * inMat[7] -
		inMat[12] * inMat[3] * inMat[5];

	inv[14] = 
	   -inMat[0] * inMat[5] * inMat[14] +
		inMat[0] * inMat[6] * inMat[13] +
		inMat[4] * inMat[1] * inMat[14] -
		inMat[4] * inMat[2] * inMat[13] -
		inMat[12] * inMat[1] * inMat[6] +
		inMat[12] * inMat[2] * inMat[5];

	inv[3] = 
	   -inMat[1] * inMat[6] * inMat[11] +
		inMat[1] * inMat[7] * inMat[10] +
		inMat[5] * inMat[2] * inMat[11] -
		inMat[5] * inMat[3] * inMat[10] -
		inMat[9] * inMat[2] * inMat[7] +
		inMat[9] * inMat[3] * inMat[6];

	inv[7] = 
		inMat[0] * inMat[6] * inMat[11] -
		inMat[0] * inMat[7] * inMat[10] -
		inMat[4] * inMat[2] * inMat[11] +
		inMat[4] * inMat[3] * inMat[10] +
		inMat[8] * inMat[2] * inMat[7] -
		inMat[8] * inMat[3] * inMat[6];

	inv[11] = 
	   -inMat[0] * inMat[5] * inMat[11] +
		inMat[0] * inMat[7] * inMat[9] +
		inMat[4] * inMat[1] * inMat[11] -
		inMat[4] * inMat[3] * inMat[9] -
		inMat[8] * inMat[1] * inMat[7] +
		inMat[8] * inMat[3] * inMat[5];

	inv[15] = 
		inMat[0] * inMat[5] * inMat[10] -
		inMat[0] * inMat[6] * inMat[9] -
		inMat[4] * inMat[1] * inMat[10] +
		inMat[4] * inMat[2] * inMat[9] +
		inMat[8] * inMat[1] * inMat[6] -
		inMat[8] * inMat[2] * inMat[5];

	det = inMat[0] * inv[0] + inMat[1] * inv[4] + inMat[2] * inv[8] + inMat[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		outMat[i] = inv[i] * det;

	return true;
}

// fungsi untuk perkalian matriks 4x4 dengan 4x1
void DotMatrix(float inMat1[16], float inMat2[4], float outMat[4])
{
	outMat[0] = inMat1[0] * inMat2[0] + inMat1[1] * inMat2[1] +
		inMat1[2] * inMat2[2] + inMat1[3] * inMat2[3];
	outMat[1] = inMat1[4] * inMat2[0] + inMat1[5] * inMat2[1] +
		inMat1[6] * inMat2[2] + inMat1[7] * inMat2[3];
	outMat[2] = inMat1[8] * inMat2[0] + inMat1[9] * inMat2[1] +
		inMat1[10] * inMat2[2] + inMat1[11] * inMat2[3];
	outMat[3] = inMat1[12] * inMat2[0] + inMat1[13] * inMat2[1] +
		inMat1[14] * inMat2[2] + inMat1[15] * inMat2[3];
}

// fungsi untuk membuat kurva spline cubic dari 4 titik kontrol
// point1 sampai point4 = titik kontrol
// nPoint = jumlah titik interpolasi antara point1 sampai point4
void drawSplineCubic(Vec3 point1, Vec3 point2, Vec3 point3, Vec3 point4, int nPoint)
{
	// hitung bobot jarak u di masing-masing titik
	float utotal = (abs(point2.X - point1.X) + abs(point3.X - point2.X) + abs(point4.X - point3.X));
	float u1 = 0;
	float u2 = abs(point2.X - point1.X) / utotal;
	float u3 = (abs(point2.X - point1.X) + abs(point3.X - point2.X)) / utotal;
	float u4 = 1;

	// hitung inverse matriks dari koefisien u (lihat slide kuliah)
	float inverseMat[16];
	float coeffMat[16] = {
		1.00f, 0.00f, 0.00f, 0.00f,
		1.00f, u2, pow(u2, 2), pow(u2, 3),
		1.00f, u3, pow(u3, 2), pow(u3, 3),
		1.00f, 1.00f, 1.00f, 1.00f  };
	bool status = inverse(coeffMat, inverseMat);

	// hitung koefisien cubic au^3 + bu^2 + cu + d
	if (status == true)
	{
		float outMatX[4], outMatY[4], outMatZ[4];
		float inMatX[4] = { point1.X, point2.X, point3.X, point4.X };
		float inMatY[4] = { point1.Y, point2.Y, point3.Y, point4.Y };
		float inMatZ[4] = { point1.Z, point2.Z, point3.Z, point4.Z };
		DotMatrix(inverseMat, inMatX, outMatX);
		DotMatrix(inverseMat, inMatY, outMatY);
		DotMatrix(inverseMat, inMatZ, outMatZ);

		// gambar kurva cubic spline dengan titik kontrol diatas
		// hitung posisi y untuk setiap x di setiap point dengan persamaan diatas
		for (int i=0; i<nPoint; i++)
		{	
			float step = 1.0f / nPoint; // jeda setiap titik pd bobot u
			// titik awal
			float pX = point1.X, pY = point1.Y, pZ = point1.Z; 
			//
			float u = 0.0f;
			for (int i = 0; i < nPoint; i++)
			{
				// bentuk segment kurva cubic spline sebanyak nPoint
				u = u + step;
				glVertex3f(pX, pY, pZ); // gambar titik awal
				// koordinat X pada kurva
				pX = outMatX[3] * pow(u, 3) + outMatX[2] * pow(u, 2) 
					+ outMatX[1] * u + outMatX[0]; 
				// koordinat Y pada kurva
				pY = outMatY[3] * pow(u, 3) + outMatY[2] * pow(u, 2) 
					+ outMatY[1] * u + outMatY[0]; 
				// koordinat Z pada kurva
				pZ = outMatZ[3] * pow(u, 3) + outMatZ[2] * pow(u, 2) 
					+ outMatZ[1] * u + outMatZ[0]; 
		
				glVertex3f(pX, pY, pZ); // gambar titik akhir
			}
		}
	}
}

// fungsi ini digunakan untuk menggambar obyek 
void drawObject()
{
	glPushMatrix();

	glRotatef(objectAngle, objectRotation.X, objectRotation.Y, objectRotation.Z);

	// membuat 4 titik kontrol kurva
	Vec3 point1 = Vec3(-150.0f, -70.0f, 0.0f);
	Vec3 point2 = Vec3( -50.0f,  50.0f, 0.0f);
	Vec3 point3 = Vec3(  50.0f,  10.0f, 0.0f);
	Vec3 point4 = Vec3( 150.0f, -50.0f, 0.0f);

	// tandai setiap titik kontrol kurva dengan warna
	markPoint(point1, Vec3(0.0f, 1.0f, 0.0f), 5.0f);
	markPoint(point2, Vec3(1.0f, 0.0f, 0.0f), 5.0f);
	markPoint(point3, Vec3(1.0f, 0.0f, 1.0f), 5.0f);
	markPoint(point4, Vec3(1.0f, 1.0f, 0.0f), 5.0f);

	// mengatur warna obyek menjadi berwarna putih
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_LINES);

	// membuat kurva spline cubic dari titik kontrol diatas
	drawSplineCubic(point1, point2, point3, point4, 30);

	glEnd();

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
	// dalam hal ini warna putih warna hitam (0.0, 0.0, 0.0, 0.0)
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// mengaktifkan depth buffer
	glEnable(GL_DEPTH_TEST);				
	glMatrixMode(GL_PROJECTION);		
	glLoadIdentity();
	// set proyeksi ke proyeksi perspektif
	//gluPerspective(fov, 1.0, 1.0, 100.0);	
	glOrtho((GLfloat)-SCREEN_WIDTH / 2, (GLfloat)SCREEN_WIDTH / 2,
		(GLfloat)-SCREEN_HEIGHT / 2, (GLfloat)SCREEN_HEIGHT / 2, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);				
	glLoadIdentity();						
	// inisialisasi kamera pandang
	// kamera berada di posisi (0.0f, 0.0f, 0.0f)
	gluLookAt(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

// fungsi ini digunakan bila layar akan diresize (default)
// (akan dijelaskan pada praktikum ke-5)
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(fov, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
	glOrtho((GLfloat)-SCREEN_WIDTH / 2, (GLfloat)SCREEN_WIDTH / 2,
		(GLfloat)-SCREEN_HEIGHT / 2, (GLfloat)SCREEN_HEIGHT / 2, 1.0, 100.0);
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
				// percepat laju pergerakan kamera
			moveSpeed = moveSpeed + 0.5f;
			glutPostRedisplay();
		break;
	case 'g':	// bila tombol 'g' pada keyboard ditekan
				// perlambat laju pergerakan kamera
			moveSpeed = moveSpeed - 0.5f > 0.5f ? moveSpeed - 0.5f : 0.5f;
			glutPostRedisplay();
		break;
	case 27:	// bila tombol 'esc' pada keyboard ditekan
				// keluar program
			exit(0);
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
	glutMouseFunc(mousebutton);	// mouse button
	glutMotionFunc(mousemove);	// mouse movement
	//glutTimerFunc(0, timer, 0); // aktifkan timer bila perlu

	// looping
	glutMainLoop();

	return 0;
}