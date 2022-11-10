/* Tells the compiler not to add padding for these structs. This may
   be useful when reading/writing to binary files.
   http://stackoverflow.com/questions/3318410/pragma-pack-effect
*/
#pragma pack(1)

typedef struct {
    int x;
    int y;
} coord;

typedef struct {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} pixels;

typedef struct {
    pixels culoare;
    int line_width;
} settings;


typedef struct {
    unsigned char  fileMarker1; /* 'B' */
    unsigned char  fileMarker2; /* 'M' */
    unsigned int   bfSize; /* File's size */
    unsigned short unused1; /* Aplication specific */
    unsigned short unused2; /* Aplication specific */
    unsigned int   imageDataOffset; /* Offset to the start of image data */
} bmp_fileheader;

typedef struct {
    unsigned int   biSize; /* Size of the info header - 40 bytes */
    unsigned int     width; /* Width of the image */
    unsigned int     height; /* Height of the image */
    unsigned short planes;
    unsigned short bitPix; /* Number of bits per pixel = 3 * 8 (for each channel R, G, B we need 8 bits */
    unsigned int   biCompression; /* Type of compression */
    unsigned int   biSizeImage; /* Size of the image data */
    int            biXPelsPerMeter;
    int            biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
} bmp_infoheader;

typedef struct {
    bmp_fileheader header;
    bmp_infoheader infoheader;
    pixels **imagine;
} img;

#pragma pack()
