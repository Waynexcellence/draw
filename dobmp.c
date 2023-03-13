#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)
typedef unsigned int uint;
typedef unsigned char uchar;

enum Mode{
    INSTANTLY , BUFFLY
};

typedef struct {           // Total: 54 bytes
    char type[2];          // Magic identifier: 0x4d42
    uint size;             // File size in bytes
    int garbage;           // Not used
    uint offset;           // Offset to image data in bytes from beginning of file (54 bytes)

    uint dib_header_size;  // DIB Header size in bytes (40 bytes)
    int width_px;          // Width of the image
    int height_px;         // Height of image
    short num_planes;      // Number of color planes
    short bits_per_pixel;  // Bits per pixel
    uint compression;      // Compression type
    uint image_size_bytes; // Image size in bytes
    int x_resolution_ppm;  // Pixels per meter
    int y_resolution_ppm;  // Pixels per meter
    uint num_colors;       // Number of colors  
    uint important_colors; // Important colors 
}BMPHeader;

typedef struct {
    BMPHeader header;
    unsigned char** data; 
} BMPImage;

void color( int x ){
    if( x>=0 && x<=15 ) SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE),x);
    else                SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE),7);
}

void get_header( BMPImage* image , FILE* file ){
    fread( image->header.type     , 2 , 1 , file );
    fread( &image->header.size    , 4 , 1 , file );
    fread( &image->header.garbage , 4 , 1 , file );
    fread( &image->header.offset  , 4 , 1 , file );

    fread( &image->header.dib_header_size  , 4 , 1 , file );
    fread( &image->header.width_px         , 4 , 1 , file );
    fread( &image->header.height_px        , 4 , 1 , file );
    fread( &image->header.num_planes       , 2 , 1 , file );
    fread( &image->header.bits_per_pixel   , 2 , 1 , file );
    fread( &image->header.compression      , 4 , 1 , file );
    fread( &image->header.image_size_bytes , 4 , 1 , file );
    fread( &image->header.x_resolution_ppm , 4 , 1 , file );
    fread( &image->header.y_resolution_ppm , 4 , 1 , file );
    fread( &image->header.num_colors       , 4 , 1 , file );
    fread( &image->header.important_colors , 4 , 1 , file );
}

void show_header( BMPImage* image ){
    printf("type: %c%c\n"  , image->header.type[0] , image->header.type[1] );
    printf("size: %u\n"    , image->header.size );
    printf("offset: %u\n"  , image->header.offset );

    printf("dib_header_size: %u\n"  , image->header.dib_header_size  );
    printf("width_px: %d\n"         , image->header.width_px         );
    printf("height_px: %d\n"        , image->header.height_px        );
    printf("num_planes: %d\n"       , image->header.num_planes       );
    printf("bits_per_pixel: %d\n"   , image->header.bits_per_pixel   );
    printf("compression: %u\n"      , image->header.compression      );
    printf("image_size_bytes: %u\n" , image->header.image_size_bytes );
    printf("x_resolution: %d\n"     , image->header.x_resolution_ppm );
    printf("y_resolution: %d\n"     , image->header.y_resolution_ppm );
    printf("num_colors: %u\n"       , image->header.num_colors       );
    printf("important_colors: %u\n" , image->header.important_colors );
}

int main( int argc , char** argv ){
    FILE** file = calloc( argc-1 , sizeof(FILE*) );
    BMPImage* bmpimage = calloc( argc-1 , sizeof(BMPImage) );
    for(int x=1;x<argc;x++){
        file[x-1] = fopen( argv[x] , "rb" );
        get_header( &(bmpimage[x-1]) , file[x-1] );
        assert( ftell(file[x-1]) == 54 );
        if( bmpimage[x-1].header.bits_per_pixel != 24 ){
            printf("picture %d's pixel depth is not 24\n" , x-1 );
            return 0;
        }
        show_header( &(bmpimage[x-1]) );
    }
    
    enum Mode display = BUFFLY;
    uchar bgr[3] = {};

    for(int x=0;x<argc-1;x++){// for argc-1 pictures
        char input[100] = {};
        scanf("%s" , input );// wait for user input
        int padding = bmpimage[x].header.width_px%4;
        if( display == BUFFLY ){
            bmpimage[x].data = calloc( bmpimage[x].header.height_px , sizeof(char*) );
            for(int y=0;y<bmpimage[x].header.height_px;y++){
                bmpimage[x].data[y] = calloc( bmpimage[x].header.width_px , sizeof(char) );
            }
        }
        for(int y=bmpimage[x].header.height_px-1;y>=0;y--){
            fseek( file[x] , bmpimage[x].header.offset+y*(3*bmpimage[x].header.width_px+padding) , SEEK_SET );
            for(int z=0;z<bmpimage[x].header.width_px;z++){
                fread( bgr , 3 , 1 , file[x] );
                if( display == BUFFLY ){
                    if( bgr[0] >= 50 && bgr[1] >= 50 && bgr[2] >= 50 ) bmpimage[x].data[bmpimage[x].header.height_px-1-y][z] = '@';
                    else bmpimage[x].data[bmpimage[x].header.height_px-1-y][z] = '-';
                }
                else if( display == INSTANTLY ){
                    if( bgr[0] >= 50 && bgr[1] >= 50 && bgr[2] >= 50 ) printf("@");
                    else printf("-");
                }
            }
            if( display == INSTANTLY ) printf("\n");
        }
        if( display == BUFFLY ){
            for(int y=0;y<bmpimage[x].header.height_px;y++){
                printf("%s\n" , bmpimage[x].data[y] );
                free( bmpimage[x].data[y] );                
            }
            free( bmpimage[x].data );
        }
    }

    return 0;
}