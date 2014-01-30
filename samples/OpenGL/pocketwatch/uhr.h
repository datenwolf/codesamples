#ifndef STRUCT_3D_OBJECT
#define STRUCT_3D_OBJECT
struct C3DObject
{
	float const (*verticies)[2][3];
	int const (*faces)[3];
	int const (*nFaces);
	float const (*matrix);
	float const (*material);
};
#endif/*STRUCT_3D_OBJECT*/
namespace uhr
{
/* <objects> */
/* <object name="boden"> */
extern float const verticies_boden[][2][3];
extern int const faces_boden[][3];
extern int const nFaces_boden;
extern float const matrix_boden[];
extern float const material_boden[];
extern struct C3DObject const object_boden;
/* </object> */

/* <object name="deckel"> */
extern float const verticies_deckel[][2][3];
extern int const faces_deckel[][3];
extern int const nFaces_deckel;
extern float const matrix_deckel[];
extern float const material_deckel[];
extern struct C3DObject const object_deckel;
/* </object> */

/* <object name="gehaeuse"> */
extern float const verticies_gehaeuse[][2][3];
extern int const faces_gehaeuse[][3];
extern int const nFaces_gehaeuse;
extern float const matrix_gehaeuse[];
extern float const material_gehaeuse[];
extern struct C3DObject const object_gehaeuse;
/* </object> */

/* <object name="minutenzeiger"> */
extern float const verticies_minutenzeiger[][2][3];
extern int const faces_minutenzeiger[][3];
extern int const nFaces_minutenzeiger;
extern float const matrix_minutenzeiger[];
extern float const material_minutenzeiger[];
extern struct C3DObject const object_minutenzeiger;
/* </object> */

/* <object name="sekundenzeiger"> */
extern float const verticies_sekundenzeiger[][2][3];
extern int const faces_sekundenzeiger[][3];
extern int const nFaces_sekundenzeiger;
extern float const matrix_sekundenzeiger[];
extern float const material_sekundenzeiger[];
extern struct C3DObject const object_sekundenzeiger;
/* </object> */

/* <object name="sekziffernblatt"> */
extern float const verticies_sekziffernblatt[][2][3];
extern int const faces_sekziffernblatt[][3];
extern int const nFaces_sekziffernblatt;
extern float const matrix_sekziffernblatt[];
extern float const material_sekziffernblatt[];
extern struct C3DObject const object_sekziffernblatt;
/* </object> */

/* <object name="stundenzeiger"> */
extern float const verticies_stundenzeiger[][2][3];
extern int const faces_stundenzeiger[][3];
extern int const nFaces_stundenzeiger;
extern float const matrix_stundenzeiger[];
extern float const material_stundenzeiger[];
extern struct C3DObject const object_stundenzeiger;
/* </object> */

/* <object name="ziffernblatt"> */
extern float const verticies_ziffernblatt[][2][3];
extern int const faces_ziffernblatt[][3];
extern int const nFaces_ziffernblatt;
extern float const matrix_ziffernblatt[];
extern float const material_ziffernblatt[];
extern struct C3DObject const object_ziffernblatt;
/* </object> */

/* </objects> */
};